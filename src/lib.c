
#include <stdio.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

// Thread Control
TCB_t t_main;
TCB_t* t_a_bola_da_vez; // Points to thread currently executing.
int num_threads = 0; // Qty of thread instances; doubles as tid generator.

// Scheduler state queues:
FILA2 Q_Ready; // Processes ready to run when the CPU becomes available.
FILA2 Q_Blocked; // Processes blocked and waiting sync.
FILA2 Q_Exec; // Process[es] running at a given time.
// Suspended states necessary?

// Notação:
// [glm] SUGESTÕES:
// threads começam com t_coisa
// queues com Q_Coisa
// ponteiros em argumentos e variaveis em funcoes com p_coisa
// semáforos com s_coisa ou x_coisa (muteX)
// obrigada por terem vindo ao meu ted talk [/glm]

void set_exec(TCB_t* p_thread) {
	p_thread->state = PROCST_EXEC;
	t_a_bola_da_vez = p_thread;
	// OBS:
	// [glm] Não sei se isso vai dar bom
	// pq tem que cuidar da thread antiga em exec /glm
}

int generate_tid() {
	num_threads++;
	return num_threads;
}

void init_lib() {

	if (CreateFila2(Q_Ready) != 0)
		printf("ERROR: Could not initialize ready queue\n");
	if (CreateFila2(Q_Blocked) != 0)
		printf("ERROR: Could not initialize blocked queue\n");
	if (CreateFila2(Q_Exec) != 0)
		printf("ERROR: Could not initialize exec queue\n");

		t_main.tid = 0;
		t_main.prio = 0;
		getcontext(&(t_main.context));
		set_exec(&t_main);

		// não acabou

		return;
}



int ccreate (void* (*start)(void*), void *arg, int prio) {
	return -1;
}

int cyield(void) {
	return -1;
}

int cjoin(int tid) {
	return -1;
}

int csem_init(csem_t *sem, int count) {
	return -1;
}

int cwait(csem_t *sem) {
	return -1;
}

int csignal(csem_t *sem) {
	return -1;
}

int cidentify (char *name, int size) {
	strncpy (name, "Sergio Cechin - 2019/2 - Teste de compilacao.", size);
	return 0;
}
