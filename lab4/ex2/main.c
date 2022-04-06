#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>


void handle_sigusr1(int sig, siginfo_t *info, void *ucontext){
    printf("\tSIGUSR1\nSignal number: %d\nSending process ID: %d\nUID of sending process: %d\nExit value or signal: %d\nAn errno value: %d\n\n",\
    info->si_signo, info->si_pid, info->si_uid, info->si_status, info->si_errno);
}


void test_sigusr1(){
    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &handle_sigusr1;
    sigaction(SIGUSR1, &act, NULL);
    raise(SIGUSR1);
}


void handle_sigchld(int sig, siginfo_t *info, void *ucontext){
    printf("\tSIGCHLD\nSignal number: %d\nSending process ID: %d\nUID of sending process: %d\nExit value or signal: %d\nAn errno value: %d\n\n",\
    info->si_signo, info->si_pid, info->si_uid, info->si_status, info->si_errno);
}


void test_sigchld(){
    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &handle_sigchld;
    sigaction(SIGCHLD, &act, NULL);
    
    if (fork() == 0)
        exit(0);
    
    wait(NULL);
}


void handle_sigterm(int sig, siginfo_t *info, void *ucontext){
    printf("\tSIGTERM\nSignal number: %d\nSending process ID: %d\nUID of sending process: %d\nExit value or signal: %d\nAn errno value: %d\n\n",\
    info->si_signo, info->si_pid, info->si_uid, info->si_status, info->si_errno);
}


void test_sigterm(){
    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &handle_sigterm;
    sigaction(SIGTERM, &act, NULL);
    raise(SIGTERM);
}


int main(){

    test_sigusr1();
    test_sigchld();
    test_sigterm();
    return 0;
}