#include <EEPROM.h>
#include <string.h>
#include <math.h>

//This function will write a 2 byte integer to the eeprom at the specified address and address + 1
void EEPROMWriteInt(int p_address, int p_value)
  {
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);

  EEPROM.write(p_address, lowByte);
  EEPROM.write(p_address + 1, highByte);
  }

//This function will read a 2 byte integer from the eeprom at the specified address and address + 1
unsigned int EEPROMReadInt(int p_address)
  {
  byte lowByte = EEPROM.read(p_address);
  byte highByte = EEPROM.read(p_address + 1);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
  }
  
//Pins:
int RGBPins[3] = { 5, 6, 7 };
int RGBLEDPins[3] = {9 , 10, 11};
int inputPin = A1;

//Calibration value:
int whiteLimits[3];
int blackLimits[3];

//Corol struct RGB format:
struct RGB{
  int red;
  int green;
  int blue;
};

int start_time = 0;
int stop_time = 0;
bool is_checked = false;


//Color set funcrion to diod
void setColor(int colorPin){
  digitalWrite(colorPin, HIGH);
 }

void printLimitsToRom() {
  int wbyte_address = 0;
  int bbyte_address = 6;
  for(int i = 0; i < 3; ++i) {
    EEPROMWriteInt(wbyte_address, whiteLimits[i]);
    wbyte_address += 2;
    EEPROMWriteInt(bbyte_address, blackLimits[i]);
    bbyte_address += 2;
  }
}

//Calibration funcrion:
void calibrate(){
  int readByte = 0;
  while((readByte = Serial.read()) != '2'){
    delay(100);   
  }
  for(int i = 0; i < 3; ++i){
    int Summ = 0;
    digitalWrite(RGBPins[i], HIGH);
    delay(100);
    for(int i = 0; i < 5; ++i) {
      delay(30);
      Summ += analogRead(inputPin);
    }
    whiteLimits[i] = Summ/5;
    digitalWrite(RGBPins[i], LOW);
  }
  while((readByte = Serial.read()) != '3'){
    //Serial.print(readByte);
    delay(100);   
  }
  for(int i = 0; i < 3; ++i){
    int Summ = 0;
    digitalWrite(RGBPins[i], HIGH);
    delay(100);
    for(int i = 0; i < 5; ++i) {
      delay(30);
      Summ += analogRead(inputPin);
    }
    blackLimits[i] = Summ/5;
    digitalWrite(RGBPins[i], LOW);
  }
  printLimitsToRom();
}

//Color check function:
int checkColor(int colorPin) {
  digitalWrite(colorPin, HIGH);
  delay(100);
  int result = analogRead(inputPin);
  digitalWrite(colorPin, LOW);
  return result;
}

//Color Translate function:
struct RGB readColor() {
  float rgb[3];
  for(int i = 0; i < 3; ++i) {
    rgb[i] = checkColor(RGBPins[i]);
    if (rgb[i] > whiteLimits[i]) {
      whiteLimits[i] = rgb[i];
    }
    if(rgb[i] < blackLimits[i]) {
      blackLimits[i] = rgb[i];
    }
    if(whiteLimits[i] == blackLimits[i]) {
      rgb[i] = 0;
    } else {
      rgb[i] = ((rgb[i] - blackLimits[i])/(whiteLimits[i] - blackLimits[i]))*255;
    }
  }
  printLimitsToRom();
  return RGB{rgb[0], rgb[1], rgb[2]};
}

//Change RGB LED
void print_color_to_led(RGB color) {
  analogWrite(RGBLEDPins[0], 255-color.red);
  analogWrite(RGBLEDPins[1], 255-color.green);
  analogWrite(RGBLEDPins[2], 255-color.green);
}

void setup(){
  for(int i = 0; i < 3; ++i) { 
    pinMode(RGBPins[i], OUTPUT);
    pinMode(RGBLEDPins[i], OUTPUT);
  }
  pinMode(inputPin, INPUT);
  pinMode(A2, OUTPUT);
  digitalWrite(A2, HIGH);
  digitalWrite(tumblerIn, HIGH);
  int wbyte_address = 0;
  int bbyte_address = 6;
  for(int i = 0; i < 3; ++i) {
    whiteLimits[i] = EEPROMReadInt(wbyte_address);
    wbyte_address += 2;
    blackLimits[i] = EEPROMReadInt(bbyte_address);
    bbyte_address += 2;
  }
  is_checked = false;
  Serial.begin(9600);
}
void loop() {
    stop_time = millis();
    if(stop_time - start_time < 1500&&is_checked){
      is_checked = false;
      calibrate();
    }
    int do_calibration = 0;
    if(Serial.available() > 0) {
      do_calibration = Serial.read();
      Serial.print(do_calibration);
      if(do_calibration == '1') {
          calibrate();
          }
      }
    RGB rColor = readColor();
    String a = String(rColor.red) + ' ' + String(rColor.green) + ' ' + String(rColor.blue) + '\n';
    Serial.print(a);
    print_color_to_led(rColor);
    delay(1000);
  }
