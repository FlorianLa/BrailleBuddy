/**
 * This sample shows how to write the needed game informations on a 
 * MIFARE Ultralight tag that are used for the game and letter cards of BrailleBuddy:
 * https://github.com/FlorianLa/BrailleBuddy
 * --------------------------------------------------------------------------------------------------
 * This sketch is based on a MFRC522 library example: 
 * https://github.com/miguelbalboa/rfid/blob/master/examples/ReadAndWrite/ReadAndWrite.ino
 * See https://github.com/miguelbalboa/rfid for further details and other examples.
 * --------------------------------------------------------------------------------------------------
 * 
 * Pin layout used:
 * --------------------------------------------------------------------------------------------------
 *             MFRC522             
 *             Reader/PCD   ESP32
 * Signal      Pin          Pin             Note
 * --------------------------------------------------------------------------------------------------
 * RST         RST          15              reset pin is used to deactivate a reader, can be any GPIO
 * SPI SS      SDA(SS)      22              select pin for readers, can be any GPIO           
 * SPI MOSI    MOSI         23  
 * SPI MISO    MISO         19  
 * SPI SCK     SCK          18 
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 *
 */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         22          // Configurable, see pin layout above
#define SS_PIN          5           // Configurable, see pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

#define NR_OF_ROWS      8           // number of rows we want to write on the tag
#define NR_OR_COLUMNS   4           // a row on our Ultralight tag has the length of 4 bytes
 
void setup() {
  // the code here is run once on setup
  Serial.begin(19200);              // Initialize serial communications with the PC
  while (!Serial);                  // Do nothing if no serial port is opened
  SPI.begin();                      // Init SPI bus
  mfrc522.PCD_Init();               // Init MFRC522 reader
  delay(1000);                      // wait to make sure the reader is ready
  Serial.println("Write Braille Char on a MIFARE PICC");
}

void loop() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if(newTagPresent()){

    //The buffer is the information that should be written on the tag.

    /*The first row stores information about the game mode, audio (track, folder), and occurance
      - game mode: a number from 1 - 3
        only set this number on game mode cards! 
        Lets the BrailleBuddy Box know, which game the player wants to play
        1: Copy Words
        2: Word Scramble
        3: Word Riddle
      - audio: 
          track: index of the audio file inside the folder
          folder: index of the folder sored od the sd card
          (should match your folder/file setup for the MP3 Module's Sd Card)
      - occurance: only needed for letter / character cards
        as there are multiple characters that occure on more than one card
        (e.g. for banana we need 3 a's) each of the letters has it's own occurance id.
        This is needed to check whether a new card was placed on the BrailleBuddy box 
    */

    /*The seven following rows store the character information (up to 7 braille characters).
      Braille characters with more than one letter, e.g. "sh" are stored in one row.
    */

    // Adjust the buffer to match the character or word you want to store on the tag
    // Buffer Example for a single braille character 'd'
    byte buffer[NR_OF_ROWS][NR_OR_COLUMNS] = {
      {0, 29, 2, 1}, // game mode, track, folder, occurance
      {0, 0, 0, 'd'},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    };

    // Buffer Example for the word "Ameise" / "ant"
    /*byte buffer[NR_OF_ROWS][NR_OR_COLUMNS] = {
      {1, 16, 6, 0}, // game mode, track, folder, occurance
      {0, 0, 0, 'A'},
      {0, 0, 0, 'm'},
      {0, 0, 'e', 'i'},
      {0, 0, 0, 's'},
      {0, 0, 0, 'e'},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    };*/
    /*byte buffer[NR_OF_ROWS][NR_OR_COLUMNS] = {
      {1, 0, 6, 0}, // game mode, track, folder, occurance
      {0, 0, 0, 'a'},
      {0, 0, 0, 'n'},
      {0, 0, 0, 't'},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    };*/

    byte page = 6;
    MFRC522::StatusCode status;
    byte len = NR_OR_COLUMNS;

    // Write block
    for(int i = 0; i < NR_OF_ROWS; i++) {
      status = mfrc522[0].MIFARE_Ultralight_Write(page, buffer[i], NR_OR_COLUMNS);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522[0].GetStatusCodeName(status));
        return;
      }
      else Serial.println(F("MIFARE_Write() success: "));
      page += 1;
    }

  }
}

bool newTagPresent() {
  return mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();
}
