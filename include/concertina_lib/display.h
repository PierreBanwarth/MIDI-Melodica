#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "SSD1306AsciiWire.h"
#include "concertina_lib/concertina.h"
#include "concertina_lib/configuration.h"

  void test(SSD1306AsciiWire oled);
  void displayShift(uint8_t i, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayMainTitle(SSD1306AsciiWire oled);
  SSD1306AsciiWire displayAttack(Configuration conf, SSD1306AsciiWire oled);
  SSD1306AsciiWire printOscWave(Configuration conf, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayOscillatorChoice(uint8_t newPos, Configuration conf, SSD1306AsciiWire oled);

  SSD1306AsciiWire displayOctave(Configuration conf, uint8_t newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayHalfTone(Configuration conf, uint8_t newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayAttack(Configuration conf, uint8_t newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayPresetsMenu(uint8_t newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayMainMenu(uint8_t mode, uint8_t newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayMidiSettings(uint8_t mode, uint8_t newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displaySynthSettingsFirstMenu(uint8_t newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayMenuAttack(Configuration conf, uint8_t newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire printOscOct(Configuration conf, SSD1306AsciiWire oled);
  SSD1306AsciiWire printOctaveMenu(uint8_t newPos, Configuration conf, SSD1306AsciiWire oled);

  SSD1306AsciiWire displayState(
    Configuration conf,
    SSD1306AsciiWire oled
  );

  SSD1306AsciiWire displayOctOrWave(
    uint8_t menuActiveItem,
    uint8_t newPos,
    Configuration conf,
    SSD1306AsciiWire oled
  );

  String keyNames[23] = {
    "Sol#/Do#",
    "La/Re",
    "Sib/Re#",
    "Si/Mi",
    "Do/Fa",
    "Do#/Fa#",
    "Re/Sol",
    "Re#/Sol#",
    "Mi/La",
    "Fa/Sib",
    "Fa#/Si",
    "Sol/Do",
    "Sol#/Do#",
    "La/Re",
    "Sib/Re#",
    "Si/Mi",
    "Do/Fa",
    "Do#/Fa#",
    "Re/Sol",
    "Re#/Sol#",
    "Mi/La",
    "Fa/Sib",
    "Fa#/Si"
  };
  String waveFormNames[5] = {
    "Sin",
    "Cos",
    "Tri",
    "Saw",
    "Square",
  };

#endif
