//
//  main.cpp
//  TWMailer
//
//  Created by Lukas Reisinger on 08.10.15.
//  Copyright (c) 2015 Lukas Reisinger. All rights reserved.
//

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

using namespace std;

int main(int argc, const char * argv[]) {
    int queue = 1;//max 1 in queue
    int socket_fd;
    int conn_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in client_addr;
    
    int maxlen = 1024;
    char buffer[maxlen];
    
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&my_addr, 0, sizeof(my_addr));
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(6002);
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
            
            //strcpy(buffer,"Hallo");
            //send(conn_fd, buffer, strlen(buffer),0);
        }
        
        
        while(true)
        {
            
            ssize_t recv_len = recv(conn_fd, &buffer, maxlen-1, 0);
            buffer[maxlen-1] = '\0';
            if(recv_len == -1){
                perror("recv error");
            }
            else if (recv_len == 0)
            {
                close(conn_fd);
                close (socket_fd);
                exit(1);
            }
            else
            {
                cout << buffer << endl;
            }
            
        }
        
    }
    
    return 0;
}