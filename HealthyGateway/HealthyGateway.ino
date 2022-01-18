//***********************************************Temp

#include "U8glib.h"            // U8glib library for the OLED
#include <Wire.h>              // Wire library for I2C communication
#include <Adafruit_MLX90614.h> // MLX90614 library from Adafruit

//***********************************************HandWash

#include <Servo.h>
#define trigPin ?     // PWM||Analog  <?????>
#define echoPin ?     // Digital pin  <?????>

//***********************************************Weather

#include <IRremote.h>
#include <stdio.h>   //clock library
#include <string.h>  //clock library
#include <DS1302.h>  //clock library
#include <dht.h>     //dht11 library
#include <LiquidCrystal_I2C.h> //LCD library

#define lmPin ?     // LM35 attach to Analog pin  <?????>

//***********************************************Oximeter

#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ENABLE_MAX30100 ?  // digital pins or preferably TX pin

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...

#define OLED_RESET    ?    //   Digital pin as Reset pin
 
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#if ENABLE_MAX30100

#define REPORTING_PERIOD_MS     5000

PulseOximeter pox;
#endif
uint32_t tsLastReport = 0;
int xPos = 0;

//***********************************************Temp

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

U8GLIB_SH1106_128X64 u8g( ? , ? , ? , ? , ? ); // D0=Digital, D1=pwm||analog, CS=pwm||analog, DC=pwm||analog, Reset=digital

//***********************************************HandWash

Servo servo;
int sound = 250;

//***********************************************Weather

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display
dht DHT; //create a variable type of dht

const int DHT11_PIN = ? ;         // attach dht11 to Digital pin                       <?????>
int sensorPin = ? ;               // select the input Analog pin for the potentiometer <?????>

const int waterSensor = 0;        //set water sensor variable
int waterValue = 0; //variable for water sensor
int mmwaterValue = 0;
int luce = 0; //variable for the ldr
int pluce = 0; //variable for the ldr
float tem = 0; //variable for the temperature
long lmVal = 0; //variable for the LM35
//ir
const int irReceiverPin = ? ;  // Attach pin preferably pwm    <?????>

IRrecv irrecv(irReceiverPin);  //Creates a variable of type IRrecv
decode_results results;


//define clock variable
uint8_t RST_PIN   = ? ;   //RST pin attach to digital or pwm   <?????>
uint8_t SDA_PIN   = ? ;   //IO pin attach to digital or pwm    <?????>
uint8_t SCL_PIN = ? ;     //clk Pin attach to digital          <?????>
/* Create buffers */
char buf[50];
char day[10];


/* Create a DS1302 object */
DS1302 rtc(RST_PIN, SDA_PIN, SCL_PIN);//create a variable type of DS1302



void setup() 
{ 
  //***********************************************HandWash
  
  Serial.begin (115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  servo.attach( ? );               // Attach Servo pin (?)    <?????>

  //***********************************************Temp

  mlx.begin();  //Receive data from the sensor
  
  //***********************************************Oximeter

  Serial.println("SSD1306 128x64 OLED TEST");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  //display.display();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 18);
  // Display static text
  display.print("Pulse OxiMeter");
  int temp1 = 0;
  int temp2 = 40;
  int temp3 = 80;
  heart_beat(&temp1);
  heart_beat(&temp2);
  heart_beat(&temp3);
  xPos = 0;
  display.display();
  delay(1000); // Pause for 1 seconds
  display.cp437(true);
  display.clearDisplay();
  Serial.print("Initializing pulse oximeter..");
#if ENABLE_MAX30100
  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }
  // The default current for the IR LED is 50mA and it could be changed
  //   by uncommenting the following line. Check MAX30100_Registers.h for all the
  //   available options.
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);
  display_data(0, 0);
#endif

  //***********************************************Weather

  //clock
  rtc.write_protect(false);
  rtc.halt(false);
  //ir
  irrecv.enableIRIn();  //enable ir receiver module

  lcd.init();  //initialize the lcd
  lcd.backlight();  //open the backlight
  pinMode(sensorPin, INPUT);
  Time t(2022, 1, 18, 11, 20, 00, 7);//initialize the time
  /* Set the time and date on the chip */
  rtc.time(t);
  
} 

//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////END OF SETUP/////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void loop() 
{

  //***********************************************Temp

  u8g.firstPage();  
  do 
    {
     draw();      
    }
  while( u8g.nextPage() );
  
  delay(1000);  // Delay of 1sec 

  //***********************************************HandWash

  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  if (distance < 5) {
  Serial.println("the distance is less than 5");
  servo.write(90);
  }
  else {
   servo.write(0);
  }
  if (distance > 60 || distance <= 0)
  {
   Serial.println("The distance is more than 60");
  }
  else 
  {
   Serial.print(distance);
   Serial.println(" cm");
  }
  delay(500);
  
  //***********************************************Oximeter
  #if ENABLE_MAX30100
    // Make sure to call update as fast as possible
    pox.update();
    int bpm = 0;
    int spo2 = 0;
    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      //Serial.print("Heart rate:");
      bpm = pox.getHeartRate();
      spo2 = pox.getSpO2();
      Serial.println(bpm);
      //Serial.print("bpm / SpO2:");
      Serial.println(spo2);
      //Serial.println("%");
      tsLastReport = millis();
      display_data(bpm, spo2);
    }
  #endif
    drawLine(&xPos);  

  //***********************************************Weather

  lcd.setCursor(0, 0);
  lcd.print("A");
  delay(50);
  lcd.setCursor(1, 0);
  lcd.print("r");
  delay(50);
  lcd.setCursor(2, 0);
  lcd.print("d");
  delay(50);
  lcd.setCursor(3, 0);
  lcd.print("u");
  delay(50);
  lcd.setCursor(4, 0);
  lcd.print("i");
  delay(50);
  lcd.setCursor(5, 0);
  lcd.print("n");
  delay(50);
  lcd.setCursor(6, 0);
  lcd.print("o");
  delay(50);
  lcd.setCursor(8, 0);
  lcd.print("W");
  delay(50);
  lcd.setCursor(9, 0);
  lcd.print("e");
  delay(50);
  lcd.setCursor(10, 0);
  lcd.print("a");
  delay(50);
  lcd.setCursor(11, 0);
  lcd.print("t");
  delay(50);
  lcd.setCursor(12, 0);
  lcd.print("h");
  delay(50);
  lcd.setCursor(13, 0);
  lcd.print("e");
  delay(50);
  lcd.setCursor(14, 0);
  lcd.print("r");
  delay(50);
  lcd.setCursor(4, 1);
  lcd.print("S");
  delay(50);
  lcd.setCursor(5, 1);
  lcd.print("t");
  delay(50);
  lcd.setCursor(6, 1);
  lcd.print("a");
  delay(50);
  lcd.setCursor(7, 1);
  lcd.print("t");
  delay(50);
  lcd.setCursor(8, 1);
  lcd.print("i");
  delay(50);
  lcd.setCursor(9, 1);
  lcd.print("o");
  delay(50);
  lcd.setCursor(10, 1);
  lcd.print("n");
  delay(50);

  if (irrecv.decode(&results)) //if the ir receiver module receiver data
    {   
  if (results.value == 0xFF6897) //if "0" is pushed print TIME
  {
    lcd.clear(); //clear the LCD
    print_time();
    delay(1000); //delay 1000ms
    lcd.clear(); //clear the LCD
    delay (200); //wait for a while
    irrecv.resume();    // Receive the next value
  }
  if (results.value == 0xFF30CF) //if "1" is pushed print TEMPERATURE and HUMIDITY
  {
    lcd.clear(); //clear the LCD
    //READ DATA of the DHT
    int chk = DHT.read11(DHT11_PIN);
    // DISPLAY DATA
    lcd.setCursor(0, 0);
    lcd.print("Tem:");
    lmVal = analogRead(lmPin);//read the value of A1
    tem = (lmVal * 0.0048828125 * 100);  // 5/1024=0.0048828125;1000/10=100
    lcd.print(tem);//print tem
    lcd.print(char(223));//print the unit"  "
    lcd.print("C      ");
    // Serial.println(" C");
    lcd.setCursor(0, 1);
    lcd.print("Hum:");
    //Serial.print("Hum:");
    lcd.print(DHT.humidity, 1); //print the humidity on lcd
    //Serial.print(DHT.humidity,1);
    lcd.print(" %      ");
    //Serial.println(" %");
    delay(1000); //wait for 13000 ms
    lcd.clear(); //clear the LCD
    delay(200); //wait for a while
    irrecv.resume();    // Receive the next value
  }
  if (results.value == 0xFF18E7) //if "2" is pushed print the DARKNESS
  {
    lcd.clear(); //clear the LCD
    lcd.setCursor(4, 0); //place the cursor on 4 column, 1 row
    lcd.print("Darkness:");
    luce = analogRead(sensorPin); //read the ldr
    pluce = map(luce, 0, 1023, 0, 100); //the value of the sensor is converted into values from 0 to 100
    lcd.setCursor(6, 1); //place the cursor on the middle of the LCD
    lcd.print(pluce); //print the percentual
    lcd.print("%"); //print the symbol
    delay(3000); //delay 10000 ms
    lcd.clear(); //clear the LCD
    delay(200); //wait for a while
    irrecv.resume();    // Receive the next value
  }
  if (results.value == 0xFF7A85) //if "3" is pushed print the SNOW or WATER LEVEL
  {
    lcd.clear(); //clear the LCD
    lcd.setCursor(0, 0); //place the cursor on 0 column, 1 row
    lcd.print("Fluid level(mm):"); //print "Fluid level(mm):"
    int waterValue = analogRead(waterSensor); // get water sensor value
    lcd.setCursor(6, 1); //place cursor at 6 column,2 row
    mmwaterValue = map(waterValue, 0, 1023, 0, 40);
    lcd.print(mmwaterValue);  //value displayed on lcd
    delay(1000); //delay 1000ms
    lcd.clear(); //clear the LCD
    delay(200);
    irrecv.resume();    // Receive the next value
  }

  if (results.value == 0xFF9867) //if "PRESENTATION" is pushed print TIME, TEM and HUM, DARKNESS and S or W LEVEL one time
  {
    lcd.clear(); //clear the LCD
    print_time();
    delay(2000); //delay 2000ms
    lcd.clear(); //clear the LCD
    delay (200); //wait for a while

    //READ DATA of the DHT
    int chk = DHT.read11(DHT11_PIN);
    // DISPLAY DATA
    lcd.setCursor(0, 0);
    lcd.print("Tem:");
    lmVal = analogRead(lmPin);//read the value of A0
    tem = (lmVal * 0.0048828125 * 100);//5/1024=0.0048828125;1000/10=100
    lcd.print(tem);//print tem
    lcd.print(char(223));//print the unit"  "
    lcd.print("C      ");
    // Serial.println(" C");
    lcd.setCursor(0, 1);
    lcd.print("Hum:");
    //Serial.print("Hum:");
    lcd.print(DHT.humidity, 1); //print the humidity on lcd
    //Serial.print(DHT.humidity,1);
    lcd.print(" %      ");
    //Serial.println(" %");
    delay(1000); //wait for 1000 ms
    lcd.clear(); //clear the LCD
    delay(200); //wait for a while

    lcd.setCursor(4, 0); //place the cursor on 4 column, 1 row
    lcd.print("Darkness:");
    luce = analogRead(sensorPin); //read the ldr
    pluce = map(luce, 0, 1023, 0, 100); //the value of the sensor is converted into values from 0 to 100
    lcd.setCursor(6, 1); //place the cursor on the middle of the LCD
    lcd.print(pluce); //print the percentual
    lcd.print("%"); //print the symbol
    delay(1000); //delay 1000 ms
    lcd.clear(); //clear the LCD
    delay(200); //wait for a while

    lcd.setCursor(0, 0); //place the cursor on 0 column, 1 row
    lcd.print("Fluid level(mm):"); //print "Fluid level(mm):"
    int waterValue = analogRead(waterSensor); // get water sensor value
    lcd.setCursor(6, 1); //place cursor at 6 column,2 row
    mmwaterValue = map(waterValue, 0, 1023, 0, 40);
    lcd.print(mmwaterValue);  //value displayed on lcd
    delay(1000); //delay 1000ms
    lcd.clear(); //clear the LCD
    delay(200);
    irrecv.resume();    // Receive the next value
  }
  }
  
}

//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////END OF LOOP//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////



//***********************************************WeatherSubFunction

void print_time()
{
  /* Get the current time and date from the chip */
  Time t = rtc.time();
  /* Name the day of the week */
  memset(day, 0, sizeof(day));
  switch (t.day)
  {
    case 1:
      strcpy(day, "Sun");
      break;
    case 2:
      strcpy(day, "Mon");
      break;
    case 3:
      strcpy(day, "Tue");
      break;
    case 4:
      strcpy(day, "Wed");
      break;
    case 5:
      strcpy(day, "Thu");
      break;
    case 6:
      strcpy(day, "Fri");
      break;
    case 7:
      strcpy(day, "Sat");
      break;
  }
  /* Format the time and date and insert into the temporary buffer */
  snprintf(buf, sizeof(buf), "%s %04d-%02d-%02d %02d:%02d:%02d", day, t.yr, t.mon, t.date, t.hr, t.min, t.sec);
  /* Print the formatted string to serial so we can see the time */
  Serial.println(buf);
  lcd.setCursor(2, 0);
  lcd.print(t.yr);
  lcd.print("-");
  lcd.print(t.mon / 10);
  lcd.print(t.mon % 10);
  lcd.print("-");
  lcd.print(t.date / 10);
  lcd.print(t.date % 10);
  lcd.print(" ");
  lcd.print(day);
  lcd.setCursor(4, 1);
  lcd.print(t.hr);
  lcd.print(":");
  lcd.print(t.min / 10);
  lcd.print(t.min % 10);
  lcd.print(":");
  lcd.print(t.sec / 10);
  lcd.print(t.sec % 10);
}

//***********************************************TempSubFunction

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


//***********************************************OximeterSubFunctions


// Callback (registered below) when a pulse is detected
void onBeatDetected()
{
  Serial.println("Beat!");
  heart_beat(&xPos);
}

void display_data(int bpm, int spo2) {
 
display.fillRect(0, 18, 127, 15, BLACK);
  //if(bpm ==0 && spo2==0){
 //    display.setTextSize(1);
 // display.setTextColor(WHITE);
  //display.setCursor(0, 18);
  // Display static text
 // display.print("Fingure Out");
  //}
  display.fillRect(0, 18, 127, 15, BLACK);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 18);
  // Display static text
  display.print("BPM ");
  display.setTextSize(2);
  display.print(bpm);
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(64, 18);
  // Display static text
  display.print("Spo2% ");
  display.setTextSize(2);
  display.println(spo2);
  display.display();
}
void drawLine(int *x_pos) {
  // Draw a single pixel in white
  display.drawPixel(*x_pos, 8, WHITE);
  display.drawPixel((*x_pos)++, 8, WHITE);
  display.drawPixel((*x_pos)++, 8, WHITE);
  display.drawPixel((*x_pos)++, 8, WHITE);
  display.drawPixel((*x_pos), 8, BLACK);  // -----
  //Serial.println(*x_pos);
  display.fillRect(*x_pos, 0, 31, 16, BLACK);
  display.display();
  //delay(1);
  if (*x_pos >= SCREEN_WIDTH) {
    *x_pos = 0;
  }
}
void heart_beat(int *x_pos) {
  /************************************************/
  //display.clearDisplay();
  display.fillRect(*x_pos, 0, 30, 15, BLACK);
  // Draw a single pixel in white
  display.drawPixel(*x_pos + 0, 8, WHITE);
  display.drawPixel(*x_pos + 1, 8, WHITE);
  display.drawPixel(*x_pos + 2, 8, WHITE);
  display.drawPixel(*x_pos + 3, 8, WHITE);
  display.drawPixel(*x_pos + 4, 8, BLACK); // -----
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 5, 7, WHITE);
  display.drawPixel(*x_pos + 6, 6, WHITE);
  display.drawPixel(*x_pos + 7, 7, WHITE); // .~.
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 8, 8, WHITE);
  display.drawPixel(*x_pos + 9, 8, WHITE); // --
  //display.display();
  //delay(1);
  /******************************************/
  display.drawPixel(*x_pos + 10, 8, WHITE);
  display.drawPixel(*x_pos + 10, 9, WHITE);
  display.drawPixel(*x_pos + 11, 10, WHITE);
  display.drawPixel(*x_pos + 11, 11, WHITE);
  //display.display();
  //delay(1);
  /******************************************/
  display.drawPixel(*x_pos + 12, 10, WHITE);
  display.drawPixel(*x_pos + 12, 9, WHITE);
  display.drawPixel(*x_pos + 12, 8, WHITE);
  display.drawPixel(*x_pos + 12, 7, WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 13, 6, WHITE);
  display.drawPixel(*x_pos + 13, 5, WHITE);
  display.drawPixel(*x_pos + 13, 4, WHITE);
  display.drawPixel(*x_pos + 13, 3, WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 14, 2, WHITE);
  display.drawPixel(*x_pos + 14, 1, WHITE);
  display.drawPixel(*x_pos + 14, 0, WHITE);
  display.drawPixel(*x_pos + 14, 0, WHITE);
  //display.display();
  //delay(1);
  /******************************************/
  display.drawPixel(*x_pos + 15, 0, WHITE);
  display.drawPixel(*x_pos + 15, 1, WHITE);
  display.drawPixel(*x_pos + 15, 2, WHITE);
  display.drawPixel(*x_pos + 15, 3, WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 15, 4, WHITE);
  display.drawPixel(*x_pos + 15, 5, WHITE);
  display.drawPixel(*x_pos + 16, 6, WHITE);
  display.drawPixel(*x_pos + 16, 7, WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 16, 8, WHITE);
  display.drawPixel(*x_pos + 16, 9, WHITE);
  display.drawPixel(*x_pos + 16, 10, WHITE);
  display.drawPixel(*x_pos + 16, 11, WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 17, 12, WHITE);
  display.drawPixel(*x_pos + 17, 13, WHITE);
  display.drawPixel(*x_pos + 17, 14, WHITE);
  display.drawPixel(*x_pos + 17, 15, WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 18, 15, WHITE);
  display.drawPixel(*x_pos + 18, 14, WHITE);
  display.drawPixel(*x_pos + 18, 13, WHITE);
  display.drawPixel(*x_pos + 18, 12, WHITE);
  //display.display();
  //delay(1);
  display.drawPixel(*x_pos + 19, 11, WHITE);
  display.drawPixel(*x_pos + 19, 10, WHITE);
  display.drawPixel(*x_pos + 19, 9, WHITE);
  display.drawPixel(*x_pos + 19, 8, WHITE);
  //display.display();
  //delay(1);
  /****************************************************/
  display.drawPixel(*x_pos + 20, 8, WHITE);
  display.drawPixel(*x_pos + 21, 8, WHITE);
  //display.display();
  //delay(1);
  /****************************************************/
  display.drawPixel(*x_pos + 22, 7, WHITE);
  display.drawPixel(*x_pos + 23, 6, WHITE);
  display.drawPixel(*x_pos + 24, 6, WHITE);
  display.drawPixel(*x_pos + 25, 7, WHITE);
  //display.display();
  //delay(1);
  /************************************************/
  display.drawPixel(*x_pos + 26, 8, WHITE);
  display.drawPixel(*x_pos + 27, 8, WHITE);
  display.drawPixel(*x_pos + 28, 8, WHITE);
  display.drawPixel(*x_pos + 29, 8, WHITE);
  display.drawPixel(*x_pos + 30, 8, WHITE); // -----
  *x_pos = *x_pos + 30;
  display.display();
  delay(1);
}

//***********************************************EndOfSubFunctions*****************************************
