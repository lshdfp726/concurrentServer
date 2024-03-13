#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "lshIO.h"
#include "lshSocket.h"

typedef struct
{
    int maxfd;  //read_set 中最大的文件描述符
    fd_set read_set;   //所有活动描述符的集合
    fd_set ready_set;   //准备读取描述符的子集
    int nready;  // select 函数准备好的文件描述符数量
    int maxi;   // clientfd 数组最大索引
    int clientfd[FD_SETSIZE]; //存所有客户端已连接的文件描述符
    lshRio_t clientRio[10]; //存所有连接的客户端缓存区数组
} lshPool;

void init_pool(int listenfd, lshPool *p);
void add_client(int connfd, lshPool *p);
void check_clients(lshPool *p);

int byte_cnt = 0; //

//基于I/O多路复用的事件循环机制
int main(int argc, char const *argv[])
{
    int listenfd, connfd;
    socklen_t clientLen;
    struct sockaddr_storage clientAddr;
    //这里一定要用static 修饰，内存开辟到全局区，如果在栈区会因为 clientRio[FD_SETSIZE] 太大导致栈帧爆了，产生段错误
    static lshPool pool = {0};

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    printf("pid is %d\n",getpid());
    listenfd = lsh_openListenfd((char *)argv[1]);
    init_pool(listenfd, &pool);

    while (1) {   
        pool.ready_set = pool.read_set;
        pool.nready = select(pool.maxfd + 1, &pool.read_set, NULL,NULL,NULL);

        if (FD_ISSET(listenfd, &pool.read_set)) {
            clientLen = sizeof(clientAddr);
            connfd = accept(listenfd, (struct sockaddr *)&clientAddr, &clientLen);
            add_client(connfd, &pool);
        }

        check_clients(&pool);
    }
    

    return 0;
}

void init_pool(int listenfd, lshPool *p) {
    int i;
    p->maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++) 
        p->clientfd[i] = -1;
    
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, lshPool *p) {
    int i;
    for (i = 0; i < FD_SETSIZE; i++) {
        //clientfd[i]初始化为-1
        if (p-> clientfd[i] < 0) {
            p->clientfd[i] = connfd;
            lshRio_readinitb(&p->clientRio[i], connfd);

            if (connfd > p->maxfd)
                p->maxfd = connfd;
            
            if (i > p->maxi) 
                p->maxi = i;
            
            break;
        }
    }
    if (i == FD_SETSIZE)
        printf("add_client error, Too many clients\n");
}

//检查客户端可读的文件描述符，然后再原路送回去。
void check_clients(lshPool *p) {
    int i, connfd, n;
    char buf[MAXBUF];
    lshRio_t rio;
    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++) {
        connfd = p->clientfd[i];
        rio = p->clientRio[i];
        if (connfd > 0) {
            p->nready --;

            while ((n = lshc_readline(&rio, buf, MAXBUF)) != 0) {
                byte_cnt += n;
                printf("Server received %d(%d total) bytes on fd %d\n", n, byte_cnt,  connfd);
                lsh_writen(connfd, buf, strlen(buf));
                if (strstr(buf, "\\r\\n")) { //为了判断数据边界，跳出while循环。
                    char end[] = "\r\n";
                    lsh_writen(connfd, end, strlen(end));//向客户端发送一个结束标识
                    break;
                }
            }
            close(connfd);
            FD_CLR(connfd, &p->read_set);
            p->clientfd[i] = -1;
        }
    }
    p->nready--;
}