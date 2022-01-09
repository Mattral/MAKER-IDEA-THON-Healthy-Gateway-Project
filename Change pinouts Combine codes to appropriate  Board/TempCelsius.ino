#include "U8glib.h"            // U8glib library for the OLED
#include <Wire.h>              // Wire library for I2C communication
#include <Adafruit_MLX90614.h> // MLX90614 library from Adafruit

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

U8GLIB_SH1106_128X64 u8g(13, 11, 10, 9, 8); // D0=13, D1=11, CS=10, DC=9, Reset=8

void draw(void) 
{
  u8g.setFont(u8g_font_profont15r);        // select font
  u8g.drawStr(1, 12, "Object Temperature");// 
  u8g.setFont(u8g_font_profont29r);        // select font for temperature readings
  u8g.println("C");                        // prints C for Celsius
  u8g.setPrintPos(35, 45);                 // set position
  u8g.println(mlx.readObjectTempC(), 0);   // display temperature from MLX90614
  u8g.drawRFrame(15, 20, 100, 30, 10);     // draws frame with rounded edges
}

void setup(void) 
{
    mlx.begin();  //Receive data from the sensor
}

void loop(void) 
{
  u8g.firstPage();  
  do 
    {
     draw();      
    }
  while( u8g.nextPage() );
  
  delay(1000);  // Delay of 1sec 
}

                       /*END OF FILE*/
