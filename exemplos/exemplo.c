
/*
 *	Programa de exemplo de uso da biblioteca cthread
 *
 *	Versão 1.0 - 14/04/2016
 *
 *	Sistemas Operacionais I - www.inf.ufrgs.br
 *
 */

#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>
#include <stdlib.h>

void* func0(void *arg) {
	printf("Eu sou a thread ID0 imprimindo %d\n", *((int *)arg));
	return NULL;
}

void* func1(void *arg) {
	printf("Eu sou a thread ID1 imprimindo %d\n", *((int *)arg));
	return NULL;
}

int main(int argc, char *argv[]) {

	char *name = (char*)malloc(300*sizeof(char*));
	cidentify(name, 300);
	printf("%s", name);

	int id1, id2,id3,id4,id5,id6,id7;
	int i = 10;

	id1 = ccreate(func0, (void *)&i, 0);
	id2 = ccreate(func1, (void *)&i, 0);

	id3=ccreate(func1, (void *)&i, 5);
	id4=ccreate(func1, (void *)&i, 5);
	id5=ccreate(func1, (void *)&i, 2);
	id6=ccreate(func1, (void *)&i, 10);
	id7=ccreate(func1, (void *)&i, 0);

	int x = cpop_ready();
printf("TID popped: %d\n", x);

cremove_ready(id6);
cremove_ready(id5);
cremove_ready(id6); 
cremove_ready(id4);
cremove_ready(id2);
cremove_ready(id1);
cremove_ready(id7);

cremove_ready(ccreate(func1, (void *)&i, 200));

cremove_ready(id3);
//empty now
cremove_ready(id2);
cremove_ready(id1);
cremove_ready(id7);


	printf("Eu sou a main após a criação de ID1 e ID2\n");

	//cjoin(id1);
	//cjoin(id2);

	printf("Eu sou a main voltando para terminar o programa\n");

	return 0;
}

