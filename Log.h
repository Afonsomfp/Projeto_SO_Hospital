#ifndef LOG_H
#define LOG_H

#include "Global.h"

void escrever_log_texto(const char* mensagem);
void log_doctor_processos(pid_t pid);
void log_triagem_threads(pthread_t id);
void log_inicio_fim_triagem(bool inicio, paciente *pac);
void pacientes_descartados(paciente *pac, char *motivo);
void sinais(int sig);
void inicio_fim_turno(bool inicio);
void log_erro(char* erro_mensagem);

#endif 