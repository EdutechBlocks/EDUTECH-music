#ifndef MUSIC_H
#define MUSIC_H

#include "Arduino.h"

class MUSIC{
    public:
        void tone(String melody, int pin);
        void toneON();
        void toneOFF();
    private:
        int AMP_SPEAKER_PIN = 0;
        bool noTone=false;
        bool amp_isNum(byte value);
        bool delayAndCheckEnc(unsigned int delayTime);
        double amp_noteLengthToMS(unsigned int curNoteLength, byte tempo);
        String amp_noteName(byte note);
        void amp_playNote(byte note, int duration, String curNoteName);
};
#endif
