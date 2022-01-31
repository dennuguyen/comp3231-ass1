/* This file will contain your solution. Modify it as you wish. */
#include <types.h>
#include <lib.h>
#include <synch.h>
#include <kern/errno.h>
#include "client_server.h"

struct request_node
{
        request_t *r;
        struct request_node *next;
};

struct request_queue
{
        struct request_node *head;
        struct request_node *tail;
        int size;
};

struct request_queue *q; // work queue
struct lock *l;          // lock variable
struct cv *c;            // condition variable

/* work_queue_enqueue():
 *
 * req: A pointer to a request to be processed. You can assume it is
 * a valid pointer or NULL. You can't assume anything about what it
 * points to, i.e. the internals of the request type.
 *
 * This function is expected to add requests to a single queue for
 * processing. The queue is a queue (FIFO). The function then returns
 * to the caller. It can be called concurrently by multiple threads.
 *
 * Note: The above is a high-level description of behaviour, not
 * detailed psuedo code. Depending on your implementation, more or
 * less code may be required. 
 */
void work_queue_enqueue(request_t *req)
{
        (void)req; /* Avoid compiler error */

        // Entering critical region
        lock_acquire(l);

        // Create node
        struct request_node *curr = kmalloc(sizeof(struct request_node));
        curr->r = req;
        curr->next = NULL;

        // Enqueue
        if (q->head == NULL)
                q->head = curr;
        if (q->tail != NULL)
                q->tail->next = curr;
        q->tail = curr;
        q->size++;

        // Leaving critical region
        cv_signal(c, l);
        lock_release(l);
}

/* 
 * work_queue_get_next():
 *
 * This function is expected to block on a synchronisation primitive
 * until there are one or more requests in the queue for processing.
 *
 * A pointer to the request is removed from the queue and returned to
 * the server.
 * 
 * Note: The above is a high-level description of behaviour, not
 * detailed psuedo code. Depending on your implementation, more or
 * less code may be required.
 */
request_t *work_queue_get_next(void)
{
        // Entering critical region
        lock_acquire(l);

        // Block until one or more requests in queue for processing
        while (q->size == 0)
                cv_wait(c, l);

        // Dequeue
        request_t *req = q->head->r;
        q->head = q->head->next;
        q->size--;
        if (q->head == NULL)
                q->tail = NULL;

        // Leaving critical region
        lock_release(l);
        return req;
}

/*
 * work_queue_setup():
 * 
 * This function is called before the client and server threads are started. It is
 * intended for you to initialise any globals or synchronisation
 * primitives that are needed by your solution.
 *
 * In returns zero on success, or non-zero on failure.
 *
 * You can assume it is not called concurrently.
 */
int work_queue_setup(void)
{
        // Initialise queue
        q = kmalloc(sizeof(struct request_queue));
        if (q == NULL)
                return ENOMEM;
        q->head = NULL;
        q->tail = NULL;
        q->size = 0;

        // Initialise lock variable
        l = lock_create("lock");
        if (l == NULL)
                return ENOMEM;

        // Initialise condition variable
        c = cv_create("condition_variable");
        if (c == NULL)
                return ENOMEM;

        return 0;
}

/* 
 * work_queue_shutdown():
 * 
 * This function is called after the participating threads have
 * exited. Use it to de-allocate or "destroy" anything allocated or created
 * on setup.
 *
 * You can assume it is not called concurrently.
 */
void work_queue_shutdown(void)
{
        // Destroy queue
        struct request_node *next, *curr = q->head;
        while (curr != NULL)
        {
                next = curr->next;
                curr = next;
        }
        kfree(q);

        // Destroy lock variable
        lock_destroy(l);

        // Destroy condition variable
        cv_destroy(c);
}
