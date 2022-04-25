#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

#define TEST_FOLDER "test_files/"

void check_res_files(char *row_numbers[], char *test_files[], char *res_file_path, int size){

    FILE *fp_res, *fp_test;
    fp_res = fopen(res_file_path, "r");

    int lines_cnt = 1, temp = 0;
    char line[4096], row_line[4096];
    while (fgets(line, 4096, fp_res) != NULL){
        if (line[0] != '\n'){
            for (int i = 0; i < size; i++){
                if (atoi(row_numbers[i]) == lines_cnt){
                    fp_test = fopen(test_files[i], "r");
                    fgets(row_line, 4096, fp_test);

                    if (line[strlen(line) - 1] == '\n')
                        row_line[strlen(row_line)] = '\n';
                    if (strcmp(row_line, line)){
                        temp = 1;
                        fclose(fp_test);
                        break;
                    }
                    row_numbers[i] = "-1";
                    memset(row_line, 0, 4096);
                    fclose(fp_test);
                }
            }
        }
        memset(line, 0, 4096);
        if (temp) break;
        lines_cnt++;
    }
    
    for (int i = 0; i < size; i++){
        if (strcmp(row_numbers[i], "-1")){
            printf("TEST %s FAILED\n", res_file_path);
            fclose(fp_res);
            return;
        }
    }
    
    printf("TEST %s SUCCEEDED\n", res_file_path);
    fclose(fp_res);
}

void many_prod_one_cons(char *n, char *res_file_path){
    char *mode = "many_prod_one_cons_";
    snprintf(res_file_path, strlen(TEST_FOLDER) + 30, "%s%s%s%s", TEST_FOLDER, mode, n, ".txt");

    char *cons_args[] = {"./consumer", "my_fifo", res_file_path, n, NULL};
    mkfifo("my_fifo", 0666);
    if (fork() == 0)
        execvp(cons_args[0], cons_args);
    
    
    char *row_numbers[] = {"10", "100", "50", "25", "1"};
    char *test_files[] = {"test_files/1.txt", "test_files/2.txt", "test_files/3.txt", "test_files/4.txt", "test_files/5.txt"};
    char *prod_args[6];
    prod_args[0] = "./producer"; prod_args[1] = "my_fifo"; prod_args[4] = n; prod_args[5] = NULL;
    
    for (int i = 0; i < 5; i++){
        if (fork() == 0){
            prod_args[2] = row_numbers[i]; prod_args[3] = test_files[i];
            execvp(prod_args[0], prod_args);
        }
    }

    while(wait(NULL) != -1);
    check_res_files(row_numbers, test_files, res_file_path, 5);
}

void one_prod_many_cons(char *n, char *res_file_path){
    char *mode = "one_prod_many_cons_";
    snprintf(res_file_path, strlen(TEST_FOLDER) + 30, "%s%s%s%s", TEST_FOLDER, mode, n, ".txt");

    mkfifo("my_fifo", 0666);
    char *cons_args[] = {"./consumer", "my_fifo", res_file_path, n, NULL};
    for (int i = 0; i < 5; i++){
        if (fork() == 0)
            execvp(cons_args[0], cons_args);
    }
    
    char *row_number[] = {"10"};
    char *test_file[] = {"test_files/1.txt"};
    char *prod_args[6];
    prod_args[0] = "./producer"; prod_args[1] = "my_fifo"; prod_args[2] = row_number[0]; prod_args[3] = test_file[0]; prod_args[4] = n; prod_args[5] = NULL;

    if (fork() == 0)
        execvp(prod_args[0], prod_args);

    while(wait(NULL) != -1);
    check_res_files(row_number, test_file, res_file_path, 1);
}

void many_prod_many_cons(char *n, char *res_file_path){
    char *mode = "many_prod_many_cons_";
    snprintf(res_file_path, strlen(TEST_FOLDER) + 30, "%s%s%s%s", TEST_FOLDER, mode, n, ".txt");

    mkfifo("my_fifo", 0666);
    char *cons_args[] = {"./consumer", "my_fifo", res_file_path, n, NULL};
    for (int i = 0; i < 5; i++){
        if (fork() == 0)
            execvp(cons_args[0], cons_args);
    }
    
    char *row_numbers[] = {"10", "100", "50", "25", "1"};
    char *test_files[] = {"test_files/1.txt", "test_files/2.txt", "test_files/3.txt", "test_files/4.txt", "test_files/5.txt"};
    char *prod_args[6];
    prod_args[0] = "./producer"; prod_args[1] = "my_fifo"; prod_args[4] = n; prod_args[5] = NULL;
    
    for (int i = 0; i < 5; i++){
        prod_args[2] = row_numbers[i]; prod_args[3] = test_files[i];
        if (fork() == 0)
            execvp(prod_args[0], prod_args);
    }

    while(wait(NULL) != -1);
    check_res_files(row_numbers, test_files, res_file_path, 5);
}

int main(){
    char *n_values[] = {"250", "500", "1000"};
    char *res_file_path = malloc((strlen(TEST_FOLDER) + 30) * sizeof(char));

    for (int i = 0; i < 3; i++){
        many_prod_one_cons(n_values[i], res_file_path);
        one_prod_many_cons(n_values[i], res_file_path);
        many_prod_many_cons(n_values[i], res_file_path);
    }

    free(res_file_path);
    return 0;
}