#include <stdio.h>
#include <signal.h>
#include <unistd.h>


int signals_received = 0;
void handle_sigusr1(int signum, siginfo_t *info, void *ucontext){
    signals_received++;
}


void handle_sigusr2(int signum, siginfo_t *info, void *ucontext){
    printf("Signals received: %d\n", signals_received);
}


int main(){

    printf("Catcher process PID: %d\n", getpid());

    struct sigaction act1;
    act1.sa_sigaction = &handle_sigusr1;
    act1.sa_flags = SA_SIGINFO;
    printf("%d\n", sigaction(SIGUSR1, &act1, NULL));

    struct sigaction act2;
    act2.sa_sigaction = &handle_sigusr2;
    act2.sa_flags = SA_SIGINFO;
    printf("%d\n", sigaction(SIGUSR2, &act2, NULL));

    sigset_t mask;
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigsuspend(&mask);

    return 0;
}