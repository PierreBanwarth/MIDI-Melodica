
#include <stdlib.h>
#include <Arduino.h>

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "RotaryEncoder.h"                                                      // RotaryEncoder v1.5.0 library by Matthais Hertel

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

SSD1306AsciiWire oled;

RotaryEncoder encoder(4, 5, RotaryEncoder::LatchMode::TWO03);


int pinButton[] = {6,7,8,9,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53};
int pousserTirer = 10;
void setup() {
  // Configuration de la broche 12 en tant qu'entree numerique.
  // for testing function
  Wire.begin();
  Wire.setClock(400000L);
#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0
  oled.setFont(Adafruit5x7);

  // Use true, normal mode, since default for Adafruit display is remap mode.
  oled.displayRemap(false);

  oled.clear();
  oled.println("Hello world!");
  oled.println("displayRemap(false)");
  oled.print("for other mode.");
  Serial.begin(9600); // Permet d'initialiser le moniteur série à 9600 bauds
  for (size_t pin = 0; pin < 36; pin++) {
    pinMode(pinButton[pin],INPUT);
    digitalWrite(pinButton[pin],HIGH);
  }
}

void loop() {

  // Loop to set pins HIGH
  static int pos = 0;

  encoder.tick();

  int newPos = encoder.getPosition();
  if (pos != newPos) {
    Serial.print("pos:");
    Serial.print(newPos);
    Serial.print(" dir:");
    Serial.println((int)(encoder.getDirection()));
    pos = newPos;
  } // if

  for(int i=0; i<36; i++)
  {
    // See if corresponding pin, and only that pin, is HIGH7
    uint8_t bouton = digitalRead(pinButton[i]);
    if(bouton == LOW){
      // Good
      Serial.println(pinButton[i]);
    }
  }

}
