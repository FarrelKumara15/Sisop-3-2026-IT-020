# Sisop-3-2026-IT-020
## Farrel Arteya Kumara - 5027251020
### Soal 1
#### Struktur Repositori Soal 1 di Akhir
<img width="305" height="199" alt="2026-04-29 11:25:45" src="https://github.com/user-attachments/assets/955f4b5f-6c74-4b89-afe1-0be93de49581" />

Program ini merupakan implementasi sistem komunikasi berbasis client-server menggunakan socket TCP dan multithreading dalam bahasa C.
Aplikasi mendukung banyak client secara bersamaan, serta menyediakan role khusus admin (The Knights) dengan fitur tambahan.
<br/>

#### Fitur Utama:
#### User:
1. Login dengan nama unik
2. Chat broadcast ke semua user
3. Keluar dengan /exit
   
#### Admin:
- Login dengan username: The Knights
- Akses dengan console khusus:
1. Check Activate Entities (Users)
2. Check Server Uptime
3. Disconnect

#### history.log
Semua aktivitas dicatat: <br/>
[YYYY-MM-DD HH:MM:SS] [ROLE] MESSAGE <br/><br/>

Contoh:
[2026-04-26 19:06:46] [System] User 'alice' connected
[2026-04-26 19:06:56] [User] [alice]: hello lain
[2026-04-26 19:07:29] [Admin] RPC_GET_USERS
<br/><br/>

#### 1. Struktur Data Pesan (protocol.h)
```bash
typedef struct {
    char name[MAX_NAME];
    char msg[MAX_MSG];
} Message;
```
Digunakan untuk komunikasi antara client dan server.
<br/>

#### 2. Abstraksi Komunikasi (protocol.c)
```bash
int send_message(int sock, Message *m){
    return send(sock, m, sizeof(Message), 0);
}
int recv_message(int sock, Message *m){
    return recv(sock, m, sizeof(Message), 0);
}
```
Membungkus fungsi send dan recv.
<br/>

#### 3. Logging System (wired.c)
```bash
void log_event(const char *role, const char *msg){
    FILE *f = fopen("history.log","a");

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
```
Menyimpan aktivitas ke file history.log.
<br/>

#### 4. Broadcast Message
```bash
void broadcast(Message *m, int sender){
    for(int i=0;i<client_count;i++){
        if(clients[i]!=sender){
            send_message(clients[i], m);
        }
    }
}
```
Mengirim pesan ke semua client.
<br/>

#### 5. Thread Handling Client
```bash
pthread_t tid;
pthread_create(&tid, NULL, client_handler, p);
```
Setiap client dijalankan dalam thread terpisah.
<br/>

#### 6. Admin Command Handling
```bash
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
}
```
Implementasi command admin.
<br/><br/>

#### Jalankan Program
<img width="411" height="169" alt="2026-05-02 23:40:11" src="https://github.com/user-attachments/assets/463de385-276e-431d-831f-107729855af8" /> <br/>
Buka terminal baru <br/>
<img width="418" height="130" alt="2026-04-29 12:31:42" src="https://github.com/user-attachments/assets/a69ad5b3-bb9d-423f-8016-3fdf1a29b22d" /> <br/>
Buka terminal bari <br/>
<img width="413" height="123" alt="2026-04-29 12:32:55" src="https://github.com/user-attachments/assets/052d1604-6e27-458c-a151-ed4918625afc" /> <br/>
Buka terminal baru untuk admin <br/>
<img width="560" height="430" alt="2026-04-29 12:34:41" src="https://github.com/user-attachments/assets/bc80a043-d9fc-44d9-8599-7a0684ce034e" /> <br/>
Disconnecting User <br/>
<img width="417" height="293" alt="2026-04-29 12:36:36" src="https://github.com/user-attachments/assets/29aea362-a415-42d8-a568-352922ce6c9f" /> <br/>
Kondisi jika login user dengan nama yang sama <br/>
<img width="378" height="114" alt="2026-04-29 12:37:35" src="https://github.com/user-attachments/assets/1961cf0a-b37f-499c-a998-ec08c96015b1" /> <br/>
Isi dari 'history.log', setelah selesai bisa langsung dihapus agar struktur repository sesuai dengan soal. <br/>
<img width="545" height="274" alt="2026-04-29 12:38:52" src="https://github.com/user-attachments/assets/8ebdb2d6-3973-4938-9baa-cf77a3ec826f" /> <br/><br/>


### Soal 2
#### Struktur Repositori Soal 2 di Akhir
<img width="310" height="193" alt="2026-05-01 00:25:24" src="https://github.com/user-attachments/assets/c3d46922-a47f-4293-8a8b-ab587326b742" /> <br/>

Game ini bernama **Battle of Eterion**, game multiplayer yang dibuat dengan konsep Inter Process Communication (IPC). <br/>
Game ini bisa,
- Register & Login
- Matchmaking (PvP real-time)
- Battle
- Membeli senjata (Armory)
- Melihat history battle

#### 1. IPC Initialization
```bash
msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
```
Kode diatas berfungsi untuk menghubungkan client <-> server. <br>

#### 2. Request & Response System
**Client mengirim request** <br/>
```bash
req.type = 1;
req.action = 2; // login
req.pid = getpid();

msgsnd(msgid, &req, sizeof(req)-sizeof(long), 0);
```
**Client menerima response**
```bash
msgrcv(msgid, &res, sizeof(res)-sizeof(long),
       TYPE_RESPONSE + getpid(), 0);
```
Kedua kode diatas membuat sistem menjadi multi-client. <br/>

#### 3. Matchmaking
Server: <br/>
```bash
if(waiting_pid == -1){
    waiting_pid = req.pid;
    strcpy(waiting_name, req.username);
}
else{
    int p1 = waiting_pid;
    int p2 = req.pid;

    MatchMsg m1 = {TYPE_MATCH + p1, -1, 0, p2};
    strcpy(m1.enemy_name, req.username);

    MatchMsg m2 = {TYPE_MATCH + p2, -1, 1, p1};
    strcpy(m2.enemy_name, waiting_name);

    msgsnd(msgid, &m1, sizeof(m1)-sizeof(long), 0);
    msgsnd(msgid, &m2, sizeof(m2)-sizeof(long), 0);

    waiting_pid = -1;
}
```
Kode diatas berfungsi untuk menemukan lawan, pairing 2 player secara real-time. <br/>

#### 4. Shared Memory (Battle State)
```bash
int shmid = shmget(SHM_KEY, sizeof(SharedArena), IPC_CREAT | 0666);
arena = (SharedArena*) shmat(shmid, NULL, 0);
```
Kode diatas untuk menyimpan HP dan room, kedua player akses data yang sama. <br/>

#### 5. Room Allocation
```bash
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
            return i;
        }
    }
}
```
Kode diatas untuk mencegah tidak menemukan room untuk battle, hanya 1 player yang membuat room. <br/>

#### 6. Semaphore
```bash
sem_wait(&r->lock);

if(my_slot==0)
    r->hp2 -= dmg;
else
    r->hp1 -= dmg;

sem_post(&r->lock);
```
Kode diatas untuk sinkronisasi HP agar tidak corrupt dan agar tidak terjadi race condition. <br/>

#### 7. Real-Time Input System
```bash
fd_set set;
struct timeval tv = {0,100000};

FD_ZERO(&set);
FD_SET(STDIN_FILENO, &set);

int rv = select(STDIN_FILENO+1, &set, NULL, NULL, &tv);
```
Kode diatas berfungsi agar input tidak nge-freeze dan game terasa real-time. <br/>

#### 8. Battle Mechanics
```bash
int dmg = getDamage(p);

if(c=='a'){
    r->hp2 -= dmg;
}
else if(c=='u' && p->weapon > 0){
    r->hp2 -= dmg*3;
}
```
Kode diatas berfungsi untuk menjalankan Attack & Ultimate. <br/>

#### 9. Match History System
```bash
strcpy(history_list[history_count].opponent, enemy_name);
strcpy(history_list[history_count].result, result);
history_list[history_count].xp = p->xp;
history_count++;
```
Kode diatas berfungsi untuk menyimpan Match History. <br/>

<br/><br/>
### Jalankan Program
**Di terminal pertama**
<img width="372" height="200" alt="2026-05-01 14:23:04" src="https://github.com/user-attachments/assets/1bd09388-7561-4dbd-844d-9d1dc9049d3b" /> <br/><br/>

**Di terminal kedua dan ketiga** <br/> 
Fitur Login Character <br/>
<img width="1897" height="230" alt="image" src="https://github.com/user-attachments/assets/04480de0-7b30-4aa2-8421-de878db8a5b7" /> <br/><br/>

**Fitur Register** <br/>
<img width="1905" height="570" alt="image" src="https://github.com/user-attachments/assets/20225f02-ac42-43e0-8940-c58637accea2" /> <br/><br/>

**Fitur Battle** <br/>
Tekan "1.Battle", <br/>
<img width="1900" height="554" alt="2026-05-01 17:59:11" src="https://github.com/user-attachments/assets/4f822b34-e8b8-4ea3-852c-356b8961c811" /> <br/>
Tekan "a" untuk attack dan "u" untuk ultimate, setalah input langsung enter. <br/><br/>

**Fitur Armory** <br/>
<img width="322" height="748" alt="2026-05-01 17:31:42" src="https://github.com/user-attachments/assets/495cd761-b13f-450b-8cff-3ca681fb43de" /> <br/>
Dengan menggunakan senjata menambah damage.

**Fitur Match History** <br/>
<img width="370" height="322" alt="2026-05-01 18:00:32" src="https://github.com/user-attachments/assets/4ec7e8e2-2bac-4572-b800-c48e87cea8fb" /> <br/><br/>

Setelah selesai bisa langsung milih "4. Logout" untuk keluar












