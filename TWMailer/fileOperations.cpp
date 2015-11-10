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
#include <algorithm>

#include <vector>

#define MAXFILESIZE 1024

using namespace std;

bool saveMessage(char* empfaenger, char* sender, char* betreff, char* nachricht){
    
    char* singleEmpfaenger = strtok (empfaenger,",");
    while (singleEmpfaenger != NULL)
    {
        
        char path[30];
        getUserpath(singleEmpfaenger, path);
        
        createDirectory(mailspool);
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
        
        
        singleEmpfaenger = strtok (NULL, ",");
    }
    return true;
}


bool saveAttachment(char* attach, int bytes, char* filename, char* user){
    
    char path[50];
    memset(path, '\0', sizeof(char)*50);
    getUserpath(user, path);
    
    createDirectory(mailspool);
    createDirectory(path);
    
    strcat(path, "Attachments/");
    createDirectory(path);
    
    strcpy(path, filename);
    
    
    FILE* attachedFile = fopen(path, "w");
    if (attachedFile == NULL)
    {
        printf("Failed to save attachment...\n");
        return false;
    }
    else
    {
        printf("opened attachment successfully\n");
        
        fwrite(attach, sizeof(char), bytes, attachedFile);
        printf("finished writing\n");
    }
    
    fclose(attachedFile);
    return true;
}

bool getAttachmentData(char* user, char* filename, char* data_out, int lenght_out){
    
    char path[50];
    memset(path, '\0', sizeof(char)*50);
    getUserpath(user, path);
    strcat(path, "Attachments/");
    strcat(path, filename);
    
    
    FILE *fp = fopen(path,"r");
    if( fp == NULL )
    {
        perror("Error while opening the file.\n");
        perror(path);
        return false;
    }
    lenght_out = fread(data_out,MAXFILESIZE, 1, fp);
    
    fclose(fp);
    return true;

}


void listMessages(char* username, vector<char*>* subjects)
{
    vector<char*> entries;

    char path[30];   // array to hold the result.
    getUserpath(username, path);


    if(isDirectoryPresent(path)){
        getDirList(path, &entries);

        for (vector<char*>::iterator it = entries.begin(); it != entries.end(); it++)
        {
            cout << "path:" << path << endl << "it:" << *it << endl;
            
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
        reverse(subjects->begin(), subjects->end());
    }
}

bool getMailSubject(char* path, char* subject_out){
    char line[81];
    memset(line, '\0', sizeof(char)*81);

    FILE *fp = fopen(path,"r");
    if( fp == NULL )
    {
        perror("Error while opening the file.\n");
        perror(path);
        return false;
    }

    int i = 0;
    while(fgets(line,80, fp)!=NULL){
        if(i == 1)
        {
            strcpy(subject_out, strtok (line,"\n"));// \n wegsplitten
            cout << "dev subj: " << subject_out << endl << "dev line" << line << endl;
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
        if(i == 0)
        {
            strcpy(msg_out, strtok (line,"\n"));// \n wegsplitten
            strcat(msg_out, "\n");
            
            strcat(msg_out, username);// \n wegsplitten
            strcat(msg_out, "\n");
        }
        else if (i > 0)
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
    getUserpath(username, basepath);
    strcat(basepath, "mail");
    
    strcpy(path, basepath);
    
    sprintf(mailNr,"%d",fileNr);
    strcat(path, mailNr);
    
    if( remove(path) != 0 )
    {
        cout << "Could not delete " << path << endl;
        return false;
    }
    
    cout << "Deleted " << path << endl;
    int nr = fileNr+1;
    
    while(true){//auf isfilepresent ändern!!!
        
        char tmp_filename[30];
        char tmp_Nr[4];
        
        
        char tmp_filename2[30];
        char tmp_Nr2[4];
        
        strcpy(tmp_filename, basepath);//data/User/mail  //alter name
        sprintf(tmp_Nr,"%d",nr);
        strcat(tmp_filename, tmp_Nr);//data/User/mail12
        
        
        strcpy(tmp_filename2, basepath);//data/User/mail  //neuer name
        sprintf(tmp_Nr2,"%d",nr-1);
        strcat(tmp_filename2, tmp_Nr2);//data/User/mail12
        
        int check = rename(tmp_filename, tmp_filename2);
        if (check == 0)
        {
            cout << "Renamed " << tmp_filename << " to " << tmp_filename2 << endl;
        }
        else
        {
            break;
        }
        
    }
    return true;
}


void getUserpath(char* username, char* path_out)
{
    strcpy(path_out,mailspool); // copy string one into the result.
    strcat(path_out,"/");
    strcat(path_out,username); // append string two to the result.
    strcat(path_out,"/");
}

bool isDirectoryPresent(const char* dir){
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

void createDirectory(const char* dir){
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
                char* tmp = (char*)calloc(1024, sizeof(char));
                
                strcpy(tmp, dirEntry->d_name);
                entries->push_back(tmp);
                cout << "getDirList entry: " << path << dirEntry->d_name << endl;
            }
        }
        closedir(dirHandle);
    }
}


void saveBans(){
    char path[30] = "./bans";

    
    
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

}

bool fileExists(const char* file) {
    struct stat buf;
    return (stat(file, &buf) == 0);
}
