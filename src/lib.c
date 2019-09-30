
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

FILA2 QQ_Teste; // isso é uma futura fila de filas para Ready

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
	// pq tem que cuidar da thread antiga que tava em exec.
	// originalmente só criei para deixar a init mais bonita /glm
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

	if (CreateFila2(&QQ_Teste) != SUCCESS)
			printf("ERROR: deu ruim no meu teste \n");

		t_main.tid = 0;
		t_main.prio = DEFAULT_PRIO;
		getcontext(&(t_main.context));
		set_exec(&t_main);



		// não acabou

		return;
}
int insert_if_same_prio_as_thread(PFILA2 a, TCB_t* b) {
	//dummy
	return 1;
}
int teste(PFILA2 p_fila2, TCB_t* p_thread) {
	int code;
	if (FirstFila2(p_fila2) != SUCCESS) {
		// CASE: SUPERFICIAL QUEUE EMPTY
		// Must create new FCFS subqueue and insert thread there.
		if (NextFila2(p_fila2) == NXTFILA_VAZIA) {
			PFILA2 p_fcfs;
			if(CreateFila2(p_fcfs) == SUCCESS) {
				if(AppendFila2(p_fcfs,p_thread) == SUCCESS) {
					// Created sub-queue for the priority in p_thread.
					// Inserted p_thread as sole element so far.
					return SUCCESS;
				}
				else {
					printf("Vish: append upon create FCFS failed.\n");
					free(p_fcfs);
					return FAILED;
				}
			}
			else {
				printf("Vish: create FCFS failed.\n");
				return FAILED;
			}
		}
		// CASE: SUPERQUEUE INVALID
		else {
			printf("Vish: superfila deu invalida\n");
			return FAILED;
		}
	}
	// CASE: SUPERQUEUE IS VALID BUT NOT EMPTY.
	// Must traverse to find correct priority FCFS, then append to the FCFS.
	else {
		PFILA2 p_cur_fcfs = GetAtIteratorFila2(p_fila2);
		while( insert_if_same_prio_as_thread(p_cur_fcfs, p_thread) == FAILEDFORHAVINGWRONGPRIO) {
				// obs. a principio, essa FCFS interno nunca pode estar empty entao n precisa testar nessa func,
				// se estiver temos problemas maiores na parte de deletar threads da fila. /glm
			code = NextFila2(p_fila2);
			if(code == NXTFILA_ENDQUEUE) {
				// Insert FCFS at end of Superqueue. (SAME CODE AS ABOVE FOR EMPTY)
				PFILA2 p_fcfs;
				if(CreateFila2(p_fcfs) == SUCCESS) {
					if(AppendFila2(p_fcfs,p_thread) == SUCCESS) {
						// Created sub-queue for the priority in p_thread.
						// Inserted p_thread as sole element so far.
						return SUCCESS;
					}
					else {
						printf("Vish: append upon create FCFS failed.\n");
						free(p_fcfs);
						return FAILED;
					}
				}
				else {
					printf("Vish: create FCFS failed.\n");
					return FAILED;
				}
			}
			else if (code == NXTFILA_ITERINVAL || code == NXTFILA_VAZIA)
				return FAILED;
			else {
				p_cur_fcfs = GetAtIteratorFila2(p_fila2);
			}
		}
		//Inserted!
		return SUCCESS;
	}
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
			if(AppendFila2(p_queue, p_thread) != SUCCESS) {
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
