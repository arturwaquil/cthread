#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

csem_t* semaforo;

void* func1(void *arg) {
	cwait(semaforo);
	print("THREAD 1 USANDO SC, E DANDO YIELD");
	cyield();
	csignal(semaforo);
	print("THREAD 1 LIBEROU SC");
	return NULL;
}

void* func2(void *arg) {
	print("THREAD 2 TENTANDO USAR SC e dando uma enrolada pra ter prioridade menor e executar depois");
	sleep(3);
	cwait(semaforo);
	print("THREAD 2 USANDO SC");
	csignal(semaforo);
	print("THREAD 2 LIBEROU SC");
	return NULL;
}

void* func3(void *arg) {
	print("THREAD 3 TENTANDO USAR SC");
	cwait(semaforo);
	print("THREAD 3 USANDO SC");
	csignal(semaforo);
	print("THREAD 3 LIBEROU SC");
	return NULL;
}

void* func4(void *arg) {
	print("THREAD 4 TENTANDO USAR SC - - DEVO CONSEGUIR");
	cwait(semaforo);
	print("THREAD 4 USANDO SC");
	csignal(semaforo);
	print("THREAD 4 LIBEROU SC");
	return NULL;
}

int main(int argc, char *argv[]) {

	semaforo = (csem_t*)malloc(sizeof(csem_t));

    csem_init(semaforo, 2); // DOIS RECURSOS

	int	id1, id2, id3, id4;
	int i;

	id1 = ccreate(func1, (void *)&i, 0);
	id2 = ccreate(func2, (void *)&i, 0);
    id3 = ccreate(func3, (void *)&i, 0);

	sleep(3); // PARA QUE A MAIN DEMORE MAIS QUE A THREAD 2

	cwait(semaforo);
	print("MAIN USANDO SC, E CHAMANDO YIELD");
	cyield();
    csignal(semaforo);
    print("MAIN LIBEROU SC");

    cjoin(id1);
	cjoin(id2);
	cjoin(id3);

	id4 = ccreate(func4, (void *)&i, 0);
	cjoin(id4);

	print("Eu sou a main voltando para terminar o programa");

	return 0;

}

