void wortraetsel_initGame() {
  Serial.println("init wort raetsel");
  freeWordTemplateAndWantedCharState();
  forceUpdateAudioQueue({5, 0}); //Willkommen beim Worträtsel
  addToAudioQueue({5, 1}); // Ich denke mir ein Wort aus, kannst du es erraten?

  int wordIdx = random(0, 10);
  wordTemplate = woerter[wordIdx];

  int counter = 0;
  while ((wordTemplate.audio.folder == 0 && wordTemplate.audio.track == 0) || counter < 10) {
    wordIdx = random(0, 10);
    wordTemplate = woerter[wordIdx];
    counter += 1;
  }
  woerter[wordIdx] = {0};

  int nrOfBrailleChars = getNumberOfBrailleChars(wordTemplate);
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (i < nrOfBrailleChars) {
      wantedCharState[i] = wordTemplate.brailleChars[i];
    } else {
      wantedCharState[i] = {0};
    }
  }
  wortraetsel_playWortRaetselIntro(false);
  Serial.println("Spiel Wortraetsel gestartet.");
  printCurrentWordTemplate();
  printWantedCharState();
  printCurrentCharState();
}

Audio wortraetsel_calcCharAdditionalAudioFeedback(int pos, int feedbackCounter) {
  Audio frequencyAudios[4] = {{5, 16}, {5, 17}, {5, 18}, {5, 19}}; // einmal, zweimal, dreimal, viermal
  int idx = wortraetsel_findIndexInWantedCharState(currentCharState[pos], pos);
  int frequency = wortraetsel_getCharFrequency(currentCharState[pos]);
  boolean isSmall = currentCharState[pos].audio.track > 36;
  boolean nichtVorhanden = idx < 0;
  boolean kleinVorhanden;
  boolean grossVorhanden;
  int idxOfKlein;
  int idxOfGross;
  int trackOfKlein;
  int trackOfGross;

  if (isSmall) {
    kleinVorhanden = true;
    idxOfGross = wortraetsel_findIndexOfGrossInWantedCharState(currentCharState[pos], pos);
    idxOfKlein = idx;
    trackOfKlein = currentCharState[pos].audio.track;
    trackOfGross = wortraetsel_getMatchingGrossTrackForKleinTrack(trackOfKlein);
    grossVorhanden = idxOfGross > -1;
  } else {
    grossVorhanden = true;
    idxOfKlein = wortraetsel_findIndexOfKleinInWantedCharState(currentCharState[pos], pos);
    idxOfGross = idx;
    trackOfGross = currentCharState[pos].audio.track;
    trackOfKlein = wortraetsel_getMatchingKleinTrackForGrossTrack(trackOfGross);
    kleinVorhanden = idxOfKlein > -1;
  }
  if (nichtVorhanden) {
    if (feedbackCounter == 1) {
      return{5, 12}; // nicht enthalten
    }
    if (isSmall && grossVorhanden) {
      if (feedbackCounter == 2) {
        return{5, 22}; // aber groß...
      }
      if (feedbackCounter == 3) {
        return{2, trackOfKlein}; // <Buchstabe>...
      }
      if (feedbackCounter == 4) {
        return{5, 23}; // ...ist enthalten
      }
    } else if (!isSmall && kleinVorhanden) {
      if (feedbackCounter == 2) {
        return{5, 21}; // aber klein...
      }
      if (feedbackCounter == 3) {
        return{2, trackOfKlein}; // <Buchstabe>...
      }
      if (feedbackCounter == 4) {
        return{5, 23}; // ...ist enthalten
      }
    }
  } else {

    int frequencySum = frequency;
    if (isSmall && grossVorhanden) {
      frequencySum += 1;
    }
    if ( frequencySum > 1) {
      if (feedbackCounter == 1) {
        return frequencyAudios[frequencySum - 1]; // ..."einmal", "zweimal", "dreimal", "viermal"...
      } else if (isSmall && grossVorhanden && feedbackCounter == 2) {
        return {5, 20}; // ... davon einmal in gross...
      } else if (feedbackCounter == 3) {
        if (idx == pos) {
          return {5, 14}; // richtige position
        } else if (idx > -1) {
          return {5, 13};  // falsche position
        }
      }
    } else if (feedbackCounter == 1) {
      return {5, 23}; // ist enthalten
    } else if (feedbackCounter == 2) {
      if (idx == pos) {
        return {5, 14}; // richtige position
      } else if (idx > -1) {
        return {5, 13};  // falsche position
      }
    }

  }

  return {0, 0};
}

int wortraetsel_getCharFrequency(BrailleChar bChar) {
  int frequency = 0;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (wordTemplate.brailleChars[i].audio.track == bChar.audio.track) {
      frequency += 1;
    }
  }
  return frequency;
}

void wortraetsel_handleSubModeChange() {
  if (currentGameSubMode == WORTRAETSEl_BEENDET) {
    wortraetsel_handleBeendet(false);
  }
}

void wortraetsel_handleBeendet(boolean force) {
  if (force) {
    forceUpdateAudioQueue({1, 4});
  } else {
    eval = true;
    Serial.println("Spiel Wortraetsel erfolgreich beendet.");
    printCurrentWordTemplate();
    printWantedCharState();
    printCurrentCharState();
    addToAudioQueue({1, 4});
  }
  // eval korrekt sound
  addToAudioQueue({5, 15});
  addToAudioQueue(wordTemplate.audio);
  // positive Feedback
  int positiveFeedback = random(7, 11);
  addToAudioQueue({1, positiveFeedback});
  // Erklärung wie man eine neue Runde startet
  //addToAudioQueue({4, 3});
  addToWordHistory(wordTemplate);
}

boolean wortraetsel_istBeendet() {
  boolean result = true;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (currentCharState[i].audio.track != wantedCharState[i].audio.track) {
      result = false;
      break;
    }
  }
  return result;
}

void wortraetsel_playWortRaetselIntro(boolean force) {
  int numberTracks[6] = {3, 4, 5, 6, 7, 8};
  int nrOfBrailleChars = getNumberOfBrailleChars(wordTemplate);
  int nrOfCapitalLetters = getNumberOfCapitalLetters(wordTemplate);
  nrOfBrailleChars += nrOfCapitalLetters;
  if (force) {
    forceUpdateAudioQueue({5, 2}); // Das Wort, an das ich denke, hat
  } else {
    addToAudioQueue({5, 2}); // Das Wort, an das ich denke, hat
  }
  addToAudioQueue({5, numberTracks[nrOfBrailleChars - 3]}); // Das Wort, an das ich denke, hat
  addToAudioQueue({5, 9}); // Braillezeichen
  if (nrOfCapitalLetters > 0) {
    addToAudioQueue({5, 10}); // inkl. Großbuchstabenzeichen
  }
  addToAudioQueue({5, 11}); // Braillezeichen
}

void wortraetsel_handleFirstButtonPressed() {
  if (currentGameSubMode == WORTRAETSEl_BEENDET && allCharPosEmpty()) {
    wortraetsel_initGame(); // neue Runde starten
  } else if (currentGameSubMode == WORTRAETSEl_BEENDET && !allCharPosEmpty()) {
    wortraetsel_handleBeendet(true);
  } else if (currentGameSubMode == NO_SUB) {
    wortraetsel_playWortRaetselIntro(true);
  }
}

int wortraetsel_findIndexInWantedCharState(BrailleChar bC, int pos) {
  int index = -1;
  if (wantedCharState[pos].audio.track == bC.audio.track) {
    index = pos;
    return index;
  }
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (currentCharState[i].audio.track != wantedCharState[i].audio.track && bC.audio.track == wantedCharState[i].audio.track && wantedCharState[i].audio.folder == 2) {
      index =  i;
      return index;
    }
  }
  return index;
}

int wortraetsel_findIndexOfGrossInWantedCharState(BrailleChar bC, int pos) {
  int index = -1;
  int matchingGrossTrack = wortraetsel_getMatchingGrossTrackForKleinTrack(bC.audio.track);
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (matchingGrossTrack == wantedCharState[i].audio.track && wantedCharState[i].audio.folder == 2) {
      index =  i;
      return index;
    }
  }
  return index;
}

int wortraetsel_findIndexOfKleinInWantedCharState(BrailleChar bC, int pos) {
  int index = -1;
  int matchingKleinTrack = wortraetsel_getMatchingKleinTrackForGrossTrack(bC.audio.track);
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (matchingKleinTrack == wantedCharState[i].audio.track && wantedCharState[i].audio.folder == 2) {
      index =  i;
      return index;
    }
  }
  return index;
}

int wortraetsel_getMatchingGrossTrackForKleinTrack(int kleinTrack) {
  return kleinTrack - 37;
}

int wortraetsel_getMatchingKleinTrackForGrossTrack(int grossTrack) {
  return grossTrack + 37;
}
