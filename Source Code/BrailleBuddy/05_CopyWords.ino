void copyWords_initGame(Tag tag) {
  
  wordTemplate.audio = tag.audio;
  forceUpdateAudioQueue(wordTemplate.audio);
  addToAudioQueue(copyWordsIntro);
  freeWordTemplateAndWantedCharState();
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    wordTemplate.brailleChars[i].value = tag.values[i];
    wantedCharState[i].value = tag.values[i];
  }

  Serial.println("A CopyWords Game Round started ");
  printCurrentWordTemplate();
  printWantedCharState();
  printCurrentCharState();
  
}

boolean copyWords_istBeendet() {
  boolean result = true;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (currentCharState[i].value != wantedCharState[i].value) {
      result = false;
      break;
    }
  }
  return result;
}

void copyWords_handleSubModeChange() {
  if (currentGameSubMode == COPY_WORD_FINISHED) {
    copyWords_handleBeendet(false);
  }
}

void copyWords_handleBeendet(boolean forceFirstAudio) {
  if (forceFirstAudio) {
    forceUpdateAudioQueue(evaluationCorrectFanfare);
  } else {
    eval = true;
    addToAudioQueue(evaluationCorrectFanfare);
    Serial.println("A round of CopyWord was finished successful ");
    printCurrentWordTemplate();
    printWantedCharState();
    printCurrentCharState();
  }
  addToAudioQueue(wordTemplate.audio);
  int positiveFeedback = random(7, 11);
  addToAudioQueue({1, positiveFeedback});
}

Audio copyWords_calcCharAdditionalAudioFeedback(int pos) {
  boolean isSmall = currentCharState[pos].audio.track >= INDEX_OF_FIRST_LOWERCASE_LETTER;
  if (
    isSmall
    && wantedCharState[pos].value[0] == toupper(currentCharState[pos].value[0])) {
    return copyWordsCapCharMissing;
  }
  return {0, 0};
}

boolean isNewWord(Tag newTag) {
  boolean result = false;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (wordTemplate.brailleChars[i].value != newTag.values[i]) {
      result = true;
      break;
    }
  }
  return result;
}
