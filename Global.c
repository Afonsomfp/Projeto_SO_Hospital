#include "Global.h"

int triage_queue_max;
int triage;
int doctors;
int shift_length;
int msq_wait_max;

int id_MSQ;
int shmid;
estatisticas *shared_var;
sem_t *sem_estatisticas;

paciente *triage_queue;
int num_pac_queue = 0;
int triage_max;
int triage_ativos;

pid_t *ids_doctors;
int criar_doctor_extra = 0;
int criar_novo_doctor = 0;

pthread_t *threads;
int *ids_triage;

char *log_mmap;
int fd_log;
sem_t *log_sem;
int tamanho_total_log = 20 * 1024 * 1024; // 20 MB
size_t log_pos = 0;

int fd_named_pipe;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t variavel_cond_triagem = PTHREAD_COND_INITIALIZER;
struct sigaction action;
int ctrlc_pressionado = 0;