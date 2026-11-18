# High-Performance Financial Ledger Database

A production-grade financial ledger database implemented in C, designed to handle high-throughput transactions (target 100,000 RPS) while maintaining strict ACID compliance.

## Features

- **Lock-Free Concurrency**: Utilizes `stdatomic.h` and CAS (Compare-And-Swap) loops to minimize mutex contention and maximize throughput in the atomic engine.
- **Group Commit**: Implements a high-speed Write-Ahead Log (WAL) with a group commit mechanism, batching multiple transactions into a single disk syscall to overcome I/O bottlenecks.
- **Horizontal Sharding**: Supports partitioning the account array across multiple shards and threads to ensure linear scalability.
- **Double-Entry Bookkeeping**: Ensures data integrity by adhering to standard financial accounting principles.
- **Fixed-Point Arithmetic**: Uses `int64_t` fixed-point representation to ensure precision and atomicity for currency balances, avoiding the pitfalls of floating-point arithmetic.
- **Non-blocking I/O**: Features an `epoll`-based networking layer to handle thousands of concurrent TCP connections efficiently.

## Technical Constraints

- **Language**: Standard C (C11).
- **Concurrency Model**: Lock-free/highly concurrent architecture.
- **Data Integrity**: WAL-backed persistence with MVCC-inspired versioning.
- **Memory Management**: Optimized for high-speed I/O and buffer pool efficiency.

## Architecture

- **Storage Engine**: LSM-Tree inspired write path with MVCC for concurrent reads.
- **Network Layer**: Non-blocking TCP server using `epoll`.
- **Atomic Engine**: Core logic for balance updates with zero-overdraft guarantees.

## Building and Running

### Prerequisites
- GCC or Clang compiler.
- Make sure you have `pthread` support installed.

### Compilation
```bash
gcc -O3 src/main.c src/engine/concurrency/*.c src/engine/storage/*.c src/network/*.c -o ledger_db -lpthread
```

### Running the Server
```bash
./ledger_db
```

### Testing the Server
You can use `nc` or `telnet` to send commands:
```bash
# Example: Deposit 50 units to account 1000
nc localhost 8080
CREDIT 1000 50

# Example: Withdraw 20 units from account 1000
DEBIT 1000 20
```

## License
[Specify License here]
