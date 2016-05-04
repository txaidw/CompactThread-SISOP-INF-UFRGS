
/********************************************************************
	Biblioteca de Filas
	Vers. 1.0 - 3/3/16
********************************************************************/

#include <stdlib.h>
#include "../include/fila2.h"



/*-------------------------------------------------------------------
Fun��o:	Inicializa uma estrutura de dados do tipo FILA2
Ret:	==0, se conseguiu
	!=0, caso contr�rio (erro ou fila vazia)
-------------------------------------------------------------------*/
int	CreateFila2(PFILA2 pFila) {
	pFila->it = pFila->first = pFila->last = NULL;
	return 0;
}


/*-------------------------------------------------------------------
Fun��o:	Seta o iterador da fila no primeiro elemento
Ret:	==0, se conseguiu
	!=0, caso contr�rio (erro ou fila vazia)
-------------------------------------------------------------------*/
int	FirstFila2(PFILA2 pFila) {
	pFila->it = pFila->first;
	return (pFila->first==NULL?-1:0);
}

/*-------------------------------------------------------------------
Fun��o:	Seta o iterador da fila no �ltimo elemento
Ret:	==0, se conseguiu
	!=0, caso contr�rio (erro ou fila vazia)
-------------------------------------------------------------------*/
int	LastFila2(PFILA2 pFila) {
	pFila->it = pFila->last;
	return (pFila->first==NULL?-1:0);
}

/*-------------------------------------------------------------------
Fun��o:	Seta o iterador da fila para o pr�ximo elemento
Ret:	==0, se conseguiu
	!=0, caso contr�rio (erro, fila vazia ou chegou ao final da fila)
-------------------------------------------------------------------*/
int	NextFila2(PFILA2 pFila) {
	
	if (pFila->first==NULL)		// fila vazia?
		return -1;
		
	if (pFila->it==NULL)		// iterador invalido?
		return -2;

	pFila->it = pFila->it->next;
	return 0;
}

/*-------------------------------------------------------------------
Fun��o:	Retorna o conte�do do nodo endere�ado pelo iterador da lista "pFila"
Ret:	Ponteiro v�lido, se conseguiu
	NULL, caso contr�rio (erro, lista vazia ou iterador invalido)
-------------------------------------------------------------------*/
void	*GetAtIteratorFila2(PFILA2 pFila) {
	
	if (pFila->first==NULL)		// fila vazia?
		return NULL;
		
	if (pFila->it==NULL)		// iterador invalido?
		return NULL;

	return pFila->it->node;
}

/*-------------------------------------------------------------------
Fun��o:	Coloca o ponteiro "content" no final da fila "pFila"
Ret:	==0, se conseguiu
	!=0, caso contr�rio (erro)
-------------------------------------------------------------------*/
int	AppendFila2(PFILA2 pFila, void *content) {
	
	PNODE2 node2;
	node2 = (PNODE2) malloc(sizeof(NODE2));
	node2->node = content;
	node2->ant = node2->next = NULL;
	
	if (pFila->first == NULL) {
		// lista vazia
		pFila->first = pFila->last = node2;
	} else {
		// lista nao vazia
		pFila->last->next = node2;		
		node2->ant = pFila->last;
		pFila->last = node2;
	}
	return 0;
}

/*-------------------------------------------------------------------
Fun��o:	Coloca o ponteiro "content" logo ap�s o elemento 
	correntemente apontado pelo iterador da fila "pFila"
Ret:	==0, se conseguiu
	!=0, caso contr�rio (erro)
-------------------------------------------------------------------*/
int	InsertAfterIteratorFila2(PFILA2 pFila, void *content) {
	
	PNODE2 node2;
	
	if (pFila->first==NULL)		// fila vazia?
		return -1;
		
	if (pFila->it==NULL)		// iterador invalido?
		return -1;

	node2 = (PNODE2) malloc(sizeof(NODE2));
	node2->node = content;	
		
	// Coloca "node2" na lista, logo apos o iterador
	node2->ant = pFila->it;
	node2->next = pFila->it->next;
	pFila->it->next = node2;
	node2->next->ant = node2;
	
	return 0;
	
}

/*-------------------------------------------------------------------
Fun��o:	Remove o elemento indicado pelo iterador, da lista "pFila"
Ret:	==0, se conseguiu
	!=0, caso contr�rio (erro, fila vazia ou iterador invalido)
-------------------------------------------------------------------*/
int	DeleteAtIteratorFila2(PFILA2 pFila) {
	
	if (pFila->first==NULL)		// fila vazia?
		return -1;
		
	if (pFila->it==NULL)		// iterador invalido?
		return -1;
		
	if (pFila->it == pFila->first) {	// Deletando o primeiro elemento da lista ?
		
		if (pFila->first == pFila->last) {		// Fila com um �nico elemento ?
			pFila->first = pFila->last = NULL;
		} else {
			pFila->first = pFila->it->next;		// Fila com mais de 1 elemento !
			pFila->first->ant = NULL;
		}
		
	} else if (pFila->it == pFila->last) {	// Deletando o ultimo elemento da lista?
		
		if (pFila->first == pFila->last) {		// Fila com um �nico elemento ?
			pFila->first = pFila->last = NULL;
		} else {
			pFila->last = pFila->it->ant;		// Fila com mais de 1 elemento !
			pFila->last->next = NULL;
		}
		
	} else {	/* Deletando um elemento intermediario da fila.
			   Certamente a fila tem 3 ou mais elementos,
			   pois esta sendo pedido para deletar um item que:
				- nao e o primeiro
				- nem � o ultimo */
		pFila->it->ant->next = pFila->it->next;
		pFila->it->next->ant = pFila->it->ant;		
	}
		
	
	struct sFilaNode2 *tmp;
	tmp = pFila->it;
	pFila->it = pFila->it->next;
	free(tmp);
	return 0;
}





