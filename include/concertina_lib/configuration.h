#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <Arduino.h>

class Configuration {       // The class
  public:             // Access specifier
    uint8_t octave;
    uint8_t shiftHalfTone;
    int8_t octaveOsc1;
    int8_t octaveOsc2;
    int8_t octaveBourdon1;
    int8_t octaveBourdon2;
    uint8_t activeOsc1;
    uint8_t activeOsc2;
    uint8_t activeBrd1;
    uint8_t activeBrd2;
    uint8_t attackTheme;
    uint8_t attackBourdon;
    String presetName;
    uint8_t getOsc1();
    uint8_t getOsc2();
    uint8_t getBrd1();
    uint8_t getBrd2();
    String getName();
    void setOscOct(uint8_t wich, uint8_t value);
    void setActiveOsc(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
    void setOctaveOsc(int8_t a, int8_t b, int8_t c, int8_t d);
    void setAllOsc(uint8_t a);

    Configuration(
      String s,
      uint8_t o,
      uint8_t shift,
      int8_t oct1,
      int8_t oct2,
      int8_t oct3,
      int8_t oct4,
      uint8_t osc1,
      uint8_t osc2,
      uint8_t brd1,
      uint8_t brd2,
      uint8_t attackT,
      uint8_t attackB
    );

};


#endif
