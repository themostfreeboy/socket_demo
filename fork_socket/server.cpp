#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <signal.h>

// #define SERVER_LISTEN_ADDR "127.0.0.1"
#define SERVER_LISTEN_PORT 12345
#define SERVER_MAX_LISTEN_FD_SIZE 1000
#define MAX_BUFFER_SIZE 1024

void sig_chld(int sig) {
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("进程终止:pid=%d\n", pid);
    }
}

int main(int argc, char** argv) {
    signal(SIGCHLD,sig_chld);
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("创建socket失败, error_code=%d\n", sock_fd);
        return -1;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_LISTEN_PORT);
    int bind_err = bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if ( bind_err < 0) {
        printf("bind失败, error_code=%d\n", bind_err);
        return -1;
    }
    int listen_err = listen(sock_fd, SERVER_MAX_LISTEN_FD_SIZE);
    if (listen_err < 0) {
        printf("listen失败, error_code=%d\n", listen_err);
        return -1;
    }
    char server_ip[MAX_BUFFER_SIZE];
    const char* server_inet_aton_err = inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, sizeof(server_ip));
    if (server_inet_aton_err == NULL) {
        printf("获取server ip失败\n");
        return -1;
    }
    printf("server端启动，开始监听\nsock_fd=%d\nserver ip:%s\nserver port:%d\n等待client端建立连接\n按ENTER键继续\n", sock_fd, server_ip, ntohs(server_addr.sin_port));
    fgetc(stdin);
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t  client_addr_len;
        int connect_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (connect_fd < 0) {
            printf("client connect失败, error_code=%d\n", connect_fd);
            return -1;
        }
        pid_t pid = fork();
        if (pid < 0) {
            printf("fork进程失败, error_code=%d\n", pid);
            return -1;
        } else if (pid == 0) {// 子进程
            printf("子进程:子进程产生, 父进程pid=%d, 子进程pid=%d, 即将关闭子进程sock_fd, sock_fd=%d\n", getppid(), getpid(), sock_fd);
            close(sock_fd);
            char client_ip[MAX_BUFFER_SIZE];
            const char* client_inet_aton_err = inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
            if (client_inet_aton_err == NULL) {
                printf("获取client ip失败\n");
                return -1;
            }
            printf("client端连接成功\nconnect_fd=%d\nclient ip:%s\nclient port:%d\n等待clien端发送数据\n", connect_fd, client_ip, ntohs(client_addr.sin_port));
            char read_buf[MAX_BUFFER_SIZE];
            int read_size;
            while (true) {
                read_size = read(connect_fd, read_buf, sizeof(read_buf));
                if (read_size > 0) {
                    printf("收到client端数据, 数据长度:%d\n数据内容:\n", read_size);
                    for (int i=0; i<read_size; ++i) {
                        printf("%c", read_buf[i]);
                    }
                    printf("\n按ENTER键继续\n");
                    fgetc(stdin);
                    char write_buf[MAX_BUFFER_SIZE];
                    snprintf((char*)&write_buf, sizeof(write_buf), "server端已成功收到数据, 数据长度:%d", read_size);
                    int write_size = write(connect_fd, write_buf, strlen(write_buf));
                    if (write_size >= 0) {
                        printf("server端发送数据成功, 发送数据, 数据长度:%d\n", write_size);
                        continue;
                    } else {
                        printf("server端发送数据失败, error_code=%d\n", write_size);
                        return -1;
                    }
                } else if(read_size == 0) {
                    printf("client端关闭连接\n");
                    close(connect_fd);
                    printf("子进程即将退出, 父进程pid=%d, 子进程pid=%d\n", getppid(), getpid());
                    return 0;
                } else { // read_size < 0
                    if (errno == EINTR) {
                        printf("遇到系统中断\n");
                        continue;
                    } else {
                        printf("read数据出错, error_code=%d\n", read_size);
                        return -1;
                    }
                }
            }
        } else {// 父进程
            printf("父进程:父进程产生子进程, 父进程pid=%d, 子进程pid=%d, 即将关闭父进程connect_fd, connect_fd=%d\n", getpid(), pid, connect_fd);
            close(connect_fd);
        }
    }
    close(sock_fd);
    printf("server端退出\n");
    return 0;
}
