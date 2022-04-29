#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include "data.h"

long client_id;
mqd_t client_qd, server_qd;
char q_name[100];
pid_t pid = -1;

int send_init(){
    char message[MAX_MESS_LEN];
    strcpy(message, q_name);

    if (mq_send(server_qd, message, MAX_MESS_LEN, INIT) == -1){
        printf("Can't send INIT message to server!\n");
        return 1;
    }

    char rec_mess[MAX_MESS_LEN];
    if (mq_receive(client_qd, rec_mess, MAX_MESS_LEN, NULL) == -1){
        printf("Can't receive feedback message!\n");
        return 1;
    }

    client_id = strtol(rec_mess, NULL, 10);
    printf("\nReceived %ld id number from server!\n", client_id);
    return 0;
}

int send_list(){
    char message[MAX_MESS_LEN];
    sprintf(message, "%ld", client_id);

    if (mq_send(server_qd, message, MAX_MESS_LEN, LIST) == -1){
        printf("Can't send LIST message to server!\n");
        return 1;
    }

    printf("LIST message sent from %ld client to server!\n", client_id);
    if (mq_receive(client_qd, message, MAX_MESS_LEN, NULL) == -1){
        printf("Failed to receive list of active clients from server!\n");
        return 1;
    }
    
    printf("List of active clients: \n%s\n", message);
    return 0;
}

int send_all_or_one(long type, long request_client_id, char *message){
    char mess[MAX_MESS_LEN];
    sprintf(mess, "%ld %ld %s", request_client_id, client_id, message);
    
    if (mq_send(server_qd, mess, MAX_MESS_LEN, type) == -1){
        printf("Can't send %s message to server!\n", (type == ALL) ? (ALL_COM) : (ONE_COM));
        return 1;
    }

    printf("\n%s sent from %ld client to server message queue!\n", (type == ALL) ? (ALL_COM) : (ONE_COM), client_id);
    return 0;
}

int send_stop(){
    char message[MAX_MESS_LEN];
    sprintf(message, "%ld", client_id);
    if (mq_send(server_qd, message, MAX_MESS_LEN, STOP) == -1){
        printf("Can't send STOP message to the server!\n");
        exit(1);
    }

    printf("\nSTOP sent from %ld client to the server!\n", client_id);
    if (pid > 0)
        kill(pid, SIGINT);
    else if (pid == 0)
        kill(getppid(), SIGINT);

    exit(0);
}

int handle_all_one(char *message){
    char *rest = NULL;
    long sender_id = strtol(strtok_r(message, " \n", &rest), NULL, 10);
    time_t curr_time = (time_t) strtol(strtok_r(rest, " ", &rest), NULL, 10);
    struct tm *info = localtime(&curr_time);
    printf("Received message from %ld at: %s", sender_id, asctime(info));
    printf("Message: \n%s", rest);
    return 0;
}

void listen_input_and_messages(){
    char *line = NULL, *command = NULL, *arg = NULL, *message = NULL, *rest = NULL, *client_id = NULL;
    size_t line_len = 0;
    int read_next = 0, client_id_num, parsed = 0;
    pid = fork();
    if (pid == 0){
        // get input from the client
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
        char message[MAX_MESS_LEN];
        struct mq_attr attr;
        unsigned int prio;
        while (1){
            // check messages from clients/server
            if (mq_getattr(client_qd, &attr) == -1){
                printf("Can't get message queue attributes!\n");
                continue;
            }            
            
            if (attr.mq_curmsgs > 0){
                if (mq_receive(client_qd, message, MAX_MESS_LEN, &prio) == -1){
                    printf("Failed to receive a message from clients!\n");
                    continue;
                }

                switch (prio){
                    case STOP:
                        send_stop();
                        break;
                    case ALL: case ONE:
                        handle_all_one(message);
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

void set_sigaction(){
    struct sigaction act;
    act.sa_handler = &sigint_handler;
    sigaction(SIGINT, &act, NULL);
}

void handle_exit(){
    mq_close(server_qd);
    mq_close(client_qd);
    mq_unlink(q_name);
}

int main(){
    atexit(handle_exit);
    set_sigaction();

    // create client message queue
    sprintf(q_name, "/client_mq_%ld", (long) getpid());
    struct mq_attr attr = {
        .mq_msgsize = MAX_MESS_LEN,
        .mq_maxmsg = MAX_QUEUE_SIZE
    };
    if ((client_qd = mq_open(q_name, O_RDONLY | O_CREAT, 0666, &attr)) == -1){
        printf("Can't create client message queue!\n");
        exit(1);
    }
    printf("Created client message queue!\n");
    
    // open server message queue
    if ((server_qd = mq_open(SERVER_MQ_NAME, O_RDWR)) == -1){
        printf("Can't open server message queue!\n");
        exit(1);
    }

    send_init();
    listen_input_and_messages();

    return 0;
}