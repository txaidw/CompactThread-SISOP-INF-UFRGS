/*
 * cdata.h: arquivo de inclusão de uso apenas na geração da libpithread
 *
 * Esse arquivo pode ser modificado. ENTRETANTO, deve ser utilizada a TCB fornecida.
 *
 */

#define _XOPEN_SOURCE 600 // Solves a OSX deprecated library problem of ucontext.h
#include <ucontext.h>

#ifndef __cdata__
#define __cdata__

#define	PROCST_CRIACAO	0
#define	PROCST_APTO	1
#define	PROCST_EXEC	2
#define	PROCST_BLOQ	3
#define	PROCST_TERMINO	4


#define CPRINT(X) printf X // <--- For DEBUG uncommet the print!

/* NÃO ALTERAR ESSA struct */
typedef struct s_TCB { 
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra
					// 0: Criação; 1: Apto; 2: Execução; 3: Bloqueado e 4: Término
	ucontext_t 	context;	// contexto de execução da thread (SP, PC, GPRs e recursos) 
} TCB_t; 


/* Essa struct representa as pilhas de TCBs da biblioteca */
typedef struct TCB_queue {
  TCB_t *first;
  TCB_t *last;
} TCB_queue_t;

#endif
