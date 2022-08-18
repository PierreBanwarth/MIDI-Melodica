#ifndef __CONCERTINA_H__
#define __CONCERTINA_H__
#include <stdlib.h>
#include <Arduino.h>

  int getOctaveValueToMultiplyForOsc(int osc, int frequence, int octaveValue);
  int getShift(int i);
  const int8_t* getWaveFromInt(int i);

#endif
