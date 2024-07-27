#include <stdio.h>  // console input/output, perror
#include <stdlib.h> // exit
#include <string.h> // string manipulation

#include <sys/socket.h>

void error(int *client_fd, char code[3], char message[])
{
    char *response = (char *)malloc((17 + strlen(message)));
    sprintf("HTTP/1.1 %s %s\r\n\n", code, message);
    send(*client_fd, response, sizeof(response), 0);
    free(response);
}