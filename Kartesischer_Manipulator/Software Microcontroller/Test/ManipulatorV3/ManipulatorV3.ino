/*
Hochschule Bielefeld
Projekt kartesischer Manipulator
Letze Änderung 12.12.2024

Funktion:

Einlesen von Koordinaten über eine Serielle Verbindung.
Anschließend anfahren dieses Punktes auf dem kürzesten (direkten) Weg.
Kommunikation mit Client(Linux-PC) nach den folgenden Protokoll:

Serielle Kommunikation mit Client mit BAUD 115200
String Input Format: x0000y0000p0r0e 
      (x[Koordinate X-Richtung *10]y[Koordinate Y-Richtung *10]p[0-> Pen up/ 1-> Pen down]r{1-> Reset Fehler}e)
String Output Format Zeile 2: Rückgabe Eingabestring
String Output Format Zeile 2: f00000e (wenn 1 --> fehler == true;1.empfangener String fehlerhaft,2. Plotter ist noch am kalibrieren, 3.Plotter ist noch am anfahren der vorherigen Position
                                       4.Endlagenschalter wurde unerwartet betätigt,5.Betriebsspannung 230V fehlt)
TODO:
- Kontrolle der eingebene Werte auf Buchstaben
- Implementrierung Erkennung aller Fehler
*/

#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0



#define USE_TIMER_2 true
#warning Using Timer2

#include "TimerInterrupt.h"
#include "ISR_Timer.h"
#include "MeOrion.h"  //Bibliothek der Makeblock Steuerplatine URL: https://github.com/Makeblock-official/Makeblock-Libraries

//Konfiguration der Schrittmotoren und Endlagenschalter
MeStepper stepperY(PORT_1);          //Schrittmotor Y-Achse; Port steht jeweils auf dem Anschluss an der Steuerplatine
MeStepper stepperX(PORT_2);          //Schrittmotor X-Achse
MeLimitSwitch limitSwitchY(PORT_6);  //Endlagenschalter Nullpunkt Y-Achse
MeLimitSwitch limitSwitchX(PORT_4);  //Endlagenschalter Nullpunkt X-Achse


String buffer = "";            //Puffer Speicher zum einlesen der Koordinaten
float xInMm = 0;               //Soll Koordinate X-Achse
float yInMm = 0;               //Soll Koordinate Y-Achse
bool failValue = false;        //Variable für Fehlermeldung: empfangener String fehlerhaft
bool failCalibration = false;  //Variable für Fehlermeldung: Plotter ist noch am kalibrieren
bool failBusy = false;         //Variable für Fehlermeldung: Plotter ist noch am anfahren der vorherigen Position
bool failLimitSwitch = false;  //Variable für Fehlermeldung: Endlagenschalter wurde unerwartet betätigt
bool failPower = false;        //Variable für Fehlermeldung: Betriebsspannung 230V fehlt

// Init ISR_Timer
ISR_Timer ISR_timer;

#define TIMER_INTERVAL_MS 5L

#define TIMER_INTERVAL_1S 1000L
#define TIMER_INTERVAL_2S 2000L
#define TIMER_INTERVAL_5S 5000L

void TimerHandler() {
  ISR_timer.run();
}

#define NUMBER_ISR_TIMERS         1

void doingSomething0() {
  if (Serial.available() > 14 && failBusy == false) {
    buffer = Serial.readString();  //String in Puffervariable einlesen

    Serial.println(buffer);  //Rückgabe des Empfangenen Strings


    /* TODO Fehlermeldug Endlagenschalter
    while (stepperX.run() || stepperY.run()) 
    {
      if (failCalibration == false && (limitSwitchX.touched() || limitSwitchY.touched())) 
      {
      failLimitSwitch = true; // Endlagenschalter ausgelöst
      break; // 
      }
    }
    */
    //Kontrolle ob String den erwarteten Format entspricht
    if (buffer.length() == 15 && buffer[0] == 'x' && buffer[5] == 'y' && buffer[10] == 'p' && buffer[12] == 'r' && buffer[14] == 'e')  //Aufbau Beispiel x5200y5200p1e
    {
      //Zerlegen des Strings in die einzelnen Werte
      float inputX = 0.1 * (buffer.substring(1, 5).toFloat());
      float inputY = 0.1 * (buffer.substring(6, 10).toFloat());
      bool pendown = buffer.substring(12, 13).toInt();
      if (buffer.substring(14, 15).toInt() == 1)  //Wenn Reset == true
      {
        failLimitSwitch = false;  // Rücksetzung Kalibrierfehler
      }

      if (inputX < 0 || inputX > 520)  //Wenn Wert X falsch
      {
        failValue = 1;                        //Wertebereich! Fehler setzen
      } else if (inputY < 0 || inputY > 530)  // Wenn Wert Y falsch
      {
        failValue = 1;                                                                             //Wertebereich! Fehler setzen
      } else if (failCalibration == 0 && failBusy == 0 && failLimitSwitch == 0 && failPower == 0)  //Wenn kein Fehler vorliegt
      {
        xInMm = inputX * 43.4444;  // Umrechnung der mm in Schritte
        yInMm = inputY * 43.4444;
      }
      failBusy = true;
      moveToNewPosition(xInMm, yInMm);  //Funktionsaufruf Diagonale Bewegung zum neuen Punkt
    } else {
      failValue = 1;
    }
    failValue = 0;
    failPower = 0;
    //Ausgabe Fehler String
    Serial.print("f");
    Serial.print(failValue);
    Serial.print(failCalibration);
    Serial.print(failBusy);
    Serial.print(failLimitSwitch);
    Serial.print(failPower);
    Serial.println("e");
    return;
  };
}


void setup() {
  //Serielle Verbindung initialisieren
  Serial.begin(115200);  //Beginn Kommunikation mit Client (Linux PC)

  //Schrittmotoren konfigurieren
  stepperX.setMaxSpeed(3000);       //Schrittmotor X-Achse maximal mögliche Geschwindigkeit [U/min]
  stepperX.setAcceleration(20000);  //Schrittmotor X-Achse Beschleunigung
  stepperY.setMaxSpeed(3000);       //Schrittmotor Y-Achse maximal mögliche Geschwindigkeit [U/min]
  stepperY.setAcceleration(20000);  //Schrittmotor Y-Achse Beschleunigung

  stepperX.setSpeed(3000);  //Schrittmotor X-Achse Einstellung Geschwindigkeit [U/min]
  stepperY.setSpeed(3000);  //Schrittmotor Y-Achse Einstellung Geschwindigkeit [U/min]

  //Refferenzfahrt der X- und Y-Achse
  home_steppers();  //Funktionsaufruf für Initialisierungsfahrt


  ITimer2.init();

  if (ITimer2.attachInterruptInterval(TIMER_INTERVAL_MS, TimerHandler)) {
    Serial.print(F("Starting  ITimer2 OK"));
  } else
    Serial.println(F("Can't set ITimer2. Select another freq. or timer"));
}


/*
  Funktion für die Initialisierungsfahrt der X- und Y-Achse

  Fährt zuerst die X-Achse immer 20 Schritte in Richtung Endlage(vom Nullpunkt) bis Endlage betätigt ist.
  Danach selbiges für die Y-Achse.
  Es werden immer 20 Schritte aufeinmal gefahren um eine höhere Geschwindigkeit der Kalibierung zu erziehlen.
  Anschließend wird die Funktion steppback_home_steppers die genaue Nullposition anzufahren und diese Werte als neuen Nullpunkt zuspeichern
  */
void home_steppers() {
  failCalibration = 1;
  while (!limitSwitchX.touched())  //Solange Endlage noch nicht berührt
  {
    stepperX.move(-20);  //mit jeweils 20 Schritten an den Endlagenschalter rantasten
    stepperX.run();
  }

  while (!limitSwitchY.touched())  //Solange Endlage noch nicht berührt
  {
    stepperY.move(-20);  //mit jeweils 20 Schritten an den Endlagenschalter rantasten
    stepperY.run();
  }

  steppback_home_steppers();  //Funktion siehe unten
}

/*
  Funktion für Feinkalibrierung der Initialisierungsfahrt der X- und Y-Achse

  Fährt zuerst die X-Achse immer einen Schritt zurück bis die Endlage nicht mehr gedrückt ist.
  Danach selbiges für die Y-Achse.
  Um den Fehler durch das Verfahren von jeweils 20 Schritten in der Funktion home_steppers auszugleichen.
  Anschließend wird die aktuelle Position der Achsen als neuer Nullpunkt definiert.
  */
void steppback_home_steppers() {
  while (limitSwitchX.touched())  //mit jeweils 1 Schritt zurück bis kein Signal der Endlage mehr
  {
    stepperX.move(1);
    stepperX.run();
  }
  while (limitSwitchY.touched())  //mit jeweils 1 Schritt zurück bis kein Signal der Endlage mehr
  {
    stepperY.move(1);
    stepperY.run();
  }
  stepperX.setCurrentPosition(0);  // Setzt die aktuelle Position als Null
  stepperY.setCurrentPosition(0);  // Setzt die aktuelle Position als Null
  failCalibration = 0;
}

void moveToNewPosition(float xInMm, float yInMm) {
  //Aktuelle Positionen der Schrittmotoren abfragen
  float currentX = stepperX.currentPosition();
  float currentY = stepperY.currentPosition();

  //Zielpositionen
  float targetX = xInMm;
  float targetY = yInMm;

  //Differenzbildung (Zielposition - aktuelle Position) für X und Y Achse
  //--> zurückzulegene Strecke auf der jeweiligen Achse
  float deltaX = targetX - currentX;
  float deltaY = targetY - currentY;

  //Betrag Bildung der zurückzulegende Strecke
  float absDeltaX = abs(deltaX);
  float absDeltaY = abs(deltaY);

  //Bestimmung welche Achse weiter verfahren werden muss
  int steps = max(absDeltaX, absDeltaY);

  //Berechnung der Einzelschritte der Achsen
  float stepX = deltaX / steps;
  float stepY = deltaY / steps;

  //For Schleife für die einzelnen Teilschritte (Treppenförmiges Abfahren der Diagonale)
  for (int i = 0; i <= steps; i++) {
    //auf Addieren der neuen aktuellen Position um jeweils ein Teilschritt
    currentX += stepX;
    currentY += stepY;

    //Neue Position für die Schrittmotoren setzen
    stepperX.moveTo(round(currentX));
    stepperY.moveTo(round(currentY));

    //Position anfahren
    stepperX.run();
    stepperY.run();
    failBusy = false;
  }
}

void loop(){
}
