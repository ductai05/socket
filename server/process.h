#ifndef PROCESS_H
#define PROCESS_H
namespace fs = std::filesystem;

// Các hàm xây dựng
void list_apps();  // tìm file .exe và viết vào file "app_list.txt"
void list_files(const string &path); // in ra các file trong thư mục path
bool run_app(string path); // chạy app khi biết đường dẫn file .exe
void get_screenshot(); // chụp màn hình rồi lưu thành screen.png
void shut_down(); // tắt máy
bool camera_switch(bool cam_status); // 0/1 -> tắt/bật cam
bool end_task(const std::string& task_name); // kill một chương trình
bool end_task_PID(const string& PID); // kill một tiến trình bằng PID
void list_running_apps(); // viết danh sách các chương trình đang chạy vào file running_apps.txt

bool camera_switch(bool cam_status) 
{
    int result;
    if (cam_status) { // turn on
        result = system("start microsoft.windows.camera:");
    } else { // turn off
        result = system("taskkill /im WindowsCamera.exe /f");
    }
    return (result == 0); // Trả về true nếu lệnh thành công, false nếu không
}

void shut_down()
{
    system("c:\\windows\\system32\\shutdown /s /t 5");
}

// nếu str kết thúc bằng ending
bool endsWith(const string& str, const string& ending) 
{
    if (str.length() >= ending.length()) 
        return str.compare(str.length() - ending.length(), ending.length(), ending) == 0;
    return false;
}

// tìm bằng cách đệ quy vào từng thư mục
void find_apps_recur(const string& path, ofstream &outFile)
{
    if (!fs::exists(path) || !fs::is_directory(path))
        return;
    try
    {
        for (const auto& entry : fs::directory_iterator(path)) 
        {
            string filename = entry.path().filename().string();

            if (fs::is_directory(entry.status())) 
                find_apps_recur(path + "/" + filename, outFile);
            else if(endsWith(filename, ".exe"))
                outFile << path + "/" + filename << '\n';
        }
    }
    catch(const fs::filesystem_error& e){}
}

void list_apps()
{
    vector<string> paths = {"C:/Program Files (x86)", "C:/Program Files"};
    const char* appDataPath = std::getenv("LOCALAPPDATA"); // Lấy biến môi trường APPDATA
    if (appDataPath)
        paths.push_back(string(appDataPath));

    ofstream outFile("uploads\\app_paths.txt");
    for(string path : paths)
    { 
        if (!fs::exists(path) || !fs::is_directory(path)) 
        {
            outFile << "The Specified Path is Invalid!";
            outFile.close();
            return;
        }
        find_apps_recur(path, outFile);
    }
    outFile.close();
}

void list_files(const string &path) 
{
    ofstream outFile("uploads\\files_list.txt");
    if (!fs::exists(path) || !fs::is_directory(path)) 
    {
        outFile << "The Specified Path is Invalid!";
        outFile.close();
        return;
    }

    outFile << path << '\n';
    for (const auto& entry : fs::directory_iterator(path)) 
    {
        outFile << "|--- " << entry.path().filename().string();
        if (fs::is_directory(entry.status())) 
        {
            outFile << "/";
        }
        outFile << '\n';
    }
    outFile.close();
}

bool run_app(string app) 
{
    // Tìm đường dẫn tương ứng
    ifstream inFile("uploads/app_paths.txt");
    string line;
    for(char &ch : app) ch = tolower(ch);
    while(getline(inFile, line))
    {
        size_t lastSlash = line.find_last_of("/");
        string name = line.substr(lastSlash + 1);
        for(char &ch : name) ch = tolower(ch);
        if(name == app)
        {
            app = line;
            break;
        }
    }
    cerr << app;
    inFile.close();

    LPCSTR applicationName = app.c_str();
    
    // Additional information
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // Set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the program up
    BOOL success = CreateProcess(
        applicationName, // Path to the executable
        NULL,            // Command line arguments (NULL for none)
        NULL,            // Process handle not inheritable
        NULL,            // Thread handle not inheritable
        FALSE,           // Set handle inheritance to FALSE
        0,               // No creation flags
        NULL,            // Use parent's environment block
        NULL,            // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi              // Pointer to PROCESS_INFORMATION structure
    );

    // Close process and thread handles
    if (success) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true; // Indicate success
    }
    return false; // Indicate failure
}

void get_screenshot()
{
    int result = system("take_screenshot.exe");
    if(result != 0)
    {
        system("g++ -o take_screenshot.exe take_screenshot.cpp -lgdiplus -lcomdlg32 -lole32 -loleaut32 -luuid -lwinmm -lgdi32 -luser32");
        system("take_screenshot.exe");
    }
    return;
}

bool end_task(const std::string& task_name) // appname.exe
{
    std::string command = "taskkill /IM " + task_name + " /F";
    int result = system(command.c_str());
    return (result == 0); // Trả về true nếu lệnh thành công, false nếu không
}

bool end_task_PID(const string& PID) // PID của tiến trình
{
    std::string command = "taskkill /PID " + PID + " /F";
    int result = system(command.c_str());
    return (result == 0);
}

void list_running_apps()
{
    system("tasklist > uploads\\running_apps.txt");
}

#endif