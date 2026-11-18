#include "wal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

void init_wal_buffer(WALBuffer *buffer, size_t capacity) {
    buffer->entries = (WALEntry *)malloc(capacity * sizeof(WALEntry));
    buffer->capacity = capacity;
    atomic_init(&buffer->write_index, 0);
    atomic_init(&buffer->commit_offset, 0);
    for (size_t i = 0; i < capacity; ++i) {
        atomic_init(&buffer->entries[i].ready, false);
    }
}

uint64_t append_to_wal(WALBuffer *buffer, uint64_t tx_id, uint64_t account_id, Amount amount) {
    uint64_t idx = atomic_fetch_add(&buffer->write_index, 1);
    
    if (idx >= buffer->capacity) {
        // In a real system, we'd handle wrap-around or block.
        // For now, we'll just return an error index.
        return (uint64_t)-1;
    }

    buffer->entries[idx].transaction_id = tx_id;
    buffer->entries[idx].account_id = account_id;
    buffer->entries[idx].amount = amount;
    atomic_store(&buffer->entries[idx].ready, true);

    return idx;
}

void flush_wal_buffer_worker(WALBuffer *buffer) {
    uint64_t current_commit = atomic_load(&buffer->commit_offset);
    uint64_t next_write = atomic_load(&buffer->write_index);

    if (current_commit >= next_write) {
        return;
    }

    // Find the highest contiguous ready entry
    uint64_t to_flush = current_commit;
    while (to_flush < next_write && atomic_load(&buffer->entries[to_flush].ready)) {
        to_flush++;
    }

    if (to_flush > current_commit) {
        FILE *fp = fopen("ledger.wal", "ab");
        if (fp) {
            for (uint64_t i = current_commit; i < to_flush; ++i) {
                fwrite(&buffer->entries[i], sizeof(WALEntry), 1, fp);
            }
            fflush(fp);
            fclose(fp);
            atomic_store(&buffer->commit_offset, to_flush);
        }
    }
}

bool wait_for_commit(WALBuffer *buffer, uint64_t write_index) {
    while (atomic_load(&buffer->commit_offset) < write_index) {
        // Spin or yield
        __builtin_ia32_pause(); 
    }
    return true;
}
