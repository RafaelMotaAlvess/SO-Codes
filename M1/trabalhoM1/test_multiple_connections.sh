#!/bin/bash

# Início da medição de tempo com precisão
start=$(date +%s%N)

# Testa múltiplas conexões para o socket de string
for i in {1..50}; do
    (
        echo "Conexão de string $i" | nc -U -q 1 /tmp/pipestring
    ) &
done

# Testa múltiplas conexões para o socket de números
# for i in {1..50}; do
#     (
#         echo "$i" | nc -U -q 1 /tmp/pipenumber
#     ) &
# done

# Aguarda o término de todas as conexões em segundo plano
wait

# Fim da medição de tempo com precisão
end=$(date +%s%N)

# Calcula a diferença de tempo em segundos com precisão (nanosegundos para segundos)
runtime=$(echo "scale=3; ($end - $start) / 1000000000" | bc)

# Exibe o tempo de execução com precisão em segundos
echo "Tempo total de execução: $runtime segundos"
