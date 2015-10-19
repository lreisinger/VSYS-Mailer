//
//  fileOperations.h
//  TWMailer
//
//  Created by Lukas Reisinger on 19.10.15.
//  Copyright (c) 2015 Lukas Reisinger. All rights reserved.
//

#ifndef __TWMailer__fileOperations__
#define __TWMailer__fileOperations__

#include <stdio.h>
#include <dirent.h>
#include <vector>

#endif /* defined(__TWMailer__fileOperations__) */



bool saveMessage(char* empfaenger, char* sender, char* betreff, char* nachricht);
void createDirectory(char* dir);
bool isDirectoryPresent(char* dir);
bool fileExists(const char* file);
void getNextFileNamePath(const char* path, char* filenamepath);
void listMessages(char* username, std::vector<char*>* subjects);
void getDirList(char* path, std::vector<char*>* entries);
void getUserpath(char* username, char* path_out);
bool getMailSubject(char* path, char* subject_out);

bool getMailMessage(char* username, int fileNr);