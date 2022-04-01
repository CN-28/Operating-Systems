#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>


void checkArgs(int argc, char *argv[]){
    if (argc != 2) {
        printf("Wrong number of arguments!\n");
        exit(1);
    }

    if (argv[1][0] == '-'){
        printf("Given number mustn't be negative!\n");
        exit(1);
    }
    else {
        int length = strlen(argv[1]);
        for (int i = 0; i < length; i++){
            if (!isdigit(argv[1][i])){
                printf("Given input is not a natural number!\n");
                exit(1);
            }
        }
    }
}


int main(int argc, char *argv[]) {
    checkArgs(argc, argv);
    
    long n = strtol(argv[1], NULL, 10);
    for (int i = 0; i < n; i++){
        if (fork() == 0){
            printf("Process PID: %d, Parent process PID: %d\n", getpid(), getppid());
            exit(0);
        }
    }
        
    return 0;
}