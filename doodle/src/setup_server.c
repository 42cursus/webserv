#include <stdint.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>


/* Set up a basic HTTP server on the localhost, using the port
   number passed on the command line. In this example, we are
   manually configuring the server to use 127.0.0.1(localhost
   loopback) as the IP address. */
int setup_server(uint16_t port)
{
    /* Manually configure socket address and protocol information */
    struct sockaddr_in localhost;
    memset(&localhost, 0, sizeof(localhost));
    localhost.sin_family = AF_INET;                 /* IPv4 address */
    localhost.sin_port = htons(port);              /* port number from user */
    localhost.sin_addr.s_addr = htonl(0x7f000001); /* localhost loopback */

    struct addrinfo server;
    memset(&server, 0, sizeof(server));
    server.ai_family = AF_INET;         /* grab IPv4 only */
    server.ai_socktype = SOCK_STREAM;   /* specify byte-streaming */
    server.ai_flags = AI_PASSIVE;       /* use default IP address */
    server.ai_protocol = IPPROTO_TCP;   /* create as a TCP socket */
    server.ai_addrlen = INET_ADDRSTRLEN; /* IPv4 address length */
    server.ai_addr =(struct sockaddr *) &localhost;

    /* Create a TCP socket, then configure it to ignore bind reuse
       false errors and set a 5-second timeout for receiving */
    int socketfd = socket(server.ai_family, server.ai_socktype, 0);
    assert(socketfd >= 0);
    int sockopt = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR,(const void *) &sockopt,
                sizeof(int));
    struct timeval timeout = { 5, 0 };
    setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO,(const void *) &timeout,
                sizeof(timeout));

    /* Bind the socket to the port number and convert it to a
       server socket */
    if(bind(socketfd, server.ai_addr, server.ai_addrlen) != 0)
    {
        close(socketfd);
        return -1;
    }
    listen(socketfd, 10);
    return socketfd;
}