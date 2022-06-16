#ifndef COMMON_FILE
#define COMMON_FILE

typedef struct Board {
    char game[9];
    int fd;
    int opp_fd;
    char sign;
    int win;
    int interrupted;
    int nick_occupied;
    int ping;
    int nick_ind;
    int opp_nick_ind;
} board;

#endif