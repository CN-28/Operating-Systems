#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>


int main(int argc, char *argv[]){
    if (argc != 5){
        printf("Wrong number of arguments!\n");
        exit(1);
    }

    char row_number[15];
    char *fifo_path = argv[1];
    strcpy(row_number, argv[2]);
    char *text_file_path = argv[3];
    int N = atoi(argv[4]);

    FILE *text_fp;
    if ((text_fp = fopen(text_file_path, "r")) == NULL){
        printf("Can't open provided file with path %s\n", text_file_path);
        exit(1);
    }

    FILE *fifo_p;
    if ((fifo_p = fopen(fifo_path, "w")) == NULL){
        printf("Can't open FIFO with path %s\n", fifo_path);
        exit(1);
    }

    setbuf(fifo_p, NULL);
    setbuf(stdout, NULL);
    char *buffer = malloc((N + 1) * sizeof(char));
    
    for (int i = strlen(row_number); i < 14; i++){
        row_number[i] = ' ';
    }
    row_number[14] = '\0';

    while((fread(buffer, sizeof(char), N, text_fp)) > 0) {
        flock(fileno(fifo_p), LOCK_EX);
        fwrite(row_number, sizeof(char), strlen(row_number), fifo_p);
        fwrite(buffer, sizeof(char), N, fifo_p);
        fwrite("\n", sizeof(char), 1, fifo_p);
        memset(buffer, '\0', N + 1);
        flock(fileno(fifo_p), LOCK_UN);
        sleep(((double) rand() / (double) RAND_MAX) + 1);
    }

    free(buffer);
    fclose(text_fp);
    fclose(fifo_p);
    return 0;
}