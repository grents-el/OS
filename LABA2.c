#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>

#define PORT 49746 
#define BUF_SIZE 1024 


volatile sig_atomic_t g_got_sighup = 0; // SIGHUP
int g_accepted_socket = -1; 


void handle_signal(int signum) {
    if (signum == SIGHUP) {
        g_got_sighup = 1; 
    }
}


void set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl set");
        exit(EXIT_FAILURE);
    }
}


void close_socket(int *socket_fd, const char *name) {
    if (*socket_fd != -1) {
        close(*socket_fd);
        printf("%s çàêðûò\n", name);
        *socket_fd = -1;
    }
}

int main() {
    int server_socket = -1; 
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    
    struct sigaction sa = {0};
    sa.sa_handler = handle_signal;
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close_socket(&server_socket, "Серверный сокет");
        exit(EXIT_FAILURE);
    }

    
    if (listen(server_socket, 10) == -1) {
        perror("listen");
        close_socket(&server_socket, "Серверный сокет");
        exit(EXIT_FAILURE);
    }

    printf("Сервер запущен и слушает порт %d\n", PORT);

    fd_set read_fds;
    sigset_t block_mask, orig_mask;

    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGHUP);

    if (sigprocmask(SIG_BLOCK, &block_mask, &orig_mask) == -1) {
        perror("sigprocmask");
        close_socket(&server_socket, "Серверный сокет");
        exit(EXIT_FAILURE);
    }

    while (1) {
        
        if (g_got_sighup) {
            printf("Получен SIGHUP. Завершаем работу сервера\n");
            g_got_sighup = 0;
            break;
        }

        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        int max_fd = server_socket;

        if (g_accepted_socket != -1) {
            FD_SET(g_accepted_socket, &read_fds);
            if (g_accepted_socket > max_fd) {
                max_fd = g_accepted_socket;
            }
        }

        
        int result = pselect(max_fd + 1, &read_fds, NULL, NULL, NULL, &orig_mask);
        if (result == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("pselect");
                break;
            }
        }

        
        if (FD_ISSET(server_socket, &read_fds)) {
            int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
            if (client_socket == -1) {
                perror("accept");
                continue;
            }
            printf("Новое соединение с %s:%d\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            if (g_accepted_socket == -1) {
                g_accepted_socket = client_socket;
                set_non_blocking(g_accepted_socket);
            } else {
                printf("Закрытие лишнего соединения \n");
                close(client_socket);
            }
        }

        
        if (g_accepted_socket != -1 && FD_ISSET(g_accepted_socket, &read_fds)) {
            char buffer[BUF_SIZE];
            ssize_t bytes_read = recv(g_accepted_socket, buffer, BUF_SIZE - 1, 0);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("Получено %zd байтов: %s\n", bytes_read, buffer);
            } else if (bytes_read == 0) {
                printf("Клиент закрыл соединение\n");
                close_socket(&g_accepted_socket, "Клиентский сокет");
            } else if (errno != EAGAIN) {
                perror("recv");
                close_socket(&g_accepted_socket, "Клиентский сокет");
            }
        }
    }

    close_socket(&server_socket, "Серверный сокет");
    close_socket(&g_accepted_socket, "Клиентский сокет");
    return 0;
}
