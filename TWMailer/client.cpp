#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define BUF 1024
#define PORT 6001

int handleInput(char* input);
int sendMail(int conSocket);
int listMail(int conSocket);

int main (int argc, char **argv) {
    int create_socket;
    char buffer[BUF];
    struct sockaddr_in address;
    int size;

    if( argc < 2 ){
        printf("Usage: %s ServerAdresse\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((create_socket = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket error");
        return EXIT_FAILURE;
    }

    memset(&address,0,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons (PORT);
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
        printf("%d\n",handleInput(buffer));
        switch (handleInput(buffer)) {
            case -1: printf("Quitting Client...\n");break;
            case 1: sendMail(create_socket); break;
            case 2: listMail(create_socket); break;
            case 3: break; //READ
            case 4: break; //DEL
            default: printf("Unknown Command");
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
    else
        return 0;
}

int sendMail(int conSocket) {
    char from[9], to[9], subject[81], message[901], msgBuffer[901];

    printf("From: ");
    fgets(from, 9, stdin);
    getc(stdin);

    printf("To: ");
    fgets(to, 9, stdin);
    getc(stdin);

    printf("Subject: ");
    fgets(subject, 81, stdin);

    printf("Message: ");
    while (1) {
        fgets(msgBuffer, 901, stdin);
        if (strcmp(msgBuffer,".\n")==0)
            break;
        strcat(message,msgBuffer);
    }

    //Protokoll bauen

    char buffer[BUF];
    strcat(buffer, "SEND\n");
    strcat(buffer, from);
    strcat(buffer, "\n");
    strcat(buffer, to);
    strcat(buffer, "\n");
    strcat(buffer, subject);
    //strcat(buffer, "\n");
    strcat(buffer, message);
    strcat(buffer, ".\n");

    send(conSocket, buffer, strlen (buffer), 0);

    recv(conSocket,buffer,BUF-1, 0);

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
    char buffer[1024];
    strcpy(buffer, "LIST\n");
    send(conSocket, buffer, strlen (buffer), 0);

    int size;
    size=recv(conSocket,buffer,BUF-1, 0);
    if (size>0) {
        buffer[size]= '\0';
        printf("%s",buffer);
    }
    return 0;
}

int readMail(int conSocket) {
    char buffer[1024], user[9], number[4];

    printf("Username: ");
    fgets(user, 9, stdin);
    getc(stdin);

    printf("Message-number: ");
    fgets(number, strlen(number), stdin);

    strcpy(buffer, "READ\n");
    strcat(buffer, user);
    strcat(buffer, "\n");
    strcat(buffer, number);
    strcat(buffer, "\n");

    send(conSocket, buffer, strlen (buffer), 0);

    recv(conSocket,buffer,BUF-1, 0);

    char* tmp=strtok(buffer, "\n");
    if (strcasecmp(tmp,"ok")!=0) {
        printf("Error\n");
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
