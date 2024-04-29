#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 7777
#define MAX_PENDING_CONNECTIONS 10

void handle_connection(int client) {
    char buf[256];
    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret < 0) {
        perror("recv failed");
        close(client);
        return;
    }
    buf[ret] = '\0';
    printf("Received: %s\n", buf);

    // Kiểm tra lệnh gửi từ client
    if (strncmp(buf, "GET_TIME", 8) != 0) {
        char *error_msg = "Invalid command. Please use GET_TIME [format]";
        send(client, error_msg, strlen(error_msg), 0);
        close(client);
        return;
    }

    // Lấy định dạng yêu cầu từ client
    char format[20];
    sscanf(buf, "GET_TIME %s", format);

    // Kiểm tra và lấy thời gian
    time_t raw_time;
    struct tm *time_info;
    time(&raw_time);
    time_info = localtime(&raw_time);
    char time_str[50];

    if (strcmp(format, "dd/mm/yyyy") == 0) {
        strftime(time_str, sizeof(time_str), "%d/%m/%Y", time_info);
    } else if (strcmp(format, "dd/mm/yy") == 0) {
        strftime(time_str, sizeof(time_str), "%d/%m/%y", time_info);
    } else if (strcmp(format, "mm/dd/yyyy") == 0) {
        strftime(time_str, sizeof(time_str), "%m/%d/%Y", time_info);
    } else if (strcmp(format, "mm/dd/yy") == 0) {
        strftime(time_str, sizeof(time_str), "%m/%d/%y", time_info);
    } else {
        char *error_msg = "Invalid date format.";
        send(client, error_msg, strlen(error_msg), 0);
        close(client);
        return;
    }

    // In thời gian ra client
    send(client, time_str, strlen(time_str), 0);
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

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client = accept(listener, (struct sockaddr *)&client_addr, &client_len);
        if (client < 0) {
            perror("accept failed");
            continue;
        }
        printf("New client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handle_connection(client);
    }

    close(listener);
    return 0;
}
