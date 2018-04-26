#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ucontext.h>

#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#define stackSize SIGSTKSZ


bool isBooted = false;
bool cyieldShouldBlock = false;
FILA2 readyQueue;
FILA2 blockedQueue;
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
    			return NULL;
    }
	}
  return NULL;
}

void dispatchThreadProc(){
	printf("[Dispatch Thread Proc] TID = %d\n", currentThread->tid);
  currentThread = NULL;
  dispatch();
  return;
}

void unjoin(int tid){
	currentThread->isJoined = false;
  currentThread->jointid = NULL;
  TCB_t* unjoinedThread = findInQueueByTid(tid, blockedQueue, 1);
  if(!unjoinedThread) return;
  else AppendFila2(&readyQueue, unjoinedThread);
  return;
}

void endThreadProc(){
	printf("[Finishing thread] TID = %d\n", currentThread->tid);
  if(currentThread->isJoined) unjoin(currentThread->tid);
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
  if(cyieldShouldBlock) {
    cyieldShouldBlock = false;
    AppendFila2(&blockedQueue, currentThread);
  } else AppendFila2(&readyQueue, currentThread);
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
  TCB_t* foundThread = (TCB_t*) findInQueueByTid(tid, readyQueue,false) || findInQueueByTid(tid, blockedQueue,false);
  if(foundThread && !foundThread->isJoined) {
      printf("[cjoin] Found thread\n");
      foundThread->isJoined = true;
      foundThread->jointid = currentThread->tid;
      cyieldShouldBlock = true;
      cyield();
      return 1;
  } else {
    return -1;
  }
}
