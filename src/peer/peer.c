/*
Para compilar:
Precisa da bibliteca libsctp-dev
gcc sctpserver.c -o server -lsctp
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <netinet/sctp.h> // Novos includes
#include <sys/types.h>

#define ECHOMAX 1024
#define ECHOMIN 512

struct arg_struct {
    int sockfd;
		int PORT;
};

struct client_arg_struct {
		int PORT;
		char *server_addr_ip;
};

struct send_arg_struct {
		int sockfd;
		int PORT;
		char *server_addr_ip;
};

char *divider = "-----------------------------------------------------";

int *client(struct client_arg_struct *args);
void *server(struct arg_struct *args);
char *execute_command(char command[ECHOMAX]);
void *receive_thread(struct arg_struct *args);

int main(int argc, char *argv[]) {
	int loc_sockfd;
	struct sockaddr_in loc_addr = {
		.sin_family = AF_INET, /* familia do protocolo */
		.sin_addr.s_addr = htonl(INADDR_ANY), /* endereco IP local */
		.sin_port = htons(4000), /* porta local */
		//.sin_zero = 0, /* por algum motivo pode se botar isso em 0 usando memset() */
	};

  loc_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP); // Mudança protocolo '0' => 'IPPROTO_SCTP'

  if (loc_sockfd < 0) {
		perror("Criando stream socket");
		exit(1);
	}

  if (bind(loc_sockfd, (struct sockaddr *) &loc_addr, sizeof(struct sockaddr)) < 0) {
		perror("Ligando stream socket");
		exit(1);
	}

  listen(loc_sockfd, 5); // Mudado para initmsg.sinit_max_instreams.

	struct arg_struct server_conn = {
		.sockfd = loc_sockfd,
		.PORT = 4000,
	};
	pthread_t tid;
	struct arg_struct servers[argc];
	int i = 0;

	for (; i < argc - 1; i++) {
		pthread_create(&tid, 0, &receive_thread, (struct arg_struct *)&server_conn); 
	}
	
	int prosseguir;
  printf("Iniciar conexão?\n1 -> Sim\n0 -> Não\n");
	scanf("%d", &prosseguir);

	if (prosseguir == 0) {
		printf("Encerrando conexão.\n");
		exit(1);
	}

	printf("Iniciando conexão.\n");
	
	struct send_arg_struct clients[argc];
	i = 0;
	for (; i < argc - 1; i++) {
		struct client_arg_struct client_conn = {
			.PORT = 4000,
			.server_addr_ip = argv[i + 1],
		};
		int client_sock = client((struct client_arg_struct *)&client_conn);
		clients[i].sockfd = client_sock;
		clients[i].PORT = client_conn.PORT;
		clients[i].server_addr_ip = client_conn.server_addr_ip;
	}

	char linha[ECHOMAX];
	do {
		printf("> ");
		scanf("%s", &linha);
		i = 0;
		for (; i < argc - 1; i++) {
			sctp_sendmsg(clients[i].sockfd, &linha, sizeof(linha), NULL, 0, 0, 0, 0, 0, 0);
			char response[ECHOMAX];
			sctp_recvmsg(clients[i].sockfd, &response, sizeof(response), NULL, 0, 0, 0);
			printf("\n%s\nCONNECTION: %s:%d\n%s\n\n%s\n%s\n", divider, clients[i].server_addr_ip, clients[i].PORT, divider, response, divider);
		}
		char *result = execute_command(linha);
		strcpy(linha, result);
		printf("\n%s\nLOCAL\n%s\n\n%s\n%s\n", divider, divider, linha, divider);
		
	} while(1);
	
	for (; i < argc - 1; i++) {
		close(clients[i].sockfd);
	}

	return 0;
}

int *client(struct client_arg_struct *args) {
	printf("Iniciando conexão...\n");
	char linha[ECHOMAX];

	int rem_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

  struct sockaddr_in rem_addr = {
		.sin_family = AF_INET, /* familia do protocolo*/
		.sin_addr.s_addr = inet_addr(args->server_addr_ip), /* endereco IP local */
		.sin_port = htons(args->PORT), /* porta local  */
	};

	printf("> Conectando no servidor '%s:%d'\n", args->server_addr_ip, args->PORT);

  if (connect(rem_sockfd, (struct sockaddr *) &rem_addr, sizeof(rem_addr)) < 0) {
		perror("Conectando stream socket");
		exit(1);
	}

	return rem_sockfd;
}

void *receive_thread(struct arg_struct *args)
{
	while (1)
	{
		sleep(2);
		server(args);
	}
}

void *server(struct arg_struct *args) {
  int tamanho;
	char linha[ECHOMAX];
	fd_set current_sockets, ready_sockets;

	int loc_sockfd = args->sockfd;

	struct sctp_initmsg initmsg = {
		.sinit_num_ostreams = 5, /* Número de streams que se deseja mandar. */
		.sinit_max_instreams = 5, /* Número máximo de streams se deseja receber. */
		.sinit_max_attempts = 4, /* Número de tentativas até remandar INIT. */
		/*.sinit_max_init_timeo = 60000, Tempo máximo em milissegundos para mandar INIT antes de abortar. Default 60 segundos.*/
	};

  struct sockaddr_in loc_addr = {
		.sin_family = AF_INET, /* familia do protocolo */
		.sin_addr.s_addr = htonl(INADDR_ANY), /* endereco IP local */
		.sin_port = htons(args->PORT), /* porta local */
	};

	if (setsockopt (loc_sockfd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof (initmsg)) < 0){
		perror("setsockopt(initmsg)");
		exit(1);
  }

	FD_ZERO(&current_sockets);
	FD_SET(loc_sockfd, &current_sockets);
	int k = 0;
	while (1) {
		k++;
		ready_sockets = current_sockets;

		if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
			perror("ERRO :(");
			exit(EXIT_FAILURE);
		}

		int i = 0;
		for (i; i < FD_SETSIZE; ++i) {
			if (FD_ISSET(i, &ready_sockets)) {
				int loc_newsockfd;
				if (i == loc_sockfd) {
					tamanho = sizeof(struct sockaddr_in);
					if ((loc_newsockfd = accept(loc_sockfd, (struct sockaddr *)&loc_addr, &tamanho)) < 0) {
						perror("accept falhou :(");
						exit(EXIT_FAILURE);
					}
					FD_SET(loc_newsockfd, &current_sockets);
				} else {
					sctp_recvmsg(i, &linha, sizeof(linha), NULL, 0, 0, 0);
					printf("\n%s\nComando: %s\n%s\n", divider, linha, divider);
					char *result = execute_command(linha);
					strcpy(linha, result);
					sctp_sendmsg(loc_newsockfd, &linha, sizeof(linha), NULL, 0, 0, 0, 0, 0, 0);

					FD_CLR(i, &current_sockets);
				}
			}
		}

		if (k == (FD_SETSIZE * 2))
				break;
	}
}

char *execute_command(char command[ECHOMAX]) {
	FILE *fp;
	char buffer[ECHOMAX];
	fp = popen(command, "r");
	int i = 0;
	while (1)
	{
		buffer[i] = fgetc(fp); // reading the file
		if (buffer[i] == EOF) break;
		++i;
	}
	pclose(fp);
		
	return buffer;
}
