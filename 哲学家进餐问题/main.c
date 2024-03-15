#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "limitNums.h"
#include "breakRecycle.h"


/**
 * 哲学家进餐问题
 * 哲学家进餐问题是一个经典的并发编程问题，描述了五位哲学家围坐在圆桌前，每人面前放着一碗意大利面和一只叉子。
 * 他们只有在两只叉子都拿到手才能吃意大利面，吃完后才能放下叉子继续思考。
 * 问题是如何设计算法，使得哲学家们能够正确地进餐而不发生死锁（每个哲学家都在等待右边的叉子）或饥饿（某些哲学家始终无法拿到叉子）的情况。
 */

int main(int argc, char const *argv[]) {
   
//    start();
   startRecycle();
   return 0;
}