#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define KILL "kill"
#define SIGQUEUE "sigqueue"
#define SIGRT "sigrt"


int main(int argc, char *argv[]){
    if (argc != 4){
        printf("Wrong number of arguments!\n");
        exit(1);
    }
    long catcher_pid = strtol(argv[1], NULL, 10);
    long n = strtol(argv[2], NULL, 10);
    char *mode = argv[3];
    
    if (strcmp(mode, KILL) && strcmp(mode, SIGQUEUE) && strcmp(mode, SIGRT)){
        printf("Provided mode is incorrect!\n");
        exit(1);
    }


    // send n SIGUSR1 signals to catcher and next SIGUSR2 signal
    if (!strcmp(mode, KILL)){
        for (int i = 0; i < n; i++)
            kill(catcher_pid, SIGUSR1);
        kill(catcher_pid, SIGUSR2);
    }
    


    return 0;
}