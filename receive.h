// receive.h
#ifndef RECEIVE_H
#define RECEIVE_H

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <windows.h>
#include <vector>
#include <map>
#include <regex>

using namespace std;

// string getCurrentDateTime() {
//     std::time_t t = std::time(nullptr);
//     std::tm* now = std::localtime(&t);

//     std::ostringstream oss;
//     oss << std::put_time(now, "%Y-%m-%d %H:%M:%S");
//     return oss.str();
// }

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

bool extractInfo(const string &subject, string &responseType, string &numTask, string &time) {
    regex pattern(R"(\[(response|request)_(\d+)\]: (\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}))");
    smatch matches;

    if (regex_search(subject, matches, pattern)) {
        responseType = matches[1]; // 'response' or 'request'
        numTask = matches[2];        // numtask
        time = matches[3];      // time
        return true;
    }

    return false;
}

// Map for base64
map<char, int> base64_map = {
    {'A', 0}, {'B', 1}, {'C', 2}, {'D', 3}, {'E', 4}, {'F', 5},
    {'G', 6}, {'H', 7}, {'I', 8}, {'J', 9}, {'K', 10}, {'L', 11},
    {'M', 12}, {'N', 13}, {'O', 14}, {'P', 15}, {'Q', 16}, {'R', 17},
    {'S', 18}, {'T', 19}, {'U', 20}, {'V', 21}, {'W', 22}, {'X', 23},
    {'Y', 24}, {'Z', 25}, {'a', 26}, {'b', 27}, {'c', 28}, {'d', 29},
    {'e', 30}, {'f', 31}, {'g', 32}, {'h', 33}, {'i', 34}, {'j', 35},
    {'k', 36}, {'l', 37}, {'m', 38}, {'n', 39}, {'o', 40}, {'p', 41},
    {'q', 42}, {'r', 43}, {'s', 44}, {'t', 45}, {'u', 46}, {'v', 47},
    {'w', 48}, {'x', 49}, {'y', 50}, {'z', 51}, {'0', 52}, {'1', 53},
    {'2', 54}, {'3', 55}, {'4', 56}, {'5', 57}, {'6', 58}, {'7', 59},
    {'8', 60}, {'9', 61}, {'+', 62}, {'/', 63}, {'=', -1}
};

// Ham giai ma base64
vector<unsigned char> base64_decode(const std::string &in) {
    vector<unsigned char> out;
    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (base64_map.find(c) == base64_map.end()) continue; // Skip ki tu ko phai base64
        val = (val << 6) + base64_map[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return out;
}

void saveFile(const std::string &filename, const std::vector<unsigned char> &data) {
    string directory = "attachment";
    string command = "mkdir " + directory;
    system(command.c_str());
    string fullPath = directory + "/" + filename;

    ofstream out(fullPath, std::ios::binary);
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    out.close();
}

void readLatestMail(){
    string fileName = "latest_email.eml";

    int k = 0;
    while(!ifstream(fileName).good()){
        Sleep(1000); k++;
        if (k == 5) return;
    } // dam bao latest_email.eml da duoc tao

    ifstream inFile(fileName);
    string line;
    string subject, body, responseType, numTask, time;
    string fileAttachmentName;
    // getSubject
    while(getline(inFile, line)){
        if (line.find("Subject:") != string::npos) {
            subject = line.substr(line.find(":") + 1);
            subject.erase(0, subject.find_first_not_of(" \t"));
            subject.erase(subject.find_last_not_of(" \t") + 1);
            break; 
        }
    }
    if (subject.find("[response_") || subject.find("[request_")){
        bool check = false;
        regex pattern(R"(\[(response|request)_(\d+)\]: (\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}))");
        smatch matches;

        if (regex_search(subject, matches, pattern)) {
            responseType = matches[1]; // 'response' or 'request'
            numTask = matches[2];      // numtask
            time = matches[3];         // time
            check = true;
        }
        if (check) cout << "Type: " + responseType + ", NumTask: " + numTask + ", Timestamp: " + time << "\n";
    } 

    // getBody
    while(getline(inFile, line)){
        if (line.find("[rep]") != string::npos || line.find("[task]") != string::npos) {
            body = line.substr(line.find("]") + 1);
            body.erase(0, body.find_first_not_of(" \t"));
            body.erase(body.find_last_not_of(" \t") + 1);
            break; 
        }
    }
    cout << body << "\n";

    // getFileAttachmentName
    while(getline(inFile, line)){
        if (line.find("filename=\"") != string::npos) {
            fileAttachmentName = line.substr(line.find("\"") + 1);
            fileAttachmentName = fileAttachmentName.substr(0, fileAttachmentName.size() - 1);

            getline(inFile, line); 
            //getline(inFile, line); // skip 1 lines
            break; 
        }
    }
    cout << fileAttachmentName << "\n";

    // get base64 
    string base64_data;
    int i = 1;
    while (getline(inFile, line)) {
        if (line == "--boundary--") break;
        base64_data += line;
    }
    
    // Giai ma base64
    vector<unsigned char> decoded_data = base64_decode(base64_data);

    // Luu file
    saveFile(fileAttachmentName, decoded_data);
}

void autoGetMail(){
    string daytime = getCurrentDateTime();
    cout << "Start listen at: " << daytime << "\n";

    string userPass = "ai23socket@gmail.com:nhrr llaa ggzb yzbj";
    string filebat = createFileBatGetID(userPass);
    getID(filebat);
    while(!std::ifstream("id.txt").good()) Sleep(1000); // dam bao id.txt da duoc tao
    cout << "exist id.txt\n"; // wait for id.txt created

    int idMailNow = 0, orderNow = 1;
    while(true){
        getID(filebat);
        while(!std::ifstream("id.txt").good()) Sleep(1000);
        if (readIDMail(orderNow, idMailNow)){
            if (getNewestMail(orderNow, userPass)){
                cout << "get new mail:: ok : " << idMailNow << "\n";
                readLatestMail();
            }
        }
        
        Sleep(5000);
    }
    // remove("latest_email.eml");
}

#endif
