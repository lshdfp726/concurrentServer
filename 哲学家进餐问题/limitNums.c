#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "limitNums.h"

#define NUM_PHILOSOPHERS 5

pthread_mutex_t forks[NUM_PHILOSOPHERS]; //叉子的互斥锁
sem_t *max_philosophers; //限制同时最多哲学家进餐人数
 
static void *philosopher(void *arg) {
    int i = *(int *)arg;
    int leftFork = (i + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int rightFork = (i + 1) % NUM_PHILOSOPHERS;

    while (1)
    {
        sem_wait(max_philosophers);
        printf("Philosopher %d is thinking.\n", i);
        pthread_mutex_lock(&forks[leftFork]);
        pthread_mutex_lock(&forks[rightFork]);

        printf("Philosopher %d is eating.\n", i);
        sleep(1);

        pthread_mutex_unlock(&forks[leftFork]);
        pthread_mutex_unlock(&forks[rightFork]);

        sem_post(max_philosophers);

        sleep(1);
    }
    
    return NULL;
}

void start(void) {
    //哲学家线程id
    pthread_t philosophers[NUM_PHILOSOPHERS];
    //标识当前线程的哲学家编号
    int indexes[NUM_PHILOSOPHERS];

    sem_unlink("max_philosophers");
    //最多支持NUM_PHILOSOPHERS - 1,个人用餐，抽屉原理可知 最少有一个人手上会拿到两把叉子
    max_philosophers = sem_open("max_philosophers", O_CREAT | O_EXCL, 0666, NUM_PHILOSOPHERS - 1);
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) 
    {
        pthread_mutex_init(&forks[i], NULL);
        indexes[i] = i;
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        pthread_create(&philosophers[i], NULL, philosopher, &indexes[i]);
    }
    
    //等待回收线程
    for (int i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        pthread_join(philosophers[i], NULL);
    }
    
    //销毁线程
    for (int i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        pthread_mutex_destroy(&forks[i]);
    }

    unlink("max_philosophers");
}