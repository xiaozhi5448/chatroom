//include the header file socket,io,epoll,list,string,unistd,stdlib
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<list>
#include<errno.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>


//namespace
using namespace std;
//client list, C++ stl list<T>
list<int> client_list;
//macro statement server_ip,server_port,epollsize,buf_size,Welcome string,server_string
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888
#define EPOLL_SIZE 500
#define BUF_SIZE 2048
#define WELCOME_STRING "Welcome to join the chat room, your chat ID is: Client# %d"
#define SERVER_STRING "Client id #%d say >> %s"
#define CAUTION "only one client in the chat room"
//set fd in nonblocking io mode, using fnctl
void setNonBlocking(int fd){
    int res = fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_NONBLOCK);
    if(res != 0){
        perror("set fd nonblock failed");
        return;
    }
}
//add fd to the epoll,with epoll_ctl
void addFdToEpoll(int epollfd, int sockfd, bool enable_et){
    struct epoll_event event;
    event.events = EPOLLIN;
    if(enable_et){
        event.events = EPOLLIN | EPOLLET;
    }
    event.data.fd = sockfd;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event) != 0){
        perror("add fd to epoll failed");
    };
    setNonBlocking(sockfd);
}
//send broadcast message to the client in clients list, except for which we received the data
void sendBroadcastMessage(int sockfd){
    char buf[BUF_SIZE], message[BUF_SIZE];
    //initialize the space
    bzero(buf, BUF_SIZE);
    bzero(message, BUF_SIZE);

    int rlen = recv(sockfd, buf, BUF_SIZE, 0);
    printf("read data from Client #%d: %s\n", sockfd, buf);
    
    if(rlen == 0){
        close(sockfd);
        client_list.remove(sockfd);
        printf("client #%ld closed, Now, there are %d clients in the chatroom\n",sockfd, client_list.size());
        
    }else{
        if(client_list.size() == 1){
            int slen = send(sockfd, CAUTION, strlen(CAUTION), 0);
            return;
        }
        sprintf(message, SERVER_STRING, sockfd, buf);
        printf("will send:%s\n", message);
        list<int>::iterator iter;
        for( iter = client_list.begin(); iter != client_list.end(); iter++ ){
            puts("sending data...");
            if(*iter != sockfd){
                send(*iter, message, BUF_SIZE, 0);
            }
        }
    }
}

