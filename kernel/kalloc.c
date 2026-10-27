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

#ifdef LAB_PGTBL
// A handful of 2MB-aligned physical regions reserved at the top of
// RAM for superpages. kalloc() never hands these out.
#define NSUPERPG 16
#define SUPERBASE (PHYSTOP - NSUPERPG * SUPERPGSIZE)

struct {
  struct spinlock lock;
  struct run *freelist;
} smem;
#endif

void
kinit()
{
  initlock(&kmem.lock, "kmem");
#ifdef LAB_PGTBL
  initlock(&smem.lock, "smem");
  freerange(end, (void*)SUPERBASE);
  for(uint64 p = SUPERBASE; p + SUPERPGSIZE <= PHYSTOP; p += SUPERPGSIZE)
    superfree((void*)p);
#else
  freerange(end, (void*)PHYSTOP);
#endif
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

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
  return (void*)r;
}

#ifdef LAB_PGTBL
// Allocate one 2MB, 2MB-aligned region of physical memory.
// Returns 0 if none is available.
void *
superalloc(void)
{
  struct run *r;

  acquire(&smem.lock);
  r = smem.freelist;
  if(r)
    smem.freelist = r->next;
  release(&smem.lock);

  if(r)
    memset((char*)r, 5, SUPERPGSIZE); // fill with junk
  return (void*)r;
}

// Free a 2MB region previously returned by superalloc().
void
superfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % SUPERPGSIZE) != 0 || (uint64)pa < SUPERBASE ||
     (uint64)pa >= PHYSTOP)
    panic("superfree");

  memset(pa, 1, SUPERPGSIZE);

  r = (struct run*)pa;
  acquire(&smem.lock);
  r->next = smem.freelist;
  smem.freelist = r;
  release(&smem.lock);
}
#endif
