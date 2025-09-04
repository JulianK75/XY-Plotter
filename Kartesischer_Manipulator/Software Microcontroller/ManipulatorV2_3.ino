/*
  Hochschule Bielefeld
  Projekt kartesischer Manipulator
  Letze Änderung 24.06.2025

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

#include "MeOrion.h"                  //Bibliothek der Makeblock Steuerplatine URL: https://github.com/Makeblock-official/Makeblock-Libraries 

//Konfiguration der Schrittmotoren und Endlagenschalter
MeStepper stepperY(PORT_1);           //Schrittmotor Y-Achse; Port steht jeweils auf dem Anschluss an der Steuerplatine
MeStepper stepperX(PORT_2);           //Schrittmotor X-Achse
MeLimitSwitch limitSwitchY(PORT_6);   //Endlagenschalter Nullpunkt Y-Achse
MeLimitSwitch limitSwitchX(PORT_4);   //Endlagenschalter Nullpunkt X-Achse
MePort port(PORT_3);                  //Port Servo Stifthöhe
Servo myservo;                        //Servo Stifthöhe
int16_t servopin = port.pin();        //Servo Pin zuweisen


String bufferIn = "x0000y0000p0r1e";                       //Puffer Speicher zum einlesen der Koordinaten
String currTar = "";                        //Aktuelles Fahrziel
float  xInMm = 0;                         //Soll Koordinate X-Achse
float  yInMm = 0;                         //Soll Koordinate Y-Achse
bool failValue = false;                   //Variable für Fehlermeldung: empfangener String fehlerhaft
bool failCalibration = false;             //Variable für Fehlermeldung: Plotter ist noch am kalibrieren
bool failBusy = false;               //Variable für Fehlermeldung: Plotter ist noch am anfahren der vorherigen Position
bool failLimitSwitch = false;             //Variable für Fehlermeldung: Endlagenschalter wurde unerwartet betätigt
bool failPower = false;                   //Variable für Fehlermeldung: Betriebsspannung 230V fehlt
bool wantCOM = true;                      //Ob Serielle Kommunikation erwünscht ist
float inputX;
float inputY;
bool pendown;


void setup()
{
  //Serielle Verbindung initialisieren
  Serial.begin(115200);                            //Beginn  Serielle Kommunikation Wichtig! Passende Baud Rate einstellen(115200) [Bei Verwendung des seriellen Monitors in der Arduino IDE Auswahl: "kein Zeilenende"]
  Serial.setTimeout(10);
  Serial.println("Kartesischer Manipulator");

  //Servo initialisieren
  myservo.attach(servopin);         //Stifthöhenservo  Pin zuweisen

  //Schrittmotoren konfigurieren
  stepperX.setMaxSpeed(5000);       //Schrittmotor X-Achse maximal mögliche Geschwindigkeit [Schritte/Sekunde]
  stepperX.setAcceleration(40000);  //Schrittmotor X-Achse Beschleunigung [Schritte/Sekunde²]
  stepperY.setMaxSpeed(5000);       //Schrittmotor Y-Achse maximal mögliche Geschwindigkeit [Schritte/Sekunde]
  stepperY.setAcceleration(40000);  //Schrittmotor Y-Achse Beschleunigung[Schritte/Sekunde²]

  stepperX.setSpeed(5000);          //Schrittmotor X-Achse Einstellung Geschwindigkeit [Schritte/Sekunde]
  stepperY.setSpeed(5000);          //Schrittmotor Y-Achse Einstellung Geschwindigkeit [Schritte/Sekunde]

  //Referenzfahrt der X- und Y-Achse
  home_steppers();                  //Funktionsaufruf für Initialisierungsfahrt


}


/*
  Funktion für die Initialisierungsfahrt der X- und Y-Achse

  Fährt zuerst die X-Achse immer 20 Schritte in Richtung Endlage(vom Nullpunkt) bis Endlage betätigt ist.
  Danach selbiges für die Y-Achse.
  Es werden immer 20 Schritte aufeinmal gefahren um eine höhere Geschwindigkeit der Kalibierung zu erziehlen.
  Anschließend wird die Funktion steppback_home_steppers die genaue Nullposition anzufahren und diese Werte als neuen Nullpunkt zuspeichern
*/
void home_steppers()
{
  failCalibration = 1;
  while (!limitSwitchX.touched()) //Solange Endlage noch nicht berührt
  {
    stepperX.move(-20);         //mit jeweils 20 Schritten an den Endlagenschalter rantasten
    stepperX.run();
  }

  while (!limitSwitchY.touched()) //Solange Endlage noch nicht berührt
  {
    stepperY.move(-20);          //mit jeweils 20 Schritten an den Endlagenschalter rantasten
    stepperY.run();
  }

  stepback_home_steppers();       //Funktion siehe unten

}

void communication() {

  bufferIn = Serial.readString(); //String in Puffervariable einlesen

  Serial.println(bufferIn);       //Rückgabe des Empfangenen Strings

}
/*
  Funktion für Feinkalibrierung der Initialisierungsfahrt der X- und Y-Achse

  Fährt zuerst die X-Achse immer einen Schritt zurück bis die Endlage nicht mehr gedrückt ist.
  Danach selbiges für die Y-Achse.
  Um den Fehler durch das Verfahren von jeweils 20 Schritten in der Funktion home_steppers auszugleichen.
  Anschließend wird die aktuelle Position der Achsen als neuer Nullpunkt definiert.
*/
void stepback_home_steppers()
{
  while (limitSwitchX.touched()) //mit jeweils 1 Schritt zurück bis kein Signal der Endlage mehr
  {
    stepperX.move(1);
    stepperX.run();
  }
  while (limitSwitchY.touched()) //mit jeweils 1 Schritt zurück bis kein Signal der Endlage mehr
  {
    stepperY.move(1);
    stepperY.run();
  }
  stepperX.setCurrentPosition(0);  // Setzt die aktuelle Position als Null
  stepperY.setCurrentPosition(0);  // Setzt die aktuelle Position als Null
  failCalibration = 0;
}

void moveToDiagonal(float xInMm, float yInMm)
{

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

  // Abbruch der Funktion wenn beide Zielwerte gleich den aktuellen Werte sind, um Systemfehler durch teilen durch 0 zu vermeiden
  if (absDeltaX == 0 && absDeltaY == 0)
  {
    failBusy = false;
    return; //Abbruch der Funktion
  }

  //Bestimmung welche Achse weiter verfahren werden muss
  int steps = max(absDeltaX, absDeltaY);

  //Berechnung der Einzelschritte der Achsen
  float stepX = deltaX / steps;
  float stepY = deltaY / steps;

  //For Schleife für die einzelnen Teilschritte (Treppenförmiges Abfahren der Diagonale)
  for (int i = 0; i <= steps; i++)
  {
    //Aufaddieren der neuen aktuellen Position um jeweils ein Teilschritt
    currentX += stepX;
    currentY += stepY;

    //Neue Position für die Schrittmotoren setzen
    stepperX.moveTo(round(currentX));
    stepperY.moveTo(round(currentY));

    //Position anfahren
    stepperX.run();
    stepperY.run();
    loop();           //Kommunikation
  }
  failBusy = false;
}

void control(String toControl)
{
  //Kontrolle ob String den erwarteten Format entspricht
  if (toControl.length() == 15 && toControl[0] == 'x' && toControl[5] == 'y' && toControl[10] == 'p' && toControl[12] == 'r' && toControl[14] == 'e') //Aufbau Beispiel x5200y5200p1r0e
  {
    //Zerlegen des Strings in die einzelnen Werte
    inputX = 0.1 * (toControl.substring(1, 5).toFloat());
    inputY = 0.1 * (toControl.substring(6, 10).toFloat());
    pendown = (toControl[11] != '0');
    if (toControl[13] == 1) //Wenn Reset == true
    {
      failLimitSwitch = false; // Rücksetzung Endlagenfehler
    }

    if (inputX < 0 || inputX > 520) //Wenn Wert X falsch
    {
      failValue = 1;   //Wertebereich! Fehler setzen
    }
    else if (inputY < 0 || inputY > 530) // Wenn Wert Y falsch
    {
      failValue = 1;  //Wertebereich! Fehler setzen
    }
    else if (failCalibration == 0 && failLimitSwitch == 0 && failPower == 0) //Wenn kein Fehler vorliegt
    {
      failValue = 0;
      wantCOM = false;

      //Ausgabe Fehler String
      Serial.print("f");
      Serial.print(failValue);
      Serial.print(failCalibration);
      Serial.print('0');
      Serial.print(failLimitSwitch);
      Serial.print(failPower);
      Serial.println("e");

    }
    else
    {
      failValue = 1;
    }
  }
}
void loop()
{


  if (Serial.available() > 14 && wantCOM == true)
  {
    communication();
    control(bufferIn);
  }

  if (failValue == 0 && failBusy == false) {
    xInMm = inputX * 43.4444; // Umrechnung der mm in Schritte;  Dieses Wert gilt nur für 8-fach Microstepping!
    yInMm = inputY * 43.4444; // Siehe Quickstart Guide Absatz "Betrachtung Auflösung des Arbeitsbereiches"
    if (pendown == 1)
    {
      myservo.write(180); //Stift runter fahren
      //delay(500);       //Wartezeit bis Stift verfahren ist
    }
    else
    {
      myservo.write(0); //Stift hoch fahren
      //delay(500);         //Wartezeit bis Stift verfahren ist
    }
    failBusy = true;
    wantCOM = true;



    moveToDiagonal( xInMm,  yInMm); //Funktionsaufruf Diagonale Bewegung zum neuen Punkt
  }


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

  /*Kontrolle ob String den erwarteten Format entspricht
    if (currTar.length() == 15 && currTar[0]=='x' && currTar[5] == 'y' && currTar[10] =='p' &&currTar[12]=='r'&&currTar[14]=='e') //Aufbau Beispiel x5200y5200p1r0e
    {
     //Zerlegen des Strings in die einzelnen Werte
     float inputX = 0.1*(currTar.substring(1, 5).toFloat());
     float inputY = 0.1*(currTar.substring(6, 10).toFloat());
     bool pendown = (currTar[11] != '0');
     if(currTar[13] == 1) //Wenn Reset == true
     {
       failLimitSwitch=false; // Rücksetzung Entlagenfehler
     }

     if (inputX < 0 || inputX > 520) //Wenn Wert X falsch
     {
       failValue = 1;   //Wertebereich! Fehler setzen
     }
     else if (inputY < 0 || inputY > 530) // Wenn Wert Y falsch
     {
       failValue = 1;  //Wertebereich! Fehler setzen
     }
     else if (failCalibration == 0 && failLimitSwitch == 0 && failPower == 0) //Wenn kein Fehler vorliegt
     {
       xInMm = inputX * 43.4444; // Umrechnung der mm in Schritte;  Dieses Wert gilt nur für 8-fach Microstepping!
       yInMm = inputY * 43.4444; // Siehe Quickstart Guide Absatz "Betrachtung Auflösung des Arbeitsbereiches"
       if(pendown==1)
       {
         myservo.write(180); //Stift runter fahren
         //delay(500);       //Wartezeit bis Stift verfahren ist
       }
       else
       {
         myservo.write(0); //Stift hoch fahren
         //delay(500);         //Wartezeit bis Stift verfahren ist
       }
     failBusy=true;
     moveToDiagonal( xInMm,  yInMm); //Funktionsaufruf Diagonale Bewegung zum neuen Punkt
    }
    else
     {
       failValue = 1;
     }*/



  return;
}
