/**
** Teste da função cjoin
**/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"

void* func0() {
	printf("Eu sou a thread ID1 \n");

}

void* func1() {
	printf("Eu sou a thread ID2 \n");

}

void* func2() {
	printf("Eu sou a thread ID3 \n");

}

int main(int argc, char *argv[]) {
	
	int id0, id1, id2;
	int retJoin1, retJoin2;

	id0 = ccreate(func0, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id0);
	
	id1 = ccreate(func1, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id1);

	id2 = ccreate(func2, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id2);
	

	printf("Eu sou a main após a criação de threads\n");

	printf("Eu sou a main aguardando o termino de %d\n", id0);
	retJoin1 = cjoin(id0);
	if (retJoin1 == 0)
		printf("Join executado com sucesso\n");

	printf("Eu sou a main aguardando o termino de %d\n", id1);
	retJoin2 = cjoin(id1);
	if (retJoin2 == 0)
		printf("Join executado com sucesso\n");

	printf("Eu sou a main após todos os cjoins terem sido efetuados\n");
	

	return 0;
}
	

	

	
	
	
	

