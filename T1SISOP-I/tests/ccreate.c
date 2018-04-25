/**
** Teste da função ccreate
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"



void* func1() {
	printf(">>>>>>>>2 \n");
	cyield();

printf(">>>>>>>>5\n");
}

void* func0() {
	printf(">>>>>>>>1\n");

	int id3 = ccreate(func1, (void *) NULL, 0);
	printf(">>>>>>>>3\n");
	cyield();
	printf(">>>>>>>>5\n");
}

void* func2() {

	printf("Eu sou a thread ID2 \n");
}

int main(int argc, char *argv[]) {

	int id0, id1, id2;

	id0 = ccreate(func0, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id0);


	printf("Eu sou a main após a criação de threads\n");

	return 0;
}
