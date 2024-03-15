#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/**
 * 吸烟者问题描述
 * 三个吸烟者在一个房间内，还有一个香烟供应者。
 * 为了制造并抽掉香烟，每个吸烟者需要三样东西:烟草、纸和火柴，供应者有丰富货物提供。
 * 三个吸烟者中，第一个有自己的烟草，第二个有自己的纸，第三个有自己的火柴。
 * 供应者随机地将两样不同的东西放在桌子上，允许一个吸烟者进行对健康不利的吸烟。
 * 当吸烟者完成吸烟后唤醒供应者，供应者再随机地把两样不同的东西放在桌子上，唤醒一个吸烟者。
 * 试用信号量的P、V操作设计该问题的同步算法，给出所用共享变量(如果需要)和_吸烟者问题
 * 
 * 这里核心问题就是一个: 当前抽烟者手里拿的资源，然后这个资源准备好之后，让供应者释放资源信号。所以互斥的信号量是当前抽烟人手里的资源，而不是另外两种资源。
 * 代理商放资源需要抽烟者线程激活。抽烟者线程也靠代理商当前手里的资源激活，形成另类互斥。
 */

sem_t *tobacco_sem, *paper_sem, *match_sem, *agent_sem;

void *agent(void *arg);
void *smoker(void *arg);

int main(int argc, char const *argv[]) {
    sem_unlink("tobacco_sem");
    tobacco_sem = sem_open("tobacco_sem", O_CREAT | O_EXCL, 0666, 0);
    sem_unlink("paper_sem");
    paper_sem = sem_open("paper_sem", O_CREAT | O_EXCL, 0666, 0);
    sem_unlink("match_sem");
    match_sem = sem_open("match_sem", O_CREAT | O_EXCL, 0666, 0);

    sem_unlink("agent_sem");
    //代理商始终有一个
    agent_sem = sem_open("agent_sem", O_CREAT | O_EXCL , 0666, 1);
    
    pthread_t agent_thread, smoker_thread[3];
    pthread_create(&agent_thread, NULL, agent, NULL);

    int ids[3] = {0,1,2};
    for(int i = 0; i < 3; i ++) {
        pthread_create(&smoker_thread[i], NULL, smoker, &ids[i]);
    }
    
    pthread_join(agent_thread, NULL);
    for (int i = 0; i < 3; i++) {
        pthread_join(smoker_thread[i], NULL);
    }
    
    sem_unlink("tobacco_sem");
    sem_unlink("paper_sem");
    sem_unlink("match_sem");
    sem_unlink("agent_sem");

    return 0;
}


void *agent(void *arg) {
    while (1) {
        sem_wait(agent_sem);
        printf("agent is wakeUped\n");
        int r = rand() % 3;
        if (r == 0) {
            printf("Agent provides tobacco_sem and paper_sem\n");
            sem_post(match_sem);
        } else if (r == 1) {
            printf("Agent provides tobacco_sem and match_sem\n");
            sem_post(paper_sem);
        } else {
            printf("Agent provides paper_sem and match_sem\n");
            sem_post(tobacco_sem);
        }
        sleep(1);
    }
}

void *smoker(void *arg) {
    int id = *(int *)arg;
    sem_t *my_sem = NULL;
    char *name = NULL;
    switch (id) {
        case 0:
            my_sem = tobacco_sem;
            name = "tobacco_sem";
            break;
        case 1:
            my_sem = paper_sem;
            name = "paper_sem";
            break;
        case 2:
            my_sem = match_sem;
            name = "match_sem";
            break;
        default:
            break;
    }

    while (1) {
        sem_wait(my_sem);
        printf("Smoker %s is smoking\n", name);
        sem_post(agent_sem);
        sleep(1); // Sleep for 1 seconds
    }
}