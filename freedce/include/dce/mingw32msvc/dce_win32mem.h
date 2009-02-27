#ifndef _WIN32_MEM_H
#define  _WIN32_MEM_H
typedef unsigned int size_t;

void *win32_heap_alloc(size_t sz);
void *win32_heap_realloc(void *mem, size_t sz);
void win32_heap_free(void *mem);
#endif

#define sys_malloc win32_heap_alloc
#define sys_realloc win32_heap_realloc
#define sys_free win32_heap_free

