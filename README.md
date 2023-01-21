## BrailleBuddy

BrailleBuddy is a tangible user interface supporting children with visual impairments in learning Braille. It was created using a human-centered inclusive design process with interviews, six design iterations with prototypes, and feedback from experts, students, and teachers. Alleviating the need for supervision, the device could be used during breaks in school or at home, providing additional opportunities to experience Braille and individual audio feedback to support the users learning.

![BrailleBuddy](https://github.com/FlorianLa/BrailleBuddy/blob/main/Photos/BrailleBuddy.jpg)

### Supported Languages

**German** - with Braille System "Vollbraille", 

**English** - with Braille System English Grade 2 Braille **(under construction)** 
TODOS: Audiofiles for abbreviations of the Braille System, Letter Cards for the abbreviations, English Word Cards, English Game Cards
To adjust the source code for the english version please comment out sections marked with GERMAN VERSION and comment in sections with ENGLISH VERSION

### Hardware

* **Microcontroller:** The brain of BrailleBuddy is an ESP32 microcontroller board that is easy to program, supports a variety of protocols, and has 27 usable General Purpose Input/Output (GPIO) pins. The board also has Wifi and Bluetooth functionality, which offers numerous possibilities for further development of BraillyBuddy. For other microcontrollers please check if there are enough GPIO pins and adjust the pin layout.
* **NFC Tags:** To store digital information on our game cards we used NFC Stickers (12 x 19 mm - NTAG213 - 180 Byte - transparent).
* **RFID Readers:** To detect game cards and read and write the digital information we used 5 x Mifare RFID-RC522 Modules, which communicate with the microcontroller via the SPI interface.
* **Photoresistors:** While the readers are responsible for receiving the information from the NFC tags, we used 7 x light dependent photoresistors to determine the corresponding position for each tag on the box as most of the readers are responsible for two slots. 
* **Multiplexer:** To handle the various needed connections we used a 16 channel digital/analog multiplexer to read out the analog signals from the photoresistors and distribute the RFID reader's reset line (RST).
* **MP3 Module and Loudspeaker:** For the auditory feedback, we used a DFPlayer-Mini – a compact serial MP3 module with a built-in amplifier and an SD card slot to store MP3 files – and a small 3 Watt 8 Ohm
loudspeaker.
* **Buttons:** To allow users to repeat the slot-related audio feedback, we used 8 x push buttons (12 x 12 x 7.3 mm).
* **Resistors:** To find out which and how many resistors you need, please consult the respective data sheets of your modules.

![Hardware](https://github.com/FlorianLa/BrailleBuddy/blob/main/Photos/BrailleBuddy_Electronics.jpg)


### Pin Layout

| **ESP32 Pin** | **Note**                              | **Multiplexer** | **MP3 Module** | **RFID Reader** | **Button**     |
|---------------|---------------------------------------|-----------------|----------------|-----------------|----------------|
| GPIO 0        |                                       | S1              |                |                 |                |
| GPIO 1        | TX                                    |                 | TX             |                 |                |
| GPIO 2        |                                       | S0              |                |                 |                |
| GPIO 3        | RX                                    |                 | RX             |                 |                |
| GPIO 4        |                                       | S2              |                |                 |                |
| GPIO 5        |                                       |                 |                | No. 3, SDA      |                |
| GPIO 6        | SPI bus for flash memory, do not use! |                 |                |                 |                |
| GPIO 7        | SPI bus for flash memory, do not use! |                 |                |                 |                |
| GPIO 8        | SPI bus for flash memory, do not use! |                 |                |                 |                |
| GPIO 9        | SPI bus for flash memory, do not use! |                 |                |                 |                |
| GPIO 10       | SPI bus for flash memory, do not use! |                 |                |                 |                |
| GPIO 11       | SPI bus for flash memory, do not use! |                 |                |                 |                |
| GPIO 12       |                                       |                 |                |                 |                |
| GPIO 13       |                                       |                 |                | No. 4, SDA      |                |
| GPIO 14       |                                       |                 |                |                 | No. 8          |
| GPIO 15       | used as Reader RST Pin                | SIG             |                |                 |                |
| GPIO 16       |                                       | S3              |                |                 |                |
| GPIO 17       |                                       |                 |                | No. 5, SDA      |                |
| GPIO 18       | CLK                                   |                 |                | All, SCK        |                |
| GPIO 19       | MISO                                  |                 |                | All, MISO       |                |
| GPIO 21       |                                       |                 |                | No. 2, SDA      |                |
| GPIO 22       |                                       |                 |                | No. 1, SDA      |                |
| GPIO 23       | MOSI                                  |                 |                | All, MOSI       |                |
| GPIO 25       |                                       |                 |                |                 | No. 5          |
| GPIO 26       |                                       |                 |                |                 | No. 6          |
| GPIO 27       |                                       |                 |                |                 | No. 7          |
| GPIO 32       |                                       |                 |                |                 | No. 3          |
| GPIO 33       |                                       |                 |                |                 | No. 4          |
| GPIO 34       | only input                            |                 |                |                 | No. 1          |
| GPIO 35       | only input                            |                 |                |                 | No. 2          |
| GPIO 36       | only input                            |                 |                |                 |                |
| GPIO 39       | only input                            |                 | BUSY           |                 |                |