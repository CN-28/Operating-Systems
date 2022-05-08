#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#define TABLE_MAX 5
#define OVEN_MAX 5

int *table;
int *oven;


int main(int argc, char *argv[]){
    if (argc != 3){
        printf("Wrong number of arguments!\n");
        exit(1);
    }

    long N = strtol(argv[1], NULL, 10);
    if (N <= 0){
        printf("Incorrect argument provided for N!\n");
        exit(1);
    }

    long M = strtol(argv[2], NULL, 10);
    if (M <= 0){
        printf("Incorrect argument provided for M!\n");
        exit(1);
    }

    // create key for shared memory segment of table array
    key_t t_key;
    if ((t_key = ftok("deliverer.c", 'T')) == -1){
        printf("Can't create a key for table array!\n");
        exit(1);
    }

    // create key for shared memory segment of oven array
    key_t o_key;
    if ((o_key = ftok("cook.c", 'O')) == -1){
        printf("Can't create a key for oven array!\n");
        exit(1);
    }

    // allocate a shared memory segment for oven array
    int o_shmid;
    if ((o_shmid = shmget(o_key, OVEN_MAX * sizeof(int), IPC_CREAT | 0666)) == -1){
        printf("Can't allocate a shared memory segment for oven array!\n");
        exit(1);
    }

    // initialize empty oven
    for (int i = 0; i < OVEN_MAX; i++)
        oven[i] = -1;

    // attach oven array to shared memory
    if ((oven = (int *) shmat(o_shmid, NULL, 0)) == (int *) -1){
        printf("Can't attach shared memory segment for oven array!\n");
        exit(1);
    }

    // allocate a shared memory segment for table array
    int t_shmid;    
    if ((t_shmid = shmget(t_key, TABLE_MAX * sizeof(int), IPC_CREAT | 0666)) == -1){
        printf("Can't allocate a shared memory segment for table array!\n");
        exit(1);
    }
    
    // initialize empty table
    for (int i = 0; i < TABLE_MAX; i++)
        table[i] = -1;

    // attach table array to shared memory
    if ((table = (int *) shmat(t_shmid, NULL, 0)) == (int *) -1){
        printf("Can't attach shared memory segment for table array!\n");
        exit(1);
    }

    // create key for semaphores set
    key_t s_key;
    if ((s_key = ftok("pizzeria.c", 'S')) == -1){
        printf("Can't create a key for semaphore!\n");
        exit(1);
    }
    
    // create 2 semaphores
    int semid;
    if ((semid = semget(s_key, 3, IPC_CREAT | 0666)) == -1){
        printf("Can't create semaphore!\n");
        exit(1);
    }    

    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } arg;
    arg.val = OVEN_MAX - 1;

    // set all semaphores' values to 0
    for (int i = 0; i < 3; i++){
        if (semctl(semid, i, SETVAL, arg) == -1){
            printf("Can't initialize semaphores values!\n");
            exit(1);
        }
        if (i == 1) arg.val = 1;
        else if (i == 2) arg.val = 0;
    }
    
    /* 
    char *cook_args[] = {"./cook", NULL};
    for (int i = 0; i < N; i++){
        if (fork() == 0)
            execvp(cook_args[0], cook_args);
    }

    char *deliverer_args[] = {"./deliverer", NULL};
    for (int i = 0; i < M; i++){
        if (fork() == 0)
            exec(deliverer_args[0], deliverer_args);
    }
    
    while (wait(NULL) != -1);
    */

    if (semctl(semid, 0, IPC_RMID, NULL) == -1){
        printf("Can't delete semaphore!\n");
        exit(1);
    }

    return 0;
}