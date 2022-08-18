#include "configuration.h"
#include <Arduino.h>


Configuration::Configuration(
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
)
{
  presetName = s;
  octave = o;
  shiftHalfTone = shift;
  octaveOsc1 = oct1;
  octaveOsc2 = oct2;
  octaveBourdon1 = oct3;
  octaveBourdon2 = oct4;
  activeOsc1 = osc1;
  activeOsc2 = osc2;
  activeBrd1 = brd1;
  activeBrd2 = brd2;
  attackTheme = attackT;
  attackBourdon = attackB;
}
String Configuration::getName(){
  return presetName;
}

void Configuration::setOctaveOsc(int8_t a, int8_t b, int8_t c, int8_t d){
  octaveOsc1 = a;
  octaveOsc2 = b;
  octaveBourdon1 = c;
  octaveBourdon2 = d;
}

void Configuration::setOscOct(uint8_t osc, uint8_t value){
  if(osc == 1){
    octaveOsc1 = value;
  }else if(osc == 2){
    octaveOsc2 = value;
  }else if(osc == 3){
    octaveBourdon1 = value;
  }else if(osc == 4){
    octaveBourdon2 = value;
  }else if(osc == 5){
    octaveOsc1 = value;
    octaveOsc2 = value;
    octaveBourdon1 = value;
    octaveBourdon2 = value;
  }
}

uint8_t Configuration::getOsc1(){return activeOsc1;}

uint8_t Configuration::getOsc2(){return activeOsc2;}

uint8_t Configuration::getBrd1(){return activeBrd1;}

uint8_t Configuration::getBrd2(){return activeBrd2;}

void Configuration::setActiveOsc(uint8_t a, uint8_t b, uint8_t c, uint8_t d){
  activeOsc1 = a;
  activeOsc2 = a;
  activeBrd1 = c;
  activeBrd2 = d;
}

void Configuration::setAllOsc(uint8_t i){
  activeOsc1 = i;
  activeOsc2 = i;
  activeBrd1 = i;
  activeBrd2 = i;

}
