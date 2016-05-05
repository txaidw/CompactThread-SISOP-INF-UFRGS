
#include "../include/cdata.h"
#include "../include/fila2.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

FILA2 *ready_active;
FILA2 *blocked_list;
FILA2 *semaphore_list;

int initializeAllQueues() {
	ready_active = (FILA2 *)malloc(sizeof(FILA2));
	blocked_list = (FILA2 *)malloc(sizeof(FILA2));
	semaphore_list = (FILA2 *)malloc(sizeof(FILA2));

	if (CreateFila2(ready_active) != 0) {
		printf("Error queue 1\n");
		return -1;
	}
	if (CreateFila2(blocked_list) != 0) {
		printf("Error queue 2\n");
		return -1;
	}
	if (CreateFila2(semaphore_list) != 0) {
		printf("Error queue 1\n");
		return -1;
	}
	return 0;
}


int ready_queue_insert(TCB_t *thread) {
	return AppendFila2(ready_active, thread);
}

TCB_t* ready_queue_remove_and_return() {
	if (FirstFila2(ready_active) == 0) {
		TCB_t *obj = (TCB_t *)GetAtIteratorFila2(ready_active);
		if (DeleteAtIteratorFila2(ready_active) == 0) {
			return obj;
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

TCB_t* ready_queue_return_thread_with_id(int tid) {
	if (FirstFila2(ready_active) == 0) {
		do {
			TCB_t *value = (TCB_t *)GetAtIteratorFila2(ready_active);
			if (value != NULL && value->tid == tid) {
				return value;
			}
		} while (NextFila2(ready_active) == 0);
		return NULL;
	} else {
		return NULL;
	}	
}

bool ready_queue_is_empty() {
	if (FirstFila2(ready_active) == 0) {
		return false;
	} else {
		return true;
	}
}

int semaphore_list_append_if_not_contained(csem_t *sem) {
	if (FirstFila2(semaphore_list) == 0) {
		do {
			csem_t *value = (csem_t *)GetAtIteratorFila2(semaphore_list);
			if (value != NULL && value == sem) {
				return 0;
			}
		} while (NextFila2(ready_active) == 0);

		AppendFila2(semaphore_list, sem);
		return 0;
	} else {
		return -1;
	}
}

TCB_t* semaphore_list_remove_and_return_first_thread_from(csem_t *sem) {
	if (FirstFila2(sem->fila) == 0) {
		TCB_t *value = (TCB_t *)GetAtIteratorFila2(sem->fila);
		DeleteAtIteratorFila2(sem->fila);
		return value;
	} else {
		return NULL;
	}
}

int blocked_list_remove(TCB_tid_waiting_t *thread) {
	if (FirstFila2(blocked_list) == 0) {
		do {
			TCB_tid_waiting_t *value = (TCB_tid_waiting_t *)GetAtIteratorFila2(blocked_list);
			if (value != NULL && value == thread) {
				return DeleteAtIteratorFila2(blocked_list);
			}
		} while (NextFila2(blocked_list) == 0);
		return -1;
	} else {
		return -1;
	}
}

int blocked_list_insert(TCB_tid_waiting_t *thread) {
	return AppendFila2(blocked_list, thread);
}

TCB_tid_waiting_t* blocked_list_thread_waiting_for(int tid) {
	if (FirstFila2(blocked_list) == 0) {
		do {
			TCB_tid_waiting_t *value = (TCB_tid_waiting_t *)GetAtIteratorFila2(blocked_list);
			if (value != NULL && value->waiting_for_thread_id == tid) {
				return value;
			}
		} while (NextFila2(blocked_list) == 0);
		return NULL;
	} else {
		return NULL;
	}
}

