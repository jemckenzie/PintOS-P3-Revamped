#ifndef VM_FRAME_H
#define VM_FRAME_H

/* Included for locking/synchronization reasons for later frame eviction. */
#include "threads/synch.h"
#include "vm/page.h"

struct frame
{
    struct lock frame_lock;     /* Prevent frame access from multiple pages. */
    void *base;                 /* Virtual base address of the kernel page associated with the frame. */
    struct page *page;          /* Process page. */
};

void frame_initialize(void);
struct frame *frame_allocate(struct page *);
void lock_frame(struct page *);
void unlock_frame(struct frame *);
void free_frame(struct frame *);

#endif
