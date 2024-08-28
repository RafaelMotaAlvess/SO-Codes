Compilar com o GCC -> gcc -o **<nome_do_output>** **<arquivo>**
executar o codigo -> ./<nome_do_output> // necessita estar no diretorio ou colocar o caminho. 
visualizar os processos -> htop

tentar no fluxo de socket enviar dados inteiros e floats
tentar colocar a comunicação dos pipes constantes. 

=====================

concorencia -> correr pelo mesma chegada
parelelismo -> correr dois ao mesmo tempo

paralelismo de dados -> distribui subconjuntos dos mesmos dados em varios nucles na mesma operação. se separa para fazer mais rapido o calculo do mesmo dado.
    - quebra o vetor em duas partes e cada parte soma o vetor.

paralelismo de tarefas -> distribui threads entre nucleos, cada thread executando uma operação exclusiva.
    - uma é responsavel por escrever e a outra por pintar.

somar todos os valores do vetor em um unico valor escalar 


Antes do processo se dividir, ele era sequencial.

Th1 -> Parte 1 - [1,2,3,4]
Th2 -> Parte 2 - [5,6,7,9]

Após a soma de cada thread, as duas precisam se somar, logo se torna sequencial.

nenhum código consegue ser completamente paralelo! 

