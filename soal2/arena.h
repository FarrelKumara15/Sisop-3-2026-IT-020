// Revisi Kode

#ifndef ARENA_H
#define ARENA_H

#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define MSG_KEY 1234 // Key untuk Message Queue
#define SHM_KEY 5678 // Key untuk Shared Memory

#define TYPE_RESPONSE 10000
#define TYPE_MATCH    20000

#define MAX_USER 50
#define MAX_ROOMS 5

// User
typedef struct {
    char username[50];
    char password[50];
    int is_logged_in;
    int pid;
} User;

// Message Queue
typedef struct {
    long type;
    int action;
    char username[50];
    char password[50];
    int pid;
} MsgRequest;

// Response dari server ke client
typedef struct {
    long type;
    int success;
    char message[100];
} MsgResponse;

// Message Matchmaking
typedef struct {
    long type;
    int room_id;
    int my_slot;
    int enemy_pid;
    char enemy_name[50]; 
} MatchMsg;

// Struktur Room Battle
typedef struct {
    int used;
    int pid1, pid2;
    int hp1, hp2;
    int ready;
    sem_t lock;
} BattleRoom;

// Struktur Arena Global
typedef struct {
    BattleRoom rooms[MAX_ROOMS];
    sem_t global_lock;
} SharedArena;

#endif
