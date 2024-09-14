Para rodar o projeto, certifiquesse que esta dentro da pasta do projeto `/trabalhoM1` E faça os seguintes passos

no terminal, rode os seguintes comandos

1. Execute o servidor

```bash
  make -f Makefile clean && make && ./build/server
```

2. execute em outro terminal os clientes se conectando e enviando requisicoes

```bash
  bash test_multiple_connections.sh
```