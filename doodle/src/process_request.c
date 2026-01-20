#include <unistd.h>
#include <sys/socket.h>
#include <string.h>

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_LENGTH 1000
#define WHITESPACE " \t\r\n"
#define CRLF "\r\n"


/* Parse the incoming HTTP request and look for the requested
   file. It must be of the form /cgi-bin/foo.cgi or it must be
   shutdown. All other requests are ignored. Returning true
   from this function keeps the server running, while returning
   false will shut the server down. */
int process_request(int connection)
{
    /* Read the HTTP request from the socket into a local buffer */
    char buffer[BUFFER_LENGTH] = {};
    size_t bytes = read(connection, buffer, BUFFER_LENGTH - 1);
    if(bytes <= 0)
    {
        perror("No data read from socket");
        shutdown(connection, SHUT_RDWR);
        close(connection);
        return 1;
    }

    /* This minimal web server only supports requests of the
       following formats:

         GET /cgi-bin/hello.cgi HTTP/1.1 ...
         GET /cgi-bin/hello.cgi?username=me HTTP/1.1 ...
         GET /shutdown ...

     */

    /* Reject any request that doesn't start with "GET" */
    char *token = strtok(buffer, WHITESPACE);
    if(strncmp(token, "GET", 4))
    {
        perror("Invalid HTTP request received");
        shutdown(connection, SHUT_RDWR);
        close(connection);
        return 1;
    }

    /* Check for a shutdown request. Note that all URIs passed from
       the web browser will begin with a '/' character, so we are
       looking for "/shutdown". */
    token = strtok(NULL, WHITESPACE);
    if(!strncmp(token, "/shutdown", 10))
    {
        /* Send a message confirming the shutdown request, then
           return false to shut the server down */
        char *message = "HTTP/1.1 200 OK" CRLF
        "Connection: close" CRLF
        "Content-Type: text/html; charset=UTF-8" CRLF CRLF
        "<html><body><h2>Shutdown received</h2>"
        "<p>Goodbye</p></body></html>\n";

        write(connection, message, strlen(message));
        shutdown(connection, SHUT_RDWR);
        close(connection);
        return 0;
    }

    /* For this example, we are only supporting CGI executable
       files and they must exist in a "cgi-bin" subdirectory of the
       server's working directory. Reject any request that does not
       begin with "/cgi-bin/". */
    if(strncmp(token, CGI_DIR, 9) != 0)
    {
        shutdown(connection, SHUT_RDWR);
        close(connection);
        return 1;
    }

    /* Remove the leading '/' character, as we are looking for
       files based on an relative path, not an absolute path */
    char *cgi_file = token + 1;

    /* Search for a query string, which begins immediately following
       the '?' character if it exists. If one is found, keep track
       of where the query string starts and replace the '?' with a
       null byte. This will ensure the file name is properly
       terminated. For example, "cgi-bin/hello.cgi?user=me" will be
       converted to the cgi_file "cgi-bin/hello.cgi" while the
       question pointer will point to "user=me". */
    char *question = strchr(cgi_file, '?');
    if(question != NULL)
    {
        *question = '\0';
        question++;
    }
    /* Check for file read and execute permissions */
    if(access(cgi_file, R_OK | X_OK) != 0)
    {
        shutdown(connection, SHUT_RDWR);
        close(connection);
        return 1;
    }

    /* Now we are ready to respond. Write back an HTTP/1.1 header
       to the web browser then run the CGI program. */
    char *message = "HTTP/1.1 200 OK" CRLF "Connection: close" CRLF
    "Content-Type: text/html; charset=UTF-8" CRLF CRLF;
    write(connection, message, strlen(message));

    pid_t child_pid = fork();
    if(child_pid < 0)
    {
        shutdown(connection, SHUT_RDWR);
        close(connection);
        return 0;
    }

    if(child_pid == 0)
    {
        /* If query string passed, set the environment variable */
        if(question != NULL)
            setenv("QUERY_STRING", question, 1);

        /* Redirect the child process's STDOUT to write into the
           socket and execute the CGI program */
        dup2(connection, STDOUT_FILENO);
        execlp(cgi_file, cgi_file, NULL);
        return 1;
    }

    /* The parent waits until the child process runs(writing to the
       client over the socket), then closes the socket and continues
       with the next request */
    wait(NULL);
    shutdown(connection, SHUT_RDWR);
    close(connection);
    return 1;
}
