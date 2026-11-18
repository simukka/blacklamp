#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../engine/concurrency/atomic_engine.h"

#define MAX_EVENTS 1024
#define BUFFER_SIZE 1024

static int make_socket_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void* wal_worker_thread(void* arg) {
    WALBuffer *buffer = (WALBuffer *)arg;
    while (1) {
        flush_wal_buffer_worker(buffer);
        usleep(1000); // Flush every 1ms
    }
    return NULL;
}

int init_server(Server *server, int port, ShardedLedger *ledger) {
    server->ledger = ledger;
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd == -1) return -1;

    make_socket_non_blocking(server->server_fd);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server->server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        return -1;
    }

    if (listen(server->server_fd, SOMAXCONN) == -1) {
        return -1;
    }

    // Start WAL workers for each shard
    for (int i = 0; i < 16; ++i) { // NUM_SHARDS is 16
        pthread_t thread;
        pthread_create(&thread, NULL, wal_worker_thread, &ledger->shards[i].wal_buffer);
        pthread_detach(thread);
    }

    return 0;
}

void run_server(Server *server) {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) return;

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = server->server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server->server_fd, &ev);

    printf("Server listening on port %d...\n", 443); // This port should be dynamic

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == server->server_fd) {
                struct sockaddr_in client_addr;
                socklen_t addrlen = sizeof(client_addr);
                int client_fd = accept(server->server_fd, (struct sockaddr *)&client_addr, &addrlen);
                if (client_fd != -1) {
                    make_socket_non_blocking(client_fd);
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                }
            } else {
                int fd = events[i].data.fd;
                char buffer[BUFFER_SIZE];
                ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
                if (bytes_read <= 0) {
                    close(fd);
                    continue;
                }
                buffer[bytes_read] = '\0';

                char cmd[10];
                uint64_t acc_id;
                Amount amount;
                if (sscanf(buffer, "%s %lu %ld", cmd, &acc_id, &amount) == 3) {
                    ShardContext *ctx = get_account_context(server->ledger, acc_id);
                    bool success = false;
                    if (strcmp(cmd, "CREDIT") == 0) {
                        success = update_balance_atomic(ctx->account, amount, ctx->wal_buffer);
                    } else if (strcmp(cmd, "DEBIT") == 0) {
                        success = update_balance_atomic(ctx->account, -amount, ctx->wal_buffer);
                    }

                    if (success) {
                        write(fd, "OK\n", 3);
                    } else {
                        write(fd, "ERR\n", 4);
                    }
                    free(ctx);
                }
            }
        }
    }
}

void close_server(Server *server) {
    close(server->server_fd);
}
