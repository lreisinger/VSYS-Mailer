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
#include <pthread.h>

#define BANMINUTES 30


pthread_mutex_t lockvar=PTHREAD_MUTEX_INITIALIZER;


extern const char* mailspool;

struct user_ldap {
    int sd;
    char username[9];
    int retries;
    char ip[16];
    int timestamp_lasttry;
} ;

extern std::vector<struct user_ldap*> wrong_logins;

bool loadBans();
bool saveBans();

bool timediffBiggerThan(long diffSeconds, int minimumMintes);

bool getAttachmentData(char* user, char* filename, char* data_out, int* lenght_out);
bool saveAttachment(char* attach, int bytes, char* filename, char* user);

bool saveMessage(char* empfaenger, char* sender, char* betreff, char* nachricht);
void createDirectory(const char* dir);
bool isDirectoryPresent(const char* dir);
bool fileExists(const char* file);
int getFileSize(FILE* pFile);
void getNextFileNamePath(const char* path, char* filenamepath);
void listMessages(char* username, std::vector<char*>* subjects);
void getDirList(char* path, std::vector<char*>* entries);
void getUserpath(char* username, char* path_out);
bool getMailSubject(char* path, char* subject_out);

bool getMailMessage(char* username, int fileNr, char* msg_out);
bool deleteMail(char* username, int fileNr);


#endif /* defined(__TWMailer__fileOperations__) */
