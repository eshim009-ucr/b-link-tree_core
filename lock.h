#ifndef LOCK_H
#define LOCK_H

#ifdef __cplusplus
#include <atomic>
typedef std::atomic_flag lock_t;
#else
#include <stdatomic.h>
typedef atomic_flag lock_t;
#endif

void lock_p(lock_t *lock);
void lock_v(lock_t *lock);

#endif
