#include <stdio.h>  // console input/output, perror
#include <stdlib.h> // exit
#include <string.h> // string manipulation
#include <netdb.h>  // getnameinfo

#include <sys/socket.h> // socket APIs
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // open, close

#include <signal.h> // signal handling
#include <time.h>   // time
#include <pthread.h>

#define SIZE 1024  // buffer size
#define PORT 2728  // port number
#define BACKLOG 10 // number of pending connections queue will hold

int serverSocket;
struct sockaddr_in server_addr;

void getFileURL(char *route, char *fileURL)
{
    char *question = strrchr(route, '?');
    if (question)
    {
        *question = '\0';
    }

    if (route[strlen(route) - 1] == '/')
    {
        strcat(route, "index.html");
    }

    strcpy(fileURL, "htdocs");
    strcat(fileURL, route);

    const char *dot = strrchr(fileURL, '.');
    if (!dot || dot == fileURL)
    {
        strcat(fileURL, ".html");
    }
}

void getTimeString(char *buf)
{
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
}

void getMimeType(char *file, char *mime)
{
    const char *dot = strrchr(file, '.');

    if (dot == NULL)
    {
        strcpy(mime, "text/html");
    }
    else if (strcmp(dot, ".html") == 0)
    {
        strcpy(mime, "text/html");
    }
    else if (strcmp(dot, ".css") == 0)
    {
        strcpy(mime, "text/css");
    }
    else if (strcmp(dot, ".js") == 0)
    {
        strcpy(mime, "application/js");
    }
    else if (strcmp(dot, ".jpg") == 0)
    {
        strcpy(mime, "image/jpeg");
    }
    else if (strcmp(dot, ".png") == 0)
    {
        strcpy(mime, "image/png");
    }
    else if (strcmp(dot, ".gif") == 0)
    {
        strcpy(mime, "image/gif");
    }
    else
    {
        strcpy(mime, "text/html");
    }
}

void *handle_client(void *client_fd)
{
    printf("2: %d\n", *(int *)client_fd);
    char *request = (char *)malloc(SIZE * sizeof(char));

    read(*(int *)client_fd, request, SIZE);

    char method[10], route[100];

    sscanf(request, "%s %s", method, route);
    printf("%s %s", method, route);

    free(request);

    if (strcmp(method, "GET") != 0)
    {
        const char response[] = "HTTP/1.1 400 Bad Request\r\n\n";
        send(*(int *)client_fd, response, sizeof(response), 0);
    }
    else
    {
        char fileURL[100];

        getFileURL(route, fileURL);

        FILE *file = fopen(fileURL, "r");

        if (!file)
        {
            const char response[] = "HTTP/1.1 404 Not Found\r\n\n";
            send(*(int *)client_fd, response, sizeof(response), 0);
        }
        else
        {
            char resHeader[SIZE];

            char timeBuf[100];
            getTimeString(timeBuf);

            char mimeType[32];
            getMimeType(fileURL, mimeType);

            sprintf(resHeader, "HTTP/1.1 200 OK\r\nDate: %s\r\nContent-Type: %s\r\n\n", timeBuf, mimeType);
            int headerSize = strlen(resHeader);

            printf(" %s", mimeType);

            fseek(file, 0, SEEK_END);
            long fsize = ftell(file);
            fseek(file, 0, SEEK_SET);

            char *resBuffer = (char *)malloc(fsize + headerSize);
            strcpy(resBuffer, resHeader);

            char *fileBuffer = resBuffer + headerSize;
            fread(fileBuffer, fsize, 1, file);

            send(*(int *)client_fd, resBuffer, fsize + headerSize, 0);
            free(resBuffer);
            fclose(file);
        }
    }
    close(*(int *)client_fd);
    printf("\n");
}

int main()
{

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if (bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 10) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    char hostBuffer[10000], serviceBuffer[100000];
    int error = getnameinfo((struct sockaddr *)&server_addr, sizeof(server_addr), hostBuffer,
                            sizeof(hostBuffer), serviceBuffer, sizeof(serviceBuffer), 0);

    printf("\nServer is listening on http://%s:%s/\n\n", hostBuffer, serviceBuffer);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));

        if ((*client_fd = accept(serverSocket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
        {
            perror("accept falied");
            continue;
        }

        printf("%d\n", *client_fd);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void *)client_fd);
        pthread_detach(thread_id);
    }

    return 0;
}