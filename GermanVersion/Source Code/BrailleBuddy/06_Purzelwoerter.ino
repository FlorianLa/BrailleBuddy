void purzelwoerter_initGame() {
  currentGameSubMode = PURZELWOERTER_BUCHSTABEN_ABLEGEN;
  freeWordTemplateAndWantedCharState();

  // choose random word from wordlist
  int wordIdx = random(0, 10);
  BrailleWord purzelwort = woerter[wordIdx];

  int counter = 0;
  while ((purzelwort.audio.folder == 0 && purzelwort.audio.track == 0) || counter < 10) {
    wordIdx = random(0, 10);
    purzelwort = woerter[wordIdx];
    counter += 1;
  }
  woerter[wordIdx] = {0};

  // shuffle chosen word
  int nrOfBrailleChars = getNumberOfBrailleChars(purzelwort);
  BrailleWord shuffledPurzelwort = shuffleWord(purzelwort, nrOfBrailleChars);

  // set word template
  wordTemplate = purzelwort;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (i < nrOfBrailleChars) {
      wordTemplate.brailleChars[i] = purzelwort.brailleChars[i];
      // set wanted char state
      wantedCharState[i] = shuffledPurzelwort.brailleChars[i];
    } else {
      wordTemplate.brailleChars[i] = {0};
      // set wanted char state
      wantedCharState[i] = {0};
    }
  }

  forceUpdateAudioQueue({4, 0});
  addToAudioQueue({4, 1});
  for (int i = 0; i < nrOfBrailleChars; i++) {
    addToAudioQueue(wantedCharState[i].audio);
  }
  Serial.println("Spiel Purzelwoerter gestartet ");
  printCurrentWordTemplate();
  printWantedCharState();
  printCurrentCharState();
}

BrailleWord shuffleWord(BrailleWord purzelwort, int nrOfBrailleChars) {
  BrailleWord shuffledWord;
  int idxs[nrOfBrailleChars];
  boolean alreadyUsed[nrOfBrailleChars];
  for (int i = 0; i < nrOfBrailleChars; i++) {
    alreadyUsed[i] = false;
  }
  for (int i = 0; i < nrOfBrailleChars; i++) {
    int idx = random(0, nrOfBrailleChars);
    while (alreadyUsed[idx] || purzelwort.brailleChars[idx].audio.track == purzelwort.brailleChars[i].audio.track) {
      idx = random(0, nrOfBrailleChars);
    }
    shuffledWord.brailleChars[i] = purzelwort.brailleChars[idx];
    alreadyUsed[idx] = true;
  }
  return shuffledWord;
}


Audio purzelwoerter_calcCharAdditionalAudioFeedback(int pos) {
  boolean isSmall = currentCharState[pos].audio.track > 36;
  if (
    isSmall
    && wantedCharState[pos].audio.folder == currentCharState[pos].audio.folder
    && wantedCharState[pos].audio.track == wortraetsel_getMatchingGrossTrackForKleinTrack(currentCharState[pos].audio.track)) {
    return {1, 12}; // Das Großschreibzeichen fehlt;
  }
  return {0, 0};
}

void purzelwoerter_handleFirstButtonPressed() {
  if (currentGameSubMode == PURZELWOERTER_BEENDET && allCharPosEmpty()) {
    purzelwoerter_initGame(); // neue Runde starten
  } else if (currentGameSubMode == PURZELWOERTER_BEENDET && !allCharPosEmpty()) {
    // wiederhole Evaluation und erkläre wie eine neue Runde gestartet wird
    forceUpdateAudioQueue({1, 4}); // eval korrekt sound
    //Die Lösung des Purzelworts lautet <Hamster>
    addToAudioQueue({4, 4});
    addToAudioQueue(wordTemplate.audio);
    // positive Feedback
    int positiveFeedback = random(7, 11);
    addToAudioQueue({1, positiveFeedback});
    // Erklärung wie man eine neue Runde startet
    addToAudioQueue({4, 3});
  } else if (currentGameSubMode == PURZELWOERTER_BUCHSTABEN_ABLEGEN) {
    // welche Buchstaben sollen in welcher Reihenfolge abgelegt werden?
    forceUpdateAudioQueue({4, 1});
    int nrOfBrailleChars = getNumberOfBrailleChars(wordTemplate);
    for (int i = 0; i < nrOfBrailleChars; i++) {
      addToAudioQueue(wantedCharState[i].audio);
    }
  } else if (currentGameSubMode == PURZELWOERTER_BUCHSTABEN_SORTIEREN) {
    forceUpdateAudioQueue({4, 2});
  }
}

boolean purzelwoerter_istSortierenBeendet() {
  boolean result = true;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (currentCharState[i].audio.track != wordTemplate.brailleChars[i].audio.track) {
      result = false;
      break;
    }
  }
  if (currentGameSubMode == PURZELWOERTER_BUCHSTABEN_SORTIEREN && result) {
    eval = true;
    addToAudioQueue({1, 4}); // eval korrekt sound
    //Die Lösung des Purzelworts lautet <Hamster>
    addToAudioQueue({4, 4});
    addToAudioQueue(wordTemplate.audio);

    int positiveFeedback = random(7, 11);
    addToAudioQueue({1, positiveFeedback});
    //addToAudioQueue({4, 3});
    Serial.println("Purzelwoerter: Buchstaben soriteren erfolgreich beendet. Spiel beendet.");
    printCurrentWordTemplate();
    printWantedCharState();
    printCurrentCharState();
  }
  return result;
}

boolean purzelwoerter_istAblegenBeendet() {
  boolean result = true;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (currentCharState[i].audio.track != wantedCharState[i].audio.track) {
      result = false;
      break;
    }
  }
  if (currentGameSubMode == PURZELWOERTER_BUCHSTABEN_ABLEGEN && result) {
    addToAudioQueue({4, 2});
    addToWordHistory(wordTemplate);
    Serial.println("Purzelwoerter: Buchstaben ablegen erfolgreich beendet.");
    printCurrentWordTemplate();
    printWantedCharState();
    printCurrentCharState();
  }
  return result;
}

void purzelwoerter_handleSubModeChange() {
  if (prevGameSubMode == PURZELWOERTER_BUCHSTABEN_ABLEGEN && currentGameSubMode == PURZELWOERTER_BUCHSTABEN_SORTIEREN) {
    //dann update den wanted state
    for (int i = 0; i < NR_OF_CHAR_POS; i++) {
      wantedCharState[i] = wordTemplate.brailleChars[i];
    }
  }
}
