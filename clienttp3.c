#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>

#define	SIZE_BUFFER 100	
#define	ERROR -1	


int connect_socket(char* addresse, int port);
int send_server(int sock, char *buffer);
int recv_server(int sock, char *buffer);


int connect_socket(char* addresse, int port)
{

	int sock = socket(AF_INET, SOCK_STREAM, 0); 
	
	if( sock == ERROR )
	{
		perror("socket()");
		exit(-1);
	}
		
	struct hostent* hostinfo = gethostbyname(addresse); 
	
	if (hostinfo == NULL)
	{
		perror("hostinfo");
		exit(-1);
	}
	
	struct sockaddr_in sin; 
	sin.sin_addr = *(struct in_addr*) hostinfo->h_addr; 
	sin.sin_port = htons(port); 
	sin.sin_family = PF_INET; 
	
	int b = connect(sock, (struct sockaddr*) &sin, sizeof(struct sockaddr)); 
	if (b == ERROR)
	{
		perror("connect()");
		exit(-1);
	}
	return sock;
}

int recv_server(int sock, char *buffer)
{
	int r = recv(sock, buffer, SIZE_BUFFER, 0);
	if( r < 0)
	{
		perror("recv()");
		exit(-1);
	}

	return r;
}

int send_server(int sock, char *buffer)
{
	int a = send(sock, buffer, SIZE_BUFFER, 0);
	if (a < 0)
	{
		perror("send()");
		exit(-1);
	}
	return a;
}

int main(int argc, char ** argv)
{
	if( argc != 4)
	{
		printf("erreur il n y a pas assez d'argument\n");	
		exit(-1);
	}

	char* pseudo = argv[1];
	char* adresse = argv[2];
	int port = atoi( argv[3]);
	printf("pseudo : %s adresse : %s port : %d\n", pseudo, adresse, port);
	
	int sock;
	sock = connect_socket( adresse, port);
	send_server(sock, pseudo);
	char buffer[SIZE_BUFFER];
	fd_set readfds;
	
	
	while(1)
	{
		memset(buffer, 0, SIZE_BUFFER);
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds); 
		FD_SET(sock, &readfds); 
		int selec = select(sock +1 , &readfds, NULL, NULL, NULL);
		if (selec == -1)
		{
			printf("erreur de select\n");
			exit(-1);
		}
		if ( FD_ISSET(STDIN_FILENO , &readfds))
		{
			fgets(buffer, SIZE_BUFFER, stdin); 
			buffer[strlen(buffer)-1]= '\0';
			send_server(sock, buffer);

		}
		if ( FD_ISSET(sock , &readfds) )
		{
			int rec= recv_server(sock,buffer);
			printf("%s\n",buffer);
			if(rec==0)
			{
				close(sock);
				printf("Deconnecte\n");
			}
		}
	}
	return 0;
}


