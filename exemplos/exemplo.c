
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
#include <stdlib.h>

#define SUCCESS 0

csem_t* semaforo;

void* f1(void *arg) {
	print("=>F1 executando");
	print_queue_ready();print("");
	//cyield();
	return NULL;
}

void* f2(void *arg) {
	print("=>F2 executando");
	print_queue_ready();print("");
	//cyield();
	return NULL;
}

int main(int argc, char *argv[]) {

	char *name = (char*)malloc(300*sizeof(char*));
	cidentify(name, 300);
	printf("%s\n", name);

	int id1, id2;
	int i = 10;

	print("=>MAIN comecando");
	print_queue_ready();print("");

	id1 = ccreate(f1, (void *)&i, 0);
	print("=>MAIN criou thread para F1");
	id2 = ccreate(f2, (void *)&i, 0);
	print("=>MAIN criou thread para F2");

	print_queue_ready();print("");

	print("=>MAIN vai esperar as threads");
	printf("%d\n", cjoin(id1));
	print("=>MAIN deu join na F1");
	cjoin(id2);
	print("=>MAIN deu join na F2");


	print("=>MAIN vai tentar cwait sem init");
	if(cwait(semaforo)==SUCCESS)
		print("Deu certo");
	else print("Deu failed");

	print("=>MAIN vai tentar csignal sem init");
	if(csignal(semaforo)==SUCCESS)
		print("Deu certo");
	else print("Deu failed");

	semaforo = (csem_t*)malloc(sizeof(csem_t));
	if(csem_init(semaforo, 1)==SUCCESS)
		print("INICIALIZOU!");
	else print("NAO CONSEGUIU INICIALIZAR");

	print("=>MAIN vai tentar cwait APOS INIT");

	if(cwait(semaforo)==SUCCESS)
		print("Deu certo");
	else print("Deu failed");

	print("=>MAIN vai tentar csignal APOS INIT");

	if(csignal(semaforo)==SUCCESS)
		print("Deu certo");
	else print("Deu failed");


	print("=>MAIN terminando");
	print("");
	return 0;
}



/*
void* func0(void *arg) {
	print("\n-FUNC0 Eu sou a thread ID0 imprimindo %d\n", *((int *)arg));

	print("-FUNC0: vai tentar semaforo binario com atual count: %d \n", semaforo->count);
	cwait(semaforo);
	print("-FUNC0 entrou no semaforo! atual count: %d \n", semaforo->count);
	print("-FUNC0 fazendo coisas de secao critica ");
	print_queue_ready();
	print("-FUNC0 vai fazer um yield agora");
	cyield();
	print("\n-FUNC0: primeira instrucao após o cyield. ainda no semaforo, count: %d\n",semaforo->count);
print_queue_ready();
	print("-FUNCO hora de liberar o semaforo...");
	csignal(semaforo);
	print_queue_ready();
	print("-FUNCO fim.\n");
	return NULL;
}

void* func1(void *arg) {
	print("\n--FUNC1 Eu sou a thread ID1 imprimindo %d\n", *((int *)arg));
print_queue_ready();
	print("--FUNC1 tentando semaforo count = %d\n",semaforo->count);

	cwait(semaforo);
	print("--FUNC1 fazendo coisas de semaforo !!");
	print("--FUNC1 abc ");
	print("--FUNC1 123 ");
	csignal(semaforo);

	return NULL;
}

int main(int argc, char *argv[]) {

	int id1, id2,id3,id4,id5,id6,id7;
	int i = 10;

	semaforo = (csem_t*)malloc(sizeof(csem_t));
	if(csem_init(semaforo,1) == 0) {
		print("Inicializou semaforo binario sucesso\n");
	}

	id1 = ccreate(func0, (void *)&i, 0);
	id2 = ccreate(func1, (void *)&i, 0);

	//id3=ccreate(func1, (void *)&i, 5);
	//id4=ccreate(func1, (void *)&i, 5);
	//id5=ccreate(func1, (void *)&i, 2);
	//id6=ccreate(func1, (void *)&i, 10);
	//id7=ccreate(func1, (void *)&i, 0);

//	int x = cpop_ready();
//print("TID popped: %d\n", x);

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


	print("\nEu sou a main ap�s a cria��o de ID1 e ID2, ocupando o recurso binario");
	if(cwait(semaforo) == 0) {
		print("=>MAIN ocupou o semaforo binario");
		print("=>MAIN Contagem semaforo: %d \n", semaforo->count);
		print("=>MAIN vai fazer CSIGNAL\n");
		print_queue(semaforo->fila);
		int retorno = csignal(semaforo);
		print("=>MAIN Codigo retornado do csignal: %d. \n=>MAIN Count semaforo atual: %d\n",retorno,
	semaforo->count);
		if (retorno == 0) {
			print("=>MAIN fez o CSignal ok, liberou o semaforo ");
		}
		print("=>MAIN vai fazer CYIELD\n");
		retorno = cyield();
		print("=>MAIN retornou esse valor do cyield dela: %d\n", retorno);
		if ( retorno == 0 )
			{
				print("Main fez o CYIELD ok");
			}
	}

	cjoin(id2);

	//cjoin(id1);
	//cjoin(id2);

	print("Eu sou a main voltando para terminar o programa");




	return 0;
}*/
