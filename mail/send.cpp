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


int main()
{
    string task;
    task = "list_apps";
    // list_apps; list_services; get_screenshot; shutdown; webcam_on; webcam_off; running_apps;
    // get_file “<đường dẫn>” 
    // list_files “<đường dẫn>” 
    // find_path “<tên app>”
    // run_app “<đường dẫn>”
    // close_app “<tên.exe>”

    newMail(true, task, "0", ""); // true =: client send request to server
}
