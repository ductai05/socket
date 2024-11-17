// main.cpp
#include <iostream>
#include "sendmail.h"

int main(){
    // client (request): 
    // newMail(true, "di dua xe", "1", "");

    // server (response): 
    newMail(false, "gui cho file cpp ne", "2", "note.txt");
    return 0;
}