// sendmail.cpp
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include "sendmail.h"

using namespace std;

string getCurrentDateTime() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(now, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

string getContentType(string fileContent){
    int n = fileContent.size();
    if(n < 3) return "";
    if (fileContent[n - 3] == 't' && fileContent[n - 2] == 'x' && fileContent[n - 1] == 't'){
        return "text/plain";
    } else if (fileContent[n - 3] == 'p' && fileContent[n - 2] == 'n' && fileContent[n - 1] == 'g'){
        return "image/png";
    } else if (fileContent[n - 3] == 'j' && fileContent[n - 2] == 'p' && fileContent[n - 1] == 'g'){
        return "image/jpeg";
    }
    return "";
}

void sendMail(string from, string to, string subject, string body, string userPass, string fileContent = "") {
    const char* filename = "sendMail.bat";

    ofstream batFile(filename);
    if (!batFile) {
        cerr << "Can't create file: " << filename << endl;
        return;
    }

    batFile << "@echo off\n";

    batFile << "(\n";
    batFile << "echo From: " << from << "\n";
    batFile << "echo To: " << to << "\n";
    batFile << "echo Subject: " << subject << "\n";

    if (fileContent.empty()) {
        // Nếu không có file đính kèm, gửi email đơn giản
        batFile << "echo Content-Type: text/plain; charset=\"UTF-8\"\n";
        batFile << "echo.\n";
        batFile << "echo " << body << "\n";
        batFile << ") > sendEmail.txt\n\n";
    } else {
        // Nếu có file đính kèm, gửi email với MIME
        batFile << "echo MIME-Version: 1.0\n";
        batFile << "echo Content-Type: multipart/mixed; boundary=\"boundary\"\n";
        batFile << "echo Content-Transfer-Encoding: 7bit\n";
        batFile << "echo.\n";
        batFile << "echo --boundary\n";
        batFile << "echo Content-Type: text/plain; charset=\"UTF-8\"\n";
        batFile << "echo.\n";
        batFile << "echo " << body << "\n";
        batFile << "echo.\n";
        batFile << "echo --boundary\n";

        string contentType = getContentType(fileContent);
        batFile << "echo Content-Type: " << contentType << "; name=\"" << fileContent << "\"\n";
        batFile << "echo Content-Disposition: attachment; filename=\"" << fileContent << "\"\n";
        batFile << "echo Content-Transfer-Encoding: base64\n";
        batFile << "echo.\n";
        batFile << "certutil -encode " << fileContent << " temp1.txt >nul\n";
        batFile << "findstr /v /c:- temp1.txt\n";
        batFile << "del temp1.txt\n";
        batFile << "echo.\n";
        batFile << "echo --boundary--\n";
        batFile << ") > sendEmail.txt\n\n";
    }

    batFile << "curl --url \"smtp://smtp.gmail.com:587\" --ssl-reqd ^\n";
    batFile << "     --mail-from \"" << from << "\" ^\n";
    batFile << "     --mail-rcpt \"" << to << "\" ^\n";
    batFile << "     --user \"" << userPass << "\" ^\n";
    batFile << "     -T sendEmail.txt\n";
    batFile << "pause\n";

    batFile.close();
    cout << "File " << filename << " created successfully." << endl;

    int result = system(filename);
    if (result == -1) {
        cerr << "Can't execute file: " << filename << endl;
        return;
    }

    remove("sendEmail.txt");
    remove("sendMail.bat");
}

// bool client == true -> request; client == false -> response 
// string task = list app / camera / screenshot....
// client request -> fileContent = ""; server response -> fileContent = "abc.txt/png"

void newMail(bool client, string task, string numTask, string fileContent){
    string daytime = getCurrentDateTime();
    string typeOfSend;
    if (client) typeOfSend = "[request_" + numTask + "]: ";
    else typeOfSend = "[response_" + numTask + "]: ";
    string from = "ductai.dt05@gmail.com";
    string to = "ductai.dt05@gmail.com";
    string subject = typeOfSend + daytime;
    string body = (client ? "[task] " : "[rep] ") + task;
    string userPass = "ductai.dt05@gmail.com:gofe zjfi ktmw fxno";

    sendMail(from, to, subject, body, userPass, fileContent);
}
