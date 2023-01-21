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
  NO_GAME,
  COPY_WORD,
  WORD_SCRAMBLE,
  WORD_RIDDLE
};

enum GameSubModes {
  NO_SUB,
  COPY_WORD_FINISHED,
  WORD_SCRAMBLE_PLACE_LETTERS,
  WORD_SCRAMBLE_SORT_LETTERS,
  WORD_SCRAMBLE_FINISHED,
  WORD_RIDDLE_FINISHED
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

// index of first lowercase letter in the letter audio folder **GERMAN VERSION**
#define INDEX_OF_FIRST_LOWERCASE_LETTER 37

// index of first lowercase letter in the letter audio folder **ENGLISH VERSION**
// #define INDEX_OF_FIRST_LOWERCASE_LETTER 26

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


Audio hello = {1, 0};
Audio evaluationCorrectFanfare = {1, 4};
Audio copyWordsCapCharMissing = {1, 12};
Audio gameCardRemoved = {1, 13};

Audio copyWordsIntro = {3, 0};

Audio welcomeToWordScramble = {4, 0};
Audio wordScrambleIntro = {4, 1};
Audio wordScrambleLettersWerePlacedInCorrectOrder = {4, 2};
Audio wordScrambleHowToStartNewGameRound = {4, 3};
Audio wordScrambleSolution = {4, 4};

Audio wordRiddleWelcome = {5, 0};
Audio wordRiddleIntro1 = {5, 1};
Audio wordRiddleIntro2 = {5, 2};
Audio wordRiddleBrailleCharacters = {5, 9};
Audio wordRiddleIncludingLeadingCapsChar = {5, 10};
Audio wordRiddleInstruction = {5, 11};
Audio wordRiddleNotIncluded = {5, 12};
Audio wordRiddleButAtAnotherPosition = {5, 13};
Audio wordRiddleAndRightPosition = {5, 14};
Audio wordRiddleWordFound = {5, 15};
Audio wordRiddleIncludedOnce = {5, 16};
Audio wordRiddleIncludedTwice = {5, 17};
Audio wordRiddleIncludedThreeTimes = {5, 18};
Audio wordRiddleIncludedFourTimes = {5, 19};
Audio wordRiddleOneUppercase = {5, 20};
Audio wordRiddleButLowercaseIs = {5, 21};
Audio wordRiddleButUppercaseIs = {5, 22};
Audio wordRiddleIsIncluded = {5, 23};

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
int photovalues[NR_OF_CHAR_POS] = {0, 0, 0, 0, 0, 0, 0};
// stores the initial light situation read at each letter slot at the initial setup
// to decide whether the light situation has changed
int initialPhotovalues[NR_OF_CHAR_POS] = {0, 0, 0, 0, 0, 0, 0};

//=================================== Multiplexer ========================================

// pins used to switch between multiplexed channels
byte multiplexerPins[4] = {2, 0, 4, 16};
// pin used to read or write to the selected channel
byte multiplexerSignalPin = 15;

// the photoresistors are being multiplexed
int mpPhotoSelection[NR_OF_CHAR_POS][4] = {
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
GameModes currentGameMode = NO_GAME;
// is used to check for a game mode change
GameModes prevGameMode = NO_GAME;


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

// collection of all needed braille char information **GERMAN VERSION START**
BrailleChar aUppercase = {2, 0, 1, "A"};
BrailleChar bUppercase = {2, 1, 1, "B"};
BrailleChar cUppercase = {2, 2, 1, "C"};
BrailleChar dUppercase = {2, 3, 1, "D"};
BrailleChar eUppercase = {2, 4, 1, "E"};
BrailleChar fUppercase = {2, 5, 1, "F"};
BrailleChar gUppercase = {2, 6, 1, "G"};
BrailleChar hUppercase = {2, 7, 1, "H"};
BrailleChar iUppercase = {2, 8, 1, "I"};
BrailleChar jUppercase = {2, 9, 1, "J"};
BrailleChar kUppercase = {2, 10, 1, "K"};
BrailleChar lUppercase = {2, 11, 1, "L"};
BrailleChar mUppercase = {2, 12, 1, "M"};
BrailleChar nUppercase = {2, 13, 1, "N"};
BrailleChar oUppercase = {2, 14, 1, "O"};
BrailleChar pUppercase = {2, 15, 1, "P"};
BrailleChar qUppercase = {2, 16, 1, "Q"};
BrailleChar rUppercase = {2, 17, 1, "R"};
BrailleChar sUppercase = {2, 18, 1, "S"};
BrailleChar tUppercase = {2, 19, 1, "T"};
BrailleChar uUppercase = {2, 20, 1, "U"};
BrailleChar vUppercase = {2, 21, 1, "V"};
BrailleChar wUppercase = {2, 22, 1, "W"};
BrailleChar xUppercase = {2, 23, 1, "X"};
BrailleChar yUppercase = {2, 24, 1, "Y"};
BrailleChar zUppercase = {2, 25, 1, "Z"};
BrailleChar aeUppercase = {2, 26, 1, "Ae"};
BrailleChar oeUppercase = {2, 27, 1, "Oe"};
BrailleChar ueUppercase = {2, 28, 1, "Ue"};
BrailleChar auUppercase = {2, 29, 1, "Au"};
BrailleChar chUppercase = {2, 30, 1, "Ch"};
BrailleChar eiUppercase = {2, 31, 1, "Ei"};
BrailleChar euUppercase = {2, 32, 1, "Eu"};
BrailleChar ieUppercase = {2, 33, 1, "Ie"};
BrailleChar schUppercase = {2, 34, 1, "Sch"};
BrailleChar stUppercase = {2, 35, 1, "St"};
BrailleChar aeuUppercase = {2, 36, 1, "Aeu"};

BrailleChar aLowercase = {2, 37, 1, "a"};
BrailleChar bLowercase = {2, 38, 1, "b"};
BrailleChar cLowercase = {2, 39, 1, "c"};
BrailleChar dLowercase = {2, 40, 1, "d"};
BrailleChar eLowercase = {2, 41, 1, "e"};
BrailleChar fLowercase = {2, 42, 1, "f"};
BrailleChar gLowercase = {2, 43, 1, "g"};
BrailleChar hLowercase = {2, 44, 1, "h"};
BrailleChar iLowercase = {2, 45, 1, "i"};
BrailleChar jLowercase = {2, 46, 1, "j"};
BrailleChar kLowercase = {2, 47, 1, "k"};
BrailleChar lLowercase = {2, 48, 1, "l"};
BrailleChar mLowercase = {2, 49, 1, "m"};
BrailleChar nLowercase = {2, 50, 1, "n"};
BrailleChar oLowercase = {2, 51, 1, "o"};
BrailleChar pLowercase = {2, 52, 1, "p"};
BrailleChar qLowercase = {2, 53, 1, "q"};
BrailleChar rLowercase = {2, 54, 1, "r"};
BrailleChar sLowercase = {2, 55, 1, "s"};
BrailleChar tLowercase = {2, 56, 1, "t"};
BrailleChar uLowercase = {2, 57, 1, "u"};
BrailleChar vLowercase = {2, 58, 1, "v"};
BrailleChar wLowercase = {2, 59, 1, "w"};
BrailleChar xLowercase = {2, 60, 1, "x"};
BrailleChar yLowercase = {2, 61, 1, "y"};
BrailleChar zLowercase = {2, 62, 1, "z"};
BrailleChar aeLowercase = {2, 63, 1, "ae"};
BrailleChar oeLowercase = {2, 64, 1, "oe"};
BrailleChar ueLowercase = {2, 65, 1, "ue"};
BrailleChar auLowercase = {2, 66, 1, "au"};
BrailleChar chLowercase = {2, 67, 1, "ch"};
BrailleChar eiLowercase = {2, 68, 1, "ei"};
BrailleChar euLowercase = {2, 69, 1, "eu"};
BrailleChar ieLowercase = {2, 70, 1, "ie"};
BrailleChar schLowercase = {2, 71, 1, "sch"};
BrailleChar stLowercase = {2, 72, 1, "st"};
BrailleChar aeuLowercase = {2, 73, 1, "aeu"};
BrailleChar ssLowercase = {2, 74, 1, "ss"};
// collection of all needed braille char information **GERMAN VERSION END**

// collection of all needed braille char information **ENGLISH VERSION START**
// TODO: include english grade 2 braille abbreviation characters to audiofiles
/*BrailleChar aLowercase = {2, 26, 1, "a"};
BrailleChar bLowercase = {2, 27, 1, "b"};
BrailleChar cLowercase = {2, 28, 1, "c"};
BrailleChar dLowercase = {2, 29, 1, "d"};
BrailleChar eLowercase = {2, 30, 1, "e"};
BrailleChar fLowercase = {2, 31, 1, "f"};
BrailleChar gLowercase = {2, 32, 1, "g"};
BrailleChar hLowercase = {2, 33, 1, "h"};
BrailleChar iLowercase = {2, 34, 1, "i"};
BrailleChar jLowercase = {2, 35, 1, "j"};
BrailleChar kLowercase = {2, 36, 1, "k"};
BrailleChar lLowercase = {2, 37, 1, "l"};
BrailleChar mLowercase = {2, 38, 1, "m"};
BrailleChar nLowercase = {2, 39, 1, "n"};
BrailleChar oLowercase = {2, 40, 1, "o"};
BrailleChar pLowercase = {2, 41, 1, "p"};
BrailleChar qLowercase = {2, 42, 1, "q"};
BrailleChar rLowercase = {2, 43, 1, "r"};
BrailleChar sLowercase = {2, 44, 1, "s"};
BrailleChar tLowercase = {2, 45, 1, "t"};
BrailleChar uLowercase = {2, 46, 1, "u"};
BrailleChar vLowercase = {2, 47, 1, "v"};
BrailleChar wLowercase = {2, 48, 1, "w"};
BrailleChar xLowercase = {2, 49, 1, "x"};
BrailleChar yLowercase = {2, 50, 1, "y"};
BrailleChar zLowercase = {2, 51, 1, "z"};*/
// collection of all needed braille char information **ENGLISH VERSION END**

// collection of all needed word information **GERMAN VERSION START**
BrailleWord ameise = { 6, 16, {aUppercase, mLowercase, eiLowercase, sLowercase, eLowercase}};
BrailleWord monat = { 9, 32, {mUppercase, oLowercase, nLowercase, aLowercase, tLowercase}};
BrailleWord muesli = { 9, 60, {mUppercase, ueLowercase, sLowercase, lLowercase, iLowercase}};
BrailleWord pferd = { 9, 171, {pUppercase, fLowercase, eLowercase, rLowercase, dLowercase}};
BrailleWord regen = { 10, 12, {rUppercase, eLowercase, gLowercase, eLowercase, nLowercase}};
BrailleWord sonne = { 10, 236, {sUppercase, oLowercase, nLowercase, nLowercase, eLowercase}};
BrailleWord tafel = { 11, 111, {tUppercase, aLowercase, fLowercase, eLowercase, lLowercase}};
BrailleWord topf = { 11, 172, {tUppercase, oLowercase, pLowercase, fLowercase}};
BrailleWord traum = { 11, 182, {tUppercase, rLowercase, auLowercase, mLowercase}};
BrailleWord vogel = { 12, 6, {vUppercase, oLowercase, gLowercase, eLowercase, lLowercase}};
BrailleWord wolke = { 12, 123, {wUppercase, oLowercase, lLowercase, kLowercase, eLowercase}};
BrailleWord zwerg = { 12, 217, {zUppercase, wLowercase, eLowercase, rLowercase, gLowercase}};
// collection of all needed word information **GERMAN VERSION END**

// collection of all needed word information **ENGLISH VERSION START**
// TODO: include english grade 2 braille abbreviation characters to audiofiles and adjust words correspondingly
/*BrailleWord ant = { 6, 0, {aLowercase, nLowercase, tLowercase}};
BrailleWord month = { 6, 20, {mLowercase, oLowercase, nLowercase, tLowercase, hLowercase}};
BrailleWord muesli = { 6, 21, {mLowercase, uLowercase, eLowercase, sLowercase, lLowercase, iLowercase}};
BrailleWord horse = { 6, 17, {hLowercase, oLowercase, rLowercase, sLowercase, eLowercase}};
BrailleWord rain = { 6, 23, {rmLowercase, aLowercase, iLowercase, nLowercase}};
BrailleWord sun = { 6, 27, {smLowercase, uLowercase, nLowercase}};
BrailleWord flower = { 6, 15, {fLowercase, lLowercase, oLowercase, wLowercase, eLowercase, rLowercase}};
BrailleWord hare = { 6, 16, {hLowercase, aLowercase, rLowercase, eLowercase}};
BrailleWord dream = { 6, 11, {dLowercase, rLowercase, eLowercase, aLowercase, mLowercase}};
BrailleWord bird = { 6, 5, {bLowercase, iLowercase, rLowercase, dLowercase}};
BrailleWord cloud = { 6, 9, {cLowercase, lLowercase, oLowercase, uLowercase, dLowercase}};
BrailleWord shark = { 6, 25, {sLowercase, hLowercase, aLowercase, rLowercase, kLowercase}};*/
// collection of all needed word information **ENGLISH VERSION END**

// collection of words the system can choose from **GERMAN VERSION**
BrailleWord wordList[11] = {traum, ameise, vogel, zwerg, topf, regen, monat, wolke, sonne, pferd, tafel};
// collection of words the system can choose from **ENGLISH VERSION**
//BrailleWord wordList[11] = {ant, month, muesli, horse, rain, sun, flower, hare, dream, bird, cloud};

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
  // initialize audio feedback for empty slots
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
  setInitialPhotovalues();
  delay(50);

  Serial.println(F("end of setup"));
  
  // Intro Audio
  addToAudioQueue(hello);
}


//============================================== PROGRAM LOOP ===================================================================

void loop() {
  if (!eval) { // if no game mode switch with important audio information is happening
    readPhotoValues(); // then update the photovalues
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
    if (currentGameMode != prevGameMode || (currentGameMode == COPY_WORD && isNewWord(currentTag))) {
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
            Serial.print("New letter was placed: "); Serial.print(bChar.value); Serial.print(" position: "); Serial.println(i);
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
      // update audio feedback to "leeres feld" / "empty slot"
      charAudioFeedback[i][0] = audioFeedbackLeeresFeld[0];
      // empty other audio feedback
      for (int a = 1; a < AUDIO_FEEDBACK_SIZE; a++) {
        charAudioFeedback[i][a] = {0};
      }
      if (bChar.occurance != 0) { // if pos wasn't already empty in a prior check
        // log event information for study
        Serial.print("Letter was removed: "); Serial.print(bChar.value); Serial.print(" position: "); Serial.println(i);
        printCurrentWordTemplate();
        printWantedCharState();
        printCurrentCharState();
      }
    }
  }
}

//returns the soundeffect for either a positive or negative feedback
Audio calcCharAudioFeedback(int pos) {
  if (currentGameMode == NO_GAME) {
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
  if (currentGameMode == NO_GAME) {
    return {0, 0};
  } else if (currentGameMode == COPY_WORD && feedbackCounter == 1) {
    return copyWords_calcCharAdditionalAudioFeedback(pos);
  } else if (currentGameMode == WORD_SCRAMBLE && feedbackCounter == 1) {
    return wordScramble_calcCharAdditionalAudioFeedback(pos);
  } else if (currentGameMode == WORD_RIDDLE) {
    return wordRiddle_calcCharAdditionalAudioFeedback(pos, feedbackCounter);
  }
  return {0, 0};
}

// initializes a game dependend on current game mode
void initGame(Tag tag) {
  if (currentGameMode == NO_GAME) {
    wordTemplate = {0}; // empty template
  } else if (currentGameMode == COPY_WORD) {
    copyWords_initGame(tag);
  } else if (currentGameMode == WORD_SCRAMBLE) {
    wordScramble_initGame();
  } else if (currentGameMode == WORD_RIDDLE) {
    wordRiddle_initGame();
  }
}

bool currentStateMatchesWantedState(int pos) {
  return currentCharState[pos].value == wantedCharState[pos].value;
}

// decides for the sub game mode depending on current game mode 
void setGameSubMode() {
  if (currentGameMode == NO_GAME) {
    prevGameSubMode = currentGameSubMode;
    currentGameSubMode = NO_SUB;
  } else if (currentGameMode == COPY_WORD) {
    if (copyWords_istBeendet()) {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = COPY_WORD_FINISHED;
    } else {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = NO_SUB;
    }
  } else if (currentGameMode == WORD_SCRAMBLE) {
    if (currentGameSubMode == WORD_SCRAMBLE_PLACE_LETTERS && wordScramble_allLettersPlaced()) {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = WORD_SCRAMBLE_SORT_LETTERS;
    } else if (currentGameSubMode == WORD_SCRAMBLE_SORT_LETTERS && wordScramble_lettersSorted()) {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = WORD_SCRAMBLE_FINISHED;
    } else if (currentGameSubMode == WORD_SCRAMBLE_SORT_LETTERS && !wordScramble_lettersSorted()) {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = WORD_SCRAMBLE_SORT_LETTERS;
    }
  } else if (currentGameMode == WORD_RIDDLE) {
    if (wordRiddle_isFinished()) {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = WORD_RIDDLE_FINISHED;
    } else {
      prevGameSubMode = currentGameSubMode;
      currentGameSubMode = NO_SUB;
    }
  }
}

// handle possible submode change
void handleGameSubModeChange() {
  if (currentGameSubMode != prevGameSubMode) {
    if (currentGameMode == COPY_WORD) {
      copyWords_handleSubModeChange();
    } else if (currentGameMode == WORD_SCRAMBLE) {
      wordScramble_handleSubModeChange();
    } else if (currentGameMode == WORD_RIDDLE) {
      wordRiddle_handleSubModeChange();
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
  Serial.print("Button number "); Serial.print(btn); Serial.println(" was pressed.");

  // if game mode button was pressed...
  if (btn == 0) {
    
    if (currentGameMode == NO_GAME) {
      //repeat braille buddy intro
      addToAudioQueue(hello);
    } else if (currentGameMode == COPY_WORD) {
      if (currentGameSubMode == COPY_WORD_FINISHED) {
        copyWords_handleBeendet(true);
      } else {
        forceUpdateAudioQueue(wordTemplate.audio);
        addToAudioQueue(copyWordsIntro);
      }
    } else if (currentGameMode == WORD_SCRAMBLE) {
      wordScramble_handleFirstButtonPressed();
    } else if (currentGameMode == WORD_RIDDLE) {
      wordRiddle_handleFirstButtonPressed();
    }
    
  } else { 
    
    // if a letter button was pressed...
    if (currentGameMode == WORD_SCRAMBLE && currentGameSubMode == WORD_SCRAMBLE_PLACE_LETTERS && currentCharState[btn - 1].occurance == 0) {
      if (wantedCharState[btn - 1].audio.folder != 0) {
        forceUpdateAudioQueue(wantedCharState[btn - 1].audio);
      } else {
        forceUpdateAudioQueue(audioFeedbackLeeresFeld[0]);
      }
    } else if (currentGameMode == NO_GAME) {
      forceUpdateAudioQueue(charAudioFeedback[btn - 1][0]);
    } else  {
      forceUpdateAudioQueue(charAudioFeedback[btn - 1][0]);
      for (int a = 1; a < AUDIO_FEEDBACK_SIZE; a++) {
        addToAudioQueue(charAudioFeedback[btn - 1][a]);
      }
    }
    
  }
  
}
