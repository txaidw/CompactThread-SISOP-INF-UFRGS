#define _XOPEN_SOURCE 600 // Solves a OSX deprecated library problem of ucontext.h
#include <ucontext.h>

#include "../include/cthread.h"
#include "../include/cdata.h"


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


/*----------------------------------------------------------------------------*/
/*                                    DATA                                    */
/*----------------------------------------------------------------------------*/
int global_var_tid = 0;
ucontext_t *ending_contex = NULL;
static bool initialized = false; // Library was already initialized
TCB_t *current_running_thread = NULL;


/*----------------------------------------------------------------------------*/
/*                              GENERAL FUNCTIONS                             */
/*----------------------------------------------------------------------------*/


/// Returns a new TCB from the ready queues
/// 
TCB_t* get_new_thread() {
	TCB_t *first_thread = ready_queue_remove_and_return();
	if (first_thread == NULL) {
		CPRINT(("ERROR_CODE: first_thread == NULL\n"));
		return NULL;
	} else {
		return first_thread;
	}
}


// /// The function run_scheduler its called when the thread current in execution needs to leave execution.
// /// It handles the current_running_thread, blocking, seting the context, etc.
// /// And after this it pops another thread from the queue, and puts in execution.
void run_scheduler() {
// 	/* Remove current thread from the ready queue. */
// 	volatile bool already_swapped_context = false;

// 	// Leaving the Execution
// 	if (current_running_thread != NULL) {
// 		// There is a thread active
// 		switch(current_running_thread->state) {
// 			case PI_EXEC: {
// 				/// current_running_thread was executing but now should be
// 				/// re-inserted on the ready queue (Usually after a Yield).
				
// 				PIPRINT(("[PI]: Thread (%d) loose #10 credits\n", current_running_thread->tid));
// 				int cred = current_running_thread->credReal;
// 				cred = cred-10;
// 				current_running_thread->credReal = (cred > 0 ? cred : 0);

// 				ready_queue_insert(current_running_thread);
// 				getcontext(&(current_running_thread->context));
// 				break;
// 			}
// 			case PI_FINISHED:
// 				break;
// 			case PI_BLOCKED: {
// 				PIPRINT(("[PI]: Blocking thread with id: %d\n", current_running_thread->tid));
// 				getcontext(&(current_running_thread->context));
// 				break;
// 			}
// 			case PI_CREATION:
// 				PIPRINT(("[PI]: First time the thread ran\n"));
// 				break;
// 			case PI_READY:
// 				PIPRINT(("[PI]: Thread wasnt running\n"));
// 				break;
// 		}
// 	}
	
// 	// Back to Execution
// 	if (!already_swapped_context) {
// 		already_swapped_context = true;

// 		current_running_thread = get_new_thread(); 

// 		if (current_running_thread == NULL || current_running_thread->tid < 0) {
// 			PIPRINT(("[PI ERROR]: ERROR_CODE: f_thread == NULL || f_thread->tid < 0\n"));
// 			return ERROR_CODE;
// 		} else {
// 			PIPRINT(("[PI]: Thread %d is active now\n", current_running_thread->tid));
// 			current_running_thread->state = PI_EXEC;
//     		setcontext(&(current_running_thread->context));
//     		return SUCCESS_CODE;
// 		}
// 	} else {
// 		return SUCCESS_CODE;
// 	}
}



/// The function executed by the end_thread_context, on the end of each thread execution (via uc_link)
/// Besides freeing resources it unlocks threads that were previously waiting for it.
void end_thread_execution() {
	if(current_running_thread != NULL) {
		int runnig_tid = current_running_thread->tid;

		CPRINT(("[C]: Thread %d is beeing released\n", runnig_tid));
		current_running_thread->state = PROCST_TERMINO;

		free(current_running_thread);
		current_running_thread = NULL;
		// Search for a thread that is waiting for this one to finish.
		TCB_t *waiting_thread = blocked_thread_waiting_for_tid(runnig_tid);
		// There was a thread waiting for it to finish
		if (waiting_thread != NULL) {

			blocked_tid_list_remove(waiting_thread->tid);
			blocked_list_wait_remove(waiting_thread);
			waiting_thread->state = PROCST_APTO;

			ready_queue_insert(waiting_thread);
			run_scheduler();
		} else {
			run_scheduler();
		}
	} else {
		CPRINT(("ERROR_CODE: No thread in the queue;"));
	}
}


/*----------------------------------------------------------------------------*/
/*                            LIB INITIALIZATION                              */
/*----------------------------------------------------------------------------*/

/// This function is responsable to the creation of the main thread TCB
/// and insert in the ready queue
int init_main_thread() {
	TCB_t *thread = (TCB_t*)malloc(sizeof(TCB_t));

	thread->tid = global_var_tid;
	thread->state = PROCST_CRIACAO;
	
	if (((thread->context).uc_stack.ss_sp = malloc(SIGSTKSZ)) == NULL) {
		CPRINT(("[C ERROR]: No memory for stack allocation!"));
		return ERROR_CODE;
	} else {
		(thread->context).uc_stack.ss_size = SIGSTKSZ;
		(thread->context).uc_link = NULL;
		ready_queue_insert(thread);
		getcontext(&(thread->context));
		return SUCCESS_CODE;
	}
}


// /// This initializer creates a end_thread_context, that runs the end_thread_execution function
// /// Each Thread that reaches the end, links (via uc_link) to this context, that handles desallock and unlock threads waiting
int init_end_thread_context() {
	ending_contex = (ucontext_t *) malloc(sizeof(ucontext_t));

	if (getcontext(ending_contex) != 0 || ending_contex == NULL) {
		return ERROR_CODE;
	} else {
		ending_contex->uc_stack.ss_sp = (char *) malloc(SIGSTKSZ);
		ending_contex->uc_stack.ss_size = SIGSTKSZ;
		ending_contex->uc_link = NULL;

		makecontext(ending_contex, end_thread_execution, 0);
		return SUCCESS_CODE;
	}
}


/// This function is called only one time, on the first function provided
/// by the lib.
int internal_init() {
	if (!initialized) {
		initialized = true;
		global_var_tid = 0;
		
		if (initializeAllQueues() != SUCCESS_CODE) {
			CPRINT(("ERROR_CODE: initializeAllQueues"));
			return ERROR_CODE;			
		}

		else if (init_end_thread_context() != SUCCESS_CODE) {
			CPRINT(("ERROR_CODE: init_end_thread_context"));
			return ERROR_CODE;
		}

		else if (init_main_thread() != SUCCESS_CODE) {
			CPRINT(("ERROR_CODE: init_main_thread"));
			return ERROR_CODE;
		}

		else { 
			CPRINT(("DONT UNDERSTAND: init_main_thread"));
			volatile bool main_thread_created = false;
			if (!main_thread_created) {
				CPRINT(("DONT UNDERSTAND: init_main_thread IFIF"));
				main_thread_created = true;
				run_scheduler();
			} else {
				CPRINT(("DONT UNDERSTAND: init_main_threadELSE ELSE"));
			}
			return SUCCESS_CODE;
		}
	} else {
		return ERROR_CODE;
	}
}


// /*----------------------------------------------------------------------------*/
// /*                               LIB FUNCTIONS                                */
// /*----------------------------------------------------------------------------*/


// /// Criacao de um thread e insercao em uma fila de aptos
// ///
// int picreate(int credCreate, void* (*start)(void*), void *arg) {
// 	internal_init();
	
// 	int new_tid = ++global_var_tid;
// 	PIPRINT(("[PI] Creating new thread with tid: %d\n", new_tid));
	
// 	TCB_t *thread = (TCB_t*)malloc(sizeof(TCB_t));
// 	thread->tid = new_tid;
// 	thread->state = PI_CREATION;
// 	thread->credCreate = thread->credReal = credCreate;
// 	thread->next = NULL;thread->prev = NULL;
	
// 	getcontext(&(thread->context));
	
// 	if (((thread->context).uc_stack.ss_sp = malloc(SIGSTKSZ)) == NULL) {
// 		printf("[PI ERROR]: No memory for stack allocation!");
// 		return ERROR_CODE;
// 	} else {
// 		(thread->context).uc_stack.ss_size = SIGSTKSZ;
// 		(thread->context).uc_link = ending_contex;
// 		makecontext(&(thread->context), (void (*)(void))start, 1, arg);
// 		ready_queue_insert(thread);
// 		return new_tid;
// 	}
// }

// /// Libera a CPU voluntariamente
// /// 
// int piyield(void) {
// 	internal_init();

// 	if(!ready_queue_is_empty()) {
// 		PIPRINT(("[PI] Yield\n"));
// 		return run_scheduler();
// 	} else {
// 		return SUCCESS_CODE;
// 	}
// }


// /// Thread atual deve aguardar finalizacao de thread com id "tid"
// /// verifica se a tid existe e apois insere a thread na lista de bloqueados
// int piwait(int tid) {
// 	internal_init();
// 	if (blocked_tid_list_contains(tid)) {
// 		return ERROR_CODE;
// 	} else {
// 		if (contains_tid_in_ready_queue(tid)) { // Thread Exist
// 			TCB_waiting_t *entry = (TCB_waiting_t *) malloc(sizeof(TCB_waiting_t));
// 			entry->blocked_thread_id = current_running_thread->tid;
// 			entry->waiting_for_thread_id = tid;
// 			blocked_tid_list_insert(entry);
// 		    current_running_thread->state = PI_BLOCKED;
// 		    blocked_list_wait_insert(current_running_thread);
// 		    return run_scheduler();
// 		} else {
// 			return SUCCESS_CODE;
// 		}
// 	}
// }






// /*----------------------------------------------------------------------------*/
// /*								     MUTEX       							  */
// /*----------------------------------------------------------------------------*/

// /// Inicializacao do ponteiro mtx com seus valores padroes
// ///
// int pimutex_init(pimutex_t *mtx) {
// 	// Initializing mutex
// 	internal_init();

// 	if(mtx == NULL) {
// 		mtx = (pimutex_t *) malloc(sizeof(pimutex_t));
// 	}
	
// 	mtx->flag = MTX_UNLOCKED;
// 	mtx->first = NULL;
// 	mtx->last = NULL;
// 	return SUCCESS_CODE;
// }


// /// Tranca o mutex se o mesmo ainda nao esta trancado, se ja estiver trancado
// /// coloca a thread em uma fila de bloqueados, aguardando a liberacao do recurso
// int pilock(pimutex_t *mtx) {
// 	internal_init();
// 	PIPRINT(("[PI] MUTEX LOCKING: "));
// 	if(mtx != NULL){
// 		if (mtx->flag == MTX_LOCKED) {
// 			// The resouce is ALREADY being used, so we must block the thread.
// 			PIPRINT(("Already locked\n"));

// 			TCB_queue_t *tempQueue = (TCB_queue_t *) malloc(sizeof(TCB_queue_t));
// 			tempQueue->start = mtx->first;
// 			tempQueue->end = mtx->last;
// 			queue_insert(&tempQueue, current_running_thread);
// 			mtx->first = tempQueue->start;
// 			mtx->last = tempQueue->end;
// 			tempQueue->start = NULL;
// 			tempQueue->end = NULL;
// 			free(tempQueue);
// 			tempQueue = NULL;


// 	    	current_running_thread->state = PI_BLOCKED;
// 	    	blocked_list_mutex_insert(current_running_thread);
// 	    	return run_scheduler();
// 		} else if(mtx->flag == MTX_UNLOCKED) {
// 			// The resouce is NOT being used, so the thread is goint to use.
// 			PIPRINT(("Now locked\n"));
// 			mtx->flag = MTX_LOCKED;
// 			return SUCCESS_CODE;
// 		} else {
// 			return ERROR_CODE;
// 		}
// 	} else {
// 		return ERROR_CODE;
// 	}
// }


// /// Destrava o mutex, e libera as threads bloqueadas esperando pelo recurso
// ///
// int piunlock(pimutex_t *mtx) {
// 	internal_init();
// 	PIPRINT(("[PI] MUTEX UNLOCKING: "));
// 	if (mtx != NULL){
// 		if ((mtx->flag == MTX_LOCKED) && (mtx->first != NULL)) {
// 			// Mutex is locked and there is threads on the blocked queue
// 			PIPRINT(("Now Unlocking\n"));

// 			TCB_queue_t *tempQueue = (TCB_queue_t *) malloc(sizeof(TCB_queue_t));
// 			tempQueue->start = mtx->first;
// 			tempQueue->end = mtx->last;
// 			TCB_t *thread = queue_remove(tempQueue);
// 			mtx->first = tempQueue->start;
// 			mtx->last = tempQueue->end;
// 			tempQueue->start = NULL;
// 			tempQueue->end = NULL;
// 			free(tempQueue);
// 			tempQueue = NULL;
			

// 			blocked_list_mutex_remove(thread);
// 			thread->state = PI_READY;
			
// 			PIPRINT(("[PI]: Thread (%d) receive #20 credits\n", thread->tid));
// 			int cred = thread->credReal;
// 			cred = cred+20;
// 			thread->credReal = (cred < 100 ? cred : 100);
			
// 			ready_queue_insert(thread);
// 		}

// 		if ((mtx->first == NULL) && (mtx->last == NULL)) {
// 			// Zero threads on the blocked queue
// 			PIPRINT(("Already unlocked\n"));

// 			mtx->flag = MTX_UNLOCKED;
// 		}

// 		return SUCCESS_CODE;
// 	} else {
// 		PIPRINT(("[PI ERROR]: Mutex unlock error"));
// 		return ERROR_CODE;
// 	}
// }