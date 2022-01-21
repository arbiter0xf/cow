#ifndef COW_SYNCHRONIZE_H_DEFINED
#define COW_SYNCHRONIZE_H_DEFINED

int cow_locks_init(void);
int cow_locks_teardown(void);
int cow_lightweight_lock_acquire(void);
int cow_lightweight_lock_release(void);

#endif
