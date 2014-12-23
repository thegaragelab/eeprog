# EEPROM Programming Tool

This is a simple serial EEPROM programmer (SPI and I2C) that I developed to
help with development of my [TGL-6502](http://thegaragelab.com/posts/introducing-the-tgl-6502-single-board-computer.html)
project. It is based around an ATtiny84.

## Supported EEPROMS

At the moment I only support EEPROMS that I have in my parts collection. The
pin outs and command sets for most serial EEPROMS are very similar so adding
support for other types would not be difficult.

|Part    |Interface|Total Size      |Page Size|Address Size  |
|--------|---------|----------------|---------|--------------|
|24C65   |I2C      |64Kbit (8K x 8) |64 bytes |16 bit        |
|24LC1025|I2C      |1Mbit (128K x 8)|128 bytes|16 bit + block|
|25AA1024|SPI      |1Mbit (128K x 8)|256 bytes|24 bit        |
|25LC1024|SPI      |1Mbit (128K x 8)|256 bytes|24 bit        |
|25AA640 |SPI      |64Kbit (8K x 8) |32 bytes |16 bit        |
