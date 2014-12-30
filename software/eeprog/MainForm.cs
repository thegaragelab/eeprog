using System;
using System.IO;
using System.IO.Ports;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace eeprog
{
  public partial class MainForm : Form
  {
    private Dictionary<string, EEPROM> m_eeproms;
    private DeviceLoader m_loader;

    public MainForm()
    {
      InitializeComponent();
      m_loader = new DeviceLoader();
      m_loader.ConnectionStateChanged += OnConnectionStateChanged;
      m_loader.Error += OnError;
      m_loader.Communications += OnCommunications;
      m_loader.Progress += OnProgress;
      // Populate known EEPROM types
      m_eeproms = new Dictionary<string, EEPROM>();
      m_eeproms.Add(
        "25AA640A",
        new EEPROM(ConnectionType.SPI, 5, 13, 2)
        );
      m_eeproms.Add(
        "25AA1024",
        new EEPROM(ConnectionType.SPI, 8, 17, 3)
        );
      // Populate lists
      m_lstChipType.Items.Add(ConnectionType.I2C.ToString());
      m_lstChipType.Items.Add(ConnectionType.SPI.ToString());
      for (int i = 0; i <= 8; i++)
        m_lstPageSize.Items.Add((1 << i).ToString());
      for (int i = 0; i <= 7; i++)
        m_lstCapacity.Items.Add((1 << i).ToString() + "K");
      for (int i = 1; i <= 4; i++)
        m_lstAddressSize.Items.Add(i.ToString());
      // Populate the list of available COM ports
      List<string> ports = new List<string>(SerialPort.GetPortNames());
      ports.Sort();
      m_lstPort.Items.AddRange(ports.ToArray());
      if (m_lstPort.Items.Count > 0)
        m_lstPort.SelectedIndex = 0;
      // Do EEPROM last, it will trigger changes in other fields
      foreach (string name in m_eeproms.Keys)
        m_lstEEPROM.Items.Add(name);
      m_lstEEPROM.SelectedIndex = 0;
    }

    #region "UI Events"

    private void OnEEPROMChanged(object sender, EventArgs e)
    {
      EEPROM eeprom;
      if (!m_eeproms.TryGetValue(m_lstEEPROM.SelectedItem.ToString(), out eeprom))
        return; // TODO: Should not happen
      // Update the EEPROM info field
      m_lstChipType.SelectedItem = eeprom.Connection.ToString();
      m_lstPageSize.SelectedItem = eeprom.PageSize.ToString();
      m_lstCapacity.SelectedItem = eeprom.SizeInK;
      m_lstAddressSize.SelectedItem = eeprom.AddressBytes.ToString();
    }

    private void OnReadClick(object sender, EventArgs e)
    {
      // Get the port and the EEPROM to use
      string port = m_lstPort.SelectedItem.ToString();
      EEPROM eeprom;
      if (!m_eeproms.TryGetValue(m_lstEEPROM.SelectedItem.ToString(), out eeprom))
        {
        MessageBox.Show("No EEPROM selected.", "Error!", MessageBoxButtons.OK, MessageBoxIcon.Error);
        return;
        }
      // Determine where to save the file
      SaveFileDialog dlg = new SaveFileDialog();
      dlg.DefaultExt = "rom";
      dlg.AddExtension = true;
      dlg.OverwritePrompt = true;
      dlg.Title = "Save ROM Image";
      dlg.Filter = "ROM Image (*.rom)|*.rom";
      DialogResult result = dlg.ShowDialog();
      if (result == DialogResult.OK)
      {
        // Start the reading task and save the data if successful
        Task.Factory.StartNew(() => {
          m_loader.Read(port, eeprom, 0, eeprom.Size, new FileInfo(dlg.FileName));
          });
      }
    }

    private void OnWriteClick(object sender, EventArgs e)
    {
      // Get the port and the EEPROM to use
      string port = m_lstPort.SelectedItem.ToString();
      EEPROM eeprom;
      if (!m_eeproms.TryGetValue(m_lstEEPROM.SelectedItem.ToString(), out eeprom))
      {
        MessageBox.Show("No EEPROM selected.", "Error!", MessageBoxButtons.OK, MessageBoxIcon.Error);
        return;
      }
      // Determine what file to write
      OpenFileDialog dlg = new OpenFileDialog();
      dlg.DefaultExt = "rom";
      dlg.AddExtension = true;
      dlg.CheckFileExists = true;
      dlg.Title = "Open ROM Image";
      dlg.Filter = "ROM Image (*.rom)|*.rom";
      DialogResult result = dlg.ShowDialog();
      if (result == DialogResult.OK)
      {
        // Start the reading task and save the data if successful
        Task.Factory.StartNew(() =>
        {
          m_loader.Write(port, eeprom, 0, new FileInfo(dlg.FileName));
        });
      }
    }

    #endregion

    private void LogMessage(TextBox target, string message)
    {
      target.AppendText(message + "\n");
    }

    private void UpdateUI()
    {
      if (m_loader.Operation == Operation.Idle)
      {
        m_btnRead.Enabled = true;
        m_btnWrite.Enabled = true;
        m_lstEEPROM.Enabled = true;
        m_lstPort.Enabled = true;
        m_progress.Value = 0;
      }
      else
      {
        m_btnRead.Enabled = false;
        m_btnWrite.Enabled = false;
        m_lstEEPROM.Enabled = false;
        m_lstPort.Enabled = false;
      }
    }

    #region "Device Events"

    void OnProgress(DeviceLoader sender, ProgressState state, int position, int target, string message)
    {
      if (InvokeRequired)
      {
        BeginInvoke(new Action(() => { OnProgress(sender, state, position, target, message); }));
        return;
      }
      // Safe to use UI
      m_progress.Maximum = target;
      m_progress.Value = position;
      if (message != null)
        LogMessage(m_txtMessages, message);
    }

    void OnCommunications(DeviceLoader sender, Direction direction, string content)
    {
      if (InvokeRequired)
      {
        BeginInvoke(new Action(() => { OnCommunications(sender, direction, content); }));
        return;
      }
      // Safe to use UI
      LogMessage(m_txtComms, String.Format("{0} {1}", (direction == Direction.Input) ? "<" : ">", content));
    }

    void OnError(DeviceLoader sender, string message, Exception ex)
    {
      if (InvokeRequired)
      {
        BeginInvoke(new Action(() => { OnError(sender, message, ex); }));
        return;
      }
      // Safe to use UI
      if (ex != null)
        message = message + "\n" + ex.Message;
      LogMessage(m_txtMessages, "ERROR: " + message + "\n");
      MessageBox.Show(message, "Error!", MessageBoxButtons.OK, MessageBoxIcon.Error);
    }

    void OnConnectionStateChanged(DeviceLoader sender, ConnectionState state)
    {
      if (InvokeRequired)
      {
        BeginInvoke(new Action(() => { OnConnectionStateChanged(sender, state); }));
        return;
      }
      // Safe to use UI
      UpdateUI();
      LogMessage(m_txtMessages, String.Format("Programmer {0}", state));
    }

    #endregion

  }
}
