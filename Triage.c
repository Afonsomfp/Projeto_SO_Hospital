#include "Global.h"
#include "Triage.h"
#include "Log.h"

int retirar_paciente(paciente *eliminar, int id_triage){
  pthread_mutex_lock(&mutex);

  while (num_pac_queue == 0 && shared_var->terminar == 0 && id_triage <= triage_ativos) {
    pthread_cond_wait(&variavel_cond_triagem, &mutex);
  }

  if (shared_var->terminar || id_triage > triage_ativos) {
    pthread_mutex_unlock(&mutex);
    return 0;
  }

  *eliminar = triage_queue[0]; //aqui é onde associo o paciente que retiro da fila

  for (int i = 1; i < num_pac_queue; i++){
    triage_queue[i - 1] = triage_queue[i];
  }
  num_pac_queue--;

  pthread_mutex_unlock(&mutex);
  return 1;
}

void* triage_work(void* arg){

  paciente pac_removido;
  MSQ new;

  int id_triage = *(int*)arg;

  while(!shared_var->terminar){

    if(id_triage > triage_ativos){
      break;
    }

    if(retirar_paciente(&pac_removido, id_triage) == 0){
      break;
    }
    
    //Instante imediatamente antes do início da triagem, não é mm o tempo da triagem
    time_t inicio_triagem = time(NULL);
    pac_removido.tempo_inicio_triagem = inicio_triagem;
    log_inicio_fim_triagem(true, &pac_removido);

    int tempo_medio_espera = (int)(pac_removido.tempo_inicio_triagem - pac_removido.tempo_chegada);

    sem_wait(sem_estatisticas);

    shared_var->somatorio_tempo_triagem += tempo_medio_espera;
    shared_var->pacientes_triados++;

    shared_var->temp_medio_antes_triagem = shared_var->somatorio_tempo_triagem / shared_var->pacientes_triados;

    sem_post(sem_estatisticas);


    //Início da triagem
    sleep(pac_removido.tempo_triagem);

    time_t fim_triagem = time(NULL);
    pac_removido.tempo_fim_triagem = fim_triagem;
    log_inicio_fim_triagem(false, &pac_removido);

    new.mtype = pac_removido.prioridade;
    new.p = pac_removido;

    msgsnd(id_MSQ, &new, sizeof(MSQ) - sizeof(long), 0);
    sem_wait(sem_estatisticas);
    shared_var->num_pacientes_msq++;
    sem_post(sem_estatisticas);
  }

  return NULL;
}