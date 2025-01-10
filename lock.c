#include "lock.h"

void lock_p(lock_t *lock) {
	while (
#ifdef __cplusplus
		lock->test_and_set(std::memory_order_acquire)
#else
		atomic_flag_test_and_set(lock)
#endif
	);
}

void lock_v(lock_t *lock) {
#ifdef __cplusplus
	lock->clear(std::memory_order_release);
#else
	atomic_flag_clear(lock);
#endif
}
