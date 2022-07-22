#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#include "display.h"
#include "concertina_lib/concertina.h"

extern void displayShift(int i, SSD1306AsciiWire oled){
  oled.print(keyNames[i]);
}

extern void displayOsc(int i, SSD1306AsciiWire oled){
  oled.print(waveFormNames[i]);
}

extern SSD1306AsciiWire displayMainTitle(SSD1306AsciiWire oled){
  oled.println("Melodica MIDI");
  oled.println("V 1.0.0.0");
  return oled;
}
extern SSD1306AsciiWire displayAttack(int attackTheme, int attackBourdon, SSD1306AsciiWire oled){
  oled.print("VOsc: ");
  oled.print(attackTheme);
  oled.print(" VBrd: ");
  oled.println(attackBourdon);
  return oled;
}

extern SSD1306AsciiWire printOscWave(int activeOsc1, int activeOsc2, int activeBrd1, int activeBrd2, SSD1306AsciiWire oled){
  displayOsc(activeOsc1, oled);
  oled.print(" ");
  displayOsc(activeOsc2, oled);
  oled.print(" ");
  displayOsc(activeBrd1, oled);
  oled.print(" ");
  displayOsc(activeBrd2, oled);
  oled.println("");
  return oled;
}
extern SSD1306AsciiWire printOscOct(int octaveOsc1, int octaveOsc2, int octaveBourdon1, int octaveBourdon2, SSD1306AsciiWire oled){
  oled.print("Oct: ");
  oled.print(octaveOsc1);
  oled.print("   ");
  oled.print(octaveOsc2);
  oled.print("   ");
  oled.print(octaveBourdon1);
  oled.print("  ");
  oled.print(octaveBourdon2);
  oled.println("");
  return oled;
}
extern SSD1306AsciiWire displayOctave(int octave, int newPos, SSD1306AsciiWire oled){
  oled.print("Octave : ");
  oled.println(octave);
  oled.print("Oct : ");
  oled.println((newPos)%6);
  oled.print(" ");
  return oled;
}
extern SSD1306AsciiWire displayHalfTone(int shiftHalfTone, int newPos, SSD1306AsciiWire oled){
  oled.print("Actual ");
  displayShift(shiftHalfTone, oled);
  oled.println("");
  oled.print("New : ");
  displayShift(newPos, oled);
  oled.print(" ");
  return oled;
}
extern SSD1306AsciiWire displayAttackSwitch(int attack, int newPos, SSD1306AsciiWire oled){
  oled.print("Volume Main : ");
  oled.println(attack);
  oled.println(" ");
  oled.print("Volume : ");
  oled.print((attack+(5*newPos))%255);
  oled.print(" ");
  return oled;
}
extern SSD1306AsciiWire displayPresetsMenu(int newPos, SSD1306AsciiWire oled){
  for(int i=0; i<7; i++){
    oled.print(" ");
    oled.println(presetsNames[i]);
  }
  oled.println("  Back");
  oled.setCursor(0, (newPos%8));
  oled.print(">");
  return oled;
}

extern SSD1306AsciiWire displayMainMenu(int mode, int newPos, SSD1306AsciiWire oled){
  oled = displayMainTitle(oled);
  oled.print("  Mode : ");
  if(mode == MODE_MIDI){
    oled.println("MIDI");

  }else if(mode == MODE_SYNTH){
    oled.println("Synth");
  }
  oled.println("  Octave");
  oled.println("  HalfTone");
  if(mode == MODE_MIDI){
    oled.println("  MIDI setup");

  }else if(mode == MODE_SYNTH){
    oled.println("  SYNTH setup");
  }

  oled.println("  State");
  oled.println("  Presets");
  oled.setCursor(0, (newPos%6)+2);
  oled.print(">");
  return oled;
}
extern SSD1306AsciiWire displayMidiSettings(int mode_midi, int newPos, SSD1306AsciiWire oled){

  oled.println("MIDI settings");
  oled.println(" ");
  oled.print("  Mode : ");
  if(mode_midi == DRUM){
    oled.println("DRUM !!!");
  }else if(mode_midi == THEME){
    oled.println("Theme");
  }
  oled.println("  Back");
  oled.setCursor(0, (newPos%2)+2);
  oled.print(">");
  return oled;

}
extern SSD1306AsciiWire displayMenuAttack(int attackMain, int attackBourdon, int newPos, SSD1306AsciiWire oled){
  oled.println("Volume :");
  oled = displayAttack(attackMain, attackBourdon, oled);
  oled.println("  Theme");
  oled.println("  Drone");
  oled.println("  Back");
  oled.setCursor(0, (newPos%3)+2);
  oled.print(">");
  return oled;

}
extern SSD1306AsciiWire displaySynthSettingsFirstMenu(int newPos, SSD1306AsciiWire oled){
  oled.println("Synth");
  oled.println("");
  oled.println("  Waveform");
  oled.println("  Volume");
  oled.println("  Osc Oct");
  oled.println("  Back");
  oled.setCursor(0, (newPos%4)+2);
  oled.print(">");
  return oled;
}

extern SSD1306AsciiWire displayState(
  int octave,
  int shiftHalfTone,
  int attackTheme,
  int attackBourdon,
  int activeOsc1,
  int activeOsc2,
  int activeBrd1,
  int activeBrd2,
  int octaveOsc1,
  int octaveOsc2,
  int octaveBourdon1,
  int octaveBourdon2,
  SSD1306AsciiWire oled
){
  oled = displayMainTitle(oled);
  oled.print("Oct: ");
  oled.print(octave);
  oled.print(" ");
  displayShift(shiftHalfTone, oled);
  oled.println("");

  oled = displayAttack(attackTheme, attackBourdon, oled);
  oled.println("  -------------------");
  oled.println("  Osc1 Osc2 Brd1 Brd2");
  oled = printOscOct(octaveOsc1, octaveOsc2, octaveBourdon1, octaveBourdon2, oled);
  oled = printOscWave(activeOsc1, activeOsc2, activeBrd1, activeBrd2, oled);
  return oled;

}
SSD1306AsciiWire displayOscillatorChoice(int newPos, int activeOsc1, int activeOsc2, int activeBrd1, int activeBrd2, SSD1306AsciiWire oled){
  oled.println("Wave Form :");
  oled = printOscWave(activeOsc1, activeOsc2, activeBrd1, activeBrd2, oled);
  oled.println("  Sin");
  oled.println("  Cos");
  oled.println("  Tri");
  oled.println("  Saw");
  oled.println("  Square");
  oled.println("  Back");
  oled.setCursor(0, (newPos%6)+2);
  oled.print(">");
  return oled;
}


SSD1306AsciiWire displayOctOrWave(
  int menuActiveItem,
  int newPos,
  int octaveOsc1,
  int octaveOsc2,
  int octaveBourdon1,
  int octaveBourdon2,
  int activeOsc1,
  int activeOsc2,
  int activeBrd1,
  int activeBrd2,
  SSD1306AsciiWire oled
){
  if(menuActiveItem == OCT_OSC){
    oled.println("Oct osc :");
    oled = printOscOct(octaveOsc1, octaveOsc2, octaveBourdon1, octaveBourdon2, oled);
  }else{
    oled.println("Wave :");
    oled = printOscWave(activeOsc1, activeOsc2, activeBrd1, activeBrd2, oled);
  }
  oled.println("  Osc1 ");
  oled.println("  Osc2 ");
  oled.println("  Drone1");
  oled.println("  Drone2 ");
  oled.println("  All ");
  oled.println("  Back ");
  oled.setCursor(0, (newPos%6)+2);
  oled.print(">");
  return oled;




}
SSD1306AsciiWire printOctaveMenu(int newPos, int octaveOsc1, int octaveOsc2, int octaveBourdon1, int octaveBourdon2, SSD1306AsciiWire oled){
  oled = printOscOct(octaveOsc1, octaveOsc2, octaveBourdon1, octaveBourdon2, oled);
  oled.println("");
  oled.print(newPos%6 -3);
  oled.print(" ");
  return oled;
}
