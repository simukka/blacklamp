#ifndef SHARD_H
#define SHARD_H

#include <stdint.h>
#include <stdatomic.h>
#include "mvcc.h"
#include "../storage/wal.h"

#define NUM_SHARDS 16

typedef struct {
    Account *accounts;
    size_t num_accounts;
    WALBuffer wal_buffer;
} Shard;

typedef struct {
    Shard shards[NUM_SHARDS];
} ShardedLedger;

typedef struct {
    Account *account;
    WALBuffer *wal_buffer;
} ShardContext;

// Helper to find the shard for a given account ID
#define GET_SHARD_INDEX(account_id) ((account_id % NUM_SHARDS))

void init_sharded_ledger(ShardedLedger *ledger, size_t accounts_per_shard);
ShardContext* get_account_context(ShardedLedger *ledger, uint64_t account_id);

#endif // SHARD_H
