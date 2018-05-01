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

}
void* func0() {

	printf("Eu sou a thred 1 antes do cjoin \n");
	cjoin(id2);
	printf("Eu sou a thread ID1 \n");


  printf("Thread 1 Acabando\n");
}

void* func1() {

	printf("Eu sou a thread ID2 \n");

  printf("Thread 2 Acabando\n");

}

void* func2() {
	printf("Eu sou a thread ID3 \n");
}

int main(int argc, char *argv[]) {



	id1 = ccreate(func0, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id1);

	id2 = ccreate(func1, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id2);

  id3 = ccreate(func3, (void *) NULL, 0);
  printf("Eu sou a thread de TID: %d\n", id3);




	printf("Eu sou a main após a criação de threads\n");





	if (cjoin(id3) != 0){
		printf("Erro no cjoin\n");
		return -1;
	}

	printf("Sou a main voltando\n");

	return 0;
}
