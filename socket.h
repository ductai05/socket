// socket.h
#ifndef SOCKET_H
#define SOCKET_H

#include <iostream>
#include <set>
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
#include <sstream>
#include <iomanip>
#include <ctime>
using namespace std;

//----------------------------SEND MAIL-----------------------------------------

string getCurrentDateTime() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(now, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// Hàm mã hóa Base64
string base64_encode(const std::string& in) {
    static const char* base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    while (valb >= 0) {
        out.push_back(base64_chars[(val >> valb) & 0x3F]);
        valb -= 6;
    }
    while (out.size() % 4) out.push_back('=');
    return out;
}

void sendMail(const string& from, const string& to, const string& subject, const string& body, const string& userPass, const string& fileName) {
    string encodedFileContent;
    if (!fileName.empty()){
        ifstream file(fileName, std::ios::binary);
        if (!file) {
            std::cerr << "Khong the mo tep dinh kem!" << std::endl;
            return;
        }

        // Đọc nội dung tệp
        stringstream buffer;
        buffer << file.rdbuf();
        string fileContent = buffer.str();
        file.close();

        // Mã hóa nội dung tệp sang Base64
        encodedFileContent = base64_encode(fileContent);
    }

    // Tạo tệp email
    ofstream emailFile("0sendEmail.txt");
    if (!emailFile) {
        std::cerr << "Khong the mo tep de ghi noi dung mail!" << std::endl;
        return;
    }

    // Ghi thông tin email vào tệp
    emailFile << "From: " << from << "\n";
    emailFile << "To: " << to << "\n";
    emailFile << "Subject: " << subject << "\n";
    emailFile << "MIME-Version: 1.0\n";
    emailFile << "Content-Type: multipart/mixed; boundary=\"boundary\"\n\n";
    emailFile << "--boundary\n";
    emailFile << "Content-Type: text/plain; charset=\"UTF-8\"\n\n";
    if (encodedFileContent.empty()){
        emailFile << body << "\n\n";
    } else {
        // co tep dinh kem:
        emailFile << body << "\n\n"; //" " << encodedFileContent << "\n\n";
    }
      
    // Thêm tệp đính kèm
    if (!fileName.empty() && (1 == 1)) {
        emailFile << "--boundary\n";
        emailFile << "Content-Type: application/octet-stream; name=\"" << fileName << "\"\n";
        
        string name = "";
        size_t lastSlash = fileName.find_last_of("/");
        if (lastSlash != std::string::npos)
            name = fileName.substr(lastSlash + 1);

        emailFile << "Content-Disposition: attachment; filename=\"" << name << "\"\n";
        emailFile << "Content-Transfer-Encoding: base64\n\n";
        emailFile << encodedFileContent << "\n";  // Nội dung mã hóa Base64 của tệp đính kèm
    }

    emailFile << "--boundary--\n";
    emailFile.close();

    // Tạo lệnh curl
    std::string ex = "curl --url \"smtp://smtp.gmail.com:587\" --ssl-reqd "
                     "--mail-from \"" + from + "\" "
                     "--mail-rcpt \"" + to + "\" "
                     "--user \"" + userPass + "\" "
                     "--upload-file \"0sendEmail.txt\"";

    // Gọi lệnh curl
    int result = system(ex.c_str());
    if (result == -1) {
        std::cerr << "Khong the gui email!\n";
        return;
    }

    // Xóa tệp tạm thời
    remove("0sendEmail.txt");
}

// bool client == true -> request; client == false -> response 
// string task = list app / camera / screenshot....
// client request -> fileContent = ""; server response -> fileContent = "abc.txt/png"

void newMail(bool client, string task, string numTask, string fileContent){
    string daytime = getCurrentDateTime();
    string typeOfSend;
    if (client) typeOfSend = "[request_" + numTask + "]: ";
    else typeOfSend = "[response_" + numTask + "]: ";
    string from = "ai23socket@gmail.com"; //from = "ductaidt05@gmail.com";
    string to = "ai23socket@gmail.com"; //to = "ductaidt05@gmail.com";
    string subject = typeOfSend + daytime;
    string body = (client ? "[task] " : "[rep] ") + task;
    string userPass = "ai23socket@gmail.com:nhrr llaa ggzb yzbj";
    //userPass = "ductaidt05@gmail.com:bveh frje cysx mjot";
    sendMail(from, to, subject, body, userPass, fileContent);
}

//----------------------------AUTO GET MAIL--------------------------------------

bool getID(string userPass, bool isClientLISTEN){
    remove("0id.txt");
    string ex = "curl -s -# -v imaps://imap.gmail.com/INBOX --ssl-reqd --connect-timeout 20 --max-time 15 -u \"" 
    + userPass +"\" -X \"UID SEARCH ALL\" -o 0id.txt";
    int result = system(ex.c_str());
    if (result == -1) {
        if (isClientLISTEN) cout << "\nCLIENT getID: FAIL; ";
        else cout << "\nSERVER getID: FAIL; ";
        return false;
    } else {
        if (isClientLISTEN) cout << "\nCLIENT getID: DONE; ";
        else cout << "\nSERVER getID: DONE; ";
        return true;
    }
}

bool compareTimeStrings(const std::string& timeStr1, const std::string& timeStr2) {
    std::tm tm1 = {}, tm2 = {};

    std::istringstream ss1(timeStr1);
    std::istringstream ss2(timeStr2);
    
    ss1 >> std::get_time(&tm1, "%Y-%m-%d %H:%M:%S");
    ss2 >> std::get_time(&tm2, "%Y-%m-%d %H:%M:%S");

    std::time_t time1 = std::mktime(&tm1);
    std::time_t time2 = std::mktime(&tm2);

    return time1 > time2;
}

bool readIDMail(int &orderNow){
    int now = -3;
    ifstream idFile("0id.txt");
    bool isHaveMail = false;

    ifstream file("0id.txt"); // Mở file
    if (!file.is_open()) {
        std::cerr << "Can't read id!\n";
        return false;
    }

    string line, number, last_number;
    if (getline(file, line)) { // Đọc dòng đầu tiên
        istringstream iss(line);

        while (iss >> number) { // Tách từng số
            last_number = number; // Cập nhật số cuối cùng
        }

    }
    file.close(); // Đóng file
    
    if (last_number == "SEARCH")
        return false;
    else
        now = stoi(last_number);

    if (now == orderNow) isHaveMail = false;
    else {
        orderNow = now;
        isHaveMail = true;
    }
    cout << "lastOrder: " << orderNow << "\n";
    return isHaveMail;
}

bool getNewestMail(int orderNow, string userPass){

    string ex = "curl -v imaps://imap.gmail.com/INBOX/;UID=" + to_string(orderNow) 
    + " --ssl-reqd --connect-timeout 20  --max-time 15 -u \"" + userPass + "\" -o 0latest_email.eml";

    int result = system(ex.c_str());
    if (result == -1) {
        cout << "Can't get newest mail.\n";
        return false;
    } 
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

void readLatestMail(const string &timeLISTEN, bool isClientLISTEN, vector<string> &MAIL, vector<string> &TASK){
    bool check = false;
    string fileName = "0latest_email.eml";

    int k = 0;
    while(!ifstream(fileName).good()){
        Sleep(1000); k++;
        if (k == 20) return;
    } // dam bao latest_email.eml da duoc tao

    ifstream inFile(fileName); string line;
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

    if ((subject.find("[request_") || !isClientLISTEN) && (subject.find("[response_") || isClientLISTEN)){
        regex pattern(R"(\[(response|request)_(\d+)\]: (\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}))");
        smatch matches;

        if (regex_search(subject, matches, pattern)) {
            responseType = matches[1]; // 'response' or 'request'
            numTask = matches[2];      // numtask
            time = matches[3];         // time
            if (find(MAIL.begin(), MAIL.end(), subject) == MAIL.end() 
            && find(TASK.begin(), TASK.end(), numTask) == TASK.end() 
            && compareTimeStrings(time, timeLISTEN)
                ){
                check = true;
                MAIL.push_back(subject);
                TASK.push_back(numTask);
            }
        }
        if (check) {
            cout << "[SUCCESSFULL] Type: " + responseType + ", NumTask: " + numTask + ", Timestamp: " + time << "\n";
        }
        else {
            cout << "subject: " << subject << "\n[INVALID] Type: " + responseType + ", NumTask: " + numTask + ", Timestamp: " + time << "\n";
            inFile.close();
            return;
        }
    } else {
        cout << "[newest mail: not acceptable responseType]\n";
        return;
    }

    // getBody
    if (isClientLISTEN){
        while(getline(inFile, line)){
            if (line.find("[rep]") != string::npos) {
                body = line.substr(line.find("]") + 1);
                body.erase(0, body.find_first_not_of(" \t"));
                body.erase(body.find_last_not_of(" \t") + 1);
                break; 
            }
        }
        cout << line << "\n";
    } else {
        while(getline(inFile, line)){
            if (line.find("[task]") != string::npos) {
                body = line.substr(line.find("]") + 1);
                body.erase(0, body.find_first_not_of(" \t"));
                body.erase(body.find_last_not_of(" \t") + 1);
                break; 
            }
        }
        cout << line << "\n";
    }
    

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
    inFile.close();
}

void autoGetMail(bool isClientLISTEN = false){
    string timeLISTEN = getCurrentDateTime();
    if (isClientLISTEN){
        cout << "CLIENT Start listen at: " << timeLISTEN << "\n"; // Start listen at: 2024-11-27 21:56:49
    } else cout << "SERVER Start listen at: " << timeLISTEN << "\n"; // Start listen at: 2024-11-27 21:56:49

    string userPass = "ai23socket@gmail.com:nhrr llaa ggzb yzbj";
    // userPass = "ductai.dt05@gmail.com:bveh frje cysx mjot";
    // string filebat = createFileBatGetID(userPass); // tao file getID ("getId.bat")
    // if (!getID(filebat, isClientLISTEN)) return;

    int orderNow = -2;
    vector<string> allMAIL;
    vector<string> allTASK;

    while(true){
        if (!getID(userPass, isClientLISTEN)) break;
        if (readIDMail(orderNow)){
            cout << "Get a new mail. Waiting server...\n";
            Sleep(3000);
            if (getNewestMail(orderNow, userPass)){
                readLatestMail(timeLISTEN, isClientLISTEN, allMAIL, allTASK);
            }
        }
        
        cout << "Sleep 7s... \n";
        Sleep(7000);
    }
    // remove("latest_email.eml");
}

#endif
