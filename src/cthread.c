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
int generate_tid();
int is_empty(PFILA2);
int is_valid(PFILA2);
int schedule_next_thread();
int unschedule_current_thread();
int emplace_in_queue(PFILA2, TCB_t*);
TCB_t* pop_queue(PFILA2 p_queue);
void terminate_current_thread();
int failed(char* msg);
void* null(char* msg);
void print(char* msg);
PFILA2 alloc_queue();
TCB_t* alloc_thread();
TCB_t* make_thread(int);
TCB_t* search_queue(PFILA2 p_queue, int tid);
int unblock_current_dormant(int tid);


int lib_initialized = 0;

// Thread Control
TCB_t t_main;
// TCB_t t_terminate;

//ucontext_t scheduler;
ucontext_t terminate;

TCB_t* t_incumbent; // Points to thread currently executing.
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
	return (NextFila2(p_queue) == -NXTFILA_VAZIA);
}

int is_valid(PFILA2 p_queue) {
	return ( (FirstFila2(p_queue) == SUCCESS) ||
					((FirstFila2(p_queue) != SUCCESS) && (NextFila2(p_queue) != -NXTFILA_ITERINVAL)));
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
		
		print("inicializando cthread...");
		
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

		//set main thread's context
		// getcontext(&(t_main.context));
		// char stack[SIGSTKSZ];
		// t_main.context.uc_link = 0;
		// t_main.context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
		// t_main.context.uc_stack.ss_size = sizeof(stack);
		// makecontext(&(t_main.context), (void (*)(void))schedule_next_thread, 0);

		// getcontext(&(scheduler));
		// char stack[SIGSTKSZ];
		// scheduler.uc_link = 0;
		// scheduler.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
		// scheduler.uc_stack.ss_size = sizeof(stack);
		// makecontext(&(scheduler), (void (*)(void))schedule_next_thread, 0);

		//context set when threads context terminate, used on ccreate (uc_link definition)
		// getcontext(&(t_terminate.context));
		// t_terminate.context.uc_link = 0;
		// t_terminate.context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
		// t_terminate.context.uc_stack.ss_size = sizeof(stack);
		// makecontext(&(t_terminate.context), (void (*)(void))terminate_current_thread, 0);

		getcontext(&terminate);
		char stack[SIGSTKSZ];
		terminate.uc_link = 0;
		terminate.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
		terminate.uc_stack.ss_size = sizeof(stack);
		makecontext(&(terminate), (void (*)(void))terminate_current_thread, 0);

		getcontext(&t_main.context);

		// printf("daqui eu volto para a main?\n");
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
		// End queue.
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
			if ((unsigned int)p_current->prio > (unsigned int)p_thread->prio) {
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
				// print("Achou tid, removendo");
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
	//printf(" codigo NEXTFILA2 : %d\n", NextFila2(p_queue) );
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
	p_thread->dormant = -1;

	getcontext(&(p_thread->context));
	return p_thread;
}

int ccreate (void* (*start)(void*), void *arg, int prio) {
	// NOTE: argument "prio" ignored; defaults to zero upon creation
	init();
	TCB_t* p_thread = make_thread(prio);
	printf("criando thread %d\n", p_thread->tid);
	// Fazer coisa de Contexto com a função passada pelo start,
	// para o sistema da maq virtual rodar pra nós. /glm

	// p_thread->context.uc_link = 0;
	// p_thread->context.uc_link = &t_main.context;
	// p_thread->context.uc_link = &t_terminate.context;
	p_thread->context.uc_link = &terminate;

	char stack[SIGSTKSZ];
	p_thread->context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
	p_thread->context.uc_stack.ss_size = sizeof(stack);
	makecontext(&(p_thread->context), (void (*)(void))start, 1, arg);

	int code = emplace_in_queue(&Q_Ready, p_thread );
	if(code != SUCCESS) {
		printf("ERROR: could not insert new thread in ready queue. Freeing thread. \n");
		free(p_thread);
		return FAILED;
	}
	else {
		// print_queue(&Q_Ready);
		return p_thread->tid;
	}
}

int schedule_next_thread() {
	// printf("\nscheduler\n");
	// printf("------------\n");
	// printf("Ready\n");
	// print_queue(&Q_Ready);
	// printf("\n");
	// printf("Blocked\n");
	// print_queue(&Q_Blocked);
	// printf("\n");
	// printf("Exec\n");
	// print_queue(&Q_Exec);
	// printf("------------\n");

	t_incumbent = pop_queue(&Q_Ready);

	if(t_incumbent != NULL ) {
		t_incumbent->state = PROCST_EXEC;
		if (emplace_in_queue(&Q_Exec, t_incumbent) == SUCCESS) {
			t_incumbent->state = PROCST_EXEC;
			startTimer();

			// printf("run-> %d\n", t_incumbent->tid);
			setcontext(&t_incumbent->context);
			// swapcontext(&t_main.context, &t_incumbent->context);

			return SUCCESS;
		}
		else
			return failed("Did not promote next thread ready->exec");
	}
	else
		return failed("Did not select next thread ready->exec");
}


int remove_emplace(TCB_t* thread, PFILA2 from_queue, PFILA2 to_queue) {

	int removal_code = removeByTID(from_queue, thread->tid);
	int emplace_code = emplace_in_queue(to_queue, thread);
	if ((removal_code == SUCCESS) && (emplace_code == SUCCESS)){
		return SUCCESS;
	}
	else {
		return failed("FAILED QUEUE CHANGE at remove_emplace");
	}
}

int unschedule_current_thread() {
	// printf("\n$$$$$$$$$$$$\nretirei da exec\n");
	t_incumbent->prio = stopTimer();
	if( remove_emplace(t_incumbent, &Q_Exec, &Q_Ready) == SUCCESS) {
		t_incumbent->state = PROCST_APTO;
		getcontext(&t_incumbent->context);
		return SUCCESS;
	}
	else return failed("Failed Unschedule curr");
}

int block_current_thread() {
	t_incumbent->prio = stopTimer();
	if( remove_emplace(t_incumbent, &Q_Exec, &Q_Blocked) == SUCCESS) {
			t_incumbent->state = PROCST_BLOQ;
			getcontext(&t_incumbent->context);
			// printf("bloqueei %d\n",t_incumbent->tid);
			return SUCCESS;
	}
	else
		return failed("Failed Block Current ");;
}

int unblock_current_dormant(int tid) {
	TCB_t* p_thread = search_queue(&Q_Blocked, tid);
	if( remove_emplace(p_thread, &Q_Blocked, &Q_Ready) == SUCCESS) {
		return SUCCESS;
	}
	else return failed("Failed Unblock Dormant");
}

void terminate_current_thread() {
	printf("terminando execucao da thread %d\n", t_incumbent->tid);

	t_incumbent->prio = stopTimer();

	//acorda a thread dorment
	if(t_incumbent->dormant!=-1)
	{
		printf("vou acordar a thread %d\n", t_incumbent->dormant);
		if(unblock_current_dormant(t_incumbent->dormant)==FAILED)
		{
			print("Could not wake thread up");
			exit(EXIT_FAILURE);
		}
	}

	if(removeByTID(&Q_Exec, t_incumbent->tid) == SUCCESS) {
		// if(emplace_in_queue(&Q_Ready, t_incumbent) == SUCCESS) {
		// 	t_incumbent->state = PROCST_APTO;
		// 	return SUCCESS;
		// }
		print("");
		t_incumbent->state = PROCST_APTO;
		schedule_next_thread();
	}
	else
	{
		print("ABORT. Cannot terminate thread. Q_Exec is empty");

	}
	

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

	TCB_t* p_thread;

	p_thread = search_queue(&Q_Ready, tid);
	if(p_thread != NULL)
		return p_thread;

	p_thread = search_queue(&Q_Blocked, tid);
	if(p_thread != NULL)
		return p_thread;

	p_thread = search_queue(&Q_Exec, tid);
	return p_thread;

}

int cyield(void) {
	init();
	// Yield takes the executing thread from the EXEC queue,
	// and places it into the ready queue with state = ready;
	// Then it calls the scheduler to promote another thread to EXEC.
	int retorno = unschedule_current_thread();
	int retorno2 = schedule_next_thread();
	printf("Retornos cyield: unschedule %d, schedule %d\n", retorno, retorno2);
	return (retorno + retorno2);
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
	TCB_t* incumbent = find(tid);
	// printf("---\nincumbet!!!!!!!!!!! %d\n---\n", incumbent->tid);

	if(incumbent==NULL)
		//return failed("NULL INCUMBENT AT CJOIN");;
		return FAILED;


	// checar se ha alguma outra thread em espera por essa thread (senao retorna failed)
	if(being_waited(tid)==FAILED)
	{
		// TODO: aqui ta mudando apenas a variavel local??
		incumbent->dormant=t_incumbent->tid;

		int result = block_current_thread() + schedule_next_thread();
		return result;
	}

	return FAILED;
}

int demote_incumbent_to(int new_state) {
	t_incumbent->prio = stopTimer();
	if(removeByTID(&Q_Exec, t_incumbent->tid) != SUCCESS)
		return failed("FAILED DEMOTE 1");

	switch(new_state) {

		case PROCST_APTO:
							if(emplace_in_queue(&Q_Ready, t_incumbent) != SUCCESS)
									return failed("FAILED DEMOTE 2");
							t_incumbent->state = new_state;
							getcontext(&t_incumbent->context);
							return SUCCESS;

		case PROCST_BLOQ:
							if(emplace_in_queue(&Q_Blocked, t_incumbent) != SUCCESS)
									return failed("FAILED DEMOTE 3");
							t_incumbent->state = new_state;
							getcontext(&t_incumbent->context);
							return SUCCESS;

		case PROCST_TERMINO:
							t_incumbent->state = new_state;
							//provavelmente desalocar ?
							return SUCCESS;
		default:
				return failed("FAILED DEMOTE 4");
	}
}


int csem_init(csem_t *sem, int count) {
	init();

	sem->count = count;
	sem->fila  = alloc_queue();
	if( CreateFila2(sem->fila) != SUCCESS ) {
		printf("ERROR: could not initialize semaphore queue\n");
		free(sem->fila);
		sem->fila = NULL;
		return FAILED;
	}
	else {
		return SUCCESS;
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
		// if(block_current_thread == SUCCESS) {
			if(emplace_in_queue(sem->fila, t_incumbent) == SUCCESS) {
				sem->count -= 1;
				return(schedule_next_thread());
			}
			else return(failed("FAILED CWAIT 1"));
		}
		else return(failed("FAILED CWAIT 2"));
	}
	else {
		// RESOURCE AVAILABLE, no need to wait at the semaphore
		sem->count -= 1;
		return SUCCESS;
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
		p_thread->state = PROCST_APTO;
		return remove_emplace(p_thread, &Q_Blocked, &Q_Ready);
	}
	else {
		printf("Ninguem tava esperando no semaforo.\n");
		return SUCCESS;
	}
}

int cidentify (char *name, int size) {
	char *nomes = "Artur Waquil Campana\t00287677\nGiovanna Lazzari Miotto\t00207758\nHenrique Chaves Pacheco\t00299902\n\0";
	
	if (size < strlen(nomes)) {
		printf("ERROR: size is too small\n");
		return FAILED;
	}
	else strncpy(name, nomes, size);
	
	return SUCCESS;
}
