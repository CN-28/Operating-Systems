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

void get_shared_data(int *o_shmid, int *t_shmid){
    // get oven array from shared memory
    key_t o_key;
    if ((o_key = ftok("cook.c", 'O')) == -1){
        printf("Can't get the key of oven array!\n");
        exit(1);
    }

    if ((*o_shmid = shmget(o_key, OVEN_MAX * sizeof(int), 0)) == -1){
        printf("Can't get oven array from shared memory segment!\n");
        exit(1);
    }

    if ((oven = shmat(*o_shmid, NULL, 0)) == (int *) -1){
        printf("Can't attach the segment to cook data space!\n");
        exit(1);
    }

    // get table array from shared memory
    key_t t_key;
    if ((t_key = ftok("deliverer.c", 'T')) == -1){
        printf("Can't get the key of table array!\n");
        exit(1);
    }

    if ((*t_shmid = shmget(t_key, TABLE_MAX * sizeof(int), IPC_CREAT | 0666)) == -1){
        printf("Can't get table array from shared memory segment!\n");
        exit(1);
    }

    if ((table = shmat(*t_shmid, NULL, 0)) == (int *) -1){
        printf("Can't attach the segment to cook data space!\n");
        exit(1);
    }
}

void get_semaphores(int *o_semid, int *t_semid){
    // get semaphore for oven
    key_t so_key;
    if ((so_key = ftok("pizzeria.c", 'O')) == -1){
        printf("Can't get the key for semaphore!\n");
        exit(1);
    }

    if ((*o_semid = semget(so_key, 0, 0)) == -1){
        printf("Can't get semaphore id!\n");
        exit(1);
    }
    
    // get semaphore for table
    key_t st_key;
    if ((st_key = ftok("pizzeria.c", 'T')) == -1){
        printf("Can't get the key for semaphore!\n");
        exit(1);
    }

    if ((*t_semid = semget(st_key, 0, 0)) == -1){
        printf("Can't get semaphore id!\n");
        exit(1);
    }
}

void prepare_pizza(int pizza_type){
    // preparing pizza
    struct timespec ts, wait_time;
    float prep_time = (rand() / (double) RAND_MAX) + 1;
    timespec_get(&ts, TIME_UTC);
    char buff[100];
    strftime(buff, sizeof (buff), "%T", localtime(&ts.tv_sec));
    printf("Cook with pid %ld is preparing pizza number: %d.\nPreparation estimated time: %f s.\nTime: %s.%ld\nPreparing pizza...\n\n", (long) getpid(), pizza_type, prep_time, buff, ts.tv_nsec / 1000000);
    wait_time.tv_sec = 1;
    wait_time.tv_nsec = (long) ((prep_time - 1) * 1e9);
    nanosleep(&wait_time, NULL);
}

void bake_pizza(int o_semid, int t_semid, int pizza_type, struct sembuf sb[], int *pizza_ind){
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } arg;
    
    // placing in the oven
    sb[0].sem_num = 0;
    sb[0].sem_op = -1;
    sb[1].sem_num = 1;
    sb[1].sem_op = -1;
    arg.val = (semctl(o_semid, 2, GETVAL, NULL) + 1) % OVEN_MAX;
    semctl(o_semid, 2, SETVAL, arg);
    if (semop(o_semid, sb, 2) == -1){
        printf("Can't execute operation on semaphore!\n");
        exit(EXIT_FAILURE);
    }
    *pizza_ind = semctl(o_semid, 2, GETVAL, NULL);
    while (oven[*pizza_ind] != -1)
        *pizza_ind = (*pizza_ind + 1) % OVEN_MAX;

    oven[*pizza_ind] = pizza_type;
    printf("Cook with pid %ld added pizza number: %d to the oven!\n", (long) getpid(), pizza_type);
    printf("Number of pizzas currently in the oven: %d\n", OVEN_MAX - semctl(o_semid, 0, GETVAL, NULL));
    struct timespec ts, wait_time;
    char buff[100];
    timespec_get(&ts, TIME_UTC);
    strftime(buff, sizeof buff, "%T", localtime(&ts.tv_sec));
    printf("Time: %s.%ld\n", buff, ts.tv_nsec / 1000000);
    float bake_time = (rand() / (double) RAND_MAX) + 4;
    printf("Baking estimated time: %f s.\n", bake_time);
    printf("Baking pizza...\n\n");

    sb[0].sem_num = 1;
    sb[0].sem_op = 1;
    if (semop(o_semid, sb, 1) == -1){
        printf("Can't execute operation on semaphore!\n");
        exit(EXIT_FAILURE);
    }

    wait_time.tv_sec = 4;
    wait_time.tv_nsec = (long) ((bake_time - 4) * 1e9);
    nanosleep(&wait_time, NULL);
}

void take_out_pizza(int o_semid, int t_semid, int pizza_type, int pizza_ind, struct sembuf sb[]){
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } arg;
    sb[0].sem_num = 0;
    sb[0].sem_op = 1;
    sb[1].sem_num = 1;
    sb[1].sem_op = -1;
    if (semop(o_semid, sb, 2) == -1){
        printf("Can't execute operation on semaphore!\n");
        exit(EXIT_FAILURE);
    }

    oven[pizza_ind] = -1;
    
    // lock table semaphore
    sb[0].sem_num = 0;
    sb[0].sem_op = -1;
    sb[1].sem_num = 1;
    sb[1].sem_op = -1;
    if (semop(t_semid, sb, 2) == -1){
        printf("Can't execute operation on semaphore!\n");
        exit(EXIT_FAILURE);
    }

    int table_id = semctl(t_semid, 2, GETVAL, NULL);
    while(table[table_id] != -1)
        table_id = (table_id + 1) % TABLE_MAX;
    
    table[table_id] = pizza_type;
    arg.val = table_id;
    semctl(t_semid, 2, SETVAL, arg);

    printf("Cook with pid %ld took out pizza number: %d from the oven and placed on the table!\n", (long) getpid(), pizza_type);
    struct timespec ts;
    char buff[100];
    timespec_get(&ts, TIME_UTC);
    strftime(buff, sizeof buff, "%T", localtime(&ts.tv_sec));
    printf("Time: %s.%ld\n", buff, ts.tv_nsec / 1000000);
    printf("Number of pizzas in the oven: %d\n", OVEN_MAX - semctl(o_semid, 0, GETVAL, NULL));
    printf("Number of pizzas on the table: %d\n\n", TABLE_MAX - semctl(t_semid, 0, GETVAL, NULL));

    // unlock table semaphore
    sb[0].sem_num = 1;
    sb[0].sem_op = 1;
    sb[1].sem_num = 3;
    sb[1].sem_op = 1;
    if (semop(t_semid, sb, 2) == -1){
        printf("Can't execute operation on semaphore!\n");
        exit(EXIT_FAILURE);
    }

    sb[0].sem_num = 1;
    sb[0].sem_op = 1;
    if (semop(o_semid, sb, 1) == -1){
        printf("Can't execute operation on semaphore!\n");
        exit(EXIT_FAILURE);
    }
}

void handle_baking(int o_semid, int t_semid){
    // structure for performing operations on semaphores
    struct sembuf sb[3];
    for (int i = 0; i < 3; i++)
        sb[i].sem_flg = 0;
    
    srand(getpid());
    int pizza_type, pizza_ind;
    setbuf(stdout, NULL);
    while(1){
        // drawing type of pizza
        pizza_type = rand() % 10;

        prepare_pizza(pizza_type);
        
        bake_pizza(o_semid, t_semid, pizza_type, sb, &pizza_ind);

        take_out_pizza(o_semid, t_semid, pizza_type, pizza_ind, sb);
    }
}

void detach_shared_memory(){
    if (shmdt(table) == -1){
        printf("Can't detach shared table array!\n");
        exit(1);
    }

    if (shmdt(oven) == -1){
        printf("Can't detach shared oven array!\n");
        exit(1);
    }
}


int main(){
    int o_shmid, t_shmid;
    get_shared_data(&o_shmid, &t_shmid);

    int o_semid, t_semid;
    get_semaphores(&o_semid, &t_semid);

    handle_baking(o_semid, t_semid);

    detach_shared_memory();

    return 0;
}