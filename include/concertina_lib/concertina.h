#ifndef __CONCERTINA_H__
#define __CONCERTINA_H__
#include "Arduino.h"
#include <MIDI.h>
#include <Tone.h>

enum
{
    // DEFINING MIDI notes
    Cn1 = 0,
    Cs1 = 1,
    Dn1 = 2,
    Ds1 = 3,
    En1 = 4,
    Fn1 = 5,
    Fs1 = 6,
    Gn1 = 7,
    Gs1 = 8,
    An1 = 9,
    As1 = 10,
    Bn1 = 11,

    Cn2 = 0 + 12,
    Cs2 = 1 + 12,
    Dn2 = 2 + 12,
    Ds2 = 3 + 12,
    En2 = 4 + 12,
    Fn2 = 5 + 12,
    Fs2 = 6 + 12,
    Gn2 = 7 + 12,
    Gs2 = 8 + 12,
    An2 = 9 + 12,
    As2 = 10 + 12,
    Bn2 = 11 + 12,

    Cn3 = 0 + 24,
    Cs3 = 1 + 24,
    Dn3 = 2 + 24,
    Ds3 = 3 + 24,
    En3 = 4 + 24,
    Fn3 = 5 + 24,
    Fs3 = 6 + 24,
    Gn3 = 7 + 24,
    Gs3 = 8 + 24,
    An3 = 9 + 24,
    As3 = 10 + 24,
    Bn3 = 11 + 24,

    Cn4 = 0 + 36,
    Cs4 = 1 + 36,
    Dn4 = 2 + 36,
    Ds4 = 3 + 36,
    En4 = 4 + 36,
    Fn4 = 5 + 36,
    Fs4 = 6 + 36,
    Gn4 = 7 + 36,
    Gs4 = 8 + 36,
    An4 = 9 + 36,
    As4 = 10 + 36,
    Bn4 = 11 + 36,

    Eb1 = As1,
    Bb1 = Ds1,
    Eb2 = As2,
    Bb2 = Ds2,
    Eb3 = As3,
    Bb3 = Ds3,
    Eb4 = As4,
    Bb4 = Ds4,

};


uint8_t oldStatePousser[] = {
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
};
uint8_t oldStateTirer[] = {
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
};

uint8_t bourdonActif[] = {
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
};

int pinButton[] = {
  6,7,8,9,22,23,
  24,25,26,27,28,29,
  30,31,32,33,34,35,
  36,37,38,39,40,41,
  42,43,44,45,46,47,
  48,49,50,51,52,53
};

int pousser[] = {
  Gn2, An2, Cs3, Cn3, Gs2, Bn2,
  En2, Gn2, Cn2, Cs2, Gn2, Dn2,
  Bn1, An1, En1, Cn1, Dn1, Dn3,
  Dn1, Dn1, Fn4, An1, Bn4, En4,
  Gn4, Cn4, Cs4, Dn4, Gn3, Fs3,
  Bn3, En3, An3, Gn3, An1, Dn1,
};

int tirer[] = {
  An2, Gn2, Ds3, Bn2, Bb2, Cn3,
  Fn2, An2, Dn2, Fs2, Bn1, Fs2,
  Dn2, Bb1, Fn1, Gn1, Dn1, En3,
  Dn1, Dn1, An4, An1, Fs4, Bn3,
  En4, An3, Ds4, Cn4, Fn3, Bb3,
  An3, Dn3, Gn3, Fs3, An1, Dn1
};
int pousserSynth[] = {
  NOTE_G2, NOTE_A2, NOTE_CS3, NOTE_C3, NOTE_GS2, NOTE_B2,
  NOTE_E2, NOTE_G2, NOTE_C2, NOTE_CS2, NOTE_G2, NOTE_D2,
  NOTE_B1, NOTE_A1, NOTE_E1, NOTE_C1, NOTE_D1, NOTE_D3,
  NOTE_D1, NOTE_D1, NOTE_F4, NOTE_A1, NOTE_B4, NOTE_E4,
  NOTE_G4, NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_G3, NOTE_FS3,
  NOTE_B3, NOTE_E3, NOTE_A3, NOTE_G3, NOTE_D1, NOTE_D1,
};

int tirerSynth[] = {
  NOTE_A2, NOTE_G2, NOTE_DS3, NOTE_B2, NOTE_AS2, NOTE_C3,
  NOTE_F2, NOTE_A2, NOTE_D2, NOTE_FS2, NOTE_B1, NOTE_FS2,
  NOTE_D2, NOTE_AS1, NOTE_F1, NOTE_G1, NOTE_D1, NOTE_E3,
  NOTE_D1, NOTE_D1, NOTE_A4, NOTE_A1, NOTE_FS4, NOTE_B3,
  NOTE_E4, NOTE_A3, NOTE_DS4, NOTE_C4, NOTE_F3, NOTE_AS3,
  NOTE_A3, NOTE_D3, NOTE_G3, NOTE_FS3, NOTE_D1, NOTE_D1
};
#endif
