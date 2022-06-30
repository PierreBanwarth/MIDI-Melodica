#include <stdlib.h>
#include <Arduino.h>
#include <MIDI.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "RotaryEncoder.h"// RotaryEncoder v1.5.0 library by Matthais Hertel
#include "concertina_lib/concertina.h"
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
int shiftHalfTone = 0;
byte activeOsc1 = 1;
byte activeOsc2 = 1;
byte activeBrd1 = 1;
byte activeBrd2 = 1;

byte attackTheme = 200;
byte attackBourdon = 125;

int wichOsc = 0;
int globalFrequence;
int stateWave = 1;
int bourdonSynthActivated = 0;
int menuActiveItem = MAIN;

byte mode = MODE_MIDI;
byte mode_midi = DRUM;

int oscillator = 0;
static void displayShift(int i){
  if(i==-11){
    oled.print("Sol#/Do#");
  }else if(i == -10){
    oled.print("La/Re");
  }else if(i == -9){
    oled.print("Sib/Re#");
  }else if(i == -8){
    oled.print("Si/Mi");
  }else if(i == -7){
    oled.print("Do/Fa");
  }else if(i == -6){
    oled.print("Do#/Fa#");
  }else if(i == -5){
    oled.print("Re/Sol");
  }else if(i == -4){
    oled.print("Re#/Sol#");
  }else if(i == -3){
    oled.print("Mi/La");
  }else if(i == -2){
    oled.print("Fa/Sib");
  }else if(i == -1){
    oled.print("Fa#/Si");
  }else if(i == 0){
    oled.print("Sol/Do");
  }else if(i == 1){
    oled.print("Sol#/Do#");
  }else if(i == 2){
    oled.print("La/Re");
  }else if(i == 3){
    oled.print("Sib/Re#");
  }else if(i == 4){
    oled.print("Si/Mi");
  }else if(i == 5){
    oled.print("Do/Fa");
  }else if(i == 6){
    oled.print("Do#/Fa#");
  }else if(i == 7){
    oled.print("Re/Sol");
  }else if(i == 8){
    oled.print("Re#/Sol#");
  }else if(i == 9){
    oled.print("Mi/La");
  }else if(i == 10){
    oled.print("Fa/Sib");
  }else if(i == 11){
    oled.print("Fa#/Si");
  }
  oled.print(" ");
  oled.print(i);
}
static void displayOsc(int i){
  if(i == 1){
    oled.print("Sin");
  }else if(i == 2){
    oled.print("Tri");
  }else if(i == 3){
    oled.print("Saw");
  }else if(i == 4){
    oled.print("Squ");
  }
}
static void displayMainTitle(){
  oled.println("Melodica MIDI");
  oled.println("V 0.9.9.5");
}
static void displayAttack(){
  oled.print("VOsc: ");
  oled.print(attackTheme);
  oled.print(" VBrd: ");
  oled.println(attackBourdon);
}
static void printOscWave(){
  displayOsc(activeOsc1);
  oled.print(" ");
  displayOsc(activeOsc2);
  oled.print(" ");
  displayOsc(activeBrd1);
  oled.print(" ");
  displayOsc(activeBrd2);
  oled.println("");
}
static void printOscOct(){
  oled.print("Oct: ");
  oled.print(octaveOsc1);
  oled.print("   ");
  oled.print(octaveOsc2);
  oled.print("   ");
  oled.print(octaveBourdon1);
  oled.print("  ");
  oled.print(octaveBourdon2);
  oled.println("");

}
static void displayState(){
  displayMainTitle();
  oled.print("Oct: ");
  oled.print(octave);
  oled.print(" ");
  displayShift(shiftHalfTone);
  oled.println("");

  displayAttack();
  oled.println("  -------------------");
  oled.println("  Osc1 Osc2 Brd1 Brd2");
  printOscOct();
  printOscWave();
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
  uint8_t noteA = pousser[index] + shiftHalfTone;
  uint8_t noteB = tirer[index] + shiftHalfTone;
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
  uint8_t note = pousser[index]+shiftHalfTone;
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

  int noteSynthA = MatriceNote[pousserSynthNew[index]+shiftHalfTone];
  int noteSynthB = MatriceNote[tirerSynthNew[index]+shiftHalfTone];

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
  uint8_t noteA = MatriceNote[pousserSynthNew[index]+shiftHalfTone];
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
static void display(byte newPos, int menuActiveItem){
  oled.clear();

  if(menuActiveItem==OCTAVE){
    oled.print("Octave : ");
    oled.println(octave);

    oled.print("Oct : ");
    oled.print((octave+newPos)%6);
    oled.print(" ");
  }
  else if(menuActiveItem==HALFTONE){
    oled.print("Actual ");
    displayShift(shiftHalfTone);
    oled.println("");
    oled.print("New : ");
    displayShift(((shiftHalfTone+newPos)%23)-11);
    oled.print(" ");
  }else if(menuActiveItem == ATTACK_MAIN){
    oled.print("Volume Main : ");
    oled.println(attackTheme);
    oled.println(" ");
    oled.print("Volume : ");
    oled.print((attackTheme+5*newPos)%255);
    oled.print(" ");
  }else if(menuActiveItem == ATTACK_DRONE){
    oled.print("Volume Main : ");
    oled.println(attackBourdon);
    oled.println(" ");
    oled.print("Volume : ");
    oled.print((attackBourdon+5*newPos)%255);
    oled.print(" ");
  }else if(menuActiveItem == CHOOSE_OCTAVE){
    printOscOct();
    oled.println("");
    oled.print(newPos%6 -3);
    oled.print(" ");
  }else if(menuActiveItem == MAIN){
    displayMainTitle();
    oled.print("  Mode : ");
    if(mode == MODE_MIDI){
      oled.println("MIDI");

    }else if(mode == MODE_SYNTH){
      oled.println("Synth");
    }
    oled.println("  Octave");
    oled.println("  HalfTone");
    oled.println("  Synth setup");
    oled.println("  MIDI setup");
    oled.println("  State");
    oled.setCursor(0, (newPos%6)+2);
    oled.print(">");
  }
  else if(menuActiveItem == MIDI_SETTINGS){
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
  }
  else if(menuActiveItem == SYNTH_SETTINGS){
    oled.println("Synth");
    oled.println("");
    oled.println("  Waveform");
    oled.println("  Volume");
    oled.println("  Osc Oct");
    oled.println("  Back");
    oled.setCursor(0, (newPos%4)+2);
    oled.print(">");
  }else if(menuActiveItem == DISPLAY_STATE){
    displayState();
  }
  else if (menuActiveItem == MENU_ATTACK){
    oled.println("Volume :");
    displayAttack();
    oled.println("  Theme");
    oled.println("  Drone");
    oled.println("  Back");
    oled.setCursor(0, (newPos%3)+2);
    oled.print(">");
  }
  else if (menuActiveItem == OCT_OSC || menuActiveItem == WAVE)
  {
    if(menuActiveItem == OCT_OSC){
      oled.println("Oct osc :");
      printOscOct();

    }else{
      oled.println("Wave :");
      printOscWave();
    }
    oled.println("  Osc1 ");
    oled.println("  Osc2 ");
    oled.println("  Drone1");
    oled.println("  Drone2 ");
    oled.println("  All ");
    oled.println("  Back ");
    oled.setCursor(0, (newPos%6)+2);
    oled.print(">");

  }else if (menuActiveItem == OSCILLATOR)
  {
    oled.println("Wave Form :");
    printOscWave();
    oled.println("  Sin");
    oled.println("  Tri");
    oled.println("  Saw");
    oled.println("  Square");
    oled.println("  Back");
    oled.setCursor(0, (newPos%5)+2);
    oled.print(">");
  }
}
static int menuSelectorSwitch(int newPos, int menuActiveItem){

  if(menuActiveItem==OCTAVE){
    octave = (octave+newPos)%6;
    menuActiveItem = MAIN;
  }else if(menuActiveItem==HALFTONE){
    shiftHalfTone = ((shiftHalfTone+newPos)%23)-11;
    menuActiveItem = MAIN;
  }else if(menuActiveItem==ATTACK_MAIN){
    attackTheme = (attackTheme+5*newPos)%255;
    encoder.setPosition(0);
    menuActiveItem = MENU_ATTACK;
  }
  else if(menuActiveItem==ATTACK_DRONE){ 
    attackBourdon = (attackBourdon+5*newPos)%255;
    encoder.setPosition(0);
    menuActiveItem = MENU_ATTACK;
  }else if(menuActiveItem==DISPLAY_STATE){
    menuActiveItem = MAIN;
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
    }
    else if(newPos%6 == 2){
      menuActiveItem = HALFTONE;
    }
    else if(newPos%6 == 3){
      menuActiveItem = SYNTH_SETTINGS;
    }
    else if(newPos%6 == 4){
      menuActiveItem = MIDI_SETTINGS;
    }
    else if(newPos%6 == 5){
      menuActiveItem = DISPLAY_STATE;
    }
  }else if(menuActiveItem==MENU_ATTACK){
    if(newPos%3 == 0){
      menuActiveItem = ATTACK_MAIN;
    }
    else if(newPos%3 == 1){
      menuActiveItem = ATTACK_DRONE;
    }
    else if(newPos%3 == 2){
      menuActiveItem = SYNTH_SETTINGS;
    }
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
  }else if(menuActiveItem==CHOOSE_OCTAVE){
      int newOctave = (newPos%6)-3;
      setOscOct(wichOsc, newOctave);
      encoder.setPosition(0);
      menuActiveItem = OCT_OSC;

  }else if(menuActiveItem==OCT_OSC){
    if(newPos%6 == 5){
      menuActiveItem = SYNTH_SETTINGS;
    }else{
      menuActiveItem = CHOOSE_OCTAVE;
      wichOsc = (newPos%6)+1;
    }

  }else if (menuActiveItem == WAVE){
    if (newPos % 6 == 5)
    {
      menuActiveItem = SYNTH_SETTINGS;
    }else{
      menuActiveItem = OSCILLATOR;
      oscillator = (newPos % 6)+1;
    }
  }else if(menuActiveItem==OSCILLATOR){
    if(newPos%5 == 0){
      if(oscillator == 1){
        oscil1.setTable(SIN2048_DATA);
        activeOsc1 = 1;

      }else if(oscillator == 2){
        oscil2.setTable(SIN2048_DATA);
        activeOsc2 = 1;

      }else if(oscillator == 3){
        bourdon1.setTable(SIN2048_DATA);
        activeBrd1 = 1;

      }else if (oscillator == 4){
        bourdon2.setTable(SIN2048_DATA);
        activeBrd2 = 1;

      }else if (oscillator == 5){
        activeOsc1 = 1;activeOsc2 = 1;activeBrd1 = 1;activeBrd2 = 1;
        oscil1.setTable(SIN2048_DATA);
        oscil2.setTable(SIN2048_DATA);
        bourdon1.setTable(SIN2048_DATA);
        bourdon2.setTable(SIN2048_DATA);
      }
    }else if(newPos%5 == 1){
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
        activeOsc1 = 2;activeOsc2 = 2;activeBrd1 = 2;activeBrd2 = 2;

        oscil1.setTable(TRIANGLE2048_DATA);
        oscil2.setTable(TRIANGLE2048_DATA);
        bourdon1.setTable(TRIANGLE2048_DATA);
        bourdon2.setTable(TRIANGLE2048_DATA);
      }
    }else if(newPos%5 == 2){
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
    }else if(newPos%5 == 3){
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
    }else if(newPos%5 == 4){
      menuActiveItem = WAVE;
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
    display(newPos, menuActiveItem);
    encoderPosition = newPos;
  }
  if(state == LOW && state != stateButtonEncoder){
    menuActiveItem = menuSelectorSwitch(newPos, menuActiveItem);
    stateButtonEncoder = LOW;
    display(encoderPosition, menuActiveItem);
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

  display(0, menuActiveItem);
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
