/* FileName: lift_sim_B.h
 * Author: Phi Do
 * Date Created: 25/03/2020
 * Last Edited: 9/05/2020
 */

#include"Request.h"
#include<semaphore.h>
#ifndef testingHeader
#define testingHeader
    typedef struct Semaphores
    {
        sem_t full;
        sem_t empty;
        sem_t logLock;
        sem_t bufferLock;
    }Semaphores;

    typedef struct FDescriptors
    {
        int fd_variables;
        int fd_semaphores;
        int fd_buffer;
    }FDescriptors;

    typedef struct Variables
    {
        int numRequests;
        int remainingReqNo; //remaining number of requests
        int liftTime;
        int bufferSize;
        int requestsComplete;
        int totalMovement;
        int currentFloor;
        int currentRequestNumber;
        FILE* file;
    }Variables;

    void create_shm(FDescriptors* file);
    void init_shm();
    void destroy_shm(FDescriptors* file);
    void init_sem();
    void destroy_sem();
    void sim_execute(char* fileName);
    void initBuffer();
    void* request(void* fileName);
    int getRequests(char* fileName);
    void makeTest(char* fileName);
    int nextIndex();
    void logSummary(void);
    void* lift(void* arg);
    int distanceCalc(int start, int pickup, int destination);
    void shiftBuffer(void);
#endif
