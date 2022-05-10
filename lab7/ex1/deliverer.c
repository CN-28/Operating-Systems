#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#define TABLE_MAX 5

int *table;

void get_shared_data(int *t_shmid){
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

void get_table_semaphore(int *t_semid){
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

void handle_delivery(int t_semid){
    // structure for performing operations on semaphores
    struct sembuf sb[3];
    for (int i = 0; i < 3; i++)
        sb[i].sem_flg = 0;
    
    srand(getpid());
    int table_id, pizza_type;
    float delivery_time, comeback_time;
    struct timespec ts, wait_time;
    char buff[100];
    setbuf(stdout, NULL);
    while(1){
        // lock table
        sb[0].sem_num = 0;
        sb[0].sem_op = 1;
        sb[1].sem_num = 1;
        sb[1].sem_op = -1;
        sb[2].sem_num = 3;
        sb[2].sem_op = -1;
        if (semop(t_semid, sb, 3) == -1){
            printf("Can't execute operation on semaphore dd!\n");
            exit(EXIT_FAILURE);
        }

        table_id = semctl(t_semid, 2, GETVAL, NULL);
        while (table[table_id] == -1)
            table_id = (table_id + 1) % TABLE_MAX;
        
        pizza_type = table[table_id];
        table[table_id] = -1;
        delivery_time = (rand() / (double) RAND_MAX) + 4;
        timespec_get(&ts, TIME_UTC);
        strftime(buff, sizeof buff, "%T", localtime(&ts.tv_sec));
        printf("Deliverer with pid %ld is taking pizza number: %d.\nNumber of pizzas on the table: %d\nTime: %s.%ld\nDelivering pizza...\n\n", (long) getpid(), pizza_type, TABLE_MAX - semctl(t_semid, 0, GETVAL, NULL), buff, ts.tv_nsec / 1000000);
        // unlock table
        sb[0].sem_num = 1;
        sb[0].sem_op = 1;
        if (semop(t_semid, sb, 1) == -1){
            printf("Can't execute operation on semaphore!\n");
            exit(EXIT_FAILURE);
        }

        wait_time.tv_sec = 4;
        wait_time.tv_nsec = (long) ((delivery_time - 4) * 1e9);
        nanosleep(&wait_time, NULL);

        timespec_get(&ts, TIME_UTC);
        strftime(buff, sizeof buff, "%T", localtime(&ts.tv_sec));
        printf("Deliverer with pid %ld has delivered pizza number: %d.\nTime: %s.%ld\nDriving back to pizzeria...\n\n", (long) getpid(), pizza_type, buff, ts.tv_nsec / 1000000);
        
        comeback_time = (rand() / (double) RAND_MAX) + 4;
        wait_time.tv_sec = 4;
        wait_time.tv_nsec = (long) ((comeback_time - 4) * 1e9);
        nanosleep(&wait_time, NULL);
        printf("Deliverer with pid %ld got back to pizzeria!\n\n", (long) getpid());
    }
}


int main(){

    int t_shmid;
    get_shared_data(&t_shmid);

    int t_semid; 
    get_table_semaphore(&t_semid);

    handle_delivery(t_semid);

    // detach shared memory
    if (shmdt(table) == -1){
        printf("Can't detach shared memory!\n");
        exit(1);
    }

    return 0;
}