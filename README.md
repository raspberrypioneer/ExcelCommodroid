# Commodore computer loader using MS Excel on Windows
This project offers a cheap and simple way of loading software onto a Commodore 64 or Commodore Vic-20 retro-computer from a macro-enabled Excel spreadsheet. It consists of two main components, an Arduino microcontroller which connects to the Commodore serial port, and the spreadsheet which provides the means to select and serve the programs to the Commodore.

![c64 tab](./docs/c64-tab.png)
![vic20 tab](./docs/vic20-tab.png)

Main features
- Load D64, PRG and T64 program files, including multi-loaders to your Commodore
- Select programs from the comfort and familiarity of an Excel spreadsheet
- View box art for the program where this has been set-up
- Switch between the Commodore 64 or Vic-20 computers if you have both computers
- Customise default settings including the Arduino pin connections
- Works well for many C64 games and Vic-20 games requiring memory expansion e.g. a 35K switchable memory expansion
- Works with the C64 EPYX fast load cartridge

## Hardware Requirements
- A Windows PC running Microsoft Excel. Excel Microsoft 365 running on a 10 year old Windows 10 laptop and also a new Windows 11 computer were used for testing this project
- For connecting to the Commodore, a 5V 16Mhz Arduino / clone microcontroller is used. This may be an ATmega32U4 device (Micro, Pro-Micro and variations) or ATmega328P device (Uno, Nano)
- The connections required are simple, go to the Hardware Interface section for details and diagrams

## Installation
- Copy the `ExcelCommodroid.xlsm` macro-enabled spreadsheet to a folder location on the PC
- When the spreadsheet is first opened, a message appears `SECURITY WARNING Some active content has been disabled. Click for more details`. This is due to the macros needed to run the loader software

    ![security warning macros](./docs/security-warning-macros.png)

- To allow the macros to run and use the spreadsheet, choose `Enable content`. If you prefer to understand more before doing so, see the Software Notes section in this guide
- Next, to compile and upload the Arduino software, download and install the [Arduino IDE](https://www.arduino.cc/en/software)
- Use the Arduino IDE to open the [sketch](./commodore_sketch/commodore_sketch.ino)
- If using the C64 EPYX fast load cartridge, additional steps are needed. Refer to that section for details
- Select the Arduino board and port. Pro-micro clones are often mentioned as being Arduino Leonardo compliant. If yours is like this, choose Arduino Leonardo as your board
- Compile and upload the Arduino sketch

## Configuration settings
- This is typically done once by updating the `settings` tab values

    ![settings tab](./docs/settings-tab.png)

- `COMPORT` is the choice of Windows USB / COM port being used. If you connect the Arduino to the same USB port each time, the COM port number usually remains the same
- Find the COM port being used by connecting the Arduino to a USB port and type `system` in the Windows search box. Choose the `System Information` app, then navigate to the serial ports section as shown

    ![find COM port](./docs/find-COM-port.png)

- If the COM port number being used is greater than 20 (probably unlikely), then the maximum port number value will need changing in the `SERIAL_PORT_VBA` code. This is done by opening this module via `Developer > Visual Basic > VBAProject > Modules` and editing the value at the top of the file
- `PINS` are the pins used on the Arduino mapped to the Commodore serial interface (Atn, Clock, Data, Reset pins). The Hardware Interface section contains more details about this
- `MEDIA_FOLDER` defines where the Commodore games / roms and box art images should be located. This project does not include Commodore programs as there are potential copyright implications in doing so. These have to be sourced independently and copied to the media folder

    ![folder structure](./docs/folder-structure.png)

- D64, PRG and T64 program files are kept in `c64` and `vic20` subfolders
- Box art images are optional, but if used must be located in the `art` subfolders and given the same name as their corresponding game with a jpg file extension, e.g. `Blue Max.d64` may have an associated image called `Blue Max.jpg`. RetroPie style image naming also works, e.g. `Blue Max-image.jpg`. Note that png image files unfortunately do not work

## Usage
- Ensure all the physical connections are made between the Commodore, Arduino and PC. Check the Hardware Interface section for details and diagrams
- Remember that the Commodore should always be turned off when physically connecting to it
- On the `c64` or `vic20` tab, click the `Connect` button to connect to the Arduino. A colour indicator around the button shows the status of the connection

| Status | Meaning |
| ------ | ------- |
| `Background colour` | Connection not started |
| `Red` | Not connected |
| `Orange` | Connected ok, handshake in progress. This status is very short and usually not seen for long |
| `Green` | Arduino handshake ok, ready to go |

- If there is a problem connecting, reset the Arduino and wait 5-10 seconds, then click `Disconnect` and `Connect` again

- To select a program to load, click on the cell of the program you want or alternatively, enter the program name in the cell beneath the `Connect` button. Usually Excel will find the name before you type it in full

- Click on the `Show box art` button to see box art associated with the program. Clicking through the program names will show art for the selected program where available

- Once you have selected the game required, over on the Commodore issue the required `LOAD` command

| Command | Description |
| ------- | ----------- |
| `LOAD "*",8` | Used to load the selected D64, PRG or T64 based program. The disk drive number is always 8 |
| `LOAD "*",8,1` | This version of the load command is needed for many Vic-20 games and some C64 games. The program will be loaded at the memory location given in the first two bytes of a PRG file |
| `/*` | Shortcut for `LOAD "*",8,1` with C64 EPYX fast load cartridge only |
| `RUN` | Runs the program following `LOAD` above |
| `LOAD "$",8` | Loads a directory listing of the contents of a D64 or T64 file |
| `LIST` | Lists the directory contents after the command above |
| `$` | Shortcut for `LOAD "$",8` with C64 EPYX fast load cartridge only |

> C64 sample list of working games
```
1942, Abbaye Des Morts, Arc of Yesod, Archon 1 and 2, Arkanoid, Armalyte, Barbarian 1 and 2, Beach Head 1 and 2, Biggles, Blue Max, Bomb Jack, Boulder Dash series, Bruce Lee, Bruce Lee Return of Fury, Bubble Bobble, Burnin Rubber, Cauldron, Cliff Hanger, Commando, Dig Dug, Dino Eggs, Donkey Kong Junior, Donkey Kong, Druid, Emlyn Hughes International Soccer, Empire Strikes Back, Fist 2, Fix-it Felix Jr, Football Manager, Fort Apocalypse 2, Galencia, Ghostbusters, Ghosts 'n Goblins, Green Beret, Heli Rescue, Impossible Mission 1 and 2, International Football, International Karate, International Karate Plus, Lady Pac, Last Ninja 1 and 2, Lock 'n' Chase, Lode Runner, Mayhem in Monsterland, Microprose Soccer, Millie and Molly, Moon Cresta, Outrun, Pacman Arcade, Pac-Man, Paperboy, Paradroid, Pitstop 1 and 2, Pooyan, Popeye, Raid Over Moscow, Rambo First Blood Part 2, Rampage, Rick Dangerous, Robin of the Wood, Rodland, R-Type, Spy Hunter, Spy vs Spy series, Super Mario Bros, Super Pipeline 1 and 2, The Great Giana Sisters, The Train Escape to Normandy, Turrican, Uridium, Volfied, Way of the Exploding Fist, Who Dares Wins 1 and 2, Wizard of Wor, Wizball, Yie Ar Kung-Fu
```
> Vic-20 sample list of working games (most with a memory expansion)
```
Adventure Land, AE, Alien Blitz, Amok, Arcadia, Astro Nell, Astroblitz, Atlantis, Attack of the Mutant Camels, Avenger, Bandits, Battlezone, Black Hole, Blitz, Buck Rogers, Capture the Flag, Cheese and Onion, Choplifter, Cosmic Cruncher, Creepy Corridors, Defender, Demon Attack, Donkey Kong, Dragonfire, Escape 2020, Final Orbit, Galaxian, Get More Diamonds, Gridrunner, Help Bodge, Hero, Jelly Monsters, Jetpac, Lala Prologue, Laser Zone, Lode Runner, Manic Miner, Metagalactic Llamas, Mickey the Bricky, Miner 2049er, Mission Impossible, Moon Patrol, Moons of Jupiter, Mosquito Infestation, Mountain King, Ms Pac-Man, Nibbler, Omega Race, Pac-Man, Pentagorat, Perils of Willy, Pharaoh's Curse, Pirate Cove, Polaris, Pool, Pumpkid, Radar Rat Race, Rigel Attack, Robotron, Robots Rumble, Rockman, Rodman, Sargon 2 Chess, Satellite Patrol, Satellites and Meteorites, Scorpion, Seafox, Serpentine, Shamus, Skramble, Skyblazer, Spider City, Spiders of Mars, Squish'em, Star Battle, Star Defence, Super Amok, Sword of Fargoal, Tenebra Macabre, TenTen, Tetris Deluxe, The Count, Traxx, Tutankham, Video Vermin, Voodoo Castle, Zombie Calavera
```

Note that load times in disk drive mode are not fast by modern standards, taking just over a minute for most C64 programs. If the C64 EPYX fast load cartridge is used, loading takes around 4-5 seconds.

As this is not a 'true' disk drive emulator, there are some related downsides and some things which have not been tested.
- Some program files, typically for the C64, do not load because they require features of the actual disk drive hardware
- Has not been tested with C64 fast-loader cartridges other than EPYX fast load
- Has not been fully tested for programs which use two or more D64 files
- Saving programs or handling disk operations e.g. renaming a file is not currently supported
- This project has used a PAL C64 and Vic-20 for testing, so it's uncertain how this might work on NTSC machines

A program title may have many different roms. If having a problem, try other versions instead, especially for a favourite game.

## Hardware Interface
**Important!** Although the connections required are simple, there is potential for variation and errors, so **only proceed if you are content to take on all risks involved**. Some specific points to be aware of:
- Use a 5V 16Mhz Arduino / clone microcontroller, such as an ATmega32U4 device (Micro, Pro-Micro and variations) or ATmega328P device (Uno, Nano). A 3.3V Arduino / clone is likely to be damaged if connected directly the Commodore
- The connections to the Commodore and Arduino are essential to get right. Damage to the Commodore and Arduino may result if this isn't done
- This project involves soldering the pin connections for the Commodore and Arduino. Ensure you're confident to do this and that connections will be secure and will not leave pins touching one another

For connecting the Commodore and Arduino, a 6 pin male DIN connector and 5 wire cable (or jumper wires) are needed, both are cheap and readily available on eBay. Soldering wires onto the DIN is a bit tricky, so it's worth checking YouTube videos for tips. Alternatively, 6 pin DIN cables are available but are not that common.

Any digital pins may be used. The ones you choose are defined in the `settings` tab. The digital pins used in these diagrams provide a working guide. If using the C64 EPYX fast load cartridge, refer to the section below before deciding on which pins to use.

> The Pro-Micro needs a standard micro-USB to USB cable to connect to your computer
![Pro-Micro to 6 pin male din](./docs/pro-micro-to-6-pin-male-din.png)

> The Pro-Micro USB Beetle needs a USB female to USB male cable to connect to your computer
![USB Beetle to 6 pin male din](./docs/pro-micro-usb-beetle-to-6-pin-male-din.png)

> The Uno needs a USB B-type male to USB male cable to connect to your computer
![Uno to 6 pin male din](./docs/arduino-uno-to-6-pin-male-din.png)

Having a reset button is useful for restarting the Arduino which is occasionally needed if a program fails in loading. The Arduino Micro and Uno already have one but many Arduino clones do not. Making one is easy, a momentary button needs to be connected between the reset pin (usually labelled RES or RST) and ground pin (GND). Alternatively, the Arduino needs to be re-plugged in to reset it.

> A completed, working example using a Pro-Micro USB Beetle with a reset button mounted in a Lego case
![Completed USB Beetle example in Lego case](./docs/pro-micro-usb-beetle-with-case.png)

## C64 EPYX fast load cartridge installation steps
The C64 EPYX fast load cartridge requires cycle-exact timing to work. For this, AVR assembler code is needed which may need minor amendment for the choice of input/output pins used on the Arduino.

### Arduino board pin to AVR chip pin mappings
Arduino board pins are mapped to pins on the AVR chip. The AVR assembler code requires the AVR-chip pin mappings not the Arduino board pins, so a slight code change is needed to accommodate this.

The board pin to AVR-chip pin mappings are found in files created when the Arduino IDE is installed, examples below for Windows Arduino IDE v2.3.2.

`Leonardo / Pro-Micro`: C:\Users\xxxx\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.8.6\variants\leonardo\pins_arduino.h

`Standard / Uno`: C:\Users\xxxx\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.8.6\variants\standard\pins_arduino.h

In the `~\leonardo\pins_arduino.h` file for instance, digital pin 9 (D9) maps to AVR pin 5 on port B (PB5). The AVR pin input and output port names are also needed (PINB and PORTB for PB5).

### Determine the AVR-chip mapping for the Arduino pins being used
The AVR-chip pin and port mapping depend on the choice of Arduino board pins. They do not depend on the type of Arduino, so although an Uno and Pro-Micro are mentioned in the examples below, it is just the Arduino board pin choice that matters.

The simplest scenario is where all board pins share the same AVR-chip input and output port names, such as the example below used for the Arduino Uno in this case.

| Pin | Description |
| ------- | ----------- |
| `Atn` | board pin 2 (D2) is PD2. The PD pins have an input port name PIND, output PORTD, mode (not used here) DDRD |
| `Clock` | board pin 3 (D3) is PD3 |
| `Data` | board pin 4 (D4) is PD4 |
| `Reset` | board pin 5 (D5) is PD5 |

A split port name example is below, used in this case for the Pro-Micro / Leonardo.

| Pin | Description |
| ------- | ----------- |
| `Atn` | board pin 9 (D9) is PB5. The PB pins have an input port name PINB, output PORTB, mode (not used here) DDRB |
| `Clock` | board pin 18 (D20) is PF7. The PF pins have an input port name PINF, output PORTF, mode (not used here) DDRF |
| `Data` | board pin 19 (D20) is PF6 |
| `Reset` | board pin 20 (D20) is PF5 |

Note. Data and clock pins must be on the same input/output port. If they are not, different board pins should be chosen.

### Apply the AVR chip mappings to the epyxfastload files
From the above, find the scenario which best matches your pin selection, create a sketch subfolder and copy existing `epyxfastload.h` and `epyxfastload.cpp` files into it. The `epyxfastload.h` and `epyxfastload.cpp` will come from the `uno` subfolder (example where all pins share the same AVR chip input and output port names) or `promicro` subfolder (example where port names are split).

Amend the `epyxfastload.h` with the pin choices. The `epyxfastload.cpp` may need amendment to change the port labels (to avoid confusion), for example IEC_INPUT_B, IEC_OUTPUT_B may become IEC_INPUT_D, IEC_OUTPUT_D where port D is being used instead of B. A global change/replace in `epyxfastload.h` and `epyxfastload.cpp` will easily do this.

Copy both files into the main sketch folder and compile the sketch.

## Software Notes
- To view the macros, enable the Developer tab via `File > Options > Customize Ribbon`, and select the `Developer` tab under the `All Tabs` dropdown

- From the `Developer tab`, choose `Visual Basic`, to view the modules and classes as shown below

    ![vba project macros](./docs/vba-project-macros.png)

| Module | Description |
| ------ | ----------- |
| `c64`, `vic20` | Code for these sheets to handle connect / disconnect, program selection and viewing box art |
| `frmBoxArtView` | Simple form for displaying box art |
| `PROGRAM_LOADER` | Contains the main program and loop for getting messages from the Arduino and sending responses back. Includes handshake with Arduino and handles open program, close, read packet of data, read directory listing |
| `SERIAL_PORT_VBA` | Contains the code to connect and communicate with the Arduino. Works like an imported library, see the Acknowledgement section for details |
| `UTILS` | Code for handling display of box art images |
| `D64_DRIVER` | Class for reading D64 program files and returning data to the program loader |
| `PRG_DRIVER` | As above for PRG program files |
| `T64_DRIVER` | As above for T64 program files |

- The VBA project references are standard except for the addition of the `VBScript Regular Expressions` library

    ![vba project references](./docs/vba-project-references.png)

- The Arduino sketch consists of a number of files. The main ones are briefly described below.

| File | Description |
| ---- | ----------- |
| `commodore_sketch.ino` | Main sketch which deals with the PC handshake and calls the disk interface handlers |
| `interface.cpp`, `interface.h` | Handles the communication events between the PC and the Commodore IEC disk interface |
| `iec_driver.cpp`, `iec_driver.h` | Provides the disk interface to the Commodore handling the Atn, Clock, Data, Reset signals |

## Authors and Acknowledgement
The information and code shared by the following developers and sources is gratefully acknowledged:
- [New 1541 emulator for arduino via desktop computer: uno2iec - Commodore 64 (C64) Forum (lemon64.com)](https://www.lemon64.com/forum/viewtopic.php?t=48771&start=0&sid=667319bb48acd56b1d4e0c2296145a84), developer Lars Wadefalk
- [Serial port library for VBA](https://github.com/Serialcomms/Serial-Ports-in-VBA-new-for-2022), developer serialcomms
- [SD2IEC project](https://www.sd2iec.de/), for EPYX fast load cartridge additions
- [How Does Epyx Fastload Make Loading Faster on a Commodore 64?](https://www.youtube.com/watch?v=pUjOLLvnhjE), YouTube video by Commodore History
- [VBA coding standards](https://github.com/spences10/VBA-Coding-Standards), developer Scott Spence

## License
[The software is provided under the terms of its MIT license](./LICENSE.txt)

## Project Status
This project is considered working for disk / serial operations loading Commodore D64, PRG and T64 based programs.
