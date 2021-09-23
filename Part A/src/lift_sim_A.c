/* FileName: lift_sim_A.c
 * Author: Phi Do
 * Date Created: 25/03/2020
 * Last Edited: 9/05/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "lift_sim_A.h"
#include "Request.h"
#include "file_io.h"
#include "thread.h"

pthread_mutex_t bufferLock;
pthread_mutex_t logLock;
pthread_cond_t full;
pthread_cond_t empty;

//buffer that contains all the requests to be moved from producer to consumer
Request *requests;
int numRequests;
int remainingReqNo; //remaining number of requests
int liftTime;
int bufferSize;
int requestsComplete;
int totalMovement;

int main(int argc, char const *argv[])
{
    if (argc == 3)
    {
        //buffer size is the first integer value from command line, is size of
        //Request requests
        bufferSize = atoi(argv[1]);
        // liftTime is the time each lift 'sleeps' inbetween each lift function
        liftTime = atoi(argv[2]);
        requests = (Request*)malloc(bufferSize * sizeof(Request));
        if(bufferSize >= 1 && bufferSize <= 100)
        {
            requests = (Request*)malloc(bufferSize * sizeof(Request));
            initBuffer();
            // a file named sim_input is created and later read for the function
            makeTest("sim_input");
            sim_execute("sim_input");
            printf("Simulation Complete! Check files 'sim_out' and 'sim_input' for more details.\n");
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
    free(requests);
    printf("Simulation Complete! Check files 'sim_out' and 'sim_input' for more details.\n");
    return 0;
}

/*
Function: sim_execute
Use: function that creates, and closes the threads created.
 */
void sim_execute(char* fileName)
{
    pthread_t lifts[4];
    int one = 1;
    int two = 2;
    int three = 3;
    int* CPU_1 = &one;
    int* CPU_2 = &two;
    int* CPU_3 = &three;

    numRequests = getRequests(fileName);
    remainingReqNo = numRequests;

    totalMovement = 0;
    requestsComplete = 0;

    // one thread for the requests, 3 others for the 3 lifts being simulated
    pthread_create(&lifts[0], NULL, request, (void*) fileName);
    pthread_create(&lifts[1], NULL, lift, (void*)CPU_1);
    pthread_create(&lifts[2], NULL, lift, (void*)CPU_2);
    pthread_create(&lifts[3], NULL, lift, (void*)CPU_3);

    // after each thread is finished, it will close when there are no jobs left
    pthread_join(lifts[0], NULL);
    pthread_join(lifts[1], NULL);
    pthread_join(lifts[2], NULL);
    pthread_join(lifts[3], NULL);

    logSummary();
}

/*
Function: initBuffer
Use: initialises the request buffer where the requests will be between the
producer and writer
 */
void initBuffer()
{
    int i;
    for(i = 0; i < bufferSize; i++)
    {
        requests[i].destFloor = 0;
        requests[i].sourceFloor = 0;
        requests[i].requestNumber = 0;
    }
}
