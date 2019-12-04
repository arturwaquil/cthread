
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


void *th2(void *param) {
	int n=(int)param;
	int cont=50;
	while(cont) {
		printf ("%d",n);
		--cont;
		setStopTimer(1, PRIO_LOW);
		cyield();
	}
	return NULL;
}

void *th1(void *param) {
	int n=(int)param;
	int cont=50;
	setStopTimer(1, PRIO_LOW);
	int id2 = ccreate(th2, (void *)2, ZERO);
	
	if (id2<0) {printf ("Erro no ccreate(th2, , 2)\n");
	} else {
		while(cont) {
			if (cont==25) {
				setStopTimer(1, PRIO_LOW);
				if (cjoin(id2)<0) {
					printf ("Erro no cjoin(th2).\n");
					break;
				}
			}
			printf ("%d",n);
			--cont;
			setStopTimer(1, PRIO_LOW);
			cyield();
		}
	}
	
	return NULL;
}

int main(int argc, char *argv[]) {
	char	name[256];
	int	id1;
	
	printf ("FUNCOES TESTADAS:\n");
	printf ("     identify()\n");
	printf ("     create()\n");
	printf ("     join()\n");
	printf ("OPERACAO:\n");
	printf ("     main (LOW), cria th1 (LOW), que cria th2 (LOW)\n");
	printf ("     Todas ficam fazendo yield()\n");
	printf ("     main (LOW) faz join(th1) e, no meio do tempo, a th1 faz join(th2)\n");
	printf ("     USA setStopTimer para controlar as prioridades\n");
	printf ("RESULTADO ESPERADO\n");
	printf ("     Roda no inicio apenas a th1, depois a th2 e no final a th1\n");
	printf ("     1212121212...22222222...111111...\n");
	printf ("DIGITE ALGO PARA INICIAR\n");	
	getchar();
	
	cidentify (name, 255);
	printf ("GRUPO: %s\n\n", name);	
	
	setStopTimer(1, PRIO_LOW);
	if ( (id1=ccreate(th1, (void *)1, BAIXA))<0) {
		printf ("Erro no ccreate(th1, , 1)\n");
	} else {
		setStopTimer(1, PRIO_LOW);
		if (cjoin(id1)<0) {
			printf ("Erro no cjoin(th1).\n");
		} else {
			setStopTimer(1, PRIO_LOW);
			cyield();
		}
	}
	printf ("Fim do main\n");
	return 0;
}


