#include <sys/socket.h>
#include "protocol.h"

int send_message(int sock, Message *m){
    return send(sock, m, sizeof(Message), 0);
}

int recv_message(int sock, Message *m){
    return recv(sock, m, sizeof(Message), 0);
}
