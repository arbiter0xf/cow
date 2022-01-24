#ifndef COW_SYNCHRONIZE_H_DEFINED
#define COW_SYNCHRONIZE_H_DEFINED

#if 0
// Static initialization used instead.
int cow_locks_init(void);
#endif
int cow_locks_teardown(void);
int cow_lightweight_lock_acquire(void);
int cow_lightweight_lock_release(void);

#endif
