#include "arena.h"
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>

User users[MAX_USER];
int user_count = 0;

int waiting_pid = -1;
char waiting_name[50];

// Cari user
int findUser(const char *u){
    for(int i=0;i<user_count;i++){
        if(strcmp(users[i].username,u)==0) return i;
    }
    return -1;
}

int main(){
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);

    printf("Orion is ready (PID: %d)\n", getpid());

    while(1){
        MsgRequest req;
        msgrcv(msgid, &req, sizeof(req)-sizeof(long), 1, 0);

        MsgResponse res;
        res.type = TYPE_RESPONSE + req.pid;

        // Register
        if(req.action==1){
            if(findUser(req.username)!=-1){
                res.success=0;
                strcpy(res.message,"Username exists");
            }else{
                strcpy(users[user_count].username, req.username);
                strcpy(users[user_count].password, req.password);
                users[user_count].is_logged_in=0;
                user_count++;

                res.success=1;
                strcpy(res.message,"Register success");
            }
            msgsnd(msgid,&res,sizeof(res)-sizeof(long),0);
        }

        // Login
        else if(req.action==2){
            int idx = findUser(req.username);

            if(idx==-1){
                res.success=0;
                strcpy(res.message,"User not found");
            }
            else if(strcmp(users[idx].password,req.password)!=0){
                res.success=0;
                strcpy(res.message,"Wrong password");
            }
            else if(users[idx].is_logged_in){
                res.success=0;
                strcpy(res.message,"Already login");
            }
            else{
                users[idx].is_logged_in=1;
                res.success=1;
                strcpy(res.message,"Welcome!");
            }

            msgsnd(msgid,&res,sizeof(res)-sizeof(long),0);
        }

        // Matchmaking
        else if(req.action==3){

            if(waiting_pid == -1){
                waiting_pid = req.pid;
                strcpy(waiting_name, req.username);
            }
            else if(waiting_pid != req.pid){

                int p1 = waiting_pid;
                int p2 = req.pid;

                MatchMsg m1;
                m1.type = TYPE_MATCH + p1;
                m1.my_slot = 0;
                m1.enemy_pid = p2;
                strcpy(m1.enemy_name, req.username);

                MatchMsg m2;
                m2.type = TYPE_MATCH + p2;
                m2.my_slot = 1;
                m2.enemy_pid = p1;
                strcpy(m2.enemy_name, waiting_name);

                msgsnd(msgid,&m1,sizeof(m1)-sizeof(long),0);
                msgsnd(msgid,&m2,sizeof(m2)-sizeof(long),0);

                waiting_pid = -1;
            }
        }
    }
}
