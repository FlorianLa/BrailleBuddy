
/* This file is used to keep the other files cleaner and to collect methods used by more than one other file */

// get game mode enum from game mode id
GameModes getGameMode(int gameModeId) {
  if (gameModeId == 0) {
    return NO_GAME;
  } else if (gameModeId == 1) {
    return COPY_WORD;
  } else if (gameModeId == 2) {
    return WORD_SCRAMBLE;
  } else if (gameModeId == 3) {
    return WORD_RIDDLE;
  }
}

// print folder and track informations of audio files in current audioqueue
void printAudioQueue() {
  Serial.println("");
  for (int i = 0; i < AUDIO_QUEUE_SIZE; i++) {
    Serial.print("folder :");
    Serial.print(audioQueue[i].folder);
    Serial.print(" + ");
    Serial.print("track :");
    Serial.print(audioQueue[i].track);
    Serial.print("     ");
  }
}

// get a substring from a given string
// idx - index of the substring
// str - given string we want to receive a substring of
String getSubstring(int idx, String str) {
  int idxCounter = 0;
  int from = 0;
  for (int i = 0; i < str.length(); i++) {
    String subString = "";
    if (str.charAt(i) == denom) {
      subString = str.substring(from, i);
      subString.trim();
      from = i + 1;
      idxCounter += 1;
    } else if (i == str.length() - 1) {
      subString = str.substring(from, i);
      subString.trim();
      idxCounter += 1;
    }
    if (idxCounter - 1 == idx && subString != "") {
      return subString;
    }
  }

  return "";
}

// this method resets the word template and game state to an empty state 
// only if a threshold was passed to prevent emptiing the template because of an RIFD reader failure
void emptyWordTemplate() {
  emptyWordCounter += 1;
  if (emptyWordCounter > emptyThreshold) {
    if(wordTemplate.audio.track != 0 && wordTemplate.audio.folder != 0) {
     Serial.println("Gamecard was removed.");  
    }
    wordTemplate = {0};
    emptyWordCounter = 0;
    prevGameMode = currentGameMode;
    currentGameMode = NO_GAME;
    currentGameSubMode = NO_SUB;
    for(int i = 0; i < NR_OF_CHAR_POS; i++) {
      wantedCharState[i] = {0};  
    }
    if (prevGameMode != NO_GAME) {
      forceUpdateAudioQueue(gameCardRemoved);
    }
  }
}

// checks if no letter is already stored for that position
boolean isCharPosEmpty(int pos) {
  // each letter has a assigned occurance > 0. if occurance is set to 0, the pos is empty
  return currentCharState[pos].occurance == 0;
}

// decides if a letter was already present on the box or is new
boolean isNewBrailleChar(BrailleChar bChar) {
  boolean result = true;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (currentCharState[i].audio.track == bChar.audio.track && currentCharState[i].occurance == bChar.occurance) {
      result = false;
      return result;
    }
  }
  return result;
}

void printCurrentBrailleCharState() {
  Serial.println("pos covered");
  for (int i = 0; i < 7; i++) {
    Serial.print(isPosCovered(i));
    Serial.print(" ");
  }
  Serial.println("");
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    Serial.print(" ");
    Serial.print(currentCharState[i].value);
    Serial.print(" "); Serial.print(currentCharState[i].occurance);
  }
  Serial.println("");
}


void debug() {
  Serial.println("photovalues:");
  for (int i = 0; i < 7; i++) {
    Serial.print(initialPhotovalues[i]);
    Serial.print(" ");
  }
  Serial.println("");
  for (int i = 0; i < 7; i++) {
    Serial.print(photovalues[i]);
    Serial.print(" ");
  }
  Serial.println("");
  Serial.println("isPosCovered:");
  for (int i = 0; i < 7; i++) {
    Serial.print(isPosCovered(i));
    Serial.print(" ");
  }
  Serial.println("");
}

// tells how many braille letter cards are needed for that word 
int getNumberOfBrailleChars(BrailleWord brailleWord) {
  int result = 0;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (brailleWord.brailleChars[i].value != 0) {
      result += 1;
    }
  }
  return result;
}

// checks if all letter positions are empty
boolean allCharPosEmpty() {
  boolean result = true;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (currentCharState[i].occurance != 0) {
      result = false;
      break;
    }
  }
  return result;
}

// returns the number of capital letters in the given word
int getNumberOfCapitalLetters(BrailleWord wt) {
  int capitalCounter = 0;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (wt.brailleChars[i].audio.track <= 28 && wt.brailleChars[i].audio.folder != 0) {
      capitalCounter += 1;
    }
  }
  return capitalCounter;
}

void printWantedCharState() {
  Serial.println("wanted char state: ");
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    Serial.print(wantedCharState[i].value); Serial.print("  ");
  }
  Serial.println("");
}

void printCurrentCharState() {
  Serial.println("current char state: ");
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    Serial.print(currentCharState[i].value); Serial.print("  ");
  }
  Serial.println("");
}

void printCurrentWordTemplate() {
  Serial.println("current word template: ");
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    Serial.print(wordTemplate.brailleChars[i].value); Serial.print("  ");
  }
  Serial.println("");
}

void addToWordHistory(BrailleWord wordToAdd) {
  if (!isWordInWordHistory(wordToAdd)) {
    for (int i = 0; i < 10; i++) {
      if (wordHistory[i].audio.folder == 0 && wordHistory[i].audio.track == 0) {
        wordHistory[i] = wordToAdd;
        return;
      }
    }
  }
  wordHistory[0] = wordToAdd; // if all places are taken "free" the first word
}

boolean isWordInWordHistory(BrailleWord wordToLookFor) {
  boolean result = false;
  for (int i = 0; i < 11; i++) {
    if (wordHistory[i].audio.folder == wordToLookFor.audio.folder && wordHistory[i].audio.track == wordToLookFor.audio.track) {
      result = true;
      break;
    }
  }
  return result;
}

void freeWordTemplateAndWantedCharState() {
  wordTemplate = {0};
  for(int i = 0; i < NR_OF_CHAR_POS; i++) {
    wantedCharState[i] = {0};
  }
}
