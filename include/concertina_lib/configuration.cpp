#include "configuration.h"
#include <Arduino.h>


Configuration::Configuration(
  String s,
  int o,
  int shift,
  int oct1,
  int oct2,
  int oct3,
  int oct4,
  int osc1,
  int osc2,
  int brd1,
  int brd2
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
}
String Configuration::getName(){
  return presetName;
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

void Configuration::setAllOsc(int i){
  activeOsc1 = i;
  activeOsc2 = i;
  activeBrd1 = i;
  activeBrd2 = i;

}
