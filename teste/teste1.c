
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

void* func1(void *arg) {
    if (*((int*)arg) + 1 <= 10)
        cjoin(*((int*)arg) + 1);

    printf("eu sou a thread imprimindo %d\n", *((int *)arg));
    return NULL;
}

int main(int argc, char *argv[]) {

    int id[10];
    int i, num[10];

    for(i = 0; i < 10; i++){
	    num[i] = i + 1;
    }

    for(i = 0; i < 10; i++){
        id[i] = ccreate(func1, (void*)&num[i], 0);
    }

    cjoin(id[0]);

    printf("Eu sou a main voltando para terminar o programa\n");
    return 0;
}

