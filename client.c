#include"common.h"
int main(){
    //server address statement
    struct sockaddr_in server_addr;
    //initilize server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    //create sock
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        perror("create socket failed");
        return 1;
    }
    if(connect(sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) != 0){
        perror("connect server failed");
        return 0;
    }
    int pipefd[2];
    if(pipe(pipefd) != 0){
        perror("create pipe failed");
        return 2;
    }
    int epollfd = epoll_create(EPOLL_SIZE);
    if(epollfd< 0){
        perror("create epollfd failed");
        return 3;
    }
    addFdToEpoll(epollfd, sock, true);
    addFdToEpoll(epollfd, pipefd[0], true);
    pid_t pid = fork();
    if(pid < 0){
        perror("create child process failed");
        return 4;
    }
    int client_running = 1;
    char message[BUF_SIZE];
    char buf[BUF_SIZE];
    struct epoll_event events[2];
    
    if(pid == 0){
        close(pipefd[0]);
        printf("please input your message, and type exit to terminate\n");
        while(client_running){

            //printf("input>");
            bzero(message, BUF_SIZE);
            //bzero(buf, BUF_SIZE);
            fgets(message, BUF_SIZE, stdin);
            if(strncmp(message, "exit", strlen("exit")) == 0){
                client_running = 0;
                close(sock);
                close(pipefd[1]);
                return 0;
                
            }else{
                write(pipefd[1], message, strlen(message));
            }
        }
    }else{
        close(pipefd[1]);
        while(client_running){
            int event_count = epoll_wait(epollfd, events, 2, -1);
            for(int i = 0; i < event_count; i++){
                if(events[i].data.fd == pipefd[0]){
                    bzero(buf, BUF_SIZE);
                    if(read(pipefd[0], buf, BUF_SIZE) == 0){
                        client_running = 0;
                    }else{
                        printf("\nyour input:%s\n", buf);
                        send(sock, buf, strlen(buf) - 1, 0);

                    }
                }else{
                    recv(sock, buf, BUF_SIZE, 0);
                    printf("\nServer message:%s\n", buf);
                }
            }
        }
    }

    close(sock);
    if(pid){
        close(pipefd[0]);
    }else{
        close(pipefd[1]);
    }
    return 0;
}
//create sock
//initialize pipefd
//create epollfd
//add fd to the epollfd
//fork client
//interact with server
//read message and show it to user
//read user input
//get input and send it to server