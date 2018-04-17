#include <stdlib.h>
#include <unistd.h>
#include <ucontext.h>

#include "support.h"
#include "cthread.h"
#include "cdata.h"
#define stackSize SIGSTKSZ

#define stateApto 0
#define stateExecutando 1
#define stateBloqueado 2
#define stateBloqueadoSuspenso 3
#define stateAptoSuspenso 4
#define stateTermino 5

typedef enum { false, true } bool;
bool isBooted = false;
FILA2 readyQueue;
FILA2 blockedQueue;
TCB_t *currentThread;
TCB_t *mainThread;
TCB_t *endThread;




//inicia proxima thread da fila de aptos

void dispatch(){
    printf("[dispatch] Starting dispatch proc\n");
    if(FirstFila2(&aptos) == 0) {
        printf("[dispatch] Last thread tid: %d\n", currentThread->tid);
        currentThread = GetAtIteratorFila2(&aptos);
        printf("[dispatch] Current thread tid: %d\n", currentThread->tid);
        setcontext(&currentThread->context);
        DeleteAtIteratorFila2(&aptos);
    } else {
        printf("[dispatch] The ready queue is empty\n")
    }
    return;
}

void endThreadProc(){

	printf("[Finishing thread] TID = %d\n", currentThread->tid);

	lookForTidinBlockedQueue(); //procura na fila de bloqueados alguma thread que esteja esperando pela que acabou

	free(currentThread);
	currentThread = NULL;

	dispatch();

}

void boot(){
    printf("[boot] Starting proc\n");

    printf("[boot] Creating main thread\n");
	mainThread = (TCB_t*) malloc(sizeof(TCB_t));
	mainThread->tid = 0;
	mainThread->prio = 0;
	mainThread->state = 0;
	getcontext(&mainThread->context);
	currentThread = mainThread;
    printf("[boot] Created main thread\n");

    printf("[boot] Creating end thread\n");
    getcontext(&endThread);
	endThread.uc_link = 0;
	endThread.uc_stack.ss_sp = (char*) malloc(stackSize);
	endThread.uc_stack.ss_size = stackSize;
	makecontext(&endThread, (void(*)(void))endThreadProc, 0);
	printf("[boot] Created end thread\n");

	printf("[boot] Starting ready and blocked queues\n");
	CreateFila2(readyQueue);
	CreateFila2(blockedQueue);
	printf("[boot] Queues started\n");

	isBooted = true;
	return
}



int ccreate (void *(*start) (void*), void *arg, int prio){
    printf("[ccreate] Starting proc\n");
	if(!isBooted) {
        printf("[ccreate] System is not booted\n");
        printf("[ccreate] Starting boot proc\n");
        boot();
	}
	printf("[ccreate] Creating new thread\n");
    prio = 0;
	TCB_t *newThread = (TCB_t*) malloc(sizeof(TCB_t));

	newThread->prio = prio;
	newThread->tid = currentThread->tid++;;
	newThread->state = stateApto;
	getcontext(&(newThread->context));

	/*
	ucontext_t *uc_link     pointer to the context that will be resumed
                            this context returns
    sigset_t    uc_sigmask  the set of signals that are blocked when this
                            context is active
    stack_t     uc_stack    the stack used by this context
    mcontext_t  uc_mcontext a machine-specific representation of the saved
                            context

	stack_t {
        void     *ss_sp       stack base or pointer
        size_t    ss_size     stack size
        int       ss_flags    flags
	}
	*/
	newThread->context.uc_link = &endThread;
	newThread->context.uc_stack.ss_sp = (char*) malloc(stackSize);
	newThread->context.uc_stack.ss_size = stackSize;
	makecontext(&(newThread->context), (void (*) (void))start, arg, prio);
    AppendFila2(readyQueue, newThread);
	return newThread->tid;
}

