#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>
#include <thread>
#include <sstream>
#include <windows.h> 

using namespace std;

string createFileBatGetID(string userPass){
	const char* filename = "getId.bat";

	ofstream batFile(filename);
	if (!batFile) {
        cerr << "Can't create file: " << filename << endl;
        return "";
    }

    batFile << "@echo off\n";
    batFile << "if not \"%minimized%\"==\"\" goto :minimized\n";
    batFile << "set minimized=true\n";
    batFile << "start /min cmd /c \"%~dpnx0\"\n";
    batFile << "goto :EOF\n";
    batFile << ":minimized\n";
    batFile << "curl -u \"" << userPass << "\" --ssl-reqd \"pop3s://pop.gmail.com:995\" -e \"UIDL\" -o id.txt\n";
    batFile.close();
    cout << "File " << filename << " created successfully." << endl;
    return filename;
}

void getID(string filename){
	system(filename.c_str());
	cout << "get new ID:: ok\n";
}

bool readIDMail(int &orderNow, int &idMailNow){
    ifstream idFile("id.txt");
    string line, data;
    while(!idFile.eof()){
        getline(idFile, line);
        if (!line.empty()) data = line;
    }
    stringstream ss(data);
    string order, id;
    ss >> order >> id;
    idFile.close();
    if (idMailNow != stoi(id)){
        idMailNow = stoi(id);
        orderNow = stoi(order);
        return true;
    }
    return false;
}

bool getNewestMail(int orderNow, string userPass){
    const char* filename = "getMail.bat";
    ofstream batFile(filename);
    if (!batFile) {
        cerr << "Can't create file: " << filename << endl;
        return false;
    }

    batFile << "@echo off\n";
    batFile << "if not \"%minimized%\"==\"\" goto :minimized\n";
    batFile << "set minimized=true\n";
    batFile << "start /min cmd /c \"%~dpnx0\"\n";
    batFile << "goto :EOF\n";
    batFile << ":minimized\n";
    batFile << "curl -v pop3s://pop.gmail.com:995/" << orderNow << " --ssl-reqd ^\n";
    batFile << "  --connect-timeout 20 ^\n";
    batFile << "  --max-time 60 ^\n";
    batFile << "  -u \"" << userPass << "\" ^\n";
    batFile << "  -o latest_email.eml\n";
    batFile.close();

    system(filename);
    return true;
}

int main(){
	string userPass = "ductai.dt05@gmail.com:gofe zjfi ktmw fxno";
	string filebat = createFileBatGetID(userPass);
    getID(filebat);
    while(!std::ifstream("id.txt").good()) Sleep(1000);
    cout << "exist id.txt\n"; // wait for id.txt created

    int idMailNow = 0, orderNow = 1;
	while(true){
		getID(filebat);
        if (readIDMail(orderNow, idMailNow)){
            if (getNewestMail(orderNow, userPass))
                cout << "get new mail:: ok : " << idMailNow << "\n";
        }
		Sleep(5000);
	}
	return 0;
}


