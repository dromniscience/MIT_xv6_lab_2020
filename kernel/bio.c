// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  // struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  // struct buf head;

  // Lock
  struct spinlock lock[NBUCKS];
  struct buf head[NBUCKS];
} bcache;

// Lock
struct buf *
remove(struct buf *b)
{
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = b->prev = (struct buf *)0;
    return b;
}

// Lock
struct buf *
insert(struct buf *b, struct buf *head)
{
    b->next = head->next;
    b->prev = head;
    head->next->prev = b;
    head->next = b;
    return b;
}

int
hash(uint blockno)
{
    return blockno % NBUCKS;
}

void
binit(void)
{
  struct buf *b;

	// Lock
	for(int i = 0;i < NBUCKS;++i)
		initlock(&bcache.lock[i], "bcache");

//  // Create linked list of buffers
//  bcache.head.prev = &bcache.head;
//  bcache.head.next = &bcache.head;
//  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
//    b->next = bcache.head.next;
//    b->prev = &bcache.head;
//    initsleeplock(&b->lock, "buffer");
//    bcache.head.next->prev = b;
//    bcache.head.next = b;
//  }

    // Lock
    for(b = bcache.head; b < bcache.head + NBUCKS; ++b){
        b->next = b;
        b->prev = b;
    }

    for(int i = 0;i < NBUF;++i)
        insert(bcache.buf + i, bcache.head);
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int ind = hash(blockno);

  acquire(&bcache.lock[ind]);

  // Is the block already cached?
  for(b = bcache.head[ind].next; b != &bcache.head[ind]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[ind]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(int turn = 0; turn < NBUCKS;++turn){
    int tmp = (ind + turn) % NBUCKS;
    if(tmp != ind)
    	acquire(&bcache.lock[tmp]);
    for(b = bcache.head[tmp].prev; b != &bcache.head[tmp]; b = b->prev){
        if(b->refcnt)
        	continue;
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        remove(b);
        if(tmp != ind)
        	release(&bcache.lock[tmp]);
        insert(b, &bcache.head[ind]);
        release(&bcache.lock[ind]);
        acquiresleep(&b->lock);
        return b;
    }
    if(tmp != ind)
    	release(&bcache.lock[tmp]);
  }

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
	int ind = hash(b->blockno);
  if(!holdingsleep(&b->lock))
    panic("brelse");
  
  releasesleep(&b->lock);

  acquire(&bcache.lock[ind]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    remove(b);
    insert(b, bcache.head[ind].prev);
  }

  release(&bcache.lock[ind]);
}

void
bpin(struct buf *b) {
	// Lock
	int ind = hash(b->blockno);
  acquire(&bcache.lock[ind]);
  b->refcnt++;
  release(&bcache.lock[ind]);
}

void
bunpin(struct buf *b) {
	// Lock
	int ind = hash(b->blockno);
  acquire(&bcache.lock[ind]);
  b->refcnt--;
  release(&bcache.lock[ind]);
}


// The very first version which is correct even under tight conditions,
// yet it fails to live up to the temporal requirement.


//// Buffer cache.
////
//// The buffer cache is a linked list of buf structures holding
//// cached copies of disk block contents.  Caching disk blocks
//// in memory reduces the number of disk reads and also provides
//// a synchronization point for disk blocks used by multiple processes.
////
//// Interface:
//// * To get a buffer for a particular disk block, call bread.
//// * After changing buffer data, call bwrite to write it to disk.
//// * When done with the buffer, call brelse.
//// * Do not use the buffer after calling brelse.
//// * Only one process at a time can use a buffer,
////     so do not keep them longer than necessary.
//
//
//#include "types.h"
//#include "param.h"
//#include "spinlock.h"
//#include "sleeplock.h"
//#include "riscv.h"
//#include "defs.h"
//#include "fs.h"
//#include "buf.h"
//
//struct {
//  // struct spinlock lock;
//  struct buf buf[NBUF];
//
//  // Linked list of all buffers, through prev/next.
//  // Sorted by how recently the buffer was used.
//  // head.next is most recent, head.prev is least.
//  // struct buf head;
//
//  // Lock
//  struct spinlock headlock[NBUCKS];
//  struct spinlock freelock[NBUCKS];
//  struct buf head[NBUCKS];
//  struct buf free[NBUCKS];
//} bcache;
//
//// Lock
//struct buf *
//remove(struct buf *b)
//{
//    b->next->prev = b->prev;
//    b->prev->next = b->next;
//    b->next = b->prev = (struct buf *)0;
//    return b;
//}
//
//// Lock
//struct buf *
//insert(struct buf *b, struct buf *head)
//{
//    b->next = head->next;
//    b->prev = head;
//    head->next->prev = b;
//    head->next = b;
//    return b;
//}
//
//int
//hash(uint blockno)
//{
//    return blockno % NBUCKS;
//}
//
//void
//binit(void)
//{
//  struct buf *b;
//
//	// Lock
//	for(int i = 0;i < NBUCKS;++i){
//		initlock(&bcache.headlock[i], "bcache");
//		initlock(&bcache.freelock[i], "bcache");
//	}
//
////  // Create linked list of buffers
////  bcache.head.prev = &bcache.head;
////  bcache.head.next = &bcache.head;
////  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
////    b->next = bcache.head.next;
////    b->prev = &bcache.head;
////    initsleeplock(&b->lock, "buffer");
////    bcache.head.next->prev = b;
////    bcache.head.next = b;
////  }
//
//    // Lock
//    for(b = bcache.head; b < bcache.head + NBUCKS; ++b){
//        b->next = b;
//        b->prev = b;
//    }
//
//    for(b = bcache.free; b < bcache.free + NBUCKS; ++b){
//        b->next = b;
//        b->prev = b;
//    }
//
//    for(int i = 0;i < NBUF;++i)
//        insert(bcache.buf + i, bcache.free);
//}
//
//// Look through buffer cache for block on device dev.
//// If not found, allocate a buffer.
//// In either case, return locked buffer.
//static struct buf*
//bget(uint dev, uint blockno)
//{
//  struct buf *b;
//  int ind = hash(blockno);
//
//  acquire(&bcache.headlock[ind]);
//
//  // Is the block already cached?
//  for(b = bcache.head[ind].next; b != &bcache.head[ind]; b = b->next){
//    if(b->dev == dev && b->blockno == blockno){
//      b->refcnt++;
//      release(&bcache.headlock[ind]);
//      acquiresleep(&b->lock);
//      return b;
//    }
//  }
//
//  // Not cached.
//  // Recycle the least recently used (LRU) unused buffer.
//  for(int turn = 0; turn < NBUCKS;++turn){
//    int tmp = (ind + turn) % NBUCKS;
//    acquire(&bcache.freelock[tmp]);
//    if((b = bcache.free[tmp].prev) != &bcache.free[tmp]){
//        if(b->refcnt)
//            panic("bget: non-zero reference count in free list");
//        b->dev = dev;
//        b->blockno = blockno;
//        b->valid = 0;
//        b->refcnt = 1;
//        remove(b);
//        release(&bcache.freelock[tmp]);
//        insert(b, &bcache.head[ind]);
//        release(&bcache.headlock[ind]);
//        acquiresleep(&b->lock);
//        return b;
//    }
//    release(&bcache.freelock[tmp]);
//  }
//
//  panic("bget: no buffers");
//}
//
//// Return a locked buf with the contents of the indicated block.
//struct buf*
//bread(uint dev, uint blockno)
//{
//  struct buf *b;
//
//  b = bget(dev, blockno);
//  if(!b->valid) {
//    virtio_disk_rw(b, 0);
//    b->valid = 1;
//  }
//  return b;
//}
//
//// Write b's contents to disk.  Must be locked.
//void
//bwrite(struct buf *b)
//{
//  if(!holdingsleep(&b->lock))
//    panic("bwrite");
//  virtio_disk_rw(b, 1);
//}
//
//// Release a locked buffer.
//// Move to the head of the most-recently-used list.
//void
//brelse(struct buf *b)
//{
//	int ind = hash(b->blockno);
//  if(!holdingsleep(&b->lock))
//    panic("brelse");
//  
//  releasesleep(&b->lock);
//
//  acquire(&bcache.headlock[ind]);
//  b->refcnt--;
//  if (b->refcnt == 0) {
//    // no one is waiting for it.
//    remove(b);
//    acquire(&bcache.freelock[ind]);
//    insert(b, &bcache.free[ind]);
//    release(&bcache.freelock[ind]);
//  }
//
//  release(&bcache.headlock[ind]);
//}
//
//void
//bpin(struct buf *b) {
//	// Lock
//	int ind = hash(b->blockno);
//  acquire(&bcache.headlock[ind]);
//  b->refcnt++;
//  release(&bcache.headlock[ind]);
//}
//
//void
//bunpin(struct buf *b) {
//	// Lock
//	int ind = hash(b->blockno);
//  acquire(&bcache.headlock[ind]);
//  b->refcnt--;
//  release(&bcache.headlock[ind]);
//}
//

