## BrailleBuddy

BrailleBuddy is a tangible user interface supporting children with visual impairments in learning Braille. It was created using a human-centered inclusive design process with interviews, six design iterations with prototypes, and feedback from experts, students, and teachers. Alleviating the need for supervision, the device could be used during breaks in school or at home, providing additional opportunities to experience Braille and individual audio feedback to support the users learning.

![BrailleBuddy](https://github.com/FlorianLa/BrailleBuddy/blob/main/Photos/BrailleBuddy.jpg)

### Hardware

* **Microcontroller:** The brain of BrailleBuddy is an ESP32 microcontroller board that is easy to program, supports a variety of protocols, and has 27 usable General Purpose Input/Output (GPIO) pins. The board also has Wifi and Bluetooth functionality, which offers numerous possibilities for further development of BraillyBuddy. For other microcontrollers please check if there are enough GPIO pins and adjust the pin layout.
* **NFC Tags:** To store digital information on our game cards we used NFC Stickers (12 x 19 mm - NTAG213 - 180 Byte - transparent).
* **RFID Readers:** To detect game cards and read and write the digital information we used 5 x Mifare RFID-RC522 Modules, which communicate with the microcontroller via the SPI interface.
* **Photoresistors:** While the readers are responsible for receiving the information from the NFC tags, we used 7 x light dependent photoresistors to determine the corresponding position for each tag on the box as most of the readers are responsible for two slots. 
* **Multiplexer:** To handle the various needed connections we used a 16 channel digital/analog multiplexer to read out the analog signals from the photoresistors and distribute the RFID reader's reset line (RST).
* **MP3 Module and Loudspeaker:** For the auditory feedback, we used a DFPlayer-Mini – a compact serial MP3 module with a built-in amplifier and an SD card slot to store MP3 files – and a small 3 Watt 8 Ohm
loudspeaker.
* **Buttons:** To allow users to repeat the slot-related audio feedback, we used 8 x push buttons (12 x 12 x 7.3 mm).
* **Resistors:** Resistors: To find out which and how many resistors you need, please consult the respective data sheet of your module.

![Hardware](https://github.com/FlorianLa/BrailleBuddy/blob/main/Photos/BrailleBuddy_Electronics.jpg)


### Pin Layout

+------------+---------------+---------------+-------------+---------------+---------------+
| ESP32 Pins | Note          | Multiplexer   | MP3 Module  | RFID-Reader   | Button        |
+============+===============+===============+=============+===============+===============+
| + 3,3V     |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| + 5V       |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GND        |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 0     |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 1     |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 2     |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 3     |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 4     |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 5     |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 6     |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 7     |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 8     |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 9     |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 10    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 11    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 12    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 13    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 14    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 15    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 16    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 17    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 18    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 19    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 20    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 21    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 22    |               |               |             | No. 1 | SDA   |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 23    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 24    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 25    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 26    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 27    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 28    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 29    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 30    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 31    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 32    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 33    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 34    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 35    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 36    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 37    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 38    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+
| GPIO 39    |               |               |             |       |       |       |       |
+------------+---------------+---------------+-------------+---------------+---------------+