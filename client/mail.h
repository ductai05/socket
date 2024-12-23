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
    time_t t = time(nullptr);
    tm now;

    // Sử dụng localtime_s để đảm bảo an toàn
    localtime_s(&now, &t);

    ostringstream oss;
    oss << put_time(&now, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

string getCurrentTime() {
    time_t t = time(nullptr);
    tm now;

    // Sử dụng localtime_s để đảm bảo an toàn
    localtime_s(&now, &t);

    ostringstream oss;
    oss << put_time(&now, "%H%M%S");
    return oss.str();
}

// Hàm mã hóa Base64
string base64_encode(const string& in) {
    static const char* base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    string out;
    int val = 0, valb = 0; // Initialize valb to 0
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 6) {
            out.push_back(base64_chars[(val >> (valb - 6)) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > 0) {
        out.push_back(base64_chars[(val << (6 - valb)) & 0x3F]); // Shift the remaining bits to the left
    }
    while (out.size() % 4) out.push_back('=');
    return out;
}

void sendMail(const string& from, const string& to, const string& subject, const string& body, const string& userPass, const string& fileName) {
    string encodedFileContent;
    if (!fileName.empty()){
        ifstream file(fileName, ios::binary);
        if (!file) {
            cerr << "Khong the mo tep dinh kem!" << endl;
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
        cerr << "Khong the mo tep de ghi noi dung mail!" << endl;
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
        emailFile << body << "\n\n"; 
    }
      
    // Thêm tệp đính kèm
    if (!fileName.empty() && (1 == 1)) {
        emailFile << "--boundary\n";
        emailFile << "Content-Type: application/octet-stream; name=\"" << fileName << "\"\n";
        
        string name = "";
        size_t lastSlash = fileName.find_last_of("/");
        if (lastSlash != string::npos)
            name = fileName.substr(lastSlash + 1);

        emailFile << "Content-Disposition: attachment; filename=\"" << name << "\"\n";
        emailFile << "Content-Transfer-Encoding: base64\n\n";
        emailFile << encodedFileContent << "\n";  // Nội dung mã hóa Base64 của tệp đính kèm
    }

    emailFile << "--boundary--\n";
    emailFile.close();

     string ex = "curl --url \"smtp://smtp.gmail.com:587\" --ssl-reqd "
                     "--mail-from \"" + from + "\" "
                     "--mail-rcpt \"" + to + "\" "
                     "--user \"" + userPass + "\" "
                     "--upload-file \"0sendEmail.txt\"";

     STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Ẩn cửa sổ

    ZeroMemory(&pi, sizeof(pi));

    // Tạo bản sao không đổi của chuỗi lệnh
     char *commandLine = new char[ex.size() + 1];
    strcpy(commandLine, ex.c_str());
    
     if (!CreateProcessA(nullptr, commandLine, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        cerr << "Khong the gui email!\n";
         delete[] commandLine;
        return;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
     delete[] commandLine;

      if (exitCode != 0) {
        cerr << "Khong the gui email!\n";
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

#include <iostream>
#include <string>
#include <windows.h>
#include <fstream>

bool getID(string userPass, bool isClientLISTEN) {
    remove("0id.txt");
    string ex = "curl -s -# -v imaps://imap.gmail.com/INBOX --ssl-reqd --connect-timeout 20 --max-time 15 -u \""
                     + userPass + "\" -X \"UID SEARCH ALL\" -o 0id.txt";
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Ẩn cửa sổ

    ZeroMemory(&pi, sizeof(pi));

    // Tạo một bản sao không đổi của chuỗi lệnh
    char *commandLine = new char[ex.size() + 1];
    strcpy(commandLine, ex.c_str());

    // Gọi CreateProcess
    if (!CreateProcessA(nullptr, commandLine, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        if (isClientLISTEN) cout << "\nCLIENT getID: FAIL; ";
        else cout << "\nSERVER getID: FAIL; ";
        delete[] commandLine;
        return false;
    }

    // Chờ tiến trình con kết thúc
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    delete[] commandLine;

    if (exitCode != 0) {
        if (isClientLISTEN) cout << "\nCLIENT getID: FAIL; ";
        else cout << "\nSERVER getID: FAIL; ";
        return false;
    } else {
        return true;
    }
}

bool compareTimeStrings(const string& timeStr1, const string& timeStr2) {
    tm tm1 = {}, tm2 = {};

    istringstream ss1(timeStr1);
    istringstream ss2(timeStr2);
    
    ss1 >> get_time(&tm1, "%Y-%m-%d %H:%M:%S");
    ss2 >> get_time(&tm2, "%Y-%m-%d %H:%M:%S");

    time_t time1 = mktime(&tm1);
    time_t time2 = mktime(&tm2);

    return time1 > time2;
}

bool readIDMail(int &orderNow){
    int now = -3;
    ifstream idFile("0id.txt");
    bool isHaveMail = false;

    ifstream file("0id.txt"); // Mở file
    if (!file.is_open()) {
        cerr << "Can't read id!\n";
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
        cout << "Get new mail: " << orderNow << "\n";
    }
    return isHaveMail;
}

bool getNewestMail(int orderNow, string userPass) {
    string ex = "curl -v imaps://imap.gmail.com/INBOX/;UID=" + to_string(orderNow)
                    + " --ssl-reqd --connect-timeout 20  --max-time 15 -u \"" + userPass + "\" -o 0latest_email.eml";

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Ẩn cửa sổ

    ZeroMemory(&pi, sizeof(pi));

    // Tạo bản sao không đổi của chuỗi lệnh
     char *commandLine = new char[ex.size() + 1];
    strcpy(commandLine, ex.c_str());
    
    if (!CreateProcessA(nullptr, commandLine, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        cout << "Can't get newest mail.\n";
        delete[] commandLine;
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
     delete[] commandLine;
    if (exitCode != 0) {
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
vector<unsigned char> base64_decode(const string& in) {
    vector<unsigned char> out;
    int val = 0, valb = 0; // Initialize valb to 0
    for (char c : in) {
        if (base64_map.find(c) == base64_map.end()) continue; // Skip non-base64 characters
        int charValue = base64_map[c];
        if (charValue == -1) break; // Stop processing if we find padding '='
        val = (val << 6) | charValue;
        valb += 6;
        while (valb >= 8) {
            out.push_back((val >> (valb - 8)) & 0xFF);
            valb -= 8;
        }
    }
    return out;
}

void saveFile(const string &filename, const vector<unsigned char> &data) {
    string directory = "attachment";
    string command = "mkdir " + directory;
    system(command.c_str());
    string fullPath = directory + "/" + filename;

    ofstream out(fullPath, ios::binary);
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
        // cout << line << "\n";
    } else {
        while(getline(inFile, line)){
            if (line.find("[task]") != string::npos) {
                body = line.substr(line.find("]") + 1);
                body.erase(0, body.find_first_not_of(" \t"));
                body.erase(body.find_last_not_of(" \t") + 1);
                break; 
            }
        }
        // cout << line << "\n";
    }
    // cout << "toi day1\n";

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
    // cout << "toi day2\n";

    string outName = "attachment/" + numTask + ".txt";
    // cout << outName << "   " << body << "\n";
    ofstream outFile(outName); 
    outFile << fileAttachmentName << "\n";
    outFile.close();

}

void autoGetMail(bool &stopClient, bool isClientLISTEN = false){
    string timeLISTEN = getCurrentDateTime();
    // if (isClientLISTEN){
    //     cout << "CLIENT Start listen at: " << timeLISTEN << "\n"; // Start listen at: 2024-11-27 21:56:49
    // } else cout << "SERVER Start listen at: " << timeLISTEN << "\n"; // Start listen at: 2024-11-27 21:56:49

    string userPass = "ai23socket@gmail.com:nhrr llaa ggzb yzbj";
    // userPass = "ductai.dt05@gmail.com:bveh frje cysx mjot";
    // string filebat = createFileBatGetID(userPass); // tao file getID ("getId.bat")
    // if (!getID(filebat, isClientLISTEN)) return;

    int orderNow = -2;
    vector<string> allMAIL;
    vector<string> allTASK;

    while(stopClient == false){
        if (!getID(userPass, isClientLISTEN)) break;
        if (readIDMail(orderNow)){
            // cout << "Get a new mail. Waiting server...\n";
            Sleep(500);
            if (getNewestMail(orderNow, userPass)){
                readLatestMail(timeLISTEN, isClientLISTEN, allMAIL, allTASK);
            }
        }
        
        // cout << "Sleep 2s... \n";
        // Sleep(2000);
    }
    // remove("latest_email.eml");
}

#endif
