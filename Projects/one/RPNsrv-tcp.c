/* Based on http://cs.baylor.edu/~donahoo/practical/CSockets/textcode.html TCP Echo Server */
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define RCVBUFSIZE 32   /* Size of receive buffer */

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void HandleTCPClient(int clntSocket)
{
    char buffer[RCVBUFSIZE];        /* Buffer for receving string */
    int value1, value2, res;        /* Values used for calculations, and result */
    char *operator;                 /* Operation to be done on value1 & 2 */
    char *token;                    /* Used for string tokenization */
    int toSendLen;                  /* Length of string to send */
    char toSend[32];                /* Buffer for sending answer */
    int bytesRcvd;                  /* Number of bytes received */
    char *rcvd;                     /* String received */


    /* Receive message from client */
    if((bytesRcvd = recv(clntSocket, buffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

    rcvd = malloc(sizeof(char) * bytesRcvd);
    strncpy(rcvd, buffer, bytesRcvd);
    printf("Received: %s\n", rcvd);
    while(strcmp(rcvd, "-1") != 0)
    {
        /* String will always be 3 tokens. Two integers and an operator */
        token = strtok(rcvd, " ");
        value1 = atoi(token); token = strtok(NULL, " ");
        value2 = atoi(token); token = strtok(NULL, " ");
        operator = token;
        
        /* Figure out what the operator is and perform calculation */
        if(strcmp(token, "+") == 0)
            res = value1 + value2;
        else if(strcmp(token, "-") == 0)
            res = value1 - value2;
        else if(strcmp(token, "*") == 0)
            res = value1 * value2;
        else if(strcmp(token, "/") == 0)
            res = value1 / value2;
        
        /* Send result */
        printf("Sending: %d\n", res);
        sprintf(buffer, "%d", res);
        toSendLen = strlen(buffer);
        if (send(clntSocket, buffer, toSendLen, 0) != strlen(buffer))
           DieWithError("send() failed");

         /* Receive message from client */
        if((bytesRcvd = recv(clntSocket, buffer, RCVBUFSIZE, 0)) < 0)
          DieWithError("recv() failed");

        rcvd = malloc(sizeof(char) * bytesRcvd);
        strncpy(rcvd, buffer, bytesRcvd);
        printf("Received: %s\n", rcvd);
    }
    printf("Closing Connection.\n");
    close(clntSocket);
}

int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in servAddr; /* Local address */
    struct sockaddr_in clntAddr; /* Client address */
    unsigned short servPort;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */
    
    if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    servPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
      
    /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));   /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(servPort);      /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(clntAddr);

        /* Wait for a client to connect */
        if ((clntSock = accept(servSock, (struct sockaddr *) &clntAddr, 
                               &clntLen)) < 0)
            DieWithError("accept() failed");

        /* clntSock is connected to a client! */

        printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));

        HandleTCPClient(clntSock);
    }
    /* NOT REACHED */
}
