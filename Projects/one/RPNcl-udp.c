#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define RCVBUFSIZE 32               /* Size of receiving buffer */
#define STACKSIZE 10                /* Size of stack used for calculations */
int sock;                           /* Socket descriptor */
struct sockaddr_in servAddr;        /* Server address */
struct sockaddr_in fromAddr;        /* Source address */

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void calculate(char *data)
{
    int value1, value2;
    char toSend[32];
    unsigned int toSendLen, fromSize, respStringLen;
    char buffer[RCVBUFSIZE];
    char *token;

    /* Stack Initialization */
    int *stack;
    int top;
    stack = malloc(STACKSIZE*sizeof(int));
    top = 0;

    token = strtok(data, " ");
    while(token != NULL)
    {
        if(strpbrk(token, "+-*/") != NULL) 
        { /* Operator. Pop off 2 values & get help from server */
            if(top < 2)
                DieWithError("Malformed RPN String");
            value2 = stack[--top];
            value1 = stack[--top];
            sprintf(toSend, "%d %d %s", value1, value2, token);
            toSendLen = strlen(toSend);

            /* Send string to server */
            if (sendto(sock, toSend, toSendLen, 0, (struct sockaddr *)
                       &servAddr, sizeof(servAddr)) != toSendLen)
                DieWithError("sendto() sent a different number of bytes than expected");
          
            /* Now get the answer from the server */
            fromSize = sizeof(fromAddr);
            if ((respStringLen = recvfrom(sock, buffer, RCVBUFSIZE, 0, 
                                         (struct sockaddr *) &fromAddr, &fromSize)) == -1)
                DieWithError("recvfrom() failed");

            if (servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
                DieWithError("Error: received a packet from unknown source.\n");

            /* Push value onto stack and continue */
            stack[top++] = atoi(buffer);
        }
        else 
        { /* Number. Push it onto the stack */
            if(top == STACKSIZE)
                DieWithError("Stack Overflow");
            stack[top++] = atoi(token);
        }
        token = strtok(NULL, " ");
    }
    if(top > 1)
        DieWithError("Malformed RPN String");
    printf("%d\n", stack[0]);
}

int main(int argc, char *argv[])
{
    unsigned short servPort;            /* Echo server port */
    char *servIP;                       /* IP address of server */
    char *string;                       /* String to send to echo server */

    if(argc != 4)    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP> <Server Port> <RPN String>\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];           /* First arg: server IP address (dotted quad) */
    servPort = atoi(argv[2]);    /* Second arg: server port */
    string = argv[3];           /* Third arg: RPN string */

    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&servAddr, 0, sizeof(servAddr));    /* Zero out structure */
    servAddr.sin_family = AF_INET;                 /* Internet addr family */
    servAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    servAddr.sin_port   = htons(servPort);     /* Server port */

    calculate(string);

    close(sock);
    exit(0);
}

