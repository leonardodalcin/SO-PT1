#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ucontext.h>

#include "../include/support.h"
#include "../include/cdata.h"
#include "../include/cthread.h"
#include "../include/insert.h"

#define stackSize SIGSTKSZ
#define defaultTID -1


//variaveis universais

int firstExec = 1;
int nroTID = 1;
int retThread = 0;
int startExec = 0;
int stopExec = 0;

FILA2 aptos;
FILA2 bloqueados;
FILA2 cjoinQueue;

TCB_t *exec;
TCB_t *threadMain;

ucontext_t endThread, dispatch_ctx;


//funcao que conta o tempo de execucao de cada thread e seta sua prioridade
//partindo do pressuposto de que exec sempre aponta para thread que possui a CPU
//retorna 1 caso SUCESSO

int setPrio(){

	if (exec->startTimer == 1){
		startTimer();
		exec->startTimer = 0;
		return 1;
	}

	else{
		exec->prio = stopTimer();
		printf("Thread de id: %d executou por %d ciclos de clock\n", exec->tid, exec->prio);
		return 1;
	}
}

//seta iterador para a primeira posicao da fila de aptos para indicar a thread a receber a CPU

TCB_t *setNewExec(){

	if (FirstFila2(&aptos) == 0){

		TCB_t *unlockedThread = NULL;
		if(GetAtIteratorFila2(&aptos) != NULL){
			unlockedThread = (TCB_t *) GetAtIteratorFila2(&aptos);
			exec = unlockedThread;
		}
		else
			exec = NULL;

	}

	else
		return NULL;

	return exec;
}

//inicia proxima thread da fila de aptos

void dispatch(){

	exec = setNewExec();
	if(exec != NULL){

		//COMECA A CONTAR O TEMPO DA NOVA THREAD QUE VAI EXECUTAR
		exec->startTimer = 1;
		if(setPrio() != 1)
			printf("Nao conseguiu setar a prioridade em dispatch!\n");

		if(FirstFila2(&aptos) == 0)
			if(DeleteAtIteratorFila2(&aptos) == 0)
				setcontext(&exec->context);
	}
	else
		printf("Nao tem mais threads para serem executadas\n");
}


//funcao que checa se nao ha mais de uma thread aguardando pelo mesmo tid
//caso positivo retorna 0 (Erro); caso negativo, 1 (Sucesso)

int checkJoin(int tid){

	if(FirstFila2(&bloqueados) == 0){
		TCB_t *threadInQueue = NULL;
		while(GetAtIteratorFila2(&bloqueados) != NULL){

			threadInQueue = (TCB_t *) GetAtIteratorFila2(&bloqueados);
			if(threadInQueue->tidCjoin == tid)
				return 0;

			else{
				if(NextFila2(&bloqueados) != 0)
					return 0;
			}
		}

		return 1;
	}

	else
		return 0;
}

void changeState(PFILA2 queue, TCB_t *threadGoingToQueue){

	//insere na fila indicada por queue

	if (exec != NULL)

		if(InsertByPrio(queue, threadGoingToQueue) != 0)
			printf("Nao conseguiu inserir em fila na changeState()\n");



}

//funcao que verifica se o tid da thread cujo termino esta sendo aguardado é válido
//veriricando na fila de aptos e bloqueados retorna 0 caso deu erro e 1 caso sucesso

int verifyCjoin(int tid, FILA2 queue){

	TCB_t *threadInQueue = NULL;

	while(GetAtIteratorFila2(&queue) != NULL){

		threadInQueue = (TCB_t *) GetAtIteratorFila2(&queue);
		if(threadInQueue->tid == tid){
			exec->tidCjoin = tid;	//indica o tid pela qual a thread que esta executando devera esperar
			exec->state = PROCST_BLOQ;

			//**THREAD QUE EXECUTAVA VAI PARA BLOQUEADO
			//**CHAMA FUNCAO QUE SETA PRIORIDADE
			if(setPrio() != 1)
				printf("Nao conseguiu setar a prioridade em verifyCjoin!\n");

			return 1;
		}

		else
			if(NextFila2(&queue) != 0)
				return 0;
	}

	return 0;

}


void lookForTidinBlockedQueue(){

	if (FirstFila2(&bloqueados) == 0){

		TCB_t *threadInQueue = NULL;

		while(GetAtIteratorFila2(&bloqueados) != NULL){
			threadInQueue = (TCB_t *) GetAtIteratorFila2(&bloqueados);
			if (threadInQueue->tidCjoin == exec->tid){

				threadInQueue->state = PROCST_APTO;
				threadInQueue->tidCjoin = defaultTID;
				changeState(&aptos, threadInQueue);			//tira thread de bloqueado e coloca em aptos

				DeleteAtIteratorFila2(&bloqueados);			//exclui thread da fila de bloqueados

				return;

			}
			if(NextFila2(&bloqueados) != 0)
				return;

		}
	}

}


void fimThread(){

	//TERMINA DE CONTAR O TEMPO QUE UMA THREAD EXECUTOU E SETA SUA PRIORIDADE
	if(setPrio() != 1)
		printf("Nao conseguiu setar a prioridade em fimThread!\n");

	printf("thread encerrando = %d\n", exec->tid);

	lookForTidinBlockedQueue(); //procura na fila de bloqueados alguma thread que esteja esperando pela que acabou

	//desaloca estrutura da thread para liberar memoria
	free(exec);
	exec = NULL;

	dispatch();

}

void initMain(){

	threadMain = (TCB_t*) malloc(sizeof(TCB_t));

	threadMain->tid = 0;
	threadMain->prio = 0;
	threadMain->state = PROCST_EXEC;
	threadMain->tidCjoin = defaultTID;
	threadMain->startTimer = 1;

	getcontext(&threadMain->context);
	exec = threadMain;

	//COMECA A CONTAR O TEMPO DE EXECUCAO DA MAIN PARA SETAR SUA PRIORIDADE
	if(setPrio() != 1)
		printf("Nao conseguiu setar a prioridade em initMain!\n");

}

void initDispatch(){

	getcontext(&dispatch_ctx);

	dispatch_ctx.uc_link = 0;
	dispatch_ctx.uc_stack.ss_sp = (char*) malloc(stackSize);
	dispatch_ctx.uc_stack.ss_size = stackSize;

	makecontext(&dispatch_ctx, (void(*)(void))dispatch, 0);
}


void initFimThread(){

	getcontext(&endThread);

	endThread.uc_link = 0;
	endThread.uc_stack.ss_sp = (char*) malloc(stackSize);
	endThread.uc_stack.ss_size = stackSize;

	makecontext(&endThread, (void(*)(void))fimThread, 0);
}


int ccreate (void *(*start) (void*), void *arg, int zero){

	//primeira execucao cria contexto da main, do dispatcher;
	//inicializa filas e funcao que testa fim de thread

	if(firstExec){

		initMain();
		initDispatch();
		initFimThread();


		//criando fila de aptos e bloqueados

		if(CreateFila2(&aptos) != 0){
			printf("Nao criou a fila de aptos\n");
			return -1;
		}

		if(CreateFila2(&bloqueados) != 0){
			printf("Nao criou a fila de bloqueados\n");
			return -1;
		}

		firstExec = 0;	//variavel que indica que nao eh mais a primeira execucao
	}


	//criando nova thread

	TCB_t *novaThread;
	novaThread = (TCB_t*) malloc(sizeof(TCB_t));

	novaThread->prio = zero;
	novaThread->tid = nroTID;
	novaThread->state = PROCST_APTO;
	novaThread->tidCjoin = defaultTID;
	novaThread->startTimer = zero;

	getcontext(&(novaThread->context));

	novaThread->context.uc_link = &endThread;
	novaThread->context.uc_stack.ss_sp = (char*) malloc(stackSize);
	novaThread->context.uc_stack.ss_size = stackSize;

	makecontext(&(novaThread->context), (void (*) (void))start, 1, arg);


	//Inserindo na Fila de Aptos;

	if (InsertByPrio(&aptos, novaThread) == 0)
		printf("Nova Thread inserida na fila de Aptos\n");
	else{
		printf("Erro na insercao na fila de Aptos\n");
		return -1;
	}

	nroTID++;

	return novaThread->tid;

}

int cjoin(int tid){

	int flagVerify;

	//verifica fila de aptos e bloqueados para ver se o tid eh valido

	if ((FirstFila2(&aptos) == 0) || (FirstFila2(&bloqueados) == 0)){		//fila de bloqueados pode estar vazia, retornando
											//algo diferente de 0

		flagVerify =  ( (verifyCjoin(tid, aptos)) || (verifyCjoin(tid, bloqueados)) );

		if (flagVerify == 0){				//se as duas hipoteses do condicional "ou" forem falsas, o retorno de
			flagVerify = -1;			//cjoin deve ser -1
			return flagVerify;
		}

		flagVerify = flagVerify && checkJoin(tid);

		if(flagVerify == 0){

			changeState(&bloqueados, exec);
			swapcontext(&exec->context, &dispatch_ctx);

			return flagVerify;
		}
		else
			return -1;
	}

	else
		return -1;
}

//altera estado da thread que ocupa atualmente a CPU (exec) para APTO;
//insere a thread na fila de aptos
//chama o dispatcher para executar a próxima thread da fila de aptos, salvando o contexto da main(swapcontext)
//variavel que indica o retorno para a thread main após o cyield (0 -> SUCESSO; -1 -> ERRO)

int cyield(){

	exec->state = PROCST_APTO;

	//seta a prioridade antes de entrar na fila de aptos
	if(setPrio() != 1)
		printf("Nao conseguiu setar a prioridade em dispatch!\n");

	changeState(&aptos, exec);
	swapcontext(&exec->context, &dispatch_ctx);

	retThread = 1;

	if(retThread){
		retThread = 0;
		return 0;
	}
	else{
		retThread = 0;
		return -1;
	}

}

int csem_init(csem_t *sem, int count){

	sem->count = count;
	sem->fila  = (FILA2 *)malloc(sizeof(FILA2));

	if(CreateFila2(sem->fila) != 0){
		printf("Nao criou fila em csem_init!\n");
		return -1;
	}

	return 0;
}

int cwait(csem_t *sem){

	sem->count--;

	if(sem->count < 0){

		if(setPrio() != 1)
			printf("Nao conseguiu setar a prioridade em cwait!\n");

		exec->state = PROCST_BLOQ;

		if(AppendFila2(sem->fila, (void *) exec) != 0){
			printf("Nao colocou no fim de sem->fila em cwait\n");
			return -1;
		}
		swapcontext(&exec->context, &dispatch_ctx);
	}

	return 0;

}


int csignal(csem_t *sem){

	sem->count++;

	if(FirstFila2(sem->fila) == 0){

		TCB_t *t_des = (TCB_t *) GetAtIteratorFila2(sem->fila);
		t_des->state = PROCST_APTO;

		changeState(&aptos, t_des);

		if(DeleteAtIteratorFila2(sem->fila) != 0){
			printf("Nao deletou da fila do semaforo em csignal\n");
			return -1;
		}

	}

	return 0;
}

//Funcao de identificacao
//retorna 0 se SUCESSO, -1 se ERRO
/*OBS*** DEIXAR ESSA FUNCAO COMO ULTIMA DO ARQUIVO cthread.c!!!
*/

int cidentify(char *name, int size){

	name = "Geronimo Veit Acosta\t00260004\nJulia Del Pino Rittmann\t00262512\nVilmar Antonio Fonseca\t00262519\n";

	if(sizeof(name) < size)

		if(puts(name))
			return 0;
		else
			return -1;
	else
		return -1;

}
