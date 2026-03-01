//Afonso Peixoto - 2024199647
//André Ramos - 2024202276 

#include "Global.h"
#include "Doctor.h"
#include "Cleanup.h"
#include "ConfigTxt.h"
#include "Log.h"
#include "Triage.h"
#include "SignalsHandlers.h"

void inserir_paciente(paciente p){
  pthread_mutex_lock(&mutex);

  if(num_pac_queue < triage_queue_max){
    triage_queue[num_pac_queue] = p;
    num_pac_queue++;
    pthread_cond_signal(&variavel_cond_triagem);
  }else{
    printf("\nFila cheia, paciente descartado\n");
    pacientes_descartados(&p,"Fila cheia.");
  }
  
  pthread_mutex_unlock(&mutex);
}

bool verificar_numero(const char *string){
  int i=0;
  while(string[i] != '\0'){
    if(!isdigit(string[i])){
      return false;
    }
    i++;
  }
  return true;
}

int main(){ 

  pid_t pid;
  srand(time(NULL));

  //criação do semaforo log
  sem_unlink("LOG_SEM");
  log_sem = sem_open("LOG_SEM",O_CREAT|O_EXCL,0666,1);
  if(log_sem == SEM_FAILED){
    printf("Erro no sem_open");
    clean_resources();
    exit(1); 
  }

  if((fd_log = open(LOG_FILE_NAME, O_RDWR | O_CREAT, 0666)) < 0){
    printf("Erro ao abrir ficheiro de log");
    clean_resources();
    exit(1);
  }
  
  //
  if(ftruncate(fd_log, tamanho_total_log) == -1){
    printf("Erro no ftruncate");
    clean_resources();
    exit(1);
  }

  if((log_mmap = mmap(NULL, tamanho_total_log, PROT_READ | PROT_WRITE, MAP_SHARED, fd_log, 0)) == MAP_FAILED){ 
    printf("Erro no mmap");
    clean_resources();
    exit(1);
  }

  escrever_log_texto("Início do programa");

  //leitura e atribuição dos valores às variáveis presentes em config.txt
  ler_configtxt();

  triage_max = triage;
  triage_ativos = triage;

  //controlo do ^C
  action.sa_handler = handler_sigint;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGINT, &action, NULL);

  //semaforo para protejer a shared var
  sem_unlink("SEM_ESTATISTICAS");
  sem_estatisticas = sem_open("SEM_ESTATISTICAS",O_CREAT|O_EXCL,0666,1);
  if(sem_estatisticas == SEM_FAILED){
    log_erro("Erro no sem_open");
    clean_resources();
    exit(1); 
  }

  //criar a named pipe
  unlink(NAME_PIPE);
  if(mkfifo(NAME_PIPE, O_CREAT|O_EXCL|0666) < 0){
    log_erro("Erro na criação da named pipe");
    clean_resources();
    exit(1);
  }

  //criar MSQ
  id_MSQ = msgget(IPC_PRIVATE, IPC_CREAT|0777);
  if(id_MSQ == -1){
    log_erro("Erro na criação da triage queue");
    clean_resources();
    exit(1);
  }

  //criação da memória partilhada e inicialização
  if((shmid = shmget(IPC_PRIVATE, sizeof(estatisticas), IPC_CREAT | 0766)) < 0){
    log_erro("Erro no shmget");
    clean_resources();
    exit(1);
  }
  if((shared_var = shmat(shmid, NULL, 0)) == (void*)-1){
    log_erro("Erro no shmat");
    clean_resources();
    exit(1);
  }

  shared_var->pacientes_triados = 0;
  shared_var->pacientes_atendidos = 0;
  shared_var->temp_medio_antes_triagem = 0;
  shared_var->temp_medio_entre_fim_incio = 0;
  shared_var->media_temp_total = 0;
  shared_var->somatorio_tempo_triagem = 0;
  shared_var->somatorio_tempo_total = 0;
  shared_var->somatorio_tempo_doutor = 0;
  shared_var->num_pacientes_msq = 0;
  shared_var->doctor_extra_flag = 0;
  shared_var->terminar = 0;

  //caso receba um sinal para mostrar estatisticas
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_handler = handler_sigusr1;
  sigaction(SIGUSR1, &action, NULL);
  //caso receba um sinal para criar doutor extra
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_handler = handler_doctor_extra;
  sigaction(SIGUSR2, &action, NULL);
  //caso receba um sinal de termino de processo
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_handler = handler_sigchild;
  sigaction(SIGCHLD, &action, NULL);


  //criação dos processos doutor
  for(int i = 0; i < doctors; i++){
    pid = fork();
    if(pid == -1){
      log_erro("Erro no fork");
      clean_resources();
      exit(1);
    }
    if(pid == 0){
      signal(SIGINT, SIG_IGN);
      doctor_work();
      exit(0);
    }
    else{
      ids_doctors[i] = pid;
      log_doctor_processos(pid);
    }
  }

  //criação das threads de  triagem
  for(int i = 0; i < triage; i++){
    ids_triage[i] = i+1;
    if(pthread_create(&threads[i], NULL, triage_work, &ids_triage[i]) != 0){
      log_erro("Erro na criação das threads");
      clean_resources();
      exit(1);
    }
    log_triagem_threads(threads[i]);
  }


  if((fd_named_pipe = open(NAME_PIPE, O_RDONLY | O_NONBLOCK)) < 0){
    log_erro("Erro na abertura da pipe");
    clean_resources();
    exit(1);
  } 

  paciente pac;                                                 
  const char *nomes[] = {"Ana","Bruno","Carla","Tomás","Maria","José","Francisca","Pedro"};
  char buffer[512];
  char *split_buffer;
  ssize_t tamanho_buffer;
  fd_set fds;
  struct timeval timeout; 

  while(ctrlc_pressionado == 0){
    FD_ZERO(&fds);
    FD_SET(fd_named_pipe, &fds);

    //timeout de 0.5s
    timeout.tv_sec = 0; 
    timeout.tv_usec = 500000;

    if(criar_doctor_extra == 1){
      criar_doctor_extra = 0;
      pid_t pid_extra = fork();
      printf("\n\n Doctor extra criado\n\n");
      if (pid_extra == -1) {
        log_erro("Erro no fork");
        clean_resources();
        exit(1);
      }else if (pid_extra == 0) {
        doctor_extra_work();
        exit(0);
      }
    }
    while(criar_novo_doctor > 0){
      criar_novo_doctor--;
      pid_t new_pid = fork();
      if (new_pid == -1) {
        log_erro("Erro no fork em handler_doctor");
        exit(1);
      }else if (new_pid == 0) {
        doctor_work();
        exit(0);
      }
      int nova_posicao = -1;
      for(int i = 0; i < doctors; i++){
        if(ids_doctors[i] == 0){
          nova_posicao = i;
          break;
        }
      }
      if(nova_posicao == -1){
        log_erro("Sem slot livre em ids_doctors para novo doctor");
      } else {
        ids_doctors[nova_posicao] = new_pid;
        log_doctor_processos(new_pid);
      }
    }
    
    if(select(fd_named_pipe + 1, &fds, NULL, NULL, &timeout)>0 && FD_ISSET(fd_named_pipe, &fds)){
      tamanho_buffer = read(fd_named_pipe, buffer, sizeof(buffer) - 1);
      if(tamanho_buffer > 0){
        buffer[tamanho_buffer] = '\0';

        pac.tempo_chegada = time(NULL);
        split_buffer = strtok(buffer, " \t\r\n");

        if(split_buffer == NULL){
          continue;
        }

        if(verificar_numero(split_buffer)){

          char *tempo_triagem = strtok(NULL, " \t\r\n");
          char *tempo_atendimento = strtok(NULL, " \t\r\n");
          char *prioridade = strtok(NULL, " \t\r\n");

          if(tempo_triagem == NULL || tempo_atendimento == NULL || prioridade == NULL){
            log_erro("Mensagem de grupo incompleta");
            continue;
          }

          for(int i = 0; i < atoi(split_buffer); i++){
            strcpy(pac.nome, nomes[rand() % 8]);
            pac.tempo_triagem = atoi(tempo_triagem);
            pac.tempo_atendimento = atoi(tempo_atendimento);
            pac.prioridade = atoi(prioridade);

            inserir_paciente(pac);
          }

        }else if(strncmp(split_buffer, "TRIAGE", 6) == 0){
          char *valor_char = strtok(NULL, " \t\r\n");

          if(valor_char == NULL){
            log_erro("valor invalido");
            continue;
          }

          int valor = atoi(valor_char);

          if (valor <= 0) {
            printf("Valor TRIAGE inválido: %d\n", valor);
            continue;
          }

          pthread_mutex_lock(&mutex);

          if (valor > triage_max) {
            int antigo_max = triage_max;

            //realocar arrays
            ids_triage = realloc(ids_triage, valor * sizeof(int));
            if (ids_triage == NULL) {
              log_erro("realloc ids_triage");
              pthread_mutex_unlock(&mutex);
              clean_resources();
              exit(1);
            }

            threads = realloc(threads, valor * sizeof(pthread_t));
            if (threads == NULL) {
              log_erro("realloc threads");
              pthread_mutex_unlock(&mutex);
              clean_resources();
              exit(1);
            }

            for (int i = antigo_max; i < valor; i++) {
              ids_triage[i] = i + 1;

              if (pthread_create(&threads[i], NULL, triage_work, &ids_triage[i]) != 0) {
                log_erro("Erro na criação das threads de triagem adicionais");
                pthread_mutex_unlock(&mutex);
                clean_resources();
                exit(1);
              }
            }

            triage_max = valor;
          }

          triage_ativos = valor;

          //acordar todas as threads de triagem para reavaliarem o estado
          pthread_cond_broadcast(&variavel_cond_triagem);

          pthread_mutex_unlock(&mutex);

          continue;
        }else{
          char *tempo_triagem = strtok(NULL, " \t\r\n");
          char *tempo_atendimento = strtok(NULL, " \t\r\n");
          char *prioridade = strtok(NULL, " \t\r\n");

          if(tempo_triagem == NULL || tempo_atendimento == NULL || prioridade == NULL){
            log_erro("Mensagem errada no pipe (falta campos)");
            continue;
          }

          strcpy(pac.nome, split_buffer);
          pac.tempo_triagem = atoi(tempo_triagem);
          pac.tempo_atendimento = atoi(tempo_atendimento);
          pac.prioridade = atoi(prioridade);

          inserir_paciente(pac);
        }
      }else{
        continue;
      }
    }
  }  
  msync(log_mmap, log_pos, MS_SYNC);

  escrever_log_texto("Fim do programa");
  clean_resources();
  return 0;
}