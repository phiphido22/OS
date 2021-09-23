/* FileName: file_io.h
 * Author: Phi Do
 * Date Created: 25/03/2020
 * Last Edited: 9/05/2020
 */

#ifndef file_ioHeader
#define file_ioHeader
    void* request(void* fileName);
    int getRequests(char* fileName);
    void makeTest(char* fileName);
    int nextIndex();
    void logSummary(void);
#endif
