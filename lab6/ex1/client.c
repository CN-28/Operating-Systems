#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include "data.h"


long client_id;
int client_qid, server_qid;
pid_t pid = -1;
void send_init(){
    my_msbuf buf = {
        .mtype = INIT,
        .sender_qid = client_qid
    };

    if (msgsnd(server_qid, &buf, MSGSZ, 0) == -1){
        printf("Can't send INIT message to the server!\n");
        exit(1);
    }

    my_msbuf rec_buf;
    if (msgrcv(client_qid, &rec_buf, MSGSZ, INIT, 0) == -1){
        printf("Failed to receive the message with id from server!\n");
        exit(1);
    }

    client_id = rec_buf.client_id;
    printf("\nReceived %ld id number from server!\n", client_id);
};

void send_list(){
    my_msbuf buf = {
        .mtype = LIST,
        .client_id = client_id
    };

    if (msgsnd(server_qid, &buf, MSGSZ, 0) == -1){
        printf("Can't send %s message to server!\n", LIST_COM);
        exit(1);
    }

    printf("\n%s message sent from %ld to server message queue!\n", LIST_COM, client_id);
    my_msbuf rec_mes;
    if (msgrcv(client_qid, &rec_mes, MSGSZ, LIST, 0) == -1){
        printf("Can't get the active list of clients from server!\n");
        exit(1);
    }

    printf("List of active clients: \n%s\n", rec_mes.mtext);
}

void send_all_or_one(long mtype, long requested_client_id, char *message){
    my_msbuf buf = {
        .mtype = mtype,
        .sender_qid = client_qid,
        .requested_client_id = requested_client_id,
        .client_id = client_id,
    };
    strcpy(buf.mtext, message); 

    if (msgsnd(server_qid, &buf, MSGSZ, 0) == -1){
        printf("Can't send %s message to server!\n", (mtype == ALL) ? (ALL_COM) : (ONE_COM));
        exit(1);
    }

    printf("\n%s sent from %ld client to server message queue!\n", (mtype == ALL) ? (ALL_COM) : (ONE_COM), client_id);
}

void send_stop(){
    my_msbuf buf = {
        .mtype = STOP,
        .sender_qid = client_qid,
        .client_id = client_id,
    };

    if (msgsnd(server_qid, &buf, MSGSZ, 0) == -1){
        printf("Can't send %s message to server!\n", STOP_COM);
        exit(1);
    }
    printf("\n%s sent from %ld to server message queue!\n", STOP_COM, client_id);
    if (pid > 0)
        kill(pid, SIGKILL);
    else if (pid == 0)
        kill(getppid(), SIGKILL);

    exit(0);
}

void handle_all_one(long client_id, char text[], time_t curr_time){
    printf("\nReceived the message from %ld client!\n", client_id);
    struct tm *info = localtime(&curr_time);
    printf("Received at: %s", asctime(info));
    printf("Message: \n%s", text);
}

void listen_input_and_messages(){
    char *line = NULL, *command = NULL, *arg = NULL, *message = NULL, *rest = NULL, *client_id = NULL;
    size_t line_len = 0;
    int read_next = 0, client_id_num, parsed = 0;
    pid = fork();
    if (pid == 0){
        while (1){
            message = NULL;
            command = NULL;
            client_id = NULL;
            parsed = 0;
            getline(&line, &line_len, stdin);
            command = strtok_r(line, " \n", &rest);
            if (strcmp(command, LIST_COM) && strcmp(command, ALL_COM) && strcmp(command, ONE_COM) && strcmp(command, STOP_COM)){
                printf("Provided command is incorrect!\n");
                continue;
            }

            if (!strcmp(command, ALL_COM)){
                message = rest;
                parsed = 1;
            }

            while (!parsed && (arg = strtok_r(NULL, " \n", &rest)) != NULL){
                if (!strcmp(command, STOP_COM) || !strcmp(command, LIST_COM)){
                    printf("%s command doesn't take any arguments!\n", command);
                    read_next = 1;
                    break;
                }

                if (!strcmp(command, ONE_COM)){
                    client_id = arg;
                    message = rest;

                    for (int i = 0; i < strlen(client_id); i++){
                        if (!isdigit(client_id[i])){
                            printf("Provided %s client_id is not a number!\n", client_id);
                            read_next = 1;
                            break;
                        }
                    }
                    
                    if (read_next) break;
                    else {
                        client_id_num = atoi(client_id);
                        if (client_id_num < 0 || client_id_num >= MAX_CLIENTS_NUM){
                            printf("Provided client_id is out of range of [0 - %d]", MAX_CLIENTS_NUM - 1);
                            read_next = 1;
                        }
                    }
                    break;
                }
            }

            if (read_next){
                read_next = 0;
                continue;
            }
            
            if (!strcmp(command, LIST_COM))
                send_list();
            else if (!strcmp(command, ALL_COM) && message != NULL)
                send_all_or_one(ALL, -1, message);
            else if (!strcmp(command, ONE_COM) && message != NULL)
                send_all_or_one(ONE, client_id_num, message);
            else if (!strcmp(command, STOP_COM))
                send_stop();
        }
    }
    else{
        my_msbuf rec_mess;
        struct msqid_ds q_stat;
        while (1){
            msgctl(client_qid, IPC_STAT, &q_stat);
            if (q_stat.msg_qnum > 0){
                if (msgrcv(client_qid, &rec_mess, MSGSZ, MSG_TYP, 0) == -1){
                    printf("Failed to receive the message from the server!\n");
                    exit(1);
                }

                switch (rec_mess.mtype){
                    case STOP:
                        send_stop();
                        break;
                    case ALL: case ONE:
                        handle_all_one(rec_mess.client_id, rec_mess.mtext, rec_mess.stime);
                        break;
                    default:
                        break;
                }
            }
        }
    } 
}

void sigint_handler(int signum){
    send_stop();
}

void set_up_sigaction(){
    struct sigaction act;
    act.sa_handler = &sigint_handler;
    sigaction(SIGINT, &act, NULL);
}

void delete_queue(){
    msgctl(client_qid, IPC_RMID, NULL);
    printf("Deleted queue at exit!\n");
}

int main(){
    // get $HOME path
    char *homedir;
    if((homedir = getenv("HOME")) == NULL){
        printf("The $HOME variable is not set!\n");
        exit(1);
    }
    
    // set up special behavioru for SIGINT
    set_up_sigaction();

    // create client IPC
    key_t client_key = ftok(homedir, getpid());
    if ((client_qid = msgget(client_key, 0666 | IPC_CREAT)) == -1){
        printf("Can't create message queue!\n");
        exit(1);
    }
    printf("Created client message queue with %d qid!\n", client_qid);

    atexit(delete_queue);

    // get server IPC id
    key_t server_key = ftok(homedir, PROJ);
    if ((server_qid = msgget(server_key, 0)) == -1){
        printf("Server message queue doesn't exist!\n");
        exit(1);
    }

    // send ipc_id to server with INIT announcement and get back id from server
    send_init(client_qid, server_qid);
    listen_input_and_messages();

    return 0;
}