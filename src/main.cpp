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
#include "concertina_lib/configuration.cpp"
#include "concertina_lib/configuration.h"
#include "concertina_lib/musicMath.h"
#include "concertina_lib/musicMath.cpp"

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
int indexMenu = 0;

Configuration conf = Configuration("Defaut", 4, 11, 0,-1,-2,-3, 0,0,0,0, 200, 200);


int wichOsc = 0;
int globalFrequence;
int stateWave = 1;
int bourdonSynthActivated = 0;
int menuActiveItem = MAIN;

byte mode = MODE_SYNTH;
byte mode_midi = DRUM;

int oscillator = 0;




// technical stuff to play put everything in a library
static void noteOn(int noteToPlay, Configuration conf, int velocity){
  MIDI.sendNoteOn(noteToPlay + 12*conf.octave, velocity, 1);
}
static void noteOff(int noteToShutdown, Configuration conf){
  MIDI.sendNoteOff(noteToShutdown + 12*conf.octave, 0, 1);
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
      noteOn(note, conf, 127);
    }

  }else{
    noteOff(note, conf);
    oldStatePousser[index] = BUTTON_RELEASED;
  }
}
static void noteMidi(uint8_t sens_soufflet, uint8_t index, Configuration conf, int velocity){
   //, uint8_t oldStatePousser, uint8_t oldStateTirer
  // velocity = 127;
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);
  // digitalWrite(pinButton[index], LOW);
  uint8_t noteA = pousser[index] + getShift(conf.shiftHalfTone);
  uint8_t noteB = tirer[index] + getShift(conf.shiftHalfTone);
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
          noteOn(noteA, conf, velocity);
          oldStatePousser[index] = BUTTON_PRESSED;
          if (oldStateTirer[index] == BUTTON_PRESSED)
          {
            noteOff(noteB, conf);
            oldStateTirer[index] = BUTTON_RELEASED;
          }
        }
      }
      else
      { // on tire sur le soufflet et on appuie sur le bouton
        if (oldStateTirer[index] == 0)
        {

          noteOn(noteB, conf, velocity);
          oldStateTirer[index] = BUTTON_PRESSED;
          if (oldStatePousser[index] == BUTTON_PRESSED)
          {
            noteOff(noteA, conf);
            oldStatePousser[index] = BUTTON_RELEASED;
          }
        }
      }
    }
    else
    {
      if (oldStatePousser[index] == BUTTON_PRESSED)
      {
        noteOff(noteA, conf);
        oldStatePousser[index] = BUTTON_RELEASED;
      }
      if (oldStateTirer[index] == BUTTON_PRESSED)
      {
        // on appuie pas sur le bouton ma
        noteOff(noteB, conf);
        oldStateTirer[index] = BUTTON_RELEASED;
      }
    }
  }

}
static void noteMidiBourdon(uint8_t index, Configuration conf, int velocity){
  //, uint8_t oldStatePousser, uint8_t oldStateTirer
  // velocity = 127;
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);
  // digitalWrite(pinButton[index], LOW);
  uint8_t note = pousser[index]+getShift(conf.shiftHalfTone);
  if (bouton == LOW)
  {

    // on pousse sur le bouton et sur le soufflet
    if(oldStatePousser[index] == BUTTON_RELEASED){
      oldStatePousser[index] = BUTTON_PRESSED;
      if(bourdonActif[index] == 1){
        noteOff(note, conf);
        bourdonActif[index] = 0;
      }else{
        noteOn(note, conf, velocity/2);
        bourdonActif[index] = 1;
      }
    }

  }else{
    oldStatePousser[index] = BUTTON_RELEASED;
  }
}
static void noteOnSynth(int frequence, Configuration conf){
  oscil1.setFreq(getOctaveValueToMultiplyForOsc(conf.octaveOsc1, frequence, conf.octave));
  oscil2.setFreq(getOctaveValueToMultiplyForOsc(conf.octaveOsc2, frequence, conf.octave));
  envelope.noteOn(); // on joue la note
}
static void noteOnBourdonSynth(int frequence, Configuration conf){
  bourdon1.setFreq(getOctaveValueToMultiplyForOsc(conf.octaveBourdon1, frequence, conf.octave));
  bourdon2.setFreq(getOctaveValueToMultiplyForOsc(conf.octaveBourdon2, frequence, conf.octave));
  envelopeBourdon.noteOn(); // on joue la note
}
static int noteSynth(uint8_t sens_soufflet, uint8_t index, Configuration conf){
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);

  int noteSynthA = MatriceNote[pousserSynthNew[index]+getShift(conf.shiftHalfTone)];
  int noteSynthB = MatriceNote[tirerSynthNew[index]+getShift(conf.shiftHalfTone)];

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
        noteOnSynth(noteSynthA, conf);
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
        noteOnSynth(noteSynthB, conf);
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
static int noteSynthBourdon(uint8_t sens_soufflet, uint8_t index, Configuration conf){
  //, uint8_t oldStatePousser, uint8_t oldStateTirer
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);
  uint8_t noteA = MatriceNote[pousserSynthNew[index]+getShift(conf.shiftHalfTone)];
  // On
  if (bouton == LOW)
  {
    // Ancien etat = on a pas appuye
    if(oldStatePousser[index] == BUTTON_RELEASED){
        // le bourdon n'etait pas en jeu
      if(bourdonActif[index] == 0){

        noteOnBourdonSynth(noteA, conf);
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
  byte attack_level = conf.attackBourdon;
  byte decay_level = 255;

  envelopeBourdon.setReleaseLevel(0);
  envelopeBourdon.setADLevels(attack_level,decay_level);
  envelopeBourdon.setTimes(0,0,0,0);
  envelopeBourdon.noteOn();
}
// Menu selection and setup the play
void updateControl(){
  byte attack_level = conf.attackTheme;
  byte decay_level = 255;

  uint8_t attack = 0;
  uint8_t decay = 0;
  uint8_t release_ms = 0; // times for the attack, decay, and release of envelope
  uint8_t sustain  = 0; //sustain time set to zero so notes can be played as long as key is presssed without being effect by previus touch
  byte pousserTirerState = 0;
  envelope.setReleaseLevel(0);
  envelope.setADLevels(attack_level,decay_level);
  envelope.setTimes(attack,decay,sustain,release_ms);
  bourdonSetup();


  uint8_t onOffSynth = 0;
  uint8_t onOffBourdon = 0;
  pousserTirerState = digitalRead(pousserTirer);
  for (int i = 0; i < 36; i++)
  {
    if(pinButton[i] == 52 || pinButton[i] == 53 || pinButton[i] == 34 || pinButton[i] == 36 || pinButton[i] == 37 || pinButton[i] == 39 ){
      onOffBourdon += noteSynthBourdon(pousserTirerState, i, conf);
    }else{
      onOffSynth += noteSynth(pousserTirerState, i, conf);
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
  conf = newPresets[i];
  oscil1.setTable(getWaveFromInt(conf.getOsc1()));
  oscil2.setTable(getWaveFromInt(conf.getOsc2()));
  bourdon1.setTable(getWaveFromInt(conf.getBrd1()));
  bourdon2.setTable(getWaveFromInt(conf.getBrd2()));
}
static int menuSelectorSwitch(int newPos, int menuActiveItem){

  if(menuActiveItem==OCTAVE){
    conf.octave = (newPos)%6;
    menuActiveItem = MAIN;
    encoder.setPosition(1);

  }else if(menuActiveItem==HALFTONE){
    conf.shiftHalfTone = (newPos%23);
    menuActiveItem = MAIN;
    encoder.setPosition(2);

  }else if(menuActiveItem==ATTACK_MAIN){
    conf.attackTheme = (conf.attackTheme+(5*newPos))%255;
    menuActiveItem = MENU_ATTACK;
    encoder.setPosition(0);
  }
  else if(menuActiveItem==ATTACK_DRONE){
    conf.attackBourdon = (conf.attackBourdon+(5*newPos))%255;
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
      encoder.setPosition(conf.octave);
    }
    else if(newPos%6 == 2){
      menuActiveItem = HALFTONE;
      encoder.setPosition(conf.shiftHalfTone);
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
      conf.setOscOct(wichOsc, newOctave);
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
    if(newPos%6 != 5){
      if(oscillator == 1){
        oscil1.setTable(getWaveFromInt(newPos%6));
        conf.activeOsc1 = newPos%6;

      }else if(oscillator == 2){
        oscil2.setTable(getWaveFromInt(newPos%6));
        conf.activeOsc2 = newPos%6;

      }else if(oscillator == 3){
        bourdon1.setTable(getWaveFromInt(newPos%6));
        conf.activeBrd1 = newPos%6;

      }else if (oscillator == 4){
        bourdon2.setTable(getWaveFromInt(newPos%6));
        conf.activeBrd2 = newPos%6;

      }else if (oscillator == 5){
        conf.setAllOsc(newPos%6);
        oscil1.setTable(getWaveFromInt(newPos%6));
        oscil2.setTable(getWaveFromInt(newPos%6));
        bourdon1.setTable(getWaveFromInt(newPos%6));
        bourdon2.setTable(getWaveFromInt(newPos%6));
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
    maindisplay(newPos, mode, mode_midi, menuActiveItem, conf, oled);
    encoderPosition = newPos;
  }
  if(state == LOW && state != stateButtonEncoder){
    menuActiveItem = menuSelectorSwitch(newPos, menuActiveItem);
    stateButtonEncoder = LOW;
    maindisplay(encoderPosition, mode, mode_midi, menuActiveItem, conf, oled);
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
  for (uint8_t pin = 0; pin < 36; pin++) {
    pinMode(pinButton[pin],INPUT);
    digitalWrite(pinButton[pin],HIGH);
  }
  maindisplay(0, mode, mode_midi, menuActiveItem, conf, oled);
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
          noteMidiBourdon(i, conf, 127);
        }else{
          noteMidi(pousserTirerState, i, conf, 127);
        }
      }
    }
  }
}
