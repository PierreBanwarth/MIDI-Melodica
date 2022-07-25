#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <Arduino.h>

class Configuration {       // The class
  public:             // Access specifier
    int octave;
    int shiftHalfTone;
    int octaveOsc1;
    int octaveOsc2;
    int octaveBourdon1;
    int octaveBourdon2;
    int activeOsc1;
    int activeOsc2;
    int activeBrd1;
    int activeBrd2;
    String presetName;
    int getOsc1();
    int getOsc2();
    int getBrd1();
    int getBrd2();
    String getName();
    void setOscOct(int wich, int value);
    void setActiveOsc(int a, int b, int c, int d);
    void setOctaveOsc(int a, int b, int c, int d);
    void setAllOsc(int a);

    Configuration(
      String presetName,
      int octave,
      int shiftHalfTone,
      int octaveOsc1,
      int octaveOsc2,
      int octaveBourdon1,
      int octaveBourdon2,
      int activeOsc1,
      int activeOsc2,
      int activeBrd1,
      int activeBrd2
    );

};


#endif
