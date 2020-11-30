// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

void freerange(void *pa_start, void *pa_end);
int get_rc(void *pa);
int incre_rc(void *pa);
int decre_rc(void *pa);


extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// COW
struct {
	struct spinlock lock;
	int refcnt[(PHYSTOP - KERNBASE) / PGSIZE];
} kcnt;

int
get_rc(void *pa)
{
	uint64 ind = PA2IDX(pa);
	int ret;
	
	acquire(&kcnt.lock);
	ret = kcnt.refcnt[ind];
	release(&kcnt.lock);
	
	return ret;
}

int
incre_rc(void *pa)
{
	uint64 ind = PA2IDX(pa);
	int ret;
	
	acquire(&kcnt.lock);
	kcnt.refcnt[ind] += 1;
	ret = kcnt.refcnt[ind];
	release(&kcnt.lock);
	
	return ret;
}

int
decre_rc(void *pa)
{
	uint64 ind = PA2IDX(pa);
	int mark = 0, ret;
	
	acquire(&kcnt.lock);
	kcnt.refcnt[ind] -= 1;
	if(kcnt.refcnt[ind] < 0)
		mark = 1;
	ret = kcnt.refcnt[ind];
	release(&kcnt.lock);
	
	if(mark)
		panic("decre_rc: negative reference");
	return ret;
}


void
kinit()
{
  initlock(&kmem.lock, "kmem");
  
  // COW
  initlock(&kcnt.lock, "kcnt");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  
  // COW
  // Set reference to every entry in the RAM to 1
  // to let kfree free them shortly afterwards.
  acquire(&kcnt.lock);
  for(uint64 i = (uint64)p;i + PGSIZE <= (uint64) pa_end; i += PGSIZE)
  	kcnt.refcnt[PA2IDX(i)] = 1;
  release(&kcnt.lock);
  
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

	// COW
	// First decrement the reference count & if no reference, free the page
	// Atomic operation 
	if(decre_rc(pa) == 0){
  	// Fill with junk to catch dangling refs.
  	memset(pa, 1, PGSIZE);
  	r = (struct run*)pa;

  	acquire(&kmem.lock);
  	r->next = kmem.freelist;
  	kmem.freelist = r;
  	release(&kmem.lock);
  }
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

  if(r){
  	// COW
		// reference count should be raised from 0 to 1
		if(incre_rc(r) != 1)
			panic("kalloc: not a free page");
    memset((char*)r, 5, PGSIZE); // fill with junk
  }
  return (void*)r;
}
