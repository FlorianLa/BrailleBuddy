void wordScramble_initGame() {
  currentGameSubMode = WORD_SCRAMBLE_PLACE_LETTERS;
  freeWordTemplateAndWantedCharState();

  // choose random word from wordlist
  int wordIdx = random(0, 10);
  BrailleWord scrambledWordSolution = wordList[wordIdx];

  int counter = 0;
  while ((scrambledWordSolution.audio.folder == 0 && scrambledWordSolution.audio.track == 0) || counter < 10) {
    wordIdx = random(0, 10);
    scrambledWordSolution = wordList[wordIdx];
    counter += 1;
  }
  wordList[wordIdx] = {0};

  // shuffle chosen word
  int nrOfBrailleChars = getNumberOfBrailleChars(scrambledWord);
  BrailleWord scrambledWord = shuffleWord(scrambledWordSolution, nrOfBrailleChars);

  // set word template
  wordTemplate = scrambledWordSolution;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (i < nrOfBrailleChars) {
      wordTemplate.brailleChars[i] = scrambledWordSolution.brailleChars[i];
      // set wanted char state
      wantedCharState[i] = scrambledWord.brailleChars[i];
    } else {
      wordTemplate.brailleChars[i] = {0};
      // set wanted char state
      wantedCharState[i] = {0};
    }
  }

  forceUpdateAudioQueue(welcomeToWordScramble);
  addToAudioQueue(wordScrambleIntro);
  for (int i = 0; i < nrOfBrailleChars; i++) {
    addToAudioQueue(wantedCharState[i].audio);
  }
  Serial.println("A Game Round Word Scramble was started ");
  printCurrentWordTemplate();
  printWantedCharState();
  printCurrentCharState();
}

BrailleWord shuffleWord(BrailleWord word, int nrOfBrailleChars) {
  BrailleWord shuffledWord;
  int idxs[nrOfBrailleChars];
  boolean alreadyUsed[nrOfBrailleChars];
  for (int i = 0; i < nrOfBrailleChars; i++) {
    alreadyUsed[i] = false;
  }
  for (int i = 0; i < nrOfBrailleChars; i++) {
    int idx = random(0, nrOfBrailleChars);
    while (alreadyUsed[idx] || word.brailleChars[idx].audio.track == word.brailleChars[i].audio.track) {
      idx = random(0, nrOfBrailleChars);
    }
    shuffledWord.brailleChars[i] = word.brailleChars[idx];
    alreadyUsed[idx] = true;
  }
  return shuffledWord;
}


Audio wordScramble_calcCharAdditionalAudioFeedback(int pos) {
  boolean isSmall = currentCharState[pos].audio.track >= INDEX_OF_FIRST_LOWERCASE_LETTER;
  if (
    isSmall
    && wantedCharState[pos].audio.folder == currentCharState[pos].audio.folder
    && wantedCharState[pos].audio.track == wordRiddle_getMatchingUppercaseLetterTrackForLowercaseLetterTrack(currentCharState[pos].audio.track)) {
    return copyWordsCapCharMissing;
  }
  return {0, 0};
}

void wordScramble_handleFirstButtonPressed() {
  if (currentGameSubMode == WORD_SCRAMBLE_FINISHED && allCharPosEmpty()) {
    wordScramble_initGame(); // start new game round
  } else if (currentGameSubMode == WORD_SCRAMBLE_FINISHED && !allCharPosEmpty()) {
    // repeat evaluation and explain how to start a new game round
    forceUpdateAudioQueue(evaluationCorrectFanfare);
    addToAudioQueue(wordScrambleSolution);
    addToAudioQueue(wordTemplate.audio);

    int positiveFeedback = random(7, 11);
    addToAudioQueue({1, positiveFeedback});

    addToAudioQueue(wordScrambleHowToStartNewGameRound);
  } else if (currentGameSubMode == WORD_SCRAMBLE_PLACE_LETTERS) {
    forceUpdateAudioQueue(wordScrambleIntro);
    int nrOfBrailleChars = getNumberOfBrailleChars(wordTemplate);
    for (int i = 0; i < nrOfBrailleChars; i++) {
      addToAudioQueue(wantedCharState[i].audio);
    }
  } else if (currentGameSubMode == WORD_SCRAMBLE_SORT_LETTERS) {
    forceUpdateAudioQueue(wordScrambleLettersWerePlacedInCorrectOrder);
  }
}

boolean wordScramble_lettersSorted() {
  boolean result = true;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (currentCharState[i].audio.track != wordTemplate.brailleChars[i].audio.track) {
      result = false;
      break;
    }
  }
  if (currentGameSubMode == WORD_SCRAMBLE_SORT_LETTERS && result) {
    eval = true;
    addToAudioQueue(evaluationCorrectFanfare);
    addToAudioQueue(wordScrambleSolution);
    addToAudioQueue(wordTemplate.audio);

    int positiveFeedback = random(7, 11);
    addToAudioQueue({1, positiveFeedback});
    Serial.println("Word Scramble: Letters were sorted successful. Game finished.");
    printCurrentWordTemplate();
    printWantedCharState();
    printCurrentCharState();
  }
  return result;
}

boolean wordScramble_allLettersPlaced() {
  boolean result = true;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (currentCharState[i].audio.track != wantedCharState[i].audio.track) {
      result = false;
      break;
    }
  }
  if (currentGameSubMode == WORD_SCRAMBLE_PLACE_LETTERS && result) {
    addToAudioQueue(wordScrambleLettersWerePlacedInCorrectOrder);
    addToWordHistory(wordTemplate);
    Serial.println("Word Scramble: Letters were successfully placed.");
    printCurrentWordTemplate();
    printWantedCharState();
    printCurrentCharState();
  }
  return result;
}

void wordScramble_handleSubModeChange() {
  if (prevGameSubMode == WORD_SCRAMBLE_PLACE_LETTERS && currentGameSubMode == WORD_SCRAMBLE_SORT_LETTERS) {
    //update wanted state
    for (int i = 0; i < NR_OF_CHAR_POS; i++) {
      wantedCharState[i] = wordTemplate.brailleChars[i];
    }
  }
}
