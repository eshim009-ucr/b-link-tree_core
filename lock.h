#ifndef LOCK_H
#define LOCK_H

#include <assert.h>

#define CSIM
#if defined(CSIM)
	#ifndef __cplusplus
		#include <stdbool.h>
	#endif
	typedef bool lock_t;
#elif defined(__SYNTHESIS__)
	#include <ap_cint.h>
	typedef uint1 lock_t;
	#define TEST_AND_SET(lockptr) ((*lockptr) == 1)
#else
	#ifdef(__cplusplus)
		#include <atomic>
		typedef std::atomic_flag lock_t;
		#define TEST_AND_SET(lockptr) \
			((lockptr)->test_and_set(std::memory_order_acquire))
	#else
		#include <stdatomic.h>
		typedef atomic_flag lock_t;
		#define TEST_AND_SET(lockptr) (atomic_flag_test_and_set(lockptr))
	#endif
#endif


//! @brief Perform an atomic test-and-set operation on the given lock
static inline bool test_and_set(lock_t *lock) {
#if defined(CSIM) || defined(__SYNTHESIS__)
	bool old = *lock;
	*lock = true;
	return old;
#elif defined(__cplusplus)
	return lock->test_and_set(std::memory_order_acquire);
#else
	return atomic_flag_test_and_set(lock);
#endif
}

//! @brief Initialize a lock to its default (unset) value
static inline void init_lock(lock_t *lock) {
#if defined(CSIM) || defined(__SYNTHESIS__)
	*lock = 0;
#elif !defined(__cplusplus)
	*lock = ATOMIC_FLAG_INIT;
#endif
}

static inline bool lock_test(lock_t const *lock) {
	//! @todo Do this better.
	//! x86 defines atomic_flag as 0 for unset,
	//! but this is not a guarantee for all architectures
	return *((bool *) lock);
}


//! @brief Set the given lock to held
static inline void lock_p(lock_t *lock) {
	while (test_and_set(lock));
}

//! @brief Release the given lock
static inline void lock_v(lock_t *lock) {
	assert(lock_test(lock));
#if defined(CSIM) || defined(__SYNTHESIS__)
	*lock = 0;
#elif defined(__cplusplus)
	lock->clear(std::memory_order_release);
#else
	atomic_flag_clear(lock);
#endif
}

#endif
