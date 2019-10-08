
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

csem_t* semaforo;

void* func0(void *arg) {
	printf("\n-FUNC0 Eu sou a thread ID0 imprimindo %d\n", *((int *)arg));

	printf("-FUNC0: vai tentar semaforo binario com atual count: %d \n", semaforo->count);
	cwait(semaforo);
	printf("-FUNC0 entrou no semaforo! atual count: %d \n", semaforo->count);
	printf("-FUNC0 fazendo coisas de secao critica \n");
	print_queue_ready();
	printf("-FUNC0 vai fazer um yield agora\n");
	cyield();
	printf("\n-FUNC0: primeira instrucao após o cyield. ainda no semaforo, count: %d\n",semaforo->count);
print_queue_ready();
	printf("-FUNCO hora de liberar o semaforo...\n");
	csignal(semaforo);
	print_queue_ready();
	printf("-FUNCO fim.\n\n");
	return NULL;
}

void* func1(void *arg) {
	printf("\n--FUNC1 Eu sou a thread ID1 imprimindo %d\n", *((int *)arg));
print_queue_ready();
	printf("--FUNC1 tentando semaforo count = %d\n",semaforo->count);

	cwait(semaforo);
	printf("--FUNC1 fazendo coisas de semaforo !!\n");
	printf("--FUNC1 abc \n");
	printf("--FUNC1 123 \n");
	csignal(semaforo);

	return NULL;
}

int main(int argc, char *argv[]) {

	int id1, id2,id3,id4,id5,id6,id7;
	int i = 10;

	semaforo = (csem_t*)malloc(sizeof(csem_t));
	if(csem_init(semaforo,1) == 0) {
		printf("Inicializou semaforo binario sucesso\n\n");
	}

	id1 = ccreate(func0, (void *)&i, 0);
	id2 = ccreate(func1, (void *)&i, 0);

	//id3=ccreate(func1, (void *)&i, 5);
	//id4=ccreate(func1, (void *)&i, 5);
	//id5=ccreate(func1, (void *)&i, 2);
	//id6=ccreate(func1, (void *)&i, 10);
	//id7=ccreate(func1, (void *)&i, 0);

//	int x = cpop_ready();
//printf("TID popped: %d\n", x);

//cremove_ready(id6);
//cremove_ready(id5);
//cremove_ready(id6);
//cremove_ready(id4);
//cremove_ready(id2);
//cremove_ready(id1);
//cremove_ready(id7);

//cremove_ready(ccreate(func1, (void *)&i, 200));

//cremove_ready(id3);
//empty now
//cremove_ready(id2);
//cremove_ready(id1);
//cremove_ready(id7);


	printf("\nEu sou a main ap�s a cria��o de ID1 e ID2, ocupando o recurso binario\n");
	if(cwait(semaforo) == 0) {
		printf("+MAIN ocupou o semaforo binario\n");
		printf("+MAIN Contagem semaforo: %d \n", semaforo->count);
		printf("+MAIN vai fazer CSIGNAL\n\n");
		print_queue(semaforo->fila);
		int retorno = csignal(semaforo);
		printf("+MAIN Codigo retornado do csignal: %d. \n+MAIN Count semaforo atual: %d\n",retorno,
	semaforo->count);
		if (retorno == 0) {
			printf("+MAIN fez o CSignal ok, liberou o semaforo \n");
		}
		printf("+MAIN vai fazer CYIELD\n\n");
		retorno = cyield();
		printf("+MAIN retornou esse valor do cyield dela: %d\n", retorno);
		if ( retorno == 0 )
			{
				printf("Main fez o CYIELD ok\n");
			}
	}

	cjoin(id2);

	//cjoin(id1);
	//cjoin(id2);

	printf("Eu sou a main voltando para terminar o programa\n");




	return 0;
}
