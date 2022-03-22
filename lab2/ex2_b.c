// system libraries functions version
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#define MAX 256


void countOccurences(int fd, int *cnt, int *lineCnt, char *ch) {
    char buffor[MAX];
    int charsRead = 0, containsChar = 0;
    while(1) {
        charsRead = read(fd, buffor, MAX);
        for (int i = 0; i < charsRead; i++) {
            if (buffor[i] == *ch){
                (*cnt)++;
                containsChar = 1;
            }
            else if (buffor[i] == '\n'){
                if (containsChar)
                    (*lineCnt)++;
                containsChar = 0;
            }
        }
        if (charsRead != MAX){
            if (containsChar)
                (*lineCnt)++;
            break;
        }
    }
}


int main(int argc, char *argv[]) {
    clock_t t;
    t = clock();

    char *fileName, *ch;
    if (argc != 3) {
        printf("Wrong number of arguments!\n");
        exit(1);
    }
    else{
        ch = argv[1];
        fileName = argv[2];
        if (strlen(ch) != 1){
            printf("Given input is not a character!\n");
            exit(1);
        }
    }

    int fd = open(fileName, O_RDONLY);
    int cnt = 0, lineCnt = 0; 
    countOccurences(fd, &cnt, &lineCnt, ch);
    printf("Number of given character occurences in the whole file: %d\nNumber of lines, which contain given character: %d\n", cnt, lineCnt);

    close(fd);

    t = clock() - t;
    double time_taken = ((double) t) / CLOCKS_PER_SEC;
    int timeFd = open("pomiar_zad_2", O_WRONLY | O_APPEND | O_CREAT, 0666);
    
    char res[100];
    snprintf(res, 100, "System libraries functions version took %f seconds\n", time_taken);
    write(timeFd, res, strlen(res));
    close(timeFd);
    return 0;
}