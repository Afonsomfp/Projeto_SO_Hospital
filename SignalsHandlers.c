#include "Global.h"
#include "SignalsHandlers.h"

void handler_sigusr1(int sig){
  (void)sig;
  sinais(SIGUSR1);

  char msg[256];

  sprintf(msg, "\n\nNúmero de threads ativas: %d", triage_ativos);
  escrever_log_texto(msg);

  sprintf(msg, "Pacientes triados: %d", shared_var->pacientes_triados);
  escrever_log_texto(msg);

  sprintf(msg, "Pacientes atendidos: %d", shared_var->pacientes_atendidos);
  escrever_log_texto(msg);

  sprintf(msg, "Tempo médio antes da triagem: %d", shared_var->temp_medio_antes_triagem);
  escrever_log_texto(msg);

  sprintf(msg, "Tempo médio entre o fim e o inicio: %d", shared_var->temp_medio_entre_fim_incio);
  escrever_log_texto(msg);

  sprintf(msg, "Média do tempo total: %d", shared_var->media_temp_total);
  escrever_log_texto(msg);

  sprintf(msg, "Somatório do tempo de triagem: %d", shared_var->somatorio_tempo_triagem);
  escrever_log_texto(msg);

  sprintf(msg, "Somatório do tempo total: %d", shared_var->somatorio_tempo_total);
  escrever_log_texto(msg);

  sprintf(msg, "Somatório do tempo do doutor: %d", shared_var->somatorio_tempo_doutor);
  escrever_log_texto(msg);

  sprintf(msg, "Número de pacientes na Message Queue: %d", shared_var->num_pacientes_msq);
  escrever_log_texto(msg);

  sprintf(msg, "Doctor extra ativo: %d\n\n", shared_var->doctor_extra_flag);
  escrever_log_texto(msg);
}


//handler que vai fazer o wait sempre que um processo morre, para evitar processos zombies
void handler_sigchild(int sig){
  (void)sig;
  pid_t pid;
  while((pid = waitpid(-1, NULL, WNOHANG)) > 0){
    for(int i = 0; i < doctors; i++){
      if(ids_doctors[i] == pid){
        ids_doctors[i] = 0;
        criar_novo_doctor++;
        break;
      }
    }
  }
}

void handler_sigalrm(int sig){
  (void)sig;

}

void handler_doctor_extra(int sig){
  (void)sig;
  sinais(SIGUSR2);
  criar_doctor_extra = 1;
}

void handler_sigint(int sig){
  (void)sig;
  sinais(SIGINT);

  ctrlc_pressionado = 1;
  pthread_mutex_lock(&mutex);
  shared_var->terminar = 1;
  pthread_cond_broadcast(&variavel_cond_triagem); //acordar todas as threads
  pthread_mutex_unlock(&mutex);

  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  sigaddset(&mask, SIGUSR2);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask, NULL);
}

