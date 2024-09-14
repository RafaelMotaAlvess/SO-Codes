#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

//#include "sensor.h"

// compilar usando: gcc -o thr_mutex thr_mutex.c -lpthread

pthread_mutex_t exclusao_mutua = PTHREAD_MUTEX_INITIALIZER; //LOCK da seção crítica

double sensor_lido = 0;

//double limite_atual = HUGE_VAL;

void atualiza_sensor();
void atualiza_display();

int main(){
   pthread_t t1, t2;

   
   pthread_create(&t1, NULL, (void *) atualiza_sensor, NULL); /*criando a thread para a sensor 1*/
   pthread_create(&t2, NULL, (void *) atualiza_display, NULL); /*criando a thread para atualizar o display*/

   /*thread principal espera as threas criadas retornarem para encerrar o processo*/
   pthread_join(t1, NULL);
   pthread_join(t2, NULL);

   return 0;
}

/*função para a thread realizar a atualização de itens detectados*/
void atualiza_sensor(){
   double lido = 0;
	   while(1){
			pthread_mutex_lock( &exclusao_mutua); /*Trava a seção critica*/
      printf("\nIn - Lido 1: %d", (int)sensor_lido);
      /*Seção critica - variável global que armazena todo o valor de detecções no sensor
      a atribuição de "lido" é para simular o dado que é lido dos sensores */
      sensor_lido += lido;//uart1_get_value() 

      pthread_mutex_unlock( &exclusao_mutua);/*destrava a seção critica*/
      printf("\nOut - Lido 1: %d", (int)sensor_lido);

      /*Simula o intervalo de tempo que itens passão pelo sensor da esteira*/
      usleep(1000 * 1000); 

      /*Simula diferentes quantidades de itens passando ao mesmo tempo pelo sensor da esteira*/
      lido++;
   }
}

void atualiza_display(){ 
   while(1){
      pthread_mutex_lock( &exclusao_mutua); /*Trava a seção critica*/

      /*Seção critica - variável global que armazena todo o valor de detecções nas três esteiras
      o printf() simula a atualização do display */
      printf("\nPeso total: %lf", sensor_lido);  

      pthread_mutex_unlock( &exclusao_mutua);/*destrava a seção critica*/

      /* tempo para ataulizar o display e que é perceptível ao olho humano */
      usleep(100 * 1000);            
   }
}