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

int main() {

    string portName = "/dev/ttyUSB0";  // Serial Port festlegen
    SerialPort serialPort(portName);

    if (!serialPort.open()) {
        return -1;  // Fehler beim Öffnen des Ports
    }

    string dataToSend;
    string dataReceived;
    char userInput;

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
        queue<string> q; //Warteschlange, in der die einzelnen Zeilen der Datei eingelesen werden
        while (std::getline(file, str)){
            ++number_of_lines;
            q.push(str);
        }

        serialPort.sendData(q.front());//Sende erste Daten der Warteschlange, bzw. erste Zeile der Datei
        while(!q.empty()){
            if(d.find("f00000e")!=-1){//Wenn ein "Alles OK" vom Mikrocontroller kommt, wird Queue gepopt
            q.pop();
            }
            d=serialPort.receiveData();//Auf Daten warten
            cout << d;//Output von empfangenen Daten
            temp=q.front();//Speichert front der queue in temp ab
            sleep(2);//Warten, da Mikrocontroller lahm und während des Fahren gar nichts macht :/
            serialPort.sendData(q.front());//front der Queue senden
            do{
                d=serialPort.receiveData();//Während nichts ankommt auf Daten warten
            }while(d=="");
            if(temp==""){
                cout << "AAAAAAAAAAAAAAAAAAAAAA";//Abbruch, wenn front der queue leer
                break;
            }
        }
    }
    else if(userInput=='d'){
        std::ofstream file("../Eckpunkte.txt");//Öffne Datei und löscht alle Inhalte
        file.close();
        cout << "Koordinaten gelöscht. \n";
    }


    // Port schließen
    serialPort.close();

    return 0;


}
