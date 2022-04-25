#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>


void save_to_file(FILE *fp, char *text_line){
    int n_digits = -1;
    for (int i = 0; i < strlen(text_line); i++){
        if (text_line[i] == ' '){
            n_digits = i;
            break;
        }
        if (!isdigit(text_line[i]))
            break;
    }
    
    if (n_digits == -1){
        printf("There is something wrong with row number!\n");
        exit(1);
    }
    
    char *row_number = malloc((n_digits + 1) * sizeof(char));
    strncpy(row_number, text_line, n_digits);
    row_number[n_digits] = '\0';
    int row_num = atoi(row_number);
    text_line += 14;
    for (int i = 0; i < strlen(text_line); i++){
        if (text_line[i] == '\n')
            text_line[i] = '\0';
    }

    fseek(fp, 0, SEEK_END);
    int f_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *file_text = malloc((f_len + 1) * sizeof(char));
    fread(file_text, sizeof(char), f_len, fp);
    fseek(fp, 0, SEEK_SET);
    
    int curr_row = 1;
    for (int i = 0; i < f_len; i++){
        if (file_text[i] == '\n'){
            if (curr_row == row_num) fwrite(text_line, sizeof(char), strlen(text_line), fp);
            curr_row++;
        }

        fwrite(&file_text[i], sizeof(char), 1, fp);
    }
    
    while (curr_row < row_num){
        fwrite("\n", sizeof(char), 1, fp);
        curr_row++;
    }

    if (curr_row == row_num)
        fwrite(text_line, sizeof(char), strlen(text_line), fp);
    
    free(row_number);
}


int main(int argc, char *argv[]){
    if (argc != 4){
        printf("Wrong number of arguments!\n");
        exit(1);
    }

    char *fifo_path = argv[1];
    char *text_file_path = argv[2];
    int N = atoi(argv[3]);

    FILE *text_fp;
    if ((text_fp = fopen(text_file_path, "w+")) == NULL){
        printf("Can't open text file with path %s\n", text_file_path);
        exit(1);
    }

    FILE *fifo_p;
    if ((fifo_p = fopen(fifo_path, "r")) == NULL){
        printf("Can't open FIFO with path %s\n", fifo_path);
        exit(1);
    }

    setbuf(fifo_p, NULL);
    setbuf(stdout, NULL);
    char *buffer = malloc((N + 16) * sizeof(char));    

    int read_n = N + 15;
    while(1){
        flock(fileno(text_fp), LOCK_EX);
        if (fread(buffer, sizeof(char), read_n, fifo_p) <= 0) break;
        save_to_file(text_fp, buffer);
        memset(buffer, '\0', N + 16);
        flock(fileno(text_fp), LOCK_UN);
    }

    free(buffer);
    fclose(fifo_p);
    fclose(text_fp);
    return 0;
}