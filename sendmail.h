// sendmail.h
#ifndef SENDMAIL_H
#define SENDMAIL_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <sstream>
#include <iomanip>
using namespace std;

string getCurrentDateTime();
string getContentType(string fileContent);
void sendMail(string from, string to, string subject, string body, string userPass, string fileContent);
void newMail(bool client, string task, string fileContent);

#endif
