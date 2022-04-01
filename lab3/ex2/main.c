#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include <time.h>


void checkArgs(int argc, char *argv[]){
    if (argc != 2) {
        printf("Wrong number of arguments!\n");
        exit(1);
    }

    if (argv[1][0] == '-'){
        printf("Given number mustn't be negative!\n");
        exit(1);
    }
    else {
        int length = strlen(argv[1]);
        for (int i = 0; i < length; i++){
            if (!isdigit(argv[1][i])){
                printf("Given input is not a natural number!\n");
                exit(1);
            }
        }
    }
}


double f(double x) {
    return 4 / (pow(x, 2) + 1);
}


double getRectangleArea(double l, double r) {
    return (r - l) * f(r);
}


void handleIntegralCalculations(FILE *fp, char buff[], char fileName[], char number[], long n) {
    double h = 1 / (double) n;
    for (int i = 1; i < n + 1; i++){
        if (fork() == 0){
            sprintf(number, "%d", i);
            strcat(fileName, "w");
            strcat(fileName, number);
            strcat(fileName, ".txt");
            fp = fopen(fileName, "w");
            memset(fileName, 0, 128);
            sprintf(buff, "%f", getRectangleArea((i - 1) * h, i * h));
            fwrite(buff, sizeof(char), strlen(buff), fp);
            fclose(fp);
            exit(0);
        }   
    }
    while (wait(NULL) != -1);
}


double sumCalculationsFromFile(FILE *fp, char buff[], char fileName[], char number[], long n){
    double res = 0;
    for (int i = 1; i < n + 1; i++){
        sprintf(number, "%d", i);
        strcat(fileName, "w");
        strcat(fileName, number);
        strcat(fileName, ".txt");
        fp = fopen(fileName, "r");
        memset(fileName, 0, 128);
        fread(buff, sizeof(char), 128, fp);
        res += strtod(buff, NULL);
        fclose(fp);
    }

    return res;
}


void saveTimeMeasurementsToFile(struct timespec start, struct timespec stop, long n, double res, FILE *fp, char buff[]){
    double t = (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec) / 1e9;
    printf("\tFor n = %ld\nResult: %f\nTime taken: %f s.\n", n, res, t);

    fp = fopen("times.txt", "a");
    snprintf(buff, 256, "For %ld intervals ", n);
    fwrite(buff, sizeof(char), strlen(buff), fp);
    snprintf(buff, 256, "calculations took %f seconds\n", t);
    fwrite(buff, sizeof(char), strlen(buff), fp);
    fclose(fp);
}


int main(int argc, char *argv[]) {
    struct timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);

    checkArgs(argc, argv);
    long n = strtol(argv[1], NULL, 10);
    if (n == 0) {
        printf("The interval can't be divided into 0 subintervals!\n");
        exit(1);
    }

    FILE *fp = NULL;
    char buff[128];
    char fileName[128];
    char number[2];
    handleIntegralCalculations(fp, buff, fileName, number, n);
    double res = sumCalculationsFromFile(fp, buff, fileName, number, n);
    
    clock_gettime(CLOCK_REALTIME, &stop);
    saveTimeMeasurementsToFile(start, stop, n, res, fp, buff);

    return 0;
}