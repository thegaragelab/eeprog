using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace eeprog
{
  public enum ConnectionType
  {
    SPI = 0,
    I2C = 1,
  }


  public class EEPROM
  {
    private int m_pageBits; //!< Number of bits per page
    private int m_sizeBits; //!< Number of bits in address
    private int m_addrBytes; //!< Number of bytes in address

    public ConnectionType Connection
    {
      get;
      set;
    }

    /// <summary>
    /// Number of bits in a page.
    /// </summary>
    public int PageBits
    {
      get
      {
        return m_pageBits;
      }

      set
      {
        if ((value < 1) || (value > 8))
          throw new ArgumentException("Invalid page size.");
        m_pageBits = value;
      }
    }

    /// <summary>
    /// Size of an individual page.
    /// </summary>
    public UInt16 PageSize
    {
      get
      {
        return (UInt16)(1 << PageBits);
      }
    }
    /// <summary>
    /// Number of bits in an address (total EEPROM size)
    /// </summary>
    public int SizeBits
    {
      get
      {
        return m_sizeBits;
      }

      set
      {
        if ((value < 1) || (value > 32))
          throw new ArgumentException("Invalid chip size.");
        m_sizeBits = value;
      }
    }

    /// <summary>
    /// Total EEPROM size
    /// </summary>
    public UInt32 Size
    {
      get
      {
        return ((UInt32)1 << SizeBits);
      }
    }

    public UInt16 ID
    {
      get
      {
        UInt16 value = (Connection == ConnectionType.I2C) ? (UInt16)0x8000 : (UInt16)0x0000;
        value |= (UInt16)((PageBits - 1) << 12);
        value |= (UInt16)((SizeBits - 1) << 7);
        value |= (UInt16)(AddressBytes << 4);
        return value;
      }
    }
    /// <summary>
    /// Get the size (in Kb) as a string.
    /// </summary>
    public string SizeInK
    {
      get
      {
        return (Size / 1024).ToString() + "K";
      }
    }

    /// <summary>
    /// Number of bytes to send for an address.
    /// </summary>
    public int AddressBytes
    {
      get
      {
        return m_addrBytes;
      }

      set
      {
        if ((value < 1) || (value > 4))
          throw new ArgumentException("Invalid address size");
        m_addrBytes = value;
      }
    }

    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="connection"></param>
    /// <param name="pageBits"></param>
    /// <param name="sizeBits"></param>
    /// <param name="addrBytes"></param>
    public EEPROM(ConnectionType connection, int pageBits, int sizeBits, int addrBytes)
    {
      Connection = connection;
      PageBits = pageBits;
      SizeBits = sizeBits;
      AddressBytes = addrBytes;
    }
  }
}
