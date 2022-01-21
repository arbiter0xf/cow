#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>

#include "cow_synchronize.h"

// Intended for relatively fast operations
static sem_t semaphore_light = {0};

int cow_locks_init(void)
{
	int err = 0;
	int ret = 0;

	ret = sem_init(&semaphore_light, 0, 1);
	if (0 != ret) {
		err = errno;
		fprintf(
			stderr,
			"Failed to initialize semaphore: (%d) %s\n",
			err,
			strerror(err));
	}

	return ret;
}

int cow_locks_teardown(void)
{
	int err = 0;
	int ret = 0;

	sem_destroy(&semaphore_light);
	if (0 != ret) {
		err = errno;
		fprintf(
			stderr,
			"Failed to destroy semaphore: (%d) %s\n",
			err,
			strerror(err));
	}

	return ret;
}

int cow_lightweight_lock_acquire(void)
{
	int err = 0;
	int ret = 0;

	ret = sem_wait(&semaphore_light);
	if (0 != ret) {
		err = errno;
		fprintf(
			stderr,
			"Failed to wait on semaphore: (%d) %s\n",
			err,
			strerror(err));
	}

	return ret;
}

int cow_lightweight_lock_release(void)
{
	int err = 0;
	int ret = 0;

	ret = sem_post(&semaphore_light);
	if (0 != ret) {
		err = errno;
		fprintf(
			stderr,
			"Failed to post on semaphore: (%d) %s\n",
			err,
			strerror(err));
	}

	return ret;
}
