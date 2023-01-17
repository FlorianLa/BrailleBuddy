#include <SPI.h>
#include <MFRC522.h>
#include "DFRobotDFPlayerMini.h"

//defines the number of letter/char slots
#define NR_OF_CHAR_POS 7
//defines the number of RFID readers
#define NR_OF_READERS 5
//defines the audio queue size
#define AUDIO_QUEUE_SIZE 10
//defines the max. audio feeback size/
// which is the max. number of audio files combined together for the feedback
#define AUDIO_FEEDBACK_SIZE 9

//====================== Enums for GAME MODES =====================================
enum GameModes {
  KEIN_SPIEL,
  WORTE_NACHLEGEN,
  PURZELWOERTER,
  WORTRAETSEl
};

enum GameSubModes {
  NO_SUB,
  WORTE_NACHLEGEN_BEENDET,
  PURZELWOERTER_BUCHSTABEN_ABLEGEN,
  PURZELWOERTER_BUCHSTABEN_SORTIEREN,
  PURZELWOERTER_BEENDET,
  WORTRAETSEl_BEENDET
};

//====================== Custom Type Definitions ===================================

// The Audio files need a folder and a track number to define where to find them on the SD card
typedef struct {
  int folder;
  int track;
} Audio;

// Each braille char has a matching audio file, a string representation (i.e. "a")
// and a occurance to distinguish same braille chars (a char can appear multiple times in a word)
typedef struct {
  Audio audio;
  int occurance;
  String value;
} BrailleChar;

//Each word has a matching audio file and a number of brailleChars it consists of
typedef struct {
  Audio audio;
  BrailleChar brailleChars[7];
} BrailleWord;

//Each NFC tag has a matching audio file, a possible occurance (if its a letter tag), 
// a possible game mode (if its a game mode tag), and string information (i.e. "Sch", "a", "f" for a word tag)
typedef struct {
  Audio audio;
  int occurance;
  GameModes gameMode;
  String values[7];
} Tag;

//=================================== Audio Feedback =====================================

HardwareSerial mySoftwareSerial(1);
DFRobotDFPlayerMini myDFPlayer;

// audioQueue stores the order in which the files should be played
Audio audioQueue[AUDIO_QUEUE_SIZE] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}};
// stores the audio feedback sequence for a letter
Audio charAudioFeedback[NR_OF_CHAR_POS][AUDIO_FEEDBACK_SIZE];
// is used to overwrite the letter's audio feedback when a letter tag is removed
Audio audioFeedbackLeeresFeld[AUDIO_FEEDBACK_SIZE] = {{1, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};

// is used to stop the program loop while audio feedback for the switch of a game state is being played
// as this must not be skipped to make sure the user knows about the game mode switch
boolean eval = false;

// the busy pin is used to check whether the mp3 player is actually playing
byte mp3PlayerBusyPin = 39;
// tells whether the mp3Player is currently busy with playing audio
boolean mp3PlayerBusy = false;
// represents the current busy value (either HIGH or LOW)
byte currentBusyValue;
// is used to check whether the busy value has changed
byte prevBusyValue;

//=================================== RFID Readers =====================================

MFRC522 mfrc522[NR_OF_READERS]; // initialize the RFID reader instances 

// tells which reader is currently active (as only one reader is active at a time to prevent interference)
int currentlyActiveReader = -1;

// slave select pins of readers, is used to define with which reader we want to communicate
byte ssReaderPins[NR_OF_READERS] = {22, 21, 5, 13, 17};
// reset pin is shared by all readers and is used to deactivate a reader (see also: multiplexer)
byte rstReaderPin = 15;
// defines which reader is responsible for which letter position on the box
// i.e. reader with index 4 is responsible to read values for position 6 and 7
byte readerForPos[NR_OF_CHAR_POS] = {1, 2, 2, 3, 3, 4, 4};

// the denominator used to divide a word into braille chars on the NFC tags
char denom = '-';

//=================================== Photoresistors =====================================

// holds information about the current light situation at each letter slot
int fotovalues[NR_OF_CHAR_POS] = {0, 0, 0, 0, 0, 0, 0};
// stores the initial light situation read at each letter slot at the initial setup
// to decide whether the light situation has changed
int initialFotovalues[NR_OF_CHAR_POS] = {0, 0, 0, 0, 0, 0, 0};

//=================================== Multiplexer ========================================

// pins used to switch between multiplexed channels
byte multiplexerPins[4] = {2, 0, 4, 16};
// pin used to read or write to the selected channel
byte multiplexerSignalPin = 15;

// the photoresistors are being multiplexed
int mpFotoSelection[NR_OF_CHAR_POS][4] = {
  {0, 1, 1, 0}, {1, 0, 1, 0}, {0, 0, 1, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}
};
// the multiplexers reset inputs are being multiplexed
int rstSelection[NR_OF_READERS][4] = {
  {1, 1, 0, 1}, {0, 1, 0, 1}, {1, 0, 0, 1}, {0, 0, 0, 1}, {1, 1, 1, 0}
};


//=================================== Buttons ========================================

byte btnPins[8] = {34, 35, 32, 33, 25, 26, 27, 14};
byte btnStates[8] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
byte btnStatesPrev[8] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};

//=============================== Game Information ===================================

// stores current game mode 
GameModes currentGameMode = KEIN_SPIEL;
// is used to check for a game mode change
GameModes prevGameMode = KEIN_SPIEL;


// stores current sub game mode 
GameSubModes currentGameSubMode = NO_SUB;
// is used to check for a sub game mode change
GameSubModes prevGameSubMode = NO_SUB;

// represents the current letters on the BrailleBox
BrailleChar currentCharState[NR_OF_CHAR_POS] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}};
// stores the goal char state for the current (sub) game mode
BrailleChar wantedCharState[NR_OF_CHAR_POS] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}};
// holds information such as audio file etc. for the current word solution
BrailleWord wordTemplate = {0};

// if no game mode card is read from the first reader the empty word counter gets counted up to empty threshold
// to make sure the slot is really empty and it wasn't just a reader fail
int emptyWordCounter = 0;
int emptyThreshold = 3;


//================ Word Collection for Purzelwörter (Word Scramble) and Worträtsel (Word Riddle) ============================
/* In the future this information should be expanded to a full primary school dictionary and be stored on a extra SD Card */

// collection of all needed braille char information
BrailleChar aGross = {2, 0, 1, "A"};
BrailleChar bGross = {2, 1, 1, "B"};
BrailleChar cGross = {2, 2, 1, "C"};
BrailleChar dGross = {2, 3, 1, "D"};
BrailleChar eGross = {2, 4, 1, "E"};
BrailleChar fGross = {2, 5, 1, "F"};
BrailleChar gGross = {2, 6, 1, "G"};
BrailleChar hGross = {2, 7, 1, "H"};
BrailleChar iGross = {2, 8, 1, "I"};
BrailleChar jGross = {2, 9, 1, "J"};
BrailleChar kGross = {2, 10, 1, "K"};
BrailleChar lGross = {2, 11, 1, "L"};
BrailleChar mGross = {2, 12, 1, "M"};
BrailleChar nGross = {2, 13, 1, "N"};
BrailleChar oGross = {2, 14, 1, "O"};
BrailleChar pGross = {2, 15, 1, "P"};
BrailleChar qGross = {2, 16, 1, "Q"};
BrailleChar rGross = {2, 17, 1, "R"};
BrailleChar sGross = {2, 18, 1, "S"};
BrailleChar tGross = {2, 19, 1, "T"};
BrailleChar uGross = {2, 20, 1, "U"};
BrailleChar vGross = {2, 21, 1, "V"};
BrailleChar wGross = {2, 22, 1, "W"};
BrailleChar xGross = {2, 23, 1, "X"};
BrailleChar yGross = {2, 24, 1, "Y"};
BrailleChar zGross = {2, 25, 1, "Z"};
BrailleChar aeGross = {2, 26, 1, "Ae"};
BrailleChar oeGross = {2, 27, 1, "Oe"};
BrailleChar ueGross = {2, 28, 1, "Ue"};
BrailleChar auGross = {2, 29, 1, "Au"};
BrailleChar chGross = {2, 30, 1, "Ch"};
BrailleChar eiGross = {2, 31, 1, "Ei"};
BrailleChar euGross = {2, 32, 1, "Eu"};
BrailleChar ieGross = {2, 33, 1, "Ie"};
BrailleChar schGross = {2, 34, 1, "Sch"};
BrailleChar stGross = {2, 35, 1, "St"};
BrailleChar aeuGross = {2, 36, 1, "Aeu"};

BrailleChar aKlein = {2, 37, 1, "a"};
BrailleChar bKlein = {2, 38, 1, "b"};
BrailleChar cKlein = {2, 39, 1, "c"};
BrailleChar dKlein = {2, 40, 1, "d"};
BrailleChar eKlein = {2, 41, 1, "e"};
BrailleChar fKlein = {2, 42, 1, "f"};
BrailleChar gKlein = {2, 43, 1, "g"};
BrailleChar hKlein = {2, 44, 1, "h"};
BrailleChar iKlein = {2, 45, 1, "i"};
BrailleChar jKlein = {2, 46, 1, "j"};
BrailleChar kKlein = {2, 47, 1, "k"};
BrailleChar lKlein = {2, 48, 1, "l"};
BrailleChar mKlein = {2, 49, 1, "m"};
BrailleChar nKlein = {2, 50, 1, "n"};
BrailleChar oKlein = {2, 51, 1, "o"};
BrailleChar pKlein = {2, 52, 1, "p"};
BrailleChar qKlein = {2, 53, 1, "q"};
BrailleChar rKlein = {2, 54, 1, "r"};
BrailleChar sKlein = {2, 55, 1, "s"};
BrailleChar tKlein = {2, 56, 1, "t"};
BrailleChar uKlein = {2, 57, 1, "u"};
BrailleChar vKlein = {2, 58, 1, "v"};
BrailleChar wKlein = {2, 59, 1, "w"};
BrailleChar xKlein = {2, 60, 1, "x"};
BrailleChar yKlein = {2, 61, 1, "y"};
BrailleChar zKlein = {2, 62, 1, "z"};
BrailleChar aeKlein = {2, 63, 1, "ae"};
BrailleChar oeKlein = {2, 64, 1, "oe"};
BrailleChar ueKlein = {2, 65, 1, "ue"};
BrailleChar auKlein = {2, 66, 1, "au"};
BrailleChar chKlein = {2, 67, 1, "ch"};
BrailleChar eiKlein = {2, 68, 1, "ei"};
BrailleChar euKlein = {2, 69, 1, "eu"};
BrailleChar ieKlein = {2, 70, 1, "ie"};
BrailleChar schKlein = {2, 71, 1, "sch"};
BrailleChar stKlein = {2, 72, 1, "st"};
BrailleChar aeuKlein = {2, 73, 1, "aeu"};
BrailleChar ssKlein = {2, 74, 1, "ss"};

// collection of all needed word information
BrailleWord ameise = { 6, 16, {aGross, mKlein, eiKlein, sKlein, eKlein}};
BrailleWord monat = { 9, 32, {mGross, oKlein, nKlein, aKlein, tKlein}};
BrailleWord muesli = { 9, 60, {mGross, ueKlein, sKlein, lKlein, iKlein}};
BrailleWord pferd = { 9, 171, {pGross, fKlein, eKlein, rKlein, dKlein}};
BrailleWord regen = { 10, 12, {rGross, eKlein, gKlein, eKlein, nKlein}};
BrailleWord sonne = { 10, 236, {sGross, oKlein, nKlein, nKlein, eKlein}};
BrailleWord tafel = { 11, 111, {tGross, aKlein, fKlein, eKlein, lKlein}};
BrailleWord topf = { 11, 172, {tGross, oKlein, pKlein, fKlein}};
BrailleWord traum = { 11, 182, {tGross, rKlein, auKlein, mKlein}};
BrailleWord vogel = { 12, 6, {vGross, oKlein, gKlein, eKlein, lKlein}};
BrailleWord wolke = { 12, 123, {wGross, oKlein, lKlein, kKlein, eKlein}};
BrailleWord zwerg = { 12, 217, {zGross, wKlein, eKlein, rKlein, gKlein}};

// collection of words the system can choose from
BrailleWord woerter[11] = {traum, ameise, vogel, zwerg, topf, regen, monat, wolke, sonne, pferd, tafel};

// stores which words were already used, to prevent them from being chosen twice
BrailleWord wordHistory[10] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}};


//============================================== SETUP ===================================================================

void setup() {
  //start serial communication for mp3 player
  mySoftwareSerial.begin(9600, SERIAL_8N1, 1, 3);
  while (!mySoftwareSerial);

  Serial.begin(115200);
  // Do nothing if no serial port is opened
  while (!Serial);

  // Init SPI bus for RFID Readers
  SPI.begin();
  delay(50); // make sure it's ready

  // set multiplexer pins to output
  for (int i = 0; i < 4; i++) pinMode(multiplexerPins[i], OUTPUT);
  // set button pins to input
  for (int i = 0; i < 8; i++) pinMode(btnPins[i], INPUT);
  // set mp3 player's busy pin to input
  pinMode(mp3PlayerBusyPin, INPUT);
  // initialize audio feed back for empty slots
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    for (int j = 0; j < AUDIO_FEEDBACK_SIZE; j++) {
      charAudioFeedback[i][j] = audioFeedbackLeeresFeld[j];
    }
  };

  // initialize MP3 Player
  initDFPPlayer();
  currentBusyValue = digitalRead(mp3PlayerBusyPin);
  prevBusyValue = currentBusyValue;

  // initialize RFID Readers
  initReaders();

  // read initial values from photoresistors
  setInitialFotovalues();
  delay(50);

  Serial.println(F("end of setup"));
  // Für Studie Intro auskommentiert.
  /*Audio hallo = {1, 0};
  addToAudioQueue(hallo);*/
}


//============================================== PROGRAM LOOP ===================================================================

void loop() {
  if (!eval) { // if no game mode switch with important audio information is happening
    readFotoValues(); // then update the photovalues
    checkGameMode(); // check current game mode
    checkBrailleCharsState(); // check current letter state
    setGameSubMode(); // set sub game mode depending on current game mode
    handleGameSubModeChange(); // check for a possible sub mode change and handle it
    checkButtons(); // look for button input
    //debug();
  }
  playAudio(); 
}

// ======================== methods for general game handling ===========================================

void checkGameMode() {
  
  // start reader at first position (game mode position)
  int reader = 0;
  initReader(reader);

  // check if a game mode card is present
  if (newTagPresent(reader)) {
    // reset empty word counter
    emptyWordCounter = 0;
    Tag currentTag = readTag(reader);
    // update game mode 
    prevGameMode = currentGameMode;
    currentGameMode = currentTag.gameMode;
    // if game mode has changed or a new word for the Worte Nachlegen / Word Copy game is present, start new game
    if (currentGameMode != prevGameMode || (currentGameMode == WORTE_NACHLEGEN && isNewWord(currentTag))) {
      initGame(currentTag);
    }
  } else {
    // if no game mode card is present, empty the word template
    emptyWordTemplate();
  }
  
  // stop communication
  mfrc522[reader].PICC_HaltA();
  mfrc522[reader].PCD_StopCrypto1();
  
}

void checkBrailleCharsState() {

  // for all letter positions...
  for (int i = 0; i < NR_OF_CHAR_POS ; i++) {

    // pick the responsible RFID reader
    int reader = readerForPos[i];
    initReader(reader);

    // as there are possibly two tags in the range of the reader, we have to loop over both of them to decide which one is valid for the current position
    for (int j = 0; j < 2; j++) {
      if (isCharPosEmpty(i)) { // if no letter is already stored for that position
        if (isPosCovered(i) && newTagPresent(reader)) { // check if a new tag is present for that reader (as mentioned above we have to check twice)
          BrailleChar bChar = readBrailleChar(reader); // convert tag information to braille char representation
          boolean isNew = isNewBrailleChar(bChar); // check if the letter was newly added to the box or was already recognized
          
          if (isNew) { // is it is a new letter...
            currentCharState[i] = bChar; // update char state
            
            // log event information for study
            Serial.print("neuer Buchstabe wurde abgelegt: "); Serial.print(bChar.value); Serial.print(" Position: "); Serial.println(i);
            debug();
            printCurrentWordTemplate();
            printWantedCharState();
            printCurrentCharState();
            
            // set audio feedback:...
            // ...letter sound, ...
            charAudioFeedback[i][0] = currentCharState[i].audio;
            // ...additional hints ...
            for (int a = 1; a < AUDIO_FEEDBACK_SIZE; a++) {
              charAudioFeedback[i][a] = calcCharAdditionalAudioFeedback(i, a);
            }
            //... feedback sound effect (correct or incorrect)
            charAudioFeedback[i][AUDIO_FEEDBACK_SIZE - 1] = calcCharAudioFeedback(i);
            
            // overwrite audio queue
            forceUpdateAudioQueue(charAudioFeedback[i][0]);
            for (int a = 1; a < AUDIO_FEEDBACK_SIZE; a++) {
              addToAudioQueue(charAudioFeedback[i][a]);
            }
            // play audio
            playAudio();
          }
        }

        // stop communication with that tag
        mfrc522[reader].PICC_HaltA();
        mfrc522[reader].PCD_StopCrypto1();
      }
    }

    // if there is no card recognized by the photoresistors
    if (!isPosCovered(i)) {
      
      BrailleChar bChar = currentCharState[i];
      // empty the char state position
      currentCharState[i] = {0};
      // update audio feedback to "leeres feld" / empty slot
      charAudioFeedback[i][0] = audioFeedbackLeeresFeld[0];
      // empty other audio feedback
      for (int a = 1; a < AUDIO_FEEDBACK_SIZE; a++) {
        charAudioFeedback[i][a] = {0};
      }
      if (bChar.occurance != 0) { // if pos wasn't already empty in a prior check
        // log event information for study
        Serial.print("Buchstabe wurde entfernt: "); Serial.print(bChar.value); Serial.print(" Position: "); Serial.println(i);
        printCurrentWordTemplate();
        printWantedCharState();
        printCurrentCharState();
      }
    }
  }
}

//returns the soundeffect for either a positive or negative feedback
Audio calcCharAudioFeedback(int pos) {
  if (currentGameMode == KEIN_SPIEL) {
    return {0, 0}; // no sfx
  } else {
    if (currentCharState[pos].audio.folder != 0 && currentStateMatchesWantedState(pos)) {
      return {1, 2}; // correct sfx
    } else if (currentCharState[pos].audio.folder != 0 && !currentStateMatchesWantedState(pos)) {
      return {1, 3}; // wrong sfx
    }
  }
  return {0, 0}; // no sfx
}

//returns the spoken text, that adds to the feedback sound effect
Audio calcCharAdditionalAudioFeedback(int pos, int feedbackCounter) {
  if (currentGameMode == KEIN_SPIEL) {
    return {0, 0};
  } else if (currentGameMode == WORTE_NACHLEGEN && feedbackCounter == 1) {
    return worteNachlegen_calcCharAdditionalAudioFeedback(pos);
  } else if (currentGameMode == PURZELWOERTER && feedbackCounter == 1) {
    return purzelwoerter_calcCharAdditionalAudioFeedback(pos);
  } else if (currentGameMode == WORTRAETSEl) {
    return wortraetsel_calcCharAdditionalAudioFeedback(pos, feedbackCounter);
  }
  return {0, 0};
}

// initializes a game dependend on current game mode
void initGame(Tag tag) {
  if (currentGameMode == KEIN_SPIEL) {
    wordTemplate = {0}; // empty template
  } else if (currentGameMode == WORTE_NACHLEGEN) {
    worteNachlegen_initGame(tag);
  } else if (currentGameMode == PURZELWOERTER) {
    purzelwoerter_initGame();
  } else if (currentGameMode == WORTRAETSEl) {
    wortraetsel_initGame();
  }
}

bool currentStateMatchesWantedState(int pos) {
  return currentCharState[pos].value == wantedCharState[pos].value;
}

// decides for the sub game mode depending on current game mode 
void setGameSubMode() {
  if (currentGameMode == KEIN_SPIEL) {
    prevGameSubMode = currentGameSubMode;
    currentGameSubMode = NO_SUB;
  } else if (currentGameMode == WORTE_NACHLEGEN) {
    if (worteNachlegen_istBeendet()) {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = WORTE_NACHLEGEN_BEENDET;
    } else {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = NO_SUB;
    }
  } else if (currentGameMode == PURZELWOERTER) {
    if (currentGameSubMode == PURZELWOERTER_BUCHSTABEN_ABLEGEN && purzelwoerter_istAblegenBeendet()) {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = PURZELWOERTER_BUCHSTABEN_SORTIEREN;
    } else if (currentGameSubMode == PURZELWOERTER_BUCHSTABEN_SORTIEREN && purzelwoerter_istSortierenBeendet()) {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = PURZELWOERTER_BEENDET;
    } else if (currentGameSubMode == PURZELWOERTER_BUCHSTABEN_SORTIEREN && !purzelwoerter_istSortierenBeendet()) {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = PURZELWOERTER_BUCHSTABEN_SORTIEREN;
    }
  } else if (currentGameMode == WORTRAETSEl) {
    if (wortraetsel_istBeendet()) {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = WORTRAETSEl_BEENDET;
    } else {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = NO_SUB;
    }
  }
}

// handle possible submode change
void handleGameSubModeChange() {
  if (currentGameSubMode != prevGameSubMode) {
    if (currentGameMode == WORTE_NACHLEGEN) {
      worteNachlegen_handleSubModeChange();
    } else if (currentGameMode == PURZELWOERTER) {
      purzelwoerter_handleSubModeChange();
    } else if (currentGameMode == WORTRAETSEl) {
      wortraetsel_handleSubModeChange();
    }
  }
}


// ======================== BUTTONS ===========================================

// check for input / if the state of a button has changed
void checkButtons() {
  for (int i = 0; i < 8; i++) {
    btnStatesPrev[i] = btnStates[i];
    btnStates[i] = digitalRead(btnPins[i]);
    if (btnStates[i] == HIGH && btnStates[i] != btnStatesPrev[i]) {
      handleButtonPressed(i);
    }
  }
}

// decides which audio should be played if a button was pressed
void handleButtonPressed(int btn) {

  // log event information for study
  Serial.print("Button Nummer "); Serial.print(btn); Serial.println(" gedrueckt.");

  // if game mode button was pressed...
  if (btn == 0) {
    
    if (currentGameMode == KEIN_SPIEL) {
      //repeat braille buddy intro hallo
      Audio hallo = {1, 0};
      addToAudioQueue(hallo);
    } else if (currentGameMode == WORTE_NACHLEGEN) {
      if (currentGameSubMode == WORTE_NACHLEGEN_BEENDET) {
        worteNachlegen_handleBeendet(true);
      } else {
        forceUpdateAudioQueue(wordTemplate.audio);
        addToAudioQueue({3, 0}); //Kannst du dieses Wort nachlegen?
      }
    } else if (currentGameMode == PURZELWOERTER) {
      purzelwoerter_handleFirstButtonPressed();
    } else if (currentGameMode == WORTRAETSEl) {
      wortraetsel_handleFirstButtonPressed();
    }
    
  } else { 
    
    // if a letter button was pressed...
    if (currentGameMode == PURZELWOERTER && currentGameSubMode == PURZELWOERTER_BUCHSTABEN_ABLEGEN && currentCharState[btn - 1].occurance == 0) {
      if (wantedCharState[btn - 1].audio.folder != 0) {
        forceUpdateAudioQueue(wantedCharState[btn - 1].audio);
      } else {
        forceUpdateAudioQueue(audioFeedbackLeeresFeld[0]);
      }
    } else if (currentGameMode == KEIN_SPIEL) {
      forceUpdateAudioQueue(charAudioFeedback[btn - 1][0]);
    } else  {
      forceUpdateAudioQueue(charAudioFeedback[btn - 1][0]);
      for (int a = 1; a < AUDIO_FEEDBACK_SIZE; a++) {
        addToAudioQueue(charAudioFeedback[btn - 1][a]);
      }
    }
    
  }
  
}
