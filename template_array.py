#if 0
import sys
from string import Template
import re
file = open(sys.argv[0]).read()
file = re.findall(r"//SOF(.*?)//EOF", file, flags=re.DOTALL)[-1]
t = Template(file)
GROW = '0x100'
for TYPE in [
"u64",
"Token",
"u32",
"float",
"Pair_string_ref_string_ref",
"Pair_u32_u32",
"Pair_u64_u64",
]:
  out = open(sys.argv[0].split("/")[-1].replace("template", "__" + TYPE).replace(".py", ".h"), "w")
  out.write(t.substitute(TYPE=TYPE, GROW=GROW))
  out.close()
sys.exit(0)
"""
#endif
//SOF
#ifndef ARRAY_${TYPE}_H
#define ARRAY_${TYPE}_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <thread_local.h>
#include <myassert.h>

typedef struct {
  ${TYPE} *ptr;
  uint64_t size;
  uint64_t capacity;
} Array_${TYPE};

Array_${TYPE} Array_${TYPE}_new(uint64_t capacity) {
  Array_${TYPE} out;
  if (capacity != 0)
    out.ptr = (${TYPE}*)tl_alloc(sizeof(${TYPE}) * capacity);
  else
    out.ptr = NULL;
  out.size = 0;
  out.capacity = capacity;
  return out;
}

void Array_${TYPE}_delete(Array_${TYPE} *array) {
  if (array->ptr != NULL) {
    tl_free(array->ptr);
  }
  array->ptr = NULL;
  array->size = 0;
  array->capacity = 0;
}

void Array_${TYPE}_resize(Array_${TYPE} *array, u64 new_size) {
  if (new_size > array->capacity) {
    uint64_t new_capacity = new_size;
    array->ptr = (${TYPE}*)tl_realloc(array->ptr,
                          sizeof(${TYPE}) * array->capacity,
                          sizeof(${TYPE}) * new_capacity);
    array->capacity = new_capacity;
  }
  ASSERT_DEBUG(array->capacity >= array->size + 1);
  ASSERT_DEBUG(array->ptr != NULL);
  array->size = new_size;
}

void Array_${TYPE}_memzero(Array_${TYPE} *array) {
  if (array->capacity > 0) {
    memset(array->ptr, 0, sizeof(${TYPE}) * array->capacity);
  }
}

Array_${TYPE} Array_${TYPE}_copy(Array_${TYPE} *array) {
  Array_${TYPE} out;
  out.ptr = NULL;
  out.size = array->size;
  out.capacity = array->capacity;
  if (array->size > 0) {
    Array_${TYPE}_resize(&out, array->capacity);
    memcpy(out.ptr, array->ptr, array->capacity * sizeof(${TYPE}));
  }
  return out;
}

void Array_${TYPE}_push(Array_${TYPE} *array, ${TYPE} elem) {
  if (array->size + 1 > array->capacity) {
    uint64_t new_capacity = array->capacity + ${GROW};
    array->ptr = (${TYPE}*)tl_realloc(array->ptr,
                          sizeof(${TYPE}) * array->capacity,
                          sizeof(${TYPE}) * new_capacity);
    array->capacity = new_capacity;
  }
  ASSERT_DEBUG(array->capacity >= array->size + 1);
  ASSERT_DEBUG(array->ptr != NULL);
  memcpy(array->ptr + array->size, &elem, sizeof(${TYPE}));
  array->size += 1;
}

${TYPE} Array_${TYPE}_pop(Array_${TYPE} *array) {
  ASSERT_DEBUG(array->size != 0);
  ASSERT_DEBUG(array->ptr != NULL);
  ${TYPE} elem = array->ptr[array->size - 1];
  if (array->size + ${GROW} < array->capacity) {
    uint64_t new_capacity = array->capacity - ${GROW};
    array->ptr = (${TYPE}*)tl_realloc(array->ptr,
                          sizeof(${TYPE}) * array->capacity,
                          sizeof(${TYPE}) * new_capacity);
    array->capacity = new_capacity;
  }
  ASSERT_DEBUG(array->size != 0);
  array->size -= 1;
  return elem;
}

#define ${TYPE}_FOREACH(array, i, code) \
  {\
    u32 i##_id = 0; ${TYPE} *i = &array.ptr[0];\
    for (; i##_id < array.size; ++i##_id, ++i) {\
      code \
    }\
  }

#ifdef __cplusplus
}
#endif

#endif
//EOF
#if 0
"""
#endif