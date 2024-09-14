#include <pthread.h>
#include <stdio.h>

#include <stdlib.h>

// gcc -o exer1_out exer1.c -l pthread


int vetor[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int resultado = 1;
int resultado2 = 0;

void *produto(){
    for(int i = 0; i < 10; i++){
        resultado *= vetor[i];
    }
    pthread_exit(0);
}

void *soma(){
    for(int i = 0; i < 10; i++){
        resultado2 += vetor[i];
    }
    pthread_exit(0);
}


// 2) realizar a soma e o produto ao mesmo tempo na thread.;

int main(int argc, char *argv[]){
    pthread_t tid, tid2;

    pthread_attr_t attr;

    pthread_attr_init(&attr);

    pthread_create(&tid, &attr, produto, NULL);
    pthread_create(&tid2, &attr, soma, NULL);
    pthread_join(tid, NULL);
    pthread_join(tid2, NULL);

    printf("Resultado produto: %d \n", resultado);
    printf("Resultado soma: %d \n", resultado2);
}
