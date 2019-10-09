
/*
 *	Programa de exemplo de uso da biblioteca cthread
 *
 *	Vers�o 1.0 - 14/04/2016
 *
 *	Sistemas Operacionais I - www.inf.ufrgs.br
 *
 */

#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>

void* func1(void *arg) {
    if (*((int*)arg) + 1 <= 10)  // thread espera a proxima mais jovem
        cjoin(*((int*)arg) + 1); // ate que chegue na ultima == 10

    printf("eu sou a thread imprimindo %d\n", *((int *)arg));
    return NULL;
}

int main(int argc, char *argv[]) {

    int id[10];
    int i, num[10];

    for(i = 0; i < 10; i++){
	    num[i] = i + 1;
    }
///////
//num[0] = 1
//num[1] = 2
//...
//num[9] = 10
///////
    for(i = 0; i < 10; i++){
        id[i] = ccreate(func1, (void*)&num[i], 0);
    }
/////
//    id[0] = // thread que passa o num[0]=1
//    id[1] = // thread num[1] = 2
///// e t c

    cjoin(id[0]); //espera a thread id[0] concluir
    // bloqueia main
    // cede para thread 0 (porque eh FCFS)
    // provavelmente baixa prioridade na fila de bloqueado
    //  pois fez um monte de atribuicao

    printf("Eu sou a main voltando para terminar o programa\n");
    return 0;
}
