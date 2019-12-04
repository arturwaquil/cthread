/* 
 * test_vetor.c: realiza a criação de 10 threads, cada uma delas escreve uma
 * sequencia de 20 letras iguais e passa a vez para outra thread. Repete até
 * preencher um vetor de 1000 caracteres.
 */

#include	"../include/support.h"
#include	"../include/cthread.h"
#include	<stdio.h>
#include	<stdlib.h>

#define ALTA 0
#define MEDIA 1
#define BAIXA 2

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

#define		MAX_SIZE	1000
#define		MAX_THR		10

int vetor[MAX_SIZE];
int  inc = 0;

void *func(void *arg){

   while ( inc < MAX_SIZE ) {
       vetor[inc] = (int)arg;
       inc++;
       if ( (inc % 20) == 0 )
           cyield();
       else
           continue;
   }

   return NULL;
}


int main(int argc, char *argv[]) {
    int i, pid[MAX_THR];

  
//    csetprio((int) NULL, MEDIA);

    setStopTimer(1, PRIO_LOW);

    for (i = 0; i < MAX_THR; i++) {

        pid[i] = ccreate(func, (void *)('A'+i), ZERO);
        if ( pid[i] == -1) {
           printf("ERRO: criação de thread!\n");
           exit(-1);
       }
     }

    for (i = 0; i < MAX_THR; i++) 
         cjoin(pid[i]);

    for (i = 0; i < MAX_SIZE; i++) {    
        if ( (i % 20) == 0 )
           printf("\n");
        printf("%c", (char)vetor[i]);
    }
      
    printf("\nConcluido vetor de letras...\n");

printf("\n SAIDA esperada: 5 conjuntos de 10 linhas\n");
printf(" compostas por 20 caracteres cada (A a J)\n");
    exit(0);
}

