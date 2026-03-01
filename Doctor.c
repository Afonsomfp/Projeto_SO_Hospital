#include "Global.h"
#include "Doctor.h"
#include "Log.h"
#include "SignalsHandlers.h"

void doctor_extra_work() {
    MSQ new_pac;

    while (!shared_var->terminar) {  
      if(msgrcv(id_MSQ, &new_pac, sizeof(MSQ) - sizeof(long), -3, 0) == -1) {
          log_erro("Erro msgrcv");
          break;
      }

      time_t inicio_doctor = time(NULL);
      new_pac.p.tempo_inicio_doutor = inicio_doctor;

      sem_wait(sem_estatisticas);

      shared_var->num_pacientes_msq--;

      shared_var->pacientes_atendidos++;
      shared_var->somatorio_tempo_doutor += (int)(new_pac.p.tempo_inicio_doutor - new_pac.p.tempo_fim_triagem);
      shared_var->temp_medio_entre_fim_incio = shared_var->somatorio_tempo_doutor / shared_var->pacientes_atendidos;

      sem_post(sem_estatisticas);

      //atendimento
      sleep(new_pac.p.tempo_atendimento);

      time_t fim_doutor = time(NULL);
      new_pac.p.tempo_fim_doutor = fim_doutor;

      sem_wait(sem_estatisticas);

      shared_var->somatorio_tempo_total += (int)(new_pac.p.tempo_fim_doutor - new_pac.p.tempo_chegada);
      shared_var->media_temp_total = shared_var->somatorio_tempo_total / shared_var->pacientes_atendidos;


      if(shared_var->num_pacientes_msq < (int)(msq_wait_max * 0.8)) {
          shared_var->doctor_extra_flag = 0;
          sem_post(sem_estatisticas);
          break;
      }
      sem_post(sem_estatisticas);
    }
}

void *turno_contador(void *arg){
  args_turno *n = (args_turno*)arg;
  sleep(n->shift);
  *(n->turno_terminado) = 1;
  kill(getpid(), SIGALRM);
  return NULL;
}

void doctor_work() {
    signal(SIGALRM, handler_sigalrm); //para o sigalrm não efetuar o comportamento padrão
    int turno_terminado = 0;
    MSQ novo_paciente;
    time_t inicio_turno = time(NULL);
    inicio_fim_turno(true);

    args_turno argumentos;
    argumentos.shift = shift_length;
    argumentos.turno_terminado = &turno_terminado;

    pthread_t thread_turno;
    if(pthread_create(&thread_turno, NULL, turno_contador, &argumentos) != 0){
      log_erro("Erro na criação das threads");
      exit(1);
    }    

    while (!shared_var->terminar) {
      if(msgrcv(id_MSQ, &novo_paciente, sizeof(MSQ) - sizeof(long), 0, 0) == -1) {
        if(errno == EINTR){ //verificar se foi interrompido por um sinal
          if(shared_var->terminar == 1){
            pthread_cancel(thread_turno);
            pthread_join(thread_turno, NULL);
            break;
          }
          pthread_join(thread_turno, NULL);
          inicio_fim_turno(false);
          sinais(SIGALRM);
          break;
        }
        log_erro("Erro msgrcv");
        break;
      }

      sem_wait(sem_estatisticas);
      if (shared_var->num_pacientes_msq > msq_wait_max && shared_var->doctor_extra_flag == 0) {
        if(shared_var->doctor_extra_flag == 0){
          shared_var->doctor_extra_flag = 1;
          kill(getppid(), SIGUSR2);
          sem_post(sem_estatisticas);
        }
        sem_post(sem_estatisticas);
      }else{
        sem_post(sem_estatisticas);
      }
      
      sem_wait(sem_estatisticas);
      shared_var->num_pacientes_msq--;
      sem_post(sem_estatisticas);

      time_t inicio_doctor = time(NULL);
      novo_paciente.p.tempo_inicio_doutor = inicio_doctor;

      sem_wait(sem_estatisticas);
      shared_var->somatorio_tempo_doutor += (int)(novo_paciente.p.tempo_inicio_doutor - novo_paciente.p.tempo_fim_triagem);
      sem_post(sem_estatisticas);

      // atendimento
      sleep(novo_paciente.p.tempo_atendimento);

      time_t fim_doutor = time(NULL);
      novo_paciente.p.tempo_fim_doutor = fim_doutor;  

      sem_wait(sem_estatisticas);

      shared_var->pacientes_atendidos++;
      shared_var->temp_medio_entre_fim_incio = shared_var->somatorio_tempo_doutor / shared_var->pacientes_atendidos;
      shared_var->somatorio_tempo_total += (int)(novo_paciente.p.tempo_fim_doutor - novo_paciente.p.tempo_chegada);
      shared_var->media_temp_total = shared_var->somatorio_tempo_total / shared_var->pacientes_atendidos;

      sem_post(sem_estatisticas);
      if(turno_terminado) {
        pthread_join(thread_turno, NULL);
        inicio_fim_turno(false);
        sinais(SIGALRM);
        break;
      }
    }
}
