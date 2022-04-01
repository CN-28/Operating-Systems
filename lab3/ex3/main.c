#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


void searchFileForString(char dirPath[], char string[], char fileName[], char dirName[]) {
    FILE *fp = fopen(dirPath, "r");
    
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *res = malloc(fileSize * sizeof(char));
    fread(res, sizeof(char), fileSize, fp);

    int contains = 0;
    for (int i = 0; i < strlen(res); i++) {
        for (int j = 0; j < strlen(string); j++){
            contains = 0;
            if (!(res[i + j] == string[j]))
                break;
            
            contains = 1;
        }
        if (contains){
            printf("\nRelative path from starting directory: ");
            int found = 0;
            for (int k = 0; k < strlen(dirPath); k++){
                if (!found){
                    for (int l = 0; l < strlen(dirName); l++){
                        found = 0;
                        if (!(dirPath[k + l] == dirName[strlen(dirName) - 1 -l]))
                            break;
                        
                        found = 1;
                    }
                    if (found) printf("%c", dirPath[k]);
                }
                else
                    printf("%c", dirPath[k]);
                
            }

            printf("\nPPID %d\nPID: %d\nFile name: %s\n\n", getppid(), getpid(), fileName);
            break;
        }    
    }

    free(res);
    fclose(fp);
}


void search(DIR * dp, char dirPath[], char string[], int currDepth, int maxDepth, char dirName[]) {
    struct dirent *entry;
    char str[PATH_MAX];
    int n;
    while((entry = readdir(dp)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
            
        strcpy(str, dirPath);
        strcat(str, "/");
        strcat(str, entry->d_name);
        n = strlen(str);
        if (entry->d_type == DT_DIR){
            if (currDepth + 1 <= maxDepth && fork() == 0){
                search(opendir(str), str, string, currDepth + 1, maxDepth, dirName);
                break;
            }
        }
        else if (entry->d_type == DT_REG && n > 4 && str[n - 4] == '.' && str[n - 3] == 't' && str[n - 2] == 'x' && str[n - 1] == 't')
            searchFileForString(str, string, entry->d_name, dirName);
        
        while(wait(NULL) != -1);
    }

    closedir(dp);
}


int main(int argc, char *argv[]) {
    if (argc != 4){
        printf("Wrong number of arguments!\n");
        exit(1);
    }
    
    char dirPath[PATH_MAX];
    char string[500];
    strcpy(dirPath, argv[1]);
    strcpy(string, argv[2]);
    long maxDepth = strtol(argv[3], NULL, 10);
    if (maxDepth < 1){
        printf("The depth of search can't be lower than 1!\n");
        exit(1);
    }

    DIR *dp;
    if ((dp = opendir(dirPath)) == NULL){
        printf("The path %s is incorrect!\n", dirPath);
        exit(1);
    }

    char dirName[PATH_MAX];
    int i = strlen(dirPath) - 1;
    int j = 0;
    while(i >= 0 && dirPath[i] != '/') {
        dirName[j] = dirPath[i];
        j++;
        i--;
    }
    dirName[j] = '\0';

    search(dp, dirPath, string, 1, maxDepth, dirName);


    return 0;
}