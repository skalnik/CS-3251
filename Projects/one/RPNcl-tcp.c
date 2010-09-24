/* Based on http://cs.baylor.edu/~donahoo/practical/CSockets/textcode.html TCP Echo Client */
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define RCVBUFSIZE 32   /* Size of receive buffer */
#define STACKSIZE 10    /* Size of stack used for calculations */
int sock;               /* Socket descriptor */
void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void calculate(char *data)
{
    int value1, value2;             /* Values popped off stack */
    char toSend[32];                /* String to be sent to server */
    unsigned int toSendLen;         /* Length of string to be sent */
    char buffer[RCVBUFSIZE];        /* Receiving buffer */
    char *token;                    /* Token used for string tokenization */
    
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
            if(send(sock, toSend, toSendLen, 0) != toSendLen)
                DieWithError("send() sent a different number of bytes than expected!");
            
            /* Now get the answer from the server */
            if(recv(sock, buffer, RCVBUFSIZE-1, 0) <= 0)
                DieWithError("recv() failed or connection closed prematurely!");

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
    /* Send -1 to close connection on server */
    if(send(sock, "-1", 2, 0) != 2)
        DieWithError("send() sent a different number of bytes than expected!");
    printf("%d\n", stack[0]);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in servAddr; /* Server address */
    unsigned short servPort;     /* Server port */
    char *servIP;                /* Server IP address (dotted quad) */
    char *string;                /* String to send to server */


    if (argc != 4)    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server IP> <Server Port> <RPN String>\n",
               argv[0]);
       exit(1);
    }

    servIP = argv[1];          /* First arg: server IP address (dotted quad) */
    servPort = atoi(argv[2]);  /* Second arg: server port */
    string = argv[3];          /* Third arg: string to send to server */

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&servAddr, 0, sizeof(servAddr));     /* Zero out structure */
    servAddr.sin_family      = AF_INET;             /* Internet address family */
    servAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    servAddr.sin_port        = htons(servPort); /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("connect() failed");
    
    calculate(string);

    close(sock);
    exit(0);
}
