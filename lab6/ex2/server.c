#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "data.h"

mqd_t server_qd;
int clients[MAX_CLIENTS_NUM] = {};
FILE *log_file;
int handle_init(char message[]){
    printf("\nReceived INIT from client with qd %s!\n", message);

    int new_client_id;
    for (int i = 0; i < MAX_CLIENTS_NUM; i++){
        if (!clients[i]){
            new_client_id = i;
            break;
        }
    }

    if ((clients[new_client_id] = mq_open(message, O_WRONLY)) == -1){
        printf("Can't open the message queue of the client with id %d\n", new_client_id);
        return 1;
    }

    char mess[MAX_MESS_LEN], cl_id[100];
    sprintf(cl_id, "%d", new_client_id);
    strcpy(mess, cl_id);
    if (mq_send(clients[new_client_id], mess, MAX_MESS_LEN, INIT) == -1){
        printf("Can't send feedback message to client!\n");
        return 1;
    }
    printf("Sent back message with order in queue value!\n");
    return 0;
}

void log_to_file(long client_id, char *cmd, char text[]){
    char buffer[1000];
    struct tm *info;
    time_t curr_time;
    time(&curr_time);
    info = localtime(&curr_time);
    
    if (strlen(text) > 0)
        text[strlen(text) - 1] = '\0';
    
    if (strcmp(text, ""))
        snprintf(buffer, 1000, "%ld %s %s %s", client_id, cmd, text, asctime(info));
    else
        snprintf(buffer, 1000, "%ld %s %s", client_id, cmd, asctime(info));
    
    
    fwrite(buffer, sizeof(char), strlen(buffer), log_file);
    fflush(log_file);
}

int handle_stop(char *message){
    int client_id = atoi(message);
    if (mq_close(clients[client_id]) == -1){
        printf("Can't close %d's message queue!\n", client_id);
        return 1;
    }
    log_to_file(client_id, STOP_COM, "");
    printf("\nClient %d has stopped working!\n", client_id);
    clients[client_id] = 0;
    return 0;
}

int handle_list(char *message){
    int client_id = atoi(message);
    log_to_file(client_id, LIST_COM, "");
    char mess[MAX_MESS_LEN] = "";
    char buffer[10] = "";

    for (int i = 0; i < MAX_CLIENTS_NUM; i++){
        if (clients[i]){
            sprintf(buffer, "%d\n", i);
            strcat(mess, buffer);
        }
    }

    
    if (mq_send(clients[client_id], mess, MAX_MESS_LEN, LIST) == -1){
        printf("Can't send list of active clients to %d\n", client_id);
        return 1;
    }

    printf("List of active clients:\n%s\n", mess);
    return 0;
}

int handle_all(char *message){
    char *rest;
    strtok_r(message, " \n", &rest);
    int client_id = atoi(strtok_r(rest, " \n", &rest));
    time_t curr_time = time(NULL);
    char mess[MAX_MESS_LEN];
    sprintf(mess, "%d %ld %s", client_id, curr_time, rest);

    char temp[500];
    strcpy(temp, rest);
    log_to_file(client_id, ALL_COM, temp);
    for (int i = 0; i < MAX_CLIENTS_NUM; i++){
        if (clients[i] && i != client_id){
            if (mq_send(clients[i], mess, MAX_MESS_LEN, ALL) == -1){
                printf("Can't send the message from %d client to %d client!\n", client_id, i);
                return 1;
            }
        }
    }
    printf("Sent message to all clients from client %d!\n", client_id);
    return 0;
}

int handle_one(char *message){
    char *rest;
    int req_client_id = atoi(strtok_r(message, " \n", &rest));
    int client_id = atoi(strtok_r(rest, " \n", &rest));
    time_t curr_time = time(NULL);
    char mess[MAX_MESS_LEN];
    sprintf(mess, "%d %ld %s", client_id, curr_time, rest);

    if (!clients[req_client_id]){
        printf("\nRequested client is not active!\n");
        return 1;
    }

    char temp[500];
    strcpy(temp, rest);
    log_to_file(client_id, ONE_COM, temp);
    if (mq_send(clients[req_client_id], mess, MAX_MESS_LEN, ONE) == -1){
        printf("Can't send the message from %d client to %d client!\n", client_id, req_client_id);
        return 1;
    }
    printf("\nSent message to client %d from %d client!\n", client_id, req_client_id);
    return 0;
}

void handle_exit(){
    mq_close(server_qd);
    mq_unlink(SERVER_MQ_NAME);
    fclose(log_file);
}

void sigint_handler(){
    for (int i = 0; i < MAX_CLIENTS_NUM; i++){
        if (clients[i]){
            if (mq_send(clients[i], "dasda", MAX_MESS_LEN, STOP) == -1)
                printf("Can't send STOP message to client %d\n", i);
            
            if (mq_close(clients[i]) == -1)
                printf("Can't close the %d's message queue!\n", i);
            
            printf("Closed connection with %d client!\n", i);
            clients[i] = 0;
        }
    }

    printf("Successfully closed all connections!\n");
    exit(0);
}

void set_sigaction(){
    struct sigaction act;
    act.sa_handler = &sigint_handler;
    sigaction(SIGINT, &act, NULL);
}

int main(){
    atexit(handle_exit);
    set_sigaction();
    
    log_file = fopen("log_file.txt", "a");

    // create server message queue
    struct mq_attr attr = {
        .mq_msgsize = MAX_MESS_LEN,
        .mq_maxmsg = MAX_QUEUE_SIZE
    };
    if ((server_qd = mq_open(SERVER_MQ_NAME, O_RDONLY | O_CREAT, 0666, &attr)) == -1){
        printf("Can't create the server queue!\n");
        exit(1);
    }
    printf("Created the server message queue!\n");
    
    // main loop to receive messages from clients
    char message[MAX_MESS_LEN];
    unsigned int prio;
    while(1){
        if (mq_receive(server_qd, message, MAX_MESS_LEN, &prio) == -1){
            printf("Failed to receive a message from clients!\n");
            exit(1);
        }

        switch(prio){
            case INIT:
                if (handle_init(message) == 1)
                    printf("Can't handle INIT message received from client!\n");
                break;
            
            case STOP:
                if (handle_stop(message) == 1)
                    printf("Can't handle STOP message received from client!\n");
                break;
            
            case LIST:
                if (handle_list(message) == 1)
                    printf("Can't handle LIST message received from client!\n");
                break;
            
            case ALL:
                if (handle_all(message) == 1)
                    printf("Can't handle ALL message received from client!\n");
                break;
            
            case ONE:
                if (handle_one(message) == 1)
                    printf("Can't handle ONE message received from client!\n");
                break;

            default:
                break;
        }
    }

    return 0;
}