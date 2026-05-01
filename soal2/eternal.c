// Revisi Kode

#include "arena.h"
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/select.h>

// Player
typedef struct {
    char username[50];
    int gold;
    int xp;
    int weapon;
    char weapon_name[50];
} Player;

// History
typedef struct {
    char time[20];
    char opponent[50];
    char result[10];
    int xp;
} History;

History history_list[100];
int history_count = 0;

char enemy_name[50]; // Musuh

int msgid;
SharedArena *arena;

// Total Damage
int getDamage(Player *p){
    return 10 + (p->xp/50) + p->weapon;
}

void clear_stdin(){
    int c;
    while((c=getchar())!='\n' && c!=EOF);
}

void press_any_key(){
    printf("\nPress any key...");
    clear_stdin();
    getchar();
}

// Room
int acquire_room(int my_pid, int enemy_pid, int *my_slot){
    sem_wait(&arena->global_lock);

    for(int i=0;i<MAX_ROOMS;i++){
        if(arena->rooms[i].used){
            if( (arena->rooms[i].pid1==my_pid && arena->rooms[i].pid2==enemy_pid) ||
                (arena->rooms[i].pid1==enemy_pid && arena->rooms[i].pid2==my_pid) ){

                if(arena->rooms[i].pid1 == my_pid) *my_slot = 0;
                else *my_slot = 1;

                sem_post(&arena->global_lock);
                return i;
            }
        }
    }

    if(my_pid < enemy_pid){
        for(int i=0;i<MAX_ROOMS;i++){
            if(!arena->rooms[i].used){
                arena->rooms[i].used = 1;
                arena->rooms[i].pid1 = my_pid;
                arena->rooms[i].pid2 = enemy_pid;
                arena->rooms[i].hp1 = 100;
                arena->rooms[i].hp2 = 100;
                sem_init(&arena->rooms[i].lock,1,1);

                *my_slot = 0;

                sem_post(&arena->global_lock);
                return i;
            }
        }
    }

    sem_post(&arena->global_lock);

    // Wait room
    for(int retry=0; retry<20; retry++){
        usleep(100000);

        sem_wait(&arena->global_lock);
        for(int i=0;i<MAX_ROOMS;i++){
            if(arena->rooms[i].used){
                if(arena->rooms[i].pid1==enemy_pid && arena->rooms[i].pid2==my_pid){
                    *my_slot = 1;
                    sem_post(&arena->global_lock);
                    return i;
                }
            }
        }
        sem_post(&arena->global_lock);
    }

    return -1;
}

// Battle
void battle_real(int enemy_pid, Player *p){

    int my_pid = getpid();
    int my_slot = 0;

    int room_id = acquire_room(my_pid, enemy_pid, &my_slot);
    if(room_id < 0){
        printf("No battle room.\n");
        return;
    }

    BattleRoom *r = &arena->rooms[room_id];

    time_t last = 0;

    char logs[5][100];
    int li = 0;

    clear_stdin();

    int my_hp, enemy_hp;

    while(1){

        sem_wait(&r->lock);
        if(my_slot==0){
            my_hp = r->hp1;
            enemy_hp = r->hp2;
        } else {
            my_hp = r->hp2;
            enemy_hp = r->hp1;
        }
        sem_post(&r->lock);

        if(my_hp<=0 || enemy_hp<=0){
            break;
        }

        fd_set set;
        struct timeval tv = {0,100000};
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);

        int rv = select(STDIN_FILENO+1,&set,NULL,NULL,&tv);

        if(rv>0 && FD_ISSET(STDIN_FILENO,&set)){
            char c = getchar();

            if(time(NULL)-last >= 1){
                int dmg = getDamage(p);
                
                // Attack
                if(c=='a'){
                    sem_wait(&r->lock);
                    if(my_slot==0) r->hp2 -= dmg;
                    else r->hp1 -= dmg;
                    sem_post(&r->lock);

                    snprintf(logs[li++%5],100,"You hit -%d",dmg);
                    last = time(NULL);
                }

                // Ultimate
                else if(c=='u' && p->weapon>0){
                    sem_wait(&r->lock);
                    if(my_slot==0) r->hp2 -= dmg*3;
                    else r->hp1 -= dmg*3;
                    sem_post(&r->lock);

                    snprintf(logs[li++%5],100,"ULTIMATE -%d",dmg*3);
                    last = time(NULL);
                }
            }
        }

        system("clear");
      
        printf("=== ARENA ===\n\n");

        if(my_slot==0){
            printf("%-8s | Lv %-2d | HP: %-3d\n",
                enemy_name,(p->xp/100)+1,enemy_hp);

            printf("%-8s | Lv %-2d | HP: %-3d | Weapon: %s\n",
                p->username,(p->xp/100)+1,my_hp,
                (p->weapon==0?"None":p->weapon_name));
        }
        else{
            printf("%-8s | Lv %-2d | HP: %-3d\n",
                p->username,(p->xp/100)+1,my_hp);

            printf("%-8s | Lv %-2d | HP: %-3d | Weapon: %s\n",
                enemy_name,(p->xp/100)+1,enemy_hp,
                (p->weapon==0?"None":p->weapon_name));
        }

        printf("\n----- LOG -----\n");
        for(int i=0;i<5;i++){
            int idx = li-1-i;
            if(idx>=0){
                printf("%s\n",logs[idx%5]);
            }
        }

        printf("\n[a]=attack | [u]=ultimate\n");
    }

    char result[10];

    if(my_hp>0){
        printf("\nYOU WIN\n");
        strcpy(result,"WIN");
        p->xp+=50;
        p->gold+=120;
    } 
    else {
        printf("\nYOU LOSE\n");
        strcpy(result,"LOSE");
        p->xp+=15;
        p->gold+=30;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    // Menyimpan match history
    snprintf(history_list[history_count].time,20,"%02d:%02d",t->tm_hour,t->tm_min);
    strcpy(history_list[history_count].opponent, enemy_name);
    strcpy(history_list[history_count].result,result);
    history_list[history_count].xp = p->xp;

    history_count++;

    press_any_key();
}

// Matchmaking
void matchmaking(Player *p){
    MsgRequest req;
    req.type=1;
    req.action=3;
    req.pid=getpid();
    strcpy(req.username,p->username);

    msgsnd(msgid,&req,sizeof(req)-sizeof(long),0);

    printf("Searching...\n");

    for(int i=0;i<35;i++){
        MatchMsg msg;

        if(msgrcv(msgid,&msg,sizeof(msg)-sizeof(long),
           TYPE_MATCH + getpid(),IPC_NOWAIT)!=-1){

            strcpy(enemy_name, msg.enemy_name); // Menyimpan nama musuh

            printf("MATCH FOUND!\n");
            sleep(1);
            battle_real(msg.enemy_pid, p);
            return;
        }

        printf("Time %d\n",i+1);
        sleep(1);
    }

    printf("No player.\n");
}

// History
void show_history(){
    printf("\n=== MATCH HISTORY ===\n");
    printf("%-6s | %-10s | %-5s | %-6s\n","Time","Opponent","Res","XP");
    printf("--------------------------------------\n");

    for(int i=history_count-1;i>=0;i--){
        printf("%-6s | %-10s | %-5s | +%d XP\n",
            history_list[i].time,
            history_list[i].opponent,
            history_list[i].result,
            history_list[i].xp
        );
    }

    press_any_key();
}

int main(){

    msgid = msgget(MSG_KEY,0666);
    if(msgid==-1){
        printf("Orton are you there?\n");
        return 0;
    }

    int shmid = shmget(SHM_KEY,sizeof(SharedArena),IPC_CREAT|0666);
    arena = (SharedArena*) shmat(shmid,NULL,0);
    
    static int initialized = 0;

    if(!initialized){
        sem_init(&arena->global_lock,1,1);
    
      for(int i=0;i<MAX_ROOMS;i++){
          arena->rooms[i].used = 0;
      }
      initialized = 1;
    }

    sem_init(&arena->global_lock,1,1);

    while(1){
        printf("\n1. Register\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Choice: ");

        int c; 
        scanf("%d",&c);

        MsgRequest req;
        req.type=1;
        req.pid=getpid();
        MsgResponse res;

        // Register
        if(c==1){
            printf("Username: "); scanf("%s",req.username);
            printf("Password: "); scanf("%s",req.password);
            req.action=1;

            msgsnd(msgid,&req,sizeof(req)-sizeof(long),0);
            msgrcv(msgid,&res,sizeof(res)-sizeof(long),
                   TYPE_RESPONSE + getpid(),0);

            printf("%s\n",res.message);
        }
        
        // Login
        else if(c==2){
            printf("Username: "); scanf("%s",req.username);
            printf("Password: "); scanf("%s",req.password);
            req.action=2;

            msgsnd(msgid,&req,sizeof(req)-sizeof(long),0);
            msgrcv(msgid,&res,sizeof(res)-sizeof(long),
                   TYPE_RESPONSE + getpid(),0);

            printf("%s\n",res.message);

            if(res.success){
                Player p;
                strcpy(p.username, req.username);
                p.gold=150; p.xp=0; p.weapon=0;
                strcpy(p.weapon_name,"None");

                while(1){
                    printf("\n=== PROFILE ===\n");
                    printf("Username: %s\n", p.username);
                    printf("Gold: %d | Lv: %d | XP: %d\n",
                           p.gold,(p.xp/100)+1,p.xp);

                    printf("\n1. Battle\n");
                    printf("2. Armory\n");
                    printf("3. Match History\n");
                    printf("4. Logout\n");
                    printf("Choice: ");

                    int x; 
                    scanf("%d",&x);

                    if(x==1){
                        matchmaking(&p);
                    }
                    else if(x==2){
                        int price[] = {100,300,600,1500,5000};
                        int dmg[]   = {5,15,30,60,150};
                        char *names[]={
                            "Wood Sword",
                            "Iron Sword",
                            "Steel Axe",
                            "Demon Blade",
                            "God Slayer"
                        };

                        while(1){
                            printf("\nGold: %d\n", p.gold);
                            printf("1. Wood Sword\n");
                            printf("2. Iron Sword\n");
                            printf("3. Steel Axe\n");
                            printf("4. Demon Blade\n");
                            printf("5. God Slayer\n");
                            printf("0. Back\n");
                            printf("Choice: ");

                            int c; 
                            scanf("%d",&c);

                            if(c==0){ 
                                break;
                            }
                            if(p.gold < price[c-1]){
                                printf("Gold tidak cukup!\n");
                                continue;
                            }

                            p.gold -= price[c-1];
                            if(dmg[c-1] > p.weapon){
                                p.weapon = dmg[c-1];
                                strcpy(p.weapon_name,names[c-1]);
                            }
                        }
                    }

                    else if(x==3){
                        show_history();
                    }
                    else{ 
                        break;
                    }
                }
            }
        }
        else{
            break;
        }
    }
}
