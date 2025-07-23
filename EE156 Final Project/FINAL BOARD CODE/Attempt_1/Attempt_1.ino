#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

// Pin definitions for Arduino Uno
#define TFT_CS        10
#define TFT_RST        -1 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(9600);
  Serial.println(F("Hello! ST77xx TFT Test"));
  tft.initR(INITR_MINI160x80_PLUGIN); // Init ST7735S mini display
  tft.fillScreen(ST77XX_RED); // Fill the screen with blue color
}

void loop() {
  tft.invertDisplay(true);
  delay(500);
  tft.invertDisplay(false);
  tft.fillScreen(ST77XX_RED); // Fill the screen with blue color

  delay(500);
  Serial.println("iterate");
}
