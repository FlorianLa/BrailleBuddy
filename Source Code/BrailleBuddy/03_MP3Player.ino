void initDFPPlayer() {
  
  myDFPlayer.begin(mySoftwareSerial);  //Use softwareSerial to communicate with mp3
  myDFPlayer.setTimeOut(500);
  delay(1000);
  myDFPlayer.volume(27);  //Set volume value (0~30).
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  
}

void addToAudioQueue(Audio audio) {
  
  for (int i = 0; i < AUDIO_QUEUE_SIZE; i++) {
    if (audioQueue[i].folder == 0 && audioQueue[i].track == 0) {
      // if pos in queue is empty
      audioQueue[i] = audio;
      break;
    }
  }
  playAudio();

}

void playAudio() {

  if (!mp3PlayerBusy && audioQueue[0].folder != 0) {

    // depending on the audio source the time we have to wait differs
    int waitFor = 450;
    if (audioQueue[0].folder > 5) {
      waitFor = 650;
    }

    // play first track in audio queue
    Audio play = {audioQueue[0].folder, audioQueue[0].track};
    myDFPlayer.playFolder(play.folder, play.track); // play first track in queue
    delay(waitFor);

    // check if playing the audio source was triggered succesfully (LOW means playing)
    int checkIfPlayedSuccessfully = digitalRead(mp3PlayerBusyPin);
    int counter = 0;
    while (checkIfPlayedSuccessfully && counter < 3) { // if there is no track playing retry for up to 3 times
      myDFPlayer.playFolder(play.folder, play.track); // play first track in queue
      delay(waitFor);
      checkIfPlayedSuccessfully = digitalRead(mp3PlayerBusyPin);
      counter++;
    }
    mp3PlayerBusy = true;
    currentBusyValue = LOW; // make sure that braille buddy notices the update on short tracks
    updateAudioQueue();
  } else {
    currentBusyValue = digitalRead(mp3PlayerBusyPin);
  }
  if ( currentBusyValue != prevBusyValue ) {
    if ( currentBusyValue == HIGH ) {
      // stopped playing so start next track
      if (eval && audioQueue[0].folder == 0) {
        eval = false;
      }
      mp3PlayerBusy = false;
    }
    prevBusyValue = currentBusyValue;
  }
}

void forceUpdateAudioQueue(Audio audio) {
  myDFPlayer.pause();
  for (int i = 0; i < AUDIO_QUEUE_SIZE; i++) {
    audioQueue[i] = {0, 0};
  }
  audioQueue[0] = audio;
  playAudio();
}

void updateAudioQueue() {
  for (int i = 1; i < AUDIO_QUEUE_SIZE; i++) {
    audioQueue[i - 1] = audioQueue[i];
  }
  audioQueue[AUDIO_QUEUE_SIZE - 1] = {0};
}
