#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#include "display.h"

void Display::displayShift(int i, SSD1306AsciiWire oled){
    if(i==0){
      oled.print("Sol#/Do#");
    }else if(i == 1){
      oled.print("La/Re");
    }else if(i == 2){
      oled.print("Sib/Re#");
    }else if(i == 3){
      oled.print("Si/Mi");
    }else if(i == 4){
      oled.print("Do/Fa");
    }else if(i == 5){
      oled.print("Do#/Fa#");
    }else if(i == 6){
      oled.print("Re/Sol");
    }else if(i == 7){
      oled.print("Re#/Sol#");
    }else if(i == 8){
      oled.print("Mi/La");
    }else if(i == 9){
      oled.print("Fa/Sib");
    }else if(i == 10){
      oled.print("Fa#/Si");
    }else if(i == 11){
      oled.print("Sol/Do");
    }else if(i == 12){
      oled.print("Sol#/Do#");
    }else if(i == 13){
      oled.print("La/Re");
    }else if(i == 14){
      oled.print("Sib/Re#");
    }else if(i == 15){
      oled.print("Si/Mi");
    }else if(i == 16){
      oled.print("Do/Fa");
    }else if(i == 17){
      oled.print("Do#/Fa#");
    }else if(i == 18){
      oled.print("Re/Sol");
    }else if(i == 19){
      oled.print("Re#/Sol#");
    }else if(i == 20){
      oled.print("Mi/La");
    }else if(i == 21){
      oled.print("Fa/Sib");
    }else if(i == 22){
      oled.print("Fa#/Si");
    }
}
