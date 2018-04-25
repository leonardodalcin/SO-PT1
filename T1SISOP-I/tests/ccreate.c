/**
** Teste da função ccreate
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"





void* func1() {
  printf("Eu sou a thread ID1 antes do yield \n");
	cjoin(1);

  printf("Eu sou a thread ID1 depois do yield \n");

}

void* func0() {
	printf("Eu sou a thread ID0 antes do yield \n");
	int id0 = ccreate(func1, (void *) NULL, 0);

  printf("Eu sou a thread ID0 depois do yield \n");
}


int main(int argc, char *argv[]) {

	int id0, id1, id2;

	id0 = ccreate(func0, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id0);


	printf("Eu sou a main após a criação de threads\n");

	return 0;
}
