#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <queue>
#include <sstream>
#include <string>
#include <chrono>

using namespace std;

class SerialPort {
public:
    SerialPort(const string& portName) : portName(portName), fd(-1) {}

    // Öffne Serial Port
    bool open() {
        fd = ::open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY); // O_NDELAY damit nicht-blockierend
        if (fd == -1) {
            cerr << "Fehler beim Öffnen des Ports: " << portName << endl;
            return false;
        }

        struct termios options;
        tcgetattr(fd, &options); // aktuelle Einstellungen holen

        // Baudrate 115200
        cfsetispeed(&options, B115200);
        cfsetospeed(&options, B115200);

        // 8 Datenbits, keine Parität und 1 Stop-Bit
        options.c_cflag &= ~PARENB; // Keine Parität
        options.c_cflag &= ~CSTOPB; // Ein Stop-Bit
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;     // 8 Datenbits

        // Port auf den "Raw"-Modus (keine Modifikation von Steuerzeichen)
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // Keine Eingabepufferung
        options.c_oflag &= ~OPOST;  // Keine Ausgabe-Post-Processing

        // Deaktiviere Hardwareflusskontrolle
        options.c_cflag &= ~CRTSCTS;

        // Setze Änderungen
        tcsetattr(fd, TCSANOW, &options);

        return true;
    }

    // Sendet Daten über Serial Port
    void sendData(const string& data) {
        if (fd == -1) {
            cerr << "Port wurde nicht geöffnet!" << endl;
            return;
        }

        ssize_t bytesWritten = write(fd, data.c_str(), data.length());
        if (bytesWritten == -1) {
            cerr << "Fehler beim Senden der Daten." << endl;
        } else {
            cout << "Daten gesendet: " << data << endl;
        }
    }

    // Daten vom Serial Port ablesen
    void getData() {
        if (fd == -1) {
            cerr << "Port wurde nicht geöffnet!" << endl;
            return;
        }

        char read_buf[256];  // Puffer für eingehende Daten
        memset(&read_buf, '\0', sizeof(read_buf));

        int num_bytes = read(fd, &read_buf, sizeof(read_buf));
        if (num_bytes < 0) {
            cerr << "Fehler beim Lesen: " << strerror(errno) << endl;
            return;
        }

        if (num_bytes > 0) {
            cout << "Empfangene Daten: " << read_buf << endl;
        } else {
            cout << "Keine Daten empfangen." << endl;
        }
    }


    string receiveData(){
        std::stringstream o;

        char read_buf[256];  // Puffer für eingehende Daten
        memset(&read_buf, '\0', sizeof(read_buf));

        int num_bytes = read(fd, &read_buf, sizeof(read_buf));
        if (num_bytes < 0) {
            o << read_buf;
            return (o.str());
        }

        if (num_bytes > 0) {
            o << read_buf;
            return (o.str());
        } else {
            cout << "Keine Daten empfangen." << endl;
            return "ERROR";
        }
    }

    bool isOpen(){
        if (fd == -1) {
            cerr << "Port wurde nicht geöffnet!" << endl;
            return false;
        }

        char read_buf[256];  // Puffer für eingehende Daten
        memset(&read_buf, '\0', sizeof(read_buf));

        int num_bytes = read(fd, &read_buf, sizeof(read_buf));
        if (num_bytes <= 0) {
            cerr << "Fehler beim Lesen: " << strerror(errno) << endl;
            return false;
        }

        if (num_bytes > 0) {
            cout << "Empfangene Daten: " << read_buf << endl;
            return true;
        } else {
            cout << "Keine Daten empfangen." << endl;
            return true;
        }
    }

    // Schließt Serial Port
    void close() {
        if (fd != -1) {
            ::close(fd);
        }
    }

private:
    string portName;
    int fd;
};

bool errording(string input){
    char output=input[3];
    return output;
}

int zeichenFlaeche(){
    char input;
    cout << "Groeße der Zeichenflaeche: \n 1 fuer A4 \n 2 fuer gesamte Flaeche \n";
    cin >> input;

    if(input == 1){
        return 1;
    }
    else if(input == 2){
        return 2;
    }
    else{
        cout << "Keine valide Eingabe! \n";
        zeichenFlaeche();
    }
    return 0;
}

void programm(int programm){

}

int main() {

    string portName = "/dev/ttyUSB0";  // Serial Port festlegen
    SerialPort serialPort(portName);

    if (!serialPort.open()) {
        return -1;  // Fehler beim Öffnen des Ports
    }

    string dataToSend;
    string dataReceived;
    char userInput;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    cout << "Manueller Input? (y/n/a/d): ";
    cin >> userInput;

    if(userInput=='y'){
        while (true) {
            // Eingabeschleife für zu sendende Daten
            cout << "Nachricht (oder 'q' zum Beenden): ";
            getline(cin, dataToSend);

            if (dataToSend == "q") {
                break;  // Schleife beenden, wenn Eingabe "q"
            }

            // Daten senden
            serialPort.sendData(dataToSend);

            // Daten empfangen
            serialPort.getData();

            // Warte auf Benutzeranweisung, ob weitere Daten senden
            cout << "Möchten Sie weitere Daten senden? (y/n): ";
            cin >> userInput;
            cin.ignore();  // Leert Eingabepuffer für nächste Eingabe

            if (userInput != 'y') {
                break;
            }
        }
    }
    else if(userInput=='n'){
        while (true) {

            int x=0;
            int y=0;
            int pen=0;
            int res=0;
            string xs;
            string ys;

            cout << "X-Coordinates: ";
            cin >> x;
            if (x<0||x>510) {
                cout << "Eingabe liegt ausserhalb des erlaubten Wertebereichs (0-510)! ";
                break;
            }
            x=x*10;
            xs=to_string(x);
            while(xs.length()<4){
                xs.insert(0,"0");
            }

            cout << "Y-Coordinates: ";
            cin >> y;
            if (y<0||y>500) {
                cout << "Eingabe liegt ausserhalb des erlaubten Wertebereichs (0-500)! ";
                break;
            }
            y=y*10;
            ys=to_string(y);
            ys=to_string(y);
            while(ys.length()<4){
                ys.insert(0,"0");
            }

            cout << "Pen up(0) or down(1)?: ";
            cin >> pen;

            if (pen<0||pen>1) {
                cout << "Eingabe liegt ausserhalb des erlaubten Wertebereichs (0,1)! ";
                break;
            }

            cout << "Reset?: ";
            cin >> res;

            if (res<0||res>1) {
                cout << "Eingabe liegt ausserhalb des erlaubten Wertebereichs (0,1)! ";
                break;
            }

            dataToSend="x"+xs+"y"+ys+"p"+to_string(pen)+"r"+to_string(res)+"e";

            cout << dataToSend;

            // Daten senden
            serialPort.sendData(dataToSend);

            // Daten empfangen
            serialPort.getData();

            // Warte auf Benutzeranweisung, ob weitere Daten senden
            cout << "Möchten Sie weitere Daten senden? (y/n): ";
            cin >> userInput;
            cin.ignore();  // Leert Eingabepuffer für nächste Eingabe

            if (userInput != 'y') {
                break;
            }
        }
    }
    else if(userInput=='a'){
        std::ifstream file("../Eckpunkte.txt");//Öffne Datei
        std::string str;
        string temp;
        string d;
        bool portOpen;//Öffne Port
        portOpen=serialPort.isOpen();
        int number_of_lines;//Anzahl an Zeilen in der geöffneten Datei

        int send = 1;
        queue<string> q; //Warteschlange, in der die einzelnen Zeilen der Datei eingelesen werden
        while (std::getline(file, str)){
            ++number_of_lines;
            q.push(str);
        }

        while(!q.empty()){
            switch (send){
                case 1:
                    serialPort.sendData(q.front());
                    begin = std::chrono::steady_clock::now();
                    send = 0;
                    break;
                default:
                    d = serialPort.receiveData();
                    if(d.find("f00000e")!= -1){
                        q.pop();
                        end = std::chrono::steady_clock::now();
                        std::cout << "Time difference (sec) = " <<  (std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) /1000000.0  <<std::endl;
                        send = 1;
                    }
            }

            /*
            do{
                serialPort.sendData(q.front());
                d=serialPort.receiveData();
                if(d == "f00100e" || d == "f00000e"){
                    break;
                }
            }while(d!="f00000e");
            q.pop();


            //if(d == "f00000e"){//Wenn ein "Alles OK" vom Mikrocontroller kommt, wird Queue gepopt
            if(d.find("f00000e")!=-1){
            q.pop();
            }
            //temp = q.front();
            //serialPort.sendData(q.front());
            //}
            d=serialPort.receiveData();//Auf Daten warten
            cout << d;//Output von empfangenen Daten
            temp=q.front();//Speichert front der queue in temp ab
            sleep(2);//Warten, da Mikrocontroller lahm und während des Fahren gar nichts macht :/
            serialPort.sendData(q.front());//front der Queue senden
            do{
                d=serialPort.receiveData();//Während nichts ankommt auf Daten warten
                //cout << d;
            //}while(d!="f00000e");
            }while(d=="");
            if(temp==""){
                cout << "AAAAAAAAAAAAAAAAAAAAAA";//Abbruch, wenn front der queue leer
                break;
            } */
        }
    }
    else if(userInput=='d'){
        std::ofstream file("../Eckpunkte.txt");//Öffne Datei und löscht alle Inhalte
        file.close();
        cout << "Koordinaten gelöscht. \n";
    }
    else if(userInput=='p'){
        string input;
        cout << "Programmname: ";
        cin >> userInput;

        std::string str;
        string temp;
        string d;
        bool portOpen;//Öffne Port
        portOpen=serialPort.isOpen();
        int number_of_lines;//Anzahl an Zeilen in der geöffneten Datei

        int send = 1;
        queue<string> q; //Warteschlange, in der die einzelnen Zeilen der Datei eingelesen werden


            std::ifstream file("../Drachen");
            while (std::getline(file, str)){
            ++number_of_lines;
            q.push(str);
        }

        while(!q.empty()){
            switch (send){
                case 1:
                    serialPort.sendData(q.front());
                    send = 0;
                    break;
                default:
                    d = serialPort.receiveData();
                    if(d.find("f00000e")!= -1 || d.find("f00100e")!= -1){
                        q.pop();
                        send = 1;
                    }
            }
            }



    }

    // Port schließen
    serialPort.close();

    return 0;


}

