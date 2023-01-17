void worteNachlegen_initGame(Tag tag) {
  
  wordTemplate.audio = tag.audio;
  forceUpdateAudioQueue(wordTemplate.audio);
  addToAudioQueue({3, 0});
  freeWordTemplateAndWantedCharState();
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    wordTemplate.brailleChars[i].value = tag.values[i];
    wantedCharState[i].value = tag.values[i];
  }

  Serial.println("Spiel Worte Nachlegen gestartet ");
  printCurrentWordTemplate();
  printWantedCharState();
  printCurrentCharState();
  
}

boolean worteNachlegen_istBeendet() {
  boolean result = true;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    if (currentCharState[i].value != wantedCharState[i].value) {
      result = false;
      break;
    }
  }
  return result;
}

void worteNachlegen_handleSubModeChange() {
  if (currentGameSubMode == WORTE_NACHLEGEN_BEENDET) {
    worteNachlegen_handleBeendet(false);
  }
}

void worteNachlegen_handleBeendet(boolean forceFirstAudio) {
  if (forceFirstAudio) {
    forceUpdateAudioQueue({1, 4});
  } else {
    eval = true;
    addToAudioQueue({1, 4});
    Serial.println("Spiel Worte Nachlegen erfolgreich beendet ");
    printCurrentWordTemplate();
    printWantedCharState();
    printCurrentCharState();
  }
  addToAudioQueue(wordTemplate.audio);
  int positiveFeedback = random(7, 11);
  addToAudioQueue({1, positiveFeedback});
}

Audio worteNachlegen_calcCharAdditionalAudioFeedback(int pos) {
  boolean isSmall = currentCharState[pos].audio.track > 36;
  if (
    isSmall
    && wantedCharState[pos].value[0] == toupper(currentCharState[pos].value[0])) {
    return {1, 12}; // Das Großschreibzeichen fehlt;
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
