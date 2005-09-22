#include <windows.h>

void *win32_heap_alloc(size_t sz)
{
        return HeapAlloc( GetProcessHeap(), 0, sz);
}

void *win32_heap_realloc(void *mem, size_t sz)
{
        return HeapReAlloc( GetProcessHeap(), 0, mem, sz);
}

void win32_heap_free(void *mem)
{
        HeapFree( GetProcessHeap(), 0, mem);
}

