#include <stdio.h>  // console input/output, perror
#include <stdlib.h> // exit

#define INDEX "/"

char *routing(int *client_fd, char *route, char *method, char *request)
{

    char *response;
    if (route == INDEX)
    {
        response = todo(client_fd, method, request);
    }
    else
    {
        response = NULL;
    }

    return response;
}