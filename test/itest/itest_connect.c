#include <criterion/criterion.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "cow_server.h"
#include "cow_state.h"

#define ITEST_THREAD_AMOUNT 2
#define ITEST_LISTEN_THREAD 0

static pthread_attr_t thread_attributes[ITEST_THREAD_AMOUNT] = {0};
static pthread_t thread_ids[ITEST_THREAD_AMOUNT] = {0};
static int thread_numbers[ITEST_THREAD_AMOUNT] = {0};

static int server_clean_exit = 0;

static int itest_threads_init(void)
{
	int ret = 0;
	static int init_done = 0;

	if (init_done) {
		return 0;
	}

	for (int i = 0; i < ITEST_THREAD_AMOUNT; i++) {
		ret = pthread_attr_init(&(thread_attributes[i]));
		if (0 != ret) {
			cr_log_error(
				"Failed to initialize thread attribute object (%d)",
				ret);
			return ret;
		}

		thread_numbers[i] = i;
	}

	init_done = 1;

	return 0;
}

static int itest_threads_cleanup(void)
{
	int ret = 0;

	for (int i = 0; i < ITEST_THREAD_AMOUNT; i++) {
		pthread_attr_destroy(&(thread_attributes[i]));
		if (0 != ret) {
			cr_log_error(
				"Failed to destroy thread attribute object (%d)",
				ret);
			return ret;
		}
	}

	return 0;
}

void* run_cow_server(void*)
{
	int ret = 0;

	ret = cow_server_serve();
	if (0 == ret) {
		server_clean_exit = 1;
	} else {
		server_clean_exit = 0;
	}

	return 0;
}

int create_thread_with_listening_server(void)
{
	int ret = 0;

	ret = itest_threads_init();
	if (0 != ret) {
		cr_log_error("Failed to init threads");
		return ret;
	}

	ret = pthread_create(
			&(thread_ids[ITEST_LISTEN_THREAD]),
			&(thread_attributes[ITEST_LISTEN_THREAD]),
			&run_cow_server,
			&(thread_numbers[ITEST_LISTEN_THREAD]));
	if (0 != ret) {
		cr_log_error("Failed to create listen thread (%d)", ret);
		return ret;
	}

	return 0;
}

int wait_for_server_to_be_listening(void)
{
	int i = 0;
	int timeout = 0;

	while(1) {
		if (cow_state_listening_get()) {
			break;
		}

		if (i > 2) {
			timeout = 1;
			break;
		}

		/*
		 * Used time(1) to see that execution up to listen call +
		 * teardown took 5ms.
		 */
		usleep(5000);
		i++;
	}
	if (0 != timeout) {
		cr_log_error("Timeout reached while waiting for "
				"server to start listening");
		return -1;
	}

	return 0;
}

Test(connect, listening_server_does_clean_shutdown_on_SIGINT)
{
	void* thread_result = 0;
	int ret = 0;

	ret = create_thread_with_listening_server();
	cr_expect(0 == ret);

	ret = wait_for_server_to_be_listening();
	cr_expect(0 == ret);

	cr_expect(0 == server_clean_exit);
	ret = pthread_kill(thread_ids[ITEST_LISTEN_THREAD], SIGINT);
	cr_expect(0 == ret);

	ret = pthread_join(thread_ids[ITEST_LISTEN_THREAD], &thread_result);

	itest_threads_cleanup();

	cr_expect(1 == server_clean_exit);
}
