#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

#define DEFAULT_PRIO 0
#define SUCCESS 0
#define FAILED -1
void init();
void initialize_cthread();
int generate_tid();
int is_empty(PFILA2);
int is_valid(PFILA2);
int schedule_next_thread();
int unschedule_current_thread();
int emplace_in_queue(PFILA2, TCB_t*);
int remove_from_queue(PFILA2, TCB_t*);
TCB_t* pop_queue(PFILA2 p_queue);
int failed(char* msg);
void* null(char* msg);
void print(char* msg);
PFILA2 alloc_queue();
TCB_t* alloc_thread();
TCB_t* make_thread(int);

int lib_initialized = 0;

// Thread Control
TCB_t t_main;
TCB_t* t_a_bola_da_vez; // Points to thread currently executing.
int num_threads = 0; // Qty of thread instances; doubles as tid generator.

// Scheduler state queues:
FILA2 Q_Ready; // Processes ready to run when the CPU becomes available.
FILA2 Q_Blocked; // Processes blocked and waiting sync.
FILA2 Q_Exec; // Process[es] running at a given time.

// Notação,SUGESTÕES:
// threads começam com t_coisa, queues com Q_Coisa
// ponteiros em argumentos e variaveis em funcoes com p_coisa
// semáforos com s_coisa ou x_coisa (muteX)

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
void print(char* msg) {
	printf("%s\n", msg);
}
void* null(char* msg) {
	printf("%s\n", msg);
	return (void*)NULL;
}
PFILA2 alloc_queue() {
	return (PFILA2)malloc(sizeof(FILA2));
}
TCB_t* alloc_thread() {
	return (TCB_t*)malloc(sizeof(TCB_t));
}


void init() {
	if (!lib_initialized){
		initialize_cthread();
		lib_initialized = 1;
	}
}
void initialize_cthread() {

	Q_Ready 	=  *alloc_queue();
	Q_Blocked =  *alloc_queue();
	Q_Exec	  =  *alloc_queue();

	if (CreateFila2(&Q_Ready) != SUCCESS)
		printf("ERROR: Could not initialize ready queue\n");
	if (CreateFila2(&Q_Blocked) != SUCCESS)
		printf("ERROR: Could not initialize blocked queue\n");
	if (CreateFila2(&Q_Exec) != SUCCESS)
		printf("ERROR: Could not initialize exec queue\n");

		t_main.tid = 0;
		t_main.prio = DEFAULT_PRIO;
		getcontext(&(t_main.context));
		t_main.state = PROCST_EXEC;
		t_a_bola_da_vez = &t_main;
		// não acabou
		return;
}

int priority_at(PFILA2 queue) {
	TCB_t* p = GetAtIteratorFila2(queue);
	if(p !=NULL ) {
		int prio = p->prio;
		return prio;
	}
	else return FAILED;
}

void print_queue(PFILA2 p_queue) {
	if (!is_valid(p_queue))
		print("PRINT QUEUE Invalid priority queue");
	else if (is_empty(p_queue)) {
		print("PRINT QUEUE Empty queue");
	}
	else {
		print("Printing queue...");
		int count = 0;
		int code = FirstFila2(p_queue);
		TCB_t* p_current = GetAtIteratorFila2(p_queue);
		while (code != -NXTFILA_ENDQUEUE) {
			count ++ ;
			printf("Item %d : prio %d, TID %d, state %d\n",
				count, p_current->prio, p_current->tid, p_current->state);
			code = NextFila2(p_queue);
			p_current = GetAtIteratorFila2(p_queue);
		}
		// End queue.
		print("PRINT QUEUE: Reached endqueue.");
	}
}

// Queue insert
int emplace_in_queue(PFILA2 p_queue, TCB_t* p_thread ) {
	int code;
	if (!is_valid(p_queue))
		return (failed("Invalid priority queue"));
	else if (is_empty(p_queue)) {
		return AppendFila2(p_queue, p_thread);
	}
	// Queue not empty:
	// traverse until priority becomes higher than new element,
	// inserting before it. If end queue reached, append to the end.
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
		// END QUEUE
		return AppendFila2(p_queue, p_thread);
	}
}

int remove_from_queue(PFILA2 p_queue, TCB_t* p_thread) {
	int code ;
	if (!is_valid(p_queue))
		return (failed("Invalid priority queue"));
	else if (is_empty(p_queue)) {
		return (failed("Empty queue: nothing to remove"));
	}
	else {
		code = FirstFila2(p_queue);
		TCB_t* p_current = GetAtIteratorFila2(p_queue);
		while (code != -NXTFILA_ENDQUEUE) {
			if (p_current->tid == p_thread->tid) {
				print("Achou thread, removendo");
				return DeleteAtIteratorFila2(p_queue);
			}
			else {
				code = NextFila2(p_queue);
				p_current = GetAtIteratorFila2(p_queue);
			}
		}
		// END QUEUE
		return failed("Thread not found in queue for removal.");
	}
}


int removeByTID(PFILA2 p_queue, int tid) {
	int code ;
	if (!is_valid(p_queue))
		return (failed("Invalid priority queue"));
	else if (is_empty(p_queue)) {
		return (failed("Empty queue: nothing to remove"));
	}
	else {
		code = FirstFila2(p_queue);
		TCB_t* p_current = GetAtIteratorFila2(p_queue);
		while (code != -NXTFILA_ENDQUEUE) {
			if (p_current->tid == tid) {
				print("Achou tid, removendo");
				return DeleteAtIteratorFila2(p_queue);
			}
			else {
				code = NextFila2(p_queue);
				p_current = GetAtIteratorFila2(p_queue);
			}
		}
		// END QUEUE
		return failed("Tid not found in queue for removal.");
	}
}

TCB_t* pop_queue(PFILA2 p_queue) {
	TCB_t* p_thread;
	if (!is_valid(p_queue))
		return (null("Invalid priority queue"));
	else if (is_empty(p_queue)) {
		return (null("Empty queue: nothing to pop"));
	}
	else {
	  FirstFila2(p_queue);
		p_thread = GetAtIteratorFila2(p_queue);
		if(DeleteAtIteratorFila2(p_queue) != SUCCESS) {
			return(null("Failed to pop thread from queue"));
		}
		else return p_thread;
	}
}

TCB_t* make_thread(int prio) {

	TCB_t* p_thread = alloc_thread();
	p_thread->tid = generate_tid();
	p_thread->state = PROCST_APTO;
	p_thread->prio = prio;
	getcontext(&(p_thread->context));
	return p_thread;
}

int ccreate (void* (*start)(void*), void *arg, int prio) {
	// NOTE: argument "prio" ignored; defaults to zero upon creation
	init();
	TCB_t* p_thread = make_thread(prio);
	// Fazer coisa de Contexto com a função passada pelo start,
	// para o sistema da maq virtual rodar pra nós. /glm

	int code = emplace_in_queue(&Q_Ready, p_thread );
	printf("depois da emplace\n");
	if(code != SUCCESS) {
		printf("ERROR: could not insert new thread in ready queue. Freeing thread. \n");
		free(p_thread);
		return FAILED;
	}
	else {
		print_queue(&Q_Ready);
		return p_thread->tid;
	}
}

int schedule_next_thread() {
	t_a_bola_da_vez = pop_queue(&Q_Ready) ;
	if(t_a_bola_da_vez != NULL ) {
		t_a_bola_da_vez->state = PROCST_EXEC;
		if (emplace_in_queue(&Q_Exec, t_a_bola_da_vez) == SUCCESS) {
			t_a_bola_da_vez->state = PROCST_EXEC;
			startTimer();
			return SUCCESS;
		}
		else
			return failed("Did not promote next thread ready->exec");
	}
	else
		return failed("Did not select next thread ready->exec");
}

int unschedule_current_thread() {
	t_a_bola_da_vez->prio = stopTimer();
	if(remove_from_queue(&Q_Exec, t_a_bola_da_vez) == SUCCESS) {
		if(emplace_in_queue(&Q_Ready, t_a_bola_da_vez) == SUCCESS) {
			t_a_bola_da_vez->state = PROCST_APTO;
			return SUCCESS;
		}
		return FAILED;
	}
	else return FAILED;
}

int cpop_ready() {
	TCB_t* thread = pop_queue(&Q_Ready);
	if (thread != NULL)
	{
		print_queue(&Q_Ready);
		return thread->tid;
	}
	else return -1;
}

void cremove_ready(int tid) {
printf("\nCalled removal of tid %d\n",tid);
removeByTID(&Q_Ready, tid);

print("Queue after call to remove element: ");
print_queue(&Q_Ready);
printf("\n");
}


int cyield(void) {
	init();
	// Yield takes the executing thread from the EXEC queue,
	// and places it into the ready queue with state = ready;
	// Then it calls the scheduler to promote another thread to EXEC.
	return unschedule_current_thread() + schedule_next_thread() ;
	// Isso eh naughty d+ e provavelmente vou mudar /glm
}

int cjoin(int tid) {
	init();
	// A thread calls cjoin to wait on another with identifier "tid".
	// -If "tid" does not exist or finished exec, return ERROR CODE.
	// -If two or more threads wait for "tid", the first is served and
	//  the latter ones receive ERROR CODE immediately.
	// -Else:
	//  The CALLER/WAITER thread, which is executing,
	//   becomes a blocked thread (Blocked Q);
	//  The joining intent is registered on the WAITED thread,
	//   (somehow);
	//  Once WAITED is finished executing,
	//   CALLER is woken from cryo-sleep and put in Ready Q;
	//

	return -1;
}

int csem_init(csem_t *sem, int count) {
	init();

	sem->count = count;
	PFILA2 p_semaphore = alloc_queue();
	if( CreateFila2(p_semaphore) != SUCCESS ) {
		printf("ERROR: could not initialize semaphore queue\n");
		free(p_semaphore);
		sem->fila = NULL;
		return FAILED;
	}
	else {
		sem->fila = p_semaphore;
		return SUCCESS;
	}
}

int demote_incumbent_to(int new_state) {
	t_a_bola_da_vez->prio = stopTimer();
	if(removeByTID(&Q_Exec, t_a_bola_da_vez->tid) != SUCCESS)
		return FAILED;

	switch(new_state) {

		case PROCST_APTO:
							if(emplace_in_queue(&Q_Ready, t_a_bola_da_vez) != SUCCESS)
									return FAILED;
							t_a_bola_da_vez->state = new_state;
							return SUCCESS;

		case PROCST_BLOQ:
							if(emplace_in_queue(&Q_Blocked, t_a_bola_da_vez) != SUCCESS)
									return FAILED;
							t_a_bola_da_vez->state = new_state;
							return SUCCESS;

		case PROCST_TERMINO:
							t_a_bola_da_vez->state = new_state;
							//provavelmente desalocar ?
							return SUCCESS;
		default:
				return FAILED;
	}
}

int cwait(csem_t *sem) {
	init();
	// A waiting call always decrements the resource count
	// (this is being done after guaranteeing that the thread
	// has been blocked and emplaced in the semaphore queue)
	if(sem->count <= 0) {
		// Resource count was already nonpositive before this call;
		// Caller thread must wait at the semaphore.
		if(demote_incumbent_to(PROCST_BLOQ) == SUCCESS) {
			if(emplace_in_queue(sem->fila, t_a_bola_da_vez) == SUCCESS) {
				sem->count -= 1;
				return(schedule_next_thread());
			}
			else return(failed("FAILED CWAIT 1"));
		}
		else return(failed("FAILED CWAIT 2"));
	}
	else {
		// RESOURCE AVAILABLE, no need to wait at the semaphore
		// nao sei o que fazer aqui com essa informacao./glm
		sem->count -= 1;
		return SUCCESS;
		//supostamente ja ta executando entao acho q eh isso? /glm
	}
}

int csignal(csem_t *sem) {
	init();
	// Thread currently executing is leaving a critical section.
	// The resource count is incremented as the resource is freed.
	sem->count += 1;
	TCB_t* p_thread;
	p_thread = pop_queue(sem->fila);
	if (p_thread != NULL) {
		if( removeByTID(&Q_Blocked, p_thread->tid)==SUCCESS
	   && emplace_in_queue(&Q_Ready, p_thread) == SUCCESS) {
			p_thread->state = PROCST_APTO;
			return SUCCESS;
		}
		else return FAILED;
	}
	else return FAILED;
}

int cidentify (char *name, int size) {
	strncpy (name, "HAG - 2019/2 - Alpha.", size);
	return SUCCESS;
}
