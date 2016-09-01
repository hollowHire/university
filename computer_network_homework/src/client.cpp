//
//  main.cpp
//  socket_sms
//
//  Created by yume on 16/6/21.
//  Copyright © 2016年 yume. All rights reserved.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

int PORT=8044;

void *recv_data(void *fd){
    int client_sockfd;
    int i,byte;
    char char_recv[100];//存放数据
    client_sockfd=*((int*)fd);
    while(1)
    {
        if((byte=recv(client_sockfd,char_recv,100,0))==-1)
        {
            perror("接收消息失败");
            exit(EXIT_FAILURE);
        }
        if(strcmp(char_recv, "exit")==0){
            printf("服务器已登出\n");
            break;
        }
        printf("%s\n",char_recv);
        memset(char_recv, 0, sizeof(char_recv));
    }
    //    free(fd);
    close(client_sockfd);
    pthread_exit(NULL);
}

int main(int argc,char *argv[])
{
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    int i,byte;
    char char_send[100] = { 0 };
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        perror("创建套接字失败");
        exit(EXIT_FAILURE);
    }
    printf("创建套接字成功！\n");
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    printf("请输入端口号:\n");
    scanf("%d",&PORT);
    printf("请输入IP地址:\n");
    char* IP_ADD;
    scanf("%s",IP_ADD);
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr=inet_addr(IP_ADD);
    len = sizeof(address);
    
    if(connect(sockfd, (struct sockaddr *)&address, len)==-1)
    {
        perror("连接到服务器失败");
        exit(EXIT_FAILURE);
    }
    printf("连接成功\n请输入用户名:\n");
    char name[40];
    scanf("%s",name);
    if((byte=send(sockfd,name,40,0))==-1)
    {
        perror("发送用户名失败");
        exit(EXIT_FAILURE);
    }
    pthread_t thread;
    if (pthread_create(&thread, NULL, recv_data, &sockfd)!=0) {
        perror("子线程创建失败");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        printf("请输入消息：\n");
        scanf("%s", char_send);//输入发送数据
        fflush(stdin);//清除输入缓存
        if(strcmp(char_send, "exit")==0){//如果输入exit，跳出循环
            if((byte=send(sockfd,char_send,100,0))==-1)
            {
                perror("发送消息失败");
                exit(EXIT_FAILURE);
            }
            printf("退出连接.\n");
            exit(1);
            break;
        }
        if((byte=send(sockfd,char_send,100,0))==-1)
        {
            perror("发送消息失败");
            exit(EXIT_FAILURE);
        }
        
    }
    close(sockfd);
    exit(0);
}

