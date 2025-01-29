#ifndef LOCK_H
#define LOCK_H

#ifdef __SYNTHESIS__
#include <ap_cint.h>
typedef uint1 lock_t;
#elif defined(__cplusplus)
#include <atomic>
typedef std::atomic_flag lock_t;
#else
#include <stdatomic.h>
typedef atomic_flag lock_t;
#endif

#include <stdbool.h>


//! @brief Set the given lock to held
static inline void lock_p(lock_t *lock) {
	while (
#ifdef __SYNTHESIS__
		*lock
#elif defined(__cplusplus)
		lock->test_and_set(std::memory_order_acquire)
#else
		atomic_flag_test_and_set(lock)
#endif
	) {
#ifdef __SYNTHESIS__
		*lock = 1;
#endif
	}
}

//! @brief Release the given lock
static inline void lock_v(lock_t *lock) {
#ifdef __SYNTHESIS__
	*lock = 0;
#elif defined(__cplusplus)
	lock->clear(std::memory_order_release);
#else
	atomic_flag_clear(lock);
#endif
}

static inline bool lock_test(lock_t const *lock) {
	//! @todo Do this better.
	//! x86 defines atomic_flag as 0 for unset,
	//! but this is not a guarantee for all architectures
	return *((bool*) lock);
}

#endif
