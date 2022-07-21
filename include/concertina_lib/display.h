#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "SSD1306AsciiWire.h"


class Display {
public:
  void displayShift(int i, SSD1306AsciiWire oled);
};
#endif
