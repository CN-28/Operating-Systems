// stream sockets version
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <signal.h>
#include "common.h"
#define NETWORK "network"
#define LOCAL "local"

board game_board;
int sockfd, opp_fd;
void sighandler(int signo){
    game_board.interrupted = 1;
    write(sockfd, &game_board, sizeof(game_board));
    _exit(0);
}


int main(int argc, char *argv[]){
    struct sigaction act;
    act.sa_handler = &sighandler;
    if (sigaction(SIGINT, &act, NULL) == -1){
        printf("sigaction error\n");
        exit(EXIT_FAILURE);
    }

    char *client_name = argv[1];
    char *conn_type = argv[2];
    char *ipv4_addr = NULL, *socket_path = NULL;
    int port_number, domain;
    if (!strcmp(conn_type, NETWORK)){
        if (argc != 5){
            printf("Wrong number of arguments!\n");
            exit(EXIT_FAILURE);
        }
        ipv4_addr = argv[3];
        port_number = atoi(argv[4]);
        domain = AF_INET;
    }
    else if (!strcmp(conn_type, LOCAL)){
        if (argc != 4){
            printf("Wrong number of arguments!\n");
            exit(EXIT_FAILURE);
        }
        socket_path = argv[3];
        domain = AF_UNIX;
    }
    else{
        printf("Provided type of connection is incorrect!\n");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(domain, SOCK_STREAM, 0);
    struct sockaddr_in net_addr;
    struct sockaddr_un loc_addr;
    struct sockaddr *addr;
    if (!strcmp(conn_type, NETWORK)){
        net_addr.sin_family = AF_INET;
        net_addr.sin_port = htons(port_number);
        net_addr.sin_addr.s_addr = inet_addr(ipv4_addr);
        addr = (struct sockaddr*) &net_addr;
    }
    else{
        loc_addr.sun_family = AF_UNIX;
        strncpy(loc_addr.sun_path, socket_path, strlen(socket_path) + 1);
        addr = (struct sockaddr*) &loc_addr;
    }

    if (connect(sockfd, addr, sizeof(*addr)) != 0){
        printf("connect error\n");
        exit(EXIT_FAILURE);
    }
    
    write(sockfd, client_name, 1024);
    read(sockfd, &game_board, sizeof(game_board));
    int cell;
    while(1){
        read(sockfd, &game_board, sizeof(game_board));

        if (game_board.nick_occupied == 1){
            printf("Provided nick is occupied!\n");
            break;
        }

        if (game_board.interrupted == 1){
            if (game_board.ping == 1)
                printf("You have been disconnected due to lack of respond!\n");
            else
                printf("The other player disconnected from the game!\n");
            break;
        }

        if (game_board.ping == 1){
            game_board.ping = 0;
            printf("Responded to ping!\n");
            write(sockfd, &game_board, sizeof(game_board));
            continue;
        }

        // check the status of the game
        if (game_board.win == 1 || game_board.win == 0 || game_board.win == 2){
            if (game_board.win == 1) printf("Congratulations! You won the game!\n");
            else if (game_board.win == 2) printf("The game has been drawn!\n");
            else printf("You lost the game!\n");
            break;
        }

        printf("Your turn! You play with: %c\n", game_board.sign);
        printf("-------------\n");
        for (int i = 0; i < 9; i++){
            if ((i + 1) % 3 == 0){
                printf(" %c |\n", game_board.game[i]);
                printf("-------------\n");
            }
            else
                printf(" %c | ", game_board.game[i]);
        }

        cell = -1;
        while(1){
            printf("Provide cell number: ");
            scanf("%d", &cell);

            if (cell >= 1 && cell <= 9 && game_board.game[cell - 1] == ' ')
                break;
            printf("Provided cell is incorrect!\n");
        }
        
        game_board.game[cell - 1] = game_board.sign;
        write(sockfd, &game_board, sizeof(game_board));
    }


    if (shutdown(sockfd, SHUT_RDWR) == -1){
        printf("Can't shutdown provided socket!\n");
        exit(EXIT_FAILURE);
    }

    if (close(sockfd) == -1){
        printf("Can't close provided socket!\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}