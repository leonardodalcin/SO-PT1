/**
** Teste da função ccreate
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"

int id0, id1, id2,id3;
csem_t sem1, sem2;

void* func3() {
  printf("Eu sou a thread ID3 \n");
	csignal(&sem1);
  cjoin(2);
  printf("Eu sou a thread ID3 voltando do wait \n");
}
void* func0() {
	printf("Eu sou a thread ID1 \n");
	cwait(&sem1);
  csignal(&sem1);
  printf("Eu sou a thread ID1 voltando do wait \n");

  printf("Thread 1 Acabando\n");
}

void* func1() {
	printf("Eu sou a thread ID2 \n");
	cwait(&sem1);
  printf("Eu sou a thread ID2 voltando do wait \n");
  cwait(&sem1);
  printf("Thread 2 Acabando\n");

}

void* func2() {
	printf("Eu sou a thread ID3 \n");
}

int main(int argc, char *argv[]) {



	id0 = ccreate(func0, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id0);

	id1 = ccreate(func1, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id1);

  id3 = ccreate(func3, (void *) NULL, 0);
  printf("Eu sou a thread de TID: %d\n", id3);




	printf("Eu sou a main após a criação de threads\n");


	if (csem_init(&sem1, 2) != 0){
		printf("Nao criou semaforo 1\n");
		return -1;
	}

	if (csem_init(&sem2, 1) != 0){
		printf("Nao criou semaforo 2\n");
		return -1;
	}


	if (cjoin(id3) != 0){
		printf("Erro no cjoin\n");
		return -1;
	}

	printf("Sou a main voltando\n");

	return 0;
}
