/*
//   Esse Teste cria uma sequencia de threads, cada uma possui uma secao critica
//   onde eh usado o mutex. alem de possuirem uma cedencia voluntaria.
//   O resultado eh a impressao sequencial inversa das threads 3, 2 e 1 pela suas prioridades
//   imprimem o valor passado por parametro de uma variavel que eh incrementada dentro
//   da secao critica.
*/

#include "../include/cthread.h"
#include <stdio.h>

csem_t mtx;

void func0(void *arg) {
    printf("Eu sou a thread #1 imprimindo %d e entrando na cwait\n", *((int *)arg));

    cwait(&mtx);
    (*(int *)arg)++;
    cyield();
    csignal(&mtx);
    printf("Fim da thread #1\n");
    return;
}

void func1(void *arg) {
    printf("Eu sou a thread #2 imprimindo %d e entrando na cwait\n", *((int *)arg));

    cwait(&mtx);
    (*(int *)arg)++;
    cyield();
    csignal(&mtx);
    printf("Fim da thread #2\n");
    return;
}

void func2(void *arg) {
    printf("Eu sou a thread #3 imprimindo %d e entrando na cwait\n", *((int *)arg));

    cwait(&mtx);
    (*(int *)arg)++;
    cyield();
    csignal(&mtx);
    printf("Fim da thread #3\n");
    return;
}

int main(int argc, char *argv[]) {
    int globalCounter = 0;

    csem_init(&mtx, 1);

    int id0 = ccreate((void*(*)(void*))func0, (void *)&globalCounter);
    int id1 = ccreate((void*(*)(void*))func1, (void *)&globalCounter);
    int id2 = ccreate((void*(*)(void*))func2, (void *)&globalCounter);


    cjoin(id1);
    cjoin(id0);
    cjoin(id2);

    printf("Eu sou a main voltando para terminar o programa\n");

    return 0;
}