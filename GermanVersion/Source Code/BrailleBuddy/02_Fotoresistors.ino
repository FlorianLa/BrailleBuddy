void readFotoValues() {
  for ( int i = 0; i < NR_OF_CHAR_POS; i++) {
    digitalWrite(multiplexerPins[0], mpFotoSelection[i][0]);
    digitalWrite(multiplexerPins[1], mpFotoSelection[i][1]);
    digitalWrite(multiplexerPins[2], mpFotoSelection[i][2]);
    digitalWrite(multiplexerPins[3], mpFotoSelection[i][3]);
    delay(30);
    int value = analogRead(multiplexerSignalPin);
    fotovalues[i] = value;
  }
}

void setInitialFotovalues() {
  int values[NR_OF_CHAR_POS][3] = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}};
  for(int j = 0; j < 3; j++) {
    for ( int i = 0; i < NR_OF_CHAR_POS; i++) {
      digitalWrite(multiplexerPins[0], mpFotoSelection[i][0]);
      digitalWrite(multiplexerPins[1], mpFotoSelection[i][1]);
      digitalWrite(multiplexerPins[2], mpFotoSelection[i][2]);
      digitalWrite(multiplexerPins[3], mpFotoSelection[i][3]);
      delay(30);
      int value = analogRead(multiplexerSignalPin);
      values[i][j] = value;
    }
  }
  for ( int i = 0; i < NR_OF_CHAR_POS; i++) {
      int value = (values[i][0] + values[i][1] + values[i][2]) / 3;
      initialFotovalues[i] = value;
    }
}

boolean isPosCovered(int pos) {
  // if the received value is under a certain threshold that is dependend on the initial value, the pos is covered
  return fotovalues[pos] <  initialFotovalues[pos] / 1.75;
}
