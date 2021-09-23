/* FileName: lift_sim_B.c
 * Author: Phi Do
 * Date Created: 25/03/2020
 * Last Edited: 9/05/2020
 */

#define _DEFAULT_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<semaphore.h>
#include<unistd.h>
#include<time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include"testing.h"
#include"Request.h"

Request *requests;
Semaphores *semaphores;
Variables *variables;
FDescriptors fileD;
int tempBuffer;

int main(int argc, char const *argv[])
{
    if (argc == 3)
    {
        /* tempBuffer is used to initialise the shared memory before,
        as variables->bufferSize is inside of the shared memory */
        tempBuffer = atoi(argv[1]);
        create_shm(&fileD);
        variables->bufferSize = atoi(argv[1]);
        variables->liftTime = atoi(argv[2]);
        init_shm();
        init_sem();

        //initialising the request buffer inside of the shared memory
        for(int i = 0; i < tempBuffer; i++)
        {
            requests[i].destFloor = 0;
            requests[i].sourceFloor = 0;
            requests[i].requestNumber = 0;
        }
        if(variables->bufferSize >= 1 && variables->bufferSize <= 100)
        {
            makeTest("sim_input");
            sim_execute("sim_input");
            destroy_sem();
            destroy_shm(&fileD);
        }
        else
        {
            printf("Invalid Arguments\nClosing Program Now.\n");
        }
    }
    else
    {
        printf("Invalid Arguments\nClosing Program Now.\n");
    }
    return 0;
}

/*
Function: create_shm
Use: creates the shared memory that the variables, semaphores and buffer take
to be used amongst both the producer and the consumer
*/
void create_shm(FDescriptors* fileD)
{
    //using POSIX shared memory access
   fileD->fd_variables = shm_open("shm_variables", O_RDWR|O_CREAT, 0666);
   fileD->fd_semaphores = shm_open("shm_semaphores", O_RDWR|O_CREAT, 0666);
   fileD->fd_buffer = shm_open("shm_buffer", O_RDWR|O_CREAT, 0666);

   ftruncate(fileD->fd_variables, (off_t)sizeof(Variables));
   ftruncate(fileD->fd_semaphores, (off_t)sizeof(Semaphores));
   ftruncate(fileD->fd_buffer, (off_t)sizeof(Request)*tempBuffer);

   semaphores = (Semaphores*)mmap(NULL, sizeof(Semaphores), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, fileD->fd_semaphores,0);
   variables = (Variables*)mmap(NULL, (sizeof(Variables)+(tempBuffer*sizeof(Request))), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, fileD->fd_variables,0);
   requests = (Request*)mmap(NULL, sizeof(Request)*tempBuffer, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, fileD->fd_buffer,0);
}

/*
Function: init_shm
Use: initialises the shared memory, more specifically initialises the variables
inside of the structs that are inside the shared memory
*/
void init_shm()
{
    variables->totalMovement = 0;
    variables->requestsComplete = 0;
    variables->numRequests = 0;
    variables->remainingReqNo = 0;
    variables->currentFloor = 1;
    variables->currentRequestNumber = 1;
}

/*
Function: destory_shm
Use: unlinks and destroys the 3 shared memory locations
*/
void destroy_shm(FDescriptors* file)
{
   munmap(semaphores, sizeof(Semaphores));
   munmap(requests, sizeof(Request));
   munmap(variables, sizeof(Variables));
   shm_unlink("shm_variables");
   shm_unlink("shm_buffer");
   shm_unlink("shm_semaphores");
}

/*
Function: init_sem
Use: initialises the semaphores inside of the semaphore set inside of shared
memory
*/
void init_sem()
{
   sem_init(&(semaphores->full), 1,0);
   sem_init(&(semaphores->empty), 1,tempBuffer);
   sem_init(&(semaphores->logLock), 1,1);
   sem_init(&(semaphores->bufferLock), 1,1);
}

/*
Function: destory_sem
Use: destorys the semaphores after they have served its use for all 4 processes
*/
void destroy_sem()
{
   sem_destroy(&semaphores->full);
   sem_destroy(&semaphores->empty);
   sem_destroy(&semaphores->logLock);
   sem_destroy(&semaphores->bufferLock);
}

/*
Function: sim_execute
Use: function that creates, and closes the threads created.
 */
void sim_execute(char* fileName)
{
    int pid, pid1, pid2;
    int one = 1;
    int two = 2;
    int three = 3;
    int* CPU_1 = &one;
    int* CPU_2 = &two;
    int* CPU_3 = &three;

    variables->numRequests = getRequests(fileName);
    printf("numRequests: %d\n", variables->numRequests);
    variables->remainingReqNo = getRequests(fileName);
    printf("remainingReqNo: %d\n", variables->remainingReqNo);

    variables->totalMovement = 0;
    variables->requestsComplete = 0;

    pid = fork();
    // if it is the first parent, then it is the producer process
    if(pid == 0)
    {
        request(fileName);
        printf("All requests have been input. Check files 'sim_out' and 'sim_input' for more details.\n");
    }
    //all other processes created are child processes that are consumers
    else
    {
        pid1 = fork();
        if(pid1 == 0)
        {
            lift((void*)CPU_1);
        }
        else
        {
            pid2 = fork();
            if(pid2 == 0)
            {
                lift((void*)CPU_2);
            }
            else
            {
                lift((void*)CPU_3);
            }
        }
    }
}

/*
Function: lift
Use: consumer function that takes requests from the request buffer, and calls a
function to operate a lift, then outputs to the sim_out file
 */
void* lift(void* arg)
{
    int currentFloor = 1;
    int numLiftRequests = 1;
    Request tempRequest;

    while(variables->remainingReqNo != 0)
    {
        sem_wait(&semaphores->bufferLock);
        sem_wait(&semaphores->full);
        /* copy the data of the request at the front of the buffer into a
        temporary struct to allow buffer to shift and let other processes take
        a request*/
        tempRequest.destFloor = requests[0].destFloor;
        tempRequest.sourceFloor = requests[0].sourceFloor;
        tempRequest.requestNumber = requests[0].requestNumber;

        //if there are no requests currently wait.
        if(tempRequest.requestNumber == 0)
        {
            sem_wait(&semaphores->empty);
            sem_wait(&semaphores->bufferLock);
            tempRequest.destFloor = requests[0].destFloor;
            tempRequest.sourceFloor = requests[0].sourceFloor;
            tempRequest.requestNumber = requests[0].requestNumber;
        }
        if(tempRequest.destFloor == 0 && tempRequest.sourceFloor == 0)
        {
            return 0;
        }
        /*remove the task from the list after it has been passed
          from request to lift*/
        shiftBuffer();

        sem_post(&semaphores->bufferLock);
        //sem_post(&semaphores->full);

        sem_wait(&semaphores->logLock);
        //variables->file = fopen("sim_out", "a");
         // counting amount of requests done by lift
        variables->remainingReqNo--;
        int floorDistance = distanceCalc(currentFloor, tempRequest.sourceFloor, tempRequest.destFloor);
        variables->totalMovement = variables->totalMovement + floorDistance;
        fclose(variables->file);
        sem_post(&semaphores->logLock);

        sem_wait(&semaphores->logLock);
        //writing the output of the lift operation to a file, also output to terminal for clarity sake
        variables->file = fopen("sim_out", "a");
        variables->requestsComplete++; //total requests amongst all lifts (done globally)
        fprintf(variables->file, "Lift-%d Operation\n"
                         "Previous position: Floor %d\n"
                         "Request: Floor %d to Floor %d\n"
                         "Detail operations:\n"
                         "  Go from Floor %d to Floor %d\n"
                         "  Go from Floor %d to Floor %d\n"
                         "  #movement for this request: %d\n"
                         "  #request: %d\n"
                         "  Total #movement: %d\n"
                         "Current position: Floor %d\n\n",
                         *(int*)arg, currentFloor, tempRequest.sourceFloor,
                         tempRequest.destFloor, currentFloor,
                         tempRequest.sourceFloor, tempRequest.sourceFloor,
                         tempRequest.destFloor, floorDistance,
                         numLiftRequests, variables->totalMovement, tempRequest.destFloor);
        printf("Lift-%d Operation\n"
                         "Previous position: Floor %d\n"
                         "Request: Floor %d to Floor %d\n"
                         "Detail operations:\n"
                         "  Go from Floor %d to Floor %d\n"
                         "  Go from Floor %d to Floor %d\n"
                         "  #movement for this request: %d\n"
                         "  #request: %d\n"
                         "  Total #movement: %d\n"
                         "Current position: Floor %d\n\n",
                         *(int*)arg, currentFloor, tempRequest.sourceFloor,
                         tempRequest.destFloor, currentFloor,
                         tempRequest.sourceFloor, tempRequest.sourceFloor,
                         tempRequest.destFloor, floorDistance,
                         numLiftRequests, variables->totalMovement, tempRequest.destFloor);

        currentFloor = tempRequest.destFloor;
        numLiftRequests++;
        sleep(variables->liftTime); //simulating the time taken for the lift to complete the request
        fclose(variables->file);
        sem_post(&semaphores->logLock);
        sem_post(&semaphores->empty);
    }
    printf("LIFT() HAS SKIPPED THE LIFT FUNCTION.\n");
    return NULL;
}

/*
Function: distanceCalc
Use: calculates the distance between the start and the floor to pick up a person
from, and calculates the distance between the pick up floor and the destination
floor
 */
int distanceCalc(int start, int pickup, int destination)
{
    int distance;

    if((start >= pickup) && (destination >= pickup))
    {
        distance = ((start - pickup) + (destination - pickup));
    }
    else if((start >= pickup) && (pickup >= destination))
    {
        distance = ((start - pickup) + (pickup - destination));
    }
    else if((pickup >= start) && (destination >= pickup))
    {
        distance = ((pickup - start) + (destination - pickup));
    }
    else if((pickup >= start) && (pickup >= destination))
    {
        distance = ((pickup - start) + (pickup - destination));
    }
    return distance;
}

/*
Function: shiftBuffer
Use: acts as a queue shuffler, pushing out the first value and shifting all
other values up one, initialising the back end value as 0 after
 */
void shiftBuffer(void)
{
    int i;

    for(i = 0; i < variables->bufferSize - 1 ; i++)
    {
        requests[i].destFloor = requests[i+1].destFloor;
        requests[i].sourceFloor = requests[i+1].sourceFloor;
        requests[i].requestNumber = requests[i+1].requestNumber;
    }
    requests[variables->bufferSize-1].destFloor = 0;
    requests[variables->bufferSize-1].sourceFloor = 0;
    requests[variables->bufferSize-1].requestNumber = 0;
}

/*
Function: request
Use: This function is the "producer" that creates the requests that are parsed from
the test file created, and sent into the buffer
*/
void* request(void* fileName)
{
    FILE* inFile = NULL;
    int index = 0;
    Request inRequest;
    int counter = 0;

    inFile = fopen((char*)fileName,"r");

    if(inFile == NULL)
    {
        /*throw an error message if the file could not be opened*/
        perror("Error: File could not be opened\nClosing Program...\n");
    }
    else
    {
        while(!feof(inFile))
        {
            sem_wait(&semaphores->empty);
            sem_wait(&semaphores->bufferLock);
            index = nextIndex();
            if(index == -1) //if there is no next available index
            {
                sem_wait(&semaphores->full);
                printf("There is no next available index/This is the final request\n");
                index = nextIndex();
            }
            fscanf(inFile, "%d %d", &requests[index].sourceFloor, &requests[index].destFloor);
            sem_post(&semaphores->bufferLock);
            sem_post(&semaphores->full);

            // need parsing through the file until the end of the file is reached
            if(!feof(inFile))
            {
                sem_wait(&semaphores->empty);
                sem_wait(&semaphores->bufferLock);
                //counter keeps track of the request number of each request
                counter++;
                inRequest.requestNumber = counter;
                requests->requestNumber = inRequest.requestNumber;
                inRequest.sourceFloor = requests[index].sourceFloor;
                inRequest.destFloor = requests[index].destFloor;
                sem_post(&semaphores->bufferLock);

                sem_wait(&semaphores->logLock);
                // write the request details to a file named sim_out
                variables->file = fopen("sim_out","a");
                fprintf(variables->file,
                        "New Lift Request From Floor %d to Floor %d\n"
                        "Request No: %d\n\n",
                        inRequest.sourceFloor, inRequest.destFloor,
                        inRequest.requestNumber);
                //print to output for clarity and knowing what is happening
                printf("New Lift Request From Floor %d to Floor %d\n"
                       "Request No: %d\n\n",
                       inRequest.sourceFloor, inRequest.destFloor,
                       inRequest.requestNumber);
                fclose(variables->file);
                sem_post(&semaphores->logLock);
                sem_post(&semaphores->full);
             }
             sem_post(&semaphores->empty);
         }
         sem_wait(&semaphores->empty);
         sem_wait(&semaphores->logLock);
         /*at the end of the function, output the total number of requests and
         movements to a file, lift time should ensure this goes last*/
         variables->file = fopen("sim_out","a");
         fprintf(variables->file,
                          "Total number of requests: %d\n"
                          "Total number of movements: %d\n\n",
                          variables->requestsComplete, variables->totalMovement);
         fclose(variables->file);
         sem_post(&semaphores->logLock);
         sem_post(&semaphores->full);
    }
    return NULL;
}

/*
Function: getRequests
Use: parses through the file, checking to see the amount of requests to be
completed
*/
int getRequests(char* fileName)
{
    FILE *inFile = NULL;
    inFile = fopen(fileName,"r");
    //char inLine[255];
    int totalRequests = 0;
    int ch = 0;

    if(inFile == NULL)
    {
        perror("Error opening file\n");
    }
    else
    {
        while(!feof(inFile))
        {
            /*fscanf(inFile, "%s", inLine);
            totalRequests++; //NOTE THIS GOES INCLUDING THE EOF INDICATOR*/
            ch = fgetc(inFile);
            if(ch == '\n')
            {
                totalRequests++;
            }
        }
    }
    fclose(inFile);
    return totalRequests;
}

/*
Function: makeTest
Use: creates the test file to be input, randomly choosing the floors of each
request, and the amount of requests in total.
*/
void makeTest(char* fileName)
{
    int randSource;
    int randDestination;
    int i;

    FILE *testFile = NULL;
    srand(time(0));

    testFile = fopen(fileName, "w");
    if(testFile == NULL)
    {
        perror("Error opening file\n");
    }
    else
    {
        //randomly chooses number between 50 and 100 as number of requests
        for(i = 0; i < (rand() % 51 + 50); i++)
        {
            //random number between 1 and 20 for starting floor
            randSource = rand() % 20 + 1;
            //random number between 1 and 20 for destination floor
            randDestination = rand() % 20 + 1;
            while(randSource == randDestination)
            {
                randDestination = rand() % 20 + 1;
            }

            fprintf(testFile,"%d %d\n", randSource, randDestination);
        }
    }
    fclose(testFile);
}

/*
Function: nextIndex
Use: checks for the next index of the array, until the buffersize is reached
*/
int nextIndex()
{
    int i = 0;
    int index = -1;
    while (i != variables->bufferSize - 1)
    {
        if (requests[i].requestNumber == 0)
        {
            index = i;
            i = variables->bufferSize - 2;
        }
        i++;
    }
    return index;
}

/*
Function: logSummary
Use: outputs the total number of requests and movements done in the program,
     prints out to a file named sim_out
*/
void logSummary(void)
{
    //sleep(5);
    variables->file = fopen("sim_out", "a");
    fprintf(variables->file, "Total number of requests: %d\n"
                     "Total number of movements: %d\n",
                     variables->requestsComplete, variables->totalMovement);
    fclose(variables->file);
}
