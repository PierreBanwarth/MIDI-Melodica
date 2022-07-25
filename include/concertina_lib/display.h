#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "SSD1306AsciiWire.h"
#include "concertina_lib/concertina.h"
#include "concertina_lib/configuration.h"

  void test(SSD1306AsciiWire oled);
  void displayShift(int i, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayMainTitle(SSD1306AsciiWire oled);
  SSD1306AsciiWire displayAttack(int attackTheme, int attackBourdon, SSD1306AsciiWire oled);
  SSD1306AsciiWire printOscWave(Configuration conf, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayOscillatorChoice(int newPos, Configuration conf, SSD1306AsciiWire oled);

  SSD1306AsciiWire displayOctave(Configuration conf, int newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayHalfTone(Configuration conf, int newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayAttack(int attack, int newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayPresetsMenu(int newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayMainMenu(int mode, int newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayMidiSettings(int mode, int newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displaySynthSettingsFirstMenu(int newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire displayMenuAttack(int attackMain, int attackBourdon, int newPos, SSD1306AsciiWire oled);
  SSD1306AsciiWire printOscOct(Configuration conf, SSD1306AsciiWire oled);
  SSD1306AsciiWire printOctaveMenu(int newPos, Configuration conf, SSD1306AsciiWire oled);

  SSD1306AsciiWire displayState(
    Configuration conf,
    int attackTheme,
    int attackBourdon,
    SSD1306AsciiWire oled
  );

  SSD1306AsciiWire displayOctOrWave(
    int menuActiveItem,
    int newPos,
    Configuration conf,
    SSD1306AsciiWire oled
  );

  const char *keyNames[23] = {
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
  const char *waveFormNames[5] = {
    "Sin",
    "Cos",
    "Tri",
    "Saw",
    "Square",
  };

#endif
