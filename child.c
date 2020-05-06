//Amy Seidel
//CS4760 - OS
//Project 6

#include "helper.h"

#define ERAND 99
#define RRAND 32001

int main(int argc, char *argv[]) {

    //variables for the process checkers
    int request = 0;
    int done = 0;
    int event = 0;

    //variables for time and the request counter and time event occurs
    unsigned int *sec = 0;
    unsigned int *nanosec = 0;
    unsigned int eventSec = 0;
    unsigned int eventNanosec = 0;
    unsigned int counter = 0;


    //clock id, semephore id, resource id, page table limit, percentage
    int cID = atoi(argv[1]);
    int sID = atoi(argv[2]);
    int rscID = atoi(argv[4]);
    int limit = atoi(argv[5]);
    int percentage = atoi(argv[6]);


    //used to get different sequences of numbers each time program is run
    srand(getpid());
    pid_t pid = getpid();

    //pointers for the resource and semaphores and shared mem
    key_t msgKey = ftok(".",M_KEY);
    int msgID = msgget(msgKey, 0666 | IPC_CREAT);
    sem_t *sPTR;
    memManager *rPTR;

    //allocating shared memory  shared memory
    check_SHMem(&sec, &nanosec, &sPTR, &rPTR, cID, sID, rscID);
    message.msgString = pid;
    message.msgString = 12345;

    randClock(sec, nanosec, &eventSec, &eventNanosec);
    while(done == 0)
    {
        if((*sec == eventSec && *nanosec >= eventNanosec) || *sec > eventSec)
        {
            request = rand()%RRAND;
            event = rand()%ERAND;
            randClock(sec, nanosec, &eventSec, &eventNanosec);
            counter++;

            //message is set to pid, sent and received, and then it is complete
            //https://stackoverflow.com/questions/30437486/how-to-get-messsage-and-message-id-from-one-file-to-another-file
            if(counter == limit && event < 50)
            {
                message.msgString = (int)pid;
                sprintf(message.msgChar,"%d", 99999);
                msgsnd(msgID, &message, sizeof(message)-sizeof(long), 0);
                msgrcv(msgID, &message, sizeof(message)-sizeof(long), (pid+118), 0);
                done = 1;
            }
            //message is set to pid, something is requested, NOT marked as complete
            else if(event < percentage)
            {
                message.msgString = (int)pid;
                sprintf(message.msgChar,"%d %d", request, 0);
                msgsnd(msgID, &message, sizeof(message)-sizeof(long), 0);
                msgrcv(msgID, &message, sizeof(message)-sizeof(long), (pid+118), 0);
            }
            //message is set to pid, something is requested, NOT marked as complete
            else if(event >= (ERAND-percentage))
            {
                message.msgString = (int)pid;
                sprintf(message.msgChar,"%d %d", request, 1);
                msgsnd(msgID, &message, sizeof(message)-sizeof(long), 0);
                msgrcv(msgID, &message, sizeof(message)-sizeof(long), (pid+118), 0);
            }
        }
    }

    //release all that memory
    shmdt(sPTR);
    shmdt(rPTR);
    shmdt(sec);
    shmctl(msgID, IPC_RMID, NULL);
    exit (0);
}