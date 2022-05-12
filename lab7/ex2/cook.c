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
#define OVEN_MAX 5
#define TABLE_MAX 5

int *table, *oven;
sem_t *o_semid, *t_semid, *o_block, *t_block, *t_pickup;

void get_shared_data(int *o_fd, int *t_fd){
    // get oven array from shared memory
    if ((*o_fd = shm_open("oven", O_RDWR, 0)) == -1){
        printf("Can't get oven array from shared memory segment!\n");
        exit(1);
    }

    if ((oven = mmap(NULL, OVEN_MAX * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *o_fd, 0)) == (void *) -1){
        printf("Can't attach the segment to cook data space!\n");
        exit(1);
    }

    if ((*t_fd = shm_open("table", O_RDWR, 0)) == -1){
        printf("Can't get oven array from shared memory segment!\n");
        exit(1);
    }

    if ((table = mmap(NULL, TABLE_MAX * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *t_fd, 0)) == (void *) -1){
        printf("Can't attach the segment to cook data space!\n");
        exit(1);
    }
}


void get_semaphores(){
    // get semaphore for oven
    if ((o_semid = sem_open("/o_sem", 0)) == SEM_FAILED){
        printf("Can't get semaphore id!\n");
        exit(1);
    }

    if ((t_semid = sem_open("/t_sem", 0)) == SEM_FAILED){
        printf("Can't get semaphore id!\n");
        exit(1);
    }

    if ((o_block = sem_open("/o_block", 0)) == SEM_FAILED){
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

void bake_pizza(int pizza_type, int *pizza_ind){
    // placing in the oven

    sem_wait(o_semid);
    sem_wait(o_block);

    *pizza_ind = 0;
    while (oven[*pizza_ind] != -1)
        *pizza_ind = (*pizza_ind + 1) % OVEN_MAX;

    oven[*pizza_ind] = pizza_type;
    printf("Cook with pid %ld added pizza number: %d to the oven!\n", (long) getpid(), pizza_type);
    int curr_avail;
    sem_getvalue(o_semid, &curr_avail);
    printf("Number of pizzas currently in the oven: %d\n", OVEN_MAX - curr_avail);
    struct timespec ts, wait_time;
    char buff[100];
    timespec_get(&ts, TIME_UTC);
    strftime(buff, sizeof buff, "%T", localtime(&ts.tv_sec));
    printf("Time: %s.%ld\n", buff, ts.tv_nsec / 1000000);
    float bake_time = (rand() / (double) RAND_MAX) + 4;
    printf("Baking estimated time: %f s.\n", bake_time);
    printf("Baking pizza...\n\n");

    sem_post(o_block);
    wait_time.tv_sec = 4;
    wait_time.tv_nsec = (long) ((bake_time - 4) * 1e9);
    nanosleep(&wait_time, NULL);
}

void take_out_pizza(int pizza_type, int pizza_ind){
    sem_wait(o_block);
    oven[pizza_ind] = -1;
    
    sem_post(o_semid);
    sem_wait(t_semid);
    sem_wait(t_block);

    int table_id = 0;
    while(table[table_id] != -1)
        table_id = (table_id + 1) % TABLE_MAX;
    
    table[table_id] = pizza_type;

    printf("Cook with pid %ld took out pizza number: %d from the oven and placed on the table!\n", (long) getpid(), pizza_type);
    struct timespec ts;
    char buff[100];
    timespec_get(&ts, TIME_UTC);
    strftime(buff, sizeof buff, "%T", localtime(&ts.tv_sec));
    printf("Time: %s.%ld\n", buff, ts.tv_nsec / 1000000);
    int curr_av_o, curr_av_t;
    sem_getvalue(o_semid, &curr_av_o);
    sem_getvalue(t_semid, &curr_av_t);
    printf("Number of pizzas in the oven: %d\n", OVEN_MAX - curr_av_o);
    printf("Number of pizzas on the table: %d\n\n", TABLE_MAX - curr_av_t);

    sem_post(t_pickup);
    sem_post(t_block);
    sem_post(o_block);
}

void handle_baking(){
    // structure for performing operations on semaphores
    
    srand(getpid());
    int pizza_type, pizza_ind;
    setbuf(stdout, NULL);
    while(1){
        // drawing type of pizza
        pizza_type = rand() % 10;

        prepare_pizza(pizza_type);
        
        bake_pizza(pizza_type, &pizza_ind);

        take_out_pizza(pizza_type, pizza_ind);
    }
}

void detach_shared_memory(){
    munmap(table, TABLE_MAX * sizeof(int));
    munmap(oven, OVEN_MAX * sizeof(int));
    sem_close(o_semid);
    sem_close(t_semid);
    sem_close(o_block);
    sem_close(t_block);
    sem_close(t_pickup);
}


int main(){
    int o_fd, t_fd;
    get_shared_data(&o_fd, &t_fd);
    
    get_semaphores();

    handle_baking();

    detach_shared_memory();

    return 0;
}