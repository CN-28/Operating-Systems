#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


int signals_received = 0;
pid_t sender_pid;

void handle_sigusr1(int signum, siginfo_t *info, void *ucontext){
    signals_received++;
}


void handle_sigusr2(int signum, siginfo_t *info, void *ucontext){
    sender_pid = info->si_pid;
    printf("Signals received: %d\n", signals_received);
}


int main(){

    printf("Catcher process PID: %d\n", getpid());

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

    for (int i = 0; i < signals_received; i++)
        kill(sender_pid, SIGUSR1);
    kill(sender_pid, SIGUSR2);


    return 0;
}