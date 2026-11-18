#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>
#include "../engine/concurrency/shard.h"

typedef struct {
    int server_fd;
    ShardedLedger *ledger;
} Server;

int init_server(Server *server, int port, ShardedLedger *ledger);
void run_server(Server *server);
void close_server(Server *server);

#endif // SERVER_H
