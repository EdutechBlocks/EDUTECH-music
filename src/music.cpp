
#include "music.h"
#include "frequencia.h"

bool MUSIC::amp_isNum(byte value) {
  if (value >= 48 && value <= 57)                   // 0 to 9
    return true;
  else
    return false;
}

bool MUSIC::delayAndCheckEnc(unsigned int delayTime) {
  unsigned long startTime = millis();
  unsigned long checkTime = millis();
  int reSwitch = 0;

  while ((millis() - startTime) < delayTime) {
    if (millis() - checkTime > 50) {
      checkTime = millis();
      //reSwitch = readRotEncSwitch();
      if (reSwitch > 0) {
//        amp_buttonPressed = reSwitch;
        return true;
      }
    } else
      delay(1);
  }
  return false;
}

double MUSIC::amp_noteLengthToMS(unsigned int curNoteLength, byte tempo) {
  double timeBase = 60000 / tempo;           // @EB-todo default 4/4

  switch (curNoteLength) {
    case  1:  return (timeBase * 4);                // whole note
    case  2:  return (timeBase * 2);                // half
    case  4:  return (timeBase);                    // quarter
    case  8:  return (timeBase / 2);                // 8th
    case 16:  return (timeBase / 4);                // 16th
    case 32:  return (timeBase / 8);                // 32nd
    case 64:  return (timeBase / 16);               // 64th

    case  3:  return (timeBase * 3);                // dotted half
    case  6:  return (timeBase * 3 / 2);            // dotted 4th
    case 12:  return (timeBase * 3 / 4);            // dotted 8th
    case 24:  return (timeBase * 3 / 8);            // dotted 16th
    case 48:  return (timeBase * 3 / 16);           // dotted 32th

    case 34:  return (timeBase / 1.5);              // triplet quarter
    case 38:  return (timeBase / 3);                // triplet 8th
    case 316: return (timeBase / 6);                // triplet 16th
    case 332: return (timeBase / 12);               // triplet 32th
  }
}

String MUSIC::amp_noteName(byte note) {
  switch (note) {
    case  0:  return "C ";  break;
    case  1:  return "C#";  break;
    case  2:  return "D ";  break;
    case  3:  return "D#";  break;
    case  4:  return "E ";  break;
    case  5:  return "F ";  break;
    case  6:  return "F#";  break;
    case  7:  return "G ";  break;
    case  8:  return "G#";  break;
    case  9:  return "A ";  break;
    case 10:  return "A#";  break;
    case 11:  return "B ";  break;
  }
}

void MUSIC::amp_playNote(byte note, int duration, String curNoteName) {
//  if (amp_lyrics > 0) {   // @EB-todo
//    displayLyrics(curNoteName);
//  }

  #ifndef DEBUG_SILENCE
    if (note >= 0 && note <= 107) {
      if (pt_notes[note]) {
        #ifdef ESP32
          ledcWriteTone(0, pt_notes[note]);
        #else
          tone(AMP_SPEAKER_PIN, pt_notes[note]);
        #endif
      } else {
        #ifdef ESP32
          ledcWriteTone(0, 0);
          ledcWrite(0, LOW);
        #else
          noTone(AMP_SPEAKER_PIN);
        #endif
      }
    }
  #endif

//  delay(duration);

  delayAndCheckEnc(duration);              // @EB-todo
}

void MUSIC::toneON() {
    noTone=false;
//    amp_playNote(0, 0, "");
//    pinMode(AMP_SPEAKER_PIN, INPUT);                     // make sure the buzzer is silent ;-)
}

void MUSIC::toneOFF() {
    noTone=true;
}

void MUSIC::tone(String melody, int pin) {
  if(noTone != false) {
    return;
  }

  static byte tempo = 120;                          // default to 120 BPM
  static byte octave = 4;                           // default octave
  static unsigned int defaultNoteLength = 4;        // default to quarter note
  static unsigned int noteLengthType = 0;           // 0: normal 7/8, 1: legato 1/1, 2: staccato 3/4, 9: mute (default 0)
  static int transpose = 0;
  AMP_SPEAKER_PIN = pin;                      // default to 0

  byte notes[] = { 0, 2, 4, 5, 7, 9, 11 };

  byte curFunc = 0;
  byte curChar = 0;
  unsigned int curVal = 0;
  bool skip = false;

  unsigned int curNoteLength = 0;
  byte sharpFlat = 0;
  bool dotted = false;

  byte curNote = 0;
  double curNoteLengthMS = 0;
  String curNoteName;

  ledcAttachPin(AMP_SPEAKER_PIN, 0);


  for (unsigned int i = 0; i < melody.length(); i++) {
/*    if (amp_buttonPressed > 0)
      break;*/

    curNoteLength = 0;
    curVal = 0;
    skip = true;

    curChar = melody[i];

    if (curChar >= 97 && curChar <= 122) {          // convert lowercase to uppercase
      curChar -= 32;
    }

    switch (curChar) {
      case 32:                                      // skip spaces
        break;

      case '$':                                     // restore default settings
        tempo = 120;
        octave = 4;
        defaultNoteLength = 4;
        noteLengthType = 0;
        transpose = 0;
        break;

      case '<':                                     // < sign, octave lower
        octave--;
        if (octave < 1) octave = 1;
        break;

      case '>':                                     // > sign, octave higher
        octave++;
        if (octave > 8) octave = 8;
        break;

      case '[':                                     // [ transpose down
        transpose--;
        if (transpose < -12) transpose = -12;
        break;

      case ']':                                     // ] transpose up
        transpose++;
        if (transpose > 12) transpose = 12;
        break;

      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'R': case '#': case '-':
                                                    // A B C D E F G notes and R rest and # sharp and - flat
        skip = false;

        if (i + 1 < melody.length()) {
          if (amp_isNum(melody[i + 1])) {
            skip = true;
          }
          if (melody[i + 1] == '#') {               // sharp
            sharpFlat = 1;
            skip = true;
          } else if (melody[i + 1] == '-') {        // flat
            sharpFlat = -1;
            skip = true;
          } else if (melody[i + 1] == '.') {        // dotted
            dotted = true;
            i++;
          }
        }

        if (curChar != '#' && curChar != '-') {
          curFunc = curChar;
        }
        break;

      case 'M':                                     // music length type
        if (i + 1 < melody.length()) {
          switch  (melody[i + 1]) {
            case 'N': case 'n':
              noteLengthType = 0;                   // normal 7/8
              i++;
              break;
            case 'L': case 'l':
              noteLengthType = 1;                   // legato 1/1
              i++;
              break;
            case 'S': case 's':
              noteLengthType = 2;                   // staccato 3/4
              i++;
              break;
            case 'U': case 'u':
              noteLengthType = 9;                   // mute
              i++;
              break;
          }
        }

      case 'L':                                     // L default note/rest length
      case 'O':                                     // O octave
      case 'T':                                     // T tempo
        curFunc = curChar;
        break;

      default:
        if (amp_isNum(curChar)) {
          curVal = curChar - 48;

          for (int j = 0; j <= 2; j++) {             // look ahead to get the next 2 numbers or dot
            if (i + 1 < melody.length()) {
              if (amp_isNum(melody[i + 1])) {
                curVal = curVal * 10 + melody[i + 1] - 48;
                i++;
              } else if (melody[i + 1] == '.') {
                dotted = true;
                break;
              } else {
                break;
              }
            }
          }
          curNoteLength= curVal;
          skip = false;
        }
    }

    if (curFunc > 0 && !skip) {
      #ifdef DEBUG_PLAY_CMD
        DEBUGPRINT("Command " + (String) curFunc + " value " + fillSpace(curVal, 3));
      #endif

      if ((curFunc >= 65 and curFunc <= 71) || (curFunc == 82)) {
        if (!curNoteLength) {
          curNoteLength = defaultNoteLength;
        }

        if (dotted) {
          curNoteLength = curNoteLength * 1.5;
        }

        curNoteLengthMS = amp_noteLengthToMS(curNoteLength, tempo);

        if (curFunc == 82) {
          curNote = 0;
          curNoteName = "";

          #ifdef DEBUG_PLAY_CMD
            DEBUGPRINT(" Pause length "+ fillSpace(curNoteLength, 3) + " " + fillSpace(curNoteLengthMS, 4) + " ms");
          #endif

        } else {
          if (curFunc <= 66) {
            curNote = notes[curFunc - 60];
          } else {
            curNote = notes[curFunc - 67];
          }

          curNote = curNote + transpose + sharpFlat;
          curNoteName = amp_noteName(curNote);

          #ifdef DEBUG_PLAY_CMD
            DEBUGPRINT(" Octave " + (String) octave + " Note " + curNoteName);
          #endif

          curNote = (octave * 12) + curNote;
          #ifdef DEBUG_PLAY_CMD
            DEBUGPRINT(" Notenumber " + fillSpace(curNote, 3) + " Frequency " + fillSpace(pt_notes[curNote], 4) + " length "+ fillSpace(curNoteLength, 3) + " " + fillSpace(curNoteLengthMS, 4) + " ms");
          #endif
        }

        switch (noteLengthType) {
          case 0: // normal 7/8
            amp_playNote(curNote, (curNoteLengthMS / 8 * 7), curNoteName);
            amp_playNote(0, (curNoteLengthMS / 8 * 1), "");
            break;

          case 1: // legato 1/1
            amp_playNote(curNote, curNoteLengthMS, curNoteName);
            break;

          case 2: // staccato
            amp_playNote(curNote, (curNoteLengthMS / 4 * 3), curNoteName);
            amp_playNote(0, (curNoteLengthMS / 4 * 1), "");
            break;

          case 9: // mute
            amp_playNote(0, curNoteLengthMS, curNoteName);
            #ifdef DEBUG_PLAY_CMD
              DEBUGPRINT(" MUTE");
            #endif
            break;
        }

        dotted = false;
        curNoteLength = 0;
        sharpFlat = 0;

      } else {
        switch (curFunc) {
          case 'L':
            switch (curVal) {
              case 1: case 2: case 3: case 4: case 6: case 8: case 12: case 16: case 24: case 32: case 48: case 64:
              case 34: case 38: case 316: case 332:
                defaultNoteLength = curVal;
                break;
            }
            break;

          case 'O': octave = constrain(curVal, 1, 8); break;
          case 'T':
            tempo = constrain(curVal, 32, 255);
            #ifdef DEBUG_PLAY_CMD
              DEBUGPRINTLN(" Tempo " + (String) tempo);
            #endif
            break;
        }
      }

/*      if (amp_lyrics > 0) {
        matrix.fillScreen(LOW);
        matrix.write();
      }*/

      curFunc = 0;
      curNoteName = "";
    }

    if (!skip) {
      #ifdef DEBUG_PLAY_CMD
        DEBUGPRINTLN("");
      #endif
    }
  }
  toneON();
}
