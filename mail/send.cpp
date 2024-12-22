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
#include "mail.h"
using namespace std;


int main()
{
    // list_apps;
    // get_screenshot;
    // shutdown;
    // webcam_on;
    // webcam_off;
    // running_apps;
    // get_file “<đường dẫn>”
    // list_files “<đường dẫn>”
    // find_path “<tên app>”
    // run_app “<đường dẫn>”
    // close_app “<tên.exe>”
    // delete_file "<đường dẫn>"
    // servers_IP
    // change_server "<IP>"
    // close_by_id "<PID>"
    string command = "close_app";
    string text = "zalo.exe";
    srand(time(0));
    string id = to_string(rand());
    
    string request = command + (text == "" ? text : " \"" + text + "\"");
    newMail(true, request, id, ""); // true =: client send request to server
}
