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
#define MAX_CLIENT  30
#define SIZE_PSEUDO 20


int listen_socket(int port);
int recv_client(int sock, char *buffer);
int send_client(int csock, char *buffer);

/*test coment2*/
int listen_socket(int port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0); 
	if( sock == ERROR ) 
	{
		perror("socket()");
		exit(-1);
	}
		
	struct sockaddr_in sin;
	sin.sin_addr.s_addr = htonl(INADDR_ANY); 
	sin.sin_port = htons(port);
	sin.sin_family = AF_INET;
	
	int a = bind (sock, (struct sockaddr*) &sin, sizeof(sin)); 
	if (a == ERROR)
	{
		perror("bind()");
		exit(-1);
	}
	
	
	int b = listen(sock, 1); 
	if ( b == ERROR )
	{
		perror("listen()");
		exit(-1);
	}

	return sock;
}


int recv_client(int csock, char *buffer)
{
	int r = recv(csock, buffer, SIZE_BUFFER, 0);
	if( r < 0)
	{
		perror("recv()");
		exit(-1);
	}
	return r;
}


int send_client(int csock, char *buffer)
{
	int s = send(csock, buffer, SIZE_BUFFER, 0);
	if( s < 0 )
	{
		perror("send()");
		exit(-1);
	}
	return s;
}


struct Client 
{
	char pseudo[SIZE_PSEUDO];
	int csock;	
};


void rmv_client(struct Client clients[MAX_CLIENT], int i_to_remove, int* nb_c)
{
	 int i;
	 close(clients[i_to_remove].csock);
	 for( i=i_to_remove; i < *nb_c-1; i++ )
	 { 
			 clients[i]=clients[i+1];
	 } 
	 (*nb_c)--;
}



struct Client add_client(int ssock, int* nb_c, int* max_fd)
{
	char buffer[SIZE_BUFFER];
	struct sockaddr_in csin; 
	int csock; 
	int csinsize = sizeof(csin);
	csock = accept(ssock, (struct sockaddr *)&csin, &csinsize); 

	struct Client new_client;
	new_client.csock = csock;
	*nb_c += 1;	

	if(*max_fd < new_client.csock)
	{
		*max_fd = new_client.csock;
	}

	int rec = recv_client(new_client.csock, buffer);
	strncpy(new_client.pseudo, buffer, SIZE_PSEUDO);

	return new_client;
}


int main(int argc, char ** argv)
{
	if( argc != 2)
	{
		printf("erreur il n y a pas assez d'argument\n");	
		exit(-1);
	}

	int port = atoi( argv[1] );

	struct Client tab_client[MAX_CLIENT];
	int nb_c = 0;
	int i,j,k;
	char buffer[SIZE_BUFFER]={0};

	int sock = listen_socket(port);
	printf("ecoute demarre\n");

	int max_fd = sock;
	fd_set readfds; 
	
	
	while(1)
	{
		FD_ZERO(&readfds);  		
		FD_SET(STDIN_FILENO, &readfds); 
		FD_SET(sock, &readfds); 		
		for(i=0; i<nb_c; i++) FD_SET(tab_client[i].csock, &readfds); 
		int selec = select( max_fd + 1, &readfds, NULL, NULL, NULL); 
		
		if (selec == -1)
		{
			printf("erreur de select\n");
			exit(-1);
		}
		
		if(FD_ISSET(sock , &readfds))
		{
			if(nb_c < MAX_CLIENT) 
				tab_client[nb_c] = add_client(sock, &nb_c, &max_fd);
			printf("connexion de %s\n", tab_client[nb_c-1].pseudo); 
		}

		for(i = 0; i < nb_c; i++)
		{
			if( FD_ISSET( tab_client[i].csock, &readfds))
			{
				memset(buffer, 0, SIZE_BUFFER);		
				int rec = recv_client( tab_client[i].csock, buffer);
				if(rec==0)
				{
					rmv_client(tab_client, i, &nb_c);
				}
				else
				{
					printf("I got this message from %s : %s\n",tab_client[i].pseudo,buffer);

					for(j = 0; j < nb_c; j++)
					{
						if( j != i)
						{
							char tab_buffer[SIZE_BUFFER] = {0};
							strcat(tab_buffer, tab_client[i].pseudo);
							strcat(tab_buffer, " : ");
							strcat(tab_buffer, buffer);
							int a= send_client(tab_client[j].csock, tab_buffer);
						}
					}
				}
			}
			char tabi[SIZE_BUFFER]={0};
			if ( FD_ISSET(STDIN_FILENO , &readfds) )			
			{
				memset(tabi,0,SIZE_BUFFER);
				fgets(tabi,SIZE_BUFFER,stdin);      
				buffer[strlen(tabi)-1]= '\0';
				for(k=0; k<nb_c;k++)
					send_client(tab_client[k].csock, tabi);   
			}
		}
	}
	return 0;
}	
