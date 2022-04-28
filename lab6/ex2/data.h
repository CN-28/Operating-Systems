#ifndef DATA_H_
#define DATA_H_

#define MAX_MESS_LEN 1024
#define MAX_CLIENTS_NUM 500
#define MAX_QUEUE_SIZE 10
#define PROJ 'C'
#define LIST_COM "LIST"
#define ALL_COM "2ALL"
#define ONE_COM "2ONE"
#define STOP_COM "STOP"
#define SERVER_MQ_NAME "/server_mq"
#define STOP 1
#define LIST 2
#define ALL 3
#define ONE 4
#define INIT 5

typedef struct my_msbuf {
    long mtype;
    char mtext[MAX_MESS_LEN];
    long sender_qid;
    long client_id;
    long requested_client_id;
    time_t stime;
} my_msbuf;

#endif // DATA_H_