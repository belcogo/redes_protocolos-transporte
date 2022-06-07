#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <netinet/sctp.h>
#include <sys/types.h>

#define ECHOMAX 1024
#define ECHOMIN 512

int SCTP_PORT = 4000;

struct client_args_struct {
	int sockfd;
	int PORT;
	char *server_addr_ip;
};

struct server_args_struct {
	int sockfd;
	int PORT;
	char *server_addr_ip;
	struct sockaddr_in addr;
	struct sctp_initmsg initmsg;
};

char *divider = "-----------------------------------------------------";

int *client(struct client_args_struct *args);
void *server(struct server_args_struct *args);
char *execute_command(char command[ECHOMAX]);
void *receive_thread(struct server_args_struct *args);
void *client_thread(struct client_args_struct *args);

void *client_thread(struct client_args_struct *args) {
	char response[ECHOMAX];
	sctp_recvmsg(args->sockfd, &response, sizeof(response), NULL, 0, 0, 0);
	printf("\n%s\nCONNECTION: %s:%d\n%s\n\n%s\n%s\n", divider, args->server_addr_ip, args->PORT, divider, response, divider);
	close(args->sockfd);
}

int main(int argc, char *argv[]) {
	int loc_sockfd;
	struct sockaddr_in loc_addr = {
		.sin_family = AF_INET, /* familia do protocolo */
		.sin_addr.s_addr = htonl(INADDR_ANY), /* endereco IP local */
		.sin_port = htons(SCTP_PORT), /* porta local */
	};

	struct sctp_initmsg sock_initmsg = {
		.sinit_num_ostreams = 5, /* Número de streams que se deseja mandar. */
		.sinit_max_instreams = 5, /* Número máximo de streams se deseja receber. */
		.sinit_max_attempts = 4, /* Número de tentativas até remandar INIT. */
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

  listen(loc_sockfd, sock_initmsg.sinit_max_instreams);

	struct server_args_struct server_conn = {
		.sockfd = loc_sockfd,
		.PORT = SCTP_PORT,
		.addr = loc_addr,
		.initmsg = sock_initmsg,
	};

	pthread_t tid;
	int i = 0;

	for (; i < argc - 1; i++) {
		pthread_create(&tid, 0, &receive_thread, (struct server_args_struct *)&server_conn); 
	}
	
	struct client_args_struct clients[argc];

	char linha[ECHOMAX];
	do {
		printf("\n> ");
		scanf(" %[^\n]", &linha);
		i = 0;
		for (; i < argc - 1; i++) {
			struct client_args_struct client_conn = {
				.PORT = 4000,
				.server_addr_ip = argv[i + 1],
			};
			int client_sock = client((struct client_args_struct *)&client_conn);
			sctp_sendmsg(client_sock, &linha, sizeof(linha), NULL, 0, 0, 0, 0, 0, 0);
			printf("Enviado");
			clients[i].sockfd = client_sock;
			clients[i].PORT = client_conn.PORT;
			clients[i].server_addr_ip = client_conn.server_addr_ip;
			pthread_create(&tid, 0, &client_thread, &clients[i]);
		}
		char *result = execute_command(linha);
		strcpy(linha, result);
		printf("\n%s\nLOCAL\n%s\n\n%s\n%s\n", divider, divider, linha, divider);		
	} while(1);

	close(loc_sockfd);
	return 0;
}

int *client(struct client_args_struct *args) {
	printf("Iniciando conexão...\n");
	char linha[ECHOMAX];

	int rem_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

  struct sockaddr_in rem_addr = {
		.sin_family = AF_INET, /* familia do protocolo*/
		.sin_addr.s_addr = inet_addr(args->server_addr_ip), /* endereco IP local */
		.sin_port = htons(args->PORT), /* porta local  */
	};

  if (connect(rem_sockfd, (struct sockaddr *) &rem_addr, sizeof(rem_addr)) < 0) {
		perror("Conectando stream socket");
		exit(1);
	}

	return rem_sockfd;
}

void *receive_thread(struct server_args_struct *args)
{
	while (1)
	{
		sleep(2);
		server(args);
	}
}

void *server(struct server_args_struct *args) {
  int tamanho;
	char linha[ECHOMAX];
	fd_set current_sockets, ready_sockets;

	int loc_sockfd = args->sockfd;
	struct sockaddr_in loc_addr = args->addr;
	struct sctp_initmsg sock_initmsg = args->initmsg;

	if (setsockopt (loc_sockfd, IPPROTO_SCTP, SCTP_INITMSG, &sock_initmsg, sizeof (sock_initmsg)) < 0){
		perror("setsockopt(sock_initmsg)");
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
				if (i == loc_sockfd) {
					int loc_newsockfd;
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
					sctp_sendmsg(i, &linha, sizeof(linha), NULL, 0, 0, 0, 0, 0, 0);

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
