#include "configuration.h"
#include <Arduino.h>


Configuration::Configuration(
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
)
{
  presetName = presetName;
  octave = octave;
  shiftHalfTone = shiftHalfTone;
  octaveOsc1 = octaveOsc1;
  octaveOsc2 = octaveOsc2;
  octaveBourdon1 = octaveBourdon1;
  octaveBourdon2 = octaveBourdon2;
  activeOsc1 = activeOsc1;
  activeOsc2 = activeOsc2;
  activeBrd1 = activeBrd1;
  activeBrd2 = activeBrd2;
}

void Configuration::setOctaveOsc(int a, int b, int c, int d){
  octaveOsc1 = a;
  octaveOsc2 = b;
  octaveBourdon1 = c;
  octaveBourdon2 = d;
}

void Configuration::setOscOct(int osc, int value){
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

int Configuration::getOsc1(){return activeOsc1;}

int Configuration::getOsc2(){return activeOsc2;}

int Configuration::getBrd1(){return activeBrd1;}

int Configuration::getBrd2(){return activeBrd2;}

void Configuration::setActiveOsc(int a, int b, int c, int d){
  activeOsc1 = a;
  activeOsc2 = a;
  activeBrd1 = c;
  activeBrd2 = d;
}
