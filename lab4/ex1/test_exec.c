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


int main(int argc, char *argv[]){
    char *input = argv[1];

    int signo = SIGUSR1;
    if (strcmp(input, PENDING)){
        raise(signo);
        if (!strcmp(input, IGNORE))
            printf("The signal was ignored!\n");
        else if (!strcmp(input, MASK))
            printf("The signal was masked!\n");
    }       
    else
        checkIfPending(signo);

    return 0;
}