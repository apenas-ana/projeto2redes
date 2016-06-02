#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/signal.h>
#include <arpa/inet.h>
#include <pthread.h>

#define telefone_tam	4
#define ip_tam		4
#define porta_tam	4
#define msg_tam		telefone_tam + ip_tam + porta_tam + 4

int flag = 0;

struct Usuario{
	char telefone[telefone_tam]; 
	char enderecoIP[ip_tam];
	char porta[porta_tam];
	int status; // online =1 e offline =0
	struct Usuario *prox;
}; 
typedef struct Usuario usuario;

usuario *LISTA;

//thread cliente
pthread_t thread_servidor;

//mutex
pthread_mutex_t mutex;

void *Servidor(void *args);
void insereFim(usuario *LISTA, char tel[], char ip[], char port[]);
int vazia(usuario *LISTA);
void exibe(usuario *LISTA);

/*
 * Servidor TCP
 */
main(argc, argv)
int argc;
char **argv;
{
    unsigned short port;       
    char sendbuf[12];                          
    struct sockaddr_in client; 
    struct sockaddr_in server; 
    int s;                     /* Socket para aceitar conexões       */
    int ns;                    /* Socket conectado ao cliente        */
    int namelen;
    int tc;
    int teste = 2;

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
    
    LISTA = (usuario *) malloc(sizeof(usuario));
    LISTA->prox = NULL;	
    
    
    pthread_mutex_init(&mutex, NULL);
    
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
      
      tc = pthread_create(&thread_servidor, NULL, Servidor, (void *) &ns);
      
      if(tc)
      {
	printf("ERRO: impossivel criar um thread cliente\n");
	exit(-1);
      }
      
      pthread_detach(thread_servidor);
    }

    /* Fecha o socket aguardando por conexões */
    close(s);

    printf("Servidor terminou com sucesso.\n");
    exit(0);
}


/*==================================================Thread Servidor==================================================*/
void *Servidor(void *args)
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
	
	pthread_mutex_lock(&mutex);
	
	sscanf(recvbuf,"%s %s %s", telefone, enderecoIP, porta);
	printf("Mensagem recebida do cliente: %s %s %s\n", telefone, enderecoIP, porta);
	
	insereFim(LISTA, telefone, enderecoIP, porta);
		
	exibe(LISTA);
	
	pthread_mutex_unlock(&mutex);

	/* Fecha o socket conectado ao cliente */
	close(ns);

	pthread_exit(NULL);
}

void insereFim(usuario *LISTA, char tel[], char ip[], char port[])
{
	usuario *novo=(usuario *) malloc(sizeof(usuario));
	if(!novo){
		printf("Sem memoria disponivel!\n");
		exit(1);
	}
	
	strcpy(novo->telefone, tel);
	strcpy(novo->enderecoIP, ip);
	strcpy(novo->porta, port);
	novo->status = 1;
	
	novo->prox = NULL;
	
	if(vazia(LISTA))
		LISTA->prox=novo;
	else{
		usuario *tmp = LISTA->prox;
		
		while(tmp->prox != NULL)
			tmp = tmp->prox;
		
		tmp->prox = novo;
	}
}


int vazia(usuario *LISTA)
{
	if(LISTA->prox == NULL)
		return 1;
	else
		return 0;
}


void exibe(usuario *LISTA)
{
	if(vazia(LISTA)){
		printf("Lista vazia!\n\n");
		return ;
	}
	
	usuario *tmp;
	tmp = LISTA->prox;
	
	while( tmp != NULL){
		printf("%s\n", tmp->telefone);
		tmp = tmp->prox;
	}
	printf("\n\n");
}


