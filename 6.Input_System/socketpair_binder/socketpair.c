/*************************************************************************
	> File Name: socketpair.c
	> Author: H233
	> Mail: 937138688@qq.com
	> Created Time: 2018年05月08日 星期二 21时54分17秒

    frameworks\native\libs\input\InputTransport.cpp  (socketpair)
 ************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SOCKET_BUFFER_SIZE (32768)

void *function_thread1(void *args)
{
    char buf[512];
    int fd = (int)args;
    int len;
    int cnt = 0;

    while(1) {
        len = sprintf(buf, "Hello, main thread, cnt = %d", cnt++);
        /*send "Hello, main thread" to main thread*/
        write(fd, buf, len);    
        
        /*recv data from main thread*/
        len = read(fd, buf, sizeof(buf));
        buf[len] = '\0';
        printf("%s\n", buf);

        sleep(5);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    int sockets[2];
    int bufferSize = SOCKET_BUFFER_SIZE;


    socketpair(AF_UNIX, SOCK_SEQPACKET, 0, sockets);
    
    setsockopt(sockets[0], SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize));
    setsockopt(sockets[0], SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize));
    setsockopt(sockets[1], SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize));
    setsockopt(sockets[1], SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize));
   
    /*create thread1*/
    pthread_t tid1;

    /*thread1 send "Hello, main thread" to main thread*/
    int ret = pthread_create(&tid1, NULL, function_thread1, (void *)sockets[1]);
    if (ret < 0) {
        printf("create tid1 failed\n");
        return -1;
    }
   


    char buf[512];
    int len;
    int cnt = 0;
    int fd = sockets[0];
    /*main thread send "Hello, thread1" to thread1*/
    while(1) {
        /*recv data from thread1*/
        len = read(fd, buf, sizeof(buf));
        buf[len] = '\0';
        printf("%s\n", buf);

        /*send msg to thread1*/
        len = sprintf(buf, "Hello, thread1, cnt = %d", cnt++);
        write(fd, buf, len);    
        sleep(5);
    }

    return 0;
}

