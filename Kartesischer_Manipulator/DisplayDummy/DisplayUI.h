#pragma once
#include <Arduino.h>

void uiBegin();  //Display initialisieren
void uiUpdate(long x_steps, long y_steps,
              bool penDown,
              bool fVal, bool fCal, bool fBusy, bool fLim, bool fPow,
              uint32_t motionElapsedMs);
