// ----------------------------------------------------------------------------
// mm_malloc.c - a memory allocator for embedded systems
// ----------------------------------------------------------------------------
// R.Hempel 2007-09-22 - Original
// ----------------------------------------------------------------------------
//
// This is a memory management library specifically designed to work with the
// ARM7 embedded processor, but it should work on many other 32 bit processors,
// as well as 16 and 8 bit devices.
//
// The memory manager assumes the following things:
//
// 1. The standard POSIX compliant malloc/realloc/free semantics are used
// 2. All memory used by the manager is allocated at link time, it is aligned
//    on a 32 bit boundary, it is contiguous, and its extent (start and end
//    address) is filled in by the linker.
// 3. All memory used by the manager is initialized to 0 as part of the
//    runtime startup routine. No other initialization is required.
//
// The fastest linked list implementations use double linked lists so that
// its possible to insert and delete blocks. This memory manager keeps track
// of both free and used blocks in a doubly linked list.
//
// Most memory managers use some kind of list structure made up of pointers
// to keep track of used - and sometimes free - blocks of memory. In an
// embedded system, this can get pretty expensive as each pointer can use
// up to 32 bits.
//
// In most embedded systems there is no need for managing large blocks
// of memory dynamically, so a full 32 bit pointer based data structure
// for the free and used block lists is wasteful. A block of memory on
// the free list would use 16 bytes just for the pointers!
//
// This memory management library sees the malloc heap as an array of blocks,
// and uses block numbers to keep track of locations. The block numbers are
// 15 bits - which allows for up to 32767 blocks of memory. The high order
// bit marks a block as being either free or in use, which will be explained
// later.
//
// The result is that a block of memory on the free list uses just 8 bytes
// instead of 16.
//
// In fact, we go even one step futher when we realize that the free block
// index values are available to store data when the block is allocated.
//
// The overhead of an allocated block is therefore just 4 bytes.
//
// Each memory block holds 8 bytes, and there are up to 32767 blocks
// available, for about 256K of heap space. If that's not enough, you
// can always add more data bytes to the body of the memory block
// at the expense of free block size overhead.
//
// There are a lot of little features and optimizations in this memory
// management system that makes it especially suited to small embedded, but
// the best way to appreciate them is to review the data structures and
// algorithms used, so let's get started.
//
// ----------------------------------------------------------------------------
//
// We have a general notation for a block that we'll use to describe the
// different scenarios that our memory allocation algorithm must deal with:
//
//    +----+----+----+----+
// c  |* n |  p | nf | pf |
//    +----+----+----+----+
//
// Where - c  is the index of this block
//         *  is the indicator for a free block
//         n  is the index of the next block in the heap
//         p  is the index of the previous block in the heap
//         nf is the index of the next block in the free list
//         pf is the index of the previous block in the free list
//
// The fact that we have forward and backward links in the block descriptors
// means that malloc() and free() operations can be very fast. It's easy
// to either allocate the whole free item to a new block or to allocate part
// of the free item and leave the rest on the free list without traversing
// the list from front to back first.
//
// The entire block of memory used by the heap is assumed to be initialized
// to 0. The very first block in the heap is special - it't the head of the
// free block list. It is never assimilated with a free block (more on this
// later).
//
// Once a block has been allocated to the application, it looks like this:
//
//    +----+----+----+----+
//  c |  n |  p |   ...   |
//    +----+----+----+----+
//
// Where - c  is the index of this block
//         n  is the index of the next block in the heap
//         p  is the index of the previous block in the heap
//
// Note that the free list information is gone, because it's now being used to
// store actual data for the application. It would have been nice to store
// the next and previous free list indexes as well, but that would be a waste
// of space. If we had even 500 items in use, that would be 2,000 bytes for
// free list information. We simply can't afford to waste that much.
//
// The address of the ... area is what is returned to the application
// for data storage.
//
// The following sections describe the scenarios encountered during the
// operation of the library. There are two additional notation conventions:
//
// ?? inside a pointer block means that the data is irrelevant. We don't care
//    about it because we don't read or modify it in the scenario being
//    described.
//
// ... between memory blocks indicates zero or more additional blocks are
//     allocated for use by the upper block.
//
// And while we're talking about "upper" and "lower" blocks, we should make
// a comment about adresses. In the diagrams, a block higher up in the
// picture is at a lower address. And the blocks grow downwards their
// block index increases as does their physical address.
//
// Finally, there's one very important characteristic of the individual
// blocks that make up the heap - there can never be two consecutive free
// memory blocks, but there can be consecutive used memory blocks.
//
// The reason is that we always want to have a short free list of the
// largest possible block sizes. By always assimilating a newly freed block
// with adjacent free blocks, we maximize the size of each free memory area.
//
//---------------------------------------------------------------------------
//
// Operation of malloc right after system startup
//
// As part of the system startup code, all of the heap has been cleared.
//
// During the very first malloc operation, we start traversing the free list
// starting at index 0. The index of the next free block is 0, which means
// we're at the end of the list!
//
// At this point, the malloc has a special test that checks if the current
// block index is 0, which it is. This special case initializes the free
// list to point at block index 1.
//
//    BEFORE                             AFTER
//
//    +----+----+----+----+              +----+----+----+----+
// 0  |  0 |  0 |  0 |  0 |           0  |  1 |  0 |  1 |  0 |
//    +----+----+----+----+              +----+----+----+----+
//                                       +----+----+----+----+
//                                    1  |  0 |  0 |  0 |  0 |
//                                       +----+----+----+----+
//
// The heap is now ready to complete the first malloc operation.
//
// ----------------------------------------------------------------------------
//
// Operation of malloc when we have reached the end of the free list and
// there is no block large enough to accommodate the request.
//
// This happens at the very first malloc operation, or any time the free
// list is traversed and no free block large enough for the request is
// found.
//
// The current block pointer will be at the end of the free list, and we
// know we're at the end of the list because the nf index is 0, like this:
//
//    BEFORE                             AFTER
//
//    +----+----+----+----+              +----+----+----+----+
// pf |*?? | ?? | cf | ?? |           pf |*?? | ?? | lf | ?? |
//    +----+----+----+----+              +----+----+----+----+
//             ...                                ...
//    +----+----+----+----+              +----+----+----+----+
//  p | cf | ?? |   ...   |            p | cf | ?? |   ...   |
//    +----+----+----+----+              +----+----+----+----+
//    +----+----+----+----+              +----+----+----+----+  Clear the free
// cf |* 0 |  p |  0 | pf |            c | lf |  p |   ...   |  list bit here
//    +----+----+----+----+              +----+----+----+----+
//                                       +----+----+----+----+
//                                    lf |* 0 | cf |  0 | pf |
//                                       +----+----+----+----+
//
// As we walk the free list looking for a block of size b or larger, we get
// to cf, which is the last item in the free list. We know this because the
// next index is 0.
//
// So we're going to turn cf into the new block of memory, and then create
// a new block that represents the last free entry (lf) and adjust the prev
// index of lf to point at the  block we just created. We also need to adjust
// the next index of the new block (c) to point to the last free block.
//
// Note that the next free index of the pf block must point to the new lf
// because cf is no longer a free block!
//
// ----------------------------------------------------------------------------
//
// Operation of malloc when we have found a block (cf) that will fit the
// current request of b units exactly.
//
// This one is pretty easy, just clear the free list bit in the current
// block and unhook it from the free list.
//
//    BEFORE                             AFTER
//
//    +----+----+----+----+              +----+----+----+----+
// pf |*?? | ?? | cf | ?? |           pf |*?? | ?? | nf | ?? |
//    +----+----+----+----+              +----+----+----+----+
//             ...                                ...
//    +----+----+----+----+              +----+----+----+----+
//  p | cf | ?? |   ...   |            p | cf | ?? |   ...   |
//    +----+----+----+----+              +----+----+----+----+
//    +----+----+----+----+              +----+----+----+----+  Clear the free
// cf |* n |  p | nf | pf |           cf |  n |  p |   ..    |  list bit here
//    +----+----+----+----+              +----+----+----+----+
//    +----+----+----+----+              +----+----+----+----+
//  n | ?? | cf |   ...   |            n | ?? | cf |   ...   |
//    +----+----+----+----+              +----+----+----+----+
//             ...                                ...
//    +----+----+----+----+              +----+----+----+----+
// nf |*?? | ?? | ?? | cf |           nf | ?? | ?? | ?? | pf |
//    +----+----+----+----+              +----+----+----+----+
//
// Unhooking from the free list is accomplished by adjusting the next and
// prev free list index values in the pf and nf blocks.
//
// ----------------------------------------------------------------------------
//
// Operation of malloc when we have found a block that will fit the current
// request of b units with some left over.
//
// We'll allocate the new block at the END of the current free block so we
// don't have to change ANY free list pointers.
//
//    BEFORE                             AFTER
//
//    +----+----+----+----+              +----+----+----+----+
// pf |*?? | ?? | cf | ?? |           pf |*?? | ?? | cf | ?? |
//    +----+----+----+----+              +----+----+----+----+
//             ...                                ...
//    +----+----+----+----+              +----+----+----+----+
//  p | cf | ?? |   ...   |            p | cf | ?? |   ...   |
//    +----+----+----+----+              +----+----+----+----+
//    +----+----+----+----+              +----+----+----+----+
// cf |* n |  p | nf | pf |           cf |* c |  p | nf | pf |
//    +----+----+----+----+              +----+----+----+----+
//                                       +----+----+----+----+ This is the new
//                                     c |  n | cf |   ..    | block at cf+b
//                                       +----+----+----+----+
//    +----+----+----+----+              +----+----+----+----+
//  n | ?? | cf |   ...   |            n | ?? |  c |   ...   |
//    +----+----+----+----+              +----+----+----+----+
//             ...                                ...
//    +----+----+----+----+              +----+----+----+----+
// nf |*?? | ?? | ?? | cf |           nf | ?? | ?? | ?? | pf |
//    +----+----+----+----+              +----+----+----+----+
//
// This one is prety easy too, except we don't need to mess with the
// free list indexes at all becasue we'll allocate the new block at the
// end of the current free block. We do, however have to adjust the
// indexes in cf, c, and n.
//
// ----------------------------------------------------------------------------
//
// That covers the initialization and all possible malloc scenarios, so now
// we need to cover the free operation possibilities...
//
// The operation of free depends on the position of the current block being
// freed relative to free list items immediately above or below it. The code
// works like this:
//
// if next block is free
//   assimilate with next block already on free list
// if prev block is free
//   assimilate with prev block already on free list
// else
//   put current block at head of free list
//
// ----------------------------------------------------------------------------
//
// Step 1 of the free operation checks if the next block is free, and if it
// is then insert this block into the free list and assimilate the next block
// with this one.
//
// Note that c is the block we are freeing up, cf is the free block that
// follows it.
//
//    BEFORE                             AFTER
//
//    +----+----+----+----+              +----+----+----+----+
// pf |*?? | ?? | cf | ?? |           pf |*?? | ?? | nf | ?? |
//    +----+----+----+----+              +----+----+----+----+
//             ...                                ...
//    +----+----+----+----+              +----+----+----+----+
//  p |  c | ?? |   ...   |            p |  c | ?? |   ...   |
//    +----+----+----+----+              +----+----+----+----+
//    +----+----+----+----+              +----+----+----+----+ This block is
//  c | cf |  p |   ...   |            c | nn |  p |   ...   | disconnected
//    +----+----+----+----+              +----+----+----+----+ from free list,
//    +----+----+----+----+                                    assimilated with
// cf |*nn |  c | nf | pf |                                    the next, and
//    +----+----+----+----+                                    ready for step 2
//    +----+----+----+----+              +----+----+----+----+
// nn | ?? | cf | ?? | ?? |           nn | ?? |  c |   ...   |
//    +----+----+----+----+              +----+----+----+----+
//             ...                                ...
//    +----+----+----+----+              +----+----+----+----+
// nf |*?? | ?? | ?? | cf |           nf |*?? | ?? | ?? | pf |
//    +----+----+----+----+              +----+----+----+----+
//
// Take special note that the newly assimilated block (c) is completely
// disconnected from the free list, and it does not have its free list
// bit set. This is important as we move on to step 2 of the procedure...
//
// ----------------------------------------------------------------------------
//
// Step 2 of the free operation checks if the prev block is free, and if it
// is then assimilate it with this block.
//
// Note that c is the block we are freeing up, pf is the free block that
// precedes it.
//
//    BEFORE                             AFTER
//
//    +----+----+----+----+              +----+----+----+----+ This block has
// pf |* c | ?? | nf | ?? |           pf |* n | ?? | nf | ?? | assimilated the
//    +----+----+----+----+              +----+----+----+----+ current block
//    +----+----+----+----+
//  c |  n | pf |   ...   |
//    +----+----+----+----+
//    +----+----+----+----+              +----+----+----+----+
//  n | ?? |  c |   ...   |            n | ?? | pf | ?? | ?? |
//    +----+----+----+----+              +----+----+----+----+
//             ...                                ...
//    +----+----+----+----+              +----+----+----+----+
// nf |*?? | ?? | ?? | pf |           nf |*?? | ?? | ?? | pf |
//    +----+----+----+----+              +----+----+----+----+
//
// Nothing magic here, except that when we're done, the current block (c)
// is gone since it's been absorbed into the previous free block. Note that
// the previous step guarantees that the next block (n) is not free.
//
// ----------------------------------------------------------------------------
//
// Step 3 of the free operation only runs if the previous block is not free.
// it just inserts the current block to the head of the free list.
//
// Remember, 0 is always the first block in the memory heap, and it's always
// head of the free list!
//
//    BEFORE                             AFTER
//
//    +----+----+----+----+              +----+----+----+----+
//  0 | ?? | ?? | nf |  0 |            0 | ?? | ?? |  c |  0 |
//    +----+----+----+----+              +----+----+----+----+
//             ...                                ...
//    +----+----+----+----+              +----+----+----+----+
//  p |  c | ?? |   ...   |            p |  c | ?? |   ...   |
//    +----+----+----+----+              +----+----+----+----+
//    +----+----+----+----+              +----+----+----+----+
//  c |  n |  p |   ..    |            c |* n |  p | nf |  0 |
//    +----+----+----+----+              +----+----+----+----+
//    +----+----+----+----+              +----+----+----+----+
//  n | ?? |  c |   ...   |            n | ?? |  c |   ...   |
//    +----+----+----+----+              +----+----+----+----+
//             ...                                ...
//    +----+----+----+----+              +----+----+----+----+
// nf |*?? | ?? | ?? |  0 |           nf |*?? | ?? | ?? |  c |
//    +----+----+----+----+              +----+----+----+----+
//
// Again, nothing spectacular here, we're simply adjusting a few pointers
// to make the most recently freed block the first item in the free list.
//
// That's because finding the previous free block would mean a reverse
// traversal of blocks until we found a free one, and it's just easier to
// put it at the head of the list. No traversal is needed.
//
// ----------------------------------------------------------------------------
//
// Finally, we can cover realloc, which has the following basic operation.
//
// The first thing we do is assimilate up with the next free block of
// memory if possible. This step might help if we're resizing to a bigger
// block of memory. It also helps if we're downsizing and creating a new
// free block with the leftover memory.
//
// After a possible assimilation with the next free block, we have three
// scenarios:
//
// 1. The current block size is exactly the right size, so no more work is
//    needed.
//
// 2. The current block is bigger than the new required size, so carve off
//    the excess and add it to the free list.
//
// 3. The current block is still smaller than the required size, so malloc
//    a new block of the correct size and copy the current data into the new
//    block before freeing the current block.
//
// The only one of these scenarios that involves an operation that has not
// yet been described is the second one, and it's shown below:
//
//    BEFORE                             AFTER
//
//    +----+----+----+----+              +----+----+----+----+
//  p |  c | ?? |   ...   |            p |  c | ?? |   ...   |
//    +----+----+----+----+              +----+----+----+----+
//    +----+----+----+----+              +----+----+----+----+
//  c |  n |  p |   ...   |            c |  s |  p |   ...   |
//    +----+----+----+----+              +----+----+----+----+
//                                       +----+----+----+----+ This is the
//                                     s |  n |  c |   ..    | new block at
//                                       +----+----+----+----+ c+blocks
//    +----+----+----+----+              +----+----+----+----+
//  n | ?? |  c |   ...   |            n | ?? |  s |   ...   |
//    +----+----+----+----+              +----+----+----+----+
//
// Then we call free() with the adress of the data portion of the new
// block (s) which adds it to the free list.
//
// ----------------------------------------------------------------------------


#include "m_malloc.h"

// ----------------------------------------------------------------------------
//
// There are a number of defines you can set at compile time that affect how
// the memory allocator will operate. In GNU C, you set these compile time
// defines like this:
//
// -D MM_TEST_MAIN
//
// Set this if you want to compile in the test suite at the end of this file.
// If you do set this variable, then the function names are left alone as
// mm_malloc() mm_free() and mm_realloc() so that they cannot be confused
// with the C runtime functions malloc() free() and realloc()
//
// If you leave this variable unset, then the function names become malloc()
// free() and realloc() so that they can be used as the C runtime functions
// in an embedded environment.
//
// -D MM_BEST_FIT (defualt)
//
// Set this if you want to use a best-fit algorithm for allocating new
// blocks
//
// -D MM_FIRST_FIT
//
// Set this if you want to use a first-fit algorithm for allocating new
// blocks
//
// -D MM_DBG_LOG_LEVEL=n
//
// Set n to a value from 0 to 6 depending on how verbose you want the debug
// log to be
//
// ----------------------------------------------------------------------------

#ifndef MM_FIRST_FIT
#  ifndef MM_BEST_FIT
#    define MM_BEST_FIT
#  endif
#endif

#ifndef MM_DBG_LOG_LEVEL
#  define DBG_LOG_LEVEL 0
#else
#  define DBG_LOG_LEVEL MM_DBG_LOG_LEVEL
#endif

#include "dbglog.h"
#include <string.h>

// ----------------------------------------------------------------------------

MM_H_ATTPACKPRE typedef struct mm_ptr_t {
  unsigned short int next;
  unsigned short int prev;
} MM_H_ATTPACKSUF mm_ptr;


MM_H_ATTPACKPRE typedef struct mm_block_t {
  union {
    mm_ptr used;
  } header;
  union {
    mm_ptr free;
    unsigned char data[4];
  } body;
} MM_H_ATTPACKSUF mm_block;

#define MM_FREELIST_MASK (0x8000)
#define MM_BLOCKNO_MASK  (0x7FFF)

// ----------------------------------------------------------------------------

#ifndef MM_TEST_MAIN

//  #define mm_free    free
//  #define mm_malloc  malloc
//  #define mm_realloc realloc

  extern mm_block mm_heap[];

  // Note that _MM_NUMBLOCKS is a value that is computed at link time, and
  // it represents the number of blocks available for the memory manager.

  extern unsigned short int _MM_NUMBLOCKS;

  // Link time calculations assign values to symbols, but you can't take
  // the value of something filled in at link time, you can only get its
  // address.
  //
  // That's why we take the address of _MM_NUMBLOCKS and assign it to
  // the constant value mm_numblocks.

  const unsigned int mm_numblocks = (unsigned int)(&_MM_NUMBLOCKS);

  #define MM_NUMBLOCKS (mm_numblocks)

#else

  mm_block mm_heap[2600];

  const unsigned short int mm_numblocks = sizeof(mm_heap)/sizeof(mm_block);

  #define MM_NUMBLOCKS (mm_numblocks)

#endif

// ----------------------------------------------------------------------------

#define MM_BLOCK(b)  (mm_heap[b])

#define MM_NBLOCK(b) (MM_BLOCK(b).header.used.next)
#define MM_PBLOCK(b) (MM_BLOCK(b).header.used.prev)
#define MM_NFREE(b)  (MM_BLOCK(b).body.free.next)
#define MM_PFREE(b)  (MM_BLOCK(b).body.free.prev)
#define MM_DATA(b)   (MM_BLOCK(b).body.data)

// ----------------------------------------------------------------------------
// One of the coolest things about this little library is that it's VERY
// easy to get debug information about the memory heap by simply iterating
// through all of the memory blocks.
//
// As you go through all the blocks, you can check to see if it's a free
// block by looking at the high order bit of the next block index. You can
// also see how big the block is by subtracting the next block index from
// the current block number.
//
// The mm_info function does all of that and makes the results available
// in the heapInfo structure.
// ----------------------------------------------------------------------------

MM_HEAP_INFO heapInfo;

void mm_info( void ) {

  unsigned short int blockNo = 0;

  // Clear out all of the entries in the heapInfo structure before doing
  // any calculations..
  //
  memset( &heapInfo, 0, sizeof( heapInfo ) );

  DBG_LOG_INFO( "\n\nDumping the mm_heap...\n" );

  DBG_LOG_INFO( "%08x -> Block S %6i NB %6i PB %6i Z %6i NF %6i PF %6i\n",
	       &MM_BLOCK(blockNo),
                blockNo,
                MM_NBLOCK(blockNo) & MM_BLOCKNO_MASK,
                MM_PBLOCK(blockNo),
                (MM_NBLOCK(blockNo) & MM_BLOCKNO_MASK )-blockNo,
                MM_NFREE(blockNo),
                MM_PFREE(blockNo) );

  // Now loop through the block lists, and keep track of the number and size
  // of used and free blocks. The terminating condition is an nb pointer with
  // a value of zero...

  while( MM_NBLOCK(blockNo) & MM_BLOCKNO_MASK ) {
    ++heapInfo.totalEntries;
    heapInfo.totalBlocks += (MM_NBLOCK(blockNo) & MM_BLOCKNO_MASK )-blockNo;

    // Is this a free block?

    if( MM_NBLOCK(blockNo) & MM_FREELIST_MASK ) {
      ++heapInfo.freeEntries;
      heapInfo.freeBlocks += (MM_NBLOCK(blockNo) & MM_BLOCKNO_MASK )-blockNo;

      DBG_LOG_INFO( "%08x -> Block S %6i NB %6i PB %6i Z %6i NF %6i PF %6i\n",
	           &MM_BLOCK(blockNo),
                    blockNo,
                    MM_NBLOCK(blockNo) & MM_BLOCKNO_MASK,
                    MM_PBLOCK(blockNo),
                    (MM_NBLOCK(blockNo) & MM_BLOCKNO_MASK )-blockNo,
                    MM_NFREE(blockNo),
                    MM_PFREE(blockNo) );
    } else {
      ++heapInfo.usedEntries;
      heapInfo.usedBlocks += (MM_NBLOCK(blockNo) & MM_BLOCKNO_MASK )-blockNo;

      DBG_LOG_INFO( "%08x -> Block S %6i NB %6i PB %6i Z %6i\n",
	           &MM_BLOCK(blockNo),
                    blockNo,
                    MM_NBLOCK(blockNo) & MM_BLOCKNO_MASK,
                    MM_PBLOCK(blockNo),
                    (MM_NBLOCK(blockNo) & MM_BLOCKNO_MASK )-blockNo );
    }

    blockNo = MM_NBLOCK(blockNo) & MM_BLOCKNO_MASK;
  }

  // Update the accounting totals with information from the last block, the
  // rest must be free!

  heapInfo.freeBlocks  += MM_NUMBLOCKS-blockNo;
  heapInfo.totalBlocks += MM_NUMBLOCKS-blockNo;

  DBG_LOG_INFO( "%08x -> Block S %6i NB %6i PB %6i Z %6i NF %6i PF %6i\n",
	       &MM_BLOCK(blockNo),
                blockNo,
                MM_NBLOCK(blockNo) & MM_BLOCKNO_MASK,
                MM_PBLOCK(blockNo),
                MM_NUMBLOCKS-blockNo,
                MM_NFREE(blockNo),
                MM_PFREE(blockNo) );

  DBG_LOG_INFO( "Total Entries %05i    Used Entries %05i    Free Entries %05i\n",
	       heapInfo.totalEntries,
	       heapInfo.usedEntries,
	       heapInfo.freeEntries );

  DBG_LOG_INFO( "Total Blocks  %05i    Used Blocks  %05i    Free Blocks  %05i\n",
	       heapInfo.totalBlocks,
	       heapInfo.usedBlocks,
                heapInfo.freeBlocks  );
}

// ----------------------------------------------------------------------------

static unsigned short int mm_blocks( size_t size ) {

  // The calculation of the block size is not too difficult, but there are
  // a few little things that we need to be mindful of.
  //
  // When a block removed from the free list, the space used by the free
  // pointers is available for data. That's what the first calculation
  // of size is doing.

  if( size <= (sizeof(((mm_block *)0)->body)) )
    return( 1 );

  // If it's for more than that, then we need to figure out the number of
  // additional whole blocks the size of an mm_block are required.

  size -= ( 1 + (sizeof(((mm_block *)0)->body)) );

  return( 2 + size/(sizeof(mm_block)) );
}

// ----------------------------------------------------------------------------

static void mm_assimilate_up( unsigned short int c ) {

  if( MM_NBLOCK(MM_NBLOCK(c)) & MM_FREELIST_MASK ) {
    // The next block is a free block, so assimilate up and remove it from
    // the free list

    DBG_LOG_DEBUG( "Assimilate up to next block, which is FREE\n" );

    // Disconnect the next block from the FREE list
    MM_NFREE(MM_PFREE(MM_NBLOCK(c))) = MM_NFREE(MM_NBLOCK(c));
    MM_PFREE(MM_NFREE(MM_NBLOCK(c))) = MM_PFREE(MM_NBLOCK(c));

    // Assimilate the next block with this one
    MM_PBLOCK(MM_NBLOCK(MM_NBLOCK(c)) & MM_BLOCKNO_MASK) = c;
    MM_NBLOCK(c) = MM_NBLOCK(MM_NBLOCK(c)) & MM_BLOCKNO_MASK;
  }
}

// ----------------------------------------------------------------------------

void mm_free( void *ptr ) {

  unsigned short int c;

  // If we're being asked to free a NULL pointer, well that's just silly!

  if( (void *)0 == ptr ) {
    DBG_LOG_DEBUG( "free a null pointer -> do nothing\n" );
    return;
  }

  // FIXME: At some point it might be a good idea to add a check to make sure
  //        that the pointer we're being asked to free up is actually within
  //        the mm_heap!

  // Figure out which block we're in. Note the use of truncated division...

  c = (ptr-(void *)(&(mm_heap[0])))/sizeof(mm_block);

  DBG_LOG_DEBUG( "Freeing block %6i\n", c );

  // Now let's assimilate this block with the next one if possible.

  mm_assimilate_up( c );

  // Then assimilate with the previous block if possible

  if( MM_NBLOCK(MM_PBLOCK(c)) & MM_FREELIST_MASK ) {

    DBG_LOG_DEBUG( "Assimilate down to next block, which is FREE\n" );

    MM_NBLOCK(MM_PBLOCK(c)) = MM_NBLOCK(c) | MM_FREELIST_MASK;
    MM_PBLOCK(MM_NBLOCK(c)) = MM_PBLOCK(c);
  } else {
    // The previous block is not a free block, so add this one to the head
    // of the free list

    DBG_LOG_DEBUG( "Just add to head of free list\n" );

    MM_PFREE(MM_NFREE(0)) = c;
    MM_NFREE(c)           = MM_NFREE(0);
    MM_PFREE(c)           = 0;
    MM_NFREE(0)           = c;

    MM_NBLOCK(c)         |= MM_FREELIST_MASK;
  }
}

// ----------------------------------------------------------------------------

void *mm_malloc( size_t size ) {

  unsigned short int blocks;
  unsigned short int blockSize;

  unsigned short int bestSize;
  unsigned short int bestBlock;

  unsigned short int cf;

  // The very first thing we do is figure out if we're being asked to allocate
  // a size of 0 - and if we are we'll simply return a NULL pointer. If not
  // then reduce the size by 1 byte so that the subsequent calculations on
  // the number of blocks to allocate are easier...

  if( 0 == size ) {
    DBG_LOG_DEBUG( "malloc a block of 0 bytes -> do nothing\n" );
    return( (void *)NULL );
  }

  blocks = mm_blocks( size );

  // Now we can scan through the free list until we find a space that's big
  // enough to hold the number of blocks we need.
  //
  // This part may be customized to be a best-fit, worst-fit, or first-fit
  // algorithm

  cf = MM_NFREE(0);

  bestBlock = MM_NFREE(0);
  bestSize  = 0x7FFF;

  while( MM_NFREE(cf) ) {
    blockSize = (MM_NBLOCK(cf) & MM_BLOCKNO_MASK) - cf;

    DBG_LOG_TRACE( "Looking at block %6i size %6i\n", cf, blockSize );

#if defined MM_FIRST_FIT
    // This is the first block that fits!
    if( (blockSize >= blocks) )
        break;
#elif defined MM_BEST_FIT
    if( (blockSize >= blocks) && (blockSize < bestSize) ) {
      bestBlock = cf;
      bestSize  = blockSize;
    }
#endif

    cf = MM_NFREE(cf);
  }

  if( 0x7FFF != bestSize ) {
    cf = bestBlock;
    blockSize = bestSize;
  }

  if( MM_NBLOCK(cf) & MM_BLOCKNO_MASK ) {
    // This is an existing block in the memory heap, we just need to split off
    // what we need, unlink it from the free list and mark it as in use, and
    // link the rest of the block back into the freelist as if it was a new
    // block on the free list...

    if( blockSize == blocks ) {
      // It's an exact fit and we don't neet to split off a block.
      DBG_LOG_DEBUG( "Allocating %6i blocks starting at %6i - exact\n", blocks, cf );

      MM_NFREE(MM_PFREE(cf)) = MM_NFREE(cf);
      MM_PFREE(MM_NFREE(cf)) = MM_PFREE(cf);

      MM_NBLOCK(cf) &= MM_BLOCKNO_MASK;
    } else {
     // It's not an exact fit and we need to split off a block.
     DBG_LOG_DEBUG( "Allocating %6i blocks starting at %6i - existing\n", blocks, cf );

     MM_NBLOCK(cf+blockSize-blocks) = MM_NBLOCK(cf) & MM_BLOCKNO_MASK;
     MM_PBLOCK(cf+blockSize-blocks) = cf;

     MM_PBLOCK(MM_NBLOCK(cf) & MM_BLOCKNO_MASK) = (cf+blockSize-blocks);
     MM_NBLOCK(cf)                              = (cf+blockSize-blocks) | MM_FREELIST_MASK;

     cf = cf+blockSize-blocks;
     }
  } else {
    // We're at the end of the heap - allocate a new block, but check to see if
    // there's enough memory left for the requested block! Actually, we may need
    // one more than that if we're initializing the mm_heap for the first
    // time, which happens in the next conditional...

    if( MM_NUMBLOCKS <= cf+blocks+1 ) {
      DBG_LOG_DEBUG(  "Can't allocate %5i blocks at %5i\n", blocks, cf );

      return( (void *)NULL );
    }

    // Now check to see if we need to initialize the free list...this assumes
    // that the BSS is set to 0 on startup. We should rarely get to the end of
    // the free list so this is the "cheapest" place to put the initialization!

    if( 0 == cf ) {
      DBG_LOG_DEBUG( "Initializing malloc free block pointer\n" );
      MM_NBLOCK(0) = 1;
      MM_NFREE(0)  = 1;
      cf = 1;
    }

    DBG_LOG_DEBUG( "Allocating %6i blocks starting at %6i - new     \n", blocks, cf );

    MM_NFREE(MM_PFREE(cf)) = cf+blocks;
    memcpy( &MM_BLOCK(cf+blocks), &MM_BLOCK(cf), sizeof(mm_block) );
    MM_NBLOCK(cf)        = cf+blocks;
    MM_PBLOCK(cf+blocks) = cf;
  }

  return( MM_DATA(cf) );
}

// ----------------------------------------------------------------------------

void *mm_realloc( void *ptr, size_t size ) {

  unsigned short int blocks;
  unsigned short int blockSize;

  unsigned short int c;

  // This code looks after the case of a NULL value for ptr. The ANSI C
  // standard says that if ptr is NULL and size is non-zero, then we've
  // got to work the same a malloc(). If size is also 0, then our version
  // of malloc() returns a NULL pointer, which is OK as far as the ANSI C
  // standard is concerned.

  if( ((void *)NULL == ptr) ) {
    DBG_LOG_DEBUG( "realloc the NULL pointer - call malloc()\n" );

    return( mm_malloc(size) );
  }

  // Now we're sure that we have a non_NULL ptr, but we're not sure what
  // we should do with it. If the size is 0, then the ANSI C standard says that
  // we should operate the same as free.

  if( 0 == size ) {
    DBG_LOG_DEBUG( "realloc to 0 size, just free the block\n" );

    mm_free( ptr );
    return( (void *)NULL );
  }

  // Otherwise we need to actually do a reallocation. A naiive approach
  // would be to malloc() a new block of the correct size, copy the old data
  // to the new block, and then free the old block.
  //
  // While this will work, we end up doing a lot of possibly unnecessary
  // copying. So first, let's figure out how many blocks we'll need.

  blocks = mm_blocks( size );

  // Figure out which block we're in. Note the use of truncated division...

  c = (ptr-(void *)(&(mm_heap[0])))/sizeof(mm_block);

  // And figure out how big this block is...

  blockSize = (MM_NBLOCK(c) - c);

  // Ok, now that we're here, we know the block number of the original chunk
  // of memory, and we know how much new memory we want, and we know the original
  // block size...

  if( blockSize == blocks ) {
    // This space intentionally left blank - return the original pointer!

    DBG_LOG_DEBUG( "realloc the same size block - %i, do nothing\n", blocks );

    return( ptr );
  }

  // Now we have a block size that could be bigger or smaller. Either
  // way, try to assimilate up to the next block before doing anything...
  //
  // If it's still too small, we have to free it anyways and it will save the
  // assimilation step later in free :-)

  mm_assimilate_up( c );

  // Now calculate the block size again...and we'll have three cases

  blockSize = (MM_NBLOCK(c) - c);

  if( blockSize == blocks ) {
    // This space intentionally left blank - return the original pointer!

    DBG_LOG_DEBUG( "realloc the same size block - %i, do nothing\n", blocks );

  } else if (blockSize > blocks ) {
    // New block is smaller than the old block, so just make a new block
    // at the end of this one and put it up on the free list...

    DBG_LOG_DEBUG( "realloc %i to a smaller block %i, shrink and free the leftover bits\n",
	          blockSize,
		 blocks );

    MM_NBLOCK(c+blocks) = MM_NBLOCK(c);
    MM_PBLOCK(c+blocks) = c;

    MM_PBLOCK(MM_NBLOCK(c)) = c+blocks;
    MM_NBLOCK(c)            = c+blocks;

    mm_free( (void *)&MM_NFREE(c+blocks) );

  } else {
    // New block is bigger than the old block, so just mm_malloc() a new
    // one, copy the old data to the new block, and free up the old block,
    // but only if the malloc was sucessful!

    void *oldptr = ptr;

    DBG_LOG_DEBUG( "realloc %i to a bigger block %i, make new, copy, and free the old\n",
	          blockSize,
		 blocks );

    if( (ptr = mm_malloc( size )) ) {
       memcpy( ptr, oldptr, size );
    }

    mm_free( oldptr );
  }

  return( ptr );
}

