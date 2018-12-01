#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

#define BUF_SIZE 1024

void usage(void) {
    printf("syntax : echoclient <host> <port>\n");
    printf("sample : echoclient 127.0.0.1 1234\n");
    exit(0);
}

int main(int argc, char * argv[]) {
    if (argc != 3) 
        usage();

    int client_socket;
    struct sockaddr_in server_addr;
    char send_buf[1100];
    char recv_buf[1100];
    memset(send_buf, 0, sizeof(send_buf));
    memset(recv_buf, 0, sizeof(recv_buf));

    // socket()
    client_socket  = socket(PF_INET, SOCK_STREAM, 0);
    if (-1 == client_socket) {
        printf("socket() error!\n");
        exit(1);
    }

    memset( &server_addr, 0, sizeof( server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // connect()
    int connect_result = connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (-1 == connect_result) {
        printf("connect() error!\n");
        exit(1);
    }

    while(1) {
        puts("input message (max 1024 bytes) >\n");
        fgets(send_buf, BUF_SIZE, stdin);

    // send()
        send(client_socket, send_buf, BUF_SIZE, 0);
        puts("\n\n");

    // recv()
        recv(client_socket, recv_buf, BUF_SIZE, 0);
        printf("%s\n", recv_buf);
    }
    close(client_socket);
    return 0;
}