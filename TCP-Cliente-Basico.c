#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define telefone_tam	4
#define ip_tam		4
#define porta_tam	4
#define msg_tam		telefone_tam + ip_tam + porta_tam + 4


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
    struct hostent *hostnm;    
    struct sockaddr_in server; 
    int s;     
    int len;
    
    char telefone[telefone_tam]; 
    char enderecoIP[ip_tam];
    char porta[porta_tam];

    /*
     * O primeiro argumento (argv[1]) é o hostname do servidor.
     * O segundo argumento (argv[2]) é a porta do servidor.
     */
    if (argc != 3)
    {
        fprintf(stderr, "Use: %s hostname porta\n", argv[0]);
        exit(1);
    }

    /*
     * Obtendo o endereço IP do servidor
     */
    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short) atoi(argv[2]);

    /*
     * Define o endereço IP e a porta do servidor
     */
    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    /*
     * Cria um socket TCP (stream)
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(3);
    }

    /* Estabelece conexão com o servidor */
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connect()");
        exit(4);
    }
    
    
    printf("Iniciando modulo de usuario...\n");
    printf("Telefone: ");
    __fpurge(stdin);
    fgets(telefone, sizeof(telefone), stdin);
    printf("Endereco IP: ");
    __fpurge(stdin);
    fgets(enderecoIP, sizeof(enderecoIP), stdin);
    printf("Porta: "); 
    __fpurge(stdin);
    fgets(porta, sizeof(porta), stdin);

    len = strlen(telefone);
    /*retirando \n do final da string*/
    if(telefone[len-1] == '\n')
      telefone[len-1] = 0;

    len = strlen(enderecoIP);
    /*retirando \n do final da string*/
    if(enderecoIP[len-1] == '\n')
      enderecoIP[len-1] = 0;
    
    sprintf(sendbuf, "%s %s %s", telefone, enderecoIP, porta);

       
    if (send(s, sendbuf, strlen(sendbuf)+1, 0) < 0)
    {
        perror("Send()");
        exit(5);
    }
    printf("Mensagem enviada ao servidor: %s\n", sendbuf);
    
    /*
    
    if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0)
    {
        perror("Recv()");
        exit(6);
    }
    printf("Mensagem recebida do servidor: %s\n", recvbuf);
    */

    /* Fecha o socket */
    close(s);

    printf("Cliente terminou com sucesso.\n");
    exit(0);

}


