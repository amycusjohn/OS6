//Amy Seidel
//CS4760 - OS
//Project 6

#include "helper.h"
#define SCND 100
#define PAGESS 32

//global variables
int pCounter = 0;
int ftPosition = 0;
int alarmm = 0;
int arr[MAX_PROCESS] = {0};

/* kill the process */
void killer(int sign_no){
    alarmm = 1;
}

/* checks the placement of the process in the array */
int placementChecker(int *place)
{
    int i = 0;
    for(i = 0; i < pCounter; i++){
        if(arr[i] == 0){
            arr[i] = 1;
            *place = i;
            return 1;
        }
    }
    return 0;
}


int main (int argc, char *argv[]) {

    //all the int variables
    int address;
    int stat;
    int percentage = 50;
    int maxProcL = 900;
    int numLines = 0;
    int numForked = 0;
    int pagefault = 0;
    int numLoop = 0;
    int counter = 0;
    int firstFork = 0;
    int tempPid = 0;
    int i = 0;
    int j = 0;
    int cID = 0;
    int rscID = 0;
    int sID = 0;
    int place = 0;


    //the unsigned ints for the clock and access speed for the statistics
    unsigned int *sec = 0;
    unsigned int *nanosec = 0;
    unsigned int forkClockSec = 0;
    unsigned int forkClockNanosec = 0;
    unsigned int accessSpeed = 0;

    //char variables for all the messages
    char child;
    char opt;
    char chMSG[20]; //message from the child
    char cSHMem[10]; //clock shared memory
    char rscShrdMem[10]; // resource shared memory
    char sSHMem[10]; //semephore shared memery
    char lSHMem[10]; //limit shared memory
    char perSHMem[10]; // percentage
    char request[20]; //type of request
    char pSHMem[10]; // position

    //double/float vairables
    double pageFaults = 0;
    double memAccess = 0;
    double memAccessSec = 0;
    float requestAdd = 0;

    //frame table of size 265 x 3 all initialized to 0
    int frameTable[256][3] = {{0}};

    //opening file for writing
    char* filename = malloc(sizeof(char));
    filename = "logfile.txt";
    FILE *infile = fopen(filename, "w");
    freopen("logfile.txt","a",infile);


    srand(time(NULL));
    pCounter = MAX_PROCESS; //counter for the processes


    while((opt = getopt(argc, argv, "hp:m:")) != -1){
        switch(opt){
            case'h':
                printf("HELP: Enter ./oss -m x where x specifies the memory schemes. Log file is stored in logfile.txt\n");
                exit(0);
            case'm':
                pCounter = atoi(optarg);
                if (pCounter > MAX_PROCESS){
                    pCounter = MAX_PROCESS;
                }
        }
    }

    //key variable declarations
    key_t cKey = 0;
    key_t rKey = 0;
    key_t sKey = 0;

    //key for messege queues
    key_t msgKey = ftok(".", M_KEY);
    int msgID = msgget(msgKey, 0666 | IPC_CREAT);

    //set the resource pointer and semephore pointer to null
    sem_t *sPTR = NULL;
    memManager *rPTR = NULL;

    //allocate the shared memory keys and allocate it
    allocate_SHMemKey(&cKey, &sKey, &rKey);
    allocate_SHMem(&cID, &sID, &rscID, cKey, sKey, rKey);
    check_SHMem(&sec, &nanosec, &sPTR, &rPTR, cID, sID, rscID);

    //Kill the program after 2 seconds
    signal(SIGALRM, killer);
    alarm(2);

    do {
        if(firstFork == 0)
        {
            randClock(sec, nanosec, &forkClockSec, &forkClockNanosec);
            firstFork = 1;
            fprintf(infile, "Master: Fork Time starts at %d : %d\n", forkClockSec, forkClockNanosec);
        }

        *nanosec += 50000;

        if(*nanosec >= BIL)
        {
            *sec += 1;
            *nanosec = 0;
            memAccessSec = (memAccess/ *sec);
        }
        if(((*sec == forkClockSec) && (*nanosec >= forkClockNanosec)) || (*sec > forkClockSec))
        {
            if(placementChecker(&place) == 1)
            {
                numForked++;
                firstFork = 0;
                fprintf(infile,"Master: Forked  at %d : %d \n", *sec, *nanosec);

                //send to formatting and shared memory messages
                format(cSHMem, sSHMem, pSHMem, rscShrdMem, lSHMem, perSHMem, cID, sID, rscID, place, maxProcL, percentage);

                //fork a process and add to array
                pid_t childPid = fork_process(cSHMem, sSHMem, pSHMem, rscShrdMem, lSHMem, perSHMem);
                rArraySize[place] = malloc(sizeof(struct memManager));
                (*rArrayPTR)[place]->pid = childPid;

                fprintf(infile,"Master: Child %d born with PID: %d\n", place, childPid);

                //32 pages allowed add 1 or -1 for the array
                for(i = 0 ; i < PAGESS; i++)
                {
                    (*rArrayPTR)[place]->tableSz[i] = -1;
                }
                (*rArrayPTR)[place]->resource_Marker = 1;

            }
        }
        for(i = 0; i < pCounter; i++)
        {

            if(arr[i] == 1)
            {
                tempPid =  (*rArrayPTR)[i]->pid;
                if((msgrcv(msgID, &message, sizeof(message)-sizeof(long), tempPid, IPC_NOWAIT|MSG_NOERROR)) > 0)
                {
                    if(atoi(message.msgChar) != 99999){ //it received  aread or write
                        fprintf(infile, "Master: P%d requesting address %d to ",i ,atoi(message.msgChar));
                        strcpy(chMSG, strtok(message.msgChar, " "));
                        address = atoi(chMSG);
                        strcpy(request, strtok(NULL, " "));

                        //if it is a 0 request then it is read else it is write
                        if(atoi(request) == 0){
                            fprintf(infile, " read at time %d : %d\n", *sec, *nanosec);
                        }else{
                            fprintf(infile, "write  at time %d : %d\n", *sec, *nanosec);
                        }


                        requestAdd = (atoi(chMSG))/1000;
                        requestAdd = (int)(floor(requestAdd));

                        //page table empty? assign it to the frame table
                        if((*rArrayPTR)[i]->tableSz[(int)requestAdd] == -1 || frameTable[(*rArrayPTR)[i]->tableSz[(int)requestAdd]][0] != (*rArrayPTR)[i]->pid)
                        {
                            numLoop = 0;
                            // is the first frame empty? Check out the loop
                            while(frameTable[ftPosition][0] != 0 && numLoop < 255)
                            {
                                ftPosition++;
                                numLoop++;
                                if(ftPosition == 256){
                                    ftPosition = 0;
                                }
                                if(numLoop == 255){
                                    pagefault = 1;
                                }
                            }
                            if(pagefault == 1){
                                pageFaults++;
                                fprintf(infile, "Master: Address %d is not in a frame, pagefault\n", address);

                                //Check if the next frame is open, set it to 0 if it was 1 before and then move on
                                while(frameTable[ftPosition][1] != 0){
                                    frameTable[ftPosition][1] = 0;
                                    ftPosition++;
                                    if(ftPosition == 256){
                                        ftPosition = 0;
                                    }
                                }
                                if(frameTable[ftPosition][1] == 0){
                                    memAccess++;
                                    fprintf(infile, "Master: Clearing frame %d and swapping in P%d page %d\n", ftPosition, i, (int)requestAdd);
                                    (*rArrayPTR)[i]->tableSz[(int)requestAdd] = ftPosition;
                                    frameTable[ftPosition][0] = (*rArrayPTR)[i]->pid;
                                    frameTable[ftPosition][2] = atoi(request);
                                    fprintf(infile, "Master: Address %d in frame %d giving data to P%d at time %d : %d\n", address, ftPosition, i, *sec, *nanosec);
                                    ftPosition++;
                                    if(ftPosition == 256){
                                        ftPosition = 0;
                                    }
                                    counter++;
                                }
                                accessSpeed +=  20000000;
                                *nanosec += 20000000;
                                fprintf(infile, "Master: Dirty bit is set to %d and clock is incremented some time\n", atoi(request));
                            }
                            //empty bit located
                            else
                                {
                                memAccess++;
                                (*rArrayPTR)[i]->tableSz[(int)requestAdd] = ftPosition;
                                frameTable[ftPosition][0] = (*rArrayPTR)[i]->pid;
                                frameTable[ftPosition][1] = 0;
                                frameTable[ftPosition][2] = atoi(request);
                                fprintf(infile, "Master: Address %d in frame %d giving data to P%d at time %d : %d\n", address, ftPosition, i, *sec, *nanosec);
                                ftPosition++;

                                //move clock by 10 seconds
                                if(ftPosition == 256){
                                    ftPosition = 0;
                                }
                                accessSpeed  += 10;
                                *nanosec += 10;
                                counter++;
                                fprintf(infile, "Master: Dirty bit is set to %d and and is incremented another 10ns to the clock\n", atoi(request));
                            }

                        } else {
                            memAccess++;
                            frameTable[(*rArrayPTR)[i]->tableSz[(int)requestAdd]][1] = 1;
                            frameTable[(*rArrayPTR)[i]->tableSz[(int)requestAdd]][2] = atoi(request);
                            *nanosec +=  20000000;
                            accessSpeed +=   20000000;
                            counter++;
                            fprintf(infile, "Master: Dirty bit is set to %d and clock is incremented some time\n", atoi(request));
                        }

                        //send the message
                        message.msgString = ((*rArrayPTR)[i]->pid+118);
                        sprintf(message.msgChar,"awaken");
                        msgsnd(msgID, &message, sizeof(message)-sizeof(long), 0);

                    }
                    //
                    else if(atoi(message.msgChar) == 99999){
                        arr[i] = 0;
                        message.msgString = ((*rArrayPTR)[i]->pid+118);
                        fprintf(infile, "Master: P%d is done! Clearing the frame: ", i);
                        for(j = 0; j < PAGESS; j++){

                            //printing out the frame table to when j is less than pagess
                            if((*rArrayPTR)[i]->tableSz[j] != -1 && frameTable[(*rArrayPTR)[i]->tableSz[j]] == (*rArrayPTR)[i]->tableSz[j]){
                                fprintf(infile, "%d, ", j);
                                frameTable[(*rArrayPTR)[i]->tableSz[j]][0] = 0;
                                frameTable[(*rArrayPTR)[i]->tableSz[j]][1] = 0;
                                frameTable[(*rArrayPTR)[i]->tableSz[j]][2] = 0;
                                (*rArrayPTR)[i]->tableSz[j] = -1;
                            }
                        }
                        fprintf(infile,"\n");
                        msgsnd(msgID, &message, sizeof(message)-sizeof(long), 0);
                        waitpid(((*rArrayPTR)[i]->pid), &stat, 0);
                        free(rArraySize[i]);
                    }

                } else {
                }
            }
        }
        while((child = fgetc(infile)) != EOF){
            if(child == '\n'){
                numLines++;
            }
        }
        if(numLines >= 100000){
            fclose(infile);
        }

    }while((*sec < SCND+10000) && alarmm == 0 && numForked < 100); //Stats sections
    fprintf(infile, "\n\nProgram has terminated sucessfully! The interesting statistics are printed belows:\n%f memory accesses per second.\n%f pagefaults per memory access.\n%f average access speed in nanosec.\n", memAccessSec, pageFaults/memAccess, accessSpeed/memAccess);

    fclose(infile);

    //remove all the shared memory
    shmdt(sec);
    shmdt(sPTR);
    shmdt(rPTR);
    msgctl(msgID, IPC_RMID, NULL);
    shmctl(msgID, IPC_RMID, NULL);
    shmctl(rscID, IPC_RMID, NULL);
    shmctl(cID, IPC_RMID, NULL);
    shmctl(sID, IPC_RMID, NULL);
    kill(0, SIGTERM);
    return ( 0 );
}