#include "musicMath.h"

extern int getOctaveValueToMultiplyForOsc(int osc, int frequence, int octaveValue){
  int freq = frequence;
  int finalOctave = osc+octaveValue;
  if(finalOctave == -3){
    freq = frequence>>1;
  }else if(finalOctave == -2){
    freq = frequence>>1;
  }else if(finalOctave == -1){
    freq = frequence>>1;
  }else if(finalOctave == 0){
    freq = frequence;
  }else if(finalOctave ==1){
    freq = frequence<<1;
  }else if(finalOctave == 3){
    freq = frequence<<2;
  }else if(finalOctave == 4){
    freq = frequence<<3;
  }else if(finalOctave == 5){
    freq = frequence<<4;
  }else if(finalOctave == 6){
    freq = frequence<<5;
  }else if(finalOctave == 7){
    freq = frequence<<6;
  }
  return freq;
}
extern int getShift(int i){
  // 0 -> 0
  return (i%23)-11;
}
