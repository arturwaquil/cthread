
#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

#define BAIXA	2
#define MEDIA	1
#define ALTA	0

#define ZERO    0


/*-------------------------------------------------------------------
Operação para Teste
  operacao==0 -> operacao normal da stopTimer
  operacao==1 -> faz stopTimer voltar SEMPRE o valor dado por int tm
  operacao==2 -> na primeira chamada a stopTimer retorna o valor dado
                 por int tm e depois volta a operação normal (modo one shot)
-------------------------------------------------------------------*/
void	setStopTimer(int op, int tm);

#define	PRIO_LOW	1000
#define	PRIO_HIGH	500


void *th(void *param) {
	int n=(int)param;
	int cont=100;
	while(cont) {
		printf ("%d",n);
		--cont;
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
	printf ("OPERACAO:\n");
	printf ("     main (prio=baixa), so faz yield()\n");
	printf ("     create 3 threads (prio=baixa), que nao fazem yield()\n");
	printf ("RESULTADO ESPERADO\n");
	printf ("     Roda apenas uma das threads de cada vez\n");
	printf ("     11111111...2222222...33333333...\n");	
	printf ("DIGITE ALGO PARA INICIAR\n");	
	getchar();
	
	setStopTimer(1, PRIO_HIGH);	// todos terão a mesma prioridade
	
	cidentify (name, 255);
	printf ("GRUPO: %s\n\n", name);

	tid[0] = ccreate(th, (void *)1, ZERO);
	if (tid[0]<0) {printf ("Erro no ccreate(1,0).\n"); goto finish;}
	
	tid[1] = ccreate(th, (void *)2, ZERO);
	if (tid[1]<0) {printf ("Erro no ccreate(2,0).\n"); goto finish;}
	
	tid[2] = ccreate(th, (void *)3, ZERO);
	if (tid[2]<0) {printf ("Erro no ccreate(3,0).\n"); goto finish;}

	while(--delay) {
		cyield();
	}

finish:	
	printf ("Fim do main\n");
	return 0;
}





