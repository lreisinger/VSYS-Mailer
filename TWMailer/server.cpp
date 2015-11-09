//
//  main.cpp
//  TWMailer
//
//  Created by Lukas Reisinger and Dominik Hofmann.
//  Copyright (c) 2015 Lukas Reisinger and Dominik Hofmann. All rights reserved.
//

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>


#include <iostream>
#include <fstream>

#include <pthread.h>

#include "fileOperations.h"
#include "ldap.h"

#define PORT 6010
#define BUF 1024

#define MAXCLIENTS 100
#define MAXRECEIVERS 100
#define MAXFILESIZE 1024

using namespace std;


const char* mailspool;

struct command {
    char cmd[5];
    //SEND
    char sender[9];
    char empfaenger[9*MAXRECEIVERS];//aaaaaaa,bbbbbbb,ccccccc\n
    char betreff[81];
    char message[918];
    //LIST
    char username[9];
    char password[50];
    //READ/DEL
    //username
    int msgNr;
    bool valid;
} ;

struct ThreadData
{
    int fd;
    struct sockaddr_in* client_addr;
    ~ThreadData() {
        free(client_addr);
    }
};

std::vector<struct user_ldap*>* loggedIn;

char* getIPfromSd(int sd);

void* handleClient(void* arg);

char* recvFile(int fileBytes, int sd);

ssize_t sendmsg(int sd, char* text);
command parseReceived(char* msg, int sd);
bool handleCommand(command* cmd, int sd, bool loggedIn);


bool handleLogin(command* cmd, int sd);
bool handleLogout(command* cmd, int sd);
bool handleSend(command* cmd, int sd);
bool handleList(command* cmd, int sd);
bool handleRead(command* cmd, int sd);
bool handleDownload(command* cmd, int sd);
bool handleDel(command* cmd, int sd);

//replies:
int sendReplySuccess(bool success, int sd);
int sendReplyText(char* text, int sd);

void sighandler(sig_t s);
void exit_server();


int socket_fd;


void sighandler(int s){
    exit_server();
}



int main(int argc, const char * argv[]) {
    struct sigaction sigIntHandler;
    
    sigIntHandler.sa_handler = sighandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    
    sigaction(SIGINT, &sigIntHandler, NULL);
    
    //signal END (ctrl+c)

    if( argc < 2 ){
        printf("Usage: %s Port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    mailspool = argv[2];
    
    int queue = 1;//max 1 in queue
    int conn_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in client_addr;


    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&my_addr, 0, sizeof(my_addr));

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(atoi(argv[1]));
    my_addr.sin_addr.s_addr = htons(INADDR_ANY);
    

    if( ::bind(socket_fd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1){
        perror("bind error");
        return EXIT_FAILURE;
    }

    if(listen(socket_fd, queue) == -1)
    {
        perror("listen error");
        exit(1);
    }

    socklen_t client_addrlen = sizeof(client_addr);

    while (true) {
        cout << "Waiting for Connection..." << endl;
        conn_fd = accept (socket_fd, (sockaddr *) &client_addr, &client_addrlen);
        
        cout << "Client: " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << endl;
        
        struct sockaddr_in* client_arg = (struct sockaddr_in*) malloc(sizeof(sockaddr_in));
        *client_arg = client_addr;
        
        //arguments
        ThreadData* td = (ThreadData*) malloc(sizeof(ThreadData));
        td->fd = conn_fd;
        td->client_addr = client_arg;
        
        pthread_t tid;
        pthread_create(&tid, NULL, handleClient, td);
    }

    return 0;
}

void* handleClient(void* arg)
{
    bool loggedIn = false;
    ThreadData* data = (ThreadData*)arg;
    int fd = data->fd;
    
    
    free(data);
    
    char buffer[BUF];
    
    if (fd > 0)
    {
        
        strcpy(buffer, "Welcome Client!\n");//welcome nachricht
        send(fd, buffer, strlen(buffer),0);
        memset(&buffer, '\0', sizeof(buffer));
        
    }
    
    
    while(true)//immer receiven
    {
        memset(&buffer, '\0', sizeof(buffer));
        ssize_t recv_len = recv(fd, &buffer, BUF-1, 0);
        buffer[BUF-1] = '\0';
        if(recv_len == -1){
            perror("recv error");
        }
        else if (recv_len == 0)//close by client
        {
            cout << "Connection closed by Client" << endl << "Still listening for Connections..." << endl;
            close(fd);
            break;
        }
        else
        {
            cout << "RAW: " << endl << buffer << endl;
            command recv_cmd = parseReceived(buffer, fd);
            handleCommand(&recv_cmd, fd, loggedIn);
        }
        
    }
    
    return NULL;
}


command parseReceived(char* msg, int sd){
    command new_cmd = {};
    new_cmd.valid = true;

    char fields[4][900];//fields except cmd (SEND, LIST,..)
    memset(&fields, 0, sizeof(char)*900*4);


    int i = 0;

    char filename[50];
    int filesize;
    
    bool finished = false;
    char* tmp = strtok (msg,"\n");
    strcpy(new_cmd.cmd, tmp);
    cout << endl << "HANDLE " << new_cmd.cmd << ":" << endl;


    tmp = strtok (NULL, "\n");
    while (tmp != NULL && !finished)
    {
        cout << tmp << endl;

        if(strcasecmp(new_cmd.cmd, "SEND") == 0){//sonderfall Message bei SEND

            if(i > 3)//message-block
            {
                if(strcmp(tmp, ".") == 0){
                    finished = true;
                }
                else
                {
                    strcat(fields[3], "\n");
                    strcat(fields[3], tmp);
                }
            }
            else
            {
                strcpy(fields[i], tmp);//jedes feld von SEND
            }
        }
        else
        {
            strcpy(fields[i], tmp);//jedes Feld von andere als SEND
        }

        i++;
        tmp = strtok (NULL, "\n");
        
    }
    

    if(strcasecmp(new_cmd.cmd, "SEND") == 0){
        if(i>3){//mind. 4zeilen
            strcpy(new_cmd.sender, fields[0]);
            strcpy(new_cmd.empfaenger, fields[1]);
            strcpy(new_cmd.betreff, fields[2]);
            strcpy(new_cmd.message, fields[3]);
            
            if(finished && tmp != NULL){
                
                char* tmp2 = strtok (tmp," ");//filesize
                filesize = atoi(tmp2);
                
                tmp2 = strtok (NULL," ");//filename
                strcpy(filename, tmp2);
                
                
                sendReplySuccess(true, sd);//damit client dann loschickt
                
                char* filebytes = recvFile(filesize, sd);
                
                bool success = saveAttachment(filebytes, filesize, filename, new_cmd.empfaenger);
                
                if(success){
                    strcat(new_cmd.message, "\n\nAttached file: ");
                    strcat(new_cmd.message, filename);
                }
                sendReplySuccess(success, sd);//??
                
                
            }

        }
        else
        {
            new_cmd.valid = false;
        }
    }
    else if(strcasecmp(new_cmd.cmd, "LIST") == 0){
        if(i>0){
            strcpy(new_cmd.username, fields[0]);
        }
        else
        {
            new_cmd.valid = false;
        }
    }
    else if((strcasecmp(new_cmd.cmd, "READ") == 0) || (strcasecmp(new_cmd.cmd, "DEL") == 0)){
        if(i>1){
            strcpy(new_cmd.username, fields[0]);
            new_cmd.msgNr = stoi(fields[1]);
        }
        else
        {
            new_cmd.valid = false;
        }
    }
    else if(strcasecmp(new_cmd.cmd, "DOWNLOAD") == 0){
        if(i>0){
            strcpy(new_cmd.username, fields[0]);//betreff als filename
            strcpy(new_cmd.betreff, fields[1]);//betreff als filename
        }
        else
        {
            new_cmd.valid = false;
        }
    }
    else if(strcasecmp(new_cmd.cmd, "LOGIN") == 0){
        if(i>1){
            strcpy(new_cmd.username, fields[0]);
            strcpy(new_cmd.password, fields[1]);
        }
        else
        {
            new_cmd.valid = false;
        }
    }
    return new_cmd;
}


bool handleCommand(command* cmd, int sd, bool loggedIn){
    if(cmd->valid){
        if(!loggedIn)
        {
            if(strcasecmp(cmd->cmd, "LOGIN") == 0){
                int success = handleLogin(cmd, sd);
                if(success > 0)
                {
                    loggedIn = true;
                }
                return success;
            }
            else
            {
                return false; //no login
            }
        }
        else//not logged in
        {
            if(strcasecmp(cmd->cmd, "SEND") == 0){
                return handleSend(cmd, sd);
            }
            else if(strcasecmp(cmd->cmd, "LIST") == 0){
                return handleList(cmd, sd);
            }
            else if(strcasecmp(cmd->cmd, "READ") == 0){
                return handleRead(cmd, sd);
            }
            else if(strcasecmp(cmd->cmd, "DOWNLOAD") == 0){
                return handleDownload(cmd, sd);
            }
            else if(strcasecmp(cmd->cmd, "DEL") == 0){
                return handleDel(cmd, sd);
            }
            else if(strcasecmp(cmd->cmd, "LOGOUT") == 0){
                return handleLogout(cmd, sd);
            }
            else
            {
                return false; //wrong cmd
            }
        }
    }
    else
    {
        return false;
    }
}



bool handleSend(command* cmd, int sd){
    bool success = saveMessage(cmd->empfaenger, cmd->sender, cmd->betreff, cmd->message);
    sendReplySuccess(success, sd);
    return success;
}

bool handleLogin(command* cmd, int sd){
    int returnvalue = login(cmd->username, cmd->password);
    bool success = (returnvalue > 0) ? true : false;
    if(success)
    {
        struct user_ldap* newuser = (struct user_ldap*)calloc(1, sizeof(user_ldap));
        strcpy(newuser->username, cmd->username);
        newuser->sd = sd;
        newuser->retries = 0;
        strcpy(newuser->ip, getIPfromSd(sd));
        
        loggedIn->push_back(newuser);
    }
    else if(returnvalue > -2 && returnvalue < 1)//fehlerhafter user oder pw
    {
        //ipban nach 3x
    }
    sendReplySuccess(success, sd);
    return success;
}

char* getIPfromSd(int sd){
    char* ret = (char*) calloc (16, sizeof(char));
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int err = getpeername(sd, (struct sockaddr *) &addr, &addr_len);
    if (err != 0) {
        return ret;
    }
    inet_ntop(AF_INET, &addr.sin_addr, ret, sizeof(char)*16);
    return ret;
}

bool handleLogout(command* cmd, int sd){

    
    return true;
}

bool handleList(command* cmd, int sd){
    vector<char*> subjects;
    char subj_count[4], reply[BUF];
    
    memset(subj_count, '\0', sizeof(char)*4);
    memset(reply, '\0', sizeof(char)*BUF);

    listMessages(cmd->username, &subjects);
    sprintf(subj_count,"%d",(int)subjects.size());
    strcpy(reply, subj_count);//#\n
    strcat(reply, "\n");

    for (vector<char*>::iterator it = subjects.begin(); it != subjects.end(); ++it)
    {
        strcat(reply, *it);//subject\n
        strcat(reply, "\n");
    }
    sendReplyText(reply, sd);

    return true;
}

bool handleRead(command* cmd, int sd){
    char reply[BUF], msg[BUF];
    
    memset(reply, '\0', sizeof(char)*BUF);
    memset(msg, '\0', sizeof(char)*BUF);

    bool success = getMailMessage(cmd->username, cmd->msgNr, msg);
    
    if(success)
    {
        strcpy(reply, "OK\n");
        strcat(reply, msg);
        sendReplyText(reply, sd);
    }
    else
    {
        strcpy(reply, "ERR\n");
        sendReplyText(reply, sd);
    }
    return success;
}



bool handleDownload(command* cmd, int sd){
    char buffer[BUF];
    char file[MAXFILESIZE];
    char reply[BUF];
    int length = 0;
    char length_s[20];
    char* filename = cmd->betreff;
    
    memset(reply, '\0', sizeof(char)*BUF);
    memset(buffer, '\0', sizeof(char)*BUF);
    memset(file, '\0', sizeof(char)*MAXFILESIZE);
    bool success = getAttachmentData(cmd->username, filename, file, length);
    
    strcpy(reply, filename);
    strcat(reply, " ");
    
    sprintf(length_s, "%d", length);
    strcat(reply, length_s);
    
    sendReplyText(reply, sd);
    //send lenght
    recv(sd, &buffer, 80, 0);
    
    if(success)
    {
        sendReplyText(file, sd);
    }
    else
    {
        strcpy(reply, "ERR\n");
        sendReplyText(reply, sd);
    }
    return success;
}


bool handleDel(command* cmd, int sd){

    bool success = deleteMail(cmd->username, cmd->msgNr);

    sendReplySuccess(success, sd);
    return true;
}

int sendReplySuccess(bool success, int sd){
    char buffer[BUF];
    if(success){
        strcpy(buffer, "OK\n");
    }
    else
    {
        strcpy(buffer, "ERR\n");
    }
    cout << "Sent " << buffer << endl;

    return (int)send(sd, buffer, strlen (buffer), 0);
}

int sendReplyText(char* text, int sd){
    char buffer[BUF];
    memset(&buffer, '\0', sizeof(char)*BUF);

    strcpy(buffer, text);
    cout << "Sent " << buffer << endl;

    return (int)send(sd, buffer, strlen (buffer), 0);
}

char* recvFile(int fileBytes, int sd){
    char* file = (char*) calloc(MAXFILESIZE, sizeof(char));
    
    char buffer[1024];
    int remainingBytes = fileBytes;
    
    while (remainingBytes > 0) {
        
        memset(&buffer, '\0', sizeof(buffer));
        ssize_t recv_len = recv(sd, &buffer, 1024-1, 0);
        buffer[1024-1] = '\0';
        
        if(recv_len == -1){
            perror("recv error");
            strcpy(file, "");
            return file;
        }
        else if (recv_len == 0)//close by client
        {
            cout << "Connection closed by Client" << endl << "Still listening for Connections..." << endl;
            close(sd);
            strcpy(file, "");
            return file;
        }
        else
        {
            strcat(file, buffer);
            remainingBytes -= recv_len;
        }
        
    }
    return file;
}


void exit_server()
{
    cout << "Server shut down" << endl;
    //close all sockets
    close(socket_fd);
    exit(1);
}
