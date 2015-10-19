//
//  fileOperations.cpp
//  TWMailer
//
//  Created by Lukas Reisinger on 19.10.15.
//  Copyright (c) 2015 Lukas Reisinger. All rights reserved.
//

#include "fileOperations.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>

#include <vector>
#define saveDir "data"
using namespace std;

bool saveMessage(char* empfaenger, char* sender, char* betreff, char* nachricht){

    char path[30];
    getUserpath(empfaenger, path);

    createDirectory(saveDir);
    createDirectory(path);


    char fileNamePath[30];
    memset(fileNamePath, '\0', sizeof(char)*30);
    getNextFileNamePath(path, fileNamePath);

    ofstream myfile;
    myfile.open (fileNamePath);
    if (myfile.fail()) {
        return false;
    }
    myfile << sender << endl << betreff << endl << nachricht << endl;
    myfile.close();
    return true;
}

void listMessages(char* username, vector<char*>* subjects)
{
    vector<char*> entries;

    char path[30];   // array to hold the result.
    getUserpath(username, path);


    if(isDirectoryPresent(path)){
        getDirList(path, &entries);

        for (vector<char*>::iterator it = entries.begin(); it != entries.end(); ++it)
        {
            //char tmp_subject[81];
            char* tmp_subject = (char*)malloc(sizeof(char)*81);

            char tmp_filePath[30];

            strcpy(tmp_filePath, path);
            strcat(tmp_filePath, *it);
            bool success = getMailSubject(tmp_filePath, tmp_subject);// 1 success; -1 not
            if(success){
                cout << "Subject: " << tmp_subject << endl;
                subjects->push_back(tmp_subject);
            }
        }
    }
}

bool getMailSubject(char* path, char* subject_out){
    char line[81];
    memset(line, '\0', sizeof(char)*81);

    FILE *fp = fopen(path,"r");
    if( fp == NULL )
    {
        perror("Error while opening the file.\n");
        return false;
    }

    int i = 0;
    while(fgets(line,80, fp)!=NULL){
        if(i == 1)
        {
            strcpy(subject_out, strtok (line,"\n"));// \n wegsplitten
            break;
        }
        i++;
    }
    fclose(fp);
    return true;
}

bool getMailMessage(char* username, int fileNr, char* msg_out)
{
    char mailNr[4];
    memset(mailNr, '\0', sizeof(char)*4);

    char path[30];   // array to hold the result.
    getUserpath(username, path);
    strcat(path, "mail");

    sprintf(mailNr,"%d",fileNr);
    strcat(path, mailNr);


    char line[81];
    memset(line, '\0', sizeof(char)*81);

    FILE *fp = fopen(path,"r");
    if( fp == NULL )
    {
        perror("Error while opening the file.\n");
        return false;
    }

    int i = 0;
    while(fgets(line,80, fp)!=NULL){
        if(i == 2)
        {
            strcpy(msg_out, strtok (line,"\n"));// \n wegsplitten
            strcat(msg_out, "\n");
        }
        else if (i > 2)
        {
            strcat(msg_out, strtok (line,"\n"));// \n wegsplitten
            strcat(msg_out, "\n");
        }
        i++;
    }
    fclose(fp);

    return true;
}

bool deleteMail(char* username, int fileNr)
{
    char mailNr[4];
    memset(mailNr, '\0', sizeof(char)*4);
    
    char basepath[30];   //data/username/mail
    char path[30];   // data/username/mail12
    getUserpath(username, path);
    strcat(path, "mail");
    
    sprintf(mailNr,"%d",fileNr);
    strcat(path, mailNr);
    
    if( remove(path) != 0 )
    {
        return false;
    }
    
    
    
    char oldname[] = "images//test_1.jpg";
    char newname[] = "images//test//test_2.jpg";
    check = rename(oldname, newname);
    if (check == 0)
        puts("Success");
    else
        perror("Failed");
    return 0;
}


void getUserpath(char* username, char* path_out)
{
    memset(path_out, '\0', sizeof(char)*30);

    strcpy(path_out,saveDir); // copy string one into the result.
    strcat(path_out,"/");
    strcat(path_out,username); // append string two to the result.
    strcat(path_out,"/");
}

bool isDirectoryPresent(char* dir){
    struct stat st = {0};

    if (stat(dir, &st) == -1) {
        cout << "dir is NOT present; path=" << dir << endl;
        return false;
    }
    cout << "dir is present; path=" << dir << endl;
    return true;
}

void getNextFileNamePath(const char* path, char* filenamepath)
{
    char filepath_tmp[30];

    int i = -1;
    char number[4];

    do
    {
        i++;
        sprintf(number, "%d", i);

        strcpy(filepath_tmp, path);
        strcat(filepath_tmp, "/mail");
        strcat(filepath_tmp, number);

    }
    while(fileExists(filepath_tmp));
    strcpy(filenamepath, filepath_tmp);
}

void createDirectory(char* dir){
    if(!isDirectoryPresent(dir))
    {
        mkdir(dir, 0700);
    }
}

void getDirList(char* path, vector<char*> * entries)
{

    DIR *dirHandle;
    dirent *dirEntry;

    dirHandle = opendir(path); /* oeffne aktuelles Verzeichnis */
    if (dirHandle) {
        while (0 != (dirEntry = readdir(dirHandle))) {
            if(dirEntry->d_type != DT_DIR){
                entries->push_back(dirEntry->d_name);
                cout << path << dirEntry->d_name << endl;
            }
        }
        closedir(dirHandle);
    }
}

bool fileExists(const char* file) {
    struct stat buf;
    return (stat(file, &buf) == 0);
}
