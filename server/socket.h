#ifndef PROCESS_H
#define PROCESS_H
namespace fs = std::filesystem;

// Các hàm xây dựng
void list_apps();  // tìm file .exe và viết vào file "app_list.txt"
void list_files(const string &path); // in ra các file trong thư mục path
void find_app_path(const string &app_name); // tìm các đường dẫn có tên là app_name
bool run_app(const std::string &path); // chạy app khi biết đường dẫn file .exe
void get_screenshot(); // chụp màn hình rồi lưu thành screen.jpeg
void shut_down(); // tắt máy
bool camera_switch(bool cam_status); // 0/1 -> tắt/bật cam
bool end_task(const std::string& task_name); // kill một chương trình
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
    system("c:\\windows\\system32\\shutdown /s");
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
    vector<string> paths = {"C:/Program Files (x86)", "C:/Program Files", "C:/Users/kHOA/AppData/Local", "C:/Users/kHOA/AppData/Roaming"};
    ofstream outFile("uploads\\apps_list.txt");
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

void buildlps(const string &s, int *&lps)
{
    int n = s.size();
    lps = new int[n];
    for(int i = 0; i < n; i++)
        lps[i] = 0;
    for(int i = 1; i < n; i++)
    {
        int j = lps[i - 1];
        while(j > 0 && s[i] != s[j]) 
            j = lps[j - 1];
        if(s[i] == s[j]) j++;
        lps[i] = j;
    }
}

bool is_substr(string a, string b, int *&lps) // KMP
{
    for(char &x : a) x = tolower(x);
    for(char &x : b) x = tolower(x);
    int n = a.size(), m = b.size();
    int j = 0;
    for(int i = 0; i < n; i++)
    {
        while(j > 0 && a[i] != b[j])
            j = lps[j - 1];
        if(a[i] == b[j]) j++;
        if(j == m)
        {
            return true;
        }
    }
    return false;
}
// in các đường dẫn tìm được vào "app_found_paths.txt"
void find_app_path(const string &app_name)
{
    const string apps_list = "uploads\\apps_list.txt";
    int *lps;
    buildlps(app_name, lps);

    bool found = false;
    ifstream fin(apps_list);
    ofstream fout("uploads\\app_found_paths.txt");
    string line;
    while(getline(fin, line))
    {
        if(is_substr(line, app_name, lps))
        {
            fout << line << '\n';
            found = true;
        }
    }
    if(!found) fout << "Application path cannot be found.";
    delete[] lps;
    fin.close();
    fout.close();
}

bool run_app(const std::string &path) {
    // Convert std::string to LPCTSTR
    LPCSTR applicationName = path.c_str();
    
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
    system("g++ -o take_screenshot.exe take_screenshot.cpp -lgdiplus -lcomdlg32 -lole32 -loleaut32 -luuid -lwinmm -lgdi32 -luser32");
    system("take_screenshot.exe");
    return;
}

bool end_task(const std::string& task_name) 
{
    std::string command = "taskkill /IM " + task_name + " /F";
    int result = system(command.c_str());
    return (result == 0); // Trả về true nếu lệnh thành công, false nếu không
}

void list_running_apps()
{
    system("tasklist > uploads\\running_apps.txt");
}

#endif
