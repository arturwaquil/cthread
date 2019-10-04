
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
int priority_level_at(PFILA2 queue) {return 1;};

PFILA2 instantiate_new_fcfs(TCB_t* p_thread) {
	// p_thread is the founder of a new priority FCFS.
	PFILA2 p_fcfs = (struct sFila2*) init_fila2;
	if(CreateFila2(p_fcfs) != SUCCESS) {
		return(null("Failed to create a FCFS queue."));
	}
	else {
		if(AppendFila2(p_fcfs,p_thread) != SUCCESS) {
			free(p_fcfs);
			return(null("Failed to append TCB to FCFS queue."));
		}
		else {
			// Created sub-queue with FCFS regimen representing the priority in p_thread.
			// Inserted p_thread as sole element so far.
			return p_fcfs;
		}
	}
}

int emplace_superteste(PFILA2 prio_queue, TCB_t* p_thread) {
	int code,flag;
	if (!is_valid(prio_queue))
		return (failed("Invalid priority queue"));
	else if (is_empty(prio_queue)) {
		// Must create new FCFS subqueue and insert thread there.
		PFILA2 p_fcfs = instantiate_new_fcfs(p_thread);
		//Attempts to append sub-queue to super if not null.
		return (p_fcfs != NULL && AppendFila2(prio_queue, p_fcfs)==SUCCESS);
	}
	// CASE: SUPERQUEUE IS VALID BUT NOT EMPTY.
	// Must traverse to find correct priority FCFS, then append to the FCFS.
	else {
		code = FirstFila2(prio_queue);
		PFILA2 current_fcfs = GetAtIteratorFila2(prio_queue);
		//traverse_until_find_correct_spot
		while (code != -NXTFILA_ENDQUEUE) {
			if (priority_level_at(current_fcfs) == p_thread->prio) {
				//insert_thread(current_fcfs, p_thread);
				return(AppendFila2(current_fcfs, p_thread));
			}
			else if (priority_level_at(current_fcfs) > p_thread->prio) {
				PFILA2 p_fcfs =instantiate_new_fcfs(p_thread);
				//!!! insert new fcfs to prioqueue, between the surrounding priorities.
				return(InsertBeforeIteratorFila2(prio_queue, p_fcfs));
			}
			else {
				//iterate.
				code = NextFila2(prio_queue);
				current_fcfs = GetAtIteratorFila2(prio_queue);
			}
		}
		//end of queue without inserting -> append new fcfs to end of prioqueue.
		PFILA2 p_fcfs = instantiate_new_fcfs(p_thread);
		return (p_fcfs != NULL && AppendFila2(prio_queue, p_fcfs) == SUCCESS);
	}
	return 1;
}

//old version, flattened queue.
int emplace_in_queue(PFILA2 p_queue, TCB_t* p_thread ) {
	int code;
	if(FirstFila2(p_queue) != SUCCESS) {
		// Something went wrong (invalid or empty queue)
		code = NextFila2(p_queue);
		if (code == -NXTFILA_ITERINVAL) {
			printf("Vish:[findinsert] fila deu invalida\n");
			return FAILED;
		}
		else if (code == -NXTFILA_VAZIA) {
			//This bitch empty, YEET
			if(AppendFila2(p_queue, p_thread) != SUCCESS) {
				printf("Vish:[findinsert] erro no append em fila vazia ");
				return FAILED;
			}
			else return SUCCESS;
		}
	}
	// Queue not empty:
	// traverse until last element with same priority (FIFO).
	else {
		TCB_t* p_current = GetAtIteratorFila2(p_queue);

		while( 	(p_current->prio <= p_thread->prio)
				 && (NextFila2(p_queue) != -NXTFILA_ENDQUEUE) ) {
		// Traverse the queue until it finds a worse priority (than p_thread).
		// While it doesn't, calling NextFila2 sets iterator to the next element
		// and tests whether it reached the end of the queue.
 			p_current = GetAtIteratorFila2(p_queue);
		}
		// Left the while loop. Two possibilities:
		if(p_current->prio > p_thread->prio ) {
			// First test on while failed, so itr points to
			// the first element with worse priority
			// -> Insert before current element!
			code = InsertBeforeIteratorFila2(p_queue, p_thread) ;
			if(code == SUCCESS)  return SUCCESS;
			else {printf("Vish: InsertB4Iterator failed"); return FAILED;}
		}
		else {
			// Failed at second conditional -> END
			// Attempts to append the TCB to the END of the queue.
			if(AppendFila2(p_queue,p_thread) != SUCCESS) {
				printf("Vish:[findinsert] erro no append em fim de fila ");
				return FAILED;
			}
			else return SUCCESS;
		}
	}
	return 0;
}

int ccreate (void* (*start)(void*), void *arg, int prio) {
	// NOTE: argument "prio" ignored; defaults to zero upon creation
	TCB_t* p_thread = (TCB_t*) malloc(sizeof(TCB_t));
	p_thread->tid = generate_tid();
	p_thread->state = PROCST_APTO;
	p_thread->prio = DEFAULT_PRIO;
	//getcontext(&(p_thread->context));

	// Fazer coisa de Contexto com a função passada pelo start,
	// para o sistema da maq virtual rodar pra nós. /glm
// OBS: Como inserir o novo na fila de ready? É Prioridade + FIFO
// Encontrar o último com prio 0 e inserir ali? /glm

	int code = emplace_superteste(&Q_Ready, p_thread );
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
