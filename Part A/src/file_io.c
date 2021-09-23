/* FileName: file_io.c
 * Author: Phi Do
 * Date Created: 25/03/2020
 * Last Edited: 9/05/2020
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "file_io.h"
#include "Request.h"
#include "shared_var.h"

/*
Function: request
Use: This function is the "producer" that creates the requests that are parsed from
the test file created, and sent into the buffer
*/
void* request(void* fileName)
{
    FILE* inFile = NULL;
    FILE* logFile = NULL;
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
            sleep(1);
            pthread_mutex_lock(&bufferLock);
            index = nextIndex();
            //if there is no valid index, then wait for both the full and buffer lock to be available
            if(index == -1)
            {
                pthread_cond_wait(&full, &bufferLock);
                index = nextIndex();
            }
            pthread_mutex_unlock(&bufferLock);

            fscanf(inFile, "%d %d", &requests[index].sourceFloor, &requests[index].destFloor);
            // need parsing through the file until the end of the file is reached
            if(!feof(inFile))
            {

                pthread_mutex_lock(&bufferLock);

                counter++;
                inRequest.requestNumber = counter;
                inRequest.sourceFloor = requests[index].sourceFloor;
                inRequest.destFloor = requests[index].destFloor;

                pthread_mutex_unlock(&bufferLock);

                pthread_mutex_lock(&logLock);

                // write the request details to a file named sim_out
                logFile = fopen("sim_out","a");
                fprintf(logFile,
                        "New Lift Request From Floor %d to Floor %d\n"
                        "Request No: %d\n\n",
                        inRequest.sourceFloor, inRequest.destFloor,
                        inRequest.requestNumber);
                fclose(logFile);
                pthread_mutex_unlock(&logLock);
             }
             pthread_cond_signal(&empty);
         }
         pthread_mutex_lock(&logLock);
         logFile = fopen("sim_out","a");
         fprintf(logFile, "Total number of requests: %d\n"
                          "Total number of movements: %d\n\n",
                          requestsComplete, totalMovement);
         fclose(logFile);
         pthread_mutex_unlock(&logLock);
    }
    fclose(inFile);
    pthread_exit(NULL);
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
    char inLine[255];
    int numRequests = 0;

    if(inFile == NULL)
    {
        perror("Error opening file\n");
    }
    else
    {
        while(!feof(inFile))
        {
            fscanf(inFile, "%s", inLine);
            numRequests++; //NOTE THIS GOES INCLUDING THE EOF INDICATOR
        }
    }
    fclose(inFile);
    return numRequests;
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
        for(i = 0; i < (rand() % 51 + 50); i++)
        {
            randSource = rand() % 20 + 1;
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
    extern Request *requests;
    extern int bufferSize;

    int i = 0;
    int index = -1;
    while (i != bufferSize - 1)
    {
        if (requests[i].requestNumber == 0)
        {
            index = i;
            i = bufferSize - 2;
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
    sleep(5);
    FILE* logFile;
    logFile = fopen("sim_out", "a");
    fprintf(logFile, "Total number of requests: %d\n"
                     "Total number of movements: %d\n",
                     requestsComplete, totalMovement);
    fclose(logFile);
}
