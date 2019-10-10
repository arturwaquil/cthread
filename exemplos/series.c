
//
// Programa de teste para primitivas de criação e sincronização
//
// Disclamer: este programa foi desenvolvido para auxiliar no desenvolvimento
//            de testes para o micronúcleo. NÃO HÁ garantias de estar correto.

#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>
	int id1, id2, id3, id4, id5, id6, id7, id8, id9, id10;

int* vetor[10];

void* fatorial(void *i) {
	int fat=1, N, n;
	N = *(int *)i;

	printf("Sou a thread %d\n",N);
	if (N < 10) {
	printf("Thread %d decidiu esperar thread %d\n",N, N+1);
	cjoin(vetor[N]);
	printf("Thread %d acordou\n",N);

	}
	for ( n=N; n > 1; --n){
	fat = n * fat;
	}
	printf("Fatorial de %d: %d\n", *(int *)i,fat);

	return NULL;
}

void* fibonnaci (void *i) {
     int fi, fj, fk, k, n;

     n = *(int *)i;

     fi = 0;
     fj = 1 ;
     printf ("0 1");
     for (k = 1; k <= n; k++) {
         fk = fi + fj;
         fi = fj;
         fj = fk;
         printf(" %d", fk);
     }

     printf("\n");
     return NULL;
}

int main(int argc, char **argv) {


	int i1=1, i2=2, i3=3, i4=4, i5=5, i6=6, i7=7, i8=8, i9=9, i10=10;

	vetor[0] = ccreate(fatorial, (void *)&i1, 0);
	vetor[1] = ccreate(fatorial, (void *)&i2, 0);
	vetor[2] = ccreate(fatorial, (void *)&i3, 0);
	vetor[3] = ccreate(fatorial, (void *)&i4, 0);
	vetor[4] = ccreate(fatorial, (void *)&i5, 0);
	vetor[5] = ccreate(fatorial, (void *)&i6, 0);
	vetor[6] = ccreate(fatorial, (void *)&i7, 0);
	vetor[7] = ccreate(fatorial, (void *)&i8, 0);
	vetor[8] = ccreate(fatorial, (void *)&i9, 0);
	vetor[9] = ccreate(fatorial, (void *)&i10, 0);
	printf("Sou a main e quero passar a vez\n");
	cyield();

	printf("Sou a main e decidi esperar meu primogenito th1mmy\n");
	cjoin(vetor[0]);
printf("Sou a main, th1mmy morreu\n");
	printf("La fin\n");


/*
	int id0, id1;
	int i = 10;

	id0 = ccreate(fatorial, (void *)&i, 0);
	id1 = ccreate(fibonnaci, (void *)&i, 0);

        printf("Threads fatorial e Fibonnaci criadas...\n");

	cjoin(id0);
	cjoin(id1);
*/
	printf("Main retornando para terminar o programa\n");

	return 0;

}
