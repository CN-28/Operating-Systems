#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#define TABLE_MAX 5
#define OVEN_MAX 5

int *table, *oven;
sem_t *o_semid, *t_semid, *o_block, *t_block, *t_pickup;

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

void allocate_shared_memory(int *o_fd, int *t_fd){
    if ((*o_fd = shm_open("oven", O_CREAT | O_RDWR, 0666)) == -1){
        printf("Can't create a shared memory segment for oven array!\n");
        exit(1);
    }

    if (ftruncate(*o_fd, OVEN_MAX * sizeof(int)) == -1) {
        printf("Can't set memory object size!\n");
        exit(1);
    }

    if ((oven = mmap(NULL, OVEN_MAX * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *o_fd, 0)) == (void *) -1){
        printf("Can't map oven array into memory!\n");
        exit(1);
    }

    // initialize empty oven
    for (int i = 0; i < OVEN_MAX; i++)
        oven[i] = -1;
      

    if ((*t_fd = shm_open("table", O_CREAT | O_RDWR, 0666)) == -1){
        printf("Can't create a shared memory segment for oven array!\n");
        exit(1);
    }

    if (ftruncate(*t_fd, TABLE_MAX * sizeof(int)) == -1) {
        printf("Can't set memory object size!\n");
        exit(1);
    }

    // attach table array to shared memory
    if ((table = mmap(NULL, TABLE_MAX * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *t_fd, 0)) == (void *) -1){
        printf("Can't map table array into memory!\n");
        exit(1);
    }

    // initialize empty table
    for (int i = 0; i < TABLE_MAX; i++)
        table[i] = -1;
}


void create_semaphores(){
 
    if ((o_semid = sem_open("/o_sem", O_CREAT, 0666, OVEN_MAX)) == SEM_FAILED){
        printf("Can't create semaphore!\n");
        exit(1);
    }
    
    if ((t_semid = sem_open("/t_sem", O_CREAT, 0666, TABLE_MAX)) == SEM_FAILED){
        printf("Can't create semaphore!\n");
        exit(1);
    }

    if ((o_block = sem_open("/o_block", O_CREAT, 0666, 1)) == SEM_FAILED){
        printf("Can't create semaphore!\n");
        exit(1);
    }

    if ((t_block = sem_open("/t_block", O_CREAT, 0666, 1)) == SEM_FAILED){
        printf("Can't create semaphore!\n");
        exit(1);
    }

    if ((t_pickup = sem_open("/t_pickup", O_CREAT, 0666, 0)) == SEM_FAILED){
        printf("Can't create semaphore!\n");
        exit(1);
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


void free_at_exit(){
    munmap(table, TABLE_MAX * sizeof(int));
    munmap(oven, OVEN_MAX * sizeof(int));
    shm_unlink("table");
    shm_unlink("oven");
    sem_close(o_semid);
    sem_unlink("o_sem");
    sem_close(t_semid);
    sem_unlink("t_sem");
    sem_close(o_block);
    sem_unlink("o_block");
    sem_close(t_block);
    sem_unlink("t_block");
    sem_close(t_pickup);
    sem_unlink("t_pickup");
}


void signal_handler(int signum){
    free_at_exit();
}

int main(int argc, char *argv[]){
    int N, M;
    check_input(argc, argv, &N, &M);
    
    // allocate a shared memory segment for table and oven arrays
    int o_fd, t_fd;
    allocate_shared_memory(&o_fd, &t_fd);
    
    create_semaphores();
    signal(SIGINT, signal_handler);
    handle_pizzeria(N, M);
    
    free_at_exit();

    return 0;
}