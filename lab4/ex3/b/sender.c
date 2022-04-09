#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define KILL "kill"
#define SIGQUEUE "sigqueue"
#define SIGRT "sigrt"

int sig1 = SIGUSR1;
int sig2 = SIGUSR2;
int signals_received = 0;
long n;
void handle_signal(int signum, siginfo_t *info, void *ucontext){
    signals_received++;
}

int is_sigqueue = 0;
void handle_stop_signal(int signum, siginfo_t *info, void *ucontext){
    if (is_sigqueue)
        printf("Catcher received %d signals\n", info->si_value.sival_int);
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

    if (!strcmp(mode, SIGRT)){
        sig1 = SIGRTMIN + 1;
        sig2 = SIGRTMIN + 2;
    }

    struct sigaction act1;
    act1.sa_sigaction = handle_signal;
    act1.sa_flags = SA_SIGINFO;
    sigemptyset(&act1.sa_mask);
    sigaction(sig1, &act1, NULL);

    struct sigaction act2;
    act2.sa_sigaction = handle_stop_signal;
    act2.sa_flags = SA_SIGINFO;
    sigemptyset(&act2.sa_mask);
    sigaction(sig2, &act2, NULL);

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, sig1);
    sigdelset(&mask, sig2);

    if (!strcmp(mode, KILL) || !strcmp(mode, SIGRT)){
        for (int i = 0; i < n; i++){
            kill(catcher_pid, sig1);
            sigsuspend(&mask);
        }
        kill(catcher_pid, sig2);
        sigsuspend(&mask);
    }
    else{
        is_sigqueue = 1;
        union sigval value;
        for (int i = 0; i < n; i++){
            value.sival_int = i + 1;
            sigqueue(catcher_pid, SIGUSR1, value);
            sigsuspend(&mask);
        }
        sigqueue(catcher_pid, SIGUSR2, value);
        sigsuspend(&mask);
    }
    
    
    printf("Sender got back %d signal, but should get back %ld signals.\n", signals_received, n);

    return 0;
}