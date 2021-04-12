#ifndef VM_PAGE_H
#define VM_PAGE_H
#include <hash.h>
#include "devices/block.h"
#include "filesys/off_t.h"
#include "threads/synch.h"

/* Sinpple page element, used for the supplemental page table. */
struct page
{
    void * addr;                     /* Virtual address of page. */
    struct thread *thread;          /* Thread that "owns" the page. */
    bool read_only;                 /* Indicates the page information is read-only */

    struct frame * frame;            /* Frame this page owns */

    struct hash_elem hash_elem;     /* hash element of page */

    block_sector_t sector;          /* Sector of the block device swap area */


};
void page_destroy(struct hash_elem *);
bool page_compare(struct hash_elem *, struct hash_elem *);
unsigned page_hash(struct hash_elem *);
void pagetable_teardown(void);
void deallocate_page(void *);
struct page *page_for_address(void *);
struct page *allocate_page(void *, bool);


#endif