#include <stdio.h>
#include <stdlib.h>

#include "cthread.h"

void func1();
void func2();
void func3();
void func4();
csem_t sem1, sem2, sem3, sem4;

int th1, th2, th3, th4;

void func1() {
	printf("[%d]: START\n", 1);
	printf("[%d]: == %d ==\n", 1, 1);
	csignal(&sem1);
	printf("[%d]: STOP\n", 1);
}

void func2() {
	printf("[%d]: START\n", 2);
	cwait(&sem1);
	printf("[%d]: == %d ==\n", 2, 2);
	csignal(&sem2);
	printf("[%d]: STOP\n", 2);
}

void func3() {
	printf("[%d]: START\n", 3);
	cwait(&sem2);
	printf("[%d]: == %d ==\n", 3, 3);
	csignal(&sem3);
	printf("[%d]: STOP\n", 3);
}

void func4() {
	printf("[%d]: START\n", 4);
	cwait(&sem3);
	printf("[%d]: == %d ==\n", 4, 4);
	csignal(&sem4);
	printf("[%d]: STOP\n", 4);
}

// filas
// sem1 : +th2(func1), -th4(func2)
// sem2 : -th1(func3),
// sem3: -th3(func4),
// sem 4: -MAIN,


/* execucao:
MAIN entra em sleep sem4,
escalona th1 func3
th1 printa START 3
th1 entra sleep sem2
escalona th2 func1
th2 printa START 1,
printa == 1 ==
th2 LIBERA/INCREMENTA semaforo 1 !! (QUE AINDA NAO TINHA NINGUEM NA FILA)
th2 printa STOP 1 e termina
escalona th3 func4
th3 printa START 4
th3 dorme em sem3
escalona th4 func2
th4 printa START 2
th4 REQUERE SEMAFORO 1 -> ESTAVA COUNT 1, ATRIBUI O RECURSO
th4 printa == 2 ==
th4 LIBERA/INCREMENTA SEM2 (QUE TINHA A th1 ESPERANDO->acorda e poe em apto )
th4 printa STOP 2 e termina

[nesse momento, quem executou mais: MAIN > th4 > th2 > th1(fcfs) == th3]
porém th3 e main estao bloqueadas, e th4 e th2 terminaram.
UNICA APTA : th1 (também seria a melhor)
escalona th1 func3, estava esperando sem2 mas foi acordada
th1 printa == 3 ==
th1 LIBERA/INCREMENTA sem3 -> tinha a th3 esperando, acorda th3 e poe APTO
th1 printa STOP 3 e termina.

escalona th3 (func4), única acordada.
th3 entra na SC, printa == 4 ==
th3 LIBERA/INCREMENTA sem4 ---> tinha a MAIN esperando, acorda e torna apta
th3 printa STOP 4 e termina

única apta e viva para escalonar  é MAIN
printa MAIN STOPPED.

fimm



*/
int main(){
	csem_init(&sem1, 0);
	csem_init(&sem2, 0);
	csem_init(&sem3, 0);
	csem_init(&sem4, 0);
	th1 = ccreate((void *)func3, (void *)NULL, 0);
	printf("[MAIN] Thread with pid = %d\n", th1);
	th2 = ccreate((void *)func1, (void *)NULL, 0);
	printf("[MAIN] Thread with pid = %d\n", th2);
	th3 = ccreate((void *)func4, (void *)NULL, 0);
	printf("[MAIN] Thread with pid = %d\n", th3);
	th4 = ccreate((void *)func2, (void *)NULL, 0);
	printf("[MAIN] Thread with pid = %d\n", th4);

	printf("[MAIN] CWAIT\n");
	cwait(&sem4);
	printf("[MAIN] STOPPED\n");
	return 0;
}
