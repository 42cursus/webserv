#include <assert.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void sigint_handler(int signum); /* SIGINT handler */
int process_request(int);
int setup_server(uint16_t);

int serverfd = -1; /* Server socket file descriptor */

#define STR1(x) #x
#define STR(x) STR1(x)
#define ASM_DBG_LABEL(name) \
    asm volatile( \
        ".loc 1 " STR(__LINE__) " 0\n\t" \
        ".globl " #name "\n\t"         \
        ".hidden " #name "\n\t" \
        #name ":\n\t" \
        : : : "memory")
#define ASM_L(name) ASM_DBG_LABEL(name)

int main(int argc, char *argv[])
{
    /* Create a sigaction and link it to the handler */
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    assert(sigaction(SIGINT, &sa, NULL) != -1);
    assert(sigaction(SIGTERM, &sa, NULL) != -1);

    /* Get the port number from the command line and set up the
       server. If the setup does not work, the socket file
       descriptor will be negative; exit if that happens. */
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    uint16_t port =(uint16_t) strtoul(argv[1], NULL, 10);
    if((serverfd = setup_server(port)) < 0) {
        perror("Failed to set up web server");
        return EXIT_FAILURE;
    }
    fprintf(stderr, "listening on: http://127.0.0.1:%s/cgi-bin/hello.cgi\n", argv[1]);

    /* Enter a loop for processing web connections */
    bool running = true;
    while(running)
    {
        ASM_L(server_loop_start);
        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        socklen_t addrlen = 0;

        int connection = accept(serverfd,(struct sockaddr *)&address, &addrlen);
        if(connection < 0) {
            perror("connection < 0");
            break;
        }
        running = process_request(connection);
        ASM_L(server_loop_iter);
    }
    ASM_L(server_loop_end);
    /* After receiving a shutdown request, close the server
       socket and exit. Note that it would be good to include a
       signal handler(Chapter 2) to close the socket if the
       process is killed with SIGINT or another signal. */
    shutdown(serverfd, SHUT_RDWR);
    close(serverfd);
    serverfd = -1;
    return EXIT_SUCCESS;
}

/* Interrupt handler in case we need to shut down with Ctrl-c.
   This helps to ensure the server socket is shutdown cleanly. */
static void
sigint_handler(int signum)
{
    if(serverfd >= 0)
    {
        shutdown(serverfd, SHUT_RDWR);
        close(serverfd);
    }
    exit(EXIT_SUCCESS);
   (void)signum;
}