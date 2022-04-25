#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SENDER "sender"
#define DATE "date"

void check_arguments(int argc, char *argv[]){
    if (argc != 2 && argc != 4){
        printf("Wrong number of arguments!\n");
        exit(1);
    }

    if (argc == 2){
        if (strcmp(argv[1], SENDER) && strcmp(argv[1], DATE)){
            printf("Provided argument is incorrect!\n");
            exit(1);
        }
    }
}

void handle_printing(char *argv[]){
    FILE *fp;
    if(!strcmp(argv[1], SENDER)){
        fp = popen("mail | sort -k 3", "r");
        printf("Mail sorted by %s\n", SENDER);
    }
    else if (!strcmp(argv[1], DATE)){
        fp = popen("mail", "r");
        printf("Mail sorted by %s\n", DATE);
    }
        
    char line[1024];
    while (fgets(line, 1024, fp) != NULL)
        printf("%s", line);

    pclose(fp);
}

void handle_mail_sending(char *argv[]){
    char *recipient = argv[1];
    char *topic = argv[2];
    char *content = argv[3];

    char *command = malloc(strlen(recipient) + strlen(topic) + strlen(content) + 50);
    sprintf(command, "echo %s | mail -s %s %s", content, topic, recipient);

    FILE *fp;
    if((fp = popen(command, "w")) == NULL){
        printf("Unable to send e-mail\n");
        free(command);
        exit(1);
    }

    printf("Email recipient: %s\n", recipient);
    printf("Topic: %s\n", topic);
    printf("Content: %s\n", content);
    free(command);
    pclose(fp);
}


int main(int argc, char *argv[]){
    check_arguments(argc, argv);

    if (argc == 2)
        handle_printing(argv);
    else
        handle_mail_sending(argv);

    return 0;
}