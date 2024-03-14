#ifndef LSHSBUF_H
#define LSHSBUF_H

#include <semaphore.h>

typedef struct lshSbuf
{
    int *buf;   //buffer array
    int n;      //Maximum number of slots
    int front;  //buf[(front + 1)%n] is first item
    int rear;   //buf[rear%n] is last item
    sem_t *mutex; //protects accesses to buf
    const char *mutexName;
    sem_t *slots; //Counts available slots
    const char *slotsName;
    sem_t *items; //Counts available items
    const char *itemsName;
} lshSbuf;

/**
 * @brief 
 * 
 * @param sp 信号量和数据的模型
 * @param n slot 数量，表示最大支持多少个生产者
 * @param name //标识信号量名称
 */
void lshSbuf_init(lshSbuf *sp, int n, const char *name);

void lshSbuf_deInit(lshSbuf *sp);

//item 插入到rear
void lshSbuf_insert(lshSbuf *sp, int item);

//从front 取
int lshSbuf_remove(lshSbuf *sp);

#endif