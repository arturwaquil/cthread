
/*
	Condição de teste
		- Várias threads com semáforo iniciando com contador em 2
	Resultado de teste
		- Entra: 0,1,2; tranca:2 -> 0101010101
		- Sai: 0,1; entra 3,4; tranca:4 -> 23232323232323
		- Sai: 2,3; entre 5; -> 454545454545
*/
#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"


#define ZERO 0

/*-------------------------------------------------------------------
Operação para Teste
  operacao==0 -> operacao normal da stopTimer
  operacao==1 -> faz stopTimer voltar SEMPRE o valor dado por int tm
  operacao==2 -> na primeira chamada a stopTimer retorna o valor dado
                 por int tm e depois volta a operação normal (modo one shot)
-------------------------------------------------------------------*/
void	setStopTimer(int op, int tm);

#define	PRIO_LOW	1000
#define PRIO_MEDIUM     750
#define	PRIO_HIGH	500

csem_t sem;

void *th(void *param) {
	int n=(int)param;
	int delay=100;
	
	printf ("Entry th %d\n", n);
	
	if (n<5) {
		if (ccreate(th, (void *)(n+1), 1)<0) {
			printf ("Erro no ccreate da th.\n");
			return NULL;
		}
	}
		
	
	cwait(&sem);
	while(--delay) {
		printf ("%d",n);
		cyield();
	}
	csignal(&sem);
	
	printf ("\nExit  th %d\n", n);
	return NULL;
}

int main(int argc, char *argv[]) {
	int delay=10000;
	char	name[256];
	
	cidentify (name, 255);
	printf ("GRUPO: %s", name);	
	
	csem_init(&sem, 2);	
	
	printf ("FUNCOES TESTADAS:\n");
	printf ("     identify()\n");
	printf ("     create()\n");
	printf ("     wait() e signal()\n");
	printf ("OPERACAO:\n");
	printf ("     main (LOW), cria thread 1 (LOW) e so faz yield()\n");
	printf ("          tambem cria um semaforo inicializado com 2\n");
	printf ("     cada thread:\n");
	printf ("          1. Coloca Entry message\n");
	printf ("          2. Cria nova thread (ate ter criado 5 delas)\n");
	printf ("          3. Realiza operacoes dentro de SC\n");
	printf ("          4. Coloca Exit message\n");
	printf ("RESULTADO ESPERADO\n");
	printf ("     So roda 2 threads por vez, devido ao semaforo\n");
	printf ("          Pode rodar qualquer combinacao de 2 threads\n");
	printf ("          Nao pode alternar entre tres ou mais threads\n");
	printf ("     Por exemplo, Roda sempre aos pares: 01010..., 23232... e 45454...\n");
	printf ("DIGITE ALGO PARA INICIAR\n");	
	getchar();
	
	setStopTimer(1, PRIO_LOW);
	
	ccreate(th, (void *)0, ZERO);
	while(--delay) {
		cyield();
	}
	printf ("Fim do main\n");
	return 0;
}


