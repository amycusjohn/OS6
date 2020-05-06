//Amy Seidel
//CS4760 - OS
//Project 6

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

//global macros
#define MAX_PROCESS 18
#define RANDOM 500000000
#define BIL 1000000000


//keys for shared memory
#define C_SHMKEY 643783
#define RD_SHMKEY 643786
#define M_KEY 643789
#define S_KEY 643775

//global variables
pid_t pid = 0;
extern int limit;
extern int percentage;

/* struct for the memory manager   */
typedef struct memManager{
    pid_t pid;
    int resource_Marker;
    int tableSz[32];
} memManager;

//variables for the size of the array of processes
struct memManager *rArraySize[MAX_PROCESS];
struct memManager *(*rArrayPTR)[] = &rArraySize;

/* struct for the memory resource message  */
struct memResource {
    long msgString;
    char msgChar[100];
} message;

/* function for forking the children processes    */
pid_t fork_process(char *cSHMem, char *sSHMem, char*pSHMem, char*rscShrdMem, char*lShMem, char*perSHMem){
    if((pid = fork()) == 0)
    {
        execlp("./child", "./child", cSHMem, sSHMem, pSHMem, rscShrdMem, lShMem, perSHMem, NULL);
    }
    if(pid < 0)
    {
        printf("ERROR! Error Forking %s\n", strerror(errno));
    }
    return pid;
};

//Allocating and error checking in the shared memory */
void check_SHMem(unsigned int **sec, unsigned int **nanosec, sem_t **semaphore, memManager **rscPointer, int cID, int sID, int rscID){


    *sec = (unsigned int*)shmat(cID, NULL, 0);
    if(**sec == -1){
        printf("ERROR! Sec %s\n", strerror(errno));
    }

    *nanosec = *sec + 1;
    if(**nanosec == -1){
        printf("ERROR! Nanosec %s\n", strerror(errno));
    }

    *semaphore = (sem_t*)shmat(sID, NULL, 0);
    if(*semaphore == (void*)-1){
        printf("ERROR! Semaphore %s\n", strerror(errno));
    }

    *rscPointer = (memManager*)shmat(rscID, NULL, 0);
    if(*rscPointer == (void*)-1){
        printf("ERROR! Resource %s\n", strerror(errno));
    }
};

/* assigning keys to shared memory   */
void allocate_SHMemKey(key_t *rKey, key_t *sKey, key_t *cKey) {
    *rKey = ftok(".", RD_SHMKEY);
    *sKey = ftok(".", S_KEY);
    *cKey = ftok(".", C_SHMKEY);
};

/* allocating shared memory and creating the IDs plus error checking    */
void allocate_SHMem(int *cID, int *sID, int *rscID, key_t cKey, key_t sKey, key_t rKey){

    *cID = shmget(cKey, (sizeof(unsigned int) * 2), 0666 | IPC_CREAT);
    *sID = shmget(sKey, (sizeof(unsigned int) * 2), 0666 | IPC_CREAT);
    *rscID = shmget(rKey, (sizeof(memManager) *2),0666|IPC_CREAT);

    //error checking
    if(*cID == -1){
        printf("ERROR! Clock ID: %s\n", strerror(errno));
    }
    if(*sID == -1){
        printf("ERROR! Semaphore ID: %s\n", strerror(errno));
    }
    if(*rscID == -1){
        printf("ERROR! Resource ID:  %s\n", strerror(errno));
    }
};

/* function for the randomized clock that will fork a user process at random times */
void randClock(unsigned int *sec, unsigned int *nanosec, unsigned int *eventSec, unsigned int *eventNanosec){
    unsigned int random = rand()%RANDOM;
    *eventNanosec = 0;
    *eventSec = 0;

    //checking the times and manually changing them
    if((random + *nanosec) >=BIL)
    {
        *eventSec += 1;
        *eventNanosec = (random + *nanosec) - BIL;
    }
    else{
        *eventNanosec = random + *nanosec;
    }
    *eventSec = *sec;
};


/* This is for formatting the shared memory with IDs  https://linux.die.net/man/3/snprintf */
void format(char *cSHMem, char *sSHMem, char*pSHMem, char*rscShrdMem, char*lShMem, char*perSHMem, int cID, int sID, int rscID, int place, int limit, int percentage){

    snprintf(cSHMem, sizeof(cSHMem)+30, "%d", cID);
    snprintf(sSHMem, sizeof(sSHMem)+30, "%d", sID);
    snprintf(pSHMem, sizeof(pSHMem)+30, "%d", place);
    snprintf(rscShrdMem, sizeof(rscShrdMem)+30, "%d", rscID);
    snprintf(lShMem, sizeof(lShMem)+30, "%d", limit);
    snprintf(perSHMem, sizeof(perSHMem)+30, "%d", percentage);
};

