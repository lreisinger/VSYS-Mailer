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
#define PORT 6001
#define BUF 1024

using namespace std;

ssize_t sendmsg(int sd, char* text);
void parseReceived(char* msg);

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
            
            strcpy(buffer, "Welcome Client!");//welcome nachricht
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
                parseReceived(buffer);
            }

        }

    }

    return 0;
}

void parseReceived(char* msg){
    char cmd[5];
    char sender[9];
    char empfaenger[9];
    char betreff[81];
    char message[918];
    
    
    memset(&cmd, '\0', sizeof(cmd));
    memset(&sender, '\0', sizeof(sender));
    memset(&empfaenger, '\0', sizeof(empfaenger));
    memset(&betreff, '\0', sizeof(betreff));
    memset(&message, '\0', sizeof(message));
    
    
    

    
    int i = 0;
    cout << endl << "HANDLE:" << endl;
    
    char* tmp = strtok (msg,"\n");
    bool finished = false;
    
    while (tmp != NULL && !finished)
    {
        cout << tmp << endl;
        switch (i) {
            case 0:
                //cmd
                strcpy(cmd, tmp);
                break;
            case 1:
                //sender
                strcpy(sender, tmp);
                break;
            case 2:
                //empfänger
                strcpy(empfaenger, tmp);
                break;
            case 3:
                //empfänger
                strcpy(betreff, tmp);
                break;
            case 4:
                //Nachricht bis Zeile mit (nur) .
                strcpy(message, tmp);
                break;
            default:
                if(strcmp(tmp, ".") == 0){
                    cout << "EOM" << endl;
                    finished = true;
                    break;
                }
                //Nachricht (Zeile 2+)
                strcat(message, "\n");
                strcat(message, tmp);
                break;
                
        }
        i++;
        
        tmp = strtok (NULL, "\n");
    }
    
}



ssize_t sendmsg(int sd, char* text){
    char sendbuffer[BUF];
    ssize_t tmp = send(sd, text, strlen(text),0);
    memset(text, '\0', sizeof(char)*BUF);
    return tmp;
}
