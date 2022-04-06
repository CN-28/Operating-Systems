#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define IGNORE "ignore"
#define MASK "mask"
#define PENDING "pending"


void checkIfPending(int signo){
    sigset_t pending_signals;
    sigpending(&pending_signals);
    printf("Is SIGUSR1 pending in the process? %s\n", sigismember(&pending_signals, signo) ? "YES" : "NO");
}


int main(int argc, char* argv[]){
    if (argc != 2){
        printf("Wrong number of arguments!\n");
        exit(1);
    }

    char *input = argv[1];
    if (strcmp(input, IGNORE) && strcmp(input, MASK) && strcmp(input, PENDING)){
        printf("Given argument is incorrect!\n");
        exit(1);
    }

    int signo = SIGUSR1;
    if (!strcmp(input, IGNORE))
        signal(signo, SIG_IGN);
    else{
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, signo);
        sigprocmask(SIG_BLOCK, &mask, NULL); 
    }
    raise(signo);
    if (!strcmp(input, IGNORE))
        printf("The signal was ignored!\n");
    else if (!strcmp(input, MASK))
        printf("The signal was masked!\n");
    else if (!strcmp(input, PENDING))
        checkIfPending(signo);


    execl("./test_exec", "./test_exec", argv[1], NULL);


    return 0;
}