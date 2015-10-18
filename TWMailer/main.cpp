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


#include <iostream>
#include <fstream>

#include "fileOperations.h"

#define PORT 6001
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
    char username[8];
    //READ/DEL
    //username
    int msgNr;
    bool valid;
} ;

ssize_t sendmsg(int sd, char* text);
command parseReceived(char* msg);
int handleCommand(command* cmd);

int handleSend(command* cmd);
int handleList(command* cmd);
int handleRead(command* cmd);
int handleDel(command* cmd);

int main(int argc, const char * argv[]) {
    int queue = 1;//max 1 in queue
    int socket_fd;
    int conn_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in client_addr;

    char buffer[BUF];

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&my_addr, 0, sizeof(my_addr));

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
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

        if (conn_fd > 0)
        {
            cout << "Client: " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << endl;
            
            strcpy(buffer, "Welcome Client!\n");//welcome nachricht
            send(conn_fd, buffer, strlen(buffer),0);
            memset(&buffer, '\0', sizeof(buffer));
            
        }


        while(true)//immer receiven
        {
            memset(&buffer, '\0', sizeof(buffer));
            ssize_t recv_len = recv(conn_fd, &buffer, BUF-1, 0);
            buffer[BUF-1] = '\0';
            if(recv_len == -1){
                perror("recv error");
            }
            else if (recv_len == 0)//close by client
            {
                cout << "Connection closed by Client" << endl << "Quitting..." << endl;
                close(conn_fd);
                close (socket_fd);
                exit(1);
            }
            else
            {
                cout << "RAW: " << endl << buffer << endl;
                command recv_cmd = parseReceived(buffer);
                handleCommand(&recv_cmd);
            }

        }

    }

    return 0;
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
    while (tmp != NULL || !finished)
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
        strcpy(new_cmd.sender, fields[0]);
        strcpy(new_cmd.empfaenger, fields[1]);
        strcpy(new_cmd.betreff, fields[2]);
        strcpy(new_cmd.message, fields[3]);
    }
    else if(strcasecmp(new_cmd.cmd, "LIST") == 0){
        strcpy(new_cmd.username, fields[0]);
    }
    else if((strcasecmp(new_cmd.cmd, "READ") == 0) || (strcasecmp(new_cmd.cmd, "DEL") == 0)){
        strcpy(new_cmd.username, fields[0]);
        new_cmd.msgNr = stoi(fields[1]);
    }
    return new_cmd;
}


int handleCommand(command* cmd){
    if(strcasecmp(cmd->cmd, "SEND") == 0){
        return handleSend(cmd);
    }
    else if(strcasecmp(cmd->cmd, "LIST") == 0){
        return handleList(cmd);
    }
    else if(strcasecmp(cmd->cmd, "READ") == 0){
        return handleRead(cmd);
    }
    else if(strcasecmp(cmd->cmd, "DEL") == 0){
        return handleDel(cmd);
    }
    else
    {
        return -1; //wrong cmd
    }
}



int handleSend(command* cmd){
    
    return 1;
}

int handleList(command* cmd){
    
    return 1;
}

int handleRead(command* cmd){
    
    return 1;
}

int handleDel(command* cmd){
    
    return 1;
}









ssize_t sendmsg(int sd, char* text){
    char sendbuffer[BUF];
    ssize_t tmp = send(sd, text, strlen(text),0);
    memset(text, '\0', sizeof(char)*BUF);
    return tmp;
}
