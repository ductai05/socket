#include <vector>
#include <fstream>
#include <iostream>
// #include <filesystem>
#include <stdio.h>
#include <stdlib.h>
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
#include <map>
#include <regex>
// #include "process.h"
#include "socket.h"
using namespace std;

// void start_server()
// {
//     list_apps();
// }

int main()
{
    // newMail(true, "list appss", "6", "");
    // start_server();
    autoGetMail(false); // false =: server
    system("pause");
}

/*
void list_apps();  // tìm file .exe và viết vào file "app_list.txt"
void list_files(const string &path); // in ra các file trong thư mục path
void find_app_path(const string &app_name); // tìm các đường dẫn có tên là app_name
void run_app(const string &path); // chạy app khi biết đường dẫn file .exe
void get_screenshot(); // chụp màn hình rồi lưu thành screen.jpeg
void shut_down(); // tắt máy
void camera_switch(bool cam_status); // 0/1 -> tắt/bật cam
*/