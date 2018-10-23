#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include <string.h>

static void syscall_handler (struct intr_frame *);
static void validate_userpointer (const void *);
static void validate_range (const void *, size_t);
static void validate_string (const char *);
static bool is_valid_fd (int);
static void thread_close_file(int);

/* Lock to be acquired before accessing the filesystem. */
static struct lock fs_lock;

void
syscall_init (void) 
{
	/* Initialize the file system lock. */
	lock_init(&fs_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall"); 
}

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
  
  struct thread *t = thread_current ();

  int i;
  for (i = 2; i < MAX_FILES; i++)
  {
    /* Close files associated with the thread. */
    thread_close_file (i);
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
	validate_range(esp, sizeof(int));
  int fd = *((int *)esp);
  esp += sizeof (int);

	/* Check if stack pointer is still valid. */
  validate_range (esp, sizeof(void *));
  const void *buffer = *((void **)esp);
  validate_userpointer (buffer);
  esp += sizeof (void *);

	/* Check if stack pointer is still valid. */
  validate_range (esp, sizeof(unsigned));
	unsigned size = *((unsigned *)esp);
  esp += sizeof (unsigned);
  
	/* Write characters one-by-one. */
  struct thread *t = thread_current ();
	unsigned charcount = 0;
  
	if(is_valid_fd(fd))
  {
    if(fd == STDOUT_FILENO)
    {
      for (charcount = 0; charcount < size; ++charcount)
      {
        putchar (*((char *) buffer + charcount));
      }
    }
    else if (t -> files[fd] != NULL)
    {
      lock_acquire (&fs_lock);
        charcount = file_write (t -> files[fd], buffer, size);
      lock_release (&fs_lock);
    }
  }

	/* Return the total number of characters written. */
  return charcount;
}

static int
read (void *esp)
{
	/* Obtain file descriptor. */
	validate_range(esp, sizeof(int));
  int fd = *((int *)esp);
  esp += sizeof (int);

	/* Check if stack pointer is still valid. */
  validate_range (esp, sizeof(void *));
  const void *buffer = *((void **)esp);
  validate_userpointer (buffer);
  esp += sizeof (void *);

	/* Check if stack pointer is still valid. */
  validate_range (esp, sizeof(unsigned));
	unsigned size = *((unsigned *)esp);
  esp += sizeof (unsigned);


	/* Write characters one-by-one. */
  struct thread *t = thread_current ();
	unsigned charcount = 0;

  if(is_valid_fd(fd))
  {
    if(fd == STDIN_FILENO)
    {
      for (charcount = 0; charcount < size; ++charcount)
      {
        putchar (*((char *) buffer + charcount));
      }
    }
    else if (t -> files[fd] != NULL)
    {
      lock_acquire (&fs_lock);
        charcount = file_read (t -> files[fd], buffer, size);
      lock_release (&fs_lock);
    }
  }

	/* Return the total number of characters read. */
  return charcount;
}

static int
create (void *esp)
{
	/* Check if valid memory range for char * type. */
	validate_range(esp, sizeof(char *));
	const char * file_name = *((char **) esp);
	esp += sizeof(char *);

	/* Check if the string lies entirely in memory. */
	validate_string(file_name);

	/* Obtain the size of file to be created. */
	validate_range (esp, sizeof(unsigned));
  unsigned file_size = *((unsigned *) esp);
  esp += sizeof (unsigned);

	/* Acquire the lock on the filesystem. */
	lock_acquire(&fs_lock);
	  int status = filesys_create (file_name, file_size);
	lock_release(&fs_lock);

	return status;
}

static int
remove (void *esp)
{
  /* Check if valid memory range for char * type. */
	validate_range(esp, sizeof(char *));
	const char * file_name = *((char **) esp);
	esp += sizeof(char *);

	/* Check if the string lies entirely in memory. */
	validate_string(file_name);

	/* Acquire the lock on the filesystem. */
	lock_acquire (&fs_lock);
  	int status = filesys_remove (file_name);
  lock_release (&fs_lock);

  return status;
}

static int
open (void *esp)
{
  /* Check if valid memory range for char * type. */
  validate_range (esp, sizeof(char *));
  const char *file_name = *((char **) esp);
  esp += sizeof (char *);

  /* Check if the string lies entirely in memory. */
  validate_string (file_name);
  
  /* Acquire the lock on the filesystem. */
  lock_acquire (&fs_lock);
  struct file * f = filesys_open (file_name);
  lock_release (&fs_lock);

  /* Check if valid file pointer. Return index if valid and available. */
  if (f != NULL)
  {
    struct thread * t = thread_current ();
    int i;
    for (i = 2; i < MAX_FILES; i++)
    {
      if (t->files[i] == NULL)
      {
        t->files[i] = f;
        return i;
      }
    }
  }

  return -1;
}

static int
close (void *esp)
{
  /* Check if valid memory for int. */
  validate_range (esp, sizeof(int));

  /* Obtain file descriptor. */
  int fd = *((int *) esp);
  esp += sizeof (int);

  if (is_valid_fd (fd))
  {
    thread_close_file (fd);
  }
}


static int
filesize (void *esp)
{
	/* Check if valid memory for int. */
  validate_range (esp, sizeof(int));

  /* Obtain file descriptor. */
  int fd = *((int *) esp);
  esp += sizeof (int);

  struct thread * t = thread_current ();

	/* Check if valid file descriptor. */
  if ((!is_valid_fd(fd)) || t->files[fd] == NULL)
    return -1;
  
  /* Acquire filesystem lock before checking file. */
  lock_acquire (&fs_lock);
    int file_size = file_length (t->files[fd]);
  lock_release (&fs_lock);

  return file_size;
}

static void
thread_close_file (int fd)
{
  struct thread * t = thread_current();

  if (t->files[fd] != NULL)
  {

  /* Acquire filesystem lock before closing file. */
    lock_acquire (&fs_lock);
      file_close (t -> files[fd]);
      t -> files[fd] = NULL;
    lock_release (&fs_lock);
  }
}

static int
exec (void *esp)
{
  /* In case of bad ptr exit process. 
  Further implementation in upcoming exercises. */
  exit (NULL);
}

static int
wait (void *esp)
{
  /* In case of bad PID exit process. 
  Further implementation in upcoming exercises. */
  
  exit (NULL);
}

static int
seek (void *esp)
{
  validate_range (esp, sizeof(int));
  
  /* Obtain file descriptor */  
  int fd = *((int *)esp);
  esp += sizeof (int);
  
  validate_range (esp, sizeof(unsigned));
  
  /* Obtain new file position */  
  unsigned position = *((unsigned *) esp);
  esp += sizeof (unsigned);
  
  struct thread * t = thread_current ();
  
  if (t->files[fd] != NULL)
  {
  /* Acquire filesystem lock before changing position. */
    lock_acquire (&fs_lock);
      file_seek (t->files[fd], position);
    lock_release (&fs_lock);
  }
}

static int
tell (void *esp)
{
  validate_range (esp, sizeof(int));
  
  /* Obtain file descriptor */
  int fd = *((int *)esp);
  esp += sizeof (int);
  
  struct thread * t = thread_current ();
  
  if (t->files[fd] != NULL)
  {
    lock_acquire (&fs_lock);
      int position = file_tell (t->files[fd]);
    lock_release (&fs_lock);
    return position;
  }
  return -1;
}

/* Skeleton structures for system calls to be implemented. */
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

/* Array of function pointers */
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

/* Validation procedures. */
static void
validate_userpointer (const void *ptr)
{
  uint32_t *pd = thread_current ()->pagedir;
  if ( ptr == NULL || !is_user_vaddr (ptr) || pagedir_get_page (pd, ptr) == NULL)
  {
		/* Terminate process */
    exit (NULL);
  }
}

static void
validate_range (const void *ptr,  size_t sz)
{
	/* Start of range. */
	validate_userpointer(ptr);

	/* End of range. */
	validate_userpointer(ptr + sz - 1);
}

static void
validate_string (const char* s)
{
	/* Check if valid pointer */
	validate_range (s, sizeof(char));

	/* Obtain length of string. */
	size_t string_length = (strlen(s)) * sizeof(char);

	/* Validate range of addresses */
	validate_range(s, string_length);
}

static bool
is_valid_fd (int fd)
{
  return fd >= 0 && fd < MAX_FILES; 
}
