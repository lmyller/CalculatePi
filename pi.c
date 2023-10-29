#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "pi.h"

int main(){
    return pi();
}

/*
 * Esta função inicia o programa.
 * Retorna EXIT_SUCCESS.
 */
int pi(){
    setlocale(LC_ALL, "Portuguese");
    pid_t processoFilho;
    Report report;
    double sum;
    int segmentId = createSharedMemory();
    unsigned char *showProgramName = attachedSegmentMemory(segmentId);
    *showProgramName = FALSE;

    desconnectSharedMemory(showProgramName);

    getReport(&report);
        
    if (createProcess() == 0)
    {
        childProcess(report, segmentId);

        exit(EXIT_SUCCESS);
    }

    else{
        report.processNumber++;
        if (createProcess() == 0)
        {
            childProcess(report, segmentId);

            exit(EXIT_SUCCESS);
        }
    }

    return EXIT_SUCCESS;
}

// Cria o processo
pid_t createProcess(){
    return fork();
}

// Obtém os dados do processo pai e armazena em uma estrutura "Report".
void getReport(Report *report){
    String message2;

    // Acrescenta na estrutura o pid do processo
    sprintf(message2, MESSAGE_2, getpid());

    strcpy(report->programName, PROGRAM_NAME);
    strcpy(report->message1, MESSAGE_1);
    strcpy(report->message2, message2);

    // Identificação do processo 1
    report->processNumber = 1;
}

// Executa as funções do processo filho, recebe uma estrutura report com os dados do processo pai.
void childProcess(Report report, int segmentId){
    CurrentTime start;
    CurrentTime end;
    double sum;
    unsigned char *showProgramName = attachedSegmentMemory(segmentId);

    report.showProgramName = showProgramName;

    start = getTime();

    sum = calculationOfNumberPi(MAXIMUM_NUMBER_OF_TERMS);
    
    end = getTime();

    // Acrescenta na estrutura "Report" os dados do processo    
    sprintf(report.processReport1.identification, PROCESS_REPORT_IDENTIFICATION, report.processNumber, getpid());
    sprintf(report.processReport1.numberOfThreads, PROCESS_REPORT_NUMBER_OF_THREADS, NUMBER_OF_THREADS);
    sprintf(report.processReport1.start, PROCESS_REPORT_START, getTimeString(start));
    sprintf(report.processReport1.end, PROCESS_REPORT_END, getTimeString(end));
    sprintf(report.processReport1.duration, PROCESS_REPORT_DURATION, getDiffTime(start, end));
    sprintf(report.processReport1.pi, PROCESS_REPORT_PI, sum);

    createReport(&report);

    *report.showProgramName = TRUE;

    desconnectSharedMemory(showProgramName);
}

/* Calcula o número pi com n (n é definido por DECIMAL_PLACES) casas decimais usando o número máximo de
   termos da série de Leibniz, que é definido por MAXIMUM_NUMBER_OF_TERMS. Esta função deve criar x threads
   usando a função createThread, onde x é igual a NUMBER_OF_THREADS. 
*/
double calculationOfNumberPi(unsigned int terms){
    Threads threads;
    double sum = 0;
    ThreadResponse *threadResponse;
    void *response;
    ProcessReport processReport;
    String FileName;
    CurrentTime start = getTime();;
    CurrentTime endTime;

    // Inicia as threads
    for (size_t i = 0; i < NUMBER_OF_THREADS; i++)
    {   
        // Altera o limite dos termos que serão usados na soma.
        unsigned int *partialNumbers = (unsigned int*)malloc(sizeof(unsigned int));
        *partialNumbers = (i + 1) * PARTIAL_NUMBER_OF_TERMS; 
        
        threads[i].threadID = createThread(partialNumbers);
    }

    for (size_t i = 0; i < NUMBER_OF_THREADS; i++)
    {
        // Recupera os dados de cada thread.
        pthread_join(threads[i].threadID, &response);
        
        // Cast para usar os dados.
        threadResponse = (ThreadResponse *) response;
        
        sum += threadResponse->sum;
        
        threads[i].tid = threadResponse->tid;
        threads[i].time = threadResponse->time;

        free(response);
    }

    endTime = getTime();

    // Altera o nome do arquivo.
    sprintf(FileName, FILE_NAME, 6);

    createFile(FileName, DESCRIPTION_FILE, threads);

    return sum *= 4.0;
}

/* Cria uma thread para fazer a soma parcial de n termos da série de Leibniz. 
   Esta função deve usar a função sumPartial para definir qual a função a ser executada por cada uma das x threads 
   do programa, onde x é igual a NUMBER_OF_THREADS. 
   Retorna a identificação da thread.
*/
pthread_t createThread(unsigned int *terms) {
    pthread_t threadId;
    
    // Cria a thread
    pthread_create(&threadId, NULL, sumPartial, terms);
    
    return threadId;
}

/* Realiza a soma parcial de n (n é definido por PARTIAL_NUMBER_OF_TERMS) termos da série de Leibniz
   começando em x, por exemplo, como PARTIAL_NUMBER_OF_TERMS é 125.000.000, então se x é:

             0 -> calcula a soma parcial de 0 até 124.999.999;
   125.000.000 -> calcula a soma parcial de 125.000.000 até 249.999.999;
   250.000.000 -> calcula a soma parcial de 250.000.000 até 374.999.999; 
       
   e assim por diante. 

   O resultado dessa soma parcial deve ser um valor do tipo double a ser retornado 
   por esta função para o processo que criou a thread.
*/
void* sumPartial(void *terms){
    ThreadResponse *threadResponse = (ThreadResponse *)malloc(sizeof(ThreadResponse));
    unsigned int limit = *((unsigned int *) terms);
    CurrentTime startTime = getTime();
    CurrentTime endTime;

    threadResponse->sum = 0;
    
    // Os termos da soma serão do limite menos o número parcial de termos até o limite menos um.
    for (unsigned int indice = limit - PARTIAL_NUMBER_OF_TERMS; indice < limit; indice++)
    {
        int sign = (indice % 2 == 0) ? 1 : -1;
        double term = sign / (2.0 * indice + 1.0);
        threadResponse->sum += term;
    }

    threadResponse->tid  = gettid();
    
    endTime = getTime();

    // Obtém a duração da execução da thread.
    threadResponse->time = getDiffTime(startTime, endTime);

    pthread_exit((void *) threadResponse);
}

// Obtém a hora atual, retorna uma estrutura "CurrentTime".
CurrentTime getTime(){
    CurrentTime currentTime;
    time_t now;

    // Necessário para obter os milisegundos.
    struct timeval timeMilli;
    struct tm * currentTimeinfo;

    gettimeofday(&timeMilli,NULL);

    time(&now);
    currentTimeinfo = localtime(&now);

    currentTime.hour = currentTimeinfo->tm_hour;
    currentTime.min = currentTimeinfo->tm_min;
    currentTime.sec = currentTimeinfo->tm_sec;
    currentTime.milisec = (timeMilli.tv_sec * 1000LL) + (timeMilli.tv_usec / 1000LL);

    return currentTime;
}

// Obtém a hora atual em formato de um vetor de caracteres(string), recebe o tempo.
char* getTimeString(CurrentTime currentTime){
    char *timeString = (char*) malloc(9 * sizeof(char));

    sprintf(timeString, "%02d:%02d:%02d", currentTime.hour, currentTime.min, currentTime.sec);

    return timeString;
}

// Obtém a diferença de tempo entre duas estruturas "CurrenTime", retorna a diferença em segundos.
double getDiffTime(const CurrentTime start, const CurrentTime end) {
    // O cálculo é feito usando milisegundos.
    return (double) (end.milisec - start.milisec) / 1000.0f;
}

/* Cria o arquivo texto no diretório atual usando o nome do arquivo, a descrição e os dados do vetor do tipo Threads.
 * Retorna TRUE se o arquivo foi criado com sucesso ou FALSE se ocorreu algum erro.
 */
int createFile(const FileName fileName, String description, const Threads threads){
    FILE *file = fopen(fileName, "w+");

    if (file == NULL)
        return FALSE;

    fprintf(file, "Arquivo: %s\n", fileName);
    fprintf(file, description);

    for (size_t i = 0; i < NUMBER_OF_THREADS; i++)
    {
        fprintf(file, "\nTID %d: %.2f", threads[i].tid, threads[i].time);
    }
    
    fprintf(file, "\n\nTotal: %.2f s", getTotalTime(threads));

    fclose(file);

    return TRUE;
}

/* Cria o relatório do programa escrevendo na tela as informações da estrutura Report.
 * Retorna TRUE se o relatório foi escrito com sucesso ou FALSE se os dados da estrutura Report são vazios ou nulos.
*/
int createReport(const Report *report){
    if (report == NULL)
        return FALSE;
    
    if (*report->showProgramName == FALSE){
        printf("%s\n", report->programName);
        printf("\n%s", report->message1);
        printf("\n%s\n", report->message2);
    }

    printf("\n%s\n", report->processReport1.identification);
    printf("\n%s\n", report->processReport1.numberOfThreads);
    printf("\n%s", report->processReport1.start);
    printf("\n%s", report->processReport1.end);
    printf("\n%s\n", report->processReport1.duration);
    printf("\n%s\n", report->processReport1.pi);

    return TRUE;
}

//Cria area de memória compartilhada
int createSharedMemory()
{
    return shmget(IPC_PRIVATE, sizeof(unsigned char), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
}

//Retorna um unsigned char anexado na área de memória compartilhada
unsigned char *attachedSegmentMemory(int segmentId)
{
    return (unsigned char *)shmat(segmentId, NULL, 0);
}

//Desconecta da área de memória compartilhada
void desconnectSharedMemory(unsigned char *showProgramName)
{
    shmdt(showProgramName);
}

// Obtém o tempo total de execução das threads, recebe as estruturas do tipo thread e retorna o tempo total.
float getTotalTime(const Threads threads){
    float totalTime = 0;

    for (size_t i = 0; i < NUMBER_OF_THREADS; i++)
    {
        totalTime += threads[i].time;
    }
    
    return totalTime;
}