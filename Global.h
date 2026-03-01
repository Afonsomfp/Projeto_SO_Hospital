#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define NAME_PIPE "/tmp/input_pipe"
#define LOG_FILE_NAME "DEI_Emergency.log"

typedef struct {
    int pacientes_triados;
    int pacientes_atendidos;
    int temp_medio_antes_triagem;
    int temp_medio_entre_fim_incio;
    int media_temp_total;
    int somatorio_tempo_triagem;
    int somatorio_tempo_doutor;
    int somatorio_tempo_total;

    int num_pacientes_msq;
    int doctor_extra_flag;
    int terminar;
} estatisticas;

typedef struct {
    char nome[30];
    int tempo_triagem;
    int tempo_atendimento;
    int prioridade;

    time_t tempo_chegada;
    time_t tempo_inicio_triagem;
    time_t tempo_fim_triagem;
    time_t tempo_inicio_doutor;
    time_t tempo_fim_doutor;
} paciente;

typedef struct {
    long mtype;
    paciente p;
} MSQ;

typedef struct{
  int shift;
  int *turno_terminado;
} args_turno;

extern int triage_queue_max;
extern int triage;
extern int doctors;
extern int shift_length;
extern int msq_wait_max;

extern int id_MSQ;
extern int shmid;
extern estatisticas *shared_var;
extern sem_t *sem_estatisticas;

extern paciente *triage_queue;
extern int num_pac_queue;
extern int triage_max;
extern int triage_ativos;

extern pid_t *ids_doctors;
extern int criar_doctor_extra;
extern int criar_novo_doctor;

extern pthread_t *threads;
extern int *ids_triage;

extern char *log_mmap;
extern int fd_log;
extern sem_t *log_sem;
extern int tamanho_total_log;
extern size_t log_pos; //posição atual de escrita dentro do ficheiro

extern int fd_named_pipe;
extern pthread_mutex_t mutex;
extern pthread_cond_t variavel_cond_triagem;
extern struct sigaction action;
extern int ctrlc_pressionado;

void escrever_log_texto(const char *mensagem);
void log_doctor_processos(pid_t pid);
void log_triagem_threads(pthread_t id);
void log_inicio_fim_triagem(bool inicio, paciente *pac);
void pacientes_descartados(paciente *pac, char *motivo);
void sinais(int sig);
void inicio_fim_turno(bool inicio);
void log_erro(char *erro_mensagem);

void ler_configtxt(void);

void *triage_work(void *arg);
void inserir_paciente(paciente p);

void doctor_work(void);
void doctor_extra_work(void);

void clean_resources(void);

#endif
