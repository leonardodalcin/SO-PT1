/**
** Teste da função cidentify
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"

#define TAM_MAX 250


int main(int argc, char *argv[]) {
	
	char *nomes;			
	int size = TAM_MAX;

	int id0;

	id0 = cidentify(nomes, size);
	if(id0 == 0)
		return 0;
	else
		return -1;
}
