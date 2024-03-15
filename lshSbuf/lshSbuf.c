#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "lshSbuf.h"

sem_t * lshSemo_open(const char *name, int oflag, mode_t mode, unsigned int value);

void lshSbuf_init(lshSbuf *sp, int n, const char *name) {
    fprintf(stdout, "lshSbuf_init");
    sp->buf = calloc(n, sizeof(int));
    sp->n = n;
    sp->front = sp->rear = 0;
    // sem_init(&sp->mutex, 0, 1); // Initialize the mutex semaphore in sp
    char *mutex_name = malloc(strlen(name) + strlen("_mutex") + 1);
    char *mutex_slots = malloc(strlen(name) + strlen("_slots") + 1);
    char *mutex_items = malloc(strlen(name) + strlen("_items")+ 1);;
    sprintf(mutex_name, "%s_mutex",name);

    //上次由于某个原因导致未调用lshSbuf_deInit来unlink. 那么调用lshSemo_open会出现问题
    sem_unlink(mutex_name);
    sp->mutex = lshSemo_open(mutex_name, O_CREAT | O_EXCL, 0644, 1);
    sp->mutexName = mutex_name;

    sprintf(mutex_slots, "%s_lots",name);
    sem_unlink(mutex_slots);
    sp->slots = lshSemo_open(mutex_slots, O_CREAT | O_EXCL, 0644, 1);
    sp->slotsName = mutex_slots;

    sprintf(mutex_items, "%s_items",name);
    sem_unlink(mutex_items);
    sp->items = lshSemo_open(mutex_items, O_CREAT | O_EXCL, 0644, 1);
    sp->itemsName = mutex_items;
}

sem_t * lshSemo_open(const char *name, int oflag, mode_t mode, unsigned int value) {
    sem_t *s = sem_open(name, O_CREAT, 0644, 1);
    if (s == SEM_FAILED) {
        perror("sem_open");
        printf("errno: %d\n", errno);
        return NULL;
    }

    return s;
}

void lshSbuf_deInit(lshSbuf *sp) {
    fprintf(stdout, "lshSbuf_deInit");
    if (sp->buf) {
        free(sp->buf);
    }

    if (sp->mutex) {
        if (sp->mutexName) {
            sem_unlink(sp->mutexName);
            free((void *)sp->mutexName);
        }
        free(sp->mutex);
    }
    if (sp->slots) {
        if (sp->slotsName) {
            sem_unlink(sp->slotsName);
            free((void *)sp->slotsName);
        }
        free(sp->slots);
    }
    if (sp->items) {
        if (sp->itemsName) {
            sem_unlink(sp->itemsName);
            free((void *)sp->itemsName);
        }
        free(sp->items);
    }   

}

void lshSbuf_insert(lshSbuf *sp, int item) {
    fprintf(stdout, "lshSbuf_insert");
    sem_wait(sp->slots);
    sem_wait(sp->mutex);
    sp->buf[(++sp->rear) % (sp->n)] = item;
    sem_post(sp->mutex);
    sem_post(sp->items);
}

int lshSbuf_remove(lshSbuf *sp) {
    fprintf(stdout, "lshSbuf_remove");
    int item;
    sem_wait(sp->items);
    sem_wait(sp->mutex);
    item = sp->buf[(++sp->front) %(sp->n)];
    sem_post(sp->mutex);
    sem_post(sp->slots);
    return item;
}