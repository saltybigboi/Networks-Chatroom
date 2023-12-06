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
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0 ;   // Remove trailing newline character

    // Initialize server address structure
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT); /*daytime server - normally 13*/

    // Convert IP address from text to binary form
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        printf("inet_pton error for %s\n", argv[1]);

    // Connect to the server
    if(connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0)
        printf("connect error\n");

    //Send username to server
    write(sockfd, username, strlen(username));

    // Call the function to handle client-server communication
    str_cli(stdin, sockfd);
    exit(0);
}

// Function to return the maximum of two numbers
int max(int a, int b)
{
    return a > b ? a : b;
}

// Function to handle client-server communication
void str_cli(FILE *fp, int sockfd)
{
    char sendline[MAXLINE], recvline[MAXLINE]; // Buffers for sending and receiving data
    int maxfdp1, stdineof; // Variables for tracking end-of-file and maximum file descriptor
    int n; // Variable for tracking number of bytes read
    fd_set rset; // Set of file descriptors for select()

    stdineof = 0; // Flag to indicate end-of-file on standard input
    FD_ZERO(&rset); // Clear the file descriptor set
    printf("Client ready\n"); // Notify that the client is ready
    sendline[0] = '\0'; // Initialize send buffer

    for(;;) // Infinite loop
    {
        if(stdineof == 0)
            FD_SET(fileno(fp), &rset); // Add standard input to file descriptor set if not end-of-file
        FD_SET(sockfd, &rset); // Add socket to file descriptor set
        maxfdp1 = max(fileno(fp), sockfd) + 1; // Calculate maximum file descriptor plus one
        select(maxfdp1, &rset, NULL, NULL, NULL); // Wait for data on either file descriptor

        if(FD_ISSET(sockfd, &rset)) // If data is available on the socket
        {
            if((n = read(sockfd, recvline, MAXLINE)) == 0) // Read data from socket
            {
                if(stdineof == 1)
                    return; // If end-of-file on standard input, return
                else{
                    printf("str_cli: server terminated prematurely\n"); // If not, server terminated prematurely
                    exit(1);
                }
            }
            recvline[n] = '\0'; // Null-terminate received data
            write(fileno(stdout), recvline, n); // Write received data to standard output
        }

        if(fgets(sendline, MAXLINE - 1, fp) == NULL) // Read data from standard input
        {
            stdineof = 1; // If end-of-file, set flag
            shutdown(sockfd, SHUT_WR); // Shutdown socket write half
            FD_CLR(fileno(fp), &rset); // Remove standard input from file descriptor set
            continue; // Continue to next iteration of loop
        }
        else{
            char message[MAXLINE];
            sprintf(message, "%s: %s", username, sendline);
            n = strlen(message); // Update n to reflect the number of bytes read
            message[n] = '\0'; // Null-terminate send data
            write(sockfd, message, n); // Write send data to socket
        }

            //write(sockfd, sendline, n); // Write send data to socket
    }
}
