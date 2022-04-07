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

#include <mozzi_rand.h>
#include <ADSR.h>
// 0X3C+SA0 - 0x3C or 0x3D

#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1
#define BUTTON_PRESSED 1
#define BUTTON_RELEASED 0

ADSR <CONTROL_RATE, AUDIO_RATE> envelope; // notre enveloppe


// Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscil1(SIN2048_DATA);
// Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscil2(SIN2048_DATA);
// Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscil1(SIN2048_DATA);
// Oscil<COS2048_NUM_CELLS, AUDIO_RATE> oscil2(COS2048_DATA);

Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscil1;
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscil2;


SSD1306AsciiWire oled;

RotaryEncoder encoder(4, 5, RotaryEncoder::LatchMode::TWO03);

int onOffSynth = 0;
int pousserTirer = 10;
int buttonEncoder = 3;
int stateButtonEncoder = HIGH;
int oldStatePousserTirer = 0;
int encoderPosition;
int octave = 4;
int globalFrequence;
int stateWave = 1;
String menuActiveItem = "MAIN";
String mode = "SYNTH";

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

void displayNote(int note){
  oled.clear();
  oled.print("Note :");
  oled.println((int)note);
}
void displayswitch(String s){
  oled.clear();
  oled.print("Note :");
}
    // Note using drone
void noteOn(int noteToPlay, int noteA, int noteB, int octave, int velocity){
  if (noteA == noteB)
  {
    MIDI.sendNoteOn(noteToPlay + octave, velocity * 2 / 3, 1);
  }
  else
  {
    MIDI.sendNoteOn(noteToPlay + octave, velocity, 1);
  }

}

void noteOff(int noteToShutdown, int noteA, int noteB, int octave){
  if (noteA != noteB)
  {
    MIDI.sendNoteOff(noteToShutdown + octave, 0, 1);
  }
}

// playing midi notes
void note(uint8_t sens_soufflet, uint8_t index, uint8_t octave, int velocity)
{ //, uint8_t oldStatePousser, uint8_t oldStateTirer
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

            noteOn(noteA, noteA, noteB, newOctave, velocity);
            oldStatePousser[index] = BUTTON_PRESSED;
            if (oldStateTirer[index] == BUTTON_PRESSED)
            {
              noteOff(noteB, noteA, noteB, newOctave);
              oldStateTirer[index] = BUTTON_RELEASED;
            }
          }
        }
        else
        { // on tire sur le soufflet et on appuie sur le bouton
          if (oldStateTirer[index] == 0)
          {

            noteOn(noteB, noteA, noteB, newOctave, velocity);
            oldStateTirer[index] = BUTTON_PRESSED;
            if (oldStatePousser[index] == BUTTON_PRESSED)
            {
              noteOff(noteA, noteA, noteB, newOctave);
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

void noteBourdon(uint8_t sens_soufflet, uint8_t index, uint8_t octave, int velocity)
{
  //, uint8_t oldStatePousser, uint8_t oldStateTirer
  uint8_t newOctave = 12 * octave;
  // velocity = 127;
  digitalWrite(pinButton[index], HIGH);
  // digitalWrite(pinButton[index], LOW);
  uint8_t noteA = pousser[index];
  uint8_t noteB = tirer[index];
  if (pousser[index] != 0 || tirer[index] != 0)
  {
    // on pousse sur le bouton et sur le soufflet
    if(oldStatePousser[index] == BUTTON_RELEASED){
      noteOn(noteA, noteA, noteB, newOctave, velocity);
      oldStatePousser[index] = BUTTON_PRESSED;

    }
    if(oldStatePousser[index] == BUTTON_PRESSED){
      noteOff(noteA, noteA, noteB, newOctave);
      oldStatePousser[index] = BUTTON_RELEASED;

            if (bourdonActif[index] == 0)
            {
              noteOn(noteA, noteA, noteB, newOctave, velocity);
              bourdonActif[index] = 1;
            }
            else
            {
              MIDI.sendNoteOff(noteA + newOctave, 0, 1);
              bourdonActif[index] = 0;
            }
            oldStateTirer[index] = BUTTON_PRESSED;
            if (oldStatePousser[index] == BUTTON_PRESSED)
            {
              oldStatePousser[index] = BUTTON_RELEASED;
            }
          }
          else

          {
            if (oldStatePousser[index] == BUTTON_PRESSED)
            {
              oldStatePousser[index] = BUTTON_RELEASED;
            }
            if (oldStateTirer[index] == BUTTON_PRESSED)
            {
              oldStateTirer[index] = BUTTON_RELEASED;
            }
          }
  }
}
// SYNTH CODE
void setTableSin() {
  oscil1.setTable(SIN2048_DATA);
  oscil2.setTable(SIN2048_DATA);
  displayswitch("sin");
}
void setTableTriangle() {
  oscil1.setTable(TRIANGLE_VALVE_2_2048_DATA);
  oscil2.setTable(TRIANGLE_VALVE_2_2048_DATA);
  displayswitch("triangle");

}
void setTableSaw() {
  oscil1.setTable(SAW2048_DATA);
  oscil2.setTable(SAW2048_DATA);
  displayswitch("triangle");

}

void noteOnSynth(int frequence){
  oscil1.setFreq(frequence);
  oscil2.setFreq(frequence/2);
  envelope.noteOn(); // on joue la note

}

int noteSynth(uint8_t sens_soufflet, uint8_t index, int octave)
{
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);
  int newOctave = 1;
  if (octave == 1){
    newOctave = 1;
  }
  if (octave == 2){
    newOctave = 4;
  }
  if (octave == 3){
    newOctave = 8;
  }
  if (octave == 4){
    newOctave = 16;
  }
  if (octave == 5){
    newOctave = 32;
  }
  int noteSynthA = pousserSynth[index]*newOctave;
  int noteSynthB = tirerSynth[index]*newOctave;

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
          noteOnSynth(noteSynthA);
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
          noteOnSynth(noteSynthB);
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
// niveau_attaque, niveau_chute, temps_attaque, temps_chute, temps_entretien, temps_extinction, fréquence
// 255, 255, 30, 30, 2000, 30, 300
//


void updateControl()
{
  byte attack_level = 127;
  byte decay_level = 255;
  envelope.setReleaseLevel(0);
  envelope.setADLevels(attack_level,decay_level);
  int attack = 0;
  int decay = 0;
  int release_ms = 0; // times for the attack, decay, and release of envelope
  int sustain  = 0; //sustain time set to zero so notes can be played as long as key is presssed without being effect by previus touch
  envelope.setTimes(attack,decay,sustain,release_ms);

  int pousserTirerState = digitalRead(pousserTirer);
  int onOffSynth = 0;

  for (int i = 0; i < 36; i++)
  {
    onOffSynth += noteSynth(pousserTirerState, i, octave);
  }
  if(onOffSynth == 0){
    envelope.noteOff();
  }

  // put changing controls in here
}
int updateAudio()
{
  // long test = (long) envelope.next() * (oscil1.next() + oscil2.next());
  return (int) (envelope.next() * (oscil1.next()+oscil2.next() ))>>8;
}

void display(int newPos, String item){
  oled.clear();
  oled.println("Melodica MIDI");
  oled.println("V 0.9");
  if(menuActiveItem=="OCTAVE"){
    oled.print("Oct : ");
    oled.print(newPos%6);
    oled.print(" ");
  }else if(menuActiveItem == "MAIN"){
    if(newPos%2 == 0){
      oled.println("> Octave");
      oled.println("  Synth");
    }
    else if(newPos%2 == 1){
      oled.println("  Octave");
      oled.println("> Synth");
    }
  }else if(menuActiveItem == "SYNTH"){
    if(newPos%2 == 0){
      oled.println("> Wave");
      oled.println("  Gain");
    }
    else if(newPos%2 == 1){
      oled.println("  Wave");
      oled.println("> Gain");
    }
  }else if(menuActiveItem == "WAVE"){
    if(newPos%4 == 0){
      oled.println("> Sin");
      oled.println("  Cos");
      oled.println("  Saw");
      oled.println("  Back");

    }
    else if(newPos%4 == 1){
      oled.println("  Sin");
      oled.println("> Cos");
      oled.println("  Saw");
      oled.println("  Back");

    }
    else if(newPos%4 == 2){
      oled.println("  Sin");
      oled.println("  Cos");
      oled.println("> Saw");
      oled.println("  Back");
    }
    else if(newPos%4 == 3){
      oled.println("  Sin");
      oled.println("  Cos");
      oled.println("  Saw");
      oled.println("> Back");
    }
  }
}

void menuSelector(){
  encoder.tick();

  int newPos = encoder.getPosition();

  if (encoderPosition != newPos) {
    display(newPos, menuActiveItem);
    encoderPosition = newPos;
  }
  int state = digitalRead(buttonEncoder);
  if(state == LOW && state != stateButtonEncoder){
    if(menuActiveItem=="OCTAVE"){
      octave = encoder.getPosition()%6;
      menuActiveItem = "MAIN";
    }else if(menuActiveItem=="MAIN"){
      if(newPos%2 == 0){
        menuActiveItem = "OCTAVE";
      }
      else if(newPos%2 == 1){
        menuActiveItem = "SYNTH";
      }
    }else if(menuActiveItem=="SYNTH"){
      if(newPos%2 == 0){
        menuActiveItem = "WAVE";
      }
      else if(newPos%2 == 1){
        menuActiveItem = "GAIN";
      }
    }
    else if(menuActiveItem=="WAVE"){
      if(newPos%4 == 0){
        setTableSin();
      }else if(newPos%4 == 1){
        setTableTriangle();
      }else if(newPos%4 == 2){
        setTableSaw();
      }else if(newPos%4 == 3){
        menuActiveItem = "SYNTH";
      }
    }
    stateButtonEncoder = LOW;
    display(encoderPosition, menuActiveItem);
  }
  stateButtonEncoder = state;

}


void setup() {
  startMozzi(112);
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
  menuSelector();
  audioHook();

  if(mode == "MIDI"){
    int pousserTirerState = digitalRead(pousserTirer);
    for (int i = 0; i < 36; i++)
    {
      if(pinButton[i] == 52 || pinButton[i] == 53 || pinButton[i] == 34 || pinButton[i] == 36 || pinButton[i] == 37 || pinButton[i] == 39 ){
        noteBourdon(pousserTirerState, i, octave, 127);
      }else{
        note(pousserTirerState, i, octave, 127);
      }
    }
  }
  else{
    // int sum =0;
    // for(int i=0; i<36; i++){
    //   sum += oldStatePousser[i]+oldStatePousser[i];
    // }

  }


}
