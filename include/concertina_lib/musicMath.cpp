#include "musicMath.h"
#include <tables/cos2048_int8.h>
#include <tables/sin2048_int8.h> //Wavetables for Oscillators
#include <tables/saw2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <tables/triangle_valve_2_2048_int8.h>
#include <stdlib.h>
#include <Arduino.h>

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

extern const int8_t* getWaveFromInt(int i){
  if(i==1){
    return(SIN2048_DATA);
  }else if(i==2){
    return(COS2048_DATA);
  }else if(i==3){
    return(TRIANGLE2048_DATA);
  }else if(i==4){
    return(SAW2048_DATA);
  }else{
    return(SQUARE_NO_ALIAS_2048_DATA);
  }
}
