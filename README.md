#### General Information and Disclaimer
This repository is linked with a scientific publication. The ACM Link, DOI and other information will be added here, after the paper is published. 

Our work is published under GNU GPLv3. However, for some files, e.g., 3D figurines or the published paper, other licenses may apply. 
This repository (or rather the releases of this repository) is mirrored on Zenodo.org. (Link will be added).

If you use or extend our work, please cite us as (bibtex.bib):
Will be added later.

## BrailleBuddy

BrailleBuddy is a tangible user interface supporting children with visual impairments in learning Braille. It was created using a human-centered inclusive design process with interviews, six design iterations with prototypes, and feedback from experts, students, and teachers. Alleviating the need for supervision, the device can be used during breaks in school or at home, providing additional opportunities to experience Braille and individual audio feedback to support the users in learning to read and spell Braille.

![BrailleBuddy](https://github.com/FlorianLa/BrailleBuddy/blob/main/Photos/BrailleBuddy.jpg)

### Case and 3D-printed Cards
We provide a *.svg file for the wooden case of BrailleBuddy in "CaseModel(Lasercutter)". The lid is constructed from two layers of wood. Please be aware, that the files are designed for material with 3mm thickness for each of the layers of the lid and 5mm for the other parts. If you use other materials, you need to adjust the contact areas accordingly.

Further, we provide language specific 3D-models for all cards used. **Right now only the German version is populated, but the models can be used by changing the Braille text beneath the figurine.**
The sources for the 3D figurines are listed next to each card. The 2D relief images were created by [Verena Pues](https://verenapues.com/).

### Supported Languages

**German** - with Braille system "Vollbraille". This version was used in the study.

**English** - with Braille system English Grade 2 Braille **(under construction)** 
For the English version, some points need to be completed, such as audio files and letter cards for abbreviations and English game and word cards.

To adjust the source code for the English version please comment out sections marked with GERMAN VERSION and comment in sections with ENGLISH VERSION

### Hardware

![Hardware](https://github.com/FlorianLa/BrailleBuddy/blob/main/Photos/BrailleBuddy_Electronics.jpg)

* **Microcontroller:** The brain of BrailleBuddy is an ESP32 microcontroller board that is easy to program, supports a variety of protocols, and has 27 usable General Purpose Input/Output (GPIO) pins. The board also has Wifi and Bluetooth functionality, which offers numerous possibilities for further development of BraillyBuddy. For other microcontrollers please check if there are enough GPIO pins and adjust the pin layout.
* **NFC Tags:** To store digital information on our game cards, we used NFC Stickers (12 x 19 mm - NTAG213 - 180 Byte - transparent).
* **RFID Readers:** To detect game cards and read and write the digital information, we used 5 x Mifare RFID-RC522 Modules, which communicate with the microcontroller via the SPI interface.
* **Photoresistors:** While the readers are responsible for receiving the information from the NFC tags, we used 7 x light dependent photoresistors to determine the corresponding position for each tag on the box as most of the readers cover two slots. 
* **Multiplexer:** We used a 16 channel digital/analog multiplexer to handle the various needed connections, i.e., to read out the analog signals from the photoresistors and distribute the RFID reader's reset line (RST).
* **MP3 Module and Loudspeaker:** For the auditory feedback, we used a DFPlayer-Mini – a compact serial MP3 module with a built-in amplifier and an SD card slot to store MP3 files – and a small 3 Watt 8 Ohm
loudspeaker.
* **Buttons:** To allow users to repeat the slot-related audio feedback, we used 8 x push buttons (12 x 12 x 7.3 mm).
* **Resistors:** To find out which and how many resistors you need, please consult the respective data sheets of your modules.

### Improvements
We describe various possible improvements for BrailleBuddy in our paper. The most relevant possibilities are:
* Add a headphone connector and an interface to control the volume.
* Instead of reading the word, BrailleBuddy could also make corresponding sounds, e.g., a car or animal sounds.
* Consider differently shaped cards to support users that find correctly rotating and placing the cards challenging.
* Use separate storage areas for the letter cards or reduce the selection of cards presented to support children just starting to learn Braille.
* The audio feedback can be improved by reacting to a button being pressed consecutively multiple times, long times in between cards placed, and multiple incorrectly placed cards. BrailleBuddy could give more diverse responses, e.g., "You placed the correct letter but in the wrong spot." or give hints towards the games solution.


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


### Audio Files

We used the following free text-to-speech converter to create most audio files: https://ttsmp3.com/.

The sources of the sound effects used are:
* wrong answer: https://pixabay.com/de/sound-effects/wrong-answer-126515/
* correct answer: https://pixabay.com/de/sound-effects/success-1-6297/
* evaluation drums: https://pixabay.com/de/sound-effects/tadaa-47995/
* dice rolling for word scramble game intro: https://pixabay.com/de/sound-effects/dice-cup-60668/