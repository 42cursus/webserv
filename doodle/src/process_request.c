#include <unistd.h>
#include <sys/socket.h>
#include <string.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define BUFFER_LENGTH 1024
#define STREAM_BUF_SIZE 8192
#define INSTREAM_BUF_SIZE 8192
#define OUTSTREAM_BUF_SIZE 8192
#define CRLF "\r\n"

enum e_exec_exit_code {
    EXIT_CANNOT_EXECUTE = 126,
    EXIT_COMMAND_NOT_FOUND = 127
};

int file_write_and_flush(FILE *out, const void *buf, size_t len)
{
    if (len == 0)
        return 0;

    errno = 0;
    size_t nw = fwrite(buf, 1, len, out);
    if (nw != len)
    {
        /* fwrite failed (or shortly wrote).
         * errno is (usually) set by the underlying write(2). */
        if (errno == EPIPE || errno == ECONNRESET)
            return -1; // peer closed while we were writing
        return -2;
    }

    if (fflush(out) != 0)
    {
        if (errno == EPIPE || errno == ECONNRESET)
            return -1; // the peer closed while we were flushing
        return -2;
    }
    return 0;
}


int peer_closed_now(int fd)
{
    ssize_t n;
    char    ch;

    while (1)
    {
        // use MSG_PEEK so to not consume bytes and keep using stdio for parsing.
        n = recv(fd, &ch, 1, MSG_PEEK);
        if (n > 0) return 0; // data available
        if (n == 0) return 1; // peer closed (FIN)
        if (errno == EINTR) continue; // syscall was interrupted before completion => retry (hence looping)
        break; // other errors (EAGAIN/EWOULDBLOCK, ECONNRESET, etc.)
    }
    return 0;
}

int process_request(int connection)
{
    int in_fd = dup(connection);
    if (in_fd < 0)
    {
        shutdown(connection, SHUT_RDWR);
        close(connection);
        return 1;
    }

    FILE    *in = fdopen(in_fd, "r");
    FILE    *out = fdopen(connection, "w");
    if (!in || !out)
    {
        if (in) fclose(in); // closes in_fd
        else  close(in_fd);

        if (out) fclose(out); // closes the connection
        else {
            shutdown(connection, SHUT_RDWR);
            close(connection);
        }
        return 1;
    }

    char inbuf[INSTREAM_BUF_SIZE];
    char outbuf[OUTSTREAM_BUF_SIZE];
    setvbuf(in, inbuf, _IOFBF, INSTREAM_BUF_SIZE);
    setvbuf(out, outbuf, _IOFBF, OUTSTREAM_BUF_SIZE);

    FILE *req_dump = tmpfile();
    if (!req_dump)
    {
        fclose(in);
        fclose(out);
        return 1;
    }
    setvbuf(req_dump, NULL, _IOFBF, STREAM_BUF_SIZE);

    char    line[BUFFER_LENGTH + 1] = {0x00};
    char    request_line[BUFFER_LENGTH + 1] = {0x00};
    if (peer_closed_now(in_fd))
    {
        fclose(in);
        fclose(out);
        return 1;
    }

    while (fgets(line, sizeof(line), in) != NULL)
    {
        fputs(line, req_dump);

        if (request_line[0] == '\0')
        {
            strncpy(request_line, line, BUFFER_LENGTH);
            request_line[BUFFER_LENGTH] = '\0';
        }

        if (strcmp(line, "\n") == 0 || strcmp(line, CRLF) == 0)
            break;
    }

    // fgets==NULL + feof() => peer closed
    if (request_line[0] == '\0' || ferror(in) || feof(in))
    {
        fclose(req_dump);
        fclose(in);
        fclose(out);
        return 1;
    }

    // Parse: METHOD _ URI _ HTTP/VERSION
    char method[16] = {0};
    char uri[BUFFER_LENGTH] = {0};
    if (sscanf(request_line, "%15s %999s", method, uri) != 2)
    {
        fclose(req_dump);
        fclose(in);
        fclose(out);
        return 1;
    }

    if (strcmp(method, "GET") != 0)
    {
        fclose(req_dump);
        fclose(in);
        fclose(out);
        return 1;
    }

    if (strncmp(uri, "/shutdown", sizeof("/shutdown") + 1) == 0)
    {
        const char *message =
            "HTTP/1.1 200 OK" CRLF
            "Connection: close" CRLF
            "Content-Type: text/html; charset=UTF-8" CRLF CRLF
            "<html><body><h2>Shutdown received</h2>"
            "<p>Goodbye</p></body></html>\n";

        if (file_write_and_flush(out, message, strlen(message)) < 0)
        {
            fclose(req_dump);
            fclose(in);
            fclose(out);
            return 1;
        }

        fclose(req_dump);
        fclose(in);
        fclose(out);
        return 0;
    }

    /* we only support /cgi-bin/... */
    if (strncmp(uri, CGI_DIR, 9) != 0)
    {
        fclose(req_dump);
        fclose(in);
        fclose(out);
        return 1;
    }

    /* Remove the leading '/' character, as we are looking for
       files based on a relative path, not an absolute path */
    char *cgi_file = uri + 1;

    /* Split query string */
    char *qmark = strchr(cgi_file, '?');
    if (qmark != NULL)
    {
        *qmark = '\0';
        qmark++;
    }

    if (access(cgi_file, R_OK | X_OK) != 0)
    {
        fclose(req_dump);
        fclose(in);
        fclose(out);
        return 1;
    }

    const char *hdr =
        "HTTP/1.1 200 OK" CRLF
        "Connection: close" CRLF
        "Content-Type: text/html; charset=UTF-8" CRLF CRLF;

    int cgi_pipe[2];
    if ((file_write_and_flush(out, hdr, strlen(hdr)) < 0) || (pipe(cgi_pipe) != 0))
    {
        fclose(req_dump);
        fclose(in);
        fclose(out);
        return 1;
    }

    pid_t child_pid = fork();
    if (child_pid == 0) // child
    {
        dup2(cgi_pipe[1], STDOUT_FILENO); // stdout -> pipe
        close(cgi_pipe[0]);
        close(cgi_pipe[1]);

        if (qmark != NULL)
            setenv("QUERY_STRING", qmark, 1);

        execlp(cgi_file, cgi_file, NULL);
        if (errno == ENOENT)
            _exit(EXIT_COMMAND_NOT_FOUND);
        _exit(EXIT_CANNOT_EXECUTE);
    }
    else if (child_pid < 0) // error
    {
        close(cgi_pipe[0]);
        close(cgi_pipe[1]);
        fclose(req_dump);
        fclose(in);
        fclose(out);
        return 1;
    }
    else // parent
    {
        close(cgi_pipe[1]);
    }

    /* read CGI output as FILE* (buffered), write to client FILE* (buffered) */
    FILE *cgi_out = fdopen(cgi_pipe[0], "r");
    if (!cgi_out)
    {
        close(cgi_pipe[0]);
        wait(NULL);
        fclose(req_dump);
        fclose(in);
        fclose(out);
        return 1;
    }

    char cgibuf[STREAM_BUF_SIZE];
    setvbuf(cgi_out, cgibuf, _IOFBF, STREAM_BUF_SIZE);

    char xfer[STREAM_BUF_SIZE / 2];
    size_t nread = 0;
    while ((nread = fread(xfer, 1, STREAM_BUF_SIZE / 2, cgi_out)) > 0)
        fwrite(xfer, 1, nread, out);

    fflush(out);
    fclose(cgi_out);
    wait(NULL);

    fclose(req_dump);
    fclose(in);
    fclose(out);

    return 1;
}
