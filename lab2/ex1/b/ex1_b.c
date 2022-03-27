// read(), write() version
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <string.h>


void handleFileCopying(int fd1, int fd2) {
    long size = lseek(fd1, 0L, SEEK_END);
    lseek(fd1, 0L, SEEK_SET);
    
    char *buffor = malloc(20 * sizeof(char));
    char *temp = malloc(20 * sizeof(char));
    int charsRead = 0, bufforSize = 20, isEmpty = 1, last_ws_i, k, tempFilled = 0, temp_i = 0, sumRead = 0;
    while (1) {
        charsRead = read(fd1, buffor, bufforSize);
        sumRead += charsRead;

        k = 0;
        last_ws_i = -1;
        for (int i = 0; i < charsRead; i++) {
            if (isspace(buffor[i])) {
                if (buffor[i] == '\n' || buffor[i] == '\v' || buffor[i] == '\f') {
                    if (!isEmpty) {
                        
                        if (tempFilled)
                            write(fd2, temp, temp_i);
                        
                        for (int j = last_ws_i + 1; j <= i; j++) {
                            buffor[k] = buffor[j];
                            k++;
                        }
                        isEmpty = 1;
                    }
                    
                    temp_i = 0;
                    tempFilled = 0;
                    last_ws_i = i;
                }
            }
            else
                isEmpty = 0;
        }
        
        if (!isEmpty) {
            for (int j = last_ws_i + 1; j < charsRead; j++) {
                buffor[k] = buffor[j];
                k++;
            }
        }
        else {
            for (int j = last_ws_i + 1; j < charsRead; j++){
                temp[temp_i] = buffor[j];
                temp_i++;
            } 
            tempFilled = 1;
        }

        if (k > 0){
            if (sumRead == size && (buffor[k - 1] == '\n' || buffor[k - 1] == '\v' || buffor[k - 1] == '\f'))
                write(fd2, buffor, k - 1);
            else
                write(fd2, buffor, k);
        }

        if (charsRead != bufforSize)
            break;

        bufforSize *= 2;
        buffor = realloc(buffor, bufforSize);
        temp = realloc(temp, sumRead + bufforSize);
    }

    free(buffor);
    free(temp);
}


int openFile(char *input, int flags) {
    int fd = open(input, flags);
    if (fd < 0) {
        printf("Cannot open %s file!\n", input);
        exit(1);
    }
    
    return fd;
}


void getFileNames(int argc, char *argv[], char **input1, char **input2) {
    if (argc != 3) {
        *input1 = malloc(256 * sizeof(char));
        *input2 = malloc(256 * sizeof(char));
        scanf("%s%s", *input1, *input2);
    }
    else {
        *input1 = argv[1];
        *input2 = argv[2];
    }
}


int main(int argc, char *argv[]) {
    clock_t t;
    t = clock();

    char *input1, *input2;
    getFileNames(argc, argv, &input1, &input2);

    int fd1 = openFile(input1, O_RDONLY);
    int fd2 = openFile(input2, O_WRONLY);
    handleFileCopying(fd1, fd2);

    close(fd1);
    close(fd2);

    t = clock() - t;
    double time_taken = ((double) t) / CLOCKS_PER_SEC;
    int timeFd = open("../pomiar_zad_1", O_WRONLY | O_APPEND | O_CREAT, 0666);
    
    char res[256];
    snprintf(res, 256, "System libraries functions version took %f seconds\n", time_taken);
    write(timeFd, res, strlen(res));
    close(timeFd);
    return 0;
}