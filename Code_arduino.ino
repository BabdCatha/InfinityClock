#include <Adafruit_NeoPixel.h>
#include <DS3231.h>
#include <Wire.h>
#include <IRremote.h>
#include <EEPROM.h>

//defining the pins and number of lights on each strip
#define PINTime 8
#define PINDigits 6
#define NumberOfMinutes 60
#define NumberOfLights 37

//variables for the rainbow effect
uint8_t effStep = 0;
uint8_t effStart = millis();

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
int darkFor = 0;
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
  UpdateDigitsColor();

}

void TurnON(){
  newDelay = 450;
  
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
  pixelsTime.setPixelColor((hour%12)*5 + 1, pixelsTime.Color(colors[0],colors[1],colors[2]));
  pixelsTime.setPixelColor((hour%12)*5, pixelsTime.Color(colors[0],colors[1],colors[2]));
  if((hour%12)*5 == 0){
    pixelsTime.setPixelColor(59, pixelsTime.Color(colors[0],colors[1],colors[2]));
  }else{
    pixelsTime.setPixelColor((hour%12)*5 - 1, pixelsTime.Color(colors[0],colors[1],colors[2]));
  }
  pixelsTime.setPixelColor(minute, pixelsTime.Color(colors[3],colors[4],colors[5]));
  pixelsTime.show();  
}

uint8_t mode2() {
  if(millis() - effStart < 10 * (effStep)) return 0x00;
  float factor1, factor2;
  uint16_t ind;
  for(uint16_t j=0;j<60;j++) {
    ind = effStep + j * 1;
    switch((int)((ind % 60) / 20)) {
      case 0: factor1 = 1.0 - ((float)(ind % 60 - 0 * 20) / 20);
              factor2 = (float)((int)(ind - 0) % 60) / 20;
              pixelsTime.setPixelColor(j, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2);
              pixelsTime.show();
              break;
      case 1: factor1 = 1.0 - ((float)(ind % 60 - 1 * 20) / 20);
              factor2 = (float)((int)(ind - 20) % 60) / 20;
              pixelsTime.setPixelColor(j, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2);
              pixelsTime.show();
              break;
      case 2: factor1 = 1.0 - ((float)(ind % 60 - 2 * 20) / 20);
              factor2 = (float)((int)(ind - 40) % 60) / 20;
              pixelsTime.setPixelColor(j, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2);
              pixelsTime.show();
              break;
    }
  }
  if(effStep >= 60) {
    effStep = 0;
    effStart = millis();
    return 0x03;
  }
  else effStep++;
  return 0x01;
}

void mode3(){
  if(effStep == 0){
    for(int i=0; i<60; i++){
      pixelsTime.setPixelColor(i, pixelsTime.Color(colors[0], colors[1], colors[2]));
    }
    pixelsTime.setBrightness(255);
  }else if(effStep < 17){
    pixelsTime.setBrightness(pixelsTime.getBrightness()-15);
  }else if(effStep < 33){
    pixelsTime.setBrightness(pixelsTime.getBrightness()+15);
  }else{
    effStep = 0;
  }
  pixelsTime.show();
  
}

void mode4(){
  for(int i=0; i<NumberOfMinutes; i++){
    pixelsTime.setPixelColor(i, pixelsTime.Color(colors[0], colors[1], colors[2]));
  }
  for(int i=0; i<NumberOfLights; i++){
    pixelsDigits.setPixelColor(i, pixelsDigits.Color(colors[9], colors[10], colors[11]));
  }
  pixelsTime.show();
  pixelsDigits.show();
}

void DecreaseBrightness(){
  if(colors[12]>0){
    colors[12] = colors[12]-15;
    pixelsTime.setBrightness(colors[12]);
    pixelsDigits.setBrightness(colors[12]);
  }
}

void IncreaseBrightness(){
  if(colors[12]<255){
    colors[12] = colors[12]+15;
    pixelsTime.setBrightness(colors[12]);
    pixelsDigits.setBrightness(colors[12]);
  }
}

void UpdateDigitsColor(){
  for(int j=0; j<NumberOfLights; j++){
    pixelsDigits.setPixelColor(j, pixelsDigits.Color(colors[9], colors[10], colors[11]));
    pixelsDigits.show();
  }
}

void DetectNightTime(){
  int luminosity = analogRead(A1);
  
  if(luminosity <= 50 && darkFor <= 25){
    darkFor++;
  }else if(darkFor >= -5){
    darkFor--;
  }

  Serial.print(String(darkFor)+" ");
  Serial.print(String(luminosity)+ "\n");
  
  if(darkFor >= 10){
    pixelsTime.setBrightness(20);
    pixelsDigits.setBrightness(20);
  }else{
    pixelsTime.setBrightness(colors[12]);
    pixelsDigits.setBrightness(colors[12]);
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
  UpdateDigitsColor();
  switch(mode){
    case 1:
      mode1();
      if(nightDetection){
        DetectNightTime();
      }
      delay(newDelay);
    break;
    case 2:
      mode2();
      delay(newDelay);
    break;
    case 3:
      mode3();
      effStep++;
      delay(newDelay);
    break;
    case 4:
      mode4();
      delay(newDelay);
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
        colors[setMode*3] = colors[setMode*3]+15;
        UpdateDigitsColor();
      break;
      case 0xFF48B7: //Increase Green
        colors[1+(setMode*3)] = colors[1+(setMode*3)]+15;
        UpdateDigitsColor();
      break;
      case 0xFF6897: //Increase Blue
        colors[2+(setMode*3)] = colors[2+(setMode*3)]+15;
        UpdateDigitsColor();
      break;
      case 0xFF02FD: //Decrease Red
        colors[setMode*3] = colors[setMode*3]-20;
        UpdateDigitsColor();
      break;
      case 0xFF32CD: //Decrease Green
        colors[1+(setMode*3)] = colors[1+(setMode*3)]-15;
        UpdateDigitsColor();
      break;
      case 0xFF20DF: //Decrease Blue
        colors[2+(setMode*3)] = colors[2+(setMode*3)]-15;
        UpdateDigitsColor();
      break;
      case 0xFFB24D:
        pixelsTime.setBrightness(colors[12]);
        mode = 1;
        newDelay = 450;
      break;
      case 0xFF00FF:
        effStep = 0;
        effStart = millis();
        pixelsTime.setBrightness(colors[12]);
        mode = 2;
        newDelay = 100;
      break;
      case 0xFF58A7:
        effStep = 0;
        effStart = millis();
        mode = 3;
        newDelay = 100;
      break;
      case 0xFF30CF:
        mode = 4;
        newDelay = 5000;
      break;
      case 0xFF28D7:
        colors[0] = 255;
        colors[1] = 0;
        colors[2] = 0;
        colors[3] = 0;
        colors[4] = 255;
        colors[5] = 0;
        colors[6] = 0;
        colors[7] = 0;
        colors[8] = 255;
        colors[9] = 30;
        colors[10] = 30;
        colors[11] = 120;
        colors[12] = 128;
      break;
      case 0xFF50AF:
        nightDetection = true;
      break;
      case 0xFF38C7:
        pixelsTime.setBrightness(colors[12]);
        nightDetection = false;
      break;
      case 0xFF708F:
        switch(setMode){
          case 0:
            Clock.setHour(Clock.getHour(h12, PM) + 1);
          break;
          case 1:
            Clock.setMinute(Clock.getMinute() + 1);
          break;
          case 2:
            Clock.setSecond(Clock.getSecond() + 1);
          break;
        }
      break;
      case 0xFFF00F:
        switch(setMode){
          case 0:
            Clock.setHour(Clock.getHour(h12, PM) - 1);
          break;
          case 1:
            Clock.setMinute(Clock.getMinute() - 1);
          break;
          case 2:
            Clock.setSecond(Clock.getSecond() - 1);
          break;
        }
      break;
    }
    irrecv.resume();
  }

}
