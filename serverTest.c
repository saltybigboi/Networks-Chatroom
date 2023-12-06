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

int checkUser(char buffer[MAXLINE],client_info *ci)
{
        char serverMessage[MAXLINE];
        char command[MAXLINE];
        char username[MAXLINE];
        char hostname[MAXLINE];
        char servername[MAXLINE];
        char realname[MAXLINE];
        int itemsRead=sscanf(buffer, "%s %s %s %s %s", command, username, hostname, servername, realname);
        if(itemsRead!=5)
        {
                strcpy(serverMessage,"Not Enough parameters\n");
                write(ci->sockfd, serverMessage, strlen(serverMessage));
                return 1;
        }
        if(strcmp(command, "/USER")!=0)
        {
        {

                strcpy(serverMessage,command);
                write(ci->sockfd, serverMessage, strlen(serverMessage));
                return 2;
        }
        else
        {
                for(int i=0;i<num_clients;i++)
                {
                        if(strcmp(clients[i].username,username)==0)
                        {
                                strcpy(serverMessage,"Duplicate Username, please pick another\n");
                                write(ci->sockfd, serverMessage, strlen(serverMessage));
                                return 3;
                        }
                }
                strcpy(ci->username,username);
                strcpy(serverMessage,"You have entered\n");
                write(ci->sockfd, serverMessage, strlen(serverMessage));
                return 0;
        }
}
// Function to handle a client
void *handle_client(void *arg) {
    client_info *ci = (client_info *)arg;
    char buffer[MAXLINE],serverMessage[MAXLINE];
    int statusCode=2, count=0;
    // Read the username from the socket
    //read(ci->sockfd, ci->username, sizeof(ci->username));
    strcpy(serverMessage, "Enter /USER <username> <hostname> <servername> <realname>");
    write(ci->sockfd, serverMessage, strlen(serverMessage));
    while(1) {
        // Read a message from the client
        int n = read(ci->sockfd, buffer, MAXLINE);
        if(statusCode!=0)
                statusCode=checkUser(buffer,&clients[num_clients-1]);
        if(n <= 0||statusCode!=0) {
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
        if(count!=0)
        {
        for(int i = 0; i <= num_clients; i++) {
            if(clients[i].sockfd != ci->sockfd) {
                write(clients[i].sockfd, buffer, n);
            }
        }
        }
                count=1;
    }
}



int main(int argc, char **argv) {
        int i, maxi, maxfd, listenfd, connfd, sockfd, errorCheck=0;
    int sockwfd;
    int nready, client[FD_SETSIZE];
    size_t n;
    fd_set rset, allset;
    socklen_t cli_len;
    struct sockaddr_in cli_addr;
    struct sockaddr_in serv_addr;
    char buff[MAXLINE], tempStorage;

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
