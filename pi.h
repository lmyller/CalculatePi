#pragma once

#include <pthread.h> // Requerido pela API Pthreads (POSIX Threads).

// Constantes lógicas.
#define TRUE 1
#define FALSE 0

// Tamanho do nome do arquivo.
#define FILE_NAME_SIZE 10

// Número de threads do processo filho.
#define NUMBER_OF_THREADS 16

// Tamanho padrão de string.
#define STRING_DEFAULT_SIZE 128

#define NUMBER_OF_PROCESS 2

// Número de casas decimais do número pi.
#define DECIMAL_PLACES 9

// Número máximo de termos da série de Leibniz.
#define MAXIMUM_NUMBER_OF_TERMS 2000000000

// Número parcial de termos da série de Leibniz.
#define PARTIAL_NUMBER_OF_TERMS 125000000

// Define uma string de tamanho padrão T, onde T é igual STRING_DEFAULT_SIZE.
typedef char String[STRING_DEFAULT_SIZE];

// Nome do arquivo.
typedef char FileName[FILE_NAME_SIZE];

// Nome do programa
#define PROGRAM_NAME "Cálculo do Número π"

// Mensagem 1 do processo pai
#define MESSAGE_1 "Criando os processos filhos pi1 e pi2..."

// Mensagem 2 do processo pai
#define MESSAGE_2 "Processo pai (PID %d) finalizou sua execução."

// Nome do arquivo
#define FILE_NAME "Processo%d.txt"

// Identificação do processo filho na estrutura "Report"
#define PROCESS_REPORT_IDENTIFICATION "Processo Filho: pi%d (PID %ld)"

// Número de threads na estrutura "Report"
#define PROCESS_REPORT_NUMBER_OF_THREADS "Nº de threads: %d"

// Início do processo filho na estrutura "Report"
#define PROCESS_REPORT_START "Início: %s"

// Encerramento do processo filho na estrutura "Report"
#define PROCESS_REPORT_END "Fim: %s"

// Duração do processo filho na estrutura "Report"
#define PROCESS_REPORT_DURATION "Duração: %.2f s"

// Soma total na estrutura "Repor"
#define PROCESS_REPORT_PI "Pi = %.9f"

// Descrição no arquivo gerado pelo processo
#define DESCRIPTION_FILE "Tempo em segundos das 16 threads do processo filho pi%d"

// Estrutura do relatório a ser gerado pelo processo.
typedef struct {
   // Os comentários abaixo são apenas exemplos de valores a serem armazenados nos campos desta estrutura.
   String 
      identification, // Processo Filho: pi1 (PID 6924)
      numberOfThreads, // Nº de threads: 16
      start, // Início: 10:45:12
      end, // Fim: 10:45:21
      duration, // Duração: 9,59 s
      pi; // Pi = 3,141592653   
} ProcessReport;

// Estrutura do relatório a ser gerado pelo programa.
typedef struct {
   // Os comentários abaixo são apenas exemplos de valores a serem armazenados nos campos desta estrutura.
   String 
      programName, // Cálculo do Número π
      message1, // Criando os processos filhos pi1 e pi2...
      message2; // Processo pai (PID 6923) finalizou sua execução.
   
   unsigned short processNumber;  // Número do processo
   unsigned char *showProgramName;
   
   ProcessReport processReport1, processReport2;     
} Report;

// Representa a identificação da thread e o seu tempo de execução em segundos.
typedef struct  {
   pthread_t threadID; // Identificação da thread obtida com pthread_create.
   pid_t tid;          // Identificação da thread obtida com gettid.
   double time; 
} Thread;

typedef struct {
   pthread_t threadID; // Identificação da thread.
   pid_t tid;          // TID da thread
   double sum;         // Soma calculada na thread.
   double time;        // Tempo de execução da thread
} ThreadResponse;

typedef struct 
{
   unsigned short hour;       // Hora
   unsigned short min;        // Minuto
   unsigned int sec;          // Segundo
   unsigned long int milisec; // Milisegundo
} CurrentTime;

// Relação de threads do processo filho.
typedef Thread Threads[NUMBER_OF_THREADS]; 

/* Cria o relatório do programa escrevendo na tela as informações da estrutura Report.
 * Retorna TRUE se o relatório foi escrito com sucesso ou FALSE se os dados da estrutura Report são vazios ou nulos.
*/
int createReport(const Report *report);

/* Cria o arquivo texto no diretório atual usando o nome do arquivo, a descrição e os dados do vetor do tipo Threads.
 * Retorna TRUE se o arquivo foi criado com sucesso ou FALSE se ocorreu algum erro.
 */
int createFile(const FileName fileName, String description, const Threads threads);

/* Cria uma thread para fazer a soma parcial de n termos da série de Leibniz. 
   Esta função deve usar a função sumPartial para definir qual a função a ser executada por cada uma das x threads 
   do programa, onde x é igual a NUMBER_OF_THREADS. 
   Retorna a identificação da thread.
*/
pthread_t createThread(unsigned int *terms); 

/* Realiza a soma parcial de n (n é definido por PARTIAL_NUMBER_OF_TERMS) termos da série de Leibniz
   começando em x, por exemplo, como PARTIAL_NUMBER_OF_TERMS é 125.000.000, então se x é:

             0 -> calcula a soma parcial de 0 até 124.999.999;
   125.000.000 -> calcula a soma parcial de 125.000.000 até 249.999.999;
   250.000.000 -> calcula a soma parcial de 250.000.000 até 374.999.999; 
       
   e assim por diante. 

   O resultado dessa soma parcial deve ser um valor do tipo double a ser retornado 
   por esta função para o processo que criou a thread.
*/
void* sumPartial(void *terms);

/* Calcula o número pi com n (n é definido por DECIMAL_PLACES) casas decimais usando o número máximo de
   termos da série de Leibniz, que é definido por MAXIMUM_NUMBER_OF_TERMS. Esta função deve criar x threads
   usando a função createThread, onde x é igual a NUMBER_OF_THREADS. 
*/
double calculationOfNumberPi(unsigned int terms);

//Cria area de memória compartilhada
int createSharedMemory();

//Retorna um unsigned char anexado na área de memória compartilhada
unsigned char *attachedSegmentMemory(int segmentId);

//Desconecta da área de memória compartilhada
void desconnectSharedMemory(unsigned char *showProgramName);

// Obtém os dados do processo pai e armazena em uma estrutura "Report".
void getReport(Report *report);

// Executa as funções do processo filho, recebe uma estrutura report com os dados do processo pai.
void childProcess(Report report, int segmentId);

// Obtém a hora atual, retorna uma estrutura "CurrentTime".
CurrentTime getTime();

// Obtém a hora atual em formato de um vetor de caracteres(string), recebe o tempo.
char* getTimeString(CurrentTime currentTime);   

// Obtém a diferença de tempo entre duas estruturas "CurrenTime", retorna a diferença em segundos.
double getDiffTime(const CurrentTime start, const CurrentTime end);

// Obtém o tempo total de execução das threads, recebe as estruturas do tipo thread e retorna o tempo total.
float getTotalTime(const Threads threads);

// Cria o processo
pid_t createProcess();

/*
 * Esta função inicia o programa.
 * Retorna EXIT_SUCCESS.
 */
int pi();