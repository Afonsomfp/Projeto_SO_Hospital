  O sistema é composto por múltiplos processos e threads que permitem simular a triagem de pacientes e o atendimento dos mesmos pelos doutores de acordo com as diferentes prioridades e tempos, sendo utilizado 
diferentes mecanismos de IPC como named pipes, semaphores, shared memory, message queues, signals e memory mapped files, garantindo a correta sincronização e funcionamento do simulador.
  O meu programa é composto por um total de oito ficheiros tipo C, cujo responsável por todo o funcionamento e processamento do respectivo software é o “Projeto_SO”. Aqui é onde
crio/inicializo todos os mecanismos de IPC como named pipes, shared memory, message queues, signals, memory mapped files e semaphores, também crio os processos doctor, bem como as threads triagem. 
O desempenho do meu projeto baseia-se num loop que apenas termina quando o ctrl-c for pressionado. Assim, enquanto que o ctrl-c não for acionado para terminar o programa, este vai estar constantemente a 
verificar se é necessária a criação de novos doctors, tal como a criação do doctor extra, do mesmo modo que verifica a entrada de elementos pela pipe, fazendo o uso de um timeout.
  Uma das componentes mais importantes é o processo de triagem. Após a recepção dos pacientes vindos do named pipe, estes vão ser alocados na fila para serem triados, posteriormente será enviado um sinal para a 
variável de condição presente na função das threads de triagem. Logo que a thread for acordada por este mesmo sinal, irá retirar um pacientes da fila e executar a sua respetiva triagem, armazenando todos os dados 
indispensáveis para o cálculo das estatísticas. Visto que o número de threads pode ser alterado, caso o número seja reduzido para algo menor que o já presente valor, então pelo uso do respetivo id de cada thread, 
terminamos a execução da função da thread, isto é, a thread termina o seu trabalho. Na eventualidade da situação oposta, então a criação de novas threads é realizada.
  Outro componente essencial para o funcionamento deste projeto é referente ao trabalho do processo doctor. Este cria uma thread para realizar a contagem do tempo de turno, thread esta que irá avisar, enviando um 
sinal (SIGALRM), quando o turno terminar. Enquanto que o turno não terminar ou o programa não receber o sinal de término, o processo espera que seja enviado um paciente da triagem para atendimento. Uma vez que 
este chegue, o seu atendimento irá ser realizado, armazenando todos os dados necessários para o cálculo das estatísticas. No momento em que o turno terminar (consequentemente o término do processo é inevitável), 
um sinal (SIGCHLD) é enviado para o admission e a criação de um novo processo doctor é efetuado. Para além disso, a produção de um processo doctor extra pode ser possível, caso o número de pacientes em espera seja 
maior que 80% do número máximo de pacientes em espera.
  Todos os acontecimentos ocorridos durante a realização do projeto são conservados num ficheiro de texto “DEI_Emergency.log”, armazenamento este com as respectivas funcionalidades presente no ficheiro “Log.c”.
  Em suma, o sistema que foi desenvolvido permitiu obter um simulador de urgência hospitalar que abrange todo o funcionamento desde a admissão de pacientes até ao seu respetivo atendimento, recorrendo a múltiplos 
processos, threads e mecanismos de IPC, garantindo assim uma gestão eficiente dos dados e recursos disponíveis, e a correta recolha da informação estatística sobre a execução.
  
  Observação: De forma a alterar o número de threads disponíveis durante a execução do programa, o comando especial a ser enviado pela named pipe para alteração do parâmetro “TRIAGE” é, por exemplo, 
echo “TRIAGE 10”> /tmp/input_pipe
