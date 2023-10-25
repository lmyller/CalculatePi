#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include "pi.h"

int main(){
    return pi();
}

int pi(){
    pid_t processoFilho;
    Report report;
    
    getReport(&report);
    
    for (int indice = 0; indice < NUMBER_OF_PROCESS; indice++){
        if (createProcess() == 0){
            calculationOfNumberPi(MAXIMUM_NUMBER_OF_TERMS);       
            
            exit(EXIT_SUCCESS);
        }

        else
            wait(0);
    }

    return EXIT_SUCCESS;
}

pid_t createProcess(){
    return fork();
}

void getReport(Report *report){
    String message2;

    sprintf(message2, MESSAGE_2, getpid());

    strcpy(report->programName, PROGRAM_NAME);
    strcpy(report->message1, MESSAGE_1);
    strcpy(report->message2, message2);
}

double calculationOfNumberPi(unsigned int terms){
    Threads threads;
    ThreadResponse *threadResponse;
    void *response;
    unsigned int partialNumbers = PARTIAL_NUMBER_OF_TERMS;
    ProcessReport processReport;
    CurrentTime startTime = getTime();
    CurrentTime endTime;
    pthread_t threadID[NUMBER_OF_THREADS];
    
    for (size_t i = 0; i < NUMBER_OF_THREADS; i++)
    {   
        threadID[i] = createThread(&partialNumbers);
    }

    for (size_t i = 0; i < NUMBER_OF_THREADS; i++)
    {
        pthread_join(threadID[i], &response);
        
        threadResponse = (ThreadResponse *) response;
        
        threads[i].threadID = threadID[i];
        threads[i].tid = gettid();
        threads[i].time = threadResponse->time;
    }

    endTime = getTime();

    for (size_t i = 0; i < NUMBER_OF_THREADS; i++)
    {
        printf("\nID: %d", threads[i].threadID);
        printf("\nTID: %d", threads[i].tid);
        printf("\nTIME: %.3f\n", threads[i].time);
    }

    return 0;
}

pthread_t createThread(unsigned int *terms) {
    pthread_t threadId;

    pthread_create(&threadId, NULL, sumPartial, terms);
   
    return threadId;
}

void* sumPartial(void *terms){
    ThreadResponse *threadResponse = (ThreadResponse *)malloc(sizeof(ThreadResponse));
    int limit = *(int *)terms;
    CurrentTime startTime = getTime();
    CurrentTime endTime;

    threadResponse->sum = 0;

    for (size_t indice = 0; indice < limit; indice++)
    {
        int sign = (indice % 2 == 0) ? 1 : -1;
        double term = sign / (2.0 * indice + 1.0);
        threadResponse->sum += term;
    }
    
    threadResponse->sum *= 4.0;

    endTime = getTime();

    threadResponse->time = getDiffTime(startTime, endTime);

    pthread_exit((void *) threadResponse);
}

CurrentTime getTime(){
    CurrentTime currentTime;
    struct timespec time;

    clock_gettime(CLOCK_MONOTONIC, &time);

    currentTime.sec = time.tv_sec;
    currentTime.nanosec = time.tv_nsec;

    return currentTime;
}

double getDiffTime(const CurrentTime start, const CurrentTime end) {
    long long startNS = (long long)start.sec * 1000000000LL + start.nanosec;
    long long endNS = (long long)end.sec * 1000000000LL + end.nanosec;
    
    if (endNS < startNS) 
        endNS += 1000000000000LL; 

    return (double)(endNS - startNS) / 1e10;
}