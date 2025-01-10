#ifndef LOCK_H
#define LOCK_H


#ifdef __cplusplus
#include <atomic>
typedef std::atomic_flag lock_t;
#else
#include <stdatomic.h>
typedef atomic_flag lock_t;
#endif


static inline void lock_p(lock_t *lock) {
	while (
#ifdef __cplusplus
		lock->test_and_set(std::memory_order_acquire)
#else
		atomic_flag_test_and_set(lock)
#endif
	);
}

static inline void lock_v(lock_t *lock) {
#ifdef __cplusplus
	lock->clear(std::memory_order_release);
#else
	atomic_flag_clear(lock);
#endif
}


#endif
