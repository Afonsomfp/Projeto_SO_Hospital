#include "Global.h"
#include "ConfigTxt.h"

void ler_configtxt(){

  // Verificar se é preciso fazer algo com o pont quando dá erro
  FILE *pont;
  pont = fopen("config.txt","r"); 
  if(pont == NULL){
    log_erro("Erro ao abrir ficheiro");
    exit(1);
  }

  //ler ficheiro
  char string[50];
  char numero[10];
  int i, j, value, line = 0;
  while(fgets(string, sizeof(string), pont) != NULL){
    i = 0;
    j = 0;
    while(string[i] != '\0' && !isdigit(string[i])){ //Enquanto não chegar ao '\0' e não for digito: i++
      if(string[i] == '-'){
        log_erro("Não são aceites números negativos");
        fclose(pont);
        exit(1);
      }
      i++;
    }
    while(isdigit(string[i]) && j<(int)sizeof(numero)-1){ //o unsigned char para garantir que não lê números negativos
      numero[j] = string[i]; //Se o nº tiver mais de 9 dígitos rebenta o array numero[10] por isso é preciso limitar até j<(int)sizeof(numero)-1
      i++;
      j++;
    }
    numero[j] = '\0';
    value = atoi(numero);

    switch(line){
    case 0:
      triage_queue_max = value;
      printf("Valor máximo da triagem: %d\n", triage_queue_max);
      break;
    case 1:
      triage = value;
      printf("Número de threads: %d\n", triage);
      break;
    case 2: 
      doctors = value;
      printf("Número de doctors: %d\n", doctors);
      break;
    case 3:
      shift_length = value;
      printf("Tamanho do turno: %d\n", shift_length);
      break;
    case 4:
      msq_wait_max = value;
      printf("Tamanho maximo da msq: %d\n", msq_wait_max);
      break;
    default:
      printf("Ficheiro contém mais linhas que o esperado.");
      fclose(pont);
      exit(1);
    }
    line++;
  }
  fclose(pont);

  if(line<5){ //Erro caso o config.txt tenha menos linhas do que o esperado
    printf("Ficheiro contém menos linhas do que o esperado");
    exit(1);
  }

  //qualquer array global cujo tamanho depende de um valor lido do ficheiro OBRIGATORIAMENTE tem de ser criado com malloc porque o compilador não lê o ficheiro
  ids_doctors = (pid_t*)malloc(sizeof(pid_t)*doctors); 
  if(ids_doctors == NULL){
    log_erro("Erro na alocação da memória");
    clean_resources();
    exit(1);
  }
  ids_triage = (int*)malloc(sizeof(int)*triage); 
  if(ids_triage == NULL){
    log_erro("Erro na alocação da memória");
    clean_resources();
    exit(1);
  }
  threads = (pthread_t*)malloc(sizeof(pthread_t)*triage);
  if(threads == NULL){
    log_erro("Erro na alocação da memória");
    clean_resources();
    exit(1);
  }
  triage_queue = (paciente*)malloc(sizeof(paciente)*triage_queue_max);
  if(triage_queue == NULL){
    log_erro("Erro na alocação da memória");
    clean_resources();
    exit(1);
  }
}