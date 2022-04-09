#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define KILL "kill"
#define SIGQUEUE "sigqueue"
#define SIGRT "sigrt"

int sig1 = SIGUSR1;
int sig2 = SIGUSR2;
int signals_received = 0;
int is_receiving = 1;
pid_t sender_pid;
char mode[256];
void handle_signal(int signum, siginfo_t *info, void *ucontext){
    signals_received++;
    sender_pid = info->si_pid;
    if (!strcmp(mode, SIGQUEUE)){
        union sigval value;
        value.sival_int = signals_received;
        sigqueue(sender_pid, sig1, value);
    }
    else
        kill(sender_pid, sig1);
}


void handle_stop_signal(int signum, siginfo_t *info, void *ucontext){
    sender_pid = info->si_pid;
    if (!strcmp(mode, SIGQUEUE)){
        union sigval value;
        value.sival_int = signals_received;
        sigqueue(sender_pid, sig2, value);
    }
    else
        kill(sender_pid, sig2);
    
    is_receiving = 0;
}


int main(int argc, char *argv[]){
    if (argc != 2){
        printf("Wrong number of arguments!\n");
        exit(0);
    }

    if (strcmp(argv[1], KILL) && strcmp(argv[1], SIGQUEUE) && strcmp(argv[1], SIGRT)){
        printf("Provided mode is incorrect!\n");
        exit(0);
    }

    strcpy(mode, argv[1]);
    printf("Catcher process PID: %d\n", getpid());

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
    
    while (is_receiving)
        sigsuspend(&mask);

    
    printf("Signals received: %d\n", signals_received);

    return 0;
}