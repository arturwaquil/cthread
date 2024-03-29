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
#define MEM 32000

int		generate_tid();
int		is_empty(PFILA2);
int		is_valid(PFILA2);
int		failed(char* msg);
void	print(char* msg);
void*	null(char* msg);
PFILA2	alloc_queue();
TCB_t*	alloc_thread();
void	init();
void	print_queue(PFILA2 p_queue);
int		emplace_in_queue(PFILA2, TCB_t*);
int		remove_from_queue(PFILA2 p_queue, int tid);
TCB_t*	pop_queue(PFILA2 p_queue);
TCB_t*	make_thread(int);
int		schedule_next_thread();
int		remove_emplace(TCB_t* thread, PFILA2 from_queue, PFILA2 to_queue);
int		unschedule_current_thread();
int		block_current_thread(unsigned int);
int		unblock_current_dormant(int tid);
void	terminate_current_thread();
int		being_waited_queue(PFILA2 p_queue, int tid);
int		being_waited(int tid);
TCB_t*	search_queue(PFILA2 p_queue, int tid);
TCB_t*	find(int tid);

int lib_initialized = 0;

ucontext_t terminate;
TCB_t t_main;
TCB_t* t_incumbent; // Points to thread currently executing.
int num_threads = 0; // Qty of thread instances; doubles as tid generator.

// Scheduler state queues:
FILA2 Q_Ready; // Processes ready to run when the CPU becomes available.
FILA2 Q_Blocked; // Processes blocked and waiting sync.
FILA2 Q_Exec; // Process[es] running at a given time.

int generate_tid() {
	num_threads++;
	return num_threads;
}

int is_empty(PFILA2 p_queue) {
	return (NextFila2(p_queue) == -NXTFILA_VAZIA);
}

int is_valid(PFILA2 p_queue) {
	return ( (FirstFila2(p_queue) == SUCCESS) ||
					((FirstFila2(p_queue) != SUCCESS) && (NextFila2(p_queue) != -NXTFILA_ITERINVAL)));
}

int failed(char* msg) {
	//printf("%s\n", msg);
	return FAILED;
}
void print(char* msg) {
	//printf("%s\n", msg);
}

void* null(char* msg) {
	//printf("%s\n", msg);
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

		//print("inicializando cthread...");

		Q_Ready		=  *alloc_queue();
		Q_Blocked	=  *alloc_queue();
		Q_Exec		=  *alloc_queue();

		if (CreateFila2(&Q_Ready) != SUCCESS)
			printf("ERROR: Could not initialize ready queue\n");
		if (CreateFila2(&Q_Blocked) != SUCCESS)
			printf("ERROR: Could not initialize blocked queue\n");
		if (CreateFila2(&Q_Exec) != SUCCESS)
			printf("ERROR: Could not initialize exec queue\n");

		t_main.tid = 0;
		t_main.prio = DEFAULT_PRIO;
		t_main.dormant = -1;

		getcontext(&terminate);
		// char stack[SIGSTKSZ];
		terminate.uc_link = 0;
		terminate.uc_stack.ss_sp = (char*) malloc(MEM);
		terminate.uc_stack.ss_size = MEM;
		makecontext(&(terminate), (void (*)(void))terminate_current_thread, 0);

		getcontext(&t_main.context);

		t_incumbent = &t_main;
		emplace_in_queue(&Q_Exec, t_incumbent);
		startTimer();
		t_main.state = PROCST_EXEC;

		lib_initialized = 1;
	}
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
			printf("Item %d : prio %d, TID %d, state %d, dormant %d\n",
				count, p_current->prio, p_current->tid, p_current->state, p_current->dormant);
			code = NextFila2(p_queue);
			p_current = GetAtIteratorFila2(p_queue);
		}
		print("PRINT QUEUE: Reached endqueue.");
	}
}

void print_queue_ready(){
	print_queue(&Q_Ready);
}
void print_queue_bloq(){
	print_queue(&Q_Blocked);
}
void print_queue_exec(){
	print_queue(&Q_Exec);
}

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
			if ((unsigned int)p_current->prio > (unsigned int)p_thread->prio) {
				return InsertBeforeIteratorFila2(p_queue, p_thread);
			}
			else {
				code = NextFila2(p_queue);
				p_current = GetAtIteratorFila2(p_queue);
			}
		}
		return AppendFila2(p_queue, p_thread);
	}
}

int remove_from_queue(PFILA2 p_queue, int tid) {
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
				return DeleteAtIteratorFila2(p_queue);
			}
			else {
				code = NextFila2(p_queue);
				p_current = GetAtIteratorFila2(p_queue);
			}
		}
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
	if(p_thread == NULL)
		return NULL;

	p_thread->tid = generate_tid();
	p_thread->state = PROCST_APTO;
	p_thread->prio = prio;
	p_thread->dormant = -1;

	getcontext(&(p_thread->context));
	return p_thread;
}

int ccreate (void* (*start)(void*), void *arg, int prio) {
	init(); // Initializes cthread.
	//Allocate thread data
	TCB_t* p_thread = make_thread(prio);
	if(p_thread == NULL)
		return FAILED;

	//Initialize and allocate thread's context
	p_thread->context.uc_link = &terminate;
	// char stack[SIGSTKSZ];
	p_thread->context.uc_stack.ss_sp = (char*) malloc(MEM);
	if(p_thread->context.uc_stack.ss_sp == NULL)
		return FAILED;

	p_thread->context.uc_stack.ss_size = MEM;
	makecontext(&(p_thread->context), (void (*)(void))start, 1, arg);

	//Insert new thread in the Q_Ready
	int code = emplace_in_queue(&Q_Ready, p_thread );
	if(code != SUCCESS) {
		//printf("ERROR: could not insert new thread in ready queue. Freeing thread. \n");
		free(p_thread);
		return FAILED;
	}
	else {
		return p_thread->tid;
	}
}

int schedule_next_thread() {
	//Selects next thread
	//print_queue_ready();
	t_incumbent = pop_queue(&Q_Ready);

	if(t_incumbent != NULL ) {
		//Insert next thread in the Q_Exec
		if (emplace_in_queue(&Q_Exec, t_incumbent) == SUCCESS) {
			t_incumbent->state = PROCST_EXEC;
			//sets next thread's context
			startTimer();
			setcontext(&t_incumbent->context);
			return SUCCESS;
		}
		else
			return failed("Did not promote next thread ready->exec");
	}
	else
		return failed("Did not select next thread ready->exec");
}


int remove_emplace(TCB_t* thread, PFILA2 from_queue, PFILA2 to_queue) {

	int removal_code = remove_from_queue(from_queue, thread->tid);
	int emplace_code = emplace_in_queue(to_queue, thread);
	if ((removal_code == SUCCESS) && (emplace_code == SUCCESS)){
		return SUCCESS;
	}
	else {
		return failed("ERROR: Failed to switch between state queues.");
	}
}

int unschedule_current_thread() {
	//Stops running thread timer
	t_incumbent->prio = (int)stopTimer();

	//Swaps running thread queue from Q_Exec to Q_Ready and saves its context
	if( remove_emplace(t_incumbent, &Q_Exec, &Q_Ready) == SUCCESS) {
		t_incumbent->state = PROCST_APTO;
		getcontext(&t_incumbent->context);
		return SUCCESS;
	}
	else return failed("Failed Unschedule curr");
}

int block_current_thread(unsigned int time) {
	//Stops running thread timer
	t_incumbent->prio = (int)time;

	//Swaps running thread queue from Q_Exec to Q_Blocked and saves its context
	if( remove_emplace(t_incumbent, &Q_Exec, &Q_Blocked) == SUCCESS) {
			t_incumbent->state = PROCST_BLOQ;
			getcontext(&t_incumbent->context);
			return SUCCESS;
	}
	else
		return failed("Failed Block Current ");;
}

int unblock_current_dormant(int tid) {
	//Get blocked thread from Q_Blocked
	TCB_t* p_thread = search_queue(&Q_Blocked, tid);

	//Swaps blocked thread queue from Q_Blocked to Q_Ready and saves its context
	if( remove_emplace(p_thread, &Q_Blocked, &Q_Ready) == SUCCESS) {
		p_thread->state = PROCST_APTO;
		return SUCCESS;
	}
	else return failed("Failed Unblock Dormant");
}

void terminate_current_thread() {
	t_incumbent->prio = (int)stopTimer(); //Stops running thread timer
	t_incumbent->state = PROCST_TERMINO;
	//Wakes up dormant thread, if any
	if(t_incumbent->dormant!=-1)
	{
		if(unblock_current_dormant(t_incumbent->dormant)==FAILED)
		{
			//print("Could not wake thread up");
			exit(EXIT_FAILURE);
		}
	}
	//Remove running thread from Q_Exec
	if(remove_from_queue(&Q_Exec, t_incumbent->tid) == SUCCESS) {

		free(t_incumbent->context.uc_stack.ss_sp);
		free(t_incumbent);
		// calls scheduler
		schedule_next_thread();
	}
	else
	{
		// ABORT. Cannot terminate thread. Q_Exec is empty
		exit(EXIT_FAILURE);
	}
}

int being_waited_queue(PFILA2 p_queue, int tid) {
	int code;
	if (!is_valid(p_queue))
		return (failed("Invalid priority queue"));
	else if (is_empty(p_queue)) {

		return FAILED;
	}
	else {

		code = FirstFila2(p_queue);
		TCB_t* p_current = GetAtIteratorFila2(p_queue);
		while (code != -NXTFILA_ENDQUEUE) {
			if(p_current->tid==tid)
			{
				if (p_current->dormant != -1) {
					return SUCCESS;
				}
				return FAILED;
			}
			else
			{
				code = NextFila2(p_queue);
				p_current = GetAtIteratorFila2(p_queue);

			}
		}
		return FAILED;
	}
}

int being_waited(int tid) {
	if(being_waited_queue(&Q_Ready, tid)==SUCCESS)
		return SUCCESS;
	if(being_waited_queue(&Q_Blocked, tid)==SUCCESS)
		return SUCCESS;
	if(being_waited_queue(&Q_Exec, tid)==SUCCESS)
		return SUCCESS;
	return FAILED;
}

TCB_t* search_queue(PFILA2 p_queue, int tid) {
	int code;
	if (!is_valid(p_queue))
		return NULL;
	else if (is_empty(p_queue)) {
		return NULL;
	}
	else {
		code = FirstFila2(p_queue);
		TCB_t* p_current = GetAtIteratorFila2(p_queue);
		while (code != -NXTFILA_ENDQUEUE) {
			if(p_current->tid==tid)
			{
				return p_current;
			}
			else
			{
				code = NextFila2(p_queue);
				p_current = GetAtIteratorFila2(p_queue);
			}
		}
		return NULL;
	}
}

TCB_t* find(int tid) {
	// Traverses all queues to find thread with given TID.
	// Returns NULL if none found.

	TCB_t* p_thread;

	p_thread = search_queue(&Q_Ready, tid);
	if(p_thread != NULL)
		return p_thread;

	p_thread = search_queue(&Q_Blocked, tid);
	if(p_thread != NULL)
		return p_thread;

	p_thread = search_queue(&Q_Exec, tid);
	return p_thread;	// returns NULL if tid is not found in any queue

}

int cyield() {
	init(); // Initializes cthread.

	// Incumbent yields its processing spot (Exec -> Ready)
	// Scheduler schedules next running thread.

	int code1 = unschedule_current_thread();
	int code2 = schedule_next_thread();
	if ( (code1 == SUCCESS) && (code2 == SUCCESS)) {
			return SUCCESS;
		}
	else return failed("ERROR: Could not context swap after cyield");
}

int cjoin(int tid) {
	init(); // Initializes cthread.

	unsigned int time_at_call = stopTimer();

	// Incumbent called to wait on thread "tid"
	TCB_t* awaited = find(tid);
	if( awaited == NULL) // "tid" does not exist (not found in any queue).
		return FAILED;


	// Only one thread may be waiting for "tid" at once.
	if(being_waited(tid) == FAILED)
	// "Tid" was previously not joined to anyone.
	{
		// Incumbent is registered as DORMANT & waiting for the awaited thread.
		awaited->dormant = t_incumbent->tid;

		int code1 = block_current_thread(time_at_call);
		int code2 = schedule_next_thread();
		if ( (code1 == SUCCESS) && (code2 == SUCCESS)) {
				return SUCCESS;
			}
		else return failed("ERROR: Could not context swap after joining threads.");
	}
	return FAILED; // Some thread is already joined to AWAITED.
}

int csem_init(csem_t *sem, int count) {
	init(); // Initializes cthread.

	if(sem == NULL){
		//print("ERROR: could not initialize semaphore (invalid pointer).");
		return FAILED;
	}

	sem->count = count;
	sem->fila  = alloc_queue();
	if( CreateFila2(sem->fila) != SUCCESS ) {
		//printf("ERROR: could not create semaphore queue.\n");
		free(sem->fila);
		sem->fila = NULL;
		return FAILED;
	}
	else {
		return SUCCESS;
	}
}

int is_valid_semaphore(csem_t* sem) {
	if (sem == NULL){
		return 0;
	}
	if (sem->fila == NULL){
		return 0;
	}
	if (!is_valid(sem->fila)){
		return 0;
	}
	return 1;
}

int cwait(csem_t *sem) {
	init(); // Initializes cthread.
	unsigned int time_at_call = stopTimer();
	// Semaphore must have been initialized beforehand.
	if(!is_valid_semaphore(sem))
		return failed("ERROR: semaphore not initialized.");

	// A waiting call always decrements the resource count
	sem->count -= 1;
	if(sem->count < 0) {
		// Resource count was already nonpositive before this call;
		// Caller must wait at the semaphore.
		if( (block_current_thread(time_at_call) == SUCCESS)
			&& (emplace_in_queue(sem->fila, t_incumbent) == SUCCESS) ) {

				return(schedule_next_thread());
		}
		else return(failed("ERROR: failed to place thread in waiting."));
	}
	else {
		// There was a free spot available for the caller.
		return SUCCESS;
	}
}

int csignal(csem_t *sem) {
	init(); // Initializes cthread.
	// Semaphore must have been initialized beforehand.
	if( !is_valid_semaphore(sem))
		return failed("ERROR: semaphore not initialized.");

	sem->count += 1; // A signal always increments resource.

	// Pops semaphore queue.
	// If empty do nothing else, otherwise wake the first thread from BLOCKED to READY.
	TCB_t* p_thread;
	p_thread = pop_queue(sem->fila);

	if (p_thread != NULL) {
		p_thread->state = PROCST_APTO;
		return remove_emplace(p_thread, &Q_Blocked, &Q_Ready);
	}
	else {
		return SUCCESS;
	}
}

int cidentify (char *name, int size) {
	char *nomes = "Artur Waquil Campana\t00287677\nGiovanna Lazzari Miotto\t00207758\nHenrique Chaves Pacheco\t00299902\n\0";
	if (size < strlen(nomes)) {
		printf("ERROR: identification requires size %zu or larger.\n", strlen(nomes));
		return FAILED;
	}
	else strncpy(name, nomes, size);
	return SUCCESS;
}
