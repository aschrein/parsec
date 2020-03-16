#ifndef THREAD_LOCAL_H
#define THREAD_LOCAL_H

#include <mytypes.h>

#ifdef __cplusplus
extern "C" {
#endif

void *tl_alloc(u64 size);
void *tl_realloc(void *ptr, u64 oldsize, u64 newsize);
void tl_free(void *ptr);
void *tl_alloc_tmp(u64 size);
void tl_alloc_tmp_enter();
void tl_alloc_tmp_exit();

#ifdef __cplusplus
}
#endif
#endif