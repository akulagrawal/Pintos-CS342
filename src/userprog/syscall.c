#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <string.h>

static void syscall_handler (struct intr_frame *);
static void validate_userpointer (const void *);

static int
halt (void *esp)
{
	/* Shutdown PintOS. */
	power_off();
}

int
exit (void *esp)
{
  /* Get the status from the stack. */
  int status = 0;
  if (esp != NULL){
    status = *((int *)esp);
    esp += sizeof (int);
  }
  else {
    status = -1;
  }
  
  char *name = thread_current () -> name;
	char *saveptr;

	/* Get the first word before a space by tokenizing. */
  name = strtok_r (name, " ", &saveptr);
  
  printf ("%s: exit(%d)\n", name, status);
  thread_exit();
}

static int
write (void *esp)
{
	/* Obtain file descriptor. */
  int fd = *((int *)esp);
  esp += sizeof (int);

	/* Check if stack pointer is still valid. */
  validate_userpointer (esp);

  const void *buffer = *((void **)esp);
  validate_userpointer (buffer);
  esp += sizeof (void *);

  validate_userpointer (esp);
  
	unsigned size = *((unsigned *)esp);
  esp += sizeof (unsigned);
  
	/* Write characters one-by-one. */
	unsigned charcount = 0;
  if(fd == STDOUT_FILENO)
  {
    for (charcount = 0; charcount < size; ++charcount)
    {
      putchar (*((char *) buffer + charcount));
    }
  }

	/* Return the total number of characters written. */
  return charcount;
}

/* Skeleton structures for system calls to be implemented. */
static int
exec (void *esp)
{
  thread_exit ();
}

static int
wait (void *esp)
{
  thread_exit ();
}

static int
create (void *esp)
{
  thread_exit ();
}

static int
remove (void *esp)
{
  thread_exit ();
}

static int
open (void *esp)
{
  thread_exit ();
}

static int
filesize (void *esp)
{
  thread_exit ();
}

static int
read (void *esp)
{
  thread_exit ();
}

static int
seek (void *esp)
{
  thread_exit ();
}

static int
tell (void *esp)
{
  thread_exit ();
}

static int
close (void *esp)
{
  thread_exit ();
}

static int
mmap (void *esp)
{
  thread_exit ();
}

static int
munmap (void *esp)
{
  thread_exit ();
}

static int
chdir (void *esp)
{
  thread_exit ();
}

static int
mkdir (void *esp)
{
  thread_exit ();
}

static int
readdir (void *esp)
{
  thread_exit ();
}

static int
isdir (void *esp)
{
  thread_exit ();
}

static int
inumber (void *esp)
{
  thread_exit ();
}

static int (*syscalls []) (void *) =
  {
    halt,
    exit,
    exec,
    wait,
    create,
    remove,
    open,
    filesize,
    read,
    write,
    seek,
    tell,
    close,

    mmap,
    munmap,

    chdir,
    mkdir,
    readdir,
    isdir,
    inumber
  };

const int syscall_size = sizeof (syscalls) / sizeof (syscalls[0]);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall"); 
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  void *esp = f -> esp;
  validate_userpointer (esp);

	/* Obtain the system call number. */
  int syscall_num = *((int *) esp);
  esp += sizeof(int);
  validate_userpointer (esp);

	/* Check if valid system call index. */
  if (syscall_num >= 0 && syscall_num < syscall_size)
  {
    int (*function) (void *) = syscalls[syscall_num];
    int ret = function (esp);
    f -> eax = ret;
  }
  else
  {
    printf ("\n Error. Invalid system call number.");
    thread_exit ();
  }
}

static void
validate_userpointer (const void *ptr)
{
  uint32_t *pd = thread_current ()->pagedir;
  if ( ptr == NULL || !is_user_vaddr (ptr) || pagedir_get_page (pd, ptr) == NULL)
  {
    exit (NULL);
  }
}
