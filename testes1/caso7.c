
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

int shared=0;
csem_t sem;


void *th1(void *param) {
	int n=(int)param;
	int flag;
	int count=100;
	
	while(count) {
		// Testa se deve encerrar a thread, quando shared chegar a 100
		// Acesso a variavel compartilhada, com yield() dentro da SC
		cwait(&sem);			// P(S)
		if (shared>=100) flag=1;
		cyield();
		csignal(&sem);			// V(S)
		
		if (flag) break;
		
		// Incrementa shared
		cwait(&sem);			// P(S)
		cyield();
		if (shared % 5 == 0) {
			cyield();
			printf ("(%d)%d-", n, shared);
			shared++;
		}
		csignal(&sem);			// V(S)
		--count;
	}
	return NULL;
}

void *th2(void *param) {
	int n=(int)param;
	int flag;
	int count=100;
	
	while(count) {
		// Testa se deve encerrar a thread, quando shared chegar a 100
		// Acesso a variavel compartilhada, com yield() dentro da SC
		cwait(&sem);			// P(S)
		if (shared>=100) flag=1;
		cyield();
		csignal(&sem);			// V(S)
		
		if (flag) break;
		
		// Incrementa shared
		cwait(&sem);			// P(S)
		cyield();
		if ((shared-1) % 5 == 0) {
			cyield();
			printf ("(%d)%d-", n, shared);
			shared++;
		}
		csignal(&sem);			// V(S)
		--count;
	}
	return NULL;
}

void *th3(void *param) {
	int n=(int)param;
	int flag;
	int count=100;
	
	while(count) {
		// Testa se deve encerrar a thread, quando shared chegar a 100
		// Acesso a variavel compartilhada, com yield() dentro da SC
		cwait(&sem);			// P(S)
		if (shared>=100) flag=1;
		cyield();
		csignal(&sem);			// V(S)
		
		if (flag) break;
		
		// Incrementa shared
		cwait(&sem);			// P(S)
		cyield();
		if ((shared-2) % 5 == 0) {
			cyield();
			printf ("(%d)%d-", n, shared);
			shared++;
		}
		csignal(&sem);			// V(S)
		--count;
	}
	return NULL;
}

void *th4(void *param) {
	int n=(int)param;
	int flag;
	int count=100;
	
	while(count) {
		// Testa se deve encerrar a thread, quando shared chegar a 100
		// Acesso a variavel compartilhada, com yield() dentro da SC
		cwait(&sem);			// P(S)
		if (shared>=100) flag=1;
		cyield();
		csignal(&sem);			// V(S)
		
		if (flag) break;
		
		// Incrementa shared
		cwait(&sem);			// P(S)
		cyield();
		if ((shared-3) % 5 == 0) {
			cyield();
			printf ("(%d)%d-", n, shared);
			shared++;
		}
		csignal(&sem);			// V(S)
		--count;
	}
	return NULL;
}

void *th5(void *param) {
	int n=(int)param;
	int flag;
	int count=100;
	
	while(count) {
		// Testa se deve encerrar a thread, quando shared chegar a 100
		// Acesso a variavel compartilhada, com yield() dentro da SC
		cwait(&sem);			// P(S)
		if (shared>=100) flag=1;
		cyield();
		csignal(&sem);			// V(S)
		
		if (flag) break;
		
		// Incrementa shared
		cwait(&sem);			// P(S)
		cyield();
		if ((shared-4) % 5 == 0) {
			cyield();
			printf ("(%d)%d\n", n, shared);
			shared++;
		}
		csignal(&sem);			// V(S)
		--count;
	}
	return NULL;
}


int main(int argc, char *argv[]) {
	
	int	delay=10000;
	char	name[256];
	int	tid[3];
	
	cidentify (name, 255);
	printf ("GRUPO: %s", name);	

	csem_init(&sem, 1);
	shared = 0;
	

	printf ("FUNCOES TESTADAS:\n");
	printf ("     identify()\n");
	printf ("     create()\n");
	printf ("     wait() e signal()\n");
	printf ("OPERACAO:\n");
	printf ("     main (LOW), cria 5 thread 1 (LOW) e so faz yield()\n");
	printf ("     tambem cria um semaforo\n");
	printf ("     cada thread acessa variavel compartilhada shared\n");
	printf ("     as threads so incrementam shared se (shared - (TH-1)) mod 5 == 0\n");
	printf ("     todos os acesso a shared sao protegidos.\n");
	printf ("     dentro das SCs tem sempre yield()\n");
	printf ("RESULTADO ESPERADO\n");
	printf ("     Roda as 5 threads em ordem: (1)-(2)-(3)-(4)-(5)\n");
	printf ("     Apresenta shared, que deve estar em ordem.\n");
	printf ("     (1)0-(2)1-(3)2-(4)3-(5)4...\n");
	printf ("DIGITE ALGO PARA INICIAR\n");	
	getchar();
	
	setStopTimer(1, PRIO_LOW);
	
	tid[0] = ccreate(th1, (void *)1, ZERO);
	if (tid[0]<0) {printf ("Erro no ccreate(1,1).\n"); goto finish;}
	
	tid[0] = ccreate(th2, (void *)2, ZERO);
	if (tid[0]<0) {printf ("Erro no ccreate(1,1).\n"); goto finish;}

	tid[0] = ccreate(th3, (void *)3, ZERO);
	if (tid[0]<0) {printf ("Erro no ccreate(1,1).\n"); goto finish;}

	tid[0] = ccreate(th4, (void *)4, ZERO);
	if (tid[0]<0) {printf ("Erro no ccreate(1,1).\n"); goto finish;}

	tid[0] = ccreate(th5, (void *)5, ZERO);
	if (tid[0]<0) {printf ("Erro no ccreate(1,1).\n"); goto finish;}
	
	while(--delay) {
		cyield();
	}
	
finish:
	printf ("Fim do main\n");
	return 0;
}


