#include "DisplayUI.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

//PINS ANPASSEN WENN NÖTIG:
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8

static Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);
static uint32_t lastDrawMs = 0;
static const uint32_t UI_REFRESH_MS = 200; //5 Hz

static void drawHeader() {
  tft.fillScreen(ST7735_BLACK);
  tft.setTextWrap(false);
  tft.setTextSize(1); tft.setTextColor(ST7735_WHITE);
  tft.setCursor(2, 2);  tft.print("Cartesian Manip.");
  tft.drawLine(0, 12, tft.width(), 12, ST7735_WHITE);
}

static String fmtTime(uint32_t ms) {
  uint32_t s = ms / 1000;
  uint16_t hh = s / 3600;
  uint8_t  mm = (s % 3600) / 60;
  uint8_t  ss = s % 60;
  char buf[12];
  snprintf(buf, sizeof(buf), "%02u:%02u:%02u", hh, mm, ss);
  return String(buf);
}

void uiBegin() {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);    //Querformat
  drawHeader();
  tft.setCursor(2, 18); tft.print("Init...");
}

void uiUpdate(long x_steps, long y_steps,
              bool penDown,
              bool fVal, bool fCal, bool fBusy, bool fLim, bool fPow,
              uint32_t motionElapsedMs)
{
  if (millis() - lastDrawMs < UI_REFRESH_MS) return;
  lastDrawMs = millis();

  // mm aus Schritten (43.4444 Schritte/mm)
  float x_mm = x_steps / 43.4444f;
  float y_mm = y_steps / 43.4444f;

  //Bereich unter Header löschen
  tft.fillRect(0, 14, tft.width(), tft.height()-14, ST7735_BLACK);
  tft.setTextSize(1); tft.setTextColor(ST7735_WHITE);

  //Position
  tft.setCursor(2, 18);  tft.print("X: "); tft.print(x_mm, 1); tft.print(" mm");
  tft.setCursor(2, 30);  tft.print("Y: "); tft.print(y_mm, 1); tft.print(" mm");

  //Pen+Busy
  tft.setCursor(2, 44);  tft.print("Pen: "); tft.print(penDown ? "DOWN" : "UP");
  tft.setCursor(80,44);  tft.print("Busy: "); tft.print(fBusy ? "YES":"NO ");

  //Timer
  tft.setCursor(2, 58);  tft.print("Run: "); tft.print(fmtTime(motionElapsedMs));

  //Fehlerflags
  tft.setCursor(2, 74);  tft.print("F:V"); tft.print(fVal);
  tft.print(" C"); tft.print(fCal);
  tft.print(" B"); tft.print(fBusy);
  tft.print(" L"); tft.print(fLim);
  tft.print(" P"); tft.print(fPow);

  //Statuszeile
  tft.setCursor(2, 90);
  if (fCal) tft.setTextColor(ST7735_YELLOW); else tft.setTextColor(ST7735_GREEN);
  tft.print(fCal ? "Calibrating..." : "Ready.");
  tft.setTextColor(ST7735_WHITE);
}
