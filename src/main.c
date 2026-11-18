#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "engine/concurrency/shard.h"
#include "network/server.h"

int main() {
    ShardedLedger ledger;
    // Each shard has 1000 accounts
    init_sharded_ledger(&ledger, 1000);

    Server server;
    if (init_server(&server, 8080, &ledger) == 0) {
        run_server(&server);
    } else {
        perror("Failed to init server");
    }

    return 0;
}
