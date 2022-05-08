#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#define OVEN_MAX 5
#define TABLE_MAX 5


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
    

   


    return 0;
}