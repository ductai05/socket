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
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <filesystem>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

#include "process.h"
#include "socket.h"

#define PORT 8080

void handle_request(const string &numTask, const string &body, SOCKET &socket, SOCKET &server_fd)
{
    if (body == "list_apps")
    {
        ofstream outFile("uploads/apps_list.txt");
        ifstream inFile("uploads/app_paths.txt");
        string line;
        while (getline(inFile, line))
        {
            size_t lastSlash = line.find_last_of("/");
            outFile << line.substr(lastSlash + 1) << '\n';
        }
        outFile.close();
        inFile.close();
        newMail(false, body, numTask, "uploads/apps_list.txt");
        remove("uploads/apps_list.txt");
    }
    else if (body == "get_screenshot")
    {
        get_screenshot();
        newMail(false, body, numTask, "uploads/screen.png");
        remove("uploads/screen.png");
    }
    else if (body == "shutdown")
    {
        // gửi response và đóng socket
        const char *response = "SUCCESS";
        send(socket, response, strlen(response), 0);
        closesocket(socket);
        closesocket(server_fd);
        WSACleanup();

        // tắt máy
        shut_down();
        exit(0);
    }
    else if (body == "webcam_on")
    {
        ofstream outFile("uploads/messages.txt");
        if (camera_switch(1))
        {
            outFile << "Successfully turned on the camera";
        }
        else
        {
            outFile << "Failed when turning on the camera";
        }
        outFile.close();
        newMail(false, body, numTask, "uploads/messages.txt");
        remove("uploads/messages.txt");
    }
    else if (body == "webcam_off")
    {
        ofstream outFile("uploads/messages.txt");
        if (camera_switch(0))
        {
            outFile << "Successfully turned off the camera";
        }
        else
        {
            outFile << "Failed when turning off the camera";
        }
        outFile.close();
        newMail(false, body, numTask, "uploads/messages.txt");
        remove("uploads/messages.txt");
    }
    else if (body.find("get_file") != string::npos)
    {
        string path = get_path(body);
        DWORD fileAttr = GetFileAttributesA(path.c_str());
        if (!(fileAttr != INVALID_FILE_ATTRIBUTES))
        {

            ofstream outFile("uploads/messages.txt");
            outFile << "Invalid path";
            outFile.close();
            newMail(false, body, numTask, "uploads/messages.txt");
            remove("uploads/messages.txt");
        }
        else
            newMail(false, body, numTask, path);
    }
    else if (body.find("list_file") != string::npos)
    {
        string path = get_path(body);
        DWORD fileAttr = GetFileAttributesA(path.c_str());
        if (!(fileAttr != INVALID_FILE_ATTRIBUTES))
        {
            ofstream outFile("uploads/messages.txt");
            outFile << "Invalid path";
            outFile.close();
            newMail(false, body, numTask, "uploads/messages.txt");
            remove("uploads/messages.txt");
        }
        else
        {
            list_files(path);
            newMail(false, body, numTask, "uploads/files_list.txt");
            remove("uploads/files_list.txt");
        }
    }
    else if (body.find("run_app") != string::npos)
    {
        ofstream outFile("uploads/messages.txt");
        string path = get_path(body);
        if (run_app(path))
            outFile << "Successfully run the application\n";
        else
            outFile << "Failed when running the application\n";
        outFile.close();
        newMail(false, body, numTask, "uploads/messages.txt");
        remove("uploads/messages.txt");
    }
    else if (body == "running_apps")
    {
        list_running_apps();
        newMail(false, body, numTask, "uploads/running_apps.txt");
        remove("uploads/running_apps.txt");
    }
    else if (body.find("close_app") != string::npos)
    {
        string app = get_path(body);
        ofstream outFile("uploads/messages.txt");
        if (end_task(app))
        {
            outFile << "Successfully close the application.\n";
        }
        else
        {
            outFile << "Failed when closing the application.\n";
        }
        outFile.close();
        newMail(false, body, numTask, "uploads/messages.txt");
        remove("uploads/messages.txt");
    }
    else if (body.find("close_by_id") != string::npos)
    {
        string PID = get_path(body);
        ofstream outFile("uploads/messages.txt");
        if (end_task_PID(PID))
        {
            outFile << "Successfully close the application.\n";
        }
        else
        {
            outFile << "Failed when closing the application.\n";
        }
        outFile.close();
        newMail(false, body, numTask, "uploads/messages.txt");
        remove("uploads/messages.txt");
    }
    else if(body.find("delete_file") != string::npos)
    {
        string path = get_path(body);
        ofstream outFile("uploads/messages.txt");
        if(remove(path.c_str()) == 0)
        {
            outFile << "successfully deleted the file.\n";
        }
        else
        {
            outFile << "Failed when deleting the file.\n";
        }
        outFile.close();
        newMail(false, body, numTask, "uploads/messages.txt");
        remove("uploads/messages.txt");
    }
    else
    {
        cerr << "[Service not found]\n";
        ofstream outFile("uploads/messages.txt");
        outFile << "Service not found!\n";
        outFile.close();
        newMail(false, body, numTask, "uploads/messages.txt");
        remove("uploads/messages.txt");
    }
    const char *response = "SUCCESS";
    send(socket, response, strlen(response), 0);
}

void createSocket(bool logIP = true)
{
    WSADATA wsaData;
    SOCKET server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "WSAStartup failed\n";
        return;
    }

    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        cerr << "Socket failed\n";
        WSACleanup();
        return;
    }

    // Thiết lập thông tin địa chỉ
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Gắn socket với địa chỉ
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
    {
        cerr << "Bind failed\n";
        closesocket(server_fd);
        WSACleanup();
        return;
    }

    if(logIP)
    {
        // Lấy địa chỉ IP thực tế
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET; // IPv4
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(hostname, NULL, &hints, &res) == 0)
        {
            char ipStr[INET_ADDRSTRLEN];
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, ipStr, sizeof(ipStr));
            cout << "Server IP Address: " << ipStr << endl;
            freeaddrinfo(res);
        }
        else
        {
            cerr << "Could not get IP address\n";
        }
    }

    // Lắng nghe kết nối
    cerr << "Waiting for new request...";
    if (listen(server_fd, 3) == SOCKET_ERROR)
    {
        cerr << "Listen failed\n";
        closesocket(server_fd);
        WSACleanup();
        return;
    }

    // Chấp nhận kết nối
    SOCKET new_socket;
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET)
    {
        cerr << "Accept failed\n";
        closesocket(server_fd);
        WSACleanup();
        return;
    }

    recv(new_socket, buffer, sizeof(buffer), 0);
    stringstream ss(buffer);
    string numTask, body;
    ss >> numTask >> body;
    
    // Xử lý yêu cầu
    handle_request(numTask, body, new_socket, server_fd);
    
    closesocket(server_fd);
    WSACleanup();
    return;
}

void serveForever()
{
    createSocket();
    while(true)
    {
        createSocket(false);
    }
}

// Khởi động và lấy các đường dẫn
void start_server()
{
    // const char* dirName = "uploads";
    // DWORD ftyp = GetFileAttributesA(dirName);
    // if (ftyp == INVALID_FILE_ATTRIBUTES)
    //     CreateDirectoryA(dirName, NULL);

    // auto start = chrono::high_resolution_clock::now();
    // cout << "Starting server...\n";
    // list_apps();
    // auto end = chrono::high_resolution_clock::now();
    // chrono::duration<double> elapsed = end - start;
    // cout << "Server started successfully after: " << elapsed.count() << " second(s)" << '\n';
    serveForever();
}

int main()
{
    // start_server();
    string path = "D:/";
    list_files(path);
}
