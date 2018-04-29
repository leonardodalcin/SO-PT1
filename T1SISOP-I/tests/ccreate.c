/**
** Teste da função ccreate
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"





void* func1() {
  printf("2 \n");
	cjoin(3);
  printf("4 \n");
}

void* func0() {
	printf("1 \n");
	int id0 = ccreate(func1, (void *) NULL, 0);
  printf("3 \n");
}


int main(int argc, char *argv[]) {

	int id0, id1, id2;
	char *nomes;

	id1 = cidentify(nomes, 250);
	id0 = ccreate(func0, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id0);


	printf("Eu sou a main após a criação de threads\n");

	return 0;
}
