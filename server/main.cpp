#include <map>
#include <set>
#include <regex>
#include <ctime>
#include <vector>
#include <string>
#include <cstdio>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <windows.h>
#include <filesystem>
using namespace std;

#include "process.h"
#include "socket.h"

// Khởi động và lấy các đường dẫn
void start_server()
{    
    auto start = chrono::high_resolution_clock::now();
    cout << "Starting server...\n";
    list_apps();
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Server started successfully after: " << elapsed.count() << " second(s)" << '\n';
    autoGetMail(false);
}

int main()
{    
    start_server();
}
