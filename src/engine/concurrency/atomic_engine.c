#include "atomic_engine.h"

bool update_balance_atomic(Account *account, Amount amount, WALBuffer *wal_buffer) {
    Amount current_balance = atomic_load(&account->balance);
    Amount new_balance;

    do {
        new_balance = current_balance + amount;

        // Prevent overdraft
        if (new_balance < 0) {
            return false;
        }

        // Try to update balance with CAS
    } while (!atomic_compare_exchange_weak(&account->balance, &current_balance, new_balance));

    // Increment version for MVCC
    atomic_fetch_add(&account->version, 1);

    // Log to WAL
    uint64_t write_idx = append_to_wal(wal_buffer, 0, account->account_id, amount);
    if (write_idx == (uint64_t)-1) {
        // In a real system, we'd handle buffer full
        return false;
    }

    // Wait for commit
    wait_for_commit(wal_buffer, write_idx);

    return true;
}
