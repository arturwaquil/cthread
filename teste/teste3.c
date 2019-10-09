#include <stdio.h>
#include <stdlib.h>

#include "cthread.h"

void func1();
void func2();
void func3();
void func4();
csem_t sem1, sem2, sem3, sem4;

int th1, th2, th3, th4;

void func1() {
	printf("[%d]: START\n", 1);
	printf("[%d]: == %d ==\n", 1, 1);
	csignal(&sem1);
	printf("[%d]: STOP\n", 1);
}

void func2() {
	printf("[%d]: START\n", 2);
	cwait(&sem1);
	printf("[%d]: == %d ==\n", 2, 2);
	csignal(&sem2);
	printf("[%d]: STOP\n", 2);
}

void func3() {
	printf("[%d]: START\n", 3);
	cwait(&sem2);
	printf("[%d]: == %d ==\n", 3, 3);
	csignal(&sem3);
	printf("[%d]: STOP\n", 3);
}

void func4() {
	printf("[%d]: START\n", 4);
	cwait(&sem3);
	printf("[%d]: == %d ==\n", 4, 4);
	csignal(&sem4);
	printf("[%d]: STOP\n", 4);
}

int main(){
	csem_init(&sem1, 0);
	csem_init(&sem2, 0);
	csem_init(&sem3, 0);
	csem_init(&sem4, 0);
	th1 = ccreate((void *)func3, (void *)NULL, 0);
	printf("[MAIN] Thread with pid = %d\n", th1);
	th2 = ccreate((void *)func1, (void *)NULL, 0);
	printf("[MAIN] Thread with pid = %d\n", th2);
	th3 = ccreate((void *)func4, (void *)NULL, 0);
	printf("[MAIN] Thread with pid = %d\n", th3);
	th4 = ccreate((void *)func2, (void *)NULL, 0);
	printf("[MAIN] Thread with pid = %d\n", th4);

	printf("[MAIN] CWAIT\n");
	cwait(&sem4);
	printf("[MAIN] STOPPED\n");
	return 0;
}
