#!/bin/bash

# Início da medição de tempo com precisão
start=$(date +%s%N)

# Função para conexões de string
test_string_connections() {
    for i in {1..50}; do
        (
            echo "Conexão de string $i" | nc -U -q 1 /tmp/pipestring
        ) &
    done
}

# Função para conexões de número
test_number_connections() {
    for i in {1..50}; do
        (
            echo "$i" | nc -U -q 1 /tmp/pipenumber
        ) &
    done
}

# Executa as conexões de string e número ao mesmo tempo
test_string_connections &
test_number_connections &

# Aguarda o término de todas as conexões em segundo plano
wait

# Fim da medição de tempo com precisão
end=$(date +%s%N)

# Calcula a diferença de tempo em segundos com precisão (nanosegundos para segundos)
runtime=$(echo "scale=3; ($end - $start) / 1000000000" | bc)

# Exibe o tempo de execução com precisão em segundos, incluindo 0 antes do ponto decimal
printf "Tempo total de execução: %.3f segundos\n" $runtime
