#define _XOPEN_SOURCE 600 // Resolve um problema do OS X com ucontext.h
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
static bool initialized = false; // Library já foi iniciada
TCB_t *current_running_thread = NULL;


/*----------------------------------------------------------------------------*/
/*                              GENERAL FUNCTIONS                             */
/*----------------------------------------------------------------------------*/


/// Retorna um novo TCB da lista de aptos 
/// 
TCB_t* get_new_thread() {
	TCB_t *first_thread = ready_queue_remove_and_return();
	if (first_thread == NULL) {
		//CPRINT(("ERROR_CODE: first_thread == NULL\n"));
		return NULL;
	} else {
		return first_thread;
	}
}


// /// The function run_scheduler its called when the thread current in execution needs to leave execution.
// /// It handles the current_running_thread, blocking, seting the context, etc.
// /// And after this it pops another thread from the queue, and puts in execution.
int run_scheduler() {
	/* Remove thread corrente da lista de apta */
	//CPRINT(("[C]: SCHEDULER STARTED \n"));
	volatile bool already_swapped_context = false;

	// Deixando a execução
	if (current_running_thread != NULL) {
		// Tem uma thread ativa
		switch(current_running_thread->state) {
			case PROCST_EXEC: {
				/// current_running_thread estava executando mas agora vai ser reincerida na lista de apta
				
				//CPRINT(("[C]: Thread Yield: \n", current_running_thread->tid));
				current_running_thread->state = PROCST_APTO;
				ready_queue_insert(current_running_thread);
				getcontext(&(current_running_thread->context));
				break;
			}
			case PROCST_TERMINO:
				break;
			case PROCST_BLOQ: {
				//CPRINT(("[C]: Blocking thread with id: %d\n", current_running_thread->tid));
				getcontext(&(current_running_thread->context));
				break;
			}
			case PROCST_CRIACAO:
				//CPRINT(("[C]: First time the thread ran\n"));
				break;
			case PROCST_APTO:
				//CPRINT(("[C]: Thread wasnt running\n"));
				break;
		}
	}
	
	// De volta a execução
	if (!already_swapped_context) {
		already_swapped_context = true;

		current_running_thread = get_new_thread(); 

		if (current_running_thread == NULL || current_running_thread->tid < 0) {
			//CPRINT(("[C ERROR]: ERROR_CODE: f_thread == NULL || f_thread->tid < 0\n"));
			return ERROR_CODE;
		} else {
			//CPRINT(("[C]: Thread %d is active now\n", current_running_thread->tid));
			current_running_thread->state = PROCST_EXEC;
    		setcontext(&(current_running_thread->context));
    		return SUCCESS_CODE;
		}
	} else {
		return SUCCESS_CODE;
	}
}


/// A função executa no fim de end_thread_context, no fim de cada execução de thread (pelo uc_link)
/// Além de liberar recursos ela destranca threads que estavam no estado wait, antes.
void end_thread_execution() {
	if(current_running_thread != NULL) {
		int runnig_tid = current_running_thread->tid;

		//CPRINT(("[C]: Thread %d is beeing released\n", runnig_tid));
		current_running_thread->state = PROCST_TERMINO;

		free(current_running_thread);
		current_running_thread = NULL;
		// Procura pela thread que estava esperando esta aqui terminar.
		TCB_tid_waiting_t *waiting_thread = blocked_list_thread_waiting_for(runnig_tid);
		// Existia uma thread esperando por esta terminar.
		if (waiting_thread != NULL) {
			//CPRINT(("[C] A thread was waiting for this to finish\n"));

			blocked_list_remove(waiting_thread);
			waiting_thread->blocked_thread->state = PROCST_APTO;

			ready_queue_insert(waiting_thread->blocked_thread);
			run_scheduler();
		} else {
			//CPRINT(("[C] No thread was waiting for this to finish\n"));
			run_scheduler();
		}
	} else {
		//CPRINT(("ERROR_CODE: No thread in the queue;\n"));
	}
}


/*----------------------------------------------------------------------------*/
/*                            LIB INITIALIZATION                              */
/*----------------------------------------------------------------------------*/

/// Essa função é responsável pela criação da thread TCB
/// E inserí-la na lista de aptos
int init_main_thread() {
	TCB_t *thread = (TCB_t*)malloc(sizeof(TCB_t));

	thread->tid = global_var_tid;
	thread->state = PROCST_CRIACAO;
	
	if (((thread->context).uc_stack.ss_sp = malloc(SIGSTKSZ)) == NULL) {
		//CPRINT(("[C ERROR]: No memory for stack allocation!"));
		return ERROR_CODE;
	} else {
		(thread->context).uc_stack.ss_size = SIGSTKSZ;
		(thread->context).uc_link = NULL;
		ready_queue_insert(thread);
		getcontext(&(thread->context));
		return SUCCESS_CODE;
	}
}


// /// Esse inicializador cria a end_thread_context, que roda a end_thread_execution
// /// Cada thread que chega ao fim linka à este contexto (com uc_link)que cuida de desalocar e liberar threads que estão esperando
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


/// Essa função é chamada uma única vez, na primeira função provida pela biblioteca (lib).
int internal_init() {
	if (!initialized) {
		initialized = true;
		global_var_tid = 0;
		
		if (initializeAllQueues() != SUCCESS_CODE) {
			//CPRINT(("ERROR_CODE: initializeAllQueues"));
			return ERROR_CODE;			
		}

		else if (init_end_thread_context() != SUCCESS_CODE) {
			//CPRINT(("ERROR_CODE: init_end_thread_context"));
			return ERROR_CODE;
		}

		else if (init_main_thread() != SUCCESS_CODE) {
			//CPRINT(("ERROR_CODE: init_main_thread"));
			return ERROR_CODE;
		}

		else { 
			//CPRINT(("[C]: init_main_thread\n"));
			volatile bool main_thread_created = false;
			if (!main_thread_created) {
				//CPRINT(("DONT UNDERSTAND WHY: NO IDEIA WHY THIS WORK"));
				main_thread_created = true;
				run_scheduler();
			} else {
				//CPRINT(("DONT UNDERSTAND WHY:"));
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


/// Criacao de um thread e insercao em uma fila de aptos
///
int ccreate (void* (*start)(void*), void *arg) {
	internal_init();
	
	int new_tid = ++global_var_tid;
	//CPRINT(("[C] Creating new thread with tid: %d\n", new_tid));
	
	TCB_t *thread = (TCB_t*)malloc(sizeof(TCB_t));
	thread->tid = new_tid;
	thread->state = PROCST_CRIACAO;
	
	getcontext(&(thread->context));
	
	if (((thread->context).uc_stack.ss_sp = malloc(SIGSTKSZ)) == NULL) {
		//CPRINT(("[C ERROR]: No memory for stack allocation!"));
		return ERROR_CODE;
	} else {
		(thread->context).uc_stack.ss_size = SIGSTKSZ;
		(thread->context).uc_link = ending_contex;
		makecontext(&(thread->context), (void (*)(void))start, 1, arg);
		ready_queue_insert(thread);
		return new_tid;
	}
}

/// Libera a CPU voluntariamente
/// 
int cyield(void) {
	internal_init();

	if(!ready_queue_is_empty()) {
		//CPRINT(("[C] Yield\n"));
		return run_scheduler();
	} else {
		return ERROR_CODE;
	}
}


/// Thread atual deve aguardar finalizacao de thread com id "tid"
/// verifica se a tid existe e apois insere a thread na lista de bloqueados
int cjoin(int tid) {
	internal_init();
	if (blocked_list_thread_waiting_for(tid) != NULL) { // Thread is already being waited by another
		return ERROR_CODE;
	} else {
		if (ready_queue_return_thread_with_id(tid) != NULL) { // Thread Exist
			TCB_tid_waiting_t *entry = (TCB_tid_waiting_t *) malloc(sizeof(TCB_tid_waiting_t));
			entry->waiting_for_thread_id = tid;
			entry->blocked_thread = current_running_thread;
		    current_running_thread->state = PROCST_BLOQ;
			blocked_list_insert(entry);
		    return run_scheduler();
		} else {
			return SUCCESS_CODE;
		}
	}
}






// /*----------------------------------------------------------------------------*/
// /*								     MUTEX       							  */
// /*----------------------------------------------------------------------------*/

/// Inicializacao do semaforo com seus valores padroes
///
int csem_init(csem_t *sem, int count) {
		// Initializing mutex
	internal_init();

	if(sem == NULL) {
		sem = (csem_t *) malloc(sizeof(csem_t));
	}
	if (sem->fila == NULL) {
		sem->fila = (FILA2 *) malloc(sizeof(FILA2));
	}
	sem->count = count;
	sem->fila = (FILA2 *) malloc(sizeof(FILA2));
	return CreateFila2(sem->fila);
}


/// Tranca o semaforo se o mesmo ainda nao esta trancado, se ja estiver trancado
/// coloca a thread em uma fila de bloqueados, aguardando a liberacao do recurso
int cwait(csem_t *sem) {
	internal_init();

	//CPRINT(("[C] REQUESTING SEMAPHORE WAIT\n"));
	if(sem != NULL){
		if (sem->count <= 0) {
			// O recurso JÁ ESTÁ sendo usado, então precisamos bloquear a thread.
		//	CPRINT(("Already locked\n"));

			AppendFila2(sem->fila, current_running_thread);
	    	current_running_thread->state = PROCST_BLOQ;

	    	semaphore_list_append_if_not_contained(sem);
	    	return run_scheduler();
		} else {
			// O recurso NÃO ESTÁ sendo usado, então a thread vai usá-lo.
			//CPRINT(("[C] Semaphore wasnt locked\n"));
			sem->count -= 1;
			return SUCCESS_CODE;
		}
	} else {
		return ERROR_CODE;
	}
}


/// Destrava o semaforo, e libera as threads bloqueadas esperando pelo recurso
///
int csignal(csem_t *sem) {
	internal_init();
	//CPRINT(("[C] SEMAPHORE UNLOCKING: "));
	if (sem != NULL) {
		sem->count += 1;

		TCB_t *thread = (TCB_t *)semaphore_list_remove_and_return_first_thread_from(sem);
		if (thread != NULL) {
			// Mutex é bloqueado e tem threads na lista de bloqueadas
			//CPRINT(("Now Unlocking\n"));
			thread->state = PROCST_APTO;
			return ready_queue_insert(thread);
		} else {
			//CPRINT(("Not Locked\n"));

			return SUCCESS_CODE;
		}
	} else {
		//CPRINT(("\n[C ERROR]: Semaphore unlock error\n"));
		return ERROR_CODE;
	}
}