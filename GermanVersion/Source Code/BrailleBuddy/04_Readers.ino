void initReader(int reader) {

  // for a quicker feedback or reaction we additionally check for button input here
  checkButtons();
  playAudio();
  
  if (reader != currentlyActiveReader) {
    disableReader(currentlyActiveReader);
    selectRstPin(reader);
    mfrc522[reader].PCD_Init(ssReaderPins[reader], rstReaderPin);
    delay(30);
    currentlyActiveReader = reader;
  }

  // and again
  checkButtons();
  playAudio();
}

void initReaders() {
  for (int reader = 0; reader < NR_OF_READERS; reader++) {
    initReader(reader);
    disableReader(reader);
  }
}

bool newTagPresent(int reader) {
  return mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial();
}

void selectRstPin(int reader) {
  digitalWrite(multiplexerPins[0], rstSelection[reader][0]);
  digitalWrite(multiplexerPins[1], rstSelection[reader][1]);
  digitalWrite(multiplexerPins[2], rstSelection[reader][2]);
  digitalWrite(multiplexerPins[3], rstSelection[reader][3]);
}

void disableReader(int reader) {
  mfrc522[reader].PCD_AntennaOff();
  digitalWrite(rstReaderPin, LOW);
}

Tag readTag(int reader) {

  Tag result;

  byte page = 6;
  String info = "                                        ";

  //read 10 pages a 4 bytes (buffer 18 because of library method implementation);
  for (byte i = 0; i < 10; i++) {
    byte byteCount = 18;
    byte buffer[18];

    MFRC522::StatusCode status = mfrc522[reader].MIFARE_Read(page, buffer, &byteCount);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(mfrc522[reader].GetStatusCodeName(status));
    }
    for (byte j = 0 ; j < 4; j++) {
      info.setCharAt(i * 4 + j, char(buffer[j]));
    }
    page += 1;
  }

  info.trim();

  char gameModeBuffer[8];
  getSubstring(0, info).toCharArray(gameModeBuffer, 8);
  int gameModeId = atoi(gameModeBuffer);

  char folderBuffer[8];
  getSubstring(1, info).toCharArray(folderBuffer, 8);
  int folder = atoi(folderBuffer);

  char trackBuffer[8];
  getSubstring(2, info).toCharArray(trackBuffer, 8);
  int track = atoi(trackBuffer);

  char occBuffer[8];
  getSubstring(3, info).toCharArray(occBuffer, 8);
  int occurance = atoi(occBuffer);

  String values[7];
  for (int i = 0; i < 7; i++) {
    values[i] = getSubstring(4 + i, info);
  }

  result.gameMode = getGameMode(gameModeId);
  result.audio.folder = folder;
  result.audio.track = track;
  result.occurance = occurance;
  for (int i = 0; i < NR_OF_CHAR_POS; i++) {
    result.values[i] = values[i];
  }
  return result;
}

BrailleChar readBrailleChar(int reader) {

  BrailleChar result;

  Tag letterTag = readTag(reader);

  result.value = letterTag.values[0];
  result.audio.track = letterTag.audio.track;
  result.audio.folder = letterTag.audio.folder;
  result.occurance = letterTag.occurance;

  return result;

}
