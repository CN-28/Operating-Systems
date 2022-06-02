#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int reindeers_home = 0, elves_waiting = 0;
pthread_mutex_t r_mutex = PTHREAD_MUTEX_INITIALIZER, e_mutex = PTHREAD_MUTEX_INITIALIZER, ready_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t r_cond = PTHREAD_COND_INITIALIZER, e_cond = PTHREAD_COND_INITIALIZER, ready_cond = PTHREAD_COND_INITIALIZER;
pthread_t santa_id, elves_id[10], reindeers_id[9];
int cnt = 0, terminated = 0;

void *handle_santa(void *ptr){
    unsigned int thread_id = (unsigned int) pthread_self();
    float time;

    while (1){
        
        pthread_mutex_lock(&ready_mutex);
        while (elves_waiting < 3 && reindeers_home < 9)
            pthread_cond_wait(&ready_cond, &ready_mutex);
        
        printf("\nSanta woke up!\n");
        pthread_mutex_unlock(&ready_mutex);
        
        pthread_mutex_lock(&r_mutex);
        
        if (reindeers_home == 9){
            time = (float) rand_r(&thread_id) / (float) (RAND_MAX / 2) + 2;
            printf("Delivering gifts! Estimated time: %f s.\n", time);
            sleep(time);
            printf("The gifts has been delivered\n");
            reindeers_home = 0;
            cnt++;
            printf("\n\n\nGifts were delivered %d times!!!\n\n\n", cnt);
            pthread_cond_broadcast(&r_cond);
        }

        pthread_mutex_unlock(&r_mutex);

        if (cnt == 3)
            break; 
        
        pthread_mutex_lock(&e_mutex);
     
        if (elves_waiting == 3){
            time = (float) rand_r(&thread_id) / (float) (RAND_MAX) + 1;
            printf("Solving elves' problems! Estimated time: %f s.\n", time);
            sleep(time);
            elves_waiting = 0;
            pthread_cond_broadcast(&e_cond);
        }

        pthread_mutex_unlock(&e_mutex);
        printf("Going back to sleep!\n\n");
    }

    // operations for unlocking all mutexes in threads
    terminated = 1;
    pthread_cond_broadcast(&e_cond);
    pthread_cond_broadcast(&r_cond);
    return NULL;
}

void *handle_rainder(void *ptr){
    unsigned int thread_id = (unsigned int) pthread_self();
    float time;
    while(cnt < 3){
        // random sleep from 5-10s
        time = (float) rand_r(&thread_id) / (float) (RAND_MAX / 5) + 5;
        sleep(time);

        // coming back to north pole
        pthread_mutex_lock(&r_mutex);
        reindeers_home++;
        printf("Reindeers waiting: %d, ID: %u\n", reindeers_home, thread_id);
        if (reindeers_home == 9){
            pthread_mutex_lock(&ready_mutex);
            printf("Reindeer: I'm waking up Santa, ID: %u\n", thread_id);
            pthread_cond_broadcast(&ready_cond);
            pthread_mutex_unlock(&ready_mutex);
        }
        
        while (!terminated && reindeers_home > 0)
            pthread_cond_wait(&r_cond, &r_mutex);

        pthread_mutex_unlock(&r_mutex);
    }
    return NULL;
}

void *handle_elf(void *ptr){
    unsigned int thread_id = (unsigned int) pthread_self();
    float time;
    while (cnt < 3){
        time = (float) rand_r(&thread_id) / (float) (RAND_MAX / 3) + 2;
        sleep(time);

        pthread_mutex_lock(&e_mutex);

        if (!terminated && elves_waiting == 3) printf("Elf: waiting for other elves to come back, ID: %u\n", thread_id);

        while (!terminated && elves_waiting == 3)
            pthread_cond_wait(&e_cond, &e_mutex);
        
        elves_waiting++;
        if (!terminated) printf("Elves waiting: %d, ID: %u\n", elves_waiting, thread_id);
        if (elves_waiting == 3){
            pthread_mutex_lock(&ready_mutex);
            printf("Elf: waking up Santa, ID: %u\n", thread_id);
            pthread_cond_broadcast(&ready_cond);
            pthread_mutex_unlock(&ready_mutex);
        }
        pthread_mutex_unlock(&e_mutex);
    }
    return NULL;
}


int main(){
    
    pthread_create(&santa_id, NULL, handle_santa, NULL);
    for (int i = 0; i < 10; i++){
        if (i < 9) pthread_create(&reindeers_id[i], NULL, handle_rainder, NULL);
        pthread_create(&elves_id[i], NULL, handle_elf, NULL);
    }
    
    pthread_join(santa_id, NULL);
    for (int i = 0; i < 10; i++){
        if (i < 9) pthread_join(reindeers_id[i], NULL);
        pthread_join(elves_id[i], NULL);
    }

    return 0;
}