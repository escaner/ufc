https://github.com/escaner/ufc

UFC is an Arduino project to interface a button box made of button switches
and rotary encoders, LCD and LEDs, with a computer and identify itself as
a game controller sending DirectX events and communicating also via DCS-BIOS.

The core of the system is a Sparkfun Pro Micro or clone (small Arduino
Micro/Leonardo compatible). The switches are attached directly to the Arduino
analog input grids with resistors, while the encoders are directly attached
to digital pins and managed by the custom made REncoderAsync library.
The Joystick library interfaces with the computer and presents the Arduino
itself as a game controller.

This project requires the following libraries:
* https://github.com/dcs-bios/dcs-bios
* https://github.com/MHeironimus/ArduinoJoystickLibrary
* https://github.com/escaner/REncoder
* https://github.com/escaner/Switch
* https://github.com/fmalpartida/New-LiquidCrystal
* EEPROM

More info can be found in the doc directory, including schematics and
pictures.

Material used (most components were purchased on Aliexpress)
* 3D printer + PLA filament to make the case
* Adhesive vinyl with carbon fiber finish
* Paper printer, paper and glue for labels
* Hot glue gun + glue to stick the Pro Micro and other portoboard assemblies
* Soldering iron, solder, wires and other related tools
* Protoboard for LSK assembly, LED resistors and +/- hubs
* Dupont terminals (optional)
* 1 x USB A-microB cable
* 1 x Pro Micro 5V (Arduino/Sparkfun clone)
* 1 x Robotdyn 4x4 keypad module
* 1 x 2004 LCD display with I2C module
* Resistors for two matrices of buttons: 6 x 3.3K, 4 x 1K, 2 x 10K
* Resistors for LEDs: 2 x 180 Ohm (actual value depends on the LEDs)
* 1 x small momentary pushbutton (the red one)
* 2 x 15mm linear potentiometer 10K + knobs
* 8 x DIP tacile button 10mm 6x6 + 6mm caps
* 4 x rotary encoders with pushbutton + knobs
* 2 x 12mm 3-position momentary switch 3pin (SH T8014A-Z1)
* 6 x 12mm momentary pushbuttons
* 1 x 16mm momentary pushbutton with red LED for 5V
* 2 x 5mm green LEDs + housing
* 19 x M3 screws and nuts, serveral lengths (or saw to trim them!)
* 4 x adhesive rubber feet

To test whether the controller is recognized by a Linux the computer, you can
use the following command:

$ dmesg | grep Joystick

* Use the device name as follows to check the events sent to the computer:
$ jstest /dev/input/js1


Copyright (C) 2021-2022, Óscar Laborda

This file is part of UFC software.

UFC is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

UFC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UFC.  If not, see <https://www.gnu.org/licenses/>.
