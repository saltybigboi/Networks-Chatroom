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

void chatFilter(char* chat, const char** username, int* len);
int user_storage(const char* client, const char** username, int* len);

// Function to handle a client
void *handle_client(void *arg) {
    client_info *ci = (client_info *)arg;
    char buffer[MAXLINE];

    // Read the username from the socket
    read(ci->sockfd, ci->username, sizeof(ci->username));

    while (1) {
        // Read a message from the client
        int n = read(ci->sockfd, buffer, MAXLINE);
        if (n <= 0) {
            // If the client disconnected, remove them from the clients array
            for (int i = 0; i < num_clients; i++) {
                if (clients[i].sockfd == ci->sockfd) {
                    clients[i] = clients[num_clients - 1];
                    num_clients--;
                    break;
                }
            }
            close(ci->sockfd);
            pthread_exit(NULL);
        }

        // Check if the message is a command
        chatFilter(buffer, NULL, NULL);

        // Handle the /USER command
        if (strncmp(buffer, "/USER", 5) == 0) {
            char username[100];
            sscanf(buffer, "/USER %s", username);

            // Check if the username is taken
            int userCheck = user_storage(username, NULL, NULL);
            if (userCheck == 0) {
                // Save the username and inform the client
                strcpy(ci->username, username);
                write(ci->sockfd, "Username set successfully", sizeof("Username set successfully"));
            } else {
                // Inform the client that the username is already taken
                write(ci->sockfd, "ERR_ALREADY_REGISTERED", sizeof("ERR_ALREADY_REGISTERED"));
            }
        } else {
            // Broadcast the message to all other clients
            for (int i = 0; i < num_clients; i++) {
                if (clients[i].sockfd != ci->sockfd) {
                    write(clients[i].sockfd, buffer, n);
                }
            }
        }
    }
}

//takes a proposed username, the user database, and the length of the database
//1 indicates a duplicate, 0 indicates success
int user_storage(const char* client, const char** username, int* len)
{
    //check if the username is taken
    for(int i=0; i<*len; i++)
    {
        if(username[i]==client)
        {
            //adjust to print to client
            printf("ERR_ALREADY_REGISTERED");
            return 1;
        }
    }
    //saves username and increments the storage int by 1
    username[*len]==client;
    *(len)++;
    return 0;
}

//takes a message, the username database, and the length of the database
//the purpose of this is to determine if a message is a command or not
void chatFilter(char* chat, const char** username, int* len)
{
    int i=0, parameters=0;
    char command[10], user[10], hostname[10], servername[10], realname[10];
    //think minecraft, / indicates a command
    if(chat[0]=='/')
    {
        //the proposed command name is stored in command
        sscanf(chat, "%s", command);
        //begening of user usecase
        if(strcmp(command, "/USER")==0)
        {
            //all parameters scanned in from chat
            parameters=sscanf(chat, "%s %s %s %s %s", command, user, hostname, servername, realname);
            if(parameters!=5)
            {
                //adjust to print to client
                //ERROR! not enough parameters
                printf("ERR_NEED_MORE_PARAMS");
                return;
            }
            i=user_storage(user, username, len);
            //Error message handled by function, so it is left out here
            if(i>0)
                return;
        }
    }
    return;
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
    while (1) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) {
            // accept a new client connection
            cli_len = sizeof(cli_addr);
            connfd = accept(listenfd, (SA *)&cli_addr, &cli_len);

            // find an unused client slot to store the socket_id
            i = 0;
            while (client[i] >= 0 && i < FD_SETSIZE)
                i++;
            if (i < FD_SETSIZE) {
                client[i] = connfd;
            } else {
                printf("Too many clients!\n");
                exit(1); // run your command or perform specific actions here
            }
            FD_SET(connfd, &allset);
            if (connfd > maxfd)
                maxfd = connfd;
            if (i > maxi)
                maxi = i;
            if (--nready < 0)
                continue; // no more readable descriptors

            // Create a new thread to handle the client
            pthread_t thread;
            clients[num_clients].sockfd = connfd;
            pthread_create(&thread, NULL, handle_client, &clients[num_clients]);
            num_clients++;
        }

        for (i = 0; i <= maxi; i++) {
            if ((sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset)) {
                // ... (Your existing code)
            }
        }
    }
    return 0;
}