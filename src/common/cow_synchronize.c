#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "cow_synchronize.h"

// Intended for relatively fast operations
static pthread_mutex_t mutex_light = PTHREAD_MUTEX_INITIALIZER;

#if 0
/*
 * Static initialization used instead.
 */
int cow_locks_init(void)
{
	int err = 0;
	int ret = 0;

	ret = pthread_mutex_init(&mutex_light, NULL);
	if (0 != ret) {
		err = errno;
		fprintf(
			stderr,
			"Failed to initialize mutex: (%d) %s\n",
			err,
			strerror(err));
	}

	return ret;
}
#endif

int cow_locks_teardown(void)
{
	int err = 0;
	int ret = 0;

	ret = pthread_mutex_destroy(&mutex_light);
	if (0 != ret) {
		err = errno;
		fprintf(
			stderr,
			"Failed to destroy mutex: (%d) %s\n",
			err,
			strerror(err));
	}

	return ret;
}

int cow_lightweight_lock_acquire(void)
{
	int err = 0;
	int ret = 0;

	ret = pthread_mutex_lock(&mutex_light);
	if (0 != ret) {
		err = errno;
		fprintf(
			stderr,
			"Failed to lock mutex: (%d) %s\n",
			err,
			strerror(err));
	}

	return ret;
}

int cow_lightweight_lock_release(void)
{
	int err = 0;
	int ret = 0;

	ret = pthread_mutex_unlock(&mutex_light);
	if (0 != ret) {
		err = errno;
		fprintf(
			stderr,
			"Failed to unlock mutex: (%d) %s\n",
			err,
			strerror(err));
	}

	return ret;
}
