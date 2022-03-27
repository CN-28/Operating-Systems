// C libraries functions version
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX 256

int main(int argc, char *argv[]) {
    clock_t t;
    t = clock();

    char *fileName, *ch;
    if (argc != 3) {
        printf("Wrong number of arguments!\n");
        exit(1);
    }
    else{
        ch = argv[1];
        fileName = argv[2];
        if (strlen(ch) != 1){
            printf("Given input is not a character!\n");
            exit(1);
        }
    }
        
    FILE *fp;
    if ((fp = fopen(fileName, "r")) == NULL) {
        printf("Cannot open %s file!\n", fileName);
        exit(1);
    }

    char buffor[MAX];
    int cnt = 0, lineCnt = 0, containsChar = 0;
    while (fgets(buffor, MAX, fp) != NULL){
        for (int i = 0; i < strlen(buffor); i++){
            if (buffor[i] == *ch){
                cnt++;
                containsChar = 1;
            }
        }

        if (containsChar){
            lineCnt++;
            containsChar = 0;
        }
    }
    printf("Number of given character occurences in the whole file: %d\nNumber of lines, which contain given character: %d\n", cnt, lineCnt);

    fclose(fp);

    t = clock() - t;
    double time_taken = ((double) t) / CLOCKS_PER_SEC;
    FILE *timeFp;
    timeFp = fopen("../pomiar_zad_2", "a");
    
    char res[100];
    snprintf(res, 100, "C libraries functions version took %f seconds\n", time_taken);
    fwrite(res, sizeof(char), strlen(res), timeFp);
    fclose(timeFp);
    return 0;
}