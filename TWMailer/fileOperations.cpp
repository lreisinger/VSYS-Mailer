//
//  fileOperations.cpp
//  TWMailer
//
//  Created by Lukas Reisinger on 19.10.15.
//  Copyright (c) 2015 Lukas Reisinger. All rights reserved.
//

#include "fileOperations.h"

#include <sys/stat.h>

#include <iostream>
#include <fstream>
#define saveDir "data"
using namespace std;

void saveMessage(char* empfaenger, char* sender, char* betreff, char* nachricht){
    
    char path[30];   // array to hold the result.
    memset(path, '\0', sizeof(char)*20);
    
    strcpy(path,saveDir); // copy string one into the result.
    strcat(path,empfaenger); // append string two to the result.
    
    createDirectory(saveDir);
    createDirectory(path);
    
    ofstream myfile;
    myfile.open ("example.txt");
    myfile << "Writing this to a file.\n";
    myfile.close();
}

bool isDirectoryPresent(char* dir){
    struct stat st = {0};
    
    if (stat(dir, &st) == -1) {
        return false;
    }
    return true;
}

void createDirectory(char* dir){
    if(!isDirectoryPresent(dir))
    {
        mkdir(dir, 0700);
    }
}