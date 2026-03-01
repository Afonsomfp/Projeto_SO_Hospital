CFLAGS = -Wall -Wextra -pthread

all: projeto_SO_mf

projeto_SO_mf: Cleanup.o ConfigTxt.o Log.o Projeto_SO.o Doctor.o SignalsHandlers.o Triage.o Global.o
	@echo "Producing the final application"
	gcc $(CFLAGS) Cleanup.o ConfigTxt.o Log.o Projeto_SO.o Doctor.o SignalsHandlers.o Triage.o Global.o -o projeto_SO_mf

Cleanup.o : Cleanup.c Cleanup.h Global.h
	@echo "Compiling Cleanup.c"
	gcc $(CFLAGS) -c Cleanup.c -o Cleanup.o

ConfigTxt.o : ConfigTxt.c ConfigTxt.h Global.h
	@echo "Compiling ConfigTxt.c"
	gcc $(CFLAGS) -c ConfigTxt.c -o ConfigTxt.o

Log.o : Log.c Log.h Global.h
	@echo "Compiling Log.c"
	gcc $(CFLAGS) -c Log.c -o Log.o

Projeto_SO.o : Projeto_SO.c Global.h Doctor.h Cleanup.h ConfigTxt.h Log.h Triage.h SignalsHandlers.h
	@echo "Compiling Projeto_SO.c"
	gcc $(CFLAGS) -c Projeto_SO.c -o Projeto_SO.o

Doctor.o : Doctor.c Doctor.h Global.h Log.h SignalsHandlers.h
	@echo "Compiling Doctor.c"
	gcc $(CFLAGS) -c Doctor.c -o Doctor.o

SignalsHandlers.o : SignalsHandlers.c SignalsHandlers.h Global.h
	@echo "Compiling SignalsHandlers.c"
	gcc $(CFLAGS) -c SignalsHandlers.c -o SignalsHandlers.o

Triage.o : Triage.c Triage.h Global.h Log.h
	@echo "Compiling Triage.c"
	gcc $(CFLAGS) -c Triage.c -o Triage.o

Global.o : Global.c Global.h 
	@echo "Compiling Global.c"
	gcc $(CFLAGS) -c Global.c -o Global.o

clean:
	@echo "Removing everything, execpt the source file"
	rm Cleanup.o ConfigTxt.o Log.o Projeto_SO.o Doctor.o SignalsHandlers.o Triage.o Global.o projeto_SO_mf
