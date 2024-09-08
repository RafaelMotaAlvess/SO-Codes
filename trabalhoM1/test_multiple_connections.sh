#!/bin/bash

# Testa múltiplas conexões para o socket de string
for i in {1..10}; do
    (
        echo "Conexão de string $i" | nc -U -q 1 /tmp/pipestring
    ) &
done

# Testa múltiplas conexões para o socket de números
for i in {1..10}; do
    (
        echo "$i" | nc -U -q 1 /tmp/pipenumber
    ) &
done

# Aguarda o término de todas as conexões em segundo plano
wait
echo "Testes concluídos"
