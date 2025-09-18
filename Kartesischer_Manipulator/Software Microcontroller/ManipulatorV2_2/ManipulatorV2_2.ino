/*
Hochschule Bielefeld
Projekt kartesischer Manipulator
Quellcode Makeblock XY Plotter MeOrion Steuerungsplatine
von: Sirin, Mert Cumhur; Kummer, Julian; Beckmann, Timo 

Stand V2.2 31.01.2025
*/

#include "MeOrion.h"                  //Bibliothek der Makeblock Steuerplatine

//Konfiguration der Schrittmotoren und Endlagenschalter
MeStepper stepperY(PORT_1);           //Schrittmotor Y-Achse; Port steht jeweils auf dem Anschluss an der Steuerplatine
MeStepper stepperX(PORT_2);           //Schrittmotor X-Achse
MeLimitSwitch limitSwitchY(PORT_6);   //Endlagenschalter Nullpunkt Y-Achse
MeLimitSwitch limitSwitchX(PORT_4);   //Endlagenschalter Nullpunkt X-Achse
MePort port(PORT_3);                  //Port Servo Stifthöhe
Servo myservo;                        //Servo Stifthöhe
int16_t servopin = port.pin();        //Servo Pin zuweisen

String buffer = "";                       //Puffer Speicher zum einlesen des Eingabestrings
float  xInMm = 0;                         //Soll Koordinate X-Achse
float  yInMm = 0;                         //Soll Koordinate Y-Achse
bool failValue = false;                   //Variable für Fehlermeldung: empfangener String fehlerhaft
bool failCalibration = false;             //Variable für Fehlermeldung: Plotter ist noch am kalibrieren
bool failIsRunning = true;                //Variable für Fehlermeldung: Plotter ist noch am anfahren der vorherigen Position; Start Wert muss true sein da Programm den Wert 0 setzt (bedingt durch Bibliothek ist nur die Abfrage möglich ob die motoren nicht verfahren)
bool failLimitSwitch = false;             //Variable für Fehlermeldung: Endlagenschalter wurde unerwartet betätigt
bool failPower = false;                   //Variable für Fehlermeldung: Betriebsspannung 230V fehlt

void setup() 
{
    //Serielle Verbindung initialisieren
    Serial.begin(115200);                            //Beginn  Serielle Kommunikation Wichtig! Passende Baud Rate einstellen(115200) [Bei Verwendung des seriellen Monitors in der Arduino IDE Auswahl: "kein Zeilenende"]
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
    
    //Refferenzfahrt der X- und Y-Achse
    home_steppers();                  //Funktionsaufruf für Initialisierungsfahrt

  
}
void loop() 
{

  failIsRunning = (stepperX.run()||stepperY.run()); //Fehler Meldung Null setzen wenn Motoren grade  nicht verfahren

  if (Serial.available() > 0 && failIsRunning == 0) //Wenn Serielle Eingabe vorhanden ist und der Plotter aktuell nicht verfährt
  {
    buffer = Serial.readString(); //String in Puffervariable einlesen
    
    Serial.println(buffer);       //Rückgabe des Empfangenen Strings

   
    
   
    
   //Kontrolle ob String den erwarteten Format entspricht
    if (buffer.length() == 15 && buffer[0]=='x' && buffer[5] == 'y' && buffer[10] =='p' &&buffer[12]=='r'&&buffer[14]=='e') //Aufbau Beispiel x5200y5200p1r0e
    {
      //Zerlegen des Strings in die einzelnen Werte
      float inputX = 0.1*(buffer.substring(1, 5).toFloat());
      float inputY = 0.1*(buffer.substring(6, 10).toFloat());
      bool pendown = (buffer[11] != '0');
      if(buffer[13] == 1) //Wenn Reset == true
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
      else if (failCalibration == 0 && failIsRunning == 0 && failLimitSwitch == 0 && failPower == 0) //Wenn kein Fehler vorliegt
      { 
        xInMm = inputX * 43.4444; // Umrechnung der mm in Schritte;  Dieses Wert gilt nur für 8-fach Microstepping!
        yInMm = inputY * 43.4444; // Siehe Quickstart Guide Absatz "Betrachtung Auflösung des Arbeitsbereiches"
        if(pendown==1)
        {
          myservo.write(180); //Stift runter fahren
          delay(500);       //Wartezeit bis Stift verfahren ist
        }
        else
        {
          myservo.write(0); //Stift hoch fahren
          delay(500);         //Wartezeit bis Stift verfahren ist
        }
      }
      
    }
    else 
      {
        failValue = 1;
      }
    failIsRunning = (stepperX.run()||stepperY.run());   //Abfrage, ob Plotter grade stillsteht
    //Ausgabe Fehler String
    Serial.print("f");
    Serial.print(failValue);
    Serial.print(failCalibration);
    Serial.print(failIsRunning);                        //Ausgabe Fehler String
    Serial.print(failLimitSwitch);
    Serial.print(failPower);
    Serial.println("e");

  failIsRunning == 1;
  failValue = 0;                                        //Rücksetztung Fehler Variablen
  failPower = 0;
  moveToDiagonal( xInMm,  yInMm); //Funktionsaufruf Diagonale Bewegung zum neuen Punkt
  }  
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
    failCalibration =1;
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
    
    steppback_home_steppers();       //Funktion siehe unten
   
  }

  /*
  Funktion für Feinkalibrierung der Initialisierungsfahrt der X- und Y-Achse

  Fährt zuerst die X-Achse immer einen Schritt zurück bis die Endlage nicht mehr gedrückt ist.
  Danach selbiges für die Y-Achse.
  Um den Fehler durch das Verfahren von jeweils 20 Schritten in der Funktion home_steppers auszugleichen.
  Anschließend wird die aktuelle Position der Achsen als neuer Nullpunkt definiert.
  */
  void steppback_home_steppers()
  {
      while(limitSwitchX.touched()) //mit jeweils 1 Schritt zurück bis kein Signal der Endlage mehr
    {
      stepperX.move(1);
      stepperX.run(); 
    }
     while(limitSwitchY.touched()) //mit jeweils 1 Schritt zurück bis kein Signal der Endlage mehr
    {
      stepperY.move(1);
      stepperY.run(); 
    }
    stepperX.setCurrentPosition(0);  // Setzt die aktuelle Position als Null
    stepperY.setCurrentPosition(0);  // Setzt die aktuelle Position als Null
    failCalibration =0;
  }
  
  void moveToDiagonal(float xInMm, float yInMm) 
  {
    while (failIsRunning == 1){
      failIsRunning = (stepperX.run()||stepperY.run());
    }
    
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
      //auf Addieren der neuen aktuellen Position um jeweils ein Teilschritt
      currentX += stepX;
      currentY += stepY;

      //Neue Position für die Schrittmotoren setzen
      stepperX.moveTo(round(currentX));
      stepperY.moveTo(round(currentY));

      //Position anfahren
      stepperX.run();
      stepperY.run();
    }
  }


 
  
