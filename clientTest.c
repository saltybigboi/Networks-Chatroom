//New
/*
 * Client implementation
 */

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

int first=0;
char username[50];

typedef struct sockaddr SA;

void str_cli(FILE *fp, int sockfd);
int main(int argc, char **argv)
{
    int sockfd, n;
    struct sockaddr_in servaddr;

    // Check if the IP address is provided as a command-line argument
    if(argc != 2) {
        printf("Usage: a.out <IPaddress>\n");
        exit(1);
    }

    // Create a socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket error\n");
        exit(2);
    }

    //Takes in username
    /*
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
