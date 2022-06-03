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

#include <netinet/sctp.h> // Novos includes
#include <sys/types.h>

#define ECHOMAX 1024

void client(int argc, char *argv[]);
void server(int argc, char *argv[]);
// char **receive_command(char command[ECHOMAX]);
// void **receive_message(char message[ECHOMAX]);

void client(int argc, char *argv[]) {
  int rem_sockfd;
	char linha[ECHOMAX];

  struct sockaddr_in rem_addr = {
		.sin_family = AF_INET, /* familia do protocolo*/
		.sin_addr.s_addr = inet_addr(argv[2]), /* endereco IP local */
		.sin_port = htons(atoi(argv[1])), /* porta local  */
		//sin_zero = 0, /* por algum motivo pode se botar isso em 0 usando memset() */
	};

  rem_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	if (rem_sockfd < 0) {
		perror("Criando stream socket");
		exit(1);
	}

	printf("> Conectando no servidor '%s:%s'\n", argv[2], argv[1]);

  if (connect(rem_sockfd, (struct sockaddr *) &rem_addr, sizeof(rem_addr)) < 0) {
		perror("Conectando stream socket");
		exit(1);
	}

  do {
    fgets (linha, ECHOMAX, stdin);
    linha[strcspn(linha, "\n")] = 0;

    sctp_sendmsg(rem_sockfd, &linha, sizeof(linha), NULL, 0, 0, 0, 0, 0, 0);

    sctp_recvmsg(rem_sockfd, &linha, sizeof(linha), NULL, 0, 0, 0);
		printf("Recebi %s\n", linha);
  } while(strcmp(linha,"exit"));

  close(rem_sockfd);
}

void server(int argc, char *argv[]) {
  int loc_sockfd, loc_newsockfd, tamanho;
	char linha[ECHOMAX];

  struct sockaddr_in loc_addr = {
		.sin_family = AF_INET, /* familia do protocolo */
		.sin_addr.s_addr = INADDR_ANY, /* endereco IP local */
		.sin_port = htons(atoi(argv[1])), /* porta local */
		//.sin_zero = 0, /* por algum motivo pode se botar isso em 0 usando memset() */
	};

  struct sctp_initmsg initmsg = {
		.sinit_num_ostreams = 5, /* Número de streams que se deseja mandar. */
  		.sinit_max_instreams = 5, /* Número máximo de streams se deseja receber. */
  		.sinit_max_attempts = 4, /* Número de tentativas até remandar INIT. */
  		/*.sinit_max_init_timeo = 60000, Tempo máximo em milissegundos para mandar INIT antes de abortar. Default 60 segundos.*/
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

  if (setsockopt (loc_sockfd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof (initmsg)) < 0){
		perror("setsockopt(initmsg)");
		exit(1);
  }

  listen(loc_sockfd, initmsg.sinit_max_instreams); // Mudado para initmsg.sinit_max_instreams.
	printf("> aguardando conexao\n");

  tamanho = sizeof(struct sockaddr_in);
  loc_newsockfd = accept(loc_sockfd, (struct sockaddr *)&loc_addr, &tamanho);

  do {
    sctp_recvmsg(loc_newsockfd, &linha, sizeof(linha), NULL, 0, 0, 0);
		printf("Recebi %s\n", linha);

    sctp_sendmsg(loc_newsockfd, &linha, sizeof(linha), NULL, 0, 0, 0, 0, 0, 0);
		printf("Renvia %s\n", linha);
  } while(strcmp(linha,"exit"));

  close(loc_sockfd);
	close(loc_newsockfd);
}

int main(int argc, char *argv[]) {
  char port = argv[1];
  char server_ip = argv[2];

  client(argc, argv);
  server(argc, argv);
}
