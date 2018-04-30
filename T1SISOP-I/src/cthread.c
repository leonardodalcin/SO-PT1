#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ucontext.h>

#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#define stackSize SIGSTKSZ


bool isBooted = false;
FILA2 readyQueue;
FILA2 blockedQueue;
FILA2 suspendedReadyQueue;
FILA2 suspendedBlockedQueue;
TCB_t *currentThread;
TCB_t *mainThread;
TCB_t *dispatchThread;
TCB_t *endThread;

void dispatch(){
  printf("[dispatch] Starting dispatch proc\n");
  if(FirstFila2(&readyQueue) == 0) {
      printf("[dispatch] There is something in the queue\n");
      if(currentThread) printf("[dispatch] Last thread tid: %d\n", currentThread->tid);
      currentThread = (TCB_t *) GetAtIteratorFila2(&readyQueue);
      DeleteAtIteratorFila2(&readyQueue);
      currentThread->state = PROCST_EXEC;
      if(currentThread) printf("[dispatch] Current thread tid: %d\n", currentThread->tid);
        setcontext(&currentThread->context);
   } else {
     printf("[dispatch] Queue is empty\n");
     return;
   }
}

TCB_t* findInQueueByTid(int tid, FILA2 queue, bool shouldRemove){
  printf("[findInQueueByTid] Starting proc\n");
  if(FirstFila2(&queue) == 0) {
    while(GetAtIteratorFila2(&queue) != NULL){
  		TCB_t *threadInQueue = (TCB_t *) GetAtIteratorFila2(&queue);
        if(threadInQueue->tid == tid) {
          if(shouldRemove) DeleteAtIteratorFila2(&queue);
          printf("[findInQueueByTid] found proc %d\n", threadInQueue->tid);
    			return threadInQueue;
        }
        if(NextFila2(&queue) != 0)
    			return -1;
    }
	}
  return -1;
}

void dispatchThreadProc(){
	printf("[Dispatch Thread Proc] TID = %d\n", currentThread->tid);
  currentThread = NULL;
  dispatch();
  return;
}

void unblock(int tid){
  printf("[unblock] TID = %d\n", tid);
	currentThread->isJoined = false;
  currentThread->jointid = NULL;
  TCB_t* unblockedThread = findInQueueByTid(tid, blockedQueue, 1);
  printf("[unblock] Found blocked thread TID = %d\n", unblockedThread->tid);
  if(!unblockedThread) return;
  else AppendFila2(&readyQueue, unblockedThread);
  return;
}

void endThreadProc(){
	printf("[Finishing thread] TID = %d\n", currentThread->tid);
  if(currentThread->isJoined) unblock(currentThread->jointid);
  free(currentThread);
  currentThread = NULL;
  dispatch();
  return;
}

void boot(){
  printf("[boot] Starting proc\n");

  printf("[boot] Creating main thread\n");
	mainThread = (TCB_t*) malloc(sizeof(TCB_t));
	mainThread->tid = 0;
	mainThread->prio = 0;
	mainThread->state = PROCST_CRIACAO;
	getcontext(&mainThread->context);
  currentThread = mainThread;
  printf("[boot] Created main thread\n");

  printf("[boot] Creating dispatch thread\n");
  dispatchThread = (TCB_t*) malloc(sizeof(TCB_t));
  getcontext(&dispatchThread->context);
  dispatchThread->context.uc_link = 0;
  dispatchThread->context.uc_stack.ss_sp = (char*) malloc(stackSize);
  dispatchThread->context.uc_stack.ss_size = stackSize;
  makecontext(&dispatchThread->context, (void(*)(void))dispatchThreadProc, 0);
  printf("[boot] Created end thread\n");

  printf("[boot] Creating end thread\n");
  endThread = (TCB_t*) malloc(sizeof(TCB_t));
  getcontext(&endThread->context);
	endThread->context.uc_link = 0;
	endThread->context.uc_stack.ss_sp = (char*) malloc(stackSize);
	endThread->context.uc_stack.ss_size = stackSize;
	makecontext(&endThread->context, (void(*)(void))endThreadProc, 0);
	printf("[boot] Created end thread\n");

	printf("[boot] Starting ready and blocked queues\n");
	CreateFila2(&readyQueue);
	CreateFila2(&blockedQueue);
  CreateFila2(&suspendedReadyQueue);
  CreateFila2(&suspendedBlockedQueue);
	printf("[boot] Queues started\n");

	return;
}

int ccreate (void *(*start) (void*), void *arg, int prio){
  printf("[ccreate] Starting proc\n");
	if(!isBooted) {
      printf("[ccreate] System is not booted\n");
      boot();
	}

  printf("[ccreate] Creating new thread\n");
  TCB_t *newThread = (TCB_t*) malloc(sizeof(TCB_t));
  newThread->prio = 0;
	newThread->tid = currentThread->tid + 1;
	newThread->state = PROCST_APTO;
	getcontext(&(newThread->context));
  newThread->context.uc_link = &endThread->context;
	newThread->context.uc_stack.ss_sp = (char*) malloc(stackSize);
	newThread->context.uc_stack.ss_size = stackSize;
	makecontext(&newThread->context, (void (*) (void))start, 1, arg);
  AppendFila2(&readyQueue,newThread);

  if(!isBooted) {
    isBooted = true;
    dispatch();
  } else {
    cyield();
  }

  printf("[ccreate] Returning thread ID\n");
	return newThread->tid;
}

// Retorno:
// Quando executada corretamente: retorna 0 (zero)
// Caso contrário, retorna um valor negativo.
int cyield(){
  printf("[cyeld] Starting proc\n");
	currentThread->state = PROCST_APTO;
  AppendFila2(&readyQueue, currentThread);
  swapcontext(&currentThread->context, &dispatchThread->context);
	return 0;
}

//  Sincronização de término: uma thread pode ser bloqueada até que outra
//  termine sua execução usando a função cjoin. A função cjoin recebe como
//  parâmetro o identificador da thread cujo término está sendo aguardado.
//  Quando essa thread terminar, a função cjoin retorna com um valor inteiro
//  indicando o sucesso ou não de sua execução. Uma determinada thread só pode
//  ser esperada por uma única outra thread. Se duas ou mais threads fizerem
//  cjoin para uma mesma thread, apenas a primeira que realizou a chamada
//  será bloqueada. As outras chamadas retornarão imediatamente com um código
//  de erro e seguirão sua execução. Se cjoin for feito para uma thread que não
//  existe (não foi criada ou já terminou), a função retornará imediatamente com
//  um código de erro. Observe que não há necessidade de um estado zombie, pois a
//  thread que aguarda o término de outra (a que fez cjoin) não recupera nenhuma
//  informação de retorno proveniente da thread aguardada.

int cjoin(int tid){
  printf("[cjoin] Starting proc\n");
  TCB_t* foundThread;
  TCB_t* foundInReady = (TCB_t*) findInQueueByTid(tid, readyQueue,false);
  TCB_t* foundInBlocked = (TCB_t*) findInQueueByTid(tid, blockedQueue,false);
  if(foundInReady) {
    foundThread = foundInReady;
  } else if(foundInBlocked) {
    foundThread = foundInBlocked;
  }
  if((foundThread != -1) && (!foundThread->isJoined)) {
      printf("[cjoin] Found thread\n");
      foundThread->isJoined = true;
      foundThread->jointid = currentThread->tid;
      AppendFila2(&blockedQueue, currentThread);
      swapcontext(&currentThread->context, &dispatchThread->context);
      return 1;
  } else {
    printf("[cjoin] ERROR -> Thread TID: %d does not exist\n", tid);
    return -1;
  }
}

/*Suspensão e retomada de execução: uma thread pode ser suspensa por outra thread a qualquer momento
através da execução da primitiva csuspend, passando como argumento o identificador da thread a ser suspensa.
A suspensão de uma thread leva-a do estado apto para o estado apto-suspenso ou do estado bloqueado para o estado
bloqueado- suspenso. Uma thread não pode se auto-suspender.
Uma vez suspensa, a thread fica no estado correspondente até que outra thread a retire desse estado usando a primitiva cresume,
passando como argumento o identificador da thread que terá seu estado alterado. Portanto, a execução de cresume faz com que uma
thread passe do estado apto-suspenso para o estado apto ou do estado bloqueado-suspenso para o estado bloqueado.
ATENÇÃO: conforme está indicado no diagrama de estados, se ocorrer um evento esperado (término de outra thread, cjoin ou a
liberação de um recurso com csignal) enquanto uma thread estiver no estado bloqueado-suspenso, a thread deve ser posta no estado
apto-suspenso.
Se for passado como parâmetro para as chamadas csuspend e cresume um identificador inválido de thread (thread inexistente ou o
próprio identificador da thread que as executa), a função deve retornar um código de erro.*/
int csuspend(int tid) {
  printf("[csuspend] Starting proc\n");
  TCB_t* foundThread;
  TCB_t* foundInReady = (TCB_t*) findInQueueByTid(tid, readyQueue, true);
  TCB_t* foundInBlocked = (TCB_t*) findInQueueByTid(tid, blockedQueue, true);
  if(foundInReady) {
    foundThread = foundInReady;
  } else if(foundInBlocked) {
    foundThread = foundInBlocked;
  }
  if(foundThread != -1) {
      printf("[csuspend] Found thread\n");
      if(foundInReady) AppendFila2(&suspendedReadyQueue, foundThread);
      else AppendFila2(&suspendedBlockedQueue, foundThread);
      swapcontext(&currentThread->context, &dispatchThread->context);
      return 0;
  } else {
    printf("[csuspend] ERROR -> Thread TID: %d does not exist in queues\n", tid);
    return -1;
  }
}

int cresume(int tid) {
  printf("[cresume] Starting proc\n");
  TCB_t* foundThread;
  TCB_t* foundInSuspendedReady = (TCB_t*) findInQueueByTid(tid, suspendedReadyQueue, true);
  TCB_t* foundInSuspendedBlocked = (TCB_t*) findInQueueByTid(tid, suspendedBlockedQueue, true);
  if(foundInSuspendedReady) {
    foundThread = foundInSuspendedReady;
  } else if(foundInSuspendedBlocked) {
    foundThread = foundInSuspendedBlocked;
  }
  if(foundThread != -1) {
      printf("[cresume] Found thread\n");
      if(foundInSuspendedReady) AppendFila2(&readyQueue, foundThread);
      else AppendFila2(&blockedQueue, foundThread);
      swapcontext(&currentThread->context, &dispatchThread->context);
      return 0;
  } else {
    printf("[cresume] ERROR -> Thread TID: %d does not exist in queues\n", tid);
    return -1;
  }
}

// o sistema prevê o emprego de uma variável especial para realizar a sincronização de acesso a recursos compartilhados
// (por exemplo, uma seção crítica). As primitivas existentes são csem_init, cwait e csignal,
// e usam uma variável especial que recebe o nome específico de semáforo.
// A primitiva csem_init é usada para inicializar a variável csem_t e deve ser chamada,
// obrigatoriamente, antes que essa variável possa ser usada com as primitivas cwait e csignal.

// a função csem_init inicializa uma variável do tipo csem_t e consiste em fornecer um valor inteiro (count),
// positivo ou negativo, que representa a quantidade existente do recurso controlado pelo semáforo.
// Para realizar exclusão mútua, esse valor inicial da variável semáforo deve ser 1 (semáforo binário).
// Ainda, cada variável semáforo deve ter associado uma estrutura que registre as threads que estão bloqueadas,
// esperando por sua liberação. Na inicialização essa lista deve estar vazia.

int csem_init (csem_t *sem, int count){
  printf("[csem_init] Starting proc\n");
  printf("[csem_init] Initializing sem\n");
  sem->count = count;
  sem->fila  = (FILA2 *)malloc(sizeof(FILA2));

  if(CreateFila2(sem->fila) != 0){
    printf("[csem_init] Failed to create queue\n");
    return -1;
  }
  return 0;
}

// a primitiva cwait será usada para solicitar um recurso.
// Se o recurso estiver livre, ele é atribuído a thread, que continuará a sua execução normalmente;
// caso contrário a thread será bloqueada e posta a espera desse recurso na fila.
// Se na chamada da função o valor de count for menor ou igual a zero,
// a thread deverá ser posta no estado bloqueado e colocada na fila associada a variável semáforo.
// Para cada chamada a cwait a variável count da estrutura semáforo é decrementada de uma unidade.

int cwait (csem_t *sem){
  printf("[cwait] Starting proc\n");
  sem->count--;
  if(sem->count < 0){
    currentThread->state = PROCST_BLOQ;
    if(AppendFila2(sem->fila, (void *) currentThread) != 0){
      printf("[cwait] Failed to Append Queue \n");
      return -1;
    }
    swapcontext(&currentThread->context, &dispatchThread->context);
  }
  return 0;
}
//: a chamada csignal serve para indicar que a thread está liberando o recurso.
// Para cada chamada da primitiva csignal, a variável count deverá ser incrementada de uma unidade.
// Se houver mais de uma thread bloqueada a espera desse recurso a primeira delas, segundo uma política de FIFO,
// deverá passar para o estado apto e as demais devem continuar no estado bloqueado.

int csignal (csem_t *sem){
  printf("[csignal] Starting proc\n");
  sem->count++;
  if(FirstFila2(sem->fila) == 0){
    TCB_t *t_des = (TCB_t *) GetAtIteratorFila2(sem->fila);
    t_des->state = PROCST_APTO;

    AppendFila2 (&readyQueue, t_des); //n sei se eh isso q eh pra fazer

    if(DeleteAtIteratorFila2(sem->fila) != 0){
      printf("[csignal] Failed to delete iterator\n");
      return -1;
    }
  }
  return 0;
}
// Além das funções de manipulação das threads e de sincronização a biblioteca deverá prover a implementação
// de uma função que forneça o nome dos alunos integrantes do grupo
// que desenvolveu a biblioteca chtread. O protótipo dessa função é:

int cidentify (char *name, int size){
  printf("[cidentify] Starting proc\n");
  name = "Gustavo Correa\t00252868\nAndreo Barros\t00252869\nLeonardo Dalcin\t00243654\n";
  if(sizeof(name) < size)
    if(puts(name))
      return 0;
    else
      return -1;
  else
    return -1;
}
