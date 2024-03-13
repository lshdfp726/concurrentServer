#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "lshSbuf.h"
#include "lshIO.h"
#include "lshSocket.h"

#define NTHREADS 4
#define SBUFSIZE 16

void echo_cnt(int connfd);
void *thread(void *vargp);

lshSbuf sbuf; //共享存储客户端文件描述符数据结构

//基于预线程化并发事件循环服务器, 一个生产者和多个消费者服务器模型
int main(int argc, char const *argv[])
{
    int i, listenfd, connfd;
    socklen_t clientLen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    listenfd = lsh_openListenfd((char *)argv[1]);

    lshSbuf_init(&sbuf, SBUFSIZE, "test");

    //预初始化线程池
    for (i = 0; i < NTHREADS; i++)
        pthread_create(&tid, NULL, thread, NULL);
    
    while (1)
    {
        clientLen = sizeof(struct sockaddr_storage);
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientLen);
        lshSbuf_insert(&sbuf, connfd);
    }

    return 0;
}

void *thread(void *vargp) 
{
    pthread_detach(pthread_self());
    while(1) {
        int connfd = lshSbuf_remove(&sbuf);
        echo_cnt(connfd);
        close(connfd);
    }
}

static int byte_cnt; //客户端接收到的总字节数
static sem_t *mutex; //byte_cnt 多线程的锁

static void init_echo_cnt(void) {
    mutex = sem_open("test1", O_CREAT, 0644, 1);
    byte_cnt = 0;
}

void echo_cnt(int connfd) {
    int n;
    char buf[MAXBUF];
    lshRio_t rio;
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, init_echo_cnt);
    lshRio_readinitb(&rio, connfd);
    while ((n = lshc_readline(&rio, buf, MAXBUF)) != 0)
    {
        // sem_wait(mutex);
        byte_cnt += n;
        printf("server received %d(%d total) bytes on fd %d\n",n, byte_cnt, connfd);
        // sem_post(mutex);
        lsh_writen(connfd, buf, n);
        if (strstr(buf, "\\r\\n")) { //为了判断数据边界，跳出while循环。
            char end[] = "\r\n";
            lsh_writen(connfd, end, strlen(end));//向客户端发送一个结束标识
            break;
        }
    }
} 