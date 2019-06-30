#include <Adafruit_NeoPixel.h>
#include <DS3231.h>
#include <Wire.h>
#include <IRremote.h>
#include <EEPROM.h>

//defining the pins and number of lights on each strip
#define PINTime 4
#define PINDigits 5
#define NumberOfMinutes 60
#define NumberOfLights 45

//defining the time variables
int second,minute,hour;
bool h12,PM;

//defining the remote variables
const int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);
decode_results results;

Adafruit_NeoPixel pixelsTime(NumberOfMinutes, PINTime, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsDigits(NumberOfLights, PINDigits, NEO_GRB + NEO_KHZ800);

DS3231 Clock;

bool nightTime = false;
bool nightDetection = true;
char mode = 1;
char setMode = 0;

int newDelay = 500;
int lastUpdate = 0;

//defining the diferent colors
char colors[13];

void TurnOFF(){
  //Setting a new delay to save power
  newDelay = 5000;

  //Saving the colors and turning the off
  for(int i = 0; i<13; i++){
    EEPROM.update(i, colors[i]);
    colors[i] = 0;
  }

}

void TurnON(){
  newDelay = 500;
  
  for(int i = 0; i<13; i++){
    colors[i] = EEPROM.read(i);
  }
  
  UpdateDigitsColor();
}

void mode1(){
  pixelsTime.clear(); // Set all pixel colors to 'off'
  second = Clock.getSecond(); //Getting the current time
  minute = Clock.getMinute();
  hour = Clock.getHour(h12, PM);

  pixelsTime.setPixelColor(second, pixelsTime.Color(colors[6],colors[7],colors[8]));
  pixelsTime.setPixelColor(minute, pixelsTime.Color(colors[3],colors[4],colors[5]));
  pixelsTime.setPixelColor((hour%12)*5, pixelsTime.Color(colors[0],colors[1],colors[2]));

  pixelsTime.show();  
}

void mode2() {
}

void DecreaseBrightness(){
  colors[12] = colors[12]-20;
  pixelsTime.setBrightness(colors[12]);
  pixelsDigits.setBrightness(colors[12]);
}

void IncreaseBrightness(){
  colors[12] = colors[12]+20;
  pixelsTime.setBrightness(colors[12]);
  pixelsDigits.setBrightness(colors[12]);
}

void UpdateDigitsColor(){
  for(int j=0; j<NumberOfLights; j++){
    pixelsDigits.setPixelColor(j, pixelsDigits.Color(colors[9], colors[10], colors[11]));
    pixelsDigits.show();
  }
}

void setup() {

  Serial.begin(9600);
  Wire.begin();

  pixelsTime.begin();
  pixelsDigits.begin();

  TurnON();

  //Setting the color of the digits
  UpdateDigitsColor();

  irrecv.enableIRIn();
  irrecv.blink13(true);

}

void loop() {
  switch(mode){
    case 1:
      mode1();
      delay(newDelay);
    break;
    case 2:
      mode2();
    break;
  }
  

  if(irrecv.decode(&results)){
    Serial.println(results.value, HEX);
    switch(results.value){
      case 0xFFB04F:
        TurnON();
      break;
      case 0xFFF807:
        TurnOFF();
      break;
      case 0xFF9867:
        setMode = 0;
      break;
      case 0xFFD827:
        setMode = 1;
      break;
      case 0xFF8877:
        setMode = 2;
      break;
      case 0xFFA857:
        setMode = 3;
      break;
      case 0xFF906F:
        IncreaseBrightness();
      break;
      case 0xFFB847:
        DecreaseBrightness();
      break;
      case 0xFFE817: //Increase Red
        colors[setMode*3] = colors[setMode*3]+20;
        UpdateDigitsColor();
      break;
      case 0xFF48B7: //Increase Green
        colors[1+(setMode*3)] = colors[1+(setMode*3)]+20;
        UpdateDigitsColor();
      break;
      case 0xFF6897: //Increase Blue
        colors[2+(setMode*3)] = colors[2+(setMode*3)]+20;
        UpdateDigitsColor();
      break;
      case 0xFF02FD: //Decrease Red
        colors[setMode*3] = colors[setMode*3]-20;
        UpdateDigitsColor();
      break;
      case 0xFF32CD: //Decrease Green
        colors[1+(setMode*3)] = colors[1+(setMode*3)]-20;
        UpdateDigitsColor();
      break;
      case 0xFF20DF: //Decrease Blue
        colors[2+(setMode*3)] = colors[2+(setMode*3)]-20;
        UpdateDigitsColor();
      break;
      case 0xFFB24D:
        mode = 1;
      break;
      case 0xFF00FF:
        mode = 2;
      break;
    }
    irrecv.resume();
  }

}
