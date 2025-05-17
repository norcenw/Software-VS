#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 20 chars and 4 line display

void setup()
{
  lcd.init();                      // initialize the lcd 
  lcd.backlight();                 // backlight on
  lcd.cursor();                    // cursor visible
  lcd.blink();                     // blink cursor
  lcd.print("ciaooooo!!!");
  delay(2000);
  lcd.clear();
  lcd.print("mucchetta");
  delay(2000);
  lcd.clear();
  lcd.print("TI AMO!!!");
}

void loop(){
  
}