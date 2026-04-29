#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "protocol.h"

#define PORT 8080

int sock;
char myname[MAX_NAME];
int is_admin = 0;

// Ctrl+C
void handle_exit(int sig){
    printf("\n[System] Disconnecting...\n");
    close(sock);
    exit(0);
}

// Receive thread
void *recv_handler(void *arg){
    Message m;
    while(recv(sock,&m,sizeof(m),0)>0){
        printf("\r\033[K");
        printf("[%s] %s\n", m.name, m.msg);

        if(is_admin)
            printf("Command >> ");
        else
            printf("> ");

        fflush(stdout);
    }
    return NULL;
}

int main(){
    signal(SIGINT, handle_exit);

    struct sockaddr_in serv;
    Message m;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    inet_pton(AF_INET,"127.0.0.1",&serv.sin_addr);

    connect(sock,(struct sockaddr*)&serv,sizeof(serv));

    // Login
    while(1){
        printf("Enter your name: ");
        fgets(myname,MAX_NAME,stdin);
        myname[strcspn(myname,"\n")] = 0;

        strcpy(m.name,myname);
        strcpy(m.msg,"");

        send(sock,&m,sizeof(m),0);
        recv(sock,&m,sizeof(m),0);

        if(strcmp(m.msg,"NAME_TAKEN")==0){
            printf("[System] Name already used.\n");
            continue;
        }
        break;
    }

    // Admin
    if(strcmp(myname,"The Knights")==0){
        char pass[50];

        printf("Enter Password: ");
        fgets(pass,50,stdin);
        pass[strcspn(pass,"\n")] = 0;

        strcpy(m.msg,pass);
        send(sock,&m,sizeof(m),0);
        recv(sock,&m,sizeof(m),0);        

        if(strcmp(m.msg,"AUTH_OK")==0){
            is_admin = 1;

            printf("\n[System] Authentication successful. Granted Admin privileges.\n\n");

            printf("=== THE KNIGHTS CONSOLE ===\n");
            printf("1. Check Active Entities (Users)\n");
            printf("2. Check Server Uptime\n");
            printf("3. Execute Emergency Shutdown\n");
            printf("4. Disconnect\n");
        }
    }

    pthread_t tid;
    pthread_create(&tid,NULL,recv_handler,NULL);

    if(is_admin)
        printf("Command >> ");
    else
        printf("> ");
    fflush(stdout);

    while(1){
        if(fgets(m.msg,MAX_MSG,stdin)==NULL){
            break;
        }
        
        m.msg[strcspn(m.msg,"\n")] = 0;

        strcpy(m.name,myname);
        send(sock,&m,sizeof(m),0);

        if(strcmp(m.msg,"/exit")==0 || strcmp(m.msg,"4")==0){
            break;
        }
    }

    close(sock);
    return 0;
}


