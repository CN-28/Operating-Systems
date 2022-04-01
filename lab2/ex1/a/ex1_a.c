// fread(), fwrite() version
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>


void handleFileCopying(FILE *fp, FILE *ofp) {
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    
    char *buffor = malloc(20 * sizeof(char));
    char *temp = malloc(20 * sizeof(char));
    int charsRead = 0, bufforSize = 20, isEmpty = 1, last_ws_i, k, tempFilled = 0, temp_i = 0, sumRead = 0;
    while (1) {
        charsRead = fread(buffor, sizeof(char), bufforSize, fp);
        sumRead += charsRead;

        k = 0;
        last_ws_i = -1;
        for (int i = 0; i < charsRead; i++) {
            if (isspace(buffor[i])) {
                if (buffor[i] == '\n' || buffor[i] == '\v' || buffor[i] == '\f') {
                    if (!isEmpty) {
                        
                        if (tempFilled)
                            fwrite(temp, sizeof(char), temp_i, ofp);
                        
                        for (int j = last_ws_i + 1; j <= i; j++) {
                            buffor[k] = buffor[j];
                            k++;
                        }
                        isEmpty = 1;
                    }
                    
                    temp_i = 0;
                    tempFilled = 0;
                    last_ws_i = i;
                }
            }
            else
                isEmpty = 0;
        }
        
        if (!isEmpty) {
            for (int j = last_ws_i + 1; j < charsRead; j++) {
                buffor[k] = buffor[j];
                k++;
            }
        }
        else {
            for (int j = last_ws_i + 1; j < charsRead; j++){
                temp[temp_i] = buffor[j];
                temp_i++;
            } 
            tempFilled = 1;
        }

        if (k > 0){
            if (sumRead == size && (buffor[k - 1] == '\n' || buffor[k - 1] == '\v' || buffor[k - 1] == '\f'))
                fwrite(buffor, sizeof(char), k - 1, ofp);
            else
                fwrite(buffor, sizeof(char), k, ofp);
        }

        if (charsRead != bufforSize)
            break;

        bufforSize *= 2;
        buffor = realloc(buffor, bufforSize);
        temp = realloc(temp, sumRead + bufforSize);
    }
    free(buffor);
    free(temp);
}


void openFile(FILE **fp, char* input, char* fileMode) {
    if ((*fp = fopen(input, fileMode)) == NULL){
        printf("Cannot open %s file!\n", input);
        exit(1);
    }
}


void getFileNames(int argc, char *argv[], char **input1, char **input2) {
    if (argc != 3) {
        *input1 = malloc(256 * sizeof(char));
        *input2 = malloc(256 * sizeof(char));
        scanf("%s%s", *input1, *input2);
    }
    else {
        *input1 = argv[1];
        *input2 = argv[2];
    }
}


int main(int argc, char *argv[]) {
    clock_t t;
    t = clock();

    char *input1, *input2;
    getFileNames(argc, argv, &input1, &input2);

    FILE *fp, *ofp;
    openFile(&fp, input1, "r");
    openFile(&ofp, input2, "r+");
    handleFileCopying(fp, ofp);

    fclose(fp);
    fclose(ofp);

      
    t = clock() - t;
    double time_taken = ((double) t) / CLOCKS_PER_SEC;
    FILE *timeFp;
    timeFp = fopen("../pomiar_zad_1", "a");
    
    char res[256];
    snprintf(res, 256, "C libraries functions version took %f seconds\n", time_taken);
    fwrite(res, sizeof(char), strlen(res), timeFp);
    fclose(timeFp);
    return 0;
}