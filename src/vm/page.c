#include <stdio.h>
#include <string.h>
#include "vm/page.h"
#include "vm/frame.h"
#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include <hash.h>


/* Function is used to map a page at vaddr to the page hash table structure.  Returns page
if allocation is successful. */
struct page *allocate_page(void *vaddr, bool read_only)
{
    struct thread *t = thread_current();
    struct page *p = malloc(sizeof *p);
    if(p != NULL)
    {
        /* Round the page address to the nearest page boundary. */
        p->addr = pg_round_down(vaddr);
        p->read_only = read_only;
        // p private field??? -> USED FOR SWAPPING?

        p->frame = NULL;

        p->sector = (block_sector_t) - 1;

        //MAY NEED TO ADD FOR MEMORY MAPPED FILES
        p->thread = thread_current();
        /* If this if check passes, this means we already have mapped the vaddr. */
        if(hash_insert(t->pages, &p->hash_elem) != NULL)
        {
            free(p);
            p = NULL;
        }
    }
    return p;
}

//UNFINISHED FUNCTION.
/* This returns a page for an address passed into it(void *address).  May later implement stack growth here. */
struct page *page_for_address(void *address)
{
    /* We're in user memory. */
    if(address < PHYS_BASE)
    {
        struct page p;
        struct hash_elem *e;

        /* Grab the address via the built in paging functionality. */
        p.addr = (void *)pg_round_down(address);
        e = hash_find(thread_current()->pages, &p.hash_elem);
        //e is found in page hash table.
        if(e != NULL)
            return hash_entry(e, struct page, hash_elem);
    }
}

/* Removes the page at the given address and removes it from the page table. */
void deallocate_page(void *vaddr)
{
    /* Find the page given the virtual address. */
    struct page *page = page_for_address(vaddr);
    if(page != NULL)
    {
        lock_frame(page);
        if(page->frame != NULL)
        {
            struct frame *frame = page->frame;
            //ADDITIONS MUST GO HERE.

            free_frame(frame);
        }
        /* Remove it from the hash table and free its allocated memory. */
        hash_delete(thread_current()->pages, &page->hash_elem);
        free(page);
    }
}

/* This function destroys the page associated with the passed-in hash table element p in the current thread's
page table.  Uses the functionality described in frame.c in order to lock and then free the frame associated with the page,
 and then free the associated page from memory using free(). */
void page_destroy(struct hash_elem *p)
{
    /* Grabs the page associated with passed in hash element. */
    struct page *tmp = hash_entry(p, struct page, hash_elem);
    //Lock the frame associated with the page, defined in the frame.c file.
    frame_lock(tmp);
    /* If it has a non-NULL frame, need to free it to destroy the page properly. */
    if(tmp->frame)
    {
        free_frame(tmp->frame);
    }
    free(tmp);
}

/* This function is used to destroy the page table(which is stored as a hash table in our implementation) of the current process. */
void pagetable_teardown(void)
{
    /* Grab the page table from the current thread, which we store as a hash table. */
    struct hash *current_pages = thread_current()->pages;
    /* Pass in the hash retrieved from the current thread to the destructor function of hash.c */
    if(current_pages != NULL)
    {
        hash_destroy(current_pages, page_destroy);
    }
}

/* This compares the address space of two pages a & b, returns true if the address of a is before b. Necessary for the hash_init functionality. */
bool page_compare(struct hash_elem *a_elem, struct hash_elem *b_elem)
{
    const struct page *a_page = hash_entry(a_elem, struct page, hash_elem);
    const struct page *b_page = hash_entry(b_elem, struct page, hash_elem);
    return a_page->addr < b_page->addr;
}

/* Function that returns a hash value for the hash element. */
unsigned page_hash(struct hash_elem *a)
{
    /* Grab the page from the hash elem entry. */
    const struct page *page = hash_entry(a, struct page, hash_elem);
    /* We shift by the offset(PGBITS) to get to the actual page number. */
    return ((uintptr_t) page->addr) >> PGBITS;
}