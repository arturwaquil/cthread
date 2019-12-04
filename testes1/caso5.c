
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
		setStopTimer(1, (n==1? PRIO_LOW: PRIO_HIGH));
		cyield();
	}
	printf ("Fim da th2\n");
	return NULL;
}

int main(int argc, char *argv[]) {
	char	name[256];
	int	id1, id2;
	
	printf ("FUNCOES TESTADAS:\n");
	printf ("     identify()\n");
	printf ("     create()\n");
	printf ("     join()\n");
	printf ("OPERACAO:\n");
	printf ("     main (ALTA), cria th1 (BAIXA) e th2 (MEDIA) e faz join na th2.\n");
	printf ("     th2 fica fazendo yield()\n");
	printf ("     USA setStopTimer para controlar, indiretamente, a prioridade\n");
	printf ("RESULTADO ESPERADO\n");
	printf ("     Roda uma vez a thread 1 e depois apenas a thread 2, pois a main \n");
        printf ("     esta esperando o final da th2\n");
	printf ("     th2 termina, desbloqueia a thread main e termina o programa\n");
	printf ("     12222222...2Fim da th2\n");
	printf ("     SE HOUVER ERRO no cjoin, o programa vai travar\n");
	printf ("DIGITE ALGO PARA INICIAR\n");	
	getchar();
	
	cidentify (name, 255);
	printf ("GRUPO: %s\n\n", name);	

        setStopTimer(1, PRIO_HIGH);  //equivale a colocar a main com a prioridade mais alta
        cyield();

        id1 = ccreate(th2, (void *)1, ZERO);
        if (id1 < 0 ) {printf ("Erro no ccreate())"); return 0;}

	if ( (id2=ccreate(th2, (void *)2, ZERO))<0) {
		printf ("Erro no ccreate(th1, , 1)\n");
	} else {
		setStopTimer(1, PRIO_HIGH);	// Main tem a maior prioridade, 
                                                //mas so vai rodar quando a th2 terminar
		if (cjoin(id2)<0) {
			printf ("Erro no cjoin(th2).\n");
		}
	}

cyield();
	printf ("Fim do main\n");
	return 0;
}


