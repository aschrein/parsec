#ifndef THREAD_LOCAL_H
#define THREAD_LOCAL_H

#include <mytypes.h>

#ifdef __cplusplus
extern "C" {
#endif

void *tl_alloc(u64 size);
void *tl_realloc(void *ptr, u64 oldsize, u64 newsize);
void tl_free(void *ptr);

#ifdef __cplusplus
}
#endif
#endif