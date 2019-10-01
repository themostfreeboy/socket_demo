#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <memory.h>

#define SERVER_LISTEN_ADDR "127.0.0.1"
#define SERVER_LISTEN_PORT 12345
// #define SERVER_MAX_LISTEN_FD_SIZE 1000
#define MAX_BUFFER_SIZE 1024

int main(int argc, char** argv) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("创建socket失败, error_code=%d\n", sock_fd);
        return -1;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_LISTEN_PORT);
    int inet_pton_err = inet_pton(AF_INET, SERVER_LISTEN_ADDR, &server_addr.sin_addr);
    if (inet_pton_err != 1) {
        printf("写入server端ip地址失败, error_code=%d\n", inet_pton_err);
        return -1;
    }
    char server_ip[MAX_BUFFER_SIZE];
    const char* server_inet_aton_err = inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, sizeof(server_ip));
    if (server_inet_aton_err == NULL) {
        printf("获取server ip失败\n");
        return -1;
    }
    printf("client端启动，开始尝试连接server端\nsock_fd=%d\nserver ip:%s\nserver port:%d\n开始连接server端\n按ENTER键继续\n", sock_fd, server_ip, ntohs(server_addr.sin_port));
    fgetc(stdin);
    int connect_fd = connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (connect_fd < 0) {
        printf("server connect失败, error_code=%d\n", connect_fd);
        return -1;
    }
    struct sockaddr_in client_addr;
    socklen_t  client_addr_len;
    int getsockname_err = getsockname(sock_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (getsockname_err != 0) {
        printf("获取client getsockname失败, error_code=%d\n", getsockname_err);
        return -1;
    }
    char client_ip[MAX_BUFFER_SIZE];
    const char* client_inet_aton_err = inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    if (client_inet_aton_err == NULL) {
        printf("获取client ip失败\n");
        return -1;
    }
    printf("server端连接成功\nconnect_fd=%d\nclient ip:%s\nclient port:%d\n等待向server端发送数据\n按ENTER键继续\n", connect_fd, client_ip, ntohs(client_addr.sin_port));
    fgetc(stdin);
    char write_buf[MAX_BUFFER_SIZE];
    snprintf((char*)&write_buf, sizeof(write_buf), "testdata");
    int write_size = write(sock_fd, write_buf, strlen(write_buf));
    if (write_size >= 0) {
        printf("向server端发送数据成功, 发送数据, 数据长度:%d\n", write_size);
    } else {
        printf("向server端发送数据失败, error_code=%d\n", write_size);
        return -1;
    }
    char read_buf[MAX_BUFFER_SIZE];
    int read_size;
    while (true) {
        read_size = read(sock_fd, read_buf, sizeof(read_buf));
        if (read_size > 0) {
            printf("收到server端数据, 数据长度:%d\n数据内容:\n", read_size);
            for (int i=0; i<read_size; ++i) {
                printf("%c", read_buf[i]);
            }
            printf("\n");
        } else if(read_size == 0) {
            printf("server端关闭连接\n");
            break;
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
    close(sock_fd);
    printf("client端退出\n");
    return 0;
}
