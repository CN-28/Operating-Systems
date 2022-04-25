// max 10 components each with max 10 arguments
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

#define MAX_LINE_LEN 1000
#define MAX_COMMANDS 100
#define MAX_ARGS 10
#define MAX_COMPS 10

void check_arguments(int argc){
    if (argc != 2){
        printf("Incorrect number of arguments!\n");
        exit(1);
    }
}

void open_file(FILE **fp, char *argv[]){
    if ((*fp = fopen(argv[1], "r")) == NULL){
        printf("Can't open provided file with path %s!", argv[1]);
        exit(1);
    }
}

char **get_args(int i, int j, char comps[][MAX_COMPS][MAX_ARGS][100], int *args_len){
    int iter = 0;
    while (strcmp(comps[i][j][iter], "NULL"))
        iter++;

    char **res = malloc((iter + 1) * sizeof(char *));
    for (int k = 0; k < iter; k++)
        res[k] = comps[i][j][k];
    res[iter] = NULL;
    *args_len = iter + 1;
    return res;
}

void execute_commands(char line[], char comps[][MAX_COMPS][MAX_ARGS][100], int len[]){
    printf("--------------------------\nPipe: ");
    if (line[strlen(line) - 1] != '\n')
        printf("%s\n", line);
    else
        printf("%s", line);

    int command[MAX_COMPS];
    char *comp;

    char *temp = line;
    int ind = 0;
    
    while ((comp = strtok(temp, " \n")) != NULL){
        if (strcmp(comp, "|")){
            while (!isdigit(comp[0]))
                comp++;

            command[ind] = atoi(comp) - 1;
            ind++;
        }
        temp = NULL;
    }

    int comm_id, args_len;
    char **args;
    int fd1[2], fd2[2];
    pipe(fd1);
    close(fd1[0]);
    close(fd2[1]);
    for (int i = 0; i < ind; i++){
        comm_id = command[i];
        for (int j = 0; j < len[comm_id]; j++){
            args = get_args(comm_id, j, comps, &args_len);
        
            pipe(fd2);
            if (fork() == 0){
                dup2(fd1[0], STDIN_FILENO);
                dup2(fd2[1], STDOUT_FILENO);
                close(fd1[1]);
                close(fd2[0]);

                execvp(args[0], args);
            }

            close(fd2[1]);
            fd1[0] = fd2[0];
            fd1[1] = fd2[1];

            free(args);
        }
    }
    while (wait(NULL) != -1);

    char ch;
    while (read(fd1[0], &ch, 1) == 1)
        printf("%c", ch);
    
    close(fd1[0]);
}

void handle_commands(FILE *fp){
    char comps[MAX_COMMANDS][MAX_COMPS][MAX_ARGS][100];
    int len[MAX_COMMANDS] = {0};

    char line[MAX_LINE_LEN];
    char *split;
    int next_args = 0, args_i = 0, i = 0, read_comps = 0, comm = 0;
    while (fgets(line, MAX_LINE_LEN, fp) != NULL){
        if (!read_comps){
            read_comps = 1;
            for (int l = 0; l < strlen(line); l++){
                if (!isspace(line[l]))
                    read_comps = 0;
            }
            if (read_comps) continue;
        }

        if (!read_comps){
            split = strtok(line, " ");
            while ((split = strtok(NULL, " \n")) != NULL){
                if (next_args && strcmp(split, "|")){
                    strcpy(comps[comm][i][args_i], split);
                    args_i++;
                }

                if (!strcmp(split, "=") || !strcmp(split, "|")){
                    next_args = 1;
                    if (!strcmp(split, "|")){
                        strcpy(comps[comm][i][args_i], "NULL");
                        i++;
                    }

                    args_i = 0;
                }
            }
            len[comm] = i + 1;
            strcpy(comps[comm][i][args_i], "NULL");
            comm++;
            args_i = 0;
            i = 0;
        }
        else
           execute_commands(line, comps, len);
    }
}


int main(int argc, char *argv[]){
    check_arguments(argc);    

    FILE *fp;
    open_file(&fp, argv);
    handle_commands(fp);

    fclose(fp);
    return 0;
}