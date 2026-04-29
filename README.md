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
<img width="414" height="147" alt="2026-04-29 11:55:26" src="https://github.com/user-attachments/assets/cf414da6-9815-4aa9-bf7b-befba0f8b70e" /> <br/>
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



