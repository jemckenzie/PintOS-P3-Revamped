#include "vm/frame.h"
#include <stdio.h>
#include "devices/timer.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "vm/page.h"

// PROJECT 3 //
/* Keeps track of the frame struct as a pointer, which effectively stores it as a "list" of sorts. */
struct frame *frames;
/* Frame count will help us keep track of this "list". */
static size_t frame_count;

/* Helps us with eviction and allocation of frames */
struct lock frame_lock;

/* Initializes the frame table. */
void frame_initialize(void)
{
    void * base;

    lock_init(&frame_lock);
    /* Initialize enough space to cover the number of frames in pages of 4kB as is stored in RAM. */
    frames = malloc(sizeof *frames * init_ram_pages);

    /* If for some reason we can't malloc these frames, panic the kernel. */
    if(frames == NULL)
    {
        PANIC("Frame table malloc failed.");
    }

    /* Each time through, a new page base address is given by palloc_get_page function. */
    while(base = (void *)palloc_get_page(PAL_USER) != NULL)
    {
        frame_count++;
        struct frame *new_frame = &frames[frame_count];
        lock_init(&new_frame->frame_lock);
        /* Base address is given by the loop, no page is associated upon initialization. */
        new_frame->base = base;
        new_frame->page = NULL;
    }
}

/* This function will allocate a frame to the page passed in. */
struct frame *frame_allocate(struct page *page)
{
    /* Will use to iterate over the pointers to the frames we have stored continously in memory. */
    size_t i;
    i = 0;
    lock_acquire(&frame_lock);

    /* First thing we do, we look for a free frame so that we do not have to evict another page. */
    while(i < frame_count)
    {
        struct frame *frame = &frames[i];
        /* If the frame is not locked by another page already, and it's empty. */
        if(!lock_try_acquire(&frame->frame_lock) && frame->page == NULL)
        {
            frame->page = page;
            lock_release(&frame_lock);
            return frame;
        }
        lock_release(&frame->frame_lock);
        i++;
    }

    //Eviction could go here.

    lock_release(&frame_lock);
    return NULL;
}

/* Locks a page's frame into memory if page has a frame.
Use unlock_frame to unlock the frame from the page. */
void lock_frame(struct page * page)
{
    struct frame * frame = page->frame;
    if(frame != NULL)
    {
        lock_acquire(&frame->frame_lock);
        if(frame != page->frame)
        {
            lock_release(&frame->frame_lock);
            /* Panic the kernel if this can't be done. */
            ASSERT(page->frame == NULL)
        }

    }
}

/* Simply unlocks the frame so that it can be evicted or otherwise operated upon. */
void unlock_frame(struct frame *frame)
{
    /* Check if thread owns the frame, if not, panic. */
    ASSERT(lock_held_by_current_thread(&frame->frame_lock));
    lock_release(&frame->frame_lock);
}

void free_frame(struct frame *frame)
{
    /* Check if thread owns the frame, if not, panic. */
    ASSERT(lock_held_by_current_thread(&frame->frame_lock));
    /* Dissociate any pages from this frame. */
    frame->page = NULL;
    lock_release(&frame->frame_lock);
}