This version of the firmware uses the Arduino core for the ATtiny84 from
[here](https://github.com/damellis/attiny/). Starting the project with an
incomplete version of my new Microbe library was a bad move in hindsight -
rather than divert energy to working on the library right now the Arduino
core should get me up and running a bit faster.

If you point your Arduino sketchbook directory to the '*firmware*' directory
you can open the '*eeprog*' sketch as well as have access to the ATtiny boards
in the board list. You will require an AVR ISP programmer (you can use an Arduino
as a programmer) and the EEprog hardware has a 10 pin ISP header built in to
make updating the firmware easier.

