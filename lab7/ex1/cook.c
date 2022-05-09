#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#define OVEN_MAX 5
#define TABLE_MAX 5

int *table, *oven;


int main(){

    // get oven array from shared memory
    key_t o_key;
    if ((o_key = ftok("cook.c", 'O')) == -1){
        printf("Can't get the key of oven array!\n");
        exit(1);
    }

    int o_shmid;
    if ((o_shmid = shmget(o_key, OVEN_MAX * sizeof(int), 0)) == -1){
        printf("Can't get oven array from shared memory segment!\n");
        exit(1);
    }

    if ((oven = shmat(o_shmid, NULL, 0)) == (int *) -1){
        printf("Can't attach the segment to cook data space!\n");
        exit(1);
    }

    // get table array from shared memory
    key_t t_key;
    if ((t_key = ftok("deliverer.c", 'T')) == -1){
        printf("Can't get the key of table array!\n");
        exit(1);
    }

    int t_shmid;    
    if ((t_shmid = shmget(t_key, TABLE_MAX * sizeof(int), IPC_CREAT | 0666)) == -1){
        printf("Can't get table array from shared memory segment!\n");
        exit(1);
    }

    if ((table = shmat(t_shmid, NULL, 0)) == (int *) -1){
        printf("Can't attach the segment to cook data space!\n");
        exit(1);
    }

    // get semaphore
    key_t s_key;
    if ((s_key = ftok("pizzeria.c", 'S')) == -1){
        printf("Can't get the key for semaphore!\n");
        exit(1);
    }
    
    int semid;
    if ((semid = semget(s_key, 0, 0)) == -1){
        printf("Can't get semaphore id!\n");
        exit(1);
    }

    // structure for performing operations on semaphores
    /*
    struct sembuf sb[3];
    sb[0].sem_num = 1;
    sb[0].sem_op = 1;
    sb[0].sem_flg = 0;
    semop(semid, sb, 1);
    */
    
    srand(time(NULL));
    int pizza_type;
    float prep_time, bake_time;
    struct timespec ts, wait_time;
    char buff[100];
    while(1){
        pizza_type = rand() % 10;

        prep_time = (rand() / (double) RAND_MAX) + 1;
        timespec_get(&ts, TIME_UTC);
        strftime(buff, sizeof buff, "%T", localtime(&ts.tv_sec));
        printf("Preparing pizza...\n");
        printf("Preparation estimated time: %f s.\n", prep_time);
        printf("Cook with pid %ld is preparing pizza number: %d.\n", (long) getpid(), pizza_type);
        printf("Time: %s.%ld\n\n", buff, ts.tv_nsec / 1000000);
        wait_time.tv_sec = 1;
        wait_time.tv_nsec = (long) ((prep_time - 1) * 1e9);
        nanosleep(&wait_time, NULL);
        
        bake_time = (rand() / (double) RAND_MAX) + 4;
        printf("Baking pizza...\n");
        printf("Baking estimated time: %f s.\n", bake_time);
        printf("Cook with pid %ld added pizza number: %d to the oven!\n", (long) getpid(), pizza_type);
        timespec_get(&ts, TIME_UTC);
        strftime(buff, sizeof buff, "%T", localtime(&ts.tv_sec));
        printf("Time: %s.%ld\n", buff, ts.tv_nsec / 1000000);
        printf("Number of pizzas currently in the oven: %d\n\n", -1);
        wait_time.tv_sec = 4;
        wait_time.tv_nsec = (long) ((bake_time - 4) * 1e9);
        nanosleep(&wait_time, NULL);

        printf("Cook with pid %ld took out pizza number: %d from the oven and placed on the table!\n", (long) getpid(), pizza_type);
        timespec_get(&ts, TIME_UTC);
        strftime(buff, sizeof buff, "%T", localtime(&ts.tv_sec));
        printf("Time: %s.%ld\n", buff, ts.tv_nsec / 1000000);
        printf("Number of pizzas in the oven: %d\n", -1);
        printf("Number of pizzas on the table: %d\n\n", -1);
    }


    return 0;
}