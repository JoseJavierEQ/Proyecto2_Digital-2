# include "pitches.h"

int melody[] = {
  NOTE_FS5, NOTE_FS5, NOTE_D5, NOTE_B4, NOTE_B4, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_GS5, NOTE_GS5, NOTE_A5, NOTE_B5,
  NOTE_A5, NOTE_A5, NOTE_A5, NOTE_E5, NOTE_D5, NOTE_FS5,
  NOTE_FS5, NOTE_FS5, NOTE_E5, NOTE_E5, NOTE_FS5, NOTE_E5
};

int durations[] = {
  8, 8, 8, 4, 4, 4,
  4, 5, 8, 8, 8, 8,
  8, 8, 8, 4, 4, 4,
  4, 5, 8, 8, 8, 8
};
int songLength = sizeof(melody) / sizeof(melody[0]);
int d;
void setup() {
  // iterate over the notes of the melody:
  pinMode (9, OUTPUT);
  pinMode (8, INPUT);
}

void loop() {
  d = digitalRead(8);
  while (d == 1) {
    for (int thisNote = 0; thisNote < songLength; thisNote++) {
      // determine the duration of the notes that the computer understands
      // divide 1000 by the value, so the first note lasts for 1000/8 milliseconds
      int duration = 1000 / durations[thisNote];
      tone(9, melody[thisNote], duration);
      // pause between notes
      int pause = duration * 1.3;
      delay(pause);
      // stop the tone
      noTone(8);
    }
    d = digitalRead(8);
  }
}
