#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>


int main(int argc, char *argv[]) {
    if (argc != 4){
        printf("Wrong number of arguments!\n");
        exit(1);
    }
    
    char dirPath[PATH_MAX];
    char string[500];
    strcpy(dirPath, argv[1]);
    strcpy(string, argv[2]);
    long depth = strtol(argv[3], NULL, 10);

    DIR *dp;
    if ((dp = opendir(dirPath)) == NULL){
        printf("The path %s is incorrect!\n", dirPath);
        exit(1);
    }

    printf("%s %s %ld\n", dirPath, string, depth);
    // find the string by searching all txt files line by line

    closedir(dp);
    return 0;
}