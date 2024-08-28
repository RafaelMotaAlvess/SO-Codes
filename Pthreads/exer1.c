#include <pthread.h>
#include <stdio.h>

#include <stdlib.h>

// gcc -o exer1_out exer1.c -l pthread

int input;
int sum = 5;

void *readInput(){
    printf("Digite um inteiro: ");
    scanf("%d", &input);
    pthread_exit(0);
}

void *sumConst(){
    sum += input;
    pthread_exit(0);
}

void *printResult(){
    printf("Resultado: %d\n", sum);
    pthread_exit(0);
}

// O Problema da abordagem solicitada, é que como dependemos de colocar o join na frente de cada create, ele torna o codigo sequencial!

int main(int argc, char *argv[]){
    pthread_t tid, tid2, tid3;

    pthread_attr_t attr;

    pthread_attr_init(&attr);

    pthread_create(&tid, &attr, readInput, NULL);

    pthread_join(tid, NULL);

    pthread_create(&tid2, &attr, sumConst, NULL);

    pthread_join(tid2, NULL);

    pthread_create(&tid3, &attr, printResult, NULL);

    pthread_join(tid3, NULL);
}
