https://github.com/escaner/ufc

UFC is an Arduino project to interface a button box made of button switches
and rotary encoders with a computer and identify itself as a game controller
sending DirectX events.

The core of the system is a Sparkfun Pro Micro or clone (small Arduino
Micro/Leonardo compatible). The switches are attached directly to the Arduino
analog inputs grids with resistors, while the encoders directly attached
to digital pins and managed by the custom made REncoderAsync library.
The Joystick library interfaces with the computer and presents the Arduino
itself as a game controller.

More info can be found in the doc directory, including schematics and
pictures.

* To test whether the controller is recognized by a Linux the computer, you can
use the following command:

$ dmesg | grep Joystick

* Use the device name as follows to check the events sent to the computer:
$ jstest /dev/input/js1


Copyright (C) 2021, Óscar Laborda

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
