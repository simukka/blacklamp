#ifndef WAL_H
#define WAL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "../../utils/fixed_point.h"

typedef struct {
    uint64_t transaction_id;
    uint64_t account_id;
    Amount amount;
    _Atomic bool ready;
} WALEntry;

typedef struct {
    WALEntry *entries;
    size_t capacity;
    _Atomic uint64_t write_index;
    _Atomic uint64_t commit_offset;
} WALBuffer;

// Basic WAL functions
void init_wal_buffer(WALBuffer *buffer, size_t capacity);
uint64_t append_to_wal(WALBuffer *buffer, uint64_t tx_id, uint64_t account_id, Amount amount);
void flush_wal_buffer_worker(WALBuffer *buffer);
bool wait_for_commit(WALBuffer *buffer, uint64_t write_index);

#endif // WAL_H
