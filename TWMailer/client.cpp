//
//  client.cpp
//  TWMailer
//
//  Created by Lukas Reisinger and Dominik Hofmann.
//  Copyright (c) 2015 Lukas Reisinger and Dominik Hofmann. All rights reserved.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define BUF 1024

int handleInput(char* input);
int sendMail(int conSocket);
int listMail(int conSocket);
int readMail(int conSocket);
int delMail(int conSocket);
int login(int conSocket);
int logout();

bool loggedIn;
char userLoggedIn[9];

int main (int argc, char **argv) {
    int create_socket;
    loggedIn=false;
    //memset(&userLoggedIn, '\0', strlen(userLoggedIn));
    char buffer[BUF];
    struct sockaddr_in address;
    int size;

    if( argc < 3 ){
        printf("Usage: %s ServerAdresse Port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((create_socket = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket error");
        return EXIT_FAILURE;
    }

    memset(&address,0,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons (atoi(argv[2]));
    inet_aton (argv[1], &address.sin_addr);

    if (connect ( create_socket, (struct sockaddr *) &address, sizeof (address)) == 0) {
        printf ("Connection with server (%s) established\n", inet_ntoa (address.sin_addr));
        size=recv(create_socket,buffer,BUF-1, 0);
            if (size>0) {
                buffer[size]= '\0';
                printf("%s",buffer);
            }
        }
    else {
        perror("Connect error - no server available");
        return EXIT_FAILURE;
    }

    do {
        printf ("Send message: ");
        fgets (buffer, BUF, stdin);
        switch (handleInput(buffer)) {
            case -1: printf("Quitting Client...\n");break;
            case 1: sendMail(create_socket); break;
            case 2: listMail(create_socket); break;
            case 3: readMail(create_socket); break;
            case 4: delMail(create_socket); break;
            case 5: login(create_socket); break;
            default: printf("Unknown Command\n");
        }
    }
    while (strcasecmp(buffer, "quit\n") != 0);
    close (create_socket);
    return EXIT_SUCCESS;
}

int handleInput(char* input) {
    if (strcasecmp(input,"QUIT\n")==0)
        return -1;
    else if (strcasecmp(input,"SEND\n")==0)
        return 1;
    else if (strcasecmp(input,"LIST\n")==0)
        return 2;
    else if (strcasecmp(input,"READ\n")==0)
        return 3;
    else if (strcasecmp(input,"DEL\n")==0)
        return 4;
    else if (strcasecmp(input,"LOGIN\n")==0)
        return 5;
    else if (strcasecmp(input,"LOGIN\n")==0)
        return 6;
    else
        return 0;
}

int sendMail(int conSocket) {
    char from[9], to[9], subject[81], message[901], msgBuffer[901], tmp[50];
    memset(from,0,sizeof(char)*9);
    memset(to,0,sizeof(char)*9);
    memset(subject,0,sizeof(char)*81);
    memset(message,0,sizeof(char)*901);
    memset(msgBuffer,0,sizeof(char)*901);
    memset(tmp,0,sizeof(char)*50);


    printf("From: ");
    fgets(from, 9, stdin);
    //printf("%d\n",strlen(from));
    if (strlen(from)>=8) {
        fgets(tmp, 50, stdin);
    }

    printf("To: ");
    fgets(to, 9, stdin);
    //printf("%d\n",strlen(to));
    if (strlen(to)>=8) {
        fgets(tmp, 50, stdin);
    }

    printf("Subject: ");
    fgets(subject, 81, stdin);

    printf("Message: ");
    while (1) {
        fgets(msgBuffer, 901, stdin);
        if (strcmp(msgBuffer,".\n")==0)
            break;
        strcat(message,msgBuffer);
        //printf("MEssagebuf: %s\n", msgBuffer);
        //printf("MEssage: %s\n", message);

    }

    //printf("MEssageALL: %s\n", message);
    //Protokoll bauen

    char buffer[BUF]={'\0'};

    strcpy(buffer, "SEND\n");
    strcat(buffer, strtok(from,"\n"));
    strcat(buffer, "\n");
    strcat(buffer, strtok(to,"\n"));
    strcat(buffer, "\n");
    strcat(buffer, subject);
    //strcat(buffer, "\n");
    strcat(buffer, message);
    strcat(buffer, ".\n");

    send(conSocket, buffer, strlen(buffer), 0);

    memset(&buffer, '\0', strlen(buffer));

    recv(conSocket,buffer,BUF-1, 0);

    printf("Recieved: %s",buffer);

    if (strcasecmp(buffer,"OK\n")==0) {
        printf("Your message was sent!\n");
        return 1;
    }
    else {
        printf("Your message could not be sent!\n");
        return 0;
    }
}

int listMail(int conSocket) {
    char buffer[1024], user[9], temp[50];

    printf("Username: ");
    fgets(user, 9, stdin);
    if (strlen(user)>=8) {
        fgets(temp, 50, stdin);
    }

    strcpy(buffer, "LIST\n");
    strcat(buffer, strtok(user,"\n"));
    strcat(buffer, "\n");
    send(conSocket, buffer, strlen (buffer), 0);

    memset(&buffer, '\0', strlen(buffer));

    recv(conSocket,buffer,BUF-1, 0);

    char* tmp=strtok(buffer,"\n");
    int anz=strtol(tmp,NULL,10);
    printf("Anzahl der Nachrichten: %d\n", anz);

    for (int i=0; i<anz; i++) {
        memset(&tmp, '\0', strlen(tmp));
        tmp=strtok(NULL,"\n");
        printf("%d: %s\n",i,tmp);
    }

    return 0;
}

int readMail(int conSocket) {
    char buffer[1024], user[10], number[4], temp[50];

    printf("Username: ");
    fgets(user, 10, stdin);

    printf("Message-number: ");
    fgets(number, 4, stdin);

    strcpy(buffer, "READ\n");
    strcat(buffer, strtok(user,"\n"));
    strcat(buffer, "\n");
    strcat(buffer, number);
    strcat(buffer, "\n");

    send(conSocket, buffer, strlen (buffer), 0);

    memset(&buffer, '\0', BUF);

    recv(conSocket,buffer,BUF-1, 0);

    char* tmp=strtok(buffer, "\n");
    if (strcasecmp(tmp,"ok")!=0) {
        printf("Error\n");
        return 1;
    }
    else {
        tmp=strtok(NULL, "\n");
        printf("From: %s\n",tmp);
        tmp=strtok(NULL, "\n");
        printf("To: %s\n",tmp);
        tmp=strtok(NULL, "\n");
        printf("Subject: %s\n",tmp);
        tmp=strtok(NULL, "\0");
        printf("Message: %s\n", tmp);
    }

    return 0;
}

int delMail(int conSocket) {
    char buffer[1024], user[10], number[4], tmp[50];

    printf("Username: ");
    fgets(user, 10, stdin);

    printf("Message-number: ");
    fgets(number, 4, stdin);

    strcpy(buffer, "DEL\n");
    strcat(buffer, strtok(user,"\n"));
    strcat(buffer, "\n");
    strcat(buffer, strtok(number, "\n"));
    strcat(buffer, "\n");

    send(conSocket, buffer, strlen (buffer), 0);

    memset(&buffer, '\0', strlen(buffer));

    recv(conSocket,buffer,BUF-1, 0);

    if (strcasecmp(buffer, "OK\n")==0) {
        printf("Message deleted successfully!\n");
        return 0;
    }
    else {
        printf("Message could not be deleted!\n");
        return 1;
    }

}

int login(int conSocket) {
    char user[10], pass[50];

    printf("Username: ");
    fgets(user, 10, stdin);

    //printf("Password: ");
    strcpy(pass,getpass("Password: "));

    char buffer[BUF];

    //Protokoll bauen
    strcpy(buffer,"LOGIN\n");
    strcat(buffer,strtok(user,"\n"));
    strcat(buffer, "\n");
    strcat(buffer, strtok(pass,"\n"));

    send(conSocket, buffer, strlen(buffer), 0);

    memset(&buffer, '\0', BUF);

    recv(conSocket,buffer,BUF-1, 0);

    if (strcasecmp(buffer,"OK\n")==0) {
        loggedIn=true;
        strcpy(userLoggedIn, user);
        printf("Login successful!\n");
        return 1;
    }
    else {
        loggedIn=false;
        memset(&userLoggedIn, '\0', strlen(userLoggedIn));
        printf("Login failed!\n");
        return 0;
    }

}

int logout() {
    loggedIn=false;
    memset(&userLoggedIn, '\0', strlen(userLoggedIn));
    printf("Logged out!\n");
    return 0;
}
