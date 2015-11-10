//
//  client.cpp
//  TWMailer
//
//  Created by Lukas Reisinger and Dominik Hofmann.
//  Copyright (c) 2015 Lukas Reisinger and Dominik Hofmann. All rights reserved.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>

#include <fstream>

#define BUF 1024

int handleInput(char* input);
int sendMail(int conSocket);
int listMail(int conSocket);
int readMail(int conSocket);
int delMail(int conSocket);
int login(int conSocket);
int logout();
int sendAttachment(int conSocket, char *filePath);
int downloadfile(int conSocket);
char* recvFile(int fileBytes, int sd);
bool saveAttachment(char* attach, int bytes, char* filename, char* user);
void createDirectory(const char* dir);
bool isDirectoryPresent(const char* dir);
long get_size (FILE *file);

bool loggedIn;
char userLoggedIn[9];

using namespace std;

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
            case 6: logout(); break;
            case 7: downloadfile(create_socket); break;
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
    else if (strcasecmp(input,"LOGOUT\n")==0)
        return 6;
    else if (strcasecmp(input,"DOWNLOAD\n")==0)
        return 7;
    else
        return 0;
}

int sendMail(int conSocket) {
    if (!loggedIn) {
        printf("You must be logged in to use that command!\n");
        return 1;
    }
    char /*from[9],*/ to[80], subject[81], message[901], msgBuffer[801], tmp[50], file[51], file2[51], tmp2[50];
    bool fileAttached=false;
    long fileSize=0;

    memset(&to, '\0', sizeof(to));
    memset(&subject, '\0', sizeof(subject));
    memset(&message, '\0', sizeof(message));

    /* Veraltet
    printf("From: ");
    fgets(from, 9, stdin);
    //printf("%d\n",strlen(from));
    if (strlen(from)>=8) {
        fgets(tmp, 50, stdin);
    }
    */

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
        fgets(msgBuffer, 800, stdin);
        if (strcmp(msgBuffer,".\n")==0)
            break;
        strcat(message,msgBuffer);
        //printf("MEssagebuf: %s\n", msgBuffer);
        //printf("MEssage: %s\n", message);

    }

    //printf("MEssageALL: %s\n", message);
    //Protokoll bauen

    memset(&tmp, '\0', 50);
    printf("Attach file? (y/n): ");
    fgets(tmp, 3, stdin);
    if (strcasecmp(strtok(tmp,"\n"),"y")==0) {
        //printf("File attaching");
        fileAttached=true;
        printf("Path: ");
        fgets(file, 50, stdin);

        printf("File1: %s\n", file);

        //Filename holen
        //printf("strcpy\n");
        char *ptr;
        strcpy(file2, file);
        ptr=strtok(file2,"/");
        //printf("%s\n",tmp);
        while (ptr!=NULL) {
            strcpy(tmp,ptr);
            //printf("do\n");
            strcpy(tmp2,tmp);
            ptr=strtok(NULL, "/");

            //printf("%s\n",tmp);
        }
        //printf("end do\n");

        memset(&tmp, '\0', 50);


        printf("File: %s\n",file);
        FILE* fd = fopen(strtok(file, "\n"), "r");
        if (fd==NULL) {printf("Error: %d (%s)\n", errno, strerror(errno)); return 0;}

        fileSize=get_size(fd);

        if (fileSize == -1) {
            printf("Could not find file %s, sending message without attachment...\n", file);
            return 0;
        }
        else {
            printf("File found. File size is: %ld\n", fileSize);
        }

        sprintf(tmp,"%ld",fileSize);

    }

    printf("Bool: %d\n",fileAttached);

    char buffer[BUF]={'\0'};

    strcpy(buffer, "SEND\n");
    strcat(buffer, strtok(userLoggedIn,"\n"));
    strcat(buffer, "\n");
    strcat(buffer, strtok(to,"\n"));
    strcat(buffer, "\n");
    strcat(buffer, subject);
    //strcat(buffer, "\n");
    strcat(buffer, message);
    strcat(buffer, ".\n");
    if (fileAttached) {
        printf("File anhängen\n");
        strcat(buffer, tmp);
        strcat(buffer, " ");
        strcat(buffer, strtok(tmp2,"\n"));
    }


    send(conSocket, buffer, strlen(buffer), 0);

    memset(&buffer, '\0', strlen(buffer));

    recv(conSocket,buffer,BUF-1, 0);

    if (fileAttached) {
        if (strcasecmp(buffer,"OK\n")==0) {
            sendAttachment(conSocket, file);

            memset(&buffer, '\0', sizeof(buffer));

            recv(conSocket,buffer,BUF-1, 0);
        }
        else {
            printf("Your message could not be sent!\n");
            return 0;
        }
    }

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
    if (!loggedIn) {
        printf("You must be logged in to use that command!\n");
        return 1;
    }
    char buffer[1024]/*, user[9], temp[50]*/;

    /* Veraltet
    printf("Username: ");
    fgets(user, 9, stdin);
    if (strlen(user)>=8) {
        fgets(temp, 50, stdin);
    }
    */

    strcpy(buffer, "LIST\n");
    strcat(buffer, strtok(userLoggedIn,"\n"));
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
    if (!loggedIn) {
        printf("You must be logged in to use that command!\n");
        return 1;
    }
    char buffer[1024], /*user[10], temp[50],*/ number[4];

    /* Veraltet
    printf("Username: ");
    fgets(user, 10, stdin);
    */

    printf("Message-number: ");
    fgets(number, 4, stdin);

    strcpy(buffer, "READ\n");
    strcat(buffer, strtok(userLoggedIn,"\n"));
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
    if (!loggedIn) {
        printf("You must be logged in to use that command!\n");
        return 1;
    }
    char buffer[1024], /*user[10],, tmp[50]*/ number[4];

    /* Veraltet
    printf("Username: ");
    fgets(user, 10, stdin);
    */

    printf("Message-number: ");
    fgets(number, 4, stdin);

    strcpy(buffer, "DEL\n");
    strcat(buffer, strtok(userLoggedIn,"\n"));
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

int sendAttachment(int conSocket, char *filePath) {

    FILE* fd = fopen (filePath, "r");

    if (fd == NULL) {
        printf("Error while opening\n");
        send(conSocket, "ERR\n", sizeof("ERR\n"), 0);
        return 0;
    }
    else {
        char buffer[BUF];
        while (1) {
            int bytesRead = fread(buffer, 1, BUF, fd);

            printf("read %d bytes.\n", bytesRead);

            if (bytesRead == 0)
                break;

            if (bytesRead < 0) {
                printf("error: %s\n", strerror(errno));
                return 0;
            }


            char *p = buffer;
            while (bytesRead > 0) {
                int bytesSent = write(conSocket, p, bytesRead);
                if (bytesSent <= 0) {
                    printf("error: %s\n", strerror(errno));
                    return 0;
                }
                bytesRead -= bytesSent;
                p+= bytesSent;
            }

        }
    }
    fclose(fd);
    return 1;
}

int downloadfile(int conSocket) {
    char fName[20], buffer[BUF];
    printf ("Filename: ");
    fgets(fName, 19, stdin);

    strcpy(buffer, "DOWNLOAD\n");
    strcat(buffer, userLoggedIn);
    strcat(buffer, "\n");
    strcat(buffer, strtok(fName, "\n"));

    send(conSocket, buffer, strlen(buffer), 0);

    memset(&buffer, '\0', BUF);

    recv(conSocket, buffer, BUF-1, 0);
    if (strcasecmp(buffer, "OK\n")!=0)
        return 0;
    char *fileName=strtok(buffer, " ");
    int fileSize=atoi(strtok(NULL, " "));
    send(conSocket, "OK\n", BUF-1, 0);
    char *fileBytes=recvFile(fileSize, conSocket);

    if (saveAttachment(fileBytes, fileSize, fileName, userLoggedIn))
        return 1;
    return 0;
}

char* recvFile(int fileBytes, int sd) {
    char* file = (char*) calloc(BUF, sizeof(char));

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

bool saveAttachment(char* attach, int bytes, char* filename, char* user) {

    char path[50];
    memset(path, '\0', sizeof(char)*50);

    createDirectory("recieved");

    strcpy(path, "./recieved/");
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

void createDirectory(const char* dir) {
    if(!isDirectoryPresent(dir))
    {
        mkdir(dir, 0700);
    }
}

bool isDirectoryPresent(const char* dir) {
    struct stat st = {0};

    if (stat(dir, &st) == -1) {
        cout << "dir is NOT present; path=" << dir << endl;
        return false;
    }
    cout << "dir is present; path=" << dir << endl;
    return true;
}

long get_size (FILE *file) {
    long pos=0;
    long size=0;
    pos=ftell(file);
    rewind(file);
    while (fgetc(file)!=EOF)
        size++;
    fseek (file,pos,SEEK_SET);
    return size;
}
