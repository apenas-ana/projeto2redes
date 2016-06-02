#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define telefone_tam	4
#define ip_tam		4
#define porta_tam	4
#define msg_tam		telefone_tam + ip_tam + porta_tam + 4

//thread cliente
pthread_t thread_modulo;

//mutex
pthread_mutex_t mutex;

void *Modulo(void *args);
/*
 * Cliente TCP
 */
main(argc, argv)
int argc;
char **argv;
{
    unsigned short port;       
    char sendbuf[msg_tam];              
    char recvbuf[12];              
    struct sockaddr_in client;     
    struct sockaddr_in server; 
    int s;  
    int ns;    
    int namelen;
    int len;
    int tc;
    
    char telefone[telefone_tam]; 
    char enderecoIP[ip_tam];
    char porta[porta_tam];
    /*
     * O primeiro argumento (argv[1]) é a porta
     * onde o servidor aguardará por conexões
     */
    if (argc != 2)
    {
        fprintf(stderr, "Use: %s porta\n", argv[0]);
        exit(1);
    }
    
    
    port = (unsigned short) atoi(argv[1]);

    /*
     * Cria um socket TCP (stream) para aguardar conexões
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(2);
    }

   /*
    * Define a qual endereço IP e porta o servidor estará ligado.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os endereços IP
    */
    server.sin_family = AF_INET;   
    server.sin_port   = htons(port);       
    server.sin_addr.s_addr = INADDR_ANY;

    /*
     * Liga o servidor à porta definida anteriormente.
     */
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
       perror("Bind()");
       exit(3);
    }

    /*
     * Prepara o socket para aguardar por conexões e
     * cria uma fila de conexões pendentes.
     */
    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }
    
    
     while(1)
    {
      /*
       * Aceita uma conexão e cria um novo socket através do qual
       * ocorrerá a comunicação com o cliente.
       */
      namelen = sizeof(client);
      
      if((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
      {
	perror("Accept()");
	exit(5);
      }
      
      tc = pthread_create(&thread_modulo, NULL, Modulo, (void *) &ns);
      
      if(tc)
      {
	printf("ERRO: impossivel criar um thread cliente\n");
	exit(-1);
      }
      
      pthread_detach(thread_modulo);
    }

    /* Fecha o socket aguardando por conexões */
    close(s);

    printf("Servidor terminou com sucesso.\n");
    exit(0);
}
    
   


void *Modulo(void *args)
{
	char telefone[telefone_tam];
	char enderecoIP[ip_tam];
	char porta[porta_tam];
	char recvbuf[msg_tam];  
	int ns;

	ns = *(int*)args;

	/* Recebe uma mensagem do cliente através do novo socket conectado */
	if(recv(ns, recvbuf, sizeof(recvbuf), 0) == -1)
	{
		perror("Recv()");
		exit(6);
	}
	
	sscanf(recvbuf,"%s %s %s", telefone, enderecoIP, porta);
	printf("Mensagem recebida do cliente: %s %s %s\n", telefone, enderecoIP, porta);
	
	/* Fecha o socket conectado ao cliente */
	close(ns);

	pthread_exit(NULL);  
}

