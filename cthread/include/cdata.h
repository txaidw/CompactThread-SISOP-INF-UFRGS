/*
 * cdata.h: arquivo de inclusão de uso apenas na geração da libpithread
 *
 * Esse arquivo pode ser modificado. ENTRETANTO, deve ser utilizada a TCB fornecida.
 *
 */

#define _XOPEN_SOURCE 600 // Solves a OSX deprecated library problem of ucontext.h
#include <ucontext.h>
#include "../include/cthread.h"
#include <stdbool.h>

#ifndef __cdata__
#define __cdata__

#define	PROCST_CRIACAO	0
#define	PROCST_APTO	1
#define	PROCST_EXEC	2
#define	PROCST_BLOQ	3
#define	PROCST_TERMINO	4

#define ERROR_CODE -1
#define SUCCESS_CODE 0

#define CPRINT(X) //printf X // <--- For DEBUG uncommet the print!

/* NÃO ALTERAR ESSA struct */
typedef struct s_TCB { 
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra
					// 0: Criação; 1: Apto; 2: Execução; 3: Bloqueado e 4: Término
	ucontext_t 	context;	// contexto de execução da thread (SP, PC, GPRs e recursos) 
} TCB_t; 

typedef struct TCB_tid_waiting {
	int waiting_for_thread_id;
	TCB_t *blocked_thread;
} TCB_tid_waiting_t;

#endif


int initializeAllQueues();
int ready_queue_insert(TCB_t *thread);
TCB_t* ready_queue_remove_and_return();
TCB_t* ready_queue_return_thread_with_id(int tid);
bool ready_queue_is_empty();

int semaphore_list_append_if_not_contained(csem_t *sem);
TCB_t* semaphore_list_remove_and_return_first_thread_from(csem_t *sem);

int blocked_list_remove(TCB_tid_waiting_t *thread);
int blocked_list_insert(TCB_tid_waiting_t *thread);
TCB_tid_waiting_t* blocked_list_thread_waiting_for(int tid);


