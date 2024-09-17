
### Instruções para executar o projeto

Certifique-se de estar no diretório correto do projeto: `M1/trabalhoM1`. Uma vez dentro da pasta, siga os passos abaixo para rodar o projeto corretamente.

#### Passo 1: Executar o servidor

No terminal, execute o comando abaixo para compilar o projeto e iniciar o servidor:

```bash
make -f Makefile clean && make && ./build/server
```

Esse comando irá:
- Limpar qualquer build anterior.
- Compilar o projeto.
- Iniciar o servidor a partir do executável gerado.

#### Passo 2: Executar os clientes e enviar requisições

Abra outro terminal e execute um dos seguintes scripts para conectar os clientes ao servidor e enviar as requisições:

- Para criar conexões manuais via *sockets*:

  ```bash
  bash test_multiple_connections.sh
  ```

- Para criar conexões automáticas usando os códigos dos clientes `client_number` e `client_string`:

  ```bash
  bash test_clients.sh
  ```