#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define KILL "kill"
#define SIGQUEUE "sigqueue"
#define SIGRT "sigrt"

int signals_received = 0;
long n;
void handle_sigusr1(int signum, siginfo_t *info, void *ucontext){
    signals_received++;
}

void handle_sigusr2(int signum, siginfo_t *info, void *ucontext){
    printf("Sender got back %d signal, but should get back %ld signals.\n", signals_received, n);
}


int main(int argc, char *argv[]){
    if (argc != 4){
        printf("Wrong number of arguments!\n");
        exit(1);
    }
    long catcher_pid = strtol(argv[1], NULL, 10);
    n = strtol(argv[2], NULL, 10);
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
    
    struct sigaction act1;
    act1.sa_sigaction = handle_sigusr1;
    act1.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act1, NULL);

    struct sigaction act2;
    act2.sa_sigaction = handle_sigusr2;
    act2.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR2, &act2, NULL);

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    
    sigsuspend(&mask);


    return 0;
}