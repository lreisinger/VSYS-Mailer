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


#include <iostream>
#include <fstream>

#include <pthread.h>

#include "fileOperations.h"

#define PORT 6010
#define BUF 1024

using namespace std;


struct command {
    char cmd[5];
    //SEND
    char sender[9];
    char empfaenger[9];
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

typedef struct
{
    int fd;
    struct sockaddr_in* client_addr;
} ThreadData;


void* handleClient(void* arg);

ssize_t sendmsg(int sd, char* text);
command parseReceived(char* msg);
bool handleCommand(command* cmd, int sd);


bool handleLogin(command* cmd, int sd);
bool handleSend(command* cmd, int sd);
bool handleList(command* cmd, int sd);
bool handleRead(command* cmd, int sd);
bool handleDel(command* cmd, int sd);

//replies:
int sendReplySuccess(bool success, int sd);
int sendReplyText(char* text, int sd);

int main(int argc, const char * argv[]) {

    if( argc < 2 ){
        printf("Usage: %s Port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int queue = 1;//max 1 in queue
    int socket_fd;
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
    ThreadData* data = (ThreadData*)arg;
    int fd = data->fd;
    
    free(data->client_addr);
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
            command recv_cmd = parseReceived(buffer);
            handleCommand(&recv_cmd, fd);
        }
        
    }
    
    return NULL;
}


command parseReceived(char* msg){
    command new_cmd = {};
    new_cmd.valid = true;

    char fields[4][900];//fields except cmd (SEND, LIST,..)
    memset(&fields, 0, sizeof(char)*900*4);


    int i = 0;
    cout << endl << "HANDLE:" << endl;


    bool finished = false;
    char* tmp = strtok (msg,"\n");
    strcpy(new_cmd.cmd, tmp);


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
    else if(strcasecmp(new_cmd.cmd, "LOGIN") == 0){
        if(i>0){
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


bool handleCommand(command* cmd, int sd){
    if(cmd->valid){
        bool loggedIn = false;
        if(loggedIn)
        {
            if(strcasecmp(cmd->cmd, "LOGIN") == 0){
                return handleLogin(cmd, sd);
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
            else if(strcasecmp(cmd->cmd, "DEL") == 0){
                return handleDel(cmd, sd);
            }
            else if(strcasecmp(cmd->cmd, "LOGIN") == 0){
                return handleLogin(cmd, sd);
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
    //bool success = saveMessage(cmd->empfaenger, cmd->sender, cmd->betreff, cmd->message);
    if(strcmp(cmd->username, "lukas3") == 0){
        sendReplySuccess(false, sd);
        return false;
    }
    else
    {
        sendReplySuccess(true, sd);
        return true;
    }
    //return success;
}

bool handleList(command* cmd, int sd){
    vector<char*> subjects;
    char subj_count[4];
    memset(subj_count, '\0', sizeof(char)*4);
    char reply[BUF];
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
    char reply[BUF];
    memset(reply, '\0', sizeof(char)*BUF);
    char msg[BUF];
    memset(msg, '\0', sizeof(char)*BUF);

    bool success = getMailMessage(cmd->username, cmd->msgNr, msg);
    
    cout << "handleREAD checkMSG: \n" << msg << endl;

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
