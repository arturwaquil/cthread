//
// Philosophers.c: Este programa implementa um dos classicos de programação
//                 concorrente: a janta dos filósofos.
//                 O objetivo deste programa é testar a implementação do
//                 micro kernel desenvolvido na disciplina INF01142
//
// Primitivas testadas: ccreate, cjoin, cyield, cwait e csignal.
// 
// Este programa é basedo na solução de Tanenbaum apresentada no livro
// "Modern Operating System" (Prentice Hall International).
//
// Disclamer: este programa foi desenvolvido para auxiliar no desenvolvimento
//            de testes para o micronúcleo. NÃO HÁ garantias de estar correto.


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/support.h"
#include "../include/cthread.h"

/*-------------------------------------------------------------------
Operação para Teste
	operacao==0	-> operacao normal
	operacao==1	-> retorna SEMPRE o valor de tempo
	operacao==2	-> reseta operacao e retorna o valor de tempo (one shot)
-------------------------------------------------------------------*/
void	setStopTimer(int op, int tm);

#define	PRIO_LOW	1000
#define	PRIO_HIGH	500

#define BAIXA 2
#define MEDIA 1
#define ALTA  0

#define ZERO  0


#define		SEED	             234
#define		MAXRAND            10000
#define		N		       5
#define		LEFT	i==0?N-1:(i-1)%N
#define		RIGHT	         (i+1)%N
#define		THINKING	       0
#define		HUNGRY		       1
#define		EATING		       2
#define		DONE		       3

int 		state[N], End[N]={0,0,0,0,0}; 
csem_t	        mutex;
csem_t		s[N];
char		status[N*2]={'H',' ','H',' ',
                             'H',' ','H',' ',
                             'H','\0'};                      
 
void sleepao(void){
     int i = 0;
 
     i = rand()%5 + 1;
     for (; i<0; i--) cyield();
     return;
}

void	test(int i) 
{
	if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
	    state[i] = EATING;
	    *(status+2*i) = 'E';
	    printf("%s \n", status);
	    csignal(&s[i]);
	    cyield();
	}
	return ;
}


void	put_forks(int i) 
{
	cwait(&mutex);
	state[i] = THINKING;
	*(status+2*i) = 'T';
	printf("%s\n", status);
	test(LEFT);
	test(RIGHT);
	csignal(&mutex);
	cyield();       /* If scheduling is FIFO without preemption */
	                /* We allow another philosopher to run      */	
	return ;
}

 
void	take_forks(int i) 
{
	cwait(&mutex);
	state[i] = HUNGRY;
	*(status+2*i) = 'H';
	printf("%s \n", status);	
	test(i);
	csignal(&mutex);
	cwait(&s[i]); 	
	return;	
}

 
void	think_eat(void) 
{
	sleepao();
	return;		
}


 
void *Philosophers(void *arg) {
	int i;
	
	i= (int)arg;
		
	while (End[i] < 5) {        /* eat five times then sleeps        */
		think_eat();        /* Philosophe goes to think          */
		take_forks(i);      /* acquire two forks or blocks       */
		think_eat();        /* Philosophe goes to eat            */
		put_forks(i);       /* put back the forks                */	
		End[i]=End[i]+1;	
	}

	*(status+2*i)='D';
	state[i] = DONE;
	return NULL;
}


//*************************** MAIN ***********************
 
int	main(int argc, char *argv[]) {
	int 	ThreadId[N];
	int	i;
        	
        srand((unsigned)time(NULL));

        printf("*******************************************\n");
	printf("Programa do Filósofos (Tanenbaum)\n");
        printf("   Teste: Estressa o uso de semáforos\n\n");
        printf("   Término OK com as mensagens \n");
        printf("     # Diner ends... All philosophers goes to sleep...\n");	
        printf("     D D D D D\n\n");
        printf("*******************************************\n");      

        csem_init(&mutex, 1);
	
	for(i = 0; i < N; i++) 
	   if (csem_init(&s[i], 0)) {
	      printf("# Semaphore initialization error...\n");
	      exit(0);
	   }

        setStopTimer(1, ZERO);

	for(i = 0; i < N; i++) {
	   if ((ThreadId[i] = ccreate(Philosophers, (void *)i,ZERO) < 0)) {
              printf("Error on creating philosophers...\n");
	      exit(0);
	   }
	}

        printf("#\n# The dinner will begin...\n");


	for(i = 0; i < N; i++)
	   cjoin(ThreadId[i]);

       cyield();

        printf("\n# Diner ends... All philosophers goes to sleep...\n\n\n");	

	printf("%s \n", status);

        for (i = 0; i < N ; i ++ )
            printf("%d-", End[i]);
        printf("\n");

        exit(0);   
}
