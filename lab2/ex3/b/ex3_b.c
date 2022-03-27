#include <stdio.h>
#include <stdlib.h>
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>

int **subCounters, **counters, length = 1;

int nftwfunc(const char *fileName, const struct stat *statptr, int fileFlags, struct FTW *pfwt) {
    if (pfwt->level + 2 > length) {
        counters = realloc(counters, (pfwt->level + 2) * sizeof(int *));
        subCounters = realloc(subCounters, (pfwt->level + 2) * sizeof(int *));
        for (int j = length; j <= pfwt->level + 1; j++){
            subCounters[j] = calloc(7, sizeof(int));
            counters[j] = calloc(7, sizeof(int));
        }
        length = pfwt->level + 2;
    }
    
    char date1[100], date2[100];
    printf("Absolute path: %s\n", fileName);
    printf("Number of hard links: %ld\n", statptr->st_nlink);
    printf("File type: ");
    switch (statptr->st_mode & S_IFMT) {
        case S_IFBLK:
            printf("Block device\n");
            subCounters[pfwt->level][0]++;
            counters[pfwt->level][0]++;
            break;
        case S_IFCHR:
            printf("Character device\n");
            subCounters[pfwt->level][1]++;
            counters[pfwt->level][1]++;
            break;
        case S_IFDIR:
            printf("Directory\n");
            subCounters[pfwt->level][2]++;
            counters[pfwt->level][2]++;
            break;
        case S_IFIFO:
            printf("FIFO/pipe\n");
            subCounters[pfwt->level][3]++;
            counters[pfwt->level][3]++;
            break;
        case S_IFLNK:
            printf("Symlink\n");
            subCounters[pfwt->level][4]++;
            counters[pfwt->level][4]++;
            break;
        case S_IFREG:
            printf("Regular file\n");
            subCounters[pfwt->level][5]++;
            counters[pfwt->level][5]++; 
            break;
        case S_IFSOCK:
            printf("Socket\n");
            subCounters[pfwt->level][6]++;
            counters[pfwt->level][6]++;
            break;
        default:
            printf("Unknown?\n");
            break;
    }

    printf("File size in bytes: %ld\n", statptr->st_size);
    struct tm *my_tm1 = localtime(&statptr->st_atim.tv_sec);
    struct tm *my_tm2 = localtime(&statptr->st_mtim.tv_sec);
    strftime(date1, 100, "%Y-%m-%d %H:%M:%S", my_tm1);
    strftime(date2, 100, "%Y-%m-%d %H:%M:%S", my_tm2);
    printf("Date of last access: %s.%ld\n", date1, statptr->st_atim.tv_nsec);
    printf("Date of last modification: %s.%ld\n\n", date2, statptr->st_mtim.tv_nsec);
    if (fileFlags == FTW_DP){
        printf("Number of different types of files in all subdirectories / current directory:\n");
        printf("Regular files: %d %d\n", subCounters[pfwt->level + 1][5], counters[pfwt->level + 1][5]);
        printf("Directories: %d %d\n", subCounters[pfwt->level + 1][2], counters[pfwt->level + 1][2]);
        printf("Character device files: %d %d\n", subCounters[pfwt->level + 1][1], counters[pfwt->level + 1][1]);
        printf("Block device files: %d %d\n", subCounters[pfwt->level + 1][0], counters[pfwt->level + 1][0]);
        printf("FIFO files: %d %d\n", subCounters[pfwt->level + 1][3], counters[pfwt->level + 1][3]);
        printf("Symbolic links: %d %d\n", subCounters[pfwt->level + 1][4], counters[pfwt->level + 1][4]);
        printf("Socket files: %d %d\n", subCounters[pfwt->level + 1][6], counters[pfwt->level + 1][6]);

        for (int i = 0; i < 7; i++){
            subCounters[pfwt->level][i] += subCounters[pfwt-> level + 1][i];
            subCounters[pfwt->level + 1][i] = 0;
            counters[pfwt->level + 1][i] = 0;
        }
    }
    printf("----------------------------------------------------\n");
    printf("\n");
    return 0;
}


int main(int argc, char *argv[]) {
    subCounters = malloc(sizeof(int *));
    counters = malloc(sizeof(int *));
    subCounters[0] = calloc(7, sizeof(int));
    counters[0] = calloc(7, sizeof(int));

    int flags = FTW_DEPTH | FTW_PHYS;
    char dirPath[PATH_MAX];
    if (argc != 2) {
        printf("Wrong number of arguments!\n");
        exit(1);
    }
    else
        realpath(argv[1], dirPath);

    
    nftw(dirPath, nftwfunc, 5, flags);

    for (int i = 0; i < length; i++){
        free(subCounters[i]);
        free(counters[i]);
    }
    free(subCounters);
    free(counters);
    return 0;
}