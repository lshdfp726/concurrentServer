#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "breakRecycle.h"


#define NUM_PHILOSOPHERS 5

pthread_mutex_t forks[NUM_PHILOSOPHERS]; //叉子的互斥锁

void pickup_forks(int leftfork, int rightfork, int philosopher_id) {
    pthread_mutex_lock(&forks[leftfork]);
    pthread_mutex_lock(&forks[rightfork]);
    printf("Philosopher %d is eating.\n", philosopher_id);
}

void return_forks(int leftfork, int rightfork) {
    pthread_mutex_unlock(&forks[leftfork]);
    pthread_mutex_unlock(&forks[rightfork]);
}


static void *philosopher(void *arg) {
    int i = *(int *)arg;
    int leftfork = (i + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int rightfork = (i + 1) % NUM_PHILOSOPHERS;

    while (1)
    {
        if (i % 2 == 0) {
            pickup_forks(leftfork, rightfork, i);
        } else {
            pickup_forks(rightfork, leftfork, i);
        }

        sleep(1);

        return_forks(leftfork, rightfork);

        sleep(1);
    }
    
    return NULL;
}

void startRecycle(void) {
  //哲学家线程id
    pthread_t philosophers[NUM_PHILOSOPHERS];
    //标识当前线程的哲学家编号
    int indexes[NUM_PHILOSOPHERS];

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
}