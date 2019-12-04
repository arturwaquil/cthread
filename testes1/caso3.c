
#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

#define BAIXA 2
#define MEDIA 1
#define ALTA  0

#define ZERO  0


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


void *th1(void *param) {
	int n=(int)param;
	int cont=100;
	while(cont) {
		printf ("%d",n);
		--cont;
		setStopTimer(1, PRIO_LOW);	// Menor prioridade
		cyield();
	}
	return NULL;
}

void *th2(void *param) {
	int n=(int)param;
	int cont=100;
	while(cont) {
		printf ("%d",n);
		--cont;
		setStopTimer(1, PRIO_HIGH);  // Maior prioridade
		cyield();		     // nao faz yield, para trancar nessa thread
	}
	return NULL;
}

void *th3(void *param) {
	int n=(int)param;
	int cont=100;
	while(cont) {
		printf ("%d",n);
		--cont;
		setStopTimer(1, PRIO_MEDIUM);	// Média prioridade
		cyield();
	}
	return NULL;
}



int main(int argc, char *argv[]) {
	int	delay=10000;
	char	name[256];
	int	tid[3];
	
	printf ("FUNCOES TESTADAS:\n");
	printf ("     identify()\n");
	printf ("     create()\n");
	printf ("     parcialmente, yield()\n");
	printf ("     mecanismo de prioridades\n");
	printf ("OPERACAO:\n");
	printf ("     main (prio=HIGH), so faz yield()\n");
	printf ("     create thread=2 (prio=ALTA), que NAO FAZ yield()\n");
	printf ("     create threads=1 (prio=BAIXA) e 3 (prio=MEDIA), que FAZEM yield()\n");
	printf ("     Usado setStopTimer, para atribuir, indiretamente, as prioridades\n");
	printf ("RESULTADO ESPERADO\n");
	printf ("     Executa as threads 1, 2 e 3 e depois apenas a thread=2\n");
	printf ("     As outras nao rodam porque a main faz yield para ela mesmo e encerra\n");
	printf ("     1232222....\n");
	printf ("DIGITE ALGO PARA INICIAR\n");	
	getchar();

	cidentify (name, 255);
	printf ("GRUPO: %s\n\n", name);	

	tid[0] = ccreate(th1, (void *)1, ZERO);
	if (tid[0]<0) {printf ("Erro no ccreate(1,1).\n"); goto finish;}
	
        tid[1] = ccreate(th2, (void *)2, ZERO);
	if (tid[1]<0) {printf ("Erro no ccreate(2,1).\n"); goto finish;}
	
	tid[2] = ccreate(th3, (void *)3, ZERO);
	if (tid[2]<0) {printf ("Erro no ccreate(3,1).\n"); goto finish;}
	
	while(--delay) {
	    setStopTimer(1, PRIO_HIGH);
	    cyield();
	}
	
finish:	
	printf ("Fim do main\n");
	return 0;
}





