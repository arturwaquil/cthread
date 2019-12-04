
#include <stdio.h>
#include <stdlib.h>
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

int	tid[3];

void *th3(void *param) {
        int n=(int)param;
   
        printf("%d",n);
        return NULL;
}

void *th2(void *param) {
        int n=(int)param;
   
        printf("%d",n);
	tid[2] = ccreate(th3, (void *)3, ZERO);
	if (tid[2]<0) {printf ("Erro no ccreate(2,1).\n"); 
                       printf ("Fim do main\n");
                       exit(0);
        }
	setStopTimer(1,PRIO_HIGH);
        cyield();
        return NULL;
}

void *th(void *param) {
	int n=(int)param;
	int cont=100;
	while(cont) {
		printf ("%d",n);
		--cont;
	        tid[1] = ccreate(th2, (void *)2, ZERO);
	        if (tid[1]<0) {printf ("Erro no ccreate(2,1).\n");
                               printf ("Fim do main\n");
                exit(0);
        }
        setStopTimer(1,PRIO_MEDIUM);
        cyield();
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	int	delay=10000;
	char	name[256];
	
	printf ("FUNCOES TESTADAS:\n");
	printf ("     identify()\n");
	printf ("     create()\n");
	printf ("     yield()\n");
	printf ("OPERACAO:\n");
	printf ("     main cria uma thread de prio baixa e so faz yield()\n");
	printf ("     threads sempre fazem yield()\n");
	printf ("     Usado setStopTimer para setar todas as prioridades iguais\n");
	printf ("RESULTADO ESPERADO\n");
	printf ("     Uma thread de prioridade mais baixa cria uma de mais alta prioridade.\n");
        printf ("     cada uma de prioriade media ou alta, imprime o seu id, faz yield e termina.\n");
	printf ("     123123123123...\n");
	printf ("DIGITE ALGO PARA INICIAR\n");
	getchar();



	cidentify (name, 255);
	printf ("GRUPO: %s\n\n", name);	
	
	printf("0\n");
	tid[0] = ccreate(th, (void *)1, ZERO);
	printf("1\n");

	if (tid[0]<0) {printf ("Erro no ccreate(1,1).\n"); 
                       printf ("Fim do main\n");
                       exit(0);
        }

	printf("2\n");

	setStopTimer(1,PRIO_LOW);
        cyield();

	printf("3\n");

	while(--delay) {
	        setStopTimer(1,PRIO_LOW);
		cyield();

	}
	printf("4\n");

	printf ("Fim do main\n");
	return 0;
}





