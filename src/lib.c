
#include <stdio.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

#define DEFAULT_PRIO 0
#define SUCCESS 0
#define FAILED -1

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
	// pq tem que cuidar da thread antiga que tava em exec /glm
}

int generate_tid() {
	num_threads++;
	return num_threads;
}

void init_lib() {

	if (CreateFila2(&Q_Ready) != SUCCESS)
		printf("ERROR: Could not initialize ready queue\n");
	if (CreateFila2(&Q_Blocked) != SUCCESS)
		printf("ERROR: Could not initialize blocked queue\n");
	if (CreateFila2(&Q_Exec) != SUCCESS)
		printf("ERROR: Could not initialize exec queue\n");

		t_main.tid = 0;
		t_main.prio = DEFAULT_PRIO;
		getcontext(&(t_main.context));
		set_exec(&t_main);



		// não acabou

		return;
}

int find_insert_position(PFILA2 p_queue, TCB_t* p_thread ) {

	int code;
	if(FirstFila2(p_queue) != SUCCESS) {
		// Something went wrong (invalid or empty queue)
		code = NextFila2(p_queue);
		if (code == NXTFILA_ITERINVAL) {
			printf("Vish:[findinsert] fila deu invalida\n");
			return FAILED;
		}
		else if (code == NXTFILA_VAZIA) {
			//This bitch empty, YEET
			if(AppendFila2(p_queue) != SUCCESS) {
				printf("Vish:[findinsert] erro no append em fila vazia ")
				return FAILED;
			}
			else return SUCCESS;
		}
	}
	// Queue not empty:
	// traverse until last element with same priority (FIFO).
	else {
		PFILA2 p_current = GetAtIteratorFila2(p_queue);

		while( 	(p_current->prio <= p_thread->prio)
				 && (NextFila2(p_queue) != NXTFILA_ENDQUEUE) ) {
		// Traverse until changes priority to a worse one than p_queue's
		// If not, NextFila2 already sets iterator to the next element
		// and tests it has reached the end.
 			p_current = GetAtIteratorFila2(p_queue);
		}
		if(p_current->prio > p_thread->prio ) {
			// First test on while failed, so IT points to
			// the first element with worse priority
			// -> Insert before current element!
			code = InsertBeforeIteratorFila2(p_queue, p_thread) ;
			if(code == SUCCESS)  return SUCCESS;
			else {printf("Vish: InsertB4Iterator failed"); return FAILED;}
		}
		else {
			// Failed at second conditional -> END
			// Attempts to append the TCB to the END of the queue.
			if(AppendFila2(p_queue) != SUCCESS) {
				printf("Vish:[findinsert] erro no append em fim de fila ")
				return FAILED;
			}
			else return SUCCESS;
		}
	}
}

int ccreate (void* (*start)(void*), void *arg, int prio) {

	// NOTE: argument "prio" ignored; defaults to zero upon creation
	TCB_t* p_thread = (TCB_t*) malloc(sizeof(TCB_t));
	p_thread->tid = generate_tid();
	p_thread->state = PROCST_APTO;
	p_thread->prio = DEFAULT_PRIO;
	getcontext(&(p_thread->context));
	// Fazer coisa de Contexto com a função passada pelo start,
	// para o sistema da maq virtual rodar pra nós. /glm

// OBS: Como inserir o novo na fila de ready? É Prioridade + FIFO
// Encontrar o último com prio 0 e inserir ali? /glm

// IDEIA para um dia melhor: fila de filas,
// no nivel mais superficial separa por prioridade
// no segundo nivel insere FIFO. É rápido barato e eco-amigável.
// Atualmente tudo a mesma fila. /glm

	int code = find_insert_position(&Q_Ready, p_thread );

	if(code != SUCCESS) {
		printf("ERROR: could not insert new thread in ready queue. Freeing thread. \n");
		free(p_thread);
		return FAILED;
	}
	else
		return p_thread->tid;
}

int cyield(void) {
	return -1;
}

int cjoin(int tid) {
	return -1;
}

int csem_init(csem_t *sem, int count) {

	sem->count = count;
	//FILA2 Q_Semaphore; ops ~como causar um segfault~
	FILA2* p_semaphore = (FILA2*) malloc(sizeof(FILA2));
	if( CreateFila2(p_semaphore) != SUCCESS ) {
		printf("ERROR: could not initialize semaphore queue\n");
		sem->fila = NULL;
		return FAILED;
	}
	else {
		sem->fila = p_semaphore;
		return SUCCESS;
	}
}

int cwait(csem_t *sem) {
	return -1;
}

int csignal(csem_t *sem) {
	return -1;
}

int cidentify (char *name, int size) {
	strncpy (name, "HAG - 2019/2 - Alpha.", size);
	return SUCCESS;
}
