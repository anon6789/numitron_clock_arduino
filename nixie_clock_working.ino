#include <Wire.h>
#include <DS3231.h>
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

// pins for leds
#define PIN_SWITCH         4
#define PIN_PIXELS         6
#define PIN_SWITCH_PULL_UP 7
#define NUMPIXELS          6
#define PIN_LATCH          11
#define PIN_CLOCK          12
#define PIN_DATA           10

// variables store date correction from serial
byte Year;
byte Month;
byte Date;
byte DoW;
byte Hour;
byte Minute;
byte Second;

//setup neopixel leds
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_PIXELS, NEO_GRB + NEO_KHZ800);

//clocks
RTClib myRTC;
DS3231 Clock;

// used to toggle second led
bool secondsOn = false;

void setup() 
{
  // setup pins for shift register
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);  
  pinMode(PIN_CLOCK, OUTPUT);

  // setup pins for light control switch
  pinMode(PIN_SWITCH_PULL_UP, OUTPUT);
  pinMode(PIN_SWITCH, INPUT);
  digitalWrite(PIN_SWITCH_PULL_UP, HIGH);

  pixels.begin();
  Serial.begin(9600);
  Wire.begin();

  showAnimation();
}

// lookup map for numbers. digit[0] is 0, digit[1] is 1 and so on
byte digit[10]= {B00100001, B11111001, B00010101, B10010001, B11001001, B10000011, B00000011, B11110001, B00000001, B11000001};

// map for different states of the animation
byte init_animation[6]= {B00100011, B00100101, B00101001, B00110001, B10100001, B01100001};


void loop() {
  while (true){

    if (Serial.available()) {
      handleSerialData(Year, Month, Date, DoW, Hour, Minute, Second);
    } else {
      updateTime();
    
      for (int i = 0; i <= 60; i++) {
        if (Serial.available()){
          break;
        }
        if (digitalRead(PIN_SWITCH) == HIGH){
           turnLightOn();
           toggleSeconds();
        } 
        else {
           turnLightOff();
           secondsOn = false;
        }
        delay(1000);
      }
    }
  }
}

void showAnimation(){
  for (int i = 0; i <= 12; i++) {
    for (int j = 0; j < 6; j++){
      digitalWrite(PIN_LATCH, LOW);
      shiftOut(PIN_DATA, PIN_CLOCK, LSBFIRST, init_animation[j]);
      shiftOut(PIN_DATA, PIN_CLOCK, LSBFIRST, init_animation[j]);
      shiftOut(PIN_DATA, PIN_CLOCK, LSBFIRST, init_animation[j]);
      shiftOut(PIN_DATA, PIN_CLOCK, LSBFIRST, init_animation[j]);
      digitalWrite(PIN_LATCH, HIGH);
      delay(70);
      turnLightOff();
    }
    turnLightOn();
  }    
}

void toggleSeconds(){
  if (secondsOn){
    pixels.setPixelColor(5, pixels.Color(0,0,0)); 
    pixels.show();
    secondsOn = false;
  } else {
    pixels.setPixelColor(5, pixels.Color(7,20,0));
    pixels.show();
    secondsOn = true;
  }
}

void turnLightOn(){
  for (int j = 0; j < 5; j++){
    pixels.setPixelColor(j, pixels.Color(60,255,0));
    pixels.show();
  }
}

void turnLightOff(){
  for (int j = 0; j < 6; j++){
    pixels.setPixelColor(j, pixels.Color(0,0,0));
    pixels.show();
  }
}

void updateTime() {
  DateTime now = myRTC.now();
  
  int m1 = (now.minute() / 10) % 10;
  int m2 = (now.minute() / 1) % 10;
  
  int h1 = (now.hour() / 10) % 10;
  int h2 = (now.hour() / 1) % 10;
  
  
  digitalWrite(PIN_LATCH, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, LSBFIRST, digit[h2]);
  shiftOut(PIN_DATA, PIN_CLOCK, LSBFIRST, digit[h1]);
  shiftOut(PIN_DATA, PIN_CLOCK, LSBFIRST, digit[m2]);
  shiftOut(PIN_DATA, PIN_CLOCK, LSBFIRST, digit[m1]);
  digitalWrite(PIN_LATCH, HIGH);
}

// from DS3231_set example
void handleSerialData(byte& Year, byte& Month, byte& Day, byte& DoW, 
    byte& Hour, byte& Minute, byte& Second) {
  // Call this if you notice something coming in on 
  // the serial port. The data coming in should be in 
  // the order YYMMDDwHHMMSS, with an 'x' at the end.
  boolean GotString = false;
  char InChar;
  byte Temp1, Temp2;
  char InString[20];

  byte j=0;
  while (!GotString) {
    if (Serial.available()) {
      InChar = Serial.read();
      InString[j] = InChar;
      j += 1;
      if (InChar == 'x') {
        GotString = true;
      }
    }
  }
  Serial.println(InString);
  // Read Year first
  Temp1 = (byte)InString[0] -48;
  Temp2 = (byte)InString[1] -48;
  Year = Temp1*10 + Temp2;
  // now month
  Temp1 = (byte)InString[2] -48;
  Temp2 = (byte)InString[3] -48;
  Month = Temp1*10 + Temp2;
  // now date
  Temp1 = (byte)InString[4] -48;
  Temp2 = (byte)InString[5] -48;
  Day = Temp1*10 + Temp2;
  // now Day of Week
  DoW = (byte)InString[6] - 48;   
  // now Hour
  Temp1 = (byte)InString[7] -48;
  Temp2 = (byte)InString[8] -48;
  Hour = Temp1*10 + Temp2;
  // now Minute
  Temp1 = (byte)InString[9] -48;
  Temp2 = (byte)InString[10] -48;
  Minute = Temp1*10 + Temp2;
  // now Second
  Temp1 = (byte)InString[11] -48;
  Temp2 = (byte)InString[12] -48;
  Second = Temp1*10 + Temp2;

  Clock.setClockMode(false);  // set to 24h
  
  Clock.setYear(Year);
  Clock.setMonth(Month);
  Clock.setDate(Date);
  Clock.setDoW(DoW);
  Clock.setHour(Hour);
  Clock.setMinute(Minute);
  Clock.setSecond(Second);

  //empty serial buffer
  while(Serial.available()) {
    Serial.read();
  }  
}
