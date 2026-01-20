/* This file needs to be compiled and the executable should be
   placed in a subdirectory of the directory where the server
   runs. If the server runs on port 8000 and this file is
   compiled into an executable named "hello.cgi", it can be
   accessed with a web browser at
     http://localhost:8000/cgi-bin/hello.cgi
 */
#include <stdio.h>
#include <stdlib.h>
/* CGI (common gateway interface) version of "Hello world".
   Programs like this can write to STDOUT. The web server that
   runs the executable form of this will redirect that output
   into a socket created for a web page access. */
int
main (void)
{
    printf ("<html>\n<body>\n");
    printf ("<h2>Hello world!</h2>\n");
    printf ("<p>Your CGI bin is set up properly</p>\n");
    /* Check to see if an HTTP query string was passed */
    char *query = getenv("QUERY_STRING");
    if (query != NULL)
        printf ("<p>Here is your query string data: %s</p>\n", query);

    printf ("</body>\n</html>\n");
    return 0;
}
