//
// Created by Leonardo Dalcin on 4/12/18.
//

#ifndef SO_PT1_LIBCTHREADS_H

#define _XOPEN_SOURCE 600
#include <sys/_types/_ucontext.h>
#include "ucontext.h"

#define SO_PT1_LIBCTHREADS_H

typedef struct s_TCB {
    int tid; // identificador da thread
    int state; // estado em que a thread se encontra
// 0: Criação; 1: Apto; 2: Execução; 3: Bloqueado e 4: Término
    int prio; // Prioridade associada a thread (não usado nesta implementação)
    ucontext_t context; // contexto de execução da thread (SP, PC, GPRs e recursos)
} TCB;

int ccreate(void *(*start)(void *), void *arg, int prio);
//Parâmetros:
//        start: ponteiro para a função que a thread executará.
//arg: um parâmetro que pode ser passado para a thread na sua criação. (Obs.: é um único parâmetro. Se for necessário
//        passar mais de um valor deve-se empregar um ponteiro para uma struct)
//prio: NÃO utilizado neste semestre, deve ser sempre zero.
//Retorno:
//        Quando executada corretamente: retorna um valor positivo, que representa o identificador da thread criada
//Caso contrário, retorna um valor negativo.

//int cyield(void);
////Retorno:
////        Quando executada corretamente: retorna 0 (zero)
////Caso contrário, retorna um valor negativo.
//
//int cjoin(int tid);
////Parâmetros:
////        tid: identificador da thread cujo término está sendo aguardado.
////Retorno:
////        Quando executada corretamente: retorna 0 (zero)
////Caso contrário, retorna um valor negativo.
//
//int csuspend(int tid);
////Parâmetros:
////        tid: identificador da thread a ser suspensa.
////Retorno:
////        Quando executada corretamente: retorna 0 (zero)
////Caso contrário, retorna um valor negativo.
//
//int cresume(int tid);
////Parâmetros:
////        tid: identificador da thread que terá sua execução retomada.
////Retorno:
////        Quando executada corretamente: retorna 0 (zero)
////Caso contrário, retorna um valor negativo.
//
//typedef struct s_sem {
//    int count; // indica se recurso está ocupado ou não (livre > 0, ocupado ≤ 0)
//    PFILA2 fila; // ponteiro para uma fila de threads bloqueadas no semáforo.
//} csem_t;
//
//int csem_init (csem_t *sem, int count);
////Parâmetros:
////        sem: ponteiro para uma variável do tipo csem_t. Aponta para uma estrutura de dados que representa a variável semáforo.
////count: valor a ser usado na inicialização do semáforo. Representa a quantidade de recursos controlados pelo semáforo.
////Retorno:
////        Quando executada corretamente: retorna 0 (zero)
////Caso contrário, retorna um valor negativo.
//
//int cwait (csem_t *sem);
////Parâmetros:
////        sem: ponteiro para uma variável do tipo semáforo.
////Retorno:
////        Quando executada corretamente: retorna 0 (zero)
////Caso contrário, retorna um valor negativo.
//
//int csignal (csem_t *sem);
////Parâmetros:
////        sem: ponteiro para uma variável do tipo semáforo.
////Retorno:
////        Quando executada corretamente: retorna 0 (zero)
////Caso contrário, retorna um valor negativo.
//
//int cidentify (char *name, int size);
//Parâmetros:
//        name: ponteiro para uma área de memória onde deve ser escrito um string que contém os nomes dos componentes do
//grupo e seus números de cartão. Deve ser uma linha por componente.
//size: quantidade máxima de caracteres que podem ser copiados para o string de identificação dos componentes do
//grupo.
//Retorno:
//        Quando executada corretamente: retorna 0 (zero)
//Caso contrário, retorna um valor negativo.
#endif //SO_PT1_LIBCTHREADS_H
