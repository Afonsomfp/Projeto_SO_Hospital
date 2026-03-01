#include "Global.h"
#include "Cleanup.h"

void clean_resources(){
  printf("A limpar recursos...\n");

  pthread_mutex_lock(&mutex);
  shared_var->terminar = 1;
  pthread_cond_broadcast(&variavel_cond_triagem);
  pthread_mutex_unlock(&mutex);

  for(int i=0;i<doctors;i++){
    if(ids_doctors[i] > 0){
      kill(ids_doctors[i], SIGTERM);
    }
  }

  for(int i=0;i<doctors;i++){
    if(ids_doctors[i] > 0){
      waitpid(ids_doctors[i], NULL, 0);
    }
  }

  for(int i=0;i<triage_max;i++){
    pthread_join(threads[i],NULL);
  }

  shmdt(shared_var);
  shmctl(shmid, IPC_RMID, NULL);
  msgctl(id_MSQ, IPC_RMID, NULL);
  unlink(NAME_PIPE);
  sem_close(sem_estatisticas);
  sem_unlink("SEM_ESTATISTICAS");
  munmap(log_mmap, tamanho_total_log);
  close(fd_log);
  sem_close(log_sem);
  sem_unlink("LOG_SEM");

  free(threads);
  free(triage_queue); 
  free(ids_triage);
  printf("\nTerminado");
  exit(0);
}