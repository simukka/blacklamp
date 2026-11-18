#ifndef ATOMIC_ENGINE_H
#define ATOMIC_ENGINE_H

#include "mvcc.h"
#include <stdbool.h>
#include "../storage/wal.h"

/**
 * Updates the balance of an account atomically using a CAS loop.
 * Ensures the balance doesn't go below zero (no overdraft).
 * 
 * @param account Pointer to the Account structure.
 * @param amount The amount to add (positive for credit, negative for debit).
 * @param wal_buffer Pointer to the WAL buffer for this shard.
 * @return true if the operation succeeded, false otherwise.
 */
bool update_balance_atomic(Account *account, Amount amount, WALBuffer *wal_buffer);

#endif // ATOMIC_ENGINE_H
