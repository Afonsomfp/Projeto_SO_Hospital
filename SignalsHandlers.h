#ifndef SIGNALHANDLERS_H
#define SIGNALHANDLERS_H

#include "Global.h"

void handler_sigusr1(int sig);
void handler_sigchild(int sig);
void handler_sigalrm(int sig);
void handler_doctor_extra(int sig);
void handler_sigint(int sig);

#endif 