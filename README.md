# EEPROM Programming Tool

This is a simple serial EEPROM programmer (SPI and I2C) that I developed to
help with development of my [TGL-6502](http://thegaragelab.com/posts/introducing-the-tgl-6502-single-board-computer.html)
project. It is based around an ATtiny84 and uses [the Arduino core](https://github.com/damellis/attiny/)
for that chip.

For full documentation on the project including the communications protocol
please see [the project page](http://thegaragelab.com/posts/a-simple-serial-i2c-spi-eeprom-programmer.html)
on The Garage Lab website.

## Supported EEPROMS

At the moment I only support EEPROMS that I have in my parts collection. The
pin outs and command sets for most serial EEPROMS are very similar so adding
support for other types would not be difficult.

|Part    |Interface|Total Size      |Page Size|Address Size  |EEProg ID|
|--------|---------|----------------|---------|--------------|---------|
|24C65   |I2C      |64Kbit (8K x 8) |64 bytes |16 bit        |(TBD)    |
|24LC1025|I2C      |1Mbit (128K x 8)|128 bytes|16 bit + block|(TBD)    |
|25AA1024|SPI      |1Mbit (128K x 8)|256 bytes|24 bit        |0x7830   |
|25LC1024|SPI      |1Mbit (128K x 8)|256 bytes|24 bit        |0x7830   |
|25AA640 |SPI      |64Kbit (8K x 8) |32 bytes |16 bit        |0x4620   |

## Client Software

The repository contains a simple Windows GUI client that can be used to load
raw binary files into an EEPROM and to extract the existing contents of an EEPROM
to a file. This client is implemented in C# and provided as a Visual Studio
solution - you can use the [Community Edition](http://www.visualstudio.com/en-us/news/vs2013-community-vs.aspx)
to compile and modify it.

The programmer is controlled over a serial port (57600 8/N/1) using a very simple
ASCII based protocol that support partial page writes and reading/writing arbitrary
locations in the EEPROM so the client could be extended to add support for these
features. I will probably start adding these features as I need them as well as
writing a Linux command line version of the programmer. If you want to add the
features please feel free to send me a patch or a pull request so I can add them
to the repository.

