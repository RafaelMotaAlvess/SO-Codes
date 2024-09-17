#!/bin/bash

NUM_CLIENTS=500
WORD="word_to_upper_case"  # Defina a palavra que você quer enviar

# Medir o tempo de execução das conexões
time {
  for ((i = 1; i <= NUM_CLIENTS; i++))
  do
    echo "$WORD" | ./client_string &
    echo $i | ./client_number &
  done
  wait  # Espera que todos os clientes finalizem
}