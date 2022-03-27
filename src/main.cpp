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

// Define proper RST_PIN if required.
#define RST_PIN -1
#define BUTTON_PRESSED 1
#define BUTTON_RELEASED 0

SSD1306AsciiWire oled;

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
  Bn2, As2, En2, An2, Dn2, Cn2, 
  An2, An2, Fn2, Gn2, Bn2, Fs2, 
  Gs2, Ds2, Cs2, Bn2, Dn2, En2, 
  Dn2, Bn2, An2, Ds2, Fn2, Gn2, 
  As2, Bn2, Fs2, Ds2, Dn2, An2, 
  Gn2, Fn2, Cn2, As2, Gn2, An2
};

int tirer[] = {
  En2, Cs2, Dn2, Gn2, An2, Dn2, 
  Cn2, Bn2, Gn2, Gs2, Cn2, Gn2, 
  Fs2, Cs2, Ds2, Bn2, Dn2, Gn2, 
  An2, En2, Bn2, As2, En2, Cn2, 
  An2, Gn2, Dn2, Cs2, Cn2, Gn2, 
  An2, En2, Bn2, Gs2, Gn2, An2
};
int pousserTirer = 10;
int buttonEncoder = 3;

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
            displayNote(oled, sens_soufflet, noteA, index);
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
            displayNote(oled, sens_soufflet, noteB, index);
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

int oldStatePousserTirer = 0;

void setup() {
  pinMode(pousserTirer, INPUT);
  digitalWrite(pousserTirer, HIGH);

  pinMode(buttonEncoder, INPUT);
  digitalWrite(buttonEncoder, HIGH);

  // for testing function
  Wire.begin();
  Wire.setClock(400000L);
#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0
  oled.setFont(Adafruit5x7);
  oled.displayRemap(false);
  Serial.begin(9600);
  
  for (size_t pin = 0; pin < 36; pin++) {
    pinMode(pinButton[pin],INPUT);
    digitalWrite(pinButton[pin],HIGH);
  }
}

void loop() {

  // Loop to set pins HIGH
  encoder.tick();
  int pousserTirerState = digitalRead(pousserTirer);
  for (int i = 0; i < 36; i++)
  {
    note(pousserTirerState, i, 3, 127);
  }

}