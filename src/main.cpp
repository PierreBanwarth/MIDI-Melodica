#include <stdlib.h>
#include <Arduino.h>
#include <MIDI.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "RotaryEncoder.h"// RotaryEncoder v1.5.0 library by Matthais Hertel

#include "concertina_lib/concertina.h"
#include "concertina_lib/display.h"
#include "concertina_lib/display.cpp"

#include <MozziGuts.h>
#include <Oscil.h>

#include <tables/cos2048_int8.h>
#include <tables/sin2048_int8.h> //Wavetables for Oscillators
#include <tables/saw2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <tables/triangle_valve_2_2048_int8.h>

#include <mozzi_rand.h>
#include <ADSR.h>

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
#define OPTIMIZE_I2C 1
// Define proper RST_PIN if required.
#define RST_PIN -1

ADSR <CONTROL_RATE, AUDIO_RATE> envelope; // notre enveloppe
ADSR <CONTROL_RATE, AUDIO_RATE> envelopeBourdon; // notre enveloppe
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscil1(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscil2(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> bourdon1(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> bourdon2(SIN2048_DATA);

SSD1306AsciiWire oled;

RotaryEncoder encoder(4, 5, RotaryEncoder::LatchMode::TWO03);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

byte bourdonActifSynth = 0;
byte bourdonPressed = 0;
byte onOffSynth = 0;
byte pousserTirer = 10;
byte buttonEncoder = 3;
byte stateButtonEncoder = HIGH;
int encoderPosition ;
byte oldStatePousserTirer = 0;

byte octave = 4;

int indexMenu = 0;
int octaveOsc1 = 0;
int octaveOsc2 = -1;
int octaveBourdon1 = -2;
int octaveBourdon2 = -3;
int8_t shiftHalfTone = 11;
byte activeOsc1 = 0;
byte activeOsc2 = 0;
byte activeBrd1 = 0;
byte activeBrd2 = 0;

byte attackTheme = 200;
byte attackBourdon = 200;

int wichOsc = 0;
int globalFrequence;
int stateWave = 1;
int bourdonSynthActivated = 0;
int menuActiveItem = MAIN;

byte mode = MODE_SYNTH;
byte mode_midi = DRUM;

int oscillator = 0;
static int getShift(int i){
  // 0 -> 0
  return (i%23)-11;
}

static void noteOn(int noteToPlay, int octave, int velocity){
  MIDI.sendNoteOn(noteToPlay + 12*octave, velocity, 1);
}
static void noteOff(int noteToShutdown, int octave){
  MIDI.sendNoteOff(noteToShutdown + 12*octave, 0, 1);
}
static void noteMidiDrum(uint8_t index, int velocity){
  uint8_t note = drum[index]+36;
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);
  if (bouton == LOW)
  {
    // on pousse sur le bouton et sur le soufflet
    if(oldStatePousser[index] == BUTTON_RELEASED){
      oldStatePousser[index] = BUTTON_PRESSED;
      noteOn(note, 0, 127);
    }

  }else{
    noteOff(note, 0);
    oldStatePousser[index] = BUTTON_RELEASED;
  }
}
static void noteMidi(uint8_t sens_soufflet, uint8_t index, uint8_t octave, int velocity){
   //, uint8_t oldStatePousser, uint8_t oldStateTirer
  // velocity = 127;
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);
  // digitalWrite(pinButton[index], LOW);
  uint8_t noteA = pousser[index] + getShift(shiftHalfTone);
  uint8_t noteB = tirer[index] + getShift(shiftHalfTone);
  if (pousser[index] != 0 || tirer[index] != 0)
  {
    // higher velocity usually makes MIDI instruments louder
    if (bouton == LOW)
    {
      if (sens_soufflet == LOW)
      {
        // on pousse sur le bouton et sur le soufflet
        if (oldStatePousser[index] == BUTTON_RELEASED)
        {
          noteOn(noteA, octave, velocity);
          oldStatePousser[index] = BUTTON_PRESSED;
          if (oldStateTirer[index] == BUTTON_PRESSED)
          {
            noteOff(noteB, octave);
            oldStateTirer[index] = BUTTON_RELEASED;
          }
        }
      }
      else
      { // on tire sur le soufflet et on appuie sur le bouton
        if (oldStateTirer[index] == 0)
        {

          noteOn(noteB, octave, velocity);
          oldStateTirer[index] = BUTTON_PRESSED;
          if (oldStatePousser[index] == BUTTON_PRESSED)
          {
            noteOff(noteA, octave);
            oldStatePousser[index] = BUTTON_RELEASED;
          }
        }
      }
    }
    else
    {
      if (oldStatePousser[index] == BUTTON_PRESSED)
      {
        noteOff(noteA, octave);
        oldStatePousser[index] = BUTTON_RELEASED;
      }
      if (oldStateTirer[index] == BUTTON_PRESSED)
      {
        // on appuie pas sur le bouton ma
        noteOff(noteB, octave);
        oldStateTirer[index] = BUTTON_RELEASED;
      }
    }
  }

}
static void noteMidiBourdon(uint8_t index, uint8_t octave, int velocity){
  //, uint8_t oldStatePousser, uint8_t oldStateTirer
  // velocity = 127;
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);
  // digitalWrite(pinButton[index], LOW);
  uint8_t note = pousser[index]+getShift(shiftHalfTone);
  if (bouton == LOW)
  {

    // on pousse sur le bouton et sur le soufflet
    if(oldStatePousser[index] == BUTTON_RELEASED){
      oldStatePousser[index] = BUTTON_PRESSED;
      if(bourdonActif[index] == 1){
        noteOff(note, octave);
        bourdonActif[index] = 0;
      }else{
        noteOn(note, octave, velocity/2);
        bourdonActif[index] = 1;
      }
    }

  }else{
    oldStatePousser[index] = BUTTON_RELEASED;
  }
}
static int getOctaveValueToMultiplyForOsc(int osc, int frequence, int octaveValue){
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
static void noteOnSynth(int frequence, int octave){
  oscil1.setFreq(getOctaveValueToMultiplyForOsc(octaveOsc1, frequence, octave));
  oscil2.setFreq(getOctaveValueToMultiplyForOsc(octaveOsc2, frequence, octave));
  envelope.noteOn(); // on joue la note
}
static void noteOnBourdonSynth(int frequence, int octave){
  bourdon1.setFreq(getOctaveValueToMultiplyForOsc(octaveBourdon1, frequence, octave));
  bourdon2.setFreq(getOctaveValueToMultiplyForOsc(octaveBourdon2, frequence, octave));
  envelopeBourdon.noteOn(); // on joue la note
}
static int noteSynth(uint8_t sens_soufflet, uint8_t index, int octave){
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);

  int noteSynthA = MatriceNote[pousserSynthNew[index]+getShift(shiftHalfTone)];
  int noteSynthB = MatriceNote[tirerSynthNew[index]+getShift(shiftHalfTone)];

  if (pousser[index] != 0 || tirer[index] != 0)
  {
    if (bouton == LOW)
    {
      if (sens_soufflet == LOW)
      {
        // on pousse sur le bouton et sur le soufflet
        if (oldStatePousser[index] == BUTTON_RELEASED)
        {
          // play note
          oldStatePousser[index] = BUTTON_PRESSED;
          if (oldStateTirer[index] == BUTTON_PRESSED)
          {
            oldStateTirer[index] = BUTTON_RELEASED;
          }
        }
        noteOnSynth(noteSynthA, octave);
      }
      else
      { // on tire sur le soufflet et on appuie sur le bouton
        if (oldStateTirer[index] == BUTTON_RELEASED)
        {
          oldStateTirer[index] = BUTTON_PRESSED;
          if (oldStatePousser[index] == BUTTON_PRESSED)
          {
            oldStatePousser[index] = BUTTON_RELEASED;
          }
        }
        noteOnSynth(noteSynthB, octave);
      }
      return 1;
    }
    else
    {
      if (oldStatePousser[index] == BUTTON_PRESSED)
      {
        oldStatePousser[index] = BUTTON_RELEASED;
      }
      if (oldStateTirer[index] == BUTTON_PRESSED)
      {
        // on appuie pas sur le bouton ma
        oldStateTirer[index] = BUTTON_RELEASED;
      }
      return 0;
    }
  }
  return 0;
}
static int noteSynthBourdon(uint8_t sens_soufflet, uint8_t index, uint8_t octave){
  //, uint8_t oldStatePousser, uint8_t oldStateTirer
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);
  uint8_t noteA = MatriceNote[pousserSynthNew[index]+getShift(shiftHalfTone)];
  // On
  if (bouton == LOW)
  {
    // Ancien etat = on a pas appuye
    if(oldStatePousser[index] == BUTTON_RELEASED){
        // le bourdon n'etait pas en jeu
      if(bourdonActif[index] == 0){

        noteOnBourdonSynth(noteA, octave);
        for(int index=0; index<36; index++){
          bourdonActif[index] = 0;
        }
        bourdonActif[index] = 1;
      }
      // le bourdon etait en jeu
      else{
        bourdonActif[index] = 0;
      }
    }
    oldStatePousser[index] = BUTTON_PRESSED;
  }
  // le bouton n'est pas pressé
  else{
    oldStatePousser[index] = BUTTON_RELEASED;
  }
  return bourdonActif[index];
}
static void bourdonSetup(){
  byte attack_level = attackBourdon;
  byte decay_level = 255;

  envelopeBourdon.setReleaseLevel(0);
  envelopeBourdon.setADLevels(attack_level,decay_level);
  envelopeBourdon.setTimes(0,0,0,0);
  envelopeBourdon.noteOn();
}
void updateControl(){
  byte attack_level = attackTheme;
  byte decay_level = 255;

  int attack = 0;
  int decay = 0;
  int release_ms = 0; // times for the attack, decay, and release of envelope
  int sustain  = 0; //sustain time set to zero so notes can be played as long as key is presssed without being effect by previus touch
  byte pousserTirerState = 0;
  envelope.setReleaseLevel(0);
  envelope.setADLevels(attack_level,decay_level);
  envelope.setTimes(attack,decay,sustain,release_ms);
  bourdonSetup();


  int onOffSynth = 0;
  int onOffBourdon = 0;
  pousserTirerState = digitalRead(pousserTirer);
  for (int i = 0; i < 36; i++)
  {
    if(pinButton[i] == 52 || pinButton[i] == 53 || pinButton[i] == 34 || pinButton[i] == 36 || pinButton[i] == 37 || pinButton[i] == 39 ){
      onOffBourdon += noteSynthBourdon(pousserTirerState, i, octave);
    }else{
      onOffSynth += noteSynth(pousserTirerState, i, octave);
    }
  }
  if(onOffSynth == 0){
    envelope.noteOff();
  }
  if(onOffBourdon == 0){
    envelopeBourdon.noteOff();
  }
}
int updateAudio(){
  long note = (long)envelope.next() * (oscil1.next()+oscil2.next());
  long noteBourdon =  (long)envelopeBourdon.next() * (bourdon1.next()+bourdon2.next());
  return (note + noteBourdon) >>9;
}
static void setOscOct(int osc, int value){

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

static void maindisplay(byte newPos, int menuActiveItem){
  oled.clear();
  if(menuActiveItem==OCTAVE){
    oled = displayOctave(octave, newPos, oled);
  }else if(menuActiveItem==HALFTONE){
    oled = displayHalfTone(shiftHalfTone, newPos, oled);
  }else if(menuActiveItem == ATTACK_MAIN){
    oled = displayAttackSwitch(attackTheme, newPos, oled);
  }else if(menuActiveItem == ATTACK_DRONE){
    oled = displayAttackSwitch(attackBourdon, newPos, oled);
  }else if(menuActiveItem == PRESETS){
    oled = displayPresetsMenu(newPos, oled);
  }else if(menuActiveItem == CHOOSE_OCTAVE){
    oled = printOctaveMenu(newPos,octaveOsc1, octaveOsc2, octaveBourdon1, octaveBourdon2, oled);
  }else if(menuActiveItem == MAIN){
    oled = displayMainMenu(mode, newPos, oled);
  }else if(menuActiveItem == MIDI_SETTINGS){
    oled = displayMidiSettings(mode_midi, newPos, oled);
  }else if(menuActiveItem == SYNTH_SETTINGS){
    oled = displaySynthSettingsFirstMenu(newPos, oled);
  }else if(menuActiveItem == DISPLAY_STATE){
    oled = displayState(
      octave,
      shiftHalfTone,
      attackTheme,
      attackBourdon,
      activeOsc1,
      activeOsc2,
      activeBrd1,
      activeBrd2,
      octaveOsc1,
      octaveOsc2,
      octaveBourdon1,
      octaveBourdon2,
      oled
    );
  }
  else if (menuActiveItem == MENU_ATTACK){
    oled = displayMenuAttack(attackTheme, attackBourdon, newPos, oled);
  }else if (menuActiveItem == OCT_OSC || menuActiveItem == WAVE){
    oled = displayOctOrWave(
      menuActiveItem,
      newPos,
      octaveOsc1,
      octaveOsc2,
      octaveBourdon1,
      octaveBourdon2,
      activeOsc1,
      activeOsc2,
      activeBrd1,
      activeBrd2,
      oled
    );
  }else if (menuActiveItem == OSCILLATOR){
    oled = displayOscillatorChoice(newPos, activeOsc1, activeOsc2, activeBrd1, activeBrd2, oled);
  }
}


static const int8_t* getWaveFromInt(int i){
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

static void setPresets(int i){
  oled.print(i);
  octave = presets[i][0];
  shiftHalfTone = presets[i][1];

  octaveOsc1 =  presets[i][2];
  octaveOsc2 =  presets[i][3];
  octaveBourdon1 =  presets[i][4];
  octaveBourdon2 =  presets[i][5];

  oscil1.setTable(getWaveFromInt(presets[i][6]));
  activeOsc1 = presets[i][6];

  oscil2.setTable(getWaveFromInt(presets[i][7]));
  activeOsc2 = presets[i][7];

  bourdon1.setTable(getWaveFromInt(presets[i][8]));
  activeBrd1 = presets[i][8];

  bourdon2.setTable(getWaveFromInt(presets[i][9]));
  activeBrd2 = presets[i][9];

}
static int menuSelectorSwitch(int newPos, int menuActiveItem){

  if(menuActiveItem==OCTAVE){
    octave = (newPos)%6;
    menuActiveItem = MAIN;
    encoder.setPosition(1);

  }else if(menuActiveItem==HALFTONE){
    shiftHalfTone = (newPos%23);
    menuActiveItem = MAIN;
    encoder.setPosition(2);

  }else if(menuActiveItem==ATTACK_MAIN){
    attackTheme = (attackTheme+(5*newPos))%255;
    menuActiveItem = MENU_ATTACK;
    encoder.setPosition(0);

  }
  else if(menuActiveItem==ATTACK_DRONE){
    attackBourdon = (attackBourdon+(5*newPos))%255;
    menuActiveItem = MENU_ATTACK;
    encoder.setPosition(1);

  }else if(menuActiveItem==DISPLAY_STATE){
    menuActiveItem = MAIN;
    encoder.setPosition(4);
  }else if(menuActiveItem == MIDI_SETTINGS){
    if(newPos%2 == 0){
      if(mode_midi == DRUM){
        mode_midi=THEME;
      }
      else if(mode_midi == THEME){
        mode_midi=DRUM;
      }
    }else if(newPos%2==1){
      menuActiveItem = MAIN;
    }

  }else if(menuActiveItem==MAIN){
    if(newPos%6 == 0){
      if(mode == MODE_SYNTH){
        mode=MODE_MIDI;
      }
      else if(mode == MODE_MIDI){
        mode=MODE_SYNTH;
      }
    }
    else if(newPos%6 == 1){
      menuActiveItem = OCTAVE;
      encoder.setPosition(octave);
    }
    else if(newPos%6 == 2){
      menuActiveItem = HALFTONE;
      encoder.setPosition(shiftHalfTone);
    }
    else if(newPos%6 == 3){
      if(mode == MODE_MIDI){
        menuActiveItem = MIDI_SETTINGS;
      }else if(mode == MODE_SYNTH){
        menuActiveItem = SYNTH_SETTINGS;
      }
      encoder.setPosition(0);

    }
    else if(newPos%6 == 4){
      menuActiveItem = DISPLAY_STATE;
    }
    else if(newPos%6 == 5){
      menuActiveItem = PRESETS;
      encoder.setPosition(0);
    }
  }else if(menuActiveItem==PRESETS){
      //if(newPos%6 == 0){

      if(newPos%8==7){
        menuActiveItem = MAIN;

      }
      else{
        setPresets(newPos);
      }
      encoder.setPosition(5);
  }
  else if(menuActiveItem==MENU_ATTACK){
    if(newPos%3 == 0){
      menuActiveItem = ATTACK_MAIN;
    }
    else if(newPos%3 == 1){
      menuActiveItem = ATTACK_DRONE;
    }
    else if(newPos%3 == 2){
      menuActiveItem = SYNTH_SETTINGS;
    }
    encoder.setPosition(1);
  }else if(menuActiveItem==SYNTH_SETTINGS){
    if(newPos%4 == 0){
      menuActiveItem = WAVE;
    }
    else if(newPos%4 == 1){
      menuActiveItem = MENU_ATTACK;
    }
    else if(newPos%4 == 2){
      menuActiveItem = OCT_OSC;
    }
    else if (newPos%4 == 3)
    {
      menuActiveItem = MAIN;
    }
    encoder.setPosition(0);
  }else if(menuActiveItem==CHOOSE_OCTAVE){
      int newOctave = (newPos%6)-3;
      setOscOct(wichOsc, newOctave);
      encoder.setPosition(wichOsc-1);
      menuActiveItem = OCT_OSC;

  }else if(menuActiveItem==OCT_OSC){
    if(newPos%6 == 5){
      menuActiveItem = SYNTH_SETTINGS;
      encoder.setPosition(2);
    }else{
      menuActiveItem = CHOOSE_OCTAVE;
      wichOsc = (newPos%6)+1;
    }

  }else if (menuActiveItem == WAVE){
    if (newPos % 6 == 5)
    {
      menuActiveItem = SYNTH_SETTINGS;
      encoder.setPosition(0);
    }else{
      menuActiveItem = OSCILLATOR;
      encoder.setPosition(0);
      oscillator = (newPos % 6)+1;
    }
  }else if(menuActiveItem==OSCILLATOR){
    if(newPos%6 == 0){
      if(oscillator == 1){
        oscil1.setTable(SIN2048_DATA);
        activeOsc1 = 0;

      }else if(oscillator == 2){
        oscil2.setTable(SIN2048_DATA);
        activeOsc2 = 0;

      }else if(oscillator == 3){
        bourdon1.setTable(SIN2048_DATA);
        activeBrd1 = 0;

      }else if (oscillator == 4){
        bourdon2.setTable(SIN2048_DATA);
        activeBrd2 = 0;

      }else if (oscillator == 5){
        activeOsc1 = 0;activeOsc2 = 0;activeBrd1 = 0;activeBrd2 = 0;
        oscil1.setTable(SIN2048_DATA);
        oscil2.setTable(SIN2048_DATA);
        bourdon1.setTable(SIN2048_DATA);
        bourdon2.setTable(SIN2048_DATA);
      }
    }else if(newPos%6 == 1){
      if (oscillator == 1){
        oscil1.setTable(COS2048_DATA);
        activeOsc1 = 1;
      }else if (oscillator == 2){
        oscil2.setTable(COS2048_DATA);
        activeOsc2 = 1;
      }else if(oscillator == 3){
        bourdon1.setTable(COS2048_DATA);
        activeBrd1 = 1;
      }else if (oscillator == 4){
        bourdon2.setTable(COS2048_DATA);
        activeBrd2 = 1;
      }else if (oscillator == 5){
        activeOsc1 = 1;activeOsc2 = 1;activeBrd1 = 1;activeBrd2 = 1;

        oscil1.setTable(COS2048_DATA);
        oscil2.setTable(COS2048_DATA);
        bourdon1.setTable(COS2048_DATA);
        bourdon2.setTable(COS2048_DATA);
      }
    }else if(newPos%6 == 2){
      if (oscillator == 1){
        oscil1.setTable(TRIANGLE2048_DATA);
        activeOsc1 = 2;
      }else if (oscillator == 2){
        oscil2.setTable(TRIANGLE2048_DATA);
        activeOsc2 = 2;
      }else if(oscillator == 3){
        bourdon1.setTable(TRIANGLE2048_DATA);
        activeBrd1 = 2;
      }else if (oscillator == 4){
        bourdon2.setTable(TRIANGLE2048_DATA);
        activeBrd2 = 2;
      }else if (oscillator == 5){
        activeOsc1 = 2;
        activeOsc2 = 2;
        activeBrd1 = 2;
        activeBrd2 = 2;

        oscil1.setTable(TRIANGLE2048_DATA);
        oscil2.setTable(TRIANGLE2048_DATA);
        bourdon1.setTable(TRIANGLE2048_DATA);
        bourdon2.setTable(TRIANGLE2048_DATA);
      }
    }else if(newPos%6 == 3){
      if (oscillator == 1){
        oscil1.setTable(SAW2048_DATA);
        activeOsc1 = 3;
      }else if (oscillator == 2){
        activeOsc2 = 3;
        oscil2.setTable(SAW2048_DATA);
      }else if(oscillator == 3){
        activeBrd1 = 3;
        bourdon1.setTable(SAW2048_DATA);
      }else if (oscillator == 4){
        activeBrd2 = 3;
        bourdon2.setTable(SAW2048_DATA);
      }else if (oscillator == 5){
        activeOsc1 = 3;activeOsc2 = 3;activeBrd1 = 3;activeBrd2 = 3;
        oscil1.setTable(SAW2048_DATA);
        oscil2.setTable(SAW2048_DATA);
        bourdon1.setTable(SAW2048_DATA);
        bourdon2.setTable(SAW2048_DATA);
      }
    }else if(newPos%6 == 4){
      if (oscillator == 1)        {
        oscil1.setTable(SQUARE_NO_ALIAS_2048_DATA);
        activeOsc1=4;
      }else if (oscillator == 2){
        oscil2.setTable(SQUARE_NO_ALIAS_2048_DATA);
        activeOsc2=4;

      }else if(oscillator == 3){
        bourdon1.setTable(SQUARE_NO_ALIAS_2048_DATA);
        activeBrd1=4;

      }else if (oscillator == 4){
        bourdon2.setTable(SQUARE_NO_ALIAS_2048_DATA);
        activeBrd2=4;

      }else if (oscillator == 5){
        activeOsc1=4;activeOsc2=4;activeBrd1=4;activeBrd2=4;
        oscil1.setTable(SQUARE_NO_ALIAS_2048_DATA);
        oscil2.setTable(SQUARE_NO_ALIAS_2048_DATA);
        bourdon1.setTable(SQUARE_NO_ALIAS_2048_DATA);
        bourdon2.setTable(SQUARE_NO_ALIAS_2048_DATA);
      }
    }else if(newPos%6 == 5){
      menuActiveItem = WAVE;
      encoder.setPosition(oscillator-1);
      oscillator = 0;
    }
  }
  return menuActiveItem;
}
static void menuSelector(){
  encoder.tick();
  int newPos = encoder.getPosition();
  int state = digitalRead(buttonEncoder);
  if (encoderPosition != newPos)
  {
    maindisplay(newPos, menuActiveItem);
    encoderPosition = newPos;
  }
  if(state == LOW && state != stateButtonEncoder){
    menuActiveItem = menuSelectorSwitch(newPos, menuActiveItem);
    stateButtonEncoder = LOW;
    maindisplay(encoderPosition, menuActiveItem);
  }
  stateButtonEncoder = state;
}
void setup() {
  startMozzi();
  pinMode(pousserTirer, INPUT);
  digitalWrite(pousserTirer, HIGH);

  pinMode(buttonEncoder, INPUT);
  digitalWrite(buttonEncoder, HIGH);
  encoderPosition = encoder.getPosition();
  // for testing function
  Wire.begin();
  Wire.setClock(400000L);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
  oled.displayRemap(false);

  MIDI.begin(1);
  Serial.begin(115200);
  for (size_t pin = 0; pin < 36; pin++) {
    pinMode(pinButton[pin],INPUT);
    digitalWrite(pinButton[pin],HIGH);
  }

  maindisplay(0, menuActiveItem);
}
void loop() {

  byte pousserTirerState;
  indexMenu++;
  if(indexMenu==10){
    menuSelector();
    indexMenu = 0;
  }
  audioHook();
  if(mode == MODE_MIDI){
    for (byte i = 0; i < 36; i++){
      menuSelector();
      if(mode_midi == DRUM){
        noteMidiDrum(i, 127);
      }else{
        pousserTirerState = digitalRead(pousserTirer);
        if(pinButton[i] == 52 || pinButton[i] == 53 || pinButton[i] == 34 || pinButton[i] == 36 || pinButton[i] == 37 || pinButton[i] == 39 ){
          noteMidiBourdon(i, octave, 127);
        }else{
          noteMidi(pousserTirerState, i, octave, 127);
        }
      }
    }
  }
}
