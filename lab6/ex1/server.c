#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "data.h"

int clients_qid[MAX_CLIENTS_NUM] = {};
int server_qid;
FILE *log_file;
void handle_init(int sender_qid){
    printf("\nReceived INIT from client with %d qid!\n", sender_qid);
    my_msbuf message;
    message.mtype = INIT;

    int new_client_id;
    for (int i = 0; i < MAX_CLIENTS_NUM; i++){
        if (!clients_qid[i]){
            new_client_id = i;
            clients_qid[i] = sender_qid;
            break;
        }
    }

    message.client_id = new_client_id;
    msgsnd(sender_qid, &message, MSGSZ, 0);
    printf("Sent back message with order in queue value!\n");
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

void handle_stop(long client_id){
    log_to_file(client_id, STOP_COM, "");    
    printf("\nClient %ld has stopped working!\n", client_id);
    clients_qid[client_id] = 0;
}

void handle_list(long client_id){
    log_to_file(client_id, LIST_COM, "");
    char list[MAX_MESS_LEN] = "";
    char buffer[10] = "";

    for (int i = 0; i < MAX_CLIENTS_NUM; i++){
        if (clients_qid[i]){
            sprintf(buffer, "%d\n", i);
            strcat(list, buffer);
        }
    }

    my_msbuf mess = {
        .mtype = LIST
    };
    strcpy(mess.mtext, list);

    if (msgsnd(clients_qid[client_id], &mess, MSGSZ, 0) == -1){
        printf("Can't send the list of active clients to client %ld", client_id);
        return;
    }

    printf("\nList has been sent to the client %ld!\n", client_id);
    printf("List of active clients: \n%s", mess.mtext);
}

void handle_all(long client_id, char text[]){
    my_msbuf mess = {
        .mtype = ALL,
        .client_id = client_id,
        .sender_qid = clients_qid[client_id],
        .stime = time(NULL)
    };
    strcpy(mess.mtext, text);
    log_to_file(client_id, ALL_COM, text); 
    for (int i = 0; i < MAX_CLIENTS_NUM; i++){
        if (clients_qid[i] && i != client_id){
            if (msgsnd(clients_qid[i], &mess, MSGSZ, 0) == -1){
                printf("Can't send the message from %ld client to %d client!\n", client_id, i);
                return;
            }
        }
            
    }

    printf("\nSent message to all clients from client %ld!\n", client_id);
}

void handle_one(long client_id, long requested_client_id, char text[]){
    if (!clients_qid[requested_client_id]){
        printf("\nRequested client is not active!\n");
        return;
    }
    
    my_msbuf mess = {
        .mtype = ALL,
        .client_id = client_id,
        .sender_qid = clients_qid[client_id],
        .stime = time(NULL)
    };
    strcpy(mess.mtext, text);
    log_to_file(client_id, ONE_COM, text);
    if (msgsnd(clients_qid[requested_client_id], &mess, MSGSZ, 0) == -1){
        printf("Can't send the message from %ld client to %ld clinet!\n", client_id, requested_client_id);
        return;
    }

    printf("\nSent message to client %ld from %ld client!\n", client_id, requested_client_id);
}

void sigint_handler(){
    my_msbuf mess = {
        .mtype = STOP
    };

    for (int i = 0; i < MAX_CLIENTS_NUM; i++){
        if (clients_qid[i]){
            if (msgsnd(clients_qid[i], &mess, MSGSZ, 0) == -1)
                printf("Can't send STOP message to client %d\n", i);
        }
    }

    exit(0);
}

void set_up_sigaction(){
    struct sigaction act;
    act.sa_handler = &sigint_handler;
    sigaction(SIGINT, &act, NULL);
}

void handle_exit(){
    my_msbuf rec_mess;
    struct msqid_ds q_stat;

    while (1){
        msgctl(server_qid, IPC_STAT, &q_stat);
        if (q_stat.msg_qnum > 0){
            if (msgrcv(server_qid, &rec_mess, MSGSZ, STOP, 0) == -1)
                printf("Failed to receive the STOP message from %ld client!\n", rec_mess.client_id);
                
            handle_stop(rec_mess.client_id);
        }
        else
            break;
    }

    msgctl(server_qid, IPC_RMID, NULL);
    printf("\nDeleted server queue at exit!\n");
    fclose(log_file);
}

int main(){
    log_file = fopen("log_file.txt", "a");
    char *homedir;
    if((homedir = getenv("HOME")) == NULL){
        printf("The $HOME variable is not set!\n");
        exit(1);
    }
    set_up_sigaction();
    atexit(handle_exit);

    key_t key = ftok(homedir, PROJ);
    if ((server_qid = msgget(key, 0666 | IPC_CREAT)) == -1){
        printf("Can't create message queue!\n");
        exit(1);
    }
    printf("\nCreated server message queue!\n");

    my_msbuf rec_mess;
    while(1){
        if ((msgrcv(server_qid, &rec_mess, MSGSZ, MSG_TYP, 0)) == -1){
            printf("Failed to receive a message!\n");
            exit(1);
        }
        
        switch (rec_mess.mtype){
            case INIT:
                handle_init(rec_mess.sender_qid);
                break;
            
            case STOP:
                handle_stop(rec_mess.client_id);
                break;
            
            case LIST:
                handle_list(rec_mess.client_id);
                break;

            case ALL:
                handle_all(rec_mess.client_id, rec_mess.mtext);
                break;

            case ONE:
                handle_one(rec_mess.client_id, rec_mess.requested_client_id, rec_mess.mtext);
                break;

            default:
                break;
        }
    }

    return 0;
}