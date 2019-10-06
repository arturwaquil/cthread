#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

#define STACK_SIZE 16384

#define DEFAULT_PRIO 0
#define SUCCESS 0
#define FAILED -1

void init();
void initialize_cthread();
void set_exec(TCB_t*);
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
TCB_t* t_main;
TCB_t* t_a_bola_da_vez; // Points to thread currently executing.
int num_threads = 0; // Qty of thread instances; doubles as tid generator.

// Scheduler state queues:
FILA2 Q_Ready; // Processes ready to run when the CPU becomes available.
FILA2 Q_Blocked; // Processes blocked and waiting sync.
FILA2 Q_Exec; // Process[es] running at a given time.


// Context
// ucontext_t 	main;

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

	printf("?\n");
	t_main = alloc_thread();

	t_main->tid = 0;
	t_main->prio = DEFAULT_PRIO;

	getcontext(&(t_main->context));

	//makecontext from ibm documentation
	t_main->context.uc_link=0;
	if((t_main->context.uc_stack.ss_sp = (char *) malloc(STACK_SIZE)) != NULL) {
		t_main->context.uc_stack.ss_size = STACK_SIZE;
		t_main->context.uc_stack.ss_flags = 0;

		printf("?\n");
		makecontext(&(t_main->context), (void*)&schedule_next_thread, 0);
	}
	else {
		printf("not enough storage for stack");
		return FAILED;
	}


	set_exec(&t_main);
		// não acabou
	return;
}

void init() {
	if (!lib_initialized){
		initialize_cthread();
		lib_initialized = 1;
	}
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
	int code;
	TCB_t* p_thread;
	if (!is_valid(p_queue))
		return (null("Invalid priority queue"));
	else if (is_empty(p_queue)) {
		return (null("Empty queue: nothing to pop"));
	}
	else {
		code = FirstFila2(p_queue);
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

int schedule_next_thread() {
	t_a_bola_da_vez = pop_queue(&Q_Ready) ;
	if(t_a_bola_da_vez != NULL ) {
		t_a_bola_da_vez->state = PROCST_EXEC;
		if (emplace_in_queue(&Q_Exec, t_a_bola_da_vez) == SUCCESS) {
			t_a_bola_da_vez->state = PROCST_EXEC;
			startTimer();
			setcontext(&t_a_bola_da_vez->context);
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

int being_waited_queue(PFILA2 p_queue, int tid) {
	int code;
	if (!is_valid(p_queue))
		return (failed("Invalid priority queue"));
	else if (is_empty(p_queue)) {
	printf("b\n");

		return FAILED;
	}
	else {
		printf("c\n");

		code = FirstFila2(p_queue);
		TCB_t* p_current = GetAtIteratorFila2(p_queue);
		while (code != -NXTFILA_ENDQUEUE) {
			if(p_current->tid==tid)
			{
				if (p_current->dormant != -1) {
					print("Achou tid\n");
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
	printf("0\n");
	if(being_waited_queue(&Q_Ready, tid)==SUCCESS)
		return SUCCESS;
	printf("1\n");
	if(being_waited_queue(&Q_Blocked, tid)==SUCCESS)
		return SUCCESS;
	printf("2\n");	
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

	printf("00\n");
	p_thread = search_queue(&Q_Ready, tid);
	if(p_thread != NULL)
		return p_thread;

	printf("001\n");
	p_thread = search_queue(&Q_Blocked, tid);
	if(p_thread != NULL)
		return p_thread;
	
	printf("002\n");
	p_thread = search_queue(&Q_Exec, tid);
	return p_thread;

}

int ccreate (void* (*start)(void*), void *arg, int prio) {
	// NOTE: argument "prio" ignored; defaults to zero upon creation
	init();
	TCB_t* p_thread = make_thread(prio);
	// Fazer coisa de Contexto com a função passada pelo start,
	// para o sistema da maq virtual rodar pra nós. /glm
	
	//makecontext from ibm documentation
	p_thread->context.uc_link=0;
	if((p_thread->context.uc_stack.ss_sp = (char *) malloc(STACK_SIZE)) != NULL) {
		// char iterator_stack[SIGSTKSZ];
		// p_thread->context.uc_stack.ss_sp = malloc(STACK_SIZE);
		p_thread->context.uc_stack.ss_size = STACK_SIZE;
		p_thread->context.uc_stack.ss_flags = 0;

		makecontext(&(p_thread->context), (void*)&start, 1, arg);
	}
	else {
		printf("not enough storage for stack");
		return FAILED;
	}




	int code = emplace_in_queue(&Q_Ready, p_thread );
	printf("depois da emplace\n");
	if(code != SUCCESS) {
		printf("ERROR: could not insert new thread in ready queue. Freeing thread. \n");
		free(p_thread);
		return FAILED;
	}
	else {
		print("ok retornando tid\n");
		print_queue(&Q_Ready);
		return p_thread->tid;
	}
}

// int swap_thread_execution(TCB_t* incumbent) {
	// t_a_bola_da_vez->prio = stopTimer();
	// if(remove_from_queue(&Q_Exec, t_a_bola_da_vez) == SUCCESS) {
	// 	if(emplace_in_queue(&Q_Blocked, t_a_bola_da_vez) == SUCCESS) {
	// 		t_a_bola_da_vez->state = PROCST_BLOQ;
	// 		return SUCCESS;
	// 	}
	// 	return FAILED;
	// }
	// else return FAILED;
// }

int cyield(void) {
	// Yield takes the executing thread from the EXEC queue,
	// and places it into the ready queue with state = ready;
	// Then it calls the scheduler to promote another thread to EXEC.
	return unschedule_current_thread() + schedule_next_thread() ;
	// Isso eh naughty d+ e provavelmente vou mudar /glm
}

int cjoin(int tid) {
	// acho que nao precisa (?)
	init();

	printf("?\n");
	// checar se nao existe (nao foi criada ou ja terminou)
	TCB_t* incumbent = find(tid);
	printf("%d\n",&incumbent->context);

	if(incumbent==NULL)
		return FAILED;
	

	// checar se ha alguma outra thread em espera por essa thread (senao retorna failed)
	if(being_waited(tid)==FAILED)
	{
		// swap_thread_execution(&incumbent);
		printf("Hummm\n");
		// printf("%d\n",t_a_bola_da_vez->tid);
		// printf("%d\n",incumbent->tid);
		// swapcontext(&(t_a_bola_da_vez->context), &(incumbent->context));
		setcontext(&incumbent->context);
	}

	return SUCCESS;
}

int csem_init(csem_t *sem, int count) {
	init();

	sem->count = count;
	PFILA2 p_semaphore = alloc_queue();
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
	init();
	return -1;
}

int csignal(csem_t *sem) {
	init();
	return -1;
}

int cidentify (char *name, int size) {
	strncpy (name, "HAG - 2019/2 - Alpha.", size);
	return SUCCESS;
}
