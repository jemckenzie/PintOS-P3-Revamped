#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void syscall_exit (void);

///Project 3///
//intergrate mmapping to fd struct
typedef int mapid_t;

#endif /* userprog/syscall.h */
