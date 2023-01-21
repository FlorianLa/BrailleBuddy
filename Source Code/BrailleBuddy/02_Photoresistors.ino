void readPhotoValues() {
  for ( int i = 0; i < NR_OF_CHAR_POS; i++) {
    digitalWrite(multiplexerPins[0], mpPhotoSelection[i][0]);
    digitalWrite(multiplexerPins[1], mpPhotoSelection[i][1]);
    digitalWrite(multiplexerPins[2], mpPhotoSelection[i][2]);
    digitalWrite(multiplexerPins[3], mpPhotoSelection[i][3]);
    delay(30);
    int value = analogRead(multiplexerSignalPin);
    photovalues[i] = value;
  }
}

void setInitialPhotovalues() {
  int values[NR_OF_CHAR_POS][3] = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}};
  for(int j = 0; j < 3; j++) {
    for ( int i = 0; i < NR_OF_CHAR_POS; i++) {
      digitalWrite(multiplexerPins[0], mpPhotoSelection[i][0]);
      digitalWrite(multiplexerPins[1], mpPhotoSelection[i][1]);
      digitalWrite(multiplexerPins[2], mpPhotoSelection[i][2]);
      digitalWrite(multiplexerPins[3], mpPhotoSelection[i][3]);
      delay(30);
      int value = analogRead(multiplexerSignalPin);
      values[i][j] = value;
    }
  }
  for ( int i = 0; i < NR_OF_CHAR_POS; i++) {
      int value = (values[i][0] + values[i][1] + values[i][2]) / 3;
      initialPhotovalues[i] = value;
    }
}

boolean isPosCovered(int pos) {
  // if the received value is under a certain threshold that is dependend on the initial value, the pos is covered
  return photovalues[pos] <  initialPhotovalues[pos] / 1.75;
}
