#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "protocol.h"

#define PORT 8080
#define MAX_CLIENT 100

int clients[MAX_CLIENT];
char names[MAX_CLIENT][MAX_NAME];
int client_count = 0;

time_t start_time;

// Log
void log_event(const char *role, const char *msg){
    FILE *f = fopen("history.log","a");
    if(!f) return;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    fprintf(f,
        "[%04d-%02d-%02d %02d:%02d:%02d] [%s] %s\n",
        tm->tm_year+1900,
        tm->tm_mon+1,
        tm->tm_mday,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec,
        role,
        msg
    );

    fclose(f);
}

// Cek Nama 
int name_exist(const char *name){
    for(int i=0;i<client_count;i++){
        if(strcmp(names[i], name)==0) return 1;
    }
    return 0;
}

// Broadcast 
void broadcast(Message *m, int sender){
    for(int i=0;i<client_count;i++){
        if(clients[i]!=sender){
            send_message(clients[i], m);
        }
    }
}

// Hapus client dari list
void remove_client(int sock){
    for(int i=0;i<client_count;i++){
        if(clients[i]==sock){
            for(int j=i;j<client_count-1;j++){
                clients[j]=clients[j+1];
                strcpy(names[j], names[j+1]);
            }
            client_count--;
            break;
        }
    }
}

void *client_handler(void *arg){
    int sock = *(int*)arg;
    free(arg);

    Message m;

    // Login
    while(1){
        if(recv_message(sock,&m)<=0){
            close(sock);
            return NULL;
        }

        if(name_exist(m.name)){
            strcpy(m.msg,"NAME_TAKEN");
            send_message(sock,&m);
            continue;
        }

        clients[client_count]=sock;
        strcpy(names[client_count],m.name);
        client_count++;

        strcpy(m.msg,"OK");
        send_message(sock,&m);

        char logbuf[200];
        sprintf(logbuf,"User '%s' connected", m.name);
        log_event("System", logbuf);

        break;
    }

    int is_admin = 0;

    // Admin
    if(strcmp(m.name,"The Knights")==0){
        Message pass;
        recv_message(sock,&pass);

        is_admin = 1; 

        strcpy(pass.msg,"AUTH_OK");
        send_message(sock,&pass);
    }

    // Welcome
    if(!is_admin){
        Message sys;
        strcpy(sys.name,"System");
        sprintf(sys.msg,"--- Welcome to The Wired, %s ---", m.name);
        send_message(sock,&sys);
    }

    // Loop
    while(recv_message(sock,&m)>0){

        // Disconnect
        if(strcmp(m.msg,"/exit")==0 || strcmp(m.msg,"4")==0){
            char logbuf[200];
            sprintf(logbuf,"User '%s' disconnected", m.name);
            log_event("System", logbuf);
            break;
        }

        // Admin Command
        if(is_admin){

            if(strcmp(m.msg,"1")==0){
                log_event("Admin","RPC_GET_USERS");

                Message reply;
                strcpy(reply.name,"Admin");

                char list[256]="User aktif:\n";
                for(int i=0;i<client_count;i++){
                    strcat(list,names[i]);
                    strcat(list,"\n");
                }

                strcpy(reply.msg,list);
                send_message(sock,&reply);
                continue;
            }

            if(strcmp(m.msg,"2")==0){
                log_event("Admin","RPC_GET_UPTIME");

                Message reply;
                strcpy(reply.name,"Admin");

                time_t now = time(NULL);
                int uptime = (int)(now - start_time);

                sprintf(reply.msg,"Server uptime: %d seconds", uptime);
                send_message(sock,&reply);
                continue;
            }

            if(strcmp(m.msg,"3")==0){
                log_event("Admin","RPC_SHUTDOWN");
                log_event("System","EMERGENCY SHUTDOWN INITIATED");

                Message sys;
                strcpy(sys.name,"System");
                strcpy(sys.msg,"SERVER SHUTDOWN");

                broadcast(&sys,sock);
                exit(0);
            }
        }

        // Log Chat
        char logbuf[300];
        sprintf(logbuf,"[%s]: %s", m.name, m.msg);
        log_event("User", logbuf);

        broadcast(&m,sock);
    }

    close(sock);
    remove_client(sock);
    return NULL;
}

int main(){
    int server_fd, new_socket;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    start_time = time(NULL);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd,(struct sockaddr*)&addr,sizeof(addr));
    listen(server_fd,10);

    printf("Server running...\n");
    log_event("System","SERVER ONLINE");

    while(1){
        new_socket = accept(server_fd,(struct sockaddr*)&addr,&addrlen);

        int *p = malloc(sizeof(int));
        *p = new_socket;

        pthread_t tid;
        pthread_create(&tid,NULL,client_handler,p);
    }
}
