#include <pthread.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>

// gcc -o exer3_out exer3.c -l pthread


// 3) Implemente um sistema em que há três threads que tem a responsabilidade de ler o valor de um sensor 
// e some ao valor de uma variável global e em uma variável local. Você deve simular a contagem com operação
//  de incremento simples (+=1 ou ++). Print a variável local a cada 1 segundo e incremente a variável a cada 1 segundo. 
// O programa deve chegar ao fim quando a soma da variável global chegar a 100. Ao fim, você consegue observar qual condição:


int global = 0;

void *sensor(){
    int local = 0;
    while(global < 100){
        global++;
        local++;
        printf("Valor local: %d \n", local);
        sleep(1);
    }
    pthread_exit(0);
}


int main(int argc, char *argv[]){
    pthread_t tid[3];

    pthread_attr_t attr;
    pthread_attr_init(&attr);


    for(int i=0; i < 3; i++){
        pthread_create(&tid[i], &attr, sensor, NULL);
    }
  

    for(int i=0; i < 3; i++){
        pthread_join(tid[i], NULL);
    }

    printf("Valor global: %d \n", global);
}

// a. todas as threads tem o mesmo valor na variavel interna? 
// R: Não, uma das threads no final tem o valor 34, uma das threads acaba antes encerrando o ciclo da funcao.

// b. O print da variavel global segue um incremento linear?
// Sim, a thread incrementa em um incremento linear pois todas as threads sempre estão aumentando em 1 a variavel global!