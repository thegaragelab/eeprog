using System;
using System.IO;
using System.IO.Ports;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;

namespace eeprog
{
  /// <summary>
  /// State information for progress events.
  /// </summary>
  public enum ProgressState
  {
    Read,   // Read a page
    Write,  // Wrote a page
    Verify, // Verified a page
    Error,  // Intermediate (non-fatal) error
  }

  /// <summary>
  /// Direction of communications.
  /// </summary>
  public enum Direction
  {
    Input,
    Output
  }

  /// <summary>
  /// The possible connection states.
  /// </summary>
  public enum ConnectionState
  {
    Connecting,   // Device is connecting
    Connected,    // Device is connected
    Disconnected, // Device is not connected
  }

  /// <summary>
  /// The possible operations the device can be performing.
  /// </summary>
  public enum Operation
  {
    Idle,    // No operation is being performed
    Reading, // Reading flash contents
    Writing, // Writing flash contents
  }

  public class Response
  {
    public string Message
    {
      get;
      set;
    }

    public byte[] Data
    {
      get;
      set;
    }
  }

  /// <summary>
  /// Exceptions for protocol events.
  /// </summary>
  public class ProtocolException : Exception
  {
    public ProtocolException(string message)
      : base(message)
    {
      // Do nothing
    }

    public ProtocolException(string message, Exception inner)
      : base(message, inner)
    {
      // Do nothing
    }
  }

  /// <summary>
  /// This class wraps all interaction with the remote machine in a background
  /// thread.
  /// </summary>
  public class DeviceLoader
  {
    #region "Constants"
    /// <summary>
    /// Maximum number of retries for failed operations
    /// </summary>
    public const int MAX_RETRIES = 3;

    /// <summary>
    /// The baud rate to use.
    /// </summary>
    private const int BAUD_RATE = 57600;

    /// <summary>
    /// Identifier string used to detect the device
    /// </summary>
    private const string SIGNATURE= "EEPROG V0.1";

    /// <summary>
    /// Character to send to request EEPROM mode
    /// </summary>
    private const byte MODE_REQUEST_CHAR = 0x55;

    /// <summary>
    /// Character sent to indicate success
    /// </summary>
    private const byte OPERATION_SUCCESS = 0x2B; // '+'

    /// <summary>
    /// Character sent to indicate failure
    /// </summary>
    private const byte OPERATION_FAILED = 0x2D; // '-'

    /// <summary>
    /// Command code to start reading
    /// </summary>
    private const byte COMMAND_READ = 0x52; // 'R'

    /// <summary>
    /// Command code to start writing
    /// </summary>
    private const byte COMMAND_WRITE = 0x57; // 'W'
    #endregion

    #region "Events"
    public delegate void ProgressHandler(DeviceLoader sender, ProgressState state, int position, int target, string message);
    public event ProgressHandler Progress;

    public delegate void CommunicationsHandler(DeviceLoader sender, Direction direction, string content);
    public event CommunicationsHandler Communications;

    public delegate void ConnectionStateChangedHandler(DeviceLoader sender, ConnectionState state);
    public event ConnectionStateChangedHandler ConnectionStateChanged;

    public delegate void ErrorHandler(DeviceLoader sender, string message, Exception ex);
    public event ErrorHandler Error;
    #endregion

    #region "Properties"
    /// <summary>
    /// The current connection state.
    /// </summary>
    public ConnectionState ConnectionState
    {
      get;
      private set;
    }

    /// <summary>
    /// The active operation. This property is only valid when the device is
    /// in the 'Connected' state.
    /// </summary>
    public Operation Operation
    {
      get;
      private set;
    }

    /// <summary>
    /// Indicates whether the loader is busy or not.
    /// </summary>
    public bool Busy
    {
      get;
      private set;
    }

    /// <summary>
    /// Provide access to the data read or written to device.
    /// </summary>
    public byte[] Data
    {
      get;
      private set;
    }
    #endregion

    #region "Instance Variables"
    private bool           m_cancel;     // Cancel of the current operation
    private AutoResetEvent m_event;      // Event to control command queue
    private SerialPort     m_serial;     // The serial port for communication
    #endregion

    #region "Event Dispatch"
    private void FireError(string message, Exception ex = null)
    {
      ErrorHandler handler = Error;
      if (handler != null)
	handler(this, message, ex);
    }

    private void FireProgress(ProgressState state, int position, int target, string message = null)
    {
      ProgressHandler handler = Progress;
      if (handler != null)
        handler(this, state, position, target, message);
    }

    private void FireCommunications(Direction direction, string content)
    {
      CommunicationsHandler handler = Communications;
      if (handler != null)
        handler(this, direction, content);
    }

    private void FireConnectionStateChanged(ConnectionState state)
    {
      ConnectionStateChangedHandler handler = ConnectionStateChanged;
      if (handler != null)
        handler(this, state);
    }
    #endregion

    #region "Helper methods"
    public int GetHexVal(char hex)
    {
      int val = (int)hex;
      return val - (val < 58 ? 48 : (val < 97 ? 55 : 87));
    }

    public byte[] StringToByteArray(string hex)
    {
      if (hex.Length % 2 == 1)
        throw new Exception("The binary key cannot have an odd number of digits");
      byte[] arr = new byte[hex.Length >> 1];
      for (int i = 0; i < hex.Length >> 1; ++i)
      {
          arr[i] = (byte)((GetHexVal(hex[i << 1]) << 4) + (GetHexVal(hex[(i << 1) + 1])));
      }
      return arr;
    }

    public string HexString(byte[] data, UInt32 address, int offset, int size)
    {
      StringBuilder builder = new StringBuilder();
      UInt16 checksum = 0;
      // Add the address to the checksum
      checksum += (UInt16)((address >> 16) & 0x00ff);
      checksum += (UInt16)((address >> 8) & 0x00ff);
      checksum += (UInt16)(address & 0x00ff);
      // And to the string
      builder.AppendFormat("{0:x6}", address);
      // Now add the data bytes
      for (int i = 0; (i < size) && ((offset + i) < data.Length); i++)
      {
        builder.AppendFormat("{0:x2}", data[offset + i]);
        checksum += (UInt16)data[offset + i];
      }
      // Add the check sum to the end of the line
      builder.AppendFormat("{0:x4}", checksum);
      return builder.ToString();
    }
    #endregion

    #region "Implementation"
    /// <summary>
    /// Open the serial port. Exceptions are passed on to the caller.
    /// </summary>
    /// <param name="port"></param>
    private void OpenPort(string port)
    {
      m_serial = new SerialPort(port, BAUD_RATE, Parity.None, 8, StopBits.One);
      m_serial.ReadTimeout = 2500;
      m_serial.Open();
    }

    private void CheckSignature(string response)
    {
      if (response != SIGNATURE)
        throw new ProtocolException("Invalid response from programmer.");
    }

    private Response CheckResponse(string response)
    {
      Response result = new Response();
      bool success = (response[0] == '+');
      result.Message = response.Substring(1);
      if (!success)
      {
        if ((result.Message == null) || (result.Message.Length == 0))
          throw new ProtocolException("No message provided.");
        throw new ProtocolException(result.Message);
      }
      else
      {
        // Try and convert the string into data
        try
        {
          result.Data = StringToByteArray(result.Message);
        }
        catch
        {
          // Ignore conversion exceptions.
        }
      }
      return result;
    }

    private void VerifyChecksum(byte[] data)
    {
      if ((data == null) || (data.Length < 2))
        throw new ProtocolException("Insufficient data received.");
      UInt16 checksum = 0;
      for (int i = 0; i < (data.Length - 2); i++)
        checksum += (UInt16)data[i];
      UInt16 received = (UInt16)((data[data.Length - 2] << 8) | data[data.Length - 1]);
      if (checksum != received)
        throw new ProtocolException(String.Format("Invalid checksum - calculated {0:X4}, expected {1:X4}", checksum, received));
    }

    /// <summary>
    /// Consume any pending input.
    /// </summary>
    private void FlushInput()
    {
      try
      {
        m_serial.ReadTimeout = 250;
        while (true)
          m_serial.ReadByte();
      }
      catch (TimeoutException)
      {

      }
      finally
      {
        m_serial.ReadTimeout = 2500;
      }
    }

    /// <summary>
    /// Send a command and read the response line.
    /// </summary>
    /// <param name="cmd"></param>
    /// <returns></returns>
    private string SendCommand(string cmd)
    {
      ASCIIEncoding encoding = new ASCIIEncoding();
      // Send the command
      byte[] data = encoding.GetBytes(cmd + "\n");
      m_serial.Write(data, 0, data.Length);
      FireCommunications(Direction.Output, cmd);
      // Wait for the response
      List<byte> result = new List<byte>();
      for (int ch = m_serial.ReadByte(); (ch != '\n') && (result.Count < 128); ch = m_serial.ReadByte())
        result.Add((byte)ch);
      string response = encoding.GetString(result.ToArray(), 0, result.Count).TrimEnd();
      FireCommunications(Direction.Input, response);
      return response;
    }

    private string SendCommand(char cmd, UInt16 ident)
    {
      return SendCommand(String.Format("{0}{1:x4}", cmd, ident));
    }
    #endregion

    #region "Public Methods"
    public DeviceLoader()
    {
      m_event = new AutoResetEvent(false);
      ConnectionState = ConnectionState.Disconnected;
    }

    public void Read(string port, EEPROM eeprom, UInt32 offset, UInt32 size, FileInfo target)
    {
      if (Operation != Operation.Idle)
        throw new InvalidOperationException("Operation already in progress.");
      Operation = Operation.Reading;
      try
      {
        // Establish a connection
        FireConnectionStateChanged(ConnectionState.Connecting);
        OpenPort(port);
        FlushInput();
        CheckSignature(SendCommand("!"));
        FireConnectionStateChanged(ConnectionState.Connected);
        // Set the EEPROM identifier
        CheckResponse(SendCommand('i', eeprom.ID));
        // Read the data
        byte[] data = new byte[size];
        UInt32 received = 0;
        FireProgress(ProgressState.Read, 0, (int)size + 1, "Reading data.");
        while (received < size)
        {
          // Ask for the next block
          Response response = CheckResponse(SendCommand(String.Format("r{0:x6}", offset)));
          VerifyChecksum(response.Data);
          // Add it to the array
          for (int i = 3; (i < (response.Data.Length - 2)) && (received < size); i++, offset++)
            data[received++] = response.Data[i];
          // Update progress
          FireProgress(ProgressState.Read, (int)offset, (int)size + 1);
        }
        // Now save the data to the file
        FireProgress(ProgressState.Read, (int)offset, (int)size + 1, String.Format("Saving to '{0}'", target.FullName));
        File.WriteAllBytes(target.FullName, data);
      }
      catch (ProtocolException ex)
      {
        FireError(ex.Message);
      }
      catch (Exception ex)
      {
        FireError("Unexpected error during operation.", ex);
      }
      finally
      {
        try
        {
          if (m_serial != null)
            m_serial.Close();
          m_serial = null;
        }
        catch {
          // Just ignore it
        }
        FireConnectionStateChanged(ConnectionState.Disconnected);
        Operation = Operation.Idle;
      }
    }

    public void Write(string port, EEPROM eeprom, UInt32 offset, FileInfo source)
    {
      if (Operation != Operation.Idle)
        throw new InvalidOperationException("Operation already in progress.");
      Operation = Operation.Writing;
      try
      {
        // Read the data from the file
        byte[] data = File.ReadAllBytes(source.FullName);
        // Establish a connection
        FireConnectionStateChanged(ConnectionState.Connecting);
        OpenPort(port);
        FlushInput();
        CheckSignature(SendCommand("!"));
        FireConnectionStateChanged(ConnectionState.Connected);
        // Set the EEPROM identifier
        CheckResponse(SendCommand('i', eeprom.ID));
        // Write the data
        int written = 0;
        FireProgress(ProgressState.Write, 0, data.Length + 1, "Writing data.");
        while (written < data.Length)
        {
          // Ask for the next block
          Response response = CheckResponse(SendCommand("w" + HexString(data, offset, written, Math.Min(32, data.Length - written))));
          // Update progress
          FireProgress(ProgressState.Write, (int)offset, data.Length + 1);
        }
        // Finish the operation
        CheckResponse(SendCommand("d"));
      }
      catch (ProtocolException ex)
      {
        FireError(ex.Message);
      }
      catch (Exception ex)
      {
        FireError("Unexpected error during operation.", ex);
      }
      finally
      {
        try
        {
          if (m_serial != null)
            m_serial.Close();
          m_serial = null;
        }
        catch
        {
          // Just ignore it
        }
        FireConnectionStateChanged(ConnectionState.Disconnected);
        Operation = Operation.Idle;
      }
    }

    /// <summary>
    /// Cancel the current operation.
    /// </summary>
    public void Cancel()
    {
      // Make sure we are connected
      if (ConnectionState != ConnectionState.Connected)
	throw new InvalidOperationException("Device is not connected.");
      // Only cancel if an operation is in progress
      if (Operation != Operation.Idle)
	m_cancel = true;
    }
    #endregion
  }
}
