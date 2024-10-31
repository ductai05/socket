#include <iostream>
#include <fstream>
#include <cstdlib> // use func system
#include <string>

using namespace std;

string getContentType(string fileContent){
    int n = fileContent.size();
    if (fileContent[n - 3] == 't' && fileContent[n - 2] == 'x' && fileContent[n - 1] == 't'){
        return "text/plain";
    } else if (fileContent[n - 3] == 'p' && fileContent[n - 2] == 'n' && fileContent[n - 1] == 'g'){
        return "image/png";
    } else if (fileContent[n - 3] == 'j' && fileContent[n - 2] == 'p' && fileContent[n - 1] == 'g'){
        return "image/png";
    }
}

void sendMail(string from, string to, string subject, string body, string userPass, string fileContent = "") {
    const char* filename = "sendMail.bat";

    // Tạo file .bat để gửi mail
    ofstream batFile(filename);
    if (!batFile) {
        cerr << "Can't create file: " << filename << endl;
        return; // Lỗi
    }

    // Ghi nội dung vào file .bat
    batFile << "@echo off\n";
    batFile << "(\n";
    batFile << "echo From: " << from << "\n";
    batFile << "echo To: " << to << "\n";
    batFile << "echo Subject: " << subject << "\n";
    batFile << "echo MIME-Version: 1.0\n";
    batFile << "echo Content-Type: multipart/mixed; boundary=\"boundary\"\n";
    batFile << "echo.\n";
    batFile << "echo --boundary\n";
    batFile << "echo Content-Type: text/plain; charset=\"UTF-8\"\n";
    batFile << "echo.\n";
    batFile << "echo " << body << "\n";
    batFile << "echo.\n";
    batFile << "echo --boundary\n\n";

    if (fileContent != ""){
        string contentType = getContentType(fileContent); 

        batFile << "echo Content-Type: "<< contentType << "; name=\"" << fileContent << "\"\n";
        batFile << "echo Content-Disposition: attachment; filename=\"" << fileContent << "\"\n";
        batFile << "echo Content-Transfer-Encoding: base64\n";
        batFile << "echo.\n";
        batFile << "certutil -encode "<< fileContent << " temp1.txt\n";
        batFile << "type temp1.txt\n";
        batFile << "del temp1.txt\n";
        batFile << "echo.\n";
        batFile << "echo --boundary--\n";
        batFile << ") > sendEmail.txt\n\n";
    }
    

    // curl execute
    batFile << "curl --url \"smtp://smtp.gmail.com:587\" --ssl-reqd ^\n";
    batFile << "     --mail-from \"" << from << "\" ^\n";
    batFile << "     --mail-rcpt \"" << to << "\" ^\n";
    batFile << "     --user \"" << userPass << "\" ^\n";
    batFile << "     -T sendEmail.txt\n";
    batFile << "pause\n"; // Dừng lại để xem thông báo

    // Đóng file
    batFile.close();
    cout << "File " << filename << " created successfully." << endl;

    // Thực thi file .bat
    int result = system(filename);
    if (result == -1) {
        cerr << "Can't execute file: " << filename << endl;
        return; // Lỗi
    }
}

int main() {
    string from = "ductai.dt05@gmail.com";
    string to = "ductai.dt05@gmail.com";
    string subject = "test subject email";
    string body = "test body email";
    string userPass = "ductai.dt05@gmail.com:mymx aeny jlka inys"; //app password, tao bang setting gmail/acc google
    string fileContent = "dandadan.png"; // file dinh kem (moi test 1 file, png/txt)

    sendMail(from, to, subject, body, userPass, fileContent);

    return 0; // Kết thúc chương trình
}