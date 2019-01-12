//
//  main.c
//  Baker Problem
//
//  Created by 懿想天开 on 2018/12/30.
//  Copyright © 2018 懿想天开. All rights reserved.
//

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} semun1,semun2;

//创建信号集，并返回信号灯集描述字
int createSemaphore() {
    //int semget(key_t key, int num_sems, int sem_flags);
    //创建一个新信号或取得一个已有信号量
    //key:信号灯标志; num_sems:信号灯集中包含信号灯的数目; sem_flags:标志位（d若为IPC_CREAT标志，那么参数num_sems必须与原来的值一致）
    //创建信号灯数目为2的信号灯集
    return semget(1, 2, IPC_CREAT);
}

//创建信号集并初始化
void init() {
    int sid = createSemaphore();
    //等待顾客数量初始值
    semun1.val = 0;
    //空闲售货员数量初始值
    semun2.val = 3;
    semctl(sid, 0, SETVAL, semun1);
    semctl(sid, 1, SETVAL, semun2);
}

/*
 信号量操作由sembuf结构表示
 struct sembuf {
    short sem_num;      //除非使用一组信号量，否则它为0
    short sem_op;       //信号量在一次操作中需要改变的数据，通常是两个数，一个是-1，即P（等待）操作，一个是+1，即V（发送信号）操作
    short sem_flg;      //通常为SEM_UNDO,使操作系统跟踪信号，并在进程没有释放该信号量而终止时，操作系统释放信号量
 };
*/

//新顾客到来，顾客等待人数+1
void cusArrival(int sid) {
    struct sembuf operation = {0, 1, SEM_UNDO};
    //对信号量集标识符为0的信号量进行V操作
    semop(sid, &operation, 1);
    printf("A New Customer has Arrived\nWaiting Customers: %d\n\n", semctl(sid, 0, GETVAL, semun1));
}

//销售完毕后空闲的售货员数量+1
void salFree(int pid, int sid) {
    struct sembuf operation = {1, 1, SEM_UNDO};
    //对信号量集标识符为1的信号量进行V操作
    semop(sid, &operation, 1);
    printf("Salesman No.%d is Free.\nFree Salesmen: %d.\n\n", pid, semctl(sid, 1, GETVAL, semun2));
}

//叫下一个号码
void nextNum(int pid, int sid) {
    //叫下一个号码时等待顾客数量-1
    struct sembuf operation0 = {0, -1, SEM_UNDO};
    //对信号量集标识符为0的信号量进行P操作
    semop(sid, &operation0, 1);
    //叫下一个号码时空闲售货员数量-1
    struct sembuf operation1 = {1, -1, SEM_UNDO};
    //对信号量集标识符为1的信号量进行P操作
    semop(sid, &operation1, 1);
    printf("Salesman No.%d is Working.\nFree Salesmen: %d.\nWaiting Customers: %d.\n\n", pid, semctl(sid, 1, GETVAL, semun2), semctl(sid, 0, GETVAL, semun1));
}

void cusFunction() {
    int sid = createSemaphore();
    while(1) {
	//顾客到来
        cusArrival(sid);
        //两个顾客到来的时间间隔
        sleep(rand()%3 + 1);
    }
}

void salFunction() {
    int pid = getpid();
    int sid = createSemaphore();
    while(1) {
	//叫号
	nextNum(pid, sid);
        //售货员服务一名顾客的时间
        sleep(rand()%10 + 5);
	//售货员销售完毕
        salFree(pid, sid);
    }
}

int main() {
    init();
    //创建三个进程，表示三个售货员
    pid_t sal1, sal2, sal3;
    srand((unsigned)time(NULL));
    sal1 = fork();
    if(sal1 == 0 )
        salFunction();
    sal2 = fork();
    if(sal2 == 0 )
        salFunction();
    sal3 = fork();
    if(sal3 == 0 )
        salFunction();
    else
        cusFunction();
    return 0;
}
