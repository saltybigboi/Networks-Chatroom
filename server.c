//New server
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define MAXLINE 1024
#define SERV_PORT 25565
#define LISTENQ 64


typedef struct sockaddr SA;

// Structure to hold client information
typedef struct {
    int sockfd;
    char username[100];
} client_info;

// Array to hold all connected clients
client_info clients[100];
int num_clients = 0;

// Function to handle a client
void *handle_client(void *arg) {
    client_info *ci = (client_info *)arg;
    char buffer[MAXLINE];

    // Read the username from the socket
    read(ci->sockfd, ci->username, sizeof(ci->username));

    while(1) {
        // Read a message from the client
        int n = read(ci->sockfd, buffer, MAXLINE);
        if(n <= 0) {
            // If the client disconnected, remove them from the clients array
            for(int i = 0; i < num_clients; i++) {
                if(clients[i].sockfd == ci->sockfd) {
                    clients[i] = clients[num_clients - 1];
                    num_clients--;
                    break;
                }
            }
            close(ci->sockfd);
            return NULL;
        }

        // Broadcast the message to all other clients
        for(int i = 0; i <= num_clients; i++) {
            if(clients[i].sockfd != ci->sockfd) {
                write(clients[i].sockfd, buffer, n);
            }
        }
    }
}

int main(int argc, char **argv) {
        int i, maxi, maxfd, listenfd, connfd, sockfd;
    int sockwfd;
    int nready, client[FD_SETSIZE];
    size_t n;
    fd_set rset, allset;
    socklen_t cli_len;
    struct sockaddr_in cli_addr;
    struct sockaddr_in serv_addr;
    char buff[MAXLINE];

    // Create a socket for the server
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // Initialize server address structure
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    // Bind the socket to the server address
    bind(listenfd, (SA *) &serv_addr, sizeof(serv_addr));

    // Listen for connections
    listen(listenfd, LISTENQ);

    // Initialize the file descriptor set
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    maxfd = listenfd;
    maxi = -1;

    for(i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;

    // Accept loop
    while(1) {
        client_info ci;
        ci.sockfd = accept(listenfd, (SA *) &cli_addr, &cli_len);

        // Add the client to the clients array
        clients[num_clients] = ci;
        num_clients++;

        // Create a new thread to handle the client
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, &clients[num_clients - 1]);
    }
}
