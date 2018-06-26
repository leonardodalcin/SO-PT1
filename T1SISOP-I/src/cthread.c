#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ucontext.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

#define stackSize SIGSTKSZ

int threadIdCount = 0;
bool isBooted = false;
FILA2 readyQueue;
FILA2 blockedQueue;
FILA2 suspendedReadyQueue;
FILA2 suspendedBlockedQueue;
TCB_t *currentThread;
TCB_t *mainThread;
TCB_t *dispatchThread;
TCB_t *endThread;


void dispatch() {

  if (FirstFila2(&readyQueue) == 0) {
    currentThread = (TCB_t *) GetAtIteratorFila2(&readyQueue);
    DeleteAtIteratorFila2(&readyQueue);
    currentThread->state = PROCST_EXEC;
    if (currentThread)
      setcontext(&currentThread->context);
  } else {

    return;
  }
}

TCB_t *findInBlockedBySemaphore(csem_t *semaphore, bool shouldRemove) {

  if (FirstFila2(&blockedQueue) == 0) {
    while (GetAtIteratorFila2(&blockedQueue) != NULL) {
      TCB_t *threadInQueue = (TCB_t *) GetAtIteratorFila2(&blockedQueue);
      if (threadInQueue->isInSemaphore) {


        if (threadInQueue->semaphore == semaphore) {


          if (shouldRemove == true) {


            DeleteAtIteratorFila2(&blockedQueue);
          }

          return threadInQueue;
        }
      }
      if (NextFila2(&blockedQueue) != 0)
        return NULL;
    }
  }
  return NULL;
}

TCB_t *findInQueueByTid(int tid, FILA2 queue, bool shouldRemove) {

  if (FirstFila2(&queue) == 0) {
    while (GetAtIteratorFila2(&queue) != NULL) {
      TCB_t *threadInQueue = (TCB_t *) GetAtIteratorFila2(&queue);
      if (threadInQueue->tid == tid) {
        if (shouldRemove == true) {
          DeleteAtIteratorFila2(&queue);
        }
        return threadInQueue;
      } else if (NextFila2(&queue) != 0) {
        return NULL;
      }
    }
  }
  return NULL;
}

void dispatchThreadProc() {

  currentThread = NULL;
  dispatch();
  return;
}

void unblock(int tid) {
  currentThread->isJoined = false;
  currentThread->jointid = 0;
  TCB_t *unblockedThread = findInQueueByTid(tid, blockedQueue, false);
  // Seg fault ?
  if (unblockedThread != NULL) {
    if (FirstFila2(&blockedQueue) == 0) {
      while (GetAtIteratorFila2(&blockedQueue) != NULL) {
        TCB_t *threadInQueue = (TCB_t *) GetAtIteratorFila2(&blockedQueue);
        if (threadInQueue->tid == unblockedThread->tid) {
          DeleteAtIteratorFila2(&blockedQueue);
          break;
        } else if (NextFila2(&blockedQueue) != 0) {
          break;
        }
      }
    }
  };
  if (unblockedThread != NULL && unblockedThread->isInSemaphore == true) {

    return;
  }

  if (unblockedThread == NULL) return;
  else AppendFila2(&readyQueue, unblockedThread);
  return;
}

void endThreadProc() {

  if (currentThread->isJoined == true) unblock(currentThread->jointid);
  free(currentThread);
  currentThread = NULL;
  dispatch();
  return;
}

void boot() {


  mainThread = (TCB_t *) malloc(sizeof(TCB_t));
  mainThread->tid = threadIdCount;
  mainThread->prio = 0;
  mainThread->isInSemaphore = false;
  mainThread->semaphore = NULL;
  mainThread->isJoined = false;
  mainThread->jointid = 0;
  mainThread->state = PROCST_EXEC;
  getcontext(&mainThread->context);
  mainThread->context.uc_link = (ucontext_t *) &endThreadProc;
  mainThread->context.uc_stack.ss_sp = (char *) malloc(stackSize);
  mainThread->context.uc_stack.ss_size = stackSize;
  makecontext(&mainThread->context, (void (*)(void)) endThreadProc, 0);
  currentThread = mainThread;


  dispatchThread = (TCB_t *) malloc(sizeof(TCB_t));
  getcontext(&dispatchThread->context);
  dispatchThread->context.uc_link = 0;
  dispatchThread->context.uc_stack.ss_sp = (char *) malloc(stackSize);
  dispatchThread->context.uc_stack.ss_size = stackSize;
  makecontext(&dispatchThread->context, (void (*)(void)) dispatchThreadProc, 0);


  endThread = (TCB_t *) malloc(sizeof(TCB_t));
  getcontext(&endThread->context);
  endThread->context.uc_link = 0;
  endThread->context.uc_stack.ss_sp = (char *) malloc(stackSize);
  endThread->context.uc_stack.ss_size = stackSize;
  makecontext(&endThread->context, (void (*)(void)) endThreadProc, 0);


  CreateFila2(&readyQueue);
  CreateFila2(&blockedQueue);
  CreateFila2(&suspendedReadyQueue);
  CreateFila2(&suspendedBlockedQueue);
  isBooted = true;


  return;
}

int ccreate(void *(*start)(void *), void *arg, int prio) {

  if (isBooted == false) {
    boot();
  }

  threadIdCount++;

  TCB_t *newThread = (TCB_t *) malloc(sizeof(TCB_t));
  newThread->prio = 0;
  newThread->tid = threadIdCount;
  newThread->state = PROCST_APTO;
  newThread->isInSemaphore = false;
  newThread->semaphore = NULL;
  getcontext(&(newThread->context));
  newThread->context.uc_link = &endThread->context;
  newThread->context.uc_stack.ss_sp = (char *) malloc(stackSize);
  newThread->context.uc_stack.ss_size = stackSize;
  makecontext((&newThread->context), (void (*)(void)) start, 1, arg);
  AppendFila2(&readyQueue, newThread);


  return newThread->tid;
}

// Retorno:
// Quando executada corretamente: retorna 0 (zero)
// Caso contrário, retorna um valor negativo.
int cyield() {

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

int cjoin(int tid) {

  TCB_t *foundThread;
  TCB_t *foundInReady;
  TCB_t *foundInBlocked;
  foundInReady = findInQueueByTid(tid, readyQueue, false);
  foundInBlocked = findInQueueByTid(tid, blockedQueue, false);
  if (foundInReady == NULL && foundInBlocked == NULL) {

    return -1;
  } else if (foundInReady != NULL) {
    foundThread = foundInReady;
  } else {
    foundThread = foundInBlocked;
  }
  if (foundThread->isJoined == false) {

    foundThread->isJoined = true;
    foundThread->jointid = currentThread->tid;
    AppendFila2(&blockedQueue, currentThread);
    swapcontext(&currentThread->context, &dispatchThread->context);
    return 0;
  } else {

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

  TCB_t *foundThread;
  TCB_t *foundInReady = (TCB_t *) findInQueueByTid(tid, readyQueue, true);
  TCB_t *foundInBlocked = (TCB_t *) findInQueueByTid(tid, blockedQueue, true);
  if (foundInReady == NULL && foundInBlocked == NULL) {

    return -1;
  } else if (foundInReady != NULL) {
    foundThread = foundInReady;
  } else {
    foundThread = foundInBlocked;
  }
  if (foundThread != NULL) {

    if (foundInReady != NULL) AppendFila2(&suspendedReadyQueue, foundThread);
    else AppendFila2(&suspendedBlockedQueue, foundThread);
    swapcontext(&currentThread->context, &dispatchThread->context);
    return 0;
  } else {

    return -1;
  }
}

int cresume(int tid) {

  TCB_t *foundThread;
  TCB_t *foundInSuspendedReady = (TCB_t *) findInQueueByTid(tid, suspendedReadyQueue, true);
  TCB_t *foundInSuspendedBlocked = (TCB_t *) findInQueueByTid(tid, suspendedBlockedQueue, true);
  if (foundInSuspendedReady == NULL && foundInSuspendedBlocked == NULL) {

    return -1;
  } else if (foundInSuspendedReady != NULL) {
    foundThread = foundInSuspendedReady;
  } else {
    foundThread = foundInSuspendedBlocked;
  }
  if (foundThread != NULL) {

    if (foundInSuspendedReady != NULL) AppendFila2(&readyQueue, foundThread);
    else AppendFila2(&blockedQueue, foundThread);
    swapcontext(&currentThread->context, &dispatchThread->context);
    return 0;
  } else {
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

int csem_init(csem_t *sem, int count) {


  sem->count = count;
  sem->fila = &blockedQueue;
  return 0;
}

// a primitiva cwait será usada para solicitar um recurso.
// Se o recurso estiver livre, ele é atribuído a thread, que continuará a sua execução normalmente;
// caso contrário a thread será bloqueada e posta a espera desse recurso na fila.
// Se na chamada da função o valor de count for menor ou igual a zero,
// a thread deverá ser posta no estado bloqueado e colocada na fila associada a variável semáforo.
// Para cada chamada a cwait a variável count da estrutura semáforo é decrementada de uma unidade.

int cwait(csem_t *sem) {

  if (sem->count <= 0) {
    currentThread->state = PROCST_BLOQ;
    currentThread->isInSemaphore = true;
    currentThread->semaphore = sem;
    if (AppendFila2(&blockedQueue, (void *) currentThread) != 0) {

      return -1;
    }
    swapcontext(&currentThread->context, &dispatchThread->context);
  }
  sem->count--;
  return 0;
}
//: a chamada csignal serve para indicar que a thread está liberando o recurso.
// Para cada chamada da primitiva csignal, a variável count deverá ser incrementada de uma unidade.
// Se houver mais de uma thread bloqueada a espera desse recurso a primeira delas, segundo uma política de FIFO,
// deverá passar para o estado apto e as demais devem continuar no estado bloqueado.

int csignal(csem_t *sem) {

  sem->count++;
  if (FirstFila2(&blockedQueue) == 0) {
    TCB_t *t_des = (TCB_t *) findInBlockedBySemaphore(sem, true);
    if (t_des == NULL) {

      return -1;
    }
    t_des->state = PROCST_APTO;
    t_des->isInSemaphore = false;
    t_des->semaphore = NULL;
    AppendFila2(&readyQueue, t_des); //n sei se eh isso q eh pra fazer
  }
  return 0;
}
// Além das funções de manipulação das threads e de sincronização a biblioteca deverá prover a implementação
// de uma função que forneça o nome dos alunos integrantes do grupo
// que desenvolveu a biblioteca chtread. O protótipo dessa função é:

int cidentify(char *name, int size) {

  name = "Gustavo Correa\t00252868\nAndreo Barros\t00252869\nLeonardo Dalcin\t00243654\n";
  if (sizeof(name) < size)
    if (puts(name))
      return 0;
    else
      return -1;
  else
    return -1;
}
