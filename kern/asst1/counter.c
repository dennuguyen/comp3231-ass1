#include "opt-synchprobs.h"
#include "counter.h"
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>

/*
 * Declare the counter variable that all threads increment or decrement
 * via the interface provided here.
 *
 * Declaring it "volatile" instructs the compiler to always (re)read the
 * variable from memory and not to optimise by keeping the value in a process 
 * register and avoid memory references.
 *
 * NOTE: The volatile declaration is actually not needed for the provided code 
 * as the variable is only loaded once in each function.
 */

static volatile int counter;
static struct lock *counter_lock;

void counter_increment(void)
{
        lock_acquire(counter_lock);
        counter++;
        lock_release(counter_lock);
}

void counter_decrement(void)
{
        lock_acquire(counter_lock);
        counter--;
        lock_release(counter_lock);
}

int counter_initialise(int val)
{
        // Initialise lock
        counter_lock = lock_create("counter_lock");
        if (counter_lock == NULL)
                return ENOMEM;

        // Assign counter value
        counter = val;

        // Exit success
        return 0;
}

int counter_read_and_destroy(void)
{
        // Free the lock
        lock_destroy(counter_lock);
        return counter;
}
