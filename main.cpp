// main.cpp
#include <iostream>
#include "sendmail.h"
#include "receive.h"

int main(){
    // client (request): 
    // newMail(true, "di dua xe", "1", "");

    // server (response): 
    // newMail(false, "hehe, test ket hop", "1", "note.txt");
    autoGetMail();
    return 0;
}