#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8080
#define MAX_PENDING_CONNECTIONS 10
#define NUM_CHILDREN 5

void handle_connection(int client) {
    char buf[256];
    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = '\0';
    printf("Received: %s\n", buf);

    char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
    send(client, msg, strlen(msg), 0);

    close(client);
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(listener, MAX_PENDING_CONNECTIONS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid == 0) { // Child process
            while (1) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client = accept(listener, (struct sockaddr *)&client_addr, &client_len);
                if (client < 0) {
                    perror("accept failed");
                    exit(EXIT_FAILURE);
                }
                printf("New client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                handle_connection(client);
            }
        } else if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
    }

    // Parent process
    // Wait for children to finish
    for (int i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL);
    }

    close(listener);
    return 0;
}
