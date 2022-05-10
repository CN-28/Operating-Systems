#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define TABLE_MAX 5
#define OVEN_MAX 5

int *table, *oven;

void check_input(int argc, char *argv[], int *N, int *M){
    if (argc != 3){
        printf("Wrong number of arguments!\n");
        exit(1);
    }

    *N = strtol(argv[1], NULL, 10);
    if (*N <= 0){
        printf("Incorrect argument provided for N!\n");
        exit(1);
    }

    *M = strtol(argv[2], NULL, 10);
    if (*M <= 0){
        printf("Incorrect argument provided for M!\n");
        exit(1);
    }
}

void create_memory_keys(key_t *t_key, key_t *o_key){
    if ((*t_key = ftok("deliverer.c", 'T')) == -1){
        printf("Can't create a key for table array!\n");
        exit(1);
    }

    if ((*o_key = ftok("cook.c", 'O')) == -1){
        printf("Can't create a key for oven array!\n");
        exit(1);
    }
}

void allocate_shared_memory(int *o_shmid, int *t_shmid, int o_key, int t_key){
    if ((*o_shmid = shmget(o_key, OVEN_MAX * sizeof(int), IPC_CREAT | 0666)) == -1){
        printf("Can't allocate a shared memory segment for oven array!\n");
        exit(1);
    }

    // attach oven array to shared memory
    if ((oven = (int *) shmat(*o_shmid, NULL, 0)) == (int *) -1){
        printf("Can't attach shared memory segment for oven array!\n");
        exit(1);
    }

    // initialize empty oven
    for (int i = 0; i < OVEN_MAX; i++)
        oven[i] = -1;
      
    if ((*t_shmid = shmget(t_key, TABLE_MAX * sizeof(int), IPC_CREAT | 0666)) == -1){
        printf("Can't allocate a shared memory segment for table array!\n");
        exit(1);
    }

    // attach table array to shared memory
    if ((table = (int *) shmat(*t_shmid, NULL, 0)) == (int *) -1){
        printf("Can't attach shared memory segment for table array!\n");
        exit(1);
    }

    // initialize empty table
    for (int i = 0; i < TABLE_MAX; i++)
        table[i] = -1;
}

void create_semaphores(int *o_semid, int *t_semid){
    key_t so_key, st_key;
    if ((so_key = ftok("pizzeria.c", 'O')) == -1){
        printf("Can't create a key for semaphore!\n");
        exit(1);
    }
    
    if ((st_key = ftok("pizzeria.c", 'T')) == -1){
        printf("Can't create a key for semaphore!\n");
        exit(1);
    }
    
    // create 3 semaphores
    if ((*o_semid = semget(so_key, 3, IPC_CREAT | 0666)) == -1){
        printf("Can't create semaphore!\n");
        exit(1);
    }    
    
    if ((*t_semid = semget(st_key, 4, IPC_CREAT | 0666)) == -1){
        printf("Can't create semaphore!\n");
        exit(1);
    }

    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } arg;
    arg.val = OVEN_MAX;

    // set all semaphores' values for oven
    for (int i = 0; i < 3; i++){
        if (i == 1) arg.val = 1;
        else if (i == 2) arg.val = 0;
        if (semctl(*o_semid, i, SETVAL, arg) == -1){
            printf("Can't initialize semaphores values!\n");
            exit(1);
        }
    }
    
    // set all semaphores' values for table
    arg.val = TABLE_MAX;
    for (int i = 0; i < 4; i++){
        if (i == 1) arg.val = 1;
        else if (i == 2) arg.val = 0;
        if (semctl(*t_semid, i, SETVAL, arg) == -1){
            printf("Can't initialize semaphores values!\n");
            exit(1);
        }
    }
}

void handle_pizzeria(int N, int M){
    char *cook_args[] = {"./cook", NULL};
    for (int i = 0; i < N; i++){
        if (fork() == 0)
            execvp(cook_args[0], cook_args);
    }

    char *deliverer_args[] = {"./deliverer", NULL};
    for (int i = 0; i < M; i++){
        if (fork() == 0)
            execvp(deliverer_args[0], deliverer_args);
    }
    while (wait(NULL) != -1);
}

void free_at_exit(int o_semid, int t_semid, int o_shmid, int t_shmid){
    if (semctl(o_semid, 0, IPC_RMID, NULL) == -1){
        printf("Can't delete semaphore!\n");
        exit(1);
    }

    if (semctl(t_semid, 0, IPC_RMID, NULL) == -1){
        printf("Can't delete semaphore!\n");
        exit(1);
    }

    if (shmdt(table) == -1){
        printf("Can't detach shared table array!\n");
        exit(1);
    }

    if (shmdt(oven) == -1){
        printf("Can't detach shared oven array!\n");
        exit(1);
    }
    
    if (shmctl(o_shmid, IPC_RMID, NULL) == -1){
        printf("Can't destroy the segment of shared memory!\n");
        exit(1);
    }

    if (shmctl(t_shmid, IPC_RMID, NULL) == -1){
        printf("Can't destroy the segment of shared memory!\n");
        exit(1);
    }
}


int main(int argc, char *argv[]){
    int N, M;
    check_input(argc, argv, &N, &M);

    // create keys for shared memory segment of table array
    key_t t_key, o_key;
    create_memory_keys(&t_key, &o_key);

    // allocate a shared memory segment for table and oven arrays
    int o_shmid, t_shmid;
    allocate_shared_memory(&o_shmid, &t_shmid, o_key, t_key);

    int o_semid, t_semid;
    create_semaphores(&o_semid, &t_semid);

    handle_pizzeria(N, M);

    free_at_exit(o_semid, t_semid, o_shmid, t_shmid);

    return 0;
}