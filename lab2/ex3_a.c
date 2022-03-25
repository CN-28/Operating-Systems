#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


int main(int argc, char *argv[]) {
    char *dirPath;
    if (argc != 2) {
        printf("Wrong number of arguments!\n");
        exit(1);
    }
    else
        dirPath = argv[1];
    

    DIR *dp;
    if ((dp = opendir(dirPath)) == NULL) {
        printf("The path %s is incorrect!\n", dirPath);
        exit(1);
    }

    char buf[PATH_MAX];
    char str[PATH_MAX];
    char date1[50];
    char date2[50];
    struct dirent *entry;
    struct stat buffer;
    while ((entry = readdir(dp)) != NULL) {
        printf(" %s %ld %d\n", entry->d_name, entry->d_ino, entry->d_type);
        
        if (entry->d_type != 4) {
            strcpy(str, dirPath);
            strcat(str, "/");
            strcat(str, entry->d_name);
            realpath(str, buf);
            stat(entry->d_name, &buffer);
            
            // ścieżka bezwzględna
            printf("%s\n", buf);
            // liczba dowiązań
            printf("%ld\n", buffer.st_nlink);
            // rodzaj pliku
            printf("%d\n", entry->d_type);
            // rozmiar w bajtach
            printf("%ld\n", buffer.st_size);
            // data ostatniego dostępu
            printf("%s\n", ctime(&buffer.st_atime));
            // data ostatniego modyfikacji
            printf("%s\n", ctime(&buffer.st_mtime));
        }
    }

    
    return 0;
}