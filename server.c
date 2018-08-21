#include "common.h"
int main(){
    //socket address statement
    struct sockaddr_in server_addr;
    //initialize the address
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_family = AF_INET;
    //create server socket
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    //bind the server address
    if(bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0){
        perror("bind failed");
        return 1;
    }
    //listen on the address
    if(listen(listener, 5) != 0){
        perror("listen failed");
        return 2;
    }
    //create epoll
    int epollfd = epoll_create(EPOLL_SIZE);
    if(epollfd < 0){
        perror("create epollfd failed");
        return 3;
    }
    //add server fd to epoll
    addFdToEpoll(epollfd, listener, true);
    //epoll_event array
    static struct epoll_event events[EPOLL_SIZE];
    //main loop
    int clientfd;
    while(1){
        //epoll_wait
        puts("waiting clients...");
        int event_count = epoll_wait(epollfd,events, EPOLL_SIZE, -1);
        printf("epoll event count is %d\n", event_count);
        for(int i = 0; i< event_count; i++){
            clientfd = events[i].data.fd;
             //if fd is serverfd, accept a connection, print information, add new sockfd to epoll
            if(clientfd == listener){
                struct sockaddr_in client_addr;
                socklen_t addrlen = sizeof(struct sockaddr_in);
                int newfd = accept(listener, (struct sockaddr*)&client_addr, &addrlen);
                if(newfd < 0){
                    perror("accept new connections failed");
                    exit(EXIT_FAILURE);
                }
                printf(
                    "new connection from %s:%d, client id #%d\n",
                    inet_ntoa(client_addr.sin_addr),
                    ntohs(client_addr.sin_port),
                    newfd
                );
                addFdToEpoll(epollfd, newfd, true);
                client_list.push_back(newfd);
                printf("welcome message\n");
                char message[BUF_SIZE];
                bzero(message, BUF_SIZE);
                sprintf(message, WELCOME_STRING, newfd);
                send(newfd, message, strlen(message), 0);
            }//if not, sendbroadcast message
            else{
                sendBroadcastMessage(clientfd);
            }
        }
    }
}
