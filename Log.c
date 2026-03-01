#include "Global.h"
#include "Log.h"

void escrever_log_texto(const char *mensagem){
  char buffer[512];
  time_t agora = time(NULL);
  char *ts = ctime(&agora);
  ts[strcspn(ts, "\n")] = '\0'; //retirar o \n do ctime
  
  sprintf(buffer, "[%s] %s\n", ts, mensagem);

  sem_wait(log_sem);

  //escrever no terminal
  printf("%s", buffer);

  //escrever no MMF
  size_t len = strlen(buffer);
  if (log_pos + len >= tamanho_total_log) {
    log_pos = 0;
  }
  memcpy(log_mmap + log_pos, buffer, len);
  log_pos += len;

  sem_post(log_sem);
}

void log_doctor_processos(pid_t pid){
    char msg[128];
    sprintf(msg, "Processo Doctor com o id %d criado", pid);
    escrever_log_texto(msg);
}

void log_triagem_threads(pthread_t id){
    char msg[128];
    sprintf(msg, "Thread de triagem com o id %ld criada", id);
    escrever_log_texto(msg);
}

void log_inicio_fim_triagem(bool inicio, paciente *pac){
    char msg[256];
    if(inicio){
        sprintf(msg, "Início da triagem do paciente %s", pac->nome);
    } else {
        sprintf(msg, "Fim da triagem do paciente %s", pac->nome);
    }
    escrever_log_texto(msg);
}

void pacientes_descartados(paciente *pac, char *motivo){
    char msg[256];
    sprintf(msg, "Paciente %s descartado porque %s", pac->nome, motivo);
    escrever_log_texto(msg);
}

void sinais(int sig){
    char msg[128];
    sprintf(msg, "Sinal %d recebido", sig);
    escrever_log_texto(msg);
}

void inicio_fim_turno(bool inicio){
    char msg[128];
    if(inicio){
        sprintf(msg, "Início do turno do doctor");
    } else {
        sprintf(msg, "Fim do turno do doctor");
    }
    escrever_log_texto(msg);
}

void log_erro(char* erro_mensagem){
    char msg[256];
    sprintf(msg, "Erro: %s", erro_mensagem);
    escrever_log_texto(msg);
}

