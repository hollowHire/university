//
//  main.cpp
//  socket_sms_server
//
//  Created by yume on 16/6/21.
//  Copyright © 2016年 yume. All rights reserved.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <list>
#include <iostream>
#include <string>

int PORT=8044;
int SIZE=1024;
char *now_message;
void *rec_data(void *fd);
std::list<int> sockets;

int main(int argc,char *argv[])
{
    
    int server_sockfd;
    int client_sockfd;
    int server_len;
    socklen_t client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t templen;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);//创建套接字
    if (server_sockfd==-1) {
        perror("创建套接字失败");
    }
    printf("创建套接字成功！\n");
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr =  htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);
    printf("请输入绑定的端口号:\n");
    scanf("%d",&PORT);
    server_len = sizeof(server_address);
    int res=bind(server_sockfd, (struct sockaddr *)&server_address, server_len);//绑定套接字
    if (res==-1) {
        perror("绑定到端口失败");
    }
    printf("绑定到端口成功!\n");
    templen = sizeof(struct sockaddr);
    while(1){
        pthread_t thread;           //创建不同的子线程以区别不同的客户端
        client_len = sizeof(client_address);
        int res=listen(server_sockfd, 10);
        if (res==-1) {
            printf("监听端口失败");
        }
        printf("正在监听...\n");
        client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, &client_len);
        if(client_sockfd==-1){
            perror("接受客户端连接失败");
            continue;
        }
        printf("接受客户端连接成功...\n");
        if(pthread_create(&thread, NULL, rec_data, &client_sockfd)!=0)//创建子线程
        {
            perror("子线程创建失败");
            break;
        }
        sockets.push_back(client_sockfd);
//        std::cout<<"socket:"<<client_sockfd<<std::endl;
    }
    shutdown(client_sockfd,2);
    shutdown(server_sockfd,2);
}

void send_to_all(){
    std::list<int>::iterator it;
    for (it=sockets.begin(); it!=sockets.end(); it++) {
        if (send(*it, now_message, strlen(now_message), 0)==-1) {
            perror("发送全局消息失败");
            exit(EXIT_FAILURE);
        }
    }
}


void *rec_data(void *fd){
    int client_sockfd;
    int i,byte;
    char char_recv[SIZE];//存放数据
    client_sockfd=*((int*)fd);
    char name[SIZE];
    if((byte=recv(client_sockfd,name,40,0))==-1)
    {
        perror("接收用户名失败");
        exit(EXIT_FAILURE);
    }
    printf("%s已进入聊天室.\n",name);
    now_message=name;
    std::string nname=name;
    strcat(now_message, "已进入聊天室.");
    send_to_all();
    while(1)
    {
        if((byte=recv(client_sockfd,char_recv,100,0))==-1)
        {
            perror("接收消息失败");
            exit(EXIT_FAILURE);
        }
//        send(client_sockfd, char_recv, 100, 0);
        if(strcmp(char_recv, "exit")==0){
            printf("%s已退出聊天室.\n",nname.c_str());
            const char* nnnmae=nname.data();
//            now_message="\0";
            memset(now_message, 0, sizeof(now_message));
            strcat(now_message, nname.c_str());
            strcat(now_message, "已退出聊天室.");
            std::list<int>::iterator it;
            send_to_all();
            for (it=sockets.begin(); it!=sockets.end(); it++) {
                if (*it==client_sockfd) {
                    sockets.remove(*it);
                }
            }
            break;
        }
        printf("%s:%s\n",nname.c_str(),char_recv);//打印收到的数据
        memset(now_message, 0, sizeof(now_message));
//        now_message="\0";
        strcat(now_message, nname.c_str());
        strcat(now_message, ":");
        strcat(now_message, char_recv);
        send_to_all();
    }
//    free(fd);
    close(client_sockfd);
    pthread_exit(NULL);
}