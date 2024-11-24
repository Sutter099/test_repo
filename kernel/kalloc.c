// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct spinlock page_counter_lock;
struct page_counter_t page_counter;

void
kinit()
{
  struct run *r;
  // int i;

  initlock(&page_counter_lock, "page_counter");
  page_counter.working = 0;
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);

  page_counter.working = 1;
  page_counter.length = 0;
  r = kmem.freelist;
  while (r != 0) {
    page_counter.length++;
    page_counter.counter[(uint64)r / 4096] = 0;
    r = r->next;
  }
  printf("number of pages: %d\n", page_counter.length);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&page_counter_lock);
  if (page_counter.working &&
      --page_counter.counter[(uint64)pa / 4096] > 0) {
    release(&page_counter_lock);
    return;
  }
  release(&page_counter_lock);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  acquire(&page_counter_lock);
  page_counter.counter[(uint64)r / 4096]++;
  release(&page_counter_lock);

  return (void*)r;
}
