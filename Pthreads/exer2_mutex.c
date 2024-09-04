#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h> 

#define SIZE_VECTOR 10

pthread_mutex_t exclusao_mutua_soma = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t exclusao_mutua_mult = PTHREAD_MUTEX_INITIALIZER;

int vet[SIZE_VECTOR ] = {1,2,3,4,5,6,7,8,9,10};

int result_soma = 0;
int result_mult = 1;
int i_soma = 0;
int i_mult = 0;

int half_vector = (int)(SIZE_VECTOR / 2);

void *soma_1(void *param){  
    while(i_soma < SIZE_VECTOR){
        pthread_mutex_lock(&exclusao_mutua_soma);

        result_soma+=vet[i_soma];
        i_soma++;
        pthread_mutex_unlock(&exclusao_mutua_soma);
    }
}

void *soma_2(void *param){
    while(i_soma < SIZE_VECTOR){
        pthread_mutex_lock(&exclusao_mutua_soma);

        result_soma+=vet[i_soma];
        i_soma++;
        pthread_mutex_unlock(&exclusao_mutua_soma);
    }
}

void *mult_1(void *param){
    while(i_mult < SIZE_VECTOR){
        pthread_mutex_lock( &exclusao_mutua_mult);

        result_mult *= vet[i_mult];
        i_mult++;

        pthread_mutex_unlock(&exclusao_mutua_mult);
    }
}

void *mult_2(void *param){
    while(i_mult < SIZE_VECTOR){
        pthread_mutex_lock( &exclusao_mutua_mult);

        result_mult *= vet[i_mult];
        i_mult++;

        pthread_mutex_unlock(&exclusao_mutua_mult);
    }
}

int main(){ 
    pthread_t tid_soma_1, tid_soma_2, tid_mult_1, tid_mult_2;
    pthread_attr_t attr;

    printf("%d", half_vector);
    pthread_attr_init(&attr);

    pthread_create(&tid_soma_1, &attr, soma_1, NULL);
    pthread_create(&tid_soma_2, &attr, soma_2, NULL);
    pthread_create(&tid_mult_1, &attr, mult_1, NULL);
    pthread_create(&tid_mult_2, &attr, mult_2, NULL);

    pthread_join(tid_soma_1, NULL);
    pthread_join(tid_soma_2, NULL);
    pthread_join(tid_mult_1, NULL);
    pthread_join(tid_mult_2, NULL);

    printf("Soma resultande: %d\n", (result_soma));
    printf("Produto resultande: %d\n", (result_mult));

}