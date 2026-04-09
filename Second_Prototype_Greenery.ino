#include <Multichannel_Gas_GMXXX.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
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


const int buttonPin = 2; // Button connected to digital pin 2
const int ledPin = 7;   // LED connected to digital pin 7

int buttonState = 0;
int lastButtonState = 0;

boolean ledState = false; // Track the current state of the LED (on/off)

static uint8_t recv_cmd[8] = {};

void setup() {

Serial.begin(9600);
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
  
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Ensure the LED starts OFF

  Serial.println("Running");

  // Rewind file for read.
display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //c
display.clearDisplay();
display.setTextColor(SSD1306_WHITE);
display.setTextSize(4);
display.setCursor(8, 28);
display.display();

}

void loop() {

buttonState = digitalRead(buttonPin);

uint8_t len = 0;
uint8_t addr = 0;
uint8_t i;
uint32_t val = 0;
int loudNess = analogRead(A0);

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

display.setTextSize(2);
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

  while (buttonState == LOW && lastButtonState == HIGH) {
    if (ledState == false) { 
      // Send "ON" command when LED turns on
      digitalWrite(ledPin, HIGH);
      ledState = true;
      (!file.open("DataCollection.csv", FILE_WRITE)); {
        file.println(
      F(loudNess));
        file.println(
      F(gas.measure_CO()));
        file.println(
      F(gas.measure_NO2()));
        file.println(
      F(gas.measure_C2H5OH()));
      }
    }
    else {
     // Send "OFF" command when LED turns off
      digitalWrite(ledPin, LOW);
      ledState = false;
        file.rewind();
  file.close();
  Serial.println(F("Done"));
    }
  }
  
  lastButtonState = buttonState;

  delay(10); 

}
