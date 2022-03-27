#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>


struct Tuple {
    int subRegularFiles, subDirs, subCharDev, subBlockDev, subFifo, subSymlinks, subSockets;
};


struct Tuple lookUpSubDir(DIR *dp, char dirPath[], struct Tuple tuple) {
    tuple.subRegularFiles = 0; tuple.subBlockDev = 0; tuple.subCharDev = 0; tuple.subDirs = 0;
    tuple.subFifo = 0; tuple.subSockets = 0; tuple.subSymlinks = 0;
    int regularFiles = 0, dirs = 0, charDev = 0, blockDev = 0, fifo = 0, symlinks = 0, sockets = 0;
    char buf[PATH_MAX];
    char str[PATH_MAX];
    char date1[100];
    char date2[100];
    struct dirent *entry;
    struct stat buffer;
    while ((entry = readdir(dp)) != NULL) {
        strcpy(str, dirPath);
        strcat(str, "/");
        strcat(str, entry->d_name);
        
        if (strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".") && !S_ISLNK(entry->d_type)) {
            printf("\t---------------------------------------\n"); 
            realpath(str, buf);
            stat(entry->d_name, &buffer);
            printf("\tAbsolute path: %s\n", buf);
            printf("\tNumber of hard links: %ld\n", buffer.st_nlink);
            switch (entry->d_type){
                case DT_FIFO:
                    printf("\tFile type: FIFO file\n");
                    fifo++;
                    break;
            
                case DT_CHR:
                    printf("\tFile type: Character device file\n");
                    charDev++;
                    break;

                case DT_DIR:
                    printf("\tFile type: Directory\n");
                    dirs++;
                    break;

                case DT_BLK:
                    printf("\tFile type: Block device files\n");
                    blockDev++;
                    break;

                case DT_REG:
                    printf("\tFile type: Regular file\n");
                    regularFiles++;
                    break;

                case DT_LNK:
                    printf("\tFile type: Symbolic link file\n");
                    symlinks++;
                    break;

                case DT_SOCK:
                    printf("\tFile type: Socket file\n");
                    sockets++;
                    break;

                default:
                    break;
            }

            printf("\tFile size in bytes: %ld\n", buffer.st_size);
            struct tm *my_tm1 = localtime(&buffer.st_atim.tv_sec);
            struct tm *my_tm2 = localtime(&buffer.st_mtim.tv_sec);
            strftime(date1, 100, "%Y-%m-%d %H:%M:%S", my_tm1);
            strftime(date2, 100, "%Y-%m-%d %H:%M:%S", my_tm2);
            printf("\tDate of last access: %s.%ld\n", date1, buffer.st_atim.tv_nsec);
            printf("\tDate of last modification: %s.%ld\n\n", date2, buffer.st_mtim.tv_nsec);
            if (entry->d_type == DT_DIR){
                struct Tuple resTuple = lookUpSubDir(opendir(str), str, tuple);
                tuple.subRegularFiles += resTuple.subRegularFiles; tuple.subBlockDev += resTuple.subBlockDev;
                tuple.subCharDev += resTuple.subCharDev; tuple.subDirs += resTuple.subDirs; tuple.subFifo += resTuple.subFifo;
                tuple.subSymlinks += resTuple.subSymlinks; tuple.subSockets += resTuple.subSockets;
            }    
        }
    }
    tuple.subRegularFiles += regularFiles;
    tuple.subDirs += dirs;
    tuple.subBlockDev += blockDev;
    tuple.subCharDev += charDev;
    tuple.subFifo += fifo;
    tuple.subSockets += sockets;
    tuple.subSymlinks += symlinks;
    
    printf("%s\n", dirPath);
    printf("Number of different types of files in current directory / all subdirectories:\n");
    printf("Regular files: %d %d\n", regularFiles, tuple.subRegularFiles);
    printf("Directories: %d %d\n", dirs, tuple.subDirs);
    printf("Character device files: %d %d\n", charDev, tuple.subCharDev);
    printf("Block device files: %d %d\n", blockDev, tuple.subBlockDev);
    printf("FIFO files: %d %d\n", fifo, tuple.subFifo);
    printf("Symbolic links: %d %d\n", symlinks, tuple.subSymlinks);
    printf("Socket files: %d %d\n", sockets, tuple.subSockets);
    printf("----------------------\n");
    closedir(dp);

    return tuple;
}


int main(int argc, char *argv[]) {
    char dirPath[PATH_MAX];
    if (argc != 2) {
        printf("Wrong number of arguments!\n");
        exit(1);
    }
    else
        realpath(argv[1], dirPath);

    DIR *dp;
    if ((dp = opendir(dirPath)) == NULL) {
        printf("The path %s is incorrect!\n", dirPath);
        exit(1);
    }

    struct Tuple tuple = {0};
    lookUpSubDir(dp, dirPath, tuple);   
    return 0;
}