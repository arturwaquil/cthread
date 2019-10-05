#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

#define DEFAULT_PRIO 0
#define SUCCESS 0
#define FAILED -1
#define FAILEDFORHAVINGWRONGPRIO -1000

int lib_initialized = 0;

// Thread Control
TCB_t t_main;
TCB_t* t_a_bola_da_vez; // Points to thread currently executing.
int num_threads = 0; // Qty of thread instances; doubles as tid generator.

// Scheduler state queues:
FILA2 Q_Ready; // Processes ready to run when the CPU becomes available.
FILA2 Q_Blocked; // Processes blocked and waiting sync.
FILA2 Q_Exec; // Process[es] running at a given time.
// Suspended states necessary?

FILA2 QQ_Teste; // isso é uma futura fila de filas para Ready

// Notação,SUGESTÕES:
// threads começam com t_coisa, queues com Q_Coisa
// ponteiros em argumentos e variaveis em funcoes com p_coisa
// semáforos com s_coisa ou x_coisa (muteX)

void set_exec(TCB_t* p_thread) {
	p_thread->state = PROCST_EXEC;
	t_a_bola_da_vez = p_thread;
	// [glm] Não sei se isso vai dar bom
	// pq tem que cuidar da thread antiga que tava em exec.
	// originalmente só criei para deixar a init mais bonita /glm
}

int generate_tid() {
	num_threads++;
	return num_threads;
}

int is_empty(PFILA2 p_queue) {
	return ((FirstFila2(p_queue) != SUCCESS)
			&& (NextFila2(p_queue) == -NXTFILA_VAZIA));
}

int is_valid(PFILA2 p_queue) {
	return ((FirstFila2(p_queue) == SUCCESS)
			|| (NextFila2(p_queue) != -NXTFILA_ITERINVAL));
}

int failed(char* msg) {
	printf("%s\n", msg);
	return FAILED;
}
void* null(char* msg) {
	printf("%s\n", msg);
	return NULL;
}
PFILA2 init_fila2() {
	return (PFILA2)malloc(sizeof(FILA2));
}
TCB_t* init_tcb() {
	return (TCB_t*)malloc(sizeof(TCB_t));
}

void init_lib() {

	Q_Ready = *((PFILA2) malloc(sizeof(FILA2)));
	Q_Blocked = *((PFILA2) malloc(sizeof(FILA2)));
	Q_Exec = *((PFILA2) malloc(sizeof(FILA2)));

	if (CreateFila2(&Q_Ready) != SUCCESS)
		printf("ERROR: Could not initialize ready queue\n");
	if (CreateFila2(&Q_Blocked) != SUCCESS)
		printf("ERROR: Could not initialize blocked queue\n");
	if (CreateFila2(&Q_Exec) != SUCCESS)
		printf("ERROR: Could not initialize exec queue\n");

	if (CreateFila2(&QQ_Teste) != SUCCESS)
			printf("ERROR: deu ruim no meu teste \n");

		t_main.tid = 0;
		t_main.prio = DEFAULT_PRIO;
		//getcontext(&(t_main.context));
		set_exec(&t_main);
		// não acabou
		return;
}
int insert_if_same_prio_as_thread(PFILA2 a, TCB_t* b) {
	//dummy
	return 1;
}
int priority_level_at(PFILA2 queue) {

	TCB_t* p = GetAtIteratorFila2(queue);

	if(p !=NULL ) {
		int prio = p->prio;
		return prio;
	}
	else return FAILED;

}

PFILA2 instantiate_new_fcfs(TCB_t* p_thread) {
	printf("i1\n");
	// p_thread is the founder of a new priority FCFS.
	PFILA2 p_fcfs = (struct sFila2*) init_fila2();
	printf("i2\n");
	if(CreateFila2(p_fcfs) != SUCCESS) {
	printf("pfvrsejaaqui\n");
		return(null("Failed to create a FCFS queue."));
	}
	else {
	printf("i3\n");
		if(AppendFila2(p_fcfs,p_thread) != SUCCESS) {
			free(p_fcfs);
			return(null("Failed to append TCB to FCFS queue."));
		}
		else {
	printf("i4\n");
			// Created sub-queue with FCFS regimen representing the priority in p_thread.
			// Inserted p_thread as sole element so far.
			return p_fcfs;
		}
	}
}

//old version, flattened queue.
int emplace_in_queue(PFILA2 p_queue, TCB_t* p_thread ) {
	int code;


	if (!is_valid(p_queue))
		return (failed("Invalid priority queue"));
	else if (is_empty(p_queue)) {
		//Attempts to append sub-queue to super if not null.
		return AppendFila2(p_queue, p_thread);
	}
	// Queue not empty:
	// traverse until last element with same priority (FIFO).
	else {
		code = FirstFila2(p_queue);
		TCB_t* p_current = GetAtIteratorFila2(p_queue);

		while (code != -NXTFILA_ENDQUEUE) {
			
			if (p_current->prio > p_thread->prio) {
				return InsertBeforeIteratorFila2(p_queue, p_thread);
			}
			else {
				code = NextFila2(p_queue);
				p_current = GetAtIteratorFila2(p_queue);
			}
		}
	}
	return 0;
}

int ccreate (void* (*start)(void*), void *arg, int prio) {
	// NOTE: argument "prio" ignored; defaults to zero upon creation

	if (!lib_initialized)
	{
		init_lib();
		lib_initialized = 1;
	}


	TCB_t* p_thread = (TCB_t*) malloc(sizeof(TCB_t));
	p_thread->tid = generate_tid();
	p_thread->state = PROCST_APTO;
	p_thread->prio = DEFAULT_PRIO;
	getcontext(&(p_thread->context));

	// Fazer coisa de Contexto com a função passada pelo start,
	// para o sistema da maq virtual rodar pra nós. /glm
// OBS: Como inserir o novo na fila de ready? É Prioridade + FIFO
// Encontrar o último com prio 0 e inserir ali? /glm

	int code = emplace_in_queue(&Q_Ready, p_thread );
	printf("depois da emplace\n");
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
