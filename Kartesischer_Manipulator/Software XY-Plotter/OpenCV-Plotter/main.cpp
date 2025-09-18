#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <string>
#include <math.h>


// Funktion, um den Abstand zwischen zwei Punkten zu berechnen
double distance(const cv::Point& p1, const cv::Point& p2) {
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

// Funktion, um den Winkel eines Punktes relativ zur Schwerkraftmitte zu berechnen
double angleToCenter(const cv::Point& center, const cv::Point& pt) {
    return std::atan2(pt.y - center.y, pt.x - center.x);
}

std::string toFour(int input){ //Wenn String kuerzer als 4 Zeichen, dann Verlaengerung auf 4 Zeichen
    std::string output;

    output = std::to_string(input);
    while(output.length() < 4){
        output.insert (0, 1, '0');
    }
    return output;
}

std::string coordConv(char direction, int imgSizeX, int imgSizeY, float coord, int origin, int drawAreaX, int drawAreaY){
    int xGlobal;        //Globale X und Y Koordinaten
    int yGlobal;

    std::string xStr;   //Ausgabe-Strings
    std::string yStr;

    float xD;
    float yD;

    float ratioX;   //Verhaeltnis Zeichenflaeche zu Bildgroesse
    float ratioY;
    float ratioXY;  //Kleineres Verhaeltnis der beiden X und Y

    ratioX = float(drawAreaX)/float(imgSizeX);  //Berechnung Verhaeltnis Zeichenflaeche zu Bildgroesse
    ratioY = float(drawAreaY)/float(imgSizeY);

    if(ratioX<ratioY){  //Auswahl des kleineren Verhaeltnisses
        ratioXY=ratioX;
    }else if(ratioY<ratioX){
        ratioXY=ratioY;
    }else{
        ratioXY=1;
    }

    if(direction=='x'){ //Fallunterscheidung, ob X oder Y, da andere Berechnung
    xD = coord*ratioXY;
    xGlobal = origin+int(xD);
    xGlobal = xGlobal*10;
    xStr = toFour(xGlobal);
    return xStr;
    }
    else if(direction=='y'){
    yD = coord*ratioXY;
    yGlobal = origin-int(yD);
    yGlobal = yGlobal*10;
    yStr = toFour(yGlobal);
    return yStr;
    }
}

int imageCapture(){
    // Zugriff auf die Kamera (Standardkamera, meist USB-Kamera)
    cv::VideoCapture cap(0);  // 0 für die Standardkamera
    if (!cap.isOpened()) {
        std::cerr << "Kamera konnte nicht geöffnet werden!" << std::endl;
        return -1;
    }

    int area;

    while(area != 1 && area != 2){  //Auswahl der Bildgroesse (aktuell A4 oder gesamte Flaeche)
    std::cout << "Bildgroesse angeben: \n 1 fuer A4 \n 2 fuer gesamte Flaeche \n";
    std::cin >> area;
    if(area != 1 && area != 2){
    std::cout << "Ungueltige Eingabe! \n";
    }
    }

    cv::Mat frame, gray, blurred, edges;
    std::vector<std::vector<cv::Point>> contours;   //Punkte um Ecken und Kanten zu markieren
    std::vector<cv::Vec4i> hierarchy;               //Vektor um Punkte zu verbinden

    // Definieren Sie eine Toleranz, um benachbarte Punkte zu vereinigen (z.B. 20 Pixel)
    double tolerance = 20.0;

    while (true) {
        cap >> frame;  // Holen des aktuellen Kamerabildes
        if (frame.empty()) {
            std::cerr << "Kein Bild erhalten!" << std::endl;
            break;
        }

        // Zeige das Live-Video
        cv::imshow("Live Video", frame);

        // Warten auf eine Taste
        char key = cv::waitKey(1);
        if (key == 27) {  // Esc-Taste zum Beenden
            break;
        }

        // Wenn eine andere Taste gedrückt wird, beginne mit der Konturen-Erkennung
        if (key != -1) {
            // Bild in Graustufen umwandeln
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

            // Bild unscharf machen, um Rauschen zu reduzieren
            cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);

            // Canny-Kantenerkennung anwenden
            cv::Canny(blurred, edges, 100, 300, 3);

            // Finde alle Konturen
            cv::findContours(edges, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            // Zeichne die gefundenen Kanten und Eckpunkte
            cv::Mat result = frame.clone();
            std::vector<cv::Point> unique_points; // Vektor für einzigartige Punkte

            for (size_t i = 0; i < contours.size(); i++) {
                // Näherungsweise die Konturen in geometrische Formen umwandeln
                std::vector<cv::Point> approx;
                cv::approxPolyDP(contours[i], approx, 0.02 * cv::arcLength(contours[i], true), true);

                // Überprüfe, wie viele Ecken die Form hat
                int corners = approx.size();
                if (corners >= 3) {  // Wenn die Kontur mindestens 3 Ecken hat (Dreieck oder mehr)
                    // Überprüfe jede Ecke auf Duplikate
                    for (size_t j = 0; j < approx.size(); j++) {
                        bool is_duplicate = false;
                        // Vergleiche den aktuellen Punkt mit den bereits gefundenen Punkten
                        for (size_t k = 0; k < unique_points.size(); k++) {
                            // Wenn der Abstand zwischen zwei Punkten kleiner als die Toleranz ist, ignoriere diesen Punkt
                            if (distance(approx[j], unique_points[k]) < tolerance) {
                                is_duplicate = true;
                                break;
                            }
                        }
                        // Füge den Punkt hinzu, wenn er kein Duplikat ist
                        if (!is_duplicate) {
                            unique_points.push_back(approx[j]);
                        }
                    }
                }
            }

            // Sortiere die Eckpunkte im Uhrzeigersinn
            if (!unique_points.empty()) {
                // Berechne die Schwerkraftmitte
                cv::Point2f center(0, 0);
                for (const auto& pt : unique_points) {
                    center.x += pt.x;
                    center.y += pt.y;
                }
                center.x /= unique_points.size();
                center.y /= unique_points.size();

                // Sortiere die Punkte basierend auf dem Winkel zur Mitte
                std::sort(unique_points.begin(), unique_points.end(),
                          [&center](const cv::Point& a, const cv::Point& b) {
                              return angleToCenter(center, a) < angleToCenter(center, b);
                          });
            }

            // Markiere und nummeriere die Eckpunkte im Uhrzeigersinn
            int index = 1;  // Start der Nummerierung
            std::ofstream file("../Eckpunkte.txt");  // Öffnen der Textdatei zum Schreiben
            float xP1=0;
            float yP1=0;
            if (file.is_open()) {
                if(area==1){
                file << "x1900y4700p0r1e\n";
                for (size_t i = 0; i < unique_points.size(); i++) {
                    const cv::Point& pt = unique_points[i];
                    const cv::Point& next_pt = unique_points[(i + 1) % unique_points.size()]; // Verbindung zum nächsten Punkt


                    // Eckpunkt markieren
                    cv::circle(result, pt, 10, cv::Scalar(0, 255, 0), -1);

                    // Nummer neben den Eckpunkt schreiben
                    std::ostringstream text;
                    text << index;
                    cv::putText(result, text.str(), pt + cv::Point(10, -10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);

                    // Zeichne Kanten zwischen den Eckpunkten
                    cv::line(result, pt, next_pt, cv::Scalar(255, 0, 0), 2);

                    if(i==0){
                        xP1=pt.x;
                        yP1=pt.y;
                        file << "x" << coordConv('x',700, 500, pt.x, 190, 290, 200) << "y" << coordConv('y', 700, 500, pt.y, 470, 290, 200) << "p0r1" << "e" << "\n";
                    }else{
                        file << "x" << coordConv('x',700, 500, pt.x, 190, 290, 200) << "y" << coordConv('y', 700, 500, pt.y, 470, 290, 200) << "p1r1" << "e" << "\n";
                    }//Koordinaten in die Textdatei schreiben
                    index++;
                }
                file << "x" << coordConv('x',700, 500, xP1, 190, 290, 200) << "y" << coordConv('y', 700, 500, yP1, 470, 290, 200) << "p1r1" << "e" << "\n";
                file << "x" << 10*190 << "y" << 10*470 << "p0r1" << "e" << "\n";
                file.close();  // Datei schließen
                std::cout << "Eckpunkte in 'eckpunkte.txt' gespeichert! \n" << std::endl;
            // Zeige das verarbeitete Bild mit den markierten Eckpunkten und Kanten
            cv::imshow("Eckpunkte und Kanten", result);
            }
            else if(area==2){
                file << "x0000y5000p0r1e\n";
                for (size_t i = 0; i < unique_points.size(); i++) {
                    const cv::Point& pt = unique_points[i];
                    const cv::Point& next_pt = unique_points[(i + 1) % unique_points.size()]; // Verbindung zum nächsten Punkt


                    // Eckpunkt markieren
                    cv::circle(result, pt, 10, cv::Scalar(0, 255, 0), -1);

                    // Nummer neben den Eckpunkt schreiben
                    std::ostringstream text;
                    text << index;
                    cv::putText(result, text.str(), pt + cv::Point(10, -10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);

                    // Zeichne Kanten zwischen den Eckpunkten
                    cv::line(result, pt, next_pt, cv::Scalar(255, 0, 0), 2);

                    if(i==0){
                        xP1=pt.x;
                        yP1=pt.y;
                        file << "x" << coordConv('x',700, 500, pt.x, 0, 510, 500) << "y" << coordConv('y', 700, 500, pt.y, 500, 510, 500) << "p0r1" << "e" << "\n";
                    }else{
                        file << "x" << coordConv('x',700, 500, pt.x, 0, 510, 500) << "y" << coordConv('y', 700, 500, pt.y, 500, 510, 500) << "p1r1" << "e" << "\n";
                    }//Koordinaten in die Textdatei schreiben
                    index++;
                }
                file << "x" << coordConv('x',700, 500, xP1, 0, 510, 500) << "y" << coordConv('y', 700, 500, yP1, 500, 510, 500) << "p1r1" << "e" << "\n";
                file << "x" << "0000" << "y" << 10*500 << "p0r1" << "e" << "\n";
                file.close();  // Datei schließen
                std::cout << "Eckpunkte in 'eckpunkte.txt' gespeichert! \n" << std::endl;
            } else {
                std::cerr << "Fehler beim Öffnen der Datei! \n" << std::endl;
            }

            // Zeige das verarbeitete Bild mit den markierten Eckpunkten und Kanten
            cv::imshow("Eckpunkte und Kanten", result);
            cv::imwrite("../AktuellesBild.jpg",result);
            }
            }
        }

    cap.release();  // Kamera freigeben
    cv::destroyAllWindows();  // Alle Fenster schließen
    return 0;
}

void mathMode(){  // Mathematischer Modus: Sinus und Kosinus zeichnen
    double T = 2*M_PI; //eine Periode 2PI
    double resolution = 5000; //Aufloesung in Anzahl der Punkte
    double dX;  //deltaX
    double dT = T/resolution; //deltaT


    std::string input;
    double doubleInput;

    std::cout << "Aufloesung \n";   //Abfrage zur Aufloesung/Anzahl an Punkten
    std::cin >> doubleInput;
    if(doubleInput > 5000){     //evtl. zu hoch angesetzt
        std::cout << "Aufloesung zu fein, muss kleiner als 5000 sein. \n";
    }else{
        resolution = doubleInput;
    }
    dX = 5100/resolution; //Berechnung deltaX

    std::cout << "Anzahl an Perioden: \n";  //Anzahl der dargestellten Perioden
    std::cin >> doubleInput;
    if(doubleInput < 50){       //evtl. zu hoch angesetzt
        T = doubleInput*T;
    }else{
        std::cout << "Zu viele Perioden. \n";
    }
    dT = T/resolution;  //Berechnung der Schritte in der Periode
    double xyCoord[(int)resolution][2]={0}; //Speicherung der X- und Y-Koordinaten

    std::cout << "sin fuer Sinus, cos fuer Cosinus. \n";    //Entscheidung, ob Sinus oder Kosinus dargestellt wird
    std::cin >> input;
    if(input == "sin"){
        for(int i=0; i<resolution; i++){    //For-Schleife für die Multiplikation mit den Deltas
        xyCoord[i][0]=dX*i;
        xyCoord[i][1]=(sin(dT*i))*2500+2500;    //*2500+2500, da Koordinatenursprung nun mittig-links
        }
    }else if(input == "cos"){   //For-Schleife für die Multiplikation mit den Deltas
        for(int i=0; i<resolution; i++){
        xyCoord[i][0]=dX*i;
        xyCoord[i][1]=(cos(dT*i))*2500+2500;    //*2500+2500, da Koordinatenursprung nun mittig-links
        }
    }

    std::ofstream file("../Eckpunkte.txt"); //Speichern in Eckpunkt-Textdatei wie bei der Bildaufnahme
        if (file.is_open()) {
            file << "x" << toFour((int)xyCoord[0][0]) << "y" << toFour((int)xyCoord[0][1]) << "p0r1" << "e" << "\n";
            for(int i=1; i<resolution; i++){
                file << "x" << toFour((int)xyCoord[i][0]) << "y" << toFour((int)xyCoord[i][1]) << "p1r1" << "e" << "\n";
            }
            file.close();  // Datei schließen
                std::cout << "Eckpunkte in 'eckpunkte.txt' gespeichert! \n" << std::endl;
            } else {
                std::cerr << "Fehler beim Öffnen der Datei! \n" << std::endl;
            }


}

int main() {
    char mode;
    std::cout << "Modus auswaehlen: b fuer Bildaufnahme, m fuer mathematischer Modus. \n";  //Modusauswahl
    std::cin >> mode;
    if(mode == 'b'){
        imageCapture(); //Bildaufnahme
    }
    else if(mode == 'm'){
        mathMode();     //Auswahl ob Sinus oder Cosinus
    }else{
        std::cout << "ERROR! \n";
    }

}
