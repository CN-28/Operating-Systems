#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#define TABLE_MAX 5

int *table;
sem_t *t_semid, *t_block, *t_pickup; 

void get_shared_data(int *t_fd){
    if ((*t_fd = shm_open("table", O_RDWR, 0)) == -1){
        printf("Can't get oven array from shared memory segment!\n");
        exit(1);
    }

    if ((table = mmap(NULL, TABLE_MAX * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *t_fd, 0)) == (void *) -1){
        printf("Can't attach the segment to cook data space!\n");
        exit(1);
    }
}

void get_table_semaphore(){
    if ((t_semid = sem_open("/t_sem", 0)) == SEM_FAILED){
        printf("Can't get semaphore id!\n");
        exit(1);
    }

    if ((t_block = sem_open("/t_block", 0)) == SEM_FAILED){
        printf("Can't get semaphore id!\n");
        exit(1);
    }

    if ((t_pickup = sem_open("/t_pickup", 0)) == SEM_FAILED){
        printf("Can't get semaphore id!\n");
        exit(1);
    }
}

void handle_delivery(){

    srand(getpid());
    int table_id, pizza_type;
    float delivery_time, comeback_time;
    struct timespec ts, wait_time;
    char buff[100];
    setbuf(stdout, NULL);
    while(1){
        // lock table
        sem_wait(t_pickup);
        sem_wait(t_block);
        sem_post(t_semid);   
        table_id = 0;
        while (table[table_id] == -1)
            table_id = (table_id + 1) % TABLE_MAX;
        
        pizza_type = table[table_id];
        table[table_id] = -1;
        delivery_time = (rand() / (double) RAND_MAX) + 4;
        timespec_get(&ts, TIME_UTC);
        strftime(buff, sizeof buff, "%T", localtime(&ts.tv_sec));
        int curr_avail;
        sem_getvalue(t_semid, &curr_avail);
        printf("Deliverer with pid %ld is taking pizza number: %d.\nNumber of pizzas on the table: %d\nTime: %s.%ld\nDelivering pizza...\n\n", (long) getpid(), pizza_type, TABLE_MAX - curr_avail, buff, ts.tv_nsec / 1000000);
        // unlock table
        sem_post(t_block);

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

    int t_fd;
    get_shared_data(&t_fd);
    
    get_table_semaphore();

    handle_delivery();

    munmap(table, TABLE_MAX * sizeof(int));
    sem_close(t_semid);
    sem_close(t_block);
    sem_close(t_pickup);

    return 0;
}