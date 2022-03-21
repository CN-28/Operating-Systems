// fread(), fwrite() version
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


int main(int argc, char *argv[]){
    char *input1, *input2;
    if (argc != 3) {
        input1 = malloc(256 * sizeof(char));
        input2 = malloc(256 * sizeof(char));
        scanf("%s%s", input1, input2);
    }
    else {
        input1 = argv[1];
        input2 = argv[2];
    }

    FILE *fp, *ofp;
    if ((fp = fopen(input1, "r")) == NULL){
        printf("I cannot open first file!\n");
        exit(1);
    }

    if ((ofp = fopen(input2, "r+")) == NULL){
        printf("I cannot open second file!\n");
        exit(1);
    }
    
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

    fclose(fp);
    fclose(ofp);
    free(buffor);
    free(temp);
    return 0;
}