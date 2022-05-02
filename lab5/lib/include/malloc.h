#include "stdint.h"

#define PAGE_MAX_ORDER 16
#define PAGE_FREE -1 
#define PAGE_ALLOCATED -2 
#define PAGE_RESERVED -3

extern unsigned long _kernel_start;
extern unsigned long _kernel_end;
extern unsigned char _heap_start;

typedef unsigned long size_t;

typedef struct frame_info {
  unsigned int status;
  int index;
} frame_info;

typedef struct page_item {
  int index;
  struct page_item *previous;
  struct page_item *next;
} page_item;

typedef struct pool_header {
  unsigned int size;
  unsigned int total;
  unsigned int used;
  struct pool_header *next;
} pool_header;

void page_init();
int page_allocate(size_t size);
void page_free(int index);
void show_frame();
void show_page_list();

void *malloc(uint64_t size);
void free(void *addr);
