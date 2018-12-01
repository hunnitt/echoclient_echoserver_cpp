#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <thread>
#include <mutex>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

using namespace std;

#define BUF_SIZE 1024

vector<int> client_sockets;
    // group of connected client sock fd
bool is_b = false;
    // -b option
mutex mtx;
    // access to client_sockets vector => critical section

void usage(void) {
    printf("syntax : echoserver <port> [-b]\n");
    printf("sample : echoserver 1234 -b\n");
    exit(0);
}

void wait_client(int server_socket);
void echo_2_client(int server_socket, int client_socket);

int main(int argc, char * argv[]) {
    if (argc != 2 || !strcmp(argv[3], "-b"))
        usage();

    if (!strcmp(argv[3], "-b")) 
        is_b = true;

    // socket()
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == server_socket) {
        printf("socket() error!\n"); 
        exit(1);
    }

    // bind()
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family          = PF_INET;
    server_addr.sin_port            = htons(atoi(argv[1]));   
    server_addr.sin_addr.s_addr     = htonl(INADDR_ANY); 

    int bind_result = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (-1 == bind_result) {
        printf("bind() error!\n");
        exit(1);
    }
 
    puts("[+] Waiting for connections ...\n");  

    thread START_SERVER(&wait_client, server_socket);
    
    return 0;
}

void wait_client(int server_socket) {
    unique_lock<mutex> lck (mtx, defer_lock);
    // listen()
    int listen_result = listen(server_socket, 10);
    if (-1 == listen_result) {
        printf("listen() error!\n");
        exit(1);
    }

    while(true) {
    // accept()
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_size = sizeof(client_addr);

        int new_client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
        if (-1 == new_client_socket) {
            printf("accept() error!\n");
            continue;
        }

        printf("[+] New connection! / socket fd : %d\n\n", new_client_socket);

        lck.lock();
        client_sockets.push_back(new_client_socket);
        lck.unlock();

        thread ECHO_2_CLI(&echo_2_client, server_socket, new_client_socket);
    }
}

void echo_2_client(int server_socket, int client_socket) {
    unique_lock<mutex> lck (mtx, defer_lock);
    vector<int>::size_type i;

    while(true) {
        char buf[1100];
        memset(buf, 0, sizeof(buf));
    // recv()
        int recv_result = recv(client_socket, buf, BUF_SIZE, 0);
        if (-1 == recv_result) 
            continue;
        else 
            break;
        
        puts(buf);

    // send()
        lck.lock();
        switch(is_b) {
            case true:
                for (i=0; i<client_sockets.size(); i++) {
                    int send_result = send(client_sockets[i], buf, strlen(buf)+1, 0);
                    if (-1 == send_result)
                        client_sockets.erase(client_sockets.begin()+i);
                }
                break;

            case false:
                int send_result = send(client_socket, buf, strlen(buf)+1, 0);
                if (-1 == send_result) {
                    for(i=0; i<client_sockets.size(); i++) {
                        if (client_sockets[i] == client_socket) 
                            client_sockets.erase(client_sockets.begin()+i);
                    }
                }                    
                break;
        }
        lck.unlock();
    }
}