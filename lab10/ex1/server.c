// stream sockets version
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <pthread.h>
#include "common.h"
#define MAX_CLIENTS 16
#define MAX_EVENTS 16
#define UNIX_PATH_MAX 108

pthread_t pinging_thread, monitoring_thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int epoll_fd, loc_sockfd, net_sockfd;
char clients_names[MAX_CLIENTS][1024];
int clients_sock_fds[MAX_CLIENTS] = {0};
int responded[MAX_CLIENTS];
int opp_fds[MAX_CLIENTS];
board game_board, ping_board;
struct epoll_event ev, events[MAX_EVENTS];

void *monitor(void *ptr){
    char buff[1024];
    for (int i = 0; i < MAX_CLIENTS; i++)
        bzero(clients_names[i], 1024);

    srand(time(NULL));
    int ev_cnt, client_fd, waiting_client = -1, waiting_client_nick_id, temp, free_nick_id, opp_fd, game_ended;
    struct sockaddr client_addr;
    socklen_t addr_size;
    while(1){
        if ((ev_cnt = epoll_wait(epoll_fd, events, MAX_EVENTS, -1)) == -1)
            printf("epoll_wait error\n");

        for (int i = 0; i < ev_cnt; i++){
            if ((events[i].events & EPOLLIN) == EPOLLIN){
                // request for connection
                if (events[i].data.fd == net_sockfd || events[i].data.fd == loc_sockfd){
                    if ((client_fd = accept((events[i].data.fd == net_sockfd) ? net_sockfd : loc_sockfd, &client_addr, &addr_size)) == -1)
                        printf("accept error\n");

                    // read nickname
                    pthread_mutex_lock(&mutex);
                    read(client_fd, &buff, 1024);
                    temp = 0;
                    for (int j = 0; j < MAX_CLIENTS; j++){
                        if (!strcmp(clients_names[j], buff)){
                            temp = 1;
                            break;
                        }

                        if (strlen(clients_names[j]) == 0)
                            free_nick_id = j;
                    }

                    if (!temp){
                        // register client
                        ev.events = EPOLLIN;
                        ev.data.fd = client_fd;
                        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
                            printf("epoll_ctl error\n");
                            
                        clients_sock_fds[free_nick_id] = client_fd;
                        strcpy(clients_names[free_nick_id], buff);
                        printf("Client with nickname: %s connected!\n", buff);
                    }
                    else{
                        game_board.nick_occupied = 1;
                        write(client_fd, &game_board, sizeof(game_board));
                        pthread_mutex_unlock(&mutex);
                        continue;
                    }
                    

                    // assign to the board
                    if (waiting_client == -1){
                        waiting_client = client_fd;
                        waiting_client_nick_id = free_nick_id;
                    }
                    else{
                        memset(game_board.game, ' ', 9);
                        int turn = rand() % 2;
                        game_board.fd = (turn) ? client_fd : waiting_client;
                        game_board.opp_fd = (turn) ? waiting_client : client_fd;
                        game_board.sign = 'O';
                        game_board.win = -1;
                        game_board.interrupted = -1;
                        game_board.ping = -1;
                        game_board.nick_occupied = 0;
                        game_board.nick_ind = (turn) ? free_nick_id : waiting_client_nick_id;
                        game_board.opp_nick_ind = (turn) ? waiting_client_nick_id : free_nick_id;
            
                        write(game_board.fd, &game_board, sizeof(game_board));
                        int temp_fd = game_board.fd;
                        game_board.fd = game_board.opp_fd;
                        game_board.opp_fd = temp_fd;
                        write(game_board.fd, &game_board, sizeof(game_board));
                        game_board.opp_fd = game_board.fd;
                        game_board.fd = temp_fd;

                        opp_fds[game_board.nick_ind] = game_board.opp_nick_ind;
                        opp_fds[game_board.opp_nick_ind] = game_board.nick_ind;

                        write(game_board.fd, &game_board, sizeof(game_board));
                        waiting_client = -1;
                    }
                    pthread_mutex_unlock(&mutex);
                }
                // receive data from existing connection
                else{
                    read(events[i].data.fd, &game_board, sizeof(game_board));
                    
                    pthread_mutex_lock(&mutex);
                    if (!game_board.ping){
                        for (int j = 0; j < MAX_CLIENTS; j++){
                            if (clients_sock_fds[j] == events[i].data.fd)
                                responded[j] = 1;
                        }
                        pthread_mutex_unlock(&mutex);
                        continue;
                    }

                    if (game_board.interrupted == 1){
                        write(game_board.opp_fd, &game_board, sizeof(game_board));
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, game_board.fd, NULL);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, game_board.opp_fd, NULL);

                        opp_fds[game_board.nick_ind] = -1;
                        opp_fds[game_board.opp_nick_ind] = -1;
                        clients_sock_fds[game_board.nick_ind] = 0;
                        clients_sock_fds[game_board.opp_nick_ind] = 0;
                        printf("The game between %s and %s has been ended due to disconnection!\n", clients_names[game_board.nick_ind], clients_names[game_board.opp_nick_ind]);
                        bzero(clients_names[game_board.nick_ind], 1024);
                        bzero(clients_names[game_board.opp_nick_ind], 1024);
                        game_board.interrupted = -1;
                        pthread_mutex_unlock(&mutex);
                        continue;
                    }

                    // check if game has not ended
                    // check lines and rows
                    game_ended = 0;
                    for (int j = 0; j < 9; j += 3){
                        // check rows
                        for (int k = 1; k < 3; k++){
                            if (game_board.game[j + k] == ' ' || game_board.game[j + k] != game_board.game[j + k - 1])
                                break;
                            
                            if (k == 2) game_ended = 1;
                        }

                        // check columns
                        for (int k = 1; k < 3; k++){
                            if (game_board.game[j / 3 + 3 * k] == ' ' || game_board.game[j / 3 + 3 * k] != game_board.game[j / 3 + 3 * (k - 1)])
                                break;

                            if (k == 2) game_ended = 1;
                        }

                        // check diagonals
                        if (game_board.game[4] != ' ' && ((game_board.game[0] == game_board.game[4] && game_board.game[4] == game_board.game[8]) ||\
                            (game_board.game[2] == game_board.game[4] && game_board.game[4] == game_board.game[6])))
                            game_ended = 1;
                    }
                    // check if game was drawn
                    if (!game_ended){
                        for (int j = 0; j < 9; j++){
                            if (game_board.game[j] == ' ')
                                break;
                            
                            if (j == 8) game_ended = 2;
                        }
                    }


                    if (game_ended){
                        if (game_ended == 2) game_board.win = 2;
                        else game_board.win = 1;
                        write(game_board.fd, &game_board, sizeof(game_board));
                        if (game_ended == 2) game_board.win = 2;
                        else game_board.win = 0;
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, game_board.fd, NULL);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, game_board.opp_fd, NULL);

                        opp_fds[game_board.nick_ind] = -1;
                        opp_fds[game_board.opp_nick_ind] = -1;
                        clients_sock_fds[game_board.nick_ind] = 0;
                        clients_sock_fds[game_board.opp_nick_ind] = 0;
                        bzero(clients_names[game_board.nick_ind], 1024);
                        bzero(clients_names[game_board.opp_nick_ind], 1024);
                    }

                    opp_fd = game_board.opp_fd;
                    game_board.opp_fd = game_board.fd;
                    game_board.fd = opp_fd;
                    game_board.sign = (game_board.sign == 'O') ? 'X' : 'O';
                    write(game_board.fd, &game_board, sizeof(game_board));
                    pthread_mutex_unlock(&mutex);
                }
            }
        }
    }
}

void *ping(void *ptr){
    ping_board.ping = 1;
    memset(ping_board.game, ' ', 9);
    for (int i = 0; i < MAX_CLIENTS; i++){
        responded[i] = -1;
        opp_fds[i] = -1;
    }
    
    while(1){
        
        pthread_mutex_lock(&mutex);
        for (int j = 0; j < MAX_CLIENTS; j++){
            if (clients_sock_fds[j] != 0){
                responded[j] = 0;
                write(clients_sock_fds[j], &ping_board, sizeof(ping_board));
            }
        }
        pthread_mutex_unlock(&mutex);
        
        // give clients some time to respond to ping
        sleep(7);
        ping_board.interrupted = 1;
        
        // check if responded
        pthread_mutex_lock(&mutex);
        for (int j = 0; j < MAX_CLIENTS; j++){
            if (clients_sock_fds[j] != 0 && !responded[j]){
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, clients_sock_fds[j], NULL);
                bzero(clients_names[j], 1024);
                write(clients_sock_fds[j], &ping_board, sizeof(ping_board));
                clients_sock_fds[j] = 0;

                if (opp_fds[j] != -1){
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, clients_sock_fds[opp_fds[j]], NULL);
                    bzero(clients_names[opp_fds[j]], 1024);
                    ping_board.ping = 0;
                    write(clients_sock_fds[opp_fds[j]], &ping_board, sizeof(ping_board));
                    ping_board.ping = 1;
                    clients_sock_fds[opp_fds[j]] = 0;
                    opp_fds[opp_fds[j]] = -1;
                    opp_fds[j] = -1;
                }
            }
            responded[j] = -1;
        }
        ping_board.interrupted = -1;
        pthread_mutex_unlock(&mutex);
    }
}


int main(int argc, char *argv[]){
    if (argc != 3){
        printf("Wrong number of arguments!\n");
        exit(EXIT_FAILURE);
    }
    
    int port_number = (int) atoi(argv[1]);
    char *unix_socket_path = argv[2];

    // creating sockets
    int loc_domain = AF_UNIX, net_domain = AF_INET, type = SOCK_STREAM;
    unlink(unix_socket_path);
    loc_sockfd = socket(loc_domain, type, 0);
    net_sockfd = socket(net_domain, type, 0);

    // assigning sockets to addresses
    struct sockaddr_in net_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port_number),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };

    struct sockaddr_un loc_addr = {.sun_family = AF_UNIX};
    strncpy(loc_addr.sun_path, unix_socket_path, strlen(unix_socket_path) + 1);

    // bind servers
    if (bind(loc_sockfd, (struct sockaddr*) &loc_addr, sizeof(loc_addr)) == -1){
        printf("bind error\n");
        exit(EXIT_FAILURE);
    }

    if (bind(net_sockfd, (struct sockaddr*) &net_addr, sizeof(net_addr)) == -1){
        printf("bind error\n");
        exit(EXIT_FAILURE);
    }
    
    // listen loc_sock
    if (listen(loc_sockfd, MAX_CLIENTS) == -1){
        printf("listen error\n");
        exit(EXIT_FAILURE);
    }

    // listen net_sock
    if (listen(net_sockfd, MAX_CLIENTS) == -1){
        printf("listen error\n");
        exit(EXIT_FAILURE);
    }

    // create event handler
    if ((epoll_fd = epoll_create1(0)) == -1){
        printf("epoll_create1 error\n");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = net_sockfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, net_sockfd, &ev) == -1){
        printf("epoll_ctl error\n");
        exit(EXIT_FAILURE);
    }

    ev.data.fd = loc_sockfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, loc_sockfd, &ev) == -1){
        printf("epoll_ctl error\n");
        exit(EXIT_FAILURE);
    }
    

    // handle monitoring descriptors and on other thread ping registered clients
    if (pthread_create(&monitoring_thread, NULL, monitor, NULL) != 0){
        printf("pthread_create error\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&pinging_thread, NULL, ping, NULL) != 0){
        printf("pthread_create error\n");
        exit(EXIT_FAILURE);
    }

    printf("Local server is waiting for the clients...\nSocket path: %s\n\n", unix_socket_path);
    printf("Network server is waiting for the clients...\nPort number: %d\n\n", port_number);
    
    if(pthread_join(monitoring_thread, NULL) != 0){
        printf("pthread join error\n");
        exit(EXIT_FAILURE);
    }

    if(pthread_join(pinging_thread, NULL) != 0){
        printf("pthread join error\n");
        exit(EXIT_FAILURE);
    }

    if (shutdown(loc_sockfd, SHUT_RDWR) == -1){
        printf("Can't shutdown provided socket!\n");
        exit(EXIT_FAILURE);
    }

    if (shutdown(net_sockfd, SHUT_RDWR) == -1){
        printf("Can't shutdown provided socket!\n");
        exit(EXIT_FAILURE);
    }
    
    if (close(loc_sockfd) == -1){
        printf("Can't close provided socket!\n");
        exit(EXIT_FAILURE);
    }

    if (close(net_sockfd) == -1){
        printf("Can't close provided socket!\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}