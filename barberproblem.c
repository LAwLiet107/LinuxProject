//
//  main.c
//  Barbers Problem
//
//  Created by 懿想天开 on 2018/12/30.
//  Copyright © 2018 懿想天开. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <math.h>

#define chairNum 4
//正在理发的顾客数量
sem_t cus;
//等待顾客的理发师数量
sem_t bar;
//互斥锁，保证在任一时刻，只能有一个线程访问该对象
pthread_mutex_t mutex;
//等待理发的人数
int waitNum = 0;
//标志量,flag==1则表示不再有人理发
int closingTime = 0;
//成功理发的人数
int num = 0;

void* barFunction() {
    while(1) {
        if(waitNum == 0) {
            printf("No customers, zzZ zzZ...\n");
        }
        //如果cus == 0，则进程阻塞，否则cus--
        sem_wait(&cus);
        //锁定互斥锁
        pthread_mutex_lock(&mutex);
        //等待人数-1
        waitNum--;
        //释放互斥锁
        pthread_mutex_unlock(&mutex);
        //开始理发
        printf("Start: No.%d.\n", ++num);
        //等待1-5s
        sleep(rand()%3 + 3);
        //理发完成，理发师数量+1
        sem_post(&bar);
	//不加sleep，下一个人的 start 和前一个人的 finish 会乱序
	sleep(1);
        //如果到了关门时间且当前没有等待顾客，则跳出
        if(closingTime && !waitNum) {
            break;
        }
    }
    printf("Go off work.\n");
    return 0;
}

void* cusFunction() {
    //锁定互斥锁
    pthread_mutex_lock(&mutex);
    //若等待区有空座，则新来的顾客做下等待
    if(waitNum < chairNum) {
        //等待人数+1
        waitNum++;
        printf("New cunstomer No.%d, %d Waiting.\n", (unsigned int)pthread_self(), waitNum);
        //释放互斥锁
        pthread_mutex_unlock(&mutex);
        //正在理发人数+1
        sem_post(&cus);
        //若当前bar==0则阻塞，bar==1则bar--
        sem_wait(&bar);
	//理发完成，并输出线程号
        printf("Customer No.%d has Finished.\n", (unsigned int)pthread_self());
    }
    //等待去无空座，顾客离开
    else {
        printf("Customer No.%d has Leaved.\n",(unsigned int)pthread_self());
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}

int main() {
    int i;
    //定义线程ID号，即线程标识符
    pthread_t barID, cusID;
    //int sem_init(sem_t *sem,int pshared,unsigned int value) 初始化一个定位在sem的匿名信号量;
    //value:参数指定信号量的初始值;
    //pashared = 0则信号量被进程内线程共享，并且放置在这个进程的所有线程都可见的地址上；!=0则信号量将在进程之间共享，并且t应该定位共享内存区域；
    //成功返回0，错误返回-1;
    //初始化cus与bar均为1
    sem_init(&cus, 0, 0);
    sem_init(&bar, 0, 0);
    //int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);
    //互斥锁的初始化，成功返回0，失败返回其他
    //attr:新建互斥锁的属性，NULL为默认属性
    pthread_mutex_init(&mutex, NULL);
    //int pthread_create(pthread_t *restrict tidp,const pthread_attr_t *restrict_attr,void*（*start_rtn)(void*),void *restrict arg);
    //第一个参数为指向线程标识符的指针
    //第二个参数用来设置线程属性
    //第三个参数是线程运行函数的地址
    //第四个参数是运行函数的参数
    //创建线程，运行barFunction()
    pthread_create(&barID, NULL, barFunction, NULL);
    //srand() + time(NULL) 保证每次seed值不同，从而保证每次rand()产生不同的随机数序列
    srand((unsigned)time(NULL));
    //每天最多服务10名顾客
    for(i = 0; i < 10; i++) {
        //sleep(1-3之内的随机数) 将每次创建线程的时间延后
        sleep(rand()%3 + 1);
        //c创建线程，运行cunFunction()函数
        pthread_create(&cusID, NULL, cusFunction, NULL);
    }
    //关门
    closingTime = 1;
    //等待线程结束
    pthread_join(barID, NULL);
    printf("Finished customers: %d, Leaved customers: %d.\n", num, 10 - num);
    return 0;
}

