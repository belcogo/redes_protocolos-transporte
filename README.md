# Projeto do GB de RedesI: Aplicação e Transporte

### Pré-requisitos
1. Rodar num ambiente linux ou possuir docker instalado na máquina

### Pré-requisitos no ambiente linux
1. Possuir a lib `libsctp-dev` instalada
```
apt-get update
apt-get install -y libsctp-dev
```

### Instruções para rodar no ambiente linux
1. Baixe o repositório
2. Acesse a pasta `src/peer/` pelo terminal
3. Execute o comando `gcc -pthread -o <nome_do_programa> peer.c -lsctp`
4. Quando o processo for finalizado, execute o comando `./<nome_do_programa> <Server1_IP> <Server2_IP> ... <ServerN_IP>`.

Repita os 4 primeiros passos em outra máquina/ambiente remoto.

5. Depois de rodar o programa nas máquinas/ambientes desejados, envie comandos como `ls`, `hostname -i`, `ps aux` de um para outro. Deve ser apresentado no terminal o resultado da execução do comando localmente e nos outros peers conectados.

### Instruções para rodar com docker
1. Baixe o repositório
2. Acesse a pasta `src/peer/` pelo terminal
3. Execute o comando `docker build -t <nome_da_imagem> .`
4. Quando o processo for finalizado, execute o comando `docker run -d -t <nome_da_imagem>` (execute o run 3x para gerar 3 containers)
5. Depois recupere o nome do container criado (`docker ps` lista os containers que estão rodando)
6. Com o nome do container execute o comando `docker exec -it <nome_do_container> bash` para acessar o terminal do container. Faça isso para cada container rodando
7. execute o comando `./peer <Server1_IP> <Server2_IP> ... <ServerN_IP>`. Faça isso para cada container rodando
8. Envie um comando como `ls`. Deve ser apresentado no terminal o resultado da execução do comando localmente e nos outros peers conectados.

---------------------------------------------------------------------------------------------------------------------------------------------------------

**Objetivo**

Implementar uma comunicação peer2peer entre, no mínimo 3 peers utilizando um protocolo de transporte diferente de UDP ou TCP.

Protocolo escolhido: SCTP

**Enunciado**
Projetar e implementar um sistema de terminal distribuído com no mínimo três nodos
(peers). Cada peer da rede tem funções de servidor e cliente para utilizar comandos em
seus terminais. O objetivo é criar um software que permita a execução de comandos em
cada peer da rede. Por exemplo, quando for digitado o comando “ls” em um peer, esse
comando deve ser executado em todos os peers da rede e a visualização da resposta dos
terminais deve aparecer no peer que disparou o comando. A rede P2P pode ser estática,
isto é, as máquinas que irão pertencer a rede devem ser conhecidas.

**Aluna**
- Bel Cogo

**Professor**
- Cristiano Bonato Both

**Instituição**
- Universidade do Vale do Rio dos Sinos (UNISINOS)

____

### Códigos utilizados de referência
- Código da pasta `src/pratica_SCTP` (disponibilizado em aula)
