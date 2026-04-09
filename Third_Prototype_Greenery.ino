#include <Multichannel_Gas_GMXXX.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define GPSECHO  true

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
String receivedData = "";

#ifndef DISABLE_FS_H_WARNING
#define DISABLE_FS_H_WARNING  // Disable warning for type File not defined. 
#endif  // DISABLE_FS_H_WARNING 
#include "SdFat.h"

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3
/*
  Change the value of SD_CS_PIN if you are using SPI and
  your hardware does not use the default value, SS.
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else   // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

// Try to select the best SD card configuration.
#if defined(HAS_TEENSY_SDIO)
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif defined(HAS_BUILTIN_PIO_SDIO)
// See the Rp2040SdioSetup example for boards without a builtin SDIO socket.
#define SD_CONFIG SdioConfig(PIN_SD_CLK, PIN_SD_CMD_MOSI, PIN_SD_DAT0_MISO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_TEENSY_SDIO
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_TEENSY_SDIO

#ifdef SOFTWAREWIRE
    #include <SoftwareWire.h>
    SoftwareWire myWire(3, 2);
    GAS_GMXXX<SoftwareWire> gas;
#else
    #include <Wire.h>
    GAS_GMXXX<TwoWire> gas;
#endif

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

char line[40];


const int buttonPinLog = 5; // Button connected to digital pin 6
const int buttonPinGas = 6; // Button connected to digital pin 7
const int ledPin = 4;   // LED connected to digital pin 5

int buttonStateLog = 0;
int buttonStateGas = 0;
int lastButtonStateLog = 0;
int lastButtonStateGas = 0;
int mode = 0;  
int chosenGas = 0;

SoftwareSerial mySerial(8, 7);
Adafruit_GPS GPS(&mySerial);


boolean ledState = false; // Track the current state of the LED (on/off)

static uint8_t recv_cmd[8] = {};

void setup() {

Serial.begin(115200);

GPS.begin(9600);

GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); 

GPS.sendCommand(PGCMD_ANTENNA);

delay(1000);

mySerial.println(PMTK_Q_RELEASE);

gas.begin(Wire, 0x08);
  // Initialize the SD.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
    return;
  }
  // Remove any existing file.
  if (sd.exists("DataCollection.csv")) {
    sd.remove("DataCollection.csv");
  }
  
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(28, OUTPUT);
  pinMode(29, OUTPUT);
  pinMode(30, OUTPUT);
  pinMode(31, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(34, OUTPUT);
  pinMode(35, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);
  pinMode(38, OUTPUT);
  pinMode(39, OUTPUT);
  pinMode(40, OUTPUT);
  pinMode(41, OUTPUT);
  pinMode(7, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Ensure the LED starts OFF

  Serial.println("Running");

display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //c
display.clearDisplay();
display.setTextColor(SSD1306_WHITE);
display.setTextSize(4);
display.setCursor(8, 28);
display.display();


}

void loop() {

buttonStateLog = digitalRead(buttonPinLog);
buttonStateGas = digitalRead(buttonPinGas);

uint8_t len = 0;
uint8_t addr = 0;
uint8_t i;
uint32_t val = 0;
int loudNess = analogRead(A0);
int carBon = gas.measure_CO();
int nitroGen = gas.measure_NO2();
int alcoHol = gas.measure_C2H5OH();

loudNess = map(loudNess, 0, 500, 0, 115);

val = gas.measure_NO2(); Serial.print("NO2: "); Serial.print(val); Serial.print("  =  ");
Serial.print(gas.calcVol(val)); Serial.println("V");
val = gas.measure_C2H5OH(); Serial.print("C2H5OH: "); Serial.print(val); Serial.print("  =  ");
Serial.print(gas.calcVol(val)); Serial.println("V");
val = gas.measure_VOC(); Serial.print("VOC: "); Serial.print(val); Serial.print("  =  ");
Serial.print(gas.calcVol(val)); Serial.println("V");
val = gas.measure_CO(); Serial.print("CO: "); Serial.print(val); Serial.print("  =  ");
Serial.print(gas.calcVol(val)); Serial.println("V");
Serial.println(loudNess);

Serial.print("\nTime: ");
    if (GPS.hour < 10) { Serial.print('0'); }
    Serial.print(GPS.hour, DEC); Serial.print(':');
    if (GPS.minute < 10) { Serial.print('0'); }
    Serial.print(GPS.minute, DEC); Serial.print(':');
    if (GPS.seconds < 10) { Serial.print('0'); }
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    if (GPS.milliseconds < 10) {
      Serial.print("00");
    } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
      Serial.print("0");
    }
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);

      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
      Serial.print("Antenna status: "); Serial.println((int)GPS.antenna);
    }

display.setTextSize(1);
display.setCursor(0, 0);
display.display();
display.clearDisplay();
display.setCursor(0, 0);
display.print("DB: "); 
display.println(loudNess);
display.print("C0: ");
display.println(gas.measure_CO());
display.print("NO2: ");
display.println(gas.measure_NO2());
display.print("C2H5OH: ");
display.println(gas.measure_C2H5OH());
display.print("Altitude: "); display.println(GPS.altitude);
display.print("Satellites: "); display.println((int)GPS.satellites);
display.print("Antenna status: "); display.println((int)GPS.antenna);
display.print("Loc:");
display.print(GPS.latitude, 4); display.print(GPS.lat);
display.print(", ");
display.print(GPS.longitude, 4); display.println(GPS.lon);


carBon = map(carBon, 0, 500, 0, 100);
nitroGen = map(nitroGen, 0, 500, 0, 100);
alcoHol = map(alcoHol, 0, 500, 0, 100);

//Button controlling logging
if (buttonStateLog == LOW && lastButtonStateLog == HIGH) {
  ledState = !ledState;
  if (ledState) {      
  digitalWrite(ledPin, HIGH);

if (file.open("DataCollection.csv", FILE_WRITE)) {
  Serial.println("Collection Started");
}

  } else {               
    digitalWrite(ledPin, LOW);

    file.close();
    Serial.println("Collection Stopped");
  }
}

lastButtonStateLog = buttonStateLog;


if (ledState && file) {
  file.print(loudNess);
  file.print(",");

  file.print(gas.measure_CO());
  file.print(",");

  file.print(gas.measure_NO2());
  file.print(",");

  file.print(gas.measure_C2H5OH());
  file.print(",");

  file.print(GPS.altitude);
  file.print(",");

  file.print(GPS.latitude);
  file.print(",");

  file.println(GPS.longitude);
}

if (buttonStateGas == LOW && lastButtonStateGas == HIGH) {
  mode++;

if (mode > 2) mode = 0;
}

lastButtonStateGas = buttonStateGas;

if (mode == 0) {
  chosenGas = carBon;
}
else if (mode == 1) {
  chosenGas = nitroGen;
} 
else if (mode == 2) {
  chosenGas = alcoHol;
}

//DB corresponding to LEDs, visual feedback
if (loudNess >= 0) {
  digitalWrite(23, HIGH);
}
else{
  digitalWrite(23, LOW);
}
if (loudNess >= 5) {
  digitalWrite(25, HIGH);
}
else{
  digitalWrite(25, LOW);
}
if (loudNess >= 10) {
  digitalWrite(27, HIGH);
}
else{
  digitalWrite(27, LOW);
}
if (loudNess >= 20) {
  digitalWrite(29, HIGH);
}
else{
  digitalWrite(29, LOW);
}
if (loudNess >= 30) {
  digitalWrite(31, HIGH);
}
else{
  digitalWrite(31, LOW);
}
if (loudNess >= 40) {
  digitalWrite(33, HIGH);
}
else{
  digitalWrite(33, LOW);
}
if (loudNess >= 50) {
  digitalWrite(35, HIGH);
}
else{
  digitalWrite(35, LOW);
}
if (loudNess >= 65) {
  digitalWrite(37, HIGH);
}
else{
  digitalWrite(37, LOW);
}
if (loudNess >= 80) {
  digitalWrite(39, HIGH);
}
else{
  digitalWrite(39, LOW);
}
if (loudNess >= 100) {
  digitalWrite(41, HIGH);
}
else{
  digitalWrite(41, LOW);
}

//Chosen Gas corresponding to LEDs, visual feedback
if (chosenGas >= 0) {
  digitalWrite(22, HIGH);
}
else{
  digitalWrite(22, LOW);
}
if (chosenGas >= 5) {
  digitalWrite(24, HIGH);
}
else{
  digitalWrite(24, LOW);
}
if (chosenGas >= 10) {
  digitalWrite(26, HIGH);
}
else{
  digitalWrite(26, LOW);
}
if (chosenGas >= 20) {
  digitalWrite(28, HIGH);
}
else{
  digitalWrite(28, LOW);
}
if (chosenGas >= 30) {
  digitalWrite(30, HIGH);
}
else{
  digitalWrite(30, LOW);
}
if (chosenGas >= 40) {
  digitalWrite(32, HIGH);
}
else{
  digitalWrite(32, LOW);
}
if (chosenGas >= 50) {
  digitalWrite(34, HIGH);
}
else{
  digitalWrite(34, LOW);
}
if (chosenGas >= 65) {
  digitalWrite(36, HIGH);
}
else{
  digitalWrite(36, LOW);
}
if (chosenGas >= 75) {
  digitalWrite(38, HIGH);
}
else{
  digitalWrite(38, LOW);
}
if (chosenGas >= 90) {
  digitalWrite(40, HIGH);
}
else{
  digitalWrite(40, LOW);
}



delay(20);

}


