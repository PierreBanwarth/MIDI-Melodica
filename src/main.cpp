#include <stdlib.h>
#include <Arduino.h>
#include <MIDI.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "RotaryEncoder.h"// RotaryEncoder v1.5.0 library by Matthais Hertel
#include "concertina_lib/concertina.h"
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
#include <Tone.h>
// Define proper RST_PIN if required.
#define RST_PIN -1
#define BUTTON_PRESSED 1
#define BUTTON_RELEASED 0

SSD1306AsciiWire oled;
Tone synth;

RotaryEncoder encoder(4, 5, RotaryEncoder::LatchMode::TWO03);

uint8_t oldStatePousser[] = {
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
};
uint8_t oldStateTirer[] = {
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
};

int pinButton[] = {
  6,7,8,9,22,23,
  24,25,26,27,28,29,
  30,31,32,33,34,35,
  36,37,38,39,40,41,
  42,43,44,45,46,47,
  48,49,50,51,52,53
};

int pousser[] = {
  Gn2, An2, Cs3, Cn3, Gs2, Bn2,
  En2, Gn2, Cn2, Cs2, Gn2, Dn2,
  Bn1, An1, En1, Cn1, Dn1, Dn3,
  Dn1, Dn1, Fn4, An1, Bn4, En4,
  Gn4, Cn4, Cs4, Dn4, Gn3, Fs3,
  Bn3, En3, An3, Gn3, An1, Dn1,
};

int tirer[] = {
  An2, Gn2, Ds3, Bn2, Bb2, Cn3,
  Fn2, An2, Dn2, Fs2, Bn1, Fs2,
  Dn2, Bb1, Fn1, Gn1, Dn1, En3,
  Dn1, Dn1, An4, An1, Fs4, Bn3,
  En4, An3, Ds4, Cn4, Fn3, Bb3,
  An3, Dn3, Gn3, Fs3, An1, Dn1
};
int pousserSynth[] = {
  NOTE_G2, NOTE_A2, NOTE_CS3, NOTE_C3, NOTE_GS2, NOTE_B2,
  NOTE_E2, NOTE_G2, NOTE_C2, NOTE_CS2, NOTE_G2, NOTE_D2,
  NOTE_B1, NOTE_A1, NOTE_E1, NOTE_C1, NOTE_D1, NOTE_D3,
  NOTE_D1, NOTE_D1, NOTE_F4, NOTE_A1, NOTE_B4, NOTE_E4,
  NOTE_G4, NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_G3, NOTE_FS3,
  NOTE_B3, NOTE_E3, NOTE_A3, NOTE_G3, NOTE_D1, NOTE_D1,
};

int tirerSynth[] = {
  NOTE_A2, NOTE_G2, NOTE_DS3, NOTE_B2, NOTE_AS2, NOTE_C3,
  NOTE_F2, NOTE_A2, NOTE_D2, NOTE_FS2, NOTE_B1, NOTE_FS2,
  NOTE_D2, NOTE_AS1, NOTE_F1, NOTE_G1, NOTE_D1, NOTE_E3,
  NOTE_D1, NOTE_D1, NOTE_A4, NOTE_A1, NOTE_FS4, NOTE_B3,
  NOTE_E4, NOTE_A3, NOTE_DS4, NOTE_C4, NOTE_F3, NOTE_AS3,
  NOTE_A3, NOTE_D3, NOTE_G3, NOTE_FS3, NOTE_D1, NOTE_D1
};



int pousserTirer = 10;
int buttonEncoder = 3;
int stateButtonEncoder = HIGH;
int oldStatePousserTirer = 0;
int encoderPosition;
int octave = 4;

String menuActiveItem = "MAIN";
String mode = "MIDI";

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

void displayNote(SSD1306AsciiWire oled, int sensSoufflet, int note, int index){
  oled.clear();
  if (sensSoufflet == 0)
  {
    oled.println("Pousser");
  }
  else
  {
    oled.println("Tirer");
  }
  oled.print("Note :");
  oled.println((int)note);
  oled.print("Pin :");
  oled.println((int)pinButton[index]);
}
    // Note using drone
void noteOn(int noteToPlay, int noteA, int noteB, int octave, int velocity)
{
  if (noteA == noteB)
  {
    MIDI.sendNoteOn(noteToPlay + octave, velocity * 2 / 3, 1);
  }
  else
  {
    MIDI.sendNoteOn(noteToPlay + octave, velocity, 1);
  }

}

void noteOff(int noteToShutdown, int noteA, int noteB, int octave)
{
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
{ //, uint8_t oldStatePousser, uint8_t oldStateTirer
  uint8_t newOctave = 12 * octave;
  // velocity = 127;
  digitalWrite(pinButton[index], HIGH);
  uint8_t bouton = digitalRead(pinButton[index]);
  // digitalWrite(pinButton[index], LOW);
  uint8_t noteA = pousser[index];
  uint8_t noteB = tirer[index];
  // higher velocity usually makes MIDI instruments louder
  if (bouton == LOW)
  {
    // on pousse sur le bouton et sur le soufflet
    if(oldStatePousser[index] == BUTTON_RELEASED){
      noteOn(noteA, noteA, noteB, newOctave, velocity);
    }
    if(oldStatePousser[index] == BUTTON_RELEASED){
      noteOff(noteA, noteA, noteB, newOctave);

    }
  }else{
      oldStatePousser[index] = BUTTON_RELEASED;
  }
}

int noteSynth(uint8_t sens_soufflet, uint8_t index, int octave)
{
  uint8_t bouton = digitalRead(pinButton[index]);
  octave = pow(2, octave);

  int noteSynthA = pousserSynth[index]*octave;
  int noteSynthB = tirerSynth[index]*octave;

  if (pousser[index] != 0 || tirer[index] != 0)
  {
    if (bouton == LOW)
    {
      if (sens_soufflet == LOW)
      {
        // on pousse sur le bouton et sur le soufflet
        if (oldStatePousser[index] == BUTTON_RELEASED)
        {
          synth.play(noteSynthA);
          oldStatePousser[index] = BUTTON_PRESSED;
          if (oldStateTirer[index] == BUTTON_PRESSED)
          {
            oldStateTirer[index] = BUTTON_RELEASED;
          }
        }
      }
      else
      { // on tire sur le soufflet et on appuie sur le bouton
        if (oldStateTirer[index] == 0)
        {
          synth.play(noteSynthB);
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
}

void display(int newPos, String item){
  oled.clear();
  oled.println("Melodica MIDI");
  oled.println("V 0.9");
  if(menuActiveItem=="OCTAVE"){
    oled.print("Oct : ");
    oled.print(newPos%6);
    oled.print(" ");
  }else if (menuActiveItem =="OUTPUT") {
    if(newPos%2 == 0){
      oled.println("> Midi");
      oled.println("  Synth");
    }
    else if(newPos%2 == 1){
      oled.println("  Midi");
      oled.println("> Synth");
    }
  }else if(menuActiveItem == "MAIN"){
    if(newPos%2 == 0){
      oled.println("  Output");
      oled.println("> Octave");
    }
    else if(newPos%2 == 1){
      oled.println("> Ouptut");
      oled.println("  Octave");
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
    }else if(menuActiveItem=="OUTPUT"){
      if(newPos%2 == 0){
        menuActiveItem = "MAIN";
        mode = "MIDI";
      }
      else if(newPos%2 == 1){
        menuActiveItem = "MAIN";
        mode = "SYNTH";
      }
    }else if(menuActiveItem=="MAIN"){
      if(newPos%2 == 0){
        menuActiveItem = "OCTAVE";
      }
      else if(newPos%2 == 1){
        menuActiveItem = "OUTPUT";
      }
    }
    stateButtonEncoder = LOW;
    display(encoderPosition, menuActiveItem);
  }
  stateButtonEncoder = state;

}
void setup() {
  synth.begin(A1);

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
    int pousserTirerState = digitalRead(pousserTirer);
    int sum = 0;
    for (int i = 0; i < 36; i++)
    {
      sum += noteSynth(pousserTirerState,i, octave);
    }
    // int sum =0;
    // for(int i=0; i<36; i++){
    //   sum += oldStatePousser[i]+oldStatePousser[i];
    // }
    if (sum==0){
      synth.stop();
    }


  }


}
