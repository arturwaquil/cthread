
#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

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
	int	delay=100;
	char	name[256];

	printf ("FUNCOES TESTADAS:\n");
	printf ("     identify()\n");
	printf ("     yield()\n");
	printf ("OPERACAO:\n");
	printf ("     main (prio=0), so faz yield()\n");
	printf ("RESULTADO ESPERADO\n");
	printf ("     Só tem a main, sem criação de threads. Portanto, só roda ela\n");
	printf ("     1) Apresentar identificacao do grupo\n");	
        printf ("     2) Verificar a inicialização da biblioteca em primeira chamada\n");
	printf ("     2) 000000000...\n");	
	printf ("DIGITE ALGO PARA INICIAR\n");	
	getchar();
	
	cidentify (name, 255);
	printf ("GRUPO: %s\n\n", name);	

	while(--delay) {
		printf ("0");
		cyield();
	}

	printf ("Fim do main\n");
	return 0;
}





