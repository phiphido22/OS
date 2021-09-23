/* FileName: thread.c
 * Author: Phi Do
 * Date Created: 25/03/2020
 * Last Edited: 9/05/2020
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include"thread.h"
#include"Request.h"
#include"shared_var.h"


/*
Function: lift
Use: consumer function that takes requests from the request buffer, and calls a
function to operate a lift, then outputs to the sim_out file
 */
void* lift(void *arg)
{
    FILE* logFile = NULL;
    int numLiftRequests = 0; //number of lift requests completed by this lift
    int currentFloor = 1;
    int floorDistance;

    Request tempRequest;

    //while there is still requests in the buffer, take a request from the buffer
    while(remainingReqNo != 0)
    {
        pthread_mutex_lock(&bufferLock);
        tempRequest.destFloor = requests[0].destFloor;
        tempRequest.sourceFloor = requests[0].sourceFloor;
        tempRequest.requestNumber = requests[0].requestNumber;

        //
        if(tempRequest.requestNumber == 0)
        {
            pthread_cond_wait(&empty, &bufferLock);
            tempRequest.destFloor = requests[0].destFloor;
            tempRequest.sourceFloor = requests[0].sourceFloor;
            tempRequest.requestNumber = requests[0].requestNumber;
        }

        if(tempRequest.destFloor == 0 && tempRequest.sourceFloor == 0)
        {
            return 0;
        }
        /*function that acts as a fifo queue, removing the first entry after it
         has been copied to the tempRequest, then shuffling the rest of the
         entries forward*/
        shiftBuffer();
        pthread_mutex_unlock(&bufferLock);
        pthread_cond_signal(&full);

        pthread_mutex_lock(&logLock);
        logFile = fopen("sim_out", "a");
        // counting amount of requests done by lift
        numLiftRequests++;
        /* the amount of requests remaining is tracked to end
         the lift function when completed.*/
        remainingReqNo--;
        floorDistance = distanceCalc(currentFloor, tempRequest.sourceFloor, tempRequest.destFloor);
        totalMovement = totalMovement + floorDistance;
        fclose(logFile);
        pthread_mutex_unlock(&logLock);

        pthread_mutex_lock(&logLock);
        //print out the lift operations to the sim_out file that already should exist
        logFile = fopen("sim_out", "a");
        requestsComplete++; //total requests amongst all lifts (done globally)
        fprintf(logFile, "Lift-%d Operation\n"
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
                         numLiftRequests, totalMovement, tempRequest.destFloor);
        sleep(liftTime); //simulating the time taken for the lift to complete the request
        fclose(logFile);
        pthread_mutex_unlock(&logLock);
    }
    pthread_exit(NULL);
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

    for(i = 0; i < bufferSize - 1 ; i++)
    {
        requests[i] = requests[i+1];
    }
    requests[bufferSize - 1].destFloor = 0;
    requests[bufferSize - 1].sourceFloor = 0;
    requests[bufferSize - 1].requestNumber = 0;
}
