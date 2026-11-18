#include "shard.h"
#include <stdlib.h>
#include <stdio.h>

void init_sharded_ledger(ShardedLedger *ledger, size_t accounts_per_shard) {
    for (int i = 0; i < NUM_SHARDS; ++i) {
        ledger->shards[i].accounts = (Account *)malloc(accounts_per_shard * sizeof(Account));
        ledger->shards[i].num_accounts = accounts_per_shard;
        init_wal_buffer(&ledger->shards[i].wal_buffer, 1024);
        
        for (size_t j = 0; j < accounts_per_shard; ++j) {
            uint64_t account_id = (uint64_t)i * accounts_per_shard + j;
            atomic_init(&ledger->shards[i].accounts[j].account_id, account_id);
            atomic_init(&ledger->shards[i].accounts[j].balance, 0);
            atomic_init(&ledger->shards[i].accounts[j].version, 0);
        }
    }
}

ShardContext* get_account_context(ShardedLedger *ledger, uint64_t account_id) {
    uint64_t shard_idx = GET_SHARD_INDEX(account_id);
    uint64_t local_idx = account_id % ledger->shards[shard_idx].num_accounts;
    
    ShardContext *ctx = (ShardContext *)malloc(sizeof(ShardContext));
    ctx->account = &ledger->shards[shard_idx].accounts[local_idx];
    ctx->wal_buffer = &ledger->shards[shard_idx].wal_buffer;
    return ctx;
}
