#if 0
import sys
from string import Template
import re
file = open(sys.argv[0]).read()
file = re.findall(r"//SOF(.*?)//EOF", file, flags=re.DOTALL)[-1]
t = Template(file)
GROW = '0x100'
for KEY in ["u64", "u32", "string_ref"]:
  for VALUE in ["u64", "string_ref", "u32"]:
    out = open(sys.argv[0].split("/")[-1].replace("template", "__" + KEY + "_" + VALUE).replace(".py", ".h"), "w")
    out.write(t.substitute(KEY=KEY,
                  VALUE=VALUE,
                  PAIR_TYPE="Pair_" + KEY + "_" + VALUE,
                  GROW=GROW, ATTEMPTS='4'))
    out.close()
sys.exit(0)
"""
#endif
//SOF
#ifndef HashArray_${KEY}_${VALUE}_H
#define HashArray_${KEY}_${VALUE}_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <thread_local.h>
#include <mytypes.h>
#include <myassert.h>

typedef struct {
  ${KEY} key;
  ${VALUE} value;
  u64 hash;
} ${PAIR_TYPE};

#include <templates/__${PAIR_TYPE}_array.h>

typedef struct {
  Array_${PAIR_TYPE} arr;
  u32 item_count;
} HashArray_${KEY}_${VALUE};

HashArray_${KEY}_${VALUE} HashArray_${KEY}_${VALUE}_new(uint64_t capacity) {
  HashArray_${KEY}_${VALUE} out;
  out.item_count = 0;
  out.arr = Array_${PAIR_TYPE}_new(capacity);
  Array_${PAIR_TYPE}_memzero(&out.arr);
  return out;
}

void HashArray_${KEY}_${VALUE}_delete(HashArray_${KEY}_${VALUE} *harr) {
  Array_${PAIR_TYPE}_delete(&harr->arr);
  harr->item_count = 0;
}

u64 hash_of_${KEY}(${KEY} key);
u8 cmp_eq_${KEY}(${KEY} a, ${KEY} b);

u8 HashArray_${KEY}_${VALUE}_push(HashArray_${KEY}_${VALUE} *harr, ${KEY} key, ${VALUE} value) {
  {
    u32 attempts = ${ATTEMPTS};
    u64 hash = hash_of_${KEY}(key);
    u64 size = harr->arr.capacity;
    Array_${PAIR_TYPE} *arr = &harr->arr;
    if (size == 0) {
      Array_${PAIR_TYPE}_resize(arr, ${GROW});
      Array_${PAIR_TYPE}_memzero(arr);
      size = harr->arr.capacity;
    }
    ${PAIR_TYPE} pair;
    pair.key = key;
    pair.value = value;
    while (attempts --> 0) {
      u64 id = hash % size;
      if (hash != 0) {
        pair.hash = hash;
        if (arr->ptr[id].hash == 0) {
          memcpy(arr->ptr + id, &pair, sizeof(${PAIR_TYPE}));
          harr->item_count += 1;
          return TRUE;
        }
      }
      hash = hash_of_u64(hash);
    }
  }
  {
    Array_${PAIR_TYPE} old_arr = harr->arr;
    {
      Array_${PAIR_TYPE} new_arr = Array_${PAIR_TYPE}_new(0);
      Array_${PAIR_TYPE}_resize(&new_arr, old_arr.capacity + ${GROW});
      Array_${PAIR_TYPE}_memzero(&new_arr);
      harr->arr = new_arr;
      harr->item_count = 0;
    }
    u32 i = 0;
    for (; i < old_arr.capacity; ++i) {
      ${PAIR_TYPE} pair = old_arr.ptr[i];
      if (pair.hash != 0) {
        HashArray_${KEY}_${VALUE}_push(harr, pair.key, pair.value);
      }
    }
    Array_${PAIR_TYPE}_delete(&old_arr);
    u8 res = HashArray_${KEY}_${VALUE}_push(harr, key, value);
    ASSERT_DEBUG(res == TRUE);
    return TRUE;
  }
  ASSERT_DEBUG(false && "unreachable");
}

u8 HashArray_${KEY}_${VALUE}_has(HashArray_${KEY}_${VALUE} *harr, ${KEY} key) {
  u32 attempts = ${ATTEMPTS};
  u64 hash = hash_of_${KEY}(key);
  u64 size = harr->arr.capacity;
  if (size == 0)
    return FALSE;
  Array_${PAIR_TYPE} *arr = &harr->arr;
  while (attempts --> 0) {
    u64 id = hash % size;
    if (hash != 0) {
      if (cmp_eq_${KEY}(arr->ptr[id].key, key) == TRUE)
        return TRUE;
    }
    hash = hash_of_u64(hash);
  }
  return FALSE;
}

#ifdef __cplusplus
}
#endif

#endif
//EOF
#if 0
"""
#endif