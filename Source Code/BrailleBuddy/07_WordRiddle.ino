void wordRiddle_initGame() {
  Serial.println("init wort raetsel");
  freeWordTemplateAndWantedCharState();
  forceUpdateAudioQueue(wordRiddleWelcome);
  addToAudioQueue(wordRiddleIntro1);

  int wordIdx = random(0, 10);
  wordTemplate = wordList[wordIdx];

  int counter = 0;
  while ((wordTemplate.audio.folder == 0 && wordTemplate.audio.track == 0) || counter < 10) {
    wordIdx = random(0, 10);
    wordTemplate = wordList[wordIdx];
    counter += 1;
  }
  wordList[wordIdx] = {0};

  int nrOfBrailleChars = getNumberOfBrailleChars(wordTemplate);
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (i < nrOfBrailleChars) {
      wantedCharState[i] = wordTemplate.brailleChars[i];
    } else {
      wantedCharState[i] = {0};
    }
  }
  wordRiddle_playWordRiddleIntro(false);
  Serial.println("Game Round Word Riddle started.");
  printCurrentWordTemplate();
  printWantedCharState();
  printCurrentCharState();
}

Audio wordRiddle_calcCharAdditionalAudioFeedback(int pos, int feedbackCounter) {
  Audio frequencyAudios[4] = {
    wordRiddleIncludedOnce, 
    wordRiddleIncludedTwice, 
    wordRiddleIncludedThreeTimes, 
    wordRiddleIncludedFourTimes
  };
  int idx = wordRiddle_findIndexInWantedCharState(currentCharState[pos], pos);
  int frequency = wordRiddle_getCharFrequency(currentCharState[pos]);
  boolean isSmall = currentCharState[pos].audio.track >= INDEX_OF_FIRST_LOWERCASE_LETTER;
  boolean notIncluded = idx < 0;
  boolean lowercaseIncluded;
  boolean uppercaseIncluded;
  int idxOfLowercase;
  int idxOfUppercase;
  int trackOfLowercase;
  int trackOfUppercase;

  if (isSmall) {
    lowercaseIncluded = true;
    idxOfUppercase = wordRiddle_findIndexOfUppercaseLetterInWantedCharState(currentCharState[pos], pos);
    idxOfLowercase = idx;
    trackOfLowercase = currentCharState[pos].audio.track;
    trackOfUppercase = wordRiddle_getMatchingUppercaseLetterTrackForLowercaseLetterTrack(trackOfLowercase);
    uppercaseIncluded = idxOfUppercase > -1;
  } else {
    uppercaseIncluded = true;
    idxOfLowercase = wordRiddle_findIndexOfLowercaseLetterInWantedCharState(currentCharState[pos], pos);
    idxOfUppercase = idx;
    trackOfUppercase = currentCharState[pos].audio.track;
    trackOfLowercase = wordRiddle_getMatchingLowercaseLetterTrackForUppercaseLetterTrack(trackOfUppercase);
    lowercaseIncluded = idxOfLowercase > -1;
  }
  if (notIncluded) {
    if (feedbackCounter == 1) {
      return wordRiddleNotIncluded;
    }
    if (isSmall && uppercaseIncluded) {
      if (feedbackCounter == 2) {
        return wordRiddleButUppercaseIs;
      }
      if (feedbackCounter == 3) {
        return {2, trackOfLowercase}; // <Letter>...
      }
      if (feedbackCounter == 4) {
        return wordRiddleIsIncluded;
      }
    } else if (!isSmall && lowercaseIncluded) {
      if (feedbackCounter == 2) {
        return wordRiddleButLowercaseIs;
      }
      if (feedbackCounter == 3) {
        return {2, trackOfLowercase}; // <Letter>...
      }
      if (feedbackCounter == 4) {
        return wordRiddleIsIncluded;
      }
    }
  } else {
    int frequencySum = frequency;
    if (isSmall && uppercaseIncluded) {
      frequencySum += 1;
    }
    if ( frequencySum > 1) {
      if (feedbackCounter == 1) {
        return frequencyAudios[frequencySum - 1];
      } else if (isSmall && uppercaseIncluded && feedbackCounter == 2) {
        return wordRiddleOneUppercase;
      } else if (feedbackCounter == 3) {
        if (idx == pos) {
          return wordRiddleAndRightPosition;
        } else if (idx > -1) {
          return wordRiddleButAtAnotherPosition; 
        }
      }
    } else if (feedbackCounter == 1) {
      return wordRiddleIsIncluded;
    } else if (feedbackCounter == 2) {
      if (idx == pos) {
        return wordRiddleAndRightPosition;
      } else if (idx > -1) {
        return wordRiddleButAtAnotherPosition;
      }
    }
  }
  return {0, 0};
}

int wordRiddle_getCharFrequency(BrailleChar bChar) {
  int frequency = 0;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (wordTemplate.brailleChars[i].audio.track == bChar.audio.track) {
      frequency += 1;
    }
  }
  return frequency;
}

void wordRiddle_handleSubModeChange() {
  if (currentGameSubMode == WORD_RIDDLE_FINISHED) {
    wordRiddle_handleFinish(false);
  }
}

void wordRiddle_handleFinish(boolean force) {
  if (force) {
    forceUpdateAudioQueue(evaluationCorrectFanfare);
  } else {
    eval = true;
    Serial.println("A round of Word Riddle was finished successful. ");
    printCurrentWordTemplate();
    printWantedCharState();
    printCurrentCharState();
    addToAudioQueue(evaluationCorrectFanfare);
  }

  addToAudioQueue(wordRiddleWordFound);
  addToAudioQueue(wordTemplate.audio);

  int positiveFeedback = random(7, 11);
  addToAudioQueue({1, positiveFeedback});
  addToWordHistory(wordTemplate);
}

boolean wordRiddle_isFinished() {
  boolean result = true;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (currentCharState[i].audio.track != wantedCharState[i].audio.track) {
      result = false;
      break;
    }
  }
  return result;
}

void wordRiddle_playWordRiddleIntro(boolean force) {
  int numberTracks[6] = {3, 4, 5, 6, 7, 8};
  int nrOfBrailleChars = getNumberOfBrailleChars(wordTemplate);
  int nrOfCapitalLetters = getNumberOfCapitalLetters(wordTemplate);
  nrOfBrailleChars += nrOfCapitalLetters;
  if (force) {
    forceUpdateAudioQueue(wordRiddleIntro2);
  } else {
    addToAudioQueue(wordRiddleIntro2);
  }
  addToAudioQueue({5, numberTracks[nrOfBrailleChars - 3]});
  addToAudioQueue(wordRiddleBrailleCharacters);
  if (nrOfCapitalLetters > 0) {
    addToAudioQueue(wordRiddleIncludingLeadingCapsChar);
  }
  addToAudioQueue(wordRiddleInstruction);
}

void wordRiddle_handleFirstButtonPressed() {
  if (currentGameSubMode == WORD_RIDDLE_FINISHED && allCharPosEmpty()) {
    wortraetsel_initGame(); // start new game round
  } else if (currentGameSubMode == WORD_RIDDLE_FINISHED && !allCharPosEmpty()) {
    wordRiddle_handleFinish(true);
  } else if (currentGameSubMode == NO_SUB) {
    wordRiddle_playWordRiddleIntro(true);
  }
}

int wordRiddle_findIndexInWantedCharState(BrailleChar bC, int pos) {
  int index = -1;
  if (wantedCharState[pos].audio.track == bC.audio.track) {
    index = pos;
    return index;
  }
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (
      currentCharState[i].audio.track != wantedCharState[i].audio.track
      && bC.audio.track == wantedCharState[i].audio.track
      && wantedCharState[i].audio.folder == 2
    ) {
      index =  i;
      return index;
    }
  }
  return index;
}

int wordRiddle_findIndexOfUppercaseLetterInWantedCharState(BrailleChar bC, int pos) {
  int index = -1;
  int matchingUppercaseTrack = wordRiddle_getMatchingUppercaseLetterTrackForLowercaseLetterTrack(bC.audio.track);
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (matchingUppercaseTrack == wantedCharState[i].audio.track && wantedCharState[i].audio.folder == 2) {
      index =  i;
      return index;
    }
  }
  return index;
}

int wordRiddle_findIndexOfLowercaseLetterInWantedCharState(BrailleChar bC, int pos) {
  int index = -1;
  int matchingLowercaseTrack = wordRiddle_getMatchingLowercaseLetterTrackForUppercaseLetterTrack(bC.audio.track);
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (matchingLowercaseTrack == wantedCharState[i].audio.track && wantedCharState[i].audio.folder == 2) {
      index =  i;
      return index;
    }
  }
  return index;
}

int wordRiddle_getMatchingUppercaseLetterTrackForLowercaseLetterTrack(int kleinTrack) {
  return kleinTrack - INDEX_OF_FIRST_LOWERCASE_LETTER;
}

int wordRiddle_getMatchingLowercaseLetterTrackForUppercaseLetterTrack(int grossTrack) {
  return grossTrack + INDEX_OF_FIRST_LOWERCASE_LETTER;
}
