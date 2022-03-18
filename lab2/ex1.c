#include <stdio.h>
#include <stdlib.h>

int main(){
    FILE *fp;
    if ((fp = fopen("file", "r")) == NULL){
        printf("I cannot open this file!\n");
        exit(1);
    }

    

    return 0;
}