#ifndef MM_MALLOC_H
#define MM_MALLOC_H

#include <stddef.h>

// ----------------------------------------------------------------------------
// A couple of macros to make packing structures less compiler dependent

#define MM_H_ATTPACKPRE
#define MM_H_ATTPACKSUF __attribute__((__packed__))

typedef struct MM_HEAP_INFO_t {
  unsigned short int totalEntries;
  unsigned short int usedEntries;
  unsigned short int freeEntries; 

  unsigned short int totalBlocks; 
  unsigned short int usedBlocks; 
  unsigned short int freeBlocks; 
  }
  MM_HEAP_INFO;
  
extern MM_HEAP_INFO heapInfo;
  
extern void mm_info( void );

// ----------------------------------------------------------------------------

void *mm_realloc( void *ptr, size_t size );
void *mm_malloc( size_t size );
void mm_free( void *ptr );

#endif // MM_MALLOC_H
