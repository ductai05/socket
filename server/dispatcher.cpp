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
#include <mutex>
#include <thread>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <windows.h>
#include <filesystem>
#pragma comment(lib, "ws2_32.lib")

using namespace std;
#include "socket.h"

#define PORT 65535

string sendTask(const string &serverIP, const string &numTask, const string &body)
{
    string message = numTask + " " + body;
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return "Error";
    }

    // Tạo socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        WSACleanup();
        return "Error";
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(serverIP.c_str());

    // Kết nối đến server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
    {
        closesocket(sock);
        WSACleanup();
        return "Error";
    }

    // Gửi dữ liệu đến server
    send(sock, message.c_str(), message.size(), 0);

    // Đọc phản hồi từ server
    recv(sock, buffer, sizeof(buffer), 0);

    // Đóng socket
    closesocket(sock);
    WSACleanup();

    return buffer;
}

mutex mtx;
bool isPortOpen(const string &ip, int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) return false;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // Try to connect
    bool isOpen = connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != SOCKET_ERROR;

    closesocket(sock);
    return isOpen;
}

#define CLOCK std::chrono::high_resolution_clock::time_point 

void checkIP(map<string, bool> &serversIP, const string &ip, int port) {
    if (isPortOpen(ip, port)) 
    {
        lock_guard<mutex> lock(mtx);
        serversIP[ip] = true;
    }
}

void getServersList(map<string, bool> &serversIP) 
{
    auto start = chrono::high_resolution_clock::now();
    cout << "Detecting servers IP...\n";
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        cerr << "WSAStartup failed\n";
        return;
    }

    const int startIP = 1;
    const int endIP = 245;
    vector<thread> threads;

    for (int i = startIP; i <= endIP; ++i) {
        string ip = "192.168.1." + to_string(i);
        threads.emplace_back(ref(checkIP), ref(serversIP), ip, PORT); // Sử dụng ref
    }

    for (auto &thread : threads)
        thread.join();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Scan completed after " << elapsed.count() << " second(s)" << '\n';

    int cnt = 1;
    for (const auto &item : serversIP) {
        cout << "server " << cnt << ": " << item.first << ' ';
        cout << "[" << (item.second ? "online" : "offline") << "]\n";
        cnt++;
    }
    if(serversIP.size() == 0)
    {
        cout << "No server found.";
        WSACleanup();
        exit(0);
    }
    WSACleanup();
}

void handleRequest(map<string, bool> &serversIP, string &currentIP, const string &numTask, const string &body)
{
    if (body == "INVALID")
    {
        ofstream outFile("messages.txt");
        outFile << "Invalid request.";
        outFile.close();
        newMail(false, body, numTask, "messages.txt");
        remove("messages.txt");
    }
    else if (currentIP == "")
    {
        ofstream outFile("messages.txt");
        outFile << "No servers available.";
        outFile.close();
        newMail(false, body, numTask, "messages.txt");
        remove("messages.txt");
    }
    else if (body == "servers_IP")
    {
        ofstream outFile("servers_IP.txt");
        int cnt = 1;
        for (const pair<string, bool> &item : serversIP)
        {
            outFile << "server " << cnt << ": " << item.first << ' ';
            outFile << "[" << (item.second == true ? "online" : "offline") << "] ";
            if (currentIP == item.first)
                outFile << "[current server]\n";
            else
                outFile << "\n";
            cnt++;
        }
        outFile.close();
        newMail(false, body, numTask, "servers_IP.txt");
        remove("servers_IP.txt");
    }
    else if (body.find("change_server") != string::npos)
    {
        ofstream outFile("messages.txt");
        string newIP = get_path(body);
        if (serversIP.count(newIP) == 0 || serversIP[newIP] == false)
        {
            outFile << "Failed when changing server.";
        }
        else
        {
            currentIP = newIP;
            outFile << "Successfully changed server.";
        }
        outFile.close();
        newMail(false, body, numTask, "messages.txt");
        remove("messages.txt");
    }
    else if (body == "shutdown")
    {
        ofstream outFile("messages.txt");
        sendTask(currentIP, numTask, body);
        serversIP[currentIP] = false;
        outFile << "Shutting down current server.";

        currentIP = "";
        bool changed = 0;
        int cnt = 1;
        for (const pair<string, bool> &item : serversIP)
        {
            if (serversIP[item.first] == true)
            {
                changed = 1;
                outFile << "Change to server " << cnt << ": " << item.first;
                currentIP = item.first;
            }
            cnt++;
        }
        if (!changed)
            outFile << "No more server available.";
        outFile.close();
        newMail(false, body, numTask, "messages.txt");
        remove("messages.txt");
    }
    else
    {
        sendTask(currentIP, numTask, body);
    }
}

void autoGetMail(map<string, bool> &serversIP, bool isClientLISTEN = false)
{
    string currentIP = serversIP.begin()->first;
    string userPass = "ai23socket@gmail.com:nhrr llaa ggzb yzbj";

    int orderNow = -2;
    vector<string> allMAIL;
    vector<string> allTASK;

    getID(userPass, isClientLISTEN);
    readIDMail(orderNow); // get init order

    string timeLISTEN = getCurrentDateTime();
    cout << "Start listen at: " << timeLISTEN << "\n";

    bool waiting = true;
    while (true)
    {
        if (!getID(userPass, isClientLISTEN))
            break;
        if (readIDMail(orderNow))
        {
            cout << "* New email has been found!\n";
            if (getNewestMail(orderNow, userPass))
            {
                string numTask, body;
                tie(numTask, body) = readLatestMail(timeLISTEN, isClientLISTEN, allMAIL, allTASK);
                handleRequest(serversIP, currentIP, numTask, body);
                waiting = true;

                // Get new order
                getID(userPass, isClientLISTEN);
                readIDMail(orderNow);
            }
        }
        if (waiting)
        {
            cout << "* Waiting for new request...\n";
            waiting = false;
        }
        Sleep(200);
    }
}

int main()
{
    map<string, bool> serversIP; // bool: accessable, string: IP
    getServersList(serversIP);
    autoGetMail(serversIP);

    return 0;
}