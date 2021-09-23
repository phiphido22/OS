/* FileName: shared_var.h
 * Author: Phi Do
 * Date Created: 25/03/2020
 * Last Edited: 9/05/2020
 */

//MUTEX LOCKS
extern pthread_mutex_t bufferLock;
extern pthread_mutex_t logLock;
extern pthread_cond_t full;
extern pthread_cond_t empty;

extern Request *requests;
extern int numRequests; //number of requests to be logged/completed
extern int remainingReqNo; //remaining number of requests
extern int liftTime; //time for lift to complete request, argv[2]
extern int bufferSize; //size of the buffer the requests go into, argv[1]
extern int requestsComplete; // total number of requests complete of all lifts
extern int totalMovement; //total movement of all lifts
