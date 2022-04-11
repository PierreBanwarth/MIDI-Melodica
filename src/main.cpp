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
#include <tables/triangle_valve_2_2048_int8.h>
#include <tables/sin1024_int8.h>
#include <mozzi_rand.h>
#include <ADSR.h>

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1
#define BUTTON_PRESSED 1
#define BUTTON_RELEASED 0

ADSR <CONTROL_RATE, AUDIO_RATE> envelope; // notre enveloppe
ADSR <CONTROL_RATE, AUDIO_RATE> envelopeBourdon; // notre enveloppe
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscil1(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscil2(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> bourdon1(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> bourdon2(SIN2048_DATA);

SSD1306AsciiWire oled;
RotaryEncoder encoder(4, 5, RotaryEncoder::LatchMode::TWO03);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

int bourdonActifSynth = 0;
int bourdonPressed = 0;
byte indexMenu = 0;
int onOffSynth = 0;
int pousserTirer = 10;
int buttonEncoder = 3;
int stateButtonEncoder = HIGH;
int oldStatePousserTirer = 0;
int encoderPosition;

int octave = 4;

int octaveOsc1 = 0;
int octaveOsc2 = -1;

byte attackTheme = 255;
byte attackBourdon = 125;

int octaveBourdon1 = -2;
int octaveBourdon2 = -3;

int wichOsc = 0;

int globalFrequence;
int stateWave = 1;
int bourdonSynthActivated = 0;
String menuActiveItem = "MAIN";
String mode = "SYNTH";
int oscillator = 0;


void displayNote(int note){
  oled.clear();
  oled.print("Note :");
  oled.println((int)note);
}

void displayswitch(String s){
  oled.clear();
  oled.print("Note :");
}

void noteOn(int noteToPlay, int octave, int velocity){
  MIDI.sendNoteOn(noteToPlay + octave, velocity, 1);
}

void noteOff(int noteToShutdown, int octave){
    MIDI.sendNoteOff(noteToShutdown + octave, 0, 1);
}

void noteMidi(uint8_t sens_soufflet, uint8_t index, uint8_t octave, int velocity){
   //, uint8_t oldStatePousser, uint8_t oldStateTirer
  uint8_t newOctave = 12 * octave;
  // velocity = 127;
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);
  // digitalWrite(pinButton[index], LOW);
  uint8_t noteA = pousser[index];
  uint8_t noteB = tirer[index];
  if (pousser[index] != 0 || tirer[index] != 0)
  {
    if (oldStatePousser[index] > 1)
    { // Tune started too recently to be changed
      oldStatePousser[index]--;
    }
    else if (oldStateTirer[index] > 1)
    { // Tune started too recently to be changed
      oldStateTirer[index]--;
    }
    else
    {
      // higher velocity usually makes MIDI instruments louder
      if (bouton == LOW)
      {
        if (sens_soufflet == LOW)
        {
          // on pousse sur le bouton et sur le soufflet
          if (oldStatePousser[index] == BUTTON_RELEASED)
          {

            noteOn(noteA, newOctave, velocity);
            oldStatePousser[index] = BUTTON_PRESSED;
            if (oldStateTirer[index] == BUTTON_PRESSED)
            {
              noteOff(noteB, newOctave);
              oldStateTirer[index] = BUTTON_RELEASED;
            }
          }
        }
        else
        { // on tire sur le soufflet et on appuie sur le bouton
          if (oldStateTirer[index] == 0)
          {

            noteOn(noteB, newOctave, velocity);
            oldStateTirer[index] = BUTTON_PRESSED;
            if (oldStatePousser[index] == BUTTON_PRESSED)
            {
              noteOff(noteA, newOctave);
              oldStatePousser[index] = BUTTON_RELEASED;
            }
          }
        }
      }
      else
      {
        if (oldStatePousser[index] == BUTTON_PRESSED)
        {
          MIDI.sendNoteOff(noteA + newOctave, 0, 1);
          oldStatePousser[index] = BUTTON_RELEASED;
        }
        if (oldStateTirer[index] == BUTTON_PRESSED)
        {
          // on appuie pas sur le bouton ma
          MIDI.sendNoteOff(noteB + newOctave, 0, 1);
          oldStateTirer[index] = BUTTON_RELEASED;
        }
      }
    }
  }
}

void noteMidiBourdon(uint8_t index, uint8_t octave, int velocity){
  //, uint8_t oldStatePousser, uint8_t oldStateTirer
  uint8_t newOctave = 12 * (octave-1);
  // velocity = 127;
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);
  // digitalWrite(pinButton[index], LOW);
  uint8_t note = pousser[index]*newOctave;

  if (bouton == LOW)
  {
    // on pousse sur le bouton et sur le soufflet
    if(oldStatePousser[index] == BUTTON_RELEASED){
      oldStatePousser[index] = BUTTON_PRESSED;
      if(bourdonActif[index] == 1){
        noteOff(note, newOctave);
        bourdonActif[index] = 0;
      }else{
        noteOn(note, newOctave, velocity);
        bourdonActif[index] = 1;
      }
    }

  }else{
    oldStatePousser[index] = BUTTON_RELEASED;
  }
}
// octaveValue = 0 1 2 3 4 5
// osc = -2 -1 0 1 2

int getOctaveValueToMultiplyForOsc(int osc, int frequence, int octaveValue){
  int freq = frequence;
  if(osc == -3){
    freq = frequence/8;
  }else if(osc == -2){
    freq = frequence/4;
  }else if(osc==-1){
    freq = frequence/2;
  }else if(osc==1){
    freq = frequence*2;
  }else if(osc==2){
    freq = frequence*4;
  }
  if(octaveValue == 1){
    freq = freq*2;
  }else if(octaveValue == 2){
    freq = freq*4;
  }else if(octaveValue == 3){
    freq = freq*8;
  }else if(octaveValue == 4){
    freq = freq*16;
  }else if(octaveValue == 5){
    freq = freq*32;
  }
  return freq;
}

void noteOnSynth(int frequence, int octave){
  oscil1.setFreq(getOctaveValueToMultiplyForOsc(octaveOsc1, frequence, octave));
  oscil2.setFreq(getOctaveValueToMultiplyForOsc(octaveOsc2, frequence, octave));
  envelope.noteOn(); // on joue la note
}

void noteOnBourdonSynth(int frequence, int octave){
  bourdon1.setFreq(getOctaveValueToMultiplyForOsc(octaveBourdon1, frequence, octave));
  bourdon2.setFreq(getOctaveValueToMultiplyForOsc(octaveBourdon2, frequence, octave));
  envelopeBourdon.noteOn(); // on joue la note
}

int noteSynth(uint8_t sens_soufflet, uint8_t index, int octave){
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);
  int noteSynthA = pousserSynth[index];
  int noteSynthB = tirerSynth[index];

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

          noteOnSynth(noteSynthA, octave);
          oldStatePousser[index] = BUTTON_PRESSED;
          if (oldStateTirer[index] == BUTTON_PRESSED)
          {
            oldStateTirer[index] = BUTTON_RELEASED;
          }
        }
      }
      else
      { // on tire sur le soufflet et on appuie sur le bouton
        if (oldStateTirer[index] == BUTTON_RELEASED)
        {
          noteOnSynth(noteSynthB, octave);
          oldStateTirer[index] = BUTTON_PRESSED;
          if (oldStatePousser[index] == BUTTON_PRESSED)
          {
            oldStatePousser[index] = BUTTON_RELEASED;
          }
        }
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

int noteSynthBourdon(uint8_t sens_soufflet, uint8_t index, uint8_t octave){
  //, uint8_t oldStatePousser, uint8_t oldStateTirer
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);

  uint8_t noteA = pousserSynth[index];
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

void bourdonSetup(){
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
  envelope.setReleaseLevel(0);
  envelope.setADLevels(attack_level,decay_level);
  envelope.setTimes(attack,decay,sustain,release_ms);
  bourdonSetup();



  int onOffSynth = 0;
  int onOffBourdon = 0;
  for (int i = 0; i < 36; i++)
  {
    int pousserTirerState = digitalRead(pousserTirer);
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

void setOscOct(int osc, int value){

  if(osc == 1){
    octaveOsc1 = value;
  }else if(osc == 2){
    octaveOsc2 = value;
  }else if(osc == 3){
    octaveBourdon1 = value;
  }else if(osc == 4){
    octaveBourdon2 = value;
  }
}

void display(int newPos, String item){
  oled.clear();
  oled.println("Melodica MIDI");
  oled.println("V 0.9.5");
  if(menuActiveItem=="OCTAVE"){
    oled.print("Oct : ");
    oled.print(newPos%6);
    oled.print(" ");
  }else if(menuActiveItem == "ATTACK_MAIN"){
    oled.print("Attack : ");
    oled.print((attackTheme+5*newPos)%255);
    oled.print(" ");
  }else if(menuActiveItem == "ATTACK_DRONE"){
    oled.print("Attack : ");
    oled.print((attackBourdon+5*newPos)%255);
    oled.print(" ");
  }else if(menuActiveItem == "CHOOSE_OCTAVE"){
    oled.print("Oct : ");
    oled.print(newPos%6 -3);
    oled.print(" ");
  }else if(menuActiveItem == "MAIN"){
    if(newPos%3 == 0){
      oled.println("> Octave");
      oled.println("  Synth");
      oled.println("  Mode");
    }
    else if(newPos%3 == 1){
      oled.println("  Octave");
      oled.println("> Synth");
      oled.println("  Mode");
    }
    else if(newPos%3 == 2){
      oled.println("  Octave");
      oled.println("  Synth");
      oled.println("> Mode");
    }
  }else if(menuActiveItem == "MODE"){
    if(newPos%2 == 0){
      oled.println("> Midi");
      oled.println("  Synth");
    }
    else if(newPos%2 == 1){
      oled.println("  Midi");
      oled.println("> Synth");
    }
  }else if(menuActiveItem == "SYNTH"){
    if(newPos%4 == 0){
      oled.println("> Wave");
      oled.println("  Attack");
      oled.println("  Osc Oct");
      oled.println("  Back");
    }
    else if(newPos%4 == 1){
      oled.println("  Wave");
      oled.println("> Attack");
      oled.println("  Osc Oct");
      oled.println("  Back");
    }
    else if (newPos%4 == 2)
    {
      oled.println("  Wave");
      oled.println("  Attack");
      oled.println("> Osc Oct");
      oled.println("  Back");
    }
    else if (newPos%4 == 3)
    {
      oled.println("  Wave");
      oled.println("  Attack");
      oled.println("  Osc Oct");
      oled.println("> Back");
    }
  }
  else if (menuActiveItem == "ATTACK"){
    if(newPos%3==0){
      oled.println("> Theme");
      oled.println("  Drone");
      oled.println("  Back");
    }else if(newPos%3==1){
      oled.println("  Theme");
      oled.println("> Drone");
      oled.println("  Back");
    }else if(newPos%3==2){
      oled.println("  Theme");
      oled.println("  Drone");
      oled.println("> Back");
    }
  }
  else if (menuActiveItem == "OCT_OSC" || menuActiveItem == "WAVE")
  {
    if( newPos % 5 == 0){
      oled.println("> Osc1 ");
      oled.println("  Osc2 ");
      oled.println("  Drone1");
      oled.println("  Drone2 ");
      oled.println("  Back ");
    }
    else if (newPos % 5 == 1){
      oled.println("  Osc1");
      oled.println("> Osc2");
      oled.println("  Drone1");
      oled.println("  Drone2");
      oled.println("  Back ");
    }
    else if (newPos % 5 == 2){
      oled.println("  Osc1");
      oled.println("  Osc2");
      oled.println("> Drone1");
      oled.println("  Drone2");
      oled.println("  Back ");
    }
    else if (newPos % 5 == 3)
    {
      oled.println("  Osc1");
      oled.println("  Osc2");
      oled.println("  Drone1");
      oled.println("> Drone2");
      oled.println("  Back ");
    }
    else if (newPos % 5 == 4)
    {
      oled.println("  Osc1");
      oled.println("  Osc2");
      oled.println("  Drone1");
      oled.println("  Drone2");
      oled.println("> Back ");
    }
  }else if (menuActiveItem == "OSCILLATOR")
    {
      if (newPos % 4 == 0)
      {
        oled.println("> Sin");
        oled.println("  Tri");
        oled.println("  Saw");
        oled.println("  Back");
      }
      else if (newPos % 4 == 1)
      {
        oled.println("  Sin");
        oled.println("> Tri");
        oled.println("  Saw");
        oled.println("  Back");
      }
      else if (newPos % 4 == 2)
      {
        oled.println("  Sin");
        oled.println("  Tri");
        oled.println("> Saw");
        oled.println("  Back");
      }
      else if (newPos % 4 == 3)
      {
        oled.println("  Sin");
        oled.println("  Tri");
        oled.println("  Saw");
        oled.println("> Back");
      }
    }
  }

void menuSelector(){
  encoder.tick();
  int newOctave;
  byte newPos = encoder.getPosition();
  if (encoderPosition != newPos)
  {
    display(newPos, menuActiveItem);
    encoderPosition = newPos;
  }
  byte state = digitalRead(buttonEncoder);

  if(state == LOW && state != stateButtonEncoder){
    if(menuActiveItem=="OCTAVE"){
      octave = newPos%6;
      menuActiveItem = "MAIN";
    }else if(menuActiveItem=="ATTACK_MAIN"){
      attackTheme = (attackTheme+5*newPos)%255;
      encoder.setPosition(0);
      menuActiveItem = "ATTACK";
    }
    else if(menuActiveItem=="ATTACK_DRONE"){
      attackBourdon = (attackBourdon+5*newPos)%255;
      encoder.setPosition(0);
      menuActiveItem = "ATTACK";
    }
    else if(menuActiveItem=="MAIN"){
      if(newPos%3 == 0){
        menuActiveItem = "OCTAVE";
      }
      else if(newPos%3 == 1){
        menuActiveItem = "SYNTH";
      }else if(newPos%3 == 2){
          menuActiveItem = "MODE";
      }
    }else if(menuActiveItem=="MODE"){
      if(newPos%2 == 0){
        mode="MIDI";
        menuActiveItem = "MAIN";
      }
      else if(newPos%2 == 1){
        mode="SYNTH";
        menuActiveItem = "MAIN";
      }
    }else if(menuActiveItem=="ATTACK"){
      if(newPos%3 == 0){
        menuActiveItem = "ATTACK_MAIN";
      }
      else if(newPos%3 == 1){
        menuActiveItem = "ATTACK_DRONE";
      }
      else if(newPos%3 == 2){
        menuActiveItem = "SYNTH";
      }
    }else if(menuActiveItem=="SYNTH"){
      if(newPos%4 == 0){
        menuActiveItem = "WAVE";
      }
      else if(newPos%4 == 1){
        menuActiveItem = "ATTACK";
      }
      else if(newPos%4 == 2){
        menuActiveItem = "OCT_OSC";
      }
      else if (newPos%4 == 3)
      {
        menuActiveItem = "MAIN";
      }
    }else if(menuActiveItem=="CHOOSE_OCTAVE"){
        newOctave = (newPos%5)-2;
        setOscOct(wichOsc, newOctave);
        encoder.setPosition(0);
        menuActiveItem = "SYNTH";

    }else if(menuActiveItem=="OCT_OSC"){
      if(newPos%5 == 0){
        menuActiveItem = "CHOOSE_OCTAVE";
        wichOsc = 1;
      }
      else if(newPos%5 == 1){
        menuActiveItem = "CHOOSE_OCTAVE";
        wichOsc = 2;

      }
      else if(newPos%5 == 2){
        menuActiveItem = "CHOOSE_OCTAVE";
        wichOsc = 3;

      }
      else if (newPos%5 == 3)
      {
        menuActiveItem = "CHOOSE_OCTAVE";
        wichOsc = 4;

      }
      else if(newPos%5 == 4){
        menuActiveItem = "SYNTH";
      }

    }else if (menuActiveItem == "WAVE")
    {
      if (newPos % 5 == 0)
      {
        menuActiveItem = "OSCILLATOR";
        oscillator = 1;
      }
      else if (newPos % 5 == 1)
      {
        menuActiveItem = "OSCILLATOR";
        oscillator = 2;
      }
      else if (newPos % 5 == 2)
      {
        menuActiveItem = "OSCILLATOR";
        oscillator = 3;
      }
      else if (newPos % 5 == 3)
      {
        menuActiveItem = "OSCILLATOR";
        oscillator = 4;
      }
      else if (newPos % 5 == 4)
      {
        menuActiveItem = "SYNTH";
      }
    }else if(menuActiveItem=="OSCILLATOR"){
      if(newPos%4 == 0){
        if(oscillator == 1){
          oscil1.setTable(SIN2048_DATA);

        }else if(oscillator == 2){
          oscil2.setTable(SIN2048_DATA);
        }
        else if(oscillator == 3){
          bourdon1.setTable(SIN2048_DATA);
          bourdon2.setTable(SIN2048_DATA);
        }
        else if (oscillator == 4)
        {
          oscil1.setTable(SIN2048_DATA);
          oscil2.setTable(SIN2048_DATA);
          bourdon1.setTable(SIN2048_DATA);
          bourdon2.setTable(SIN2048_DATA);
        }
      }else if(newPos%4 == 1){
        if (oscillator == 1){
          oscil1.setTable(TRIANGLE_VALVE_2_2048_DATA);
        }
        else if (oscillator == 2){
          oscil2.setTable(TRIANGLE_VALVE_2_2048_DATA);
        }
        else if(oscillator == 3){
          bourdon1.setTable(TRIANGLE_VALVE_2_2048_DATA);
          bourdon2.setTable(TRIANGLE_VALVE_2_2048_DATA);
        }
        else if (oscillator == 4)
        {
          oscil1.setTable(TRIANGLE_VALVE_2_2048_DATA);
          oscil2.setTable(TRIANGLE_VALVE_2_2048_DATA);
          bourdon1.setTable(TRIANGLE_VALVE_2_2048_DATA);
          bourdon2.setTable(TRIANGLE_VALVE_2_2048_DATA);
        }
      }else if(newPos%4 == 2){
        if (oscillator == 1)        {
          oscil1.setTable(SAW2048_DATA);
        }
        else if (oscillator == 2){
          oscil2.setTable(SAW2048_DATA);
        }
        else if(oscillator == 3){
          bourdon1.setTable(SAW2048_DATA);
          bourdon2.setTable(SAW2048_DATA);
        }
        else if (oscillator == 4)
        {
          oscil1.setTable(SAW2048_DATA);
          oscil2.setTable(SAW2048_DATA);
          bourdon1.setTable(SAW2048_DATA);
          bourdon2.setTable(SAW2048_DATA);
        }
      }else if(newPos%4 == 3){
        menuActiveItem = "WAVE";
        oscillator = 0;
      }
    }
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
  encoderPosition = encoder.getPosition();
  display(0, menuActiveItem);
}

void loop() {
  indexMenu++;
  if(indexMenu==30){
    menuSelector();
    indexMenu = 0;
  }
  audioHook();
  if(mode == "MIDI"){
    byte pousserTirerState = digitalRead(pousserTirer);
    for (byte i = 0; i < 36; i++)
    {
      if(pinButton[i] == 52 || pinButton[i] == 53 || pinButton[i] == 34 || pinButton[i] == 36 || pinButton[i] == 37 || pinButton[i] == 39 ){

        noteMidiBourdon(i, octave, 127);
      }else{
        noteMidi(pousserTirerState, i, octave, 127);
      }
    }
  }
}
