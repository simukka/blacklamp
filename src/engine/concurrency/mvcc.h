#ifndef MVCC_H
#define MVCC_H

#include <stdint.h>
#include <stdatomic.h>
#include "../utils/fixed_point.h"

typedef struct {
    _Atomic uint64_t account_id;
    _Atomic Amount balance;
    _Atomic uint64_t version;
} Account;

#endif // MVCC_H
