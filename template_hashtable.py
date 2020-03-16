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
                  GROW=GROW, ATTEMPTS='16'))
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
u64 hash_of_u64(u64 key);
u8 cmp_eq_${KEY}(${KEY} a, ${KEY} b);

u8 HashArray_${KEY}_${VALUE}_push(HashArray_${KEY}_${VALUE} *harr, ${KEY} key, ${VALUE} value) {
  PERF_ENTER("HashArray_${KEY}_${VALUE}_push");
  ASSERT_DEBUG(harr != NULL);
  {
    u32 attempts = ${ATTEMPTS};
    u64 hash = hash_of_${KEY}(key);
    u64 size = harr->arr.capacity;
    if (size == 0) {
      Array_${PAIR_TYPE}_resize(&harr->arr, ${GROW});
      Array_${PAIR_TYPE}_memzero(&harr->arr);
      size = harr->arr.capacity;
    }
    ${PAIR_TYPE} pair;
    pair.key = key;
    pair.value = value;
    u32 attempt_id = 0;
    for (; attempt_id < attempts; ++attempt_id) {
      u64 id = hash % size;
      if (hash != 0) {
        pair.hash = hash;
        if (harr->arr.ptr[id].hash == 0) {
          memcpy(harr->arr.ptr + id, &pair, sizeof(${PAIR_TYPE}));
          harr->item_count += 1;
          PERF_HIST_ADD("HashArray_${KEY}_${VALUE}_push:hit", attempt_id);
          PERF_EXIT("HashArray_${KEY}_${VALUE}_push");
          return TRUE;
        }
      }
      hash = hash_of_u64(hash);
    }
  }
  PERF_HIST_ADD("HashArray_${KEY}_${VALUE}_push:miss", 0);
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
    PERF_EXIT("HashArray_${KEY}_${VALUE}_push");
    return TRUE;
  }
  ASSERT_DEBUG(false && "unreachable");
}

u8 HashArray_${KEY}_${VALUE}_get(HashArray_${KEY}_${VALUE} *harr, ${KEY} key, ${VALUE} *value) {
  PERF_ENTER("HashArray_${KEY}_${VALUE}_get");
  ASSERT_DEBUG(harr != NULL);
  u32 attempts = ${ATTEMPTS};
  u64 hash = hash_of_${KEY}(key);
  u64 size = harr->arr.capacity;
  if (size == 0)
    return FALSE;
  Array_${PAIR_TYPE} *arr = &harr->arr;
  u32 attempt_id = 0;
  for (; attempt_id < attempts; ++attempt_id) {
    u64 id = hash % size;
    if (hash != 0) {
      if (cmp_eq_${KEY}(arr->ptr[id].key, key) == TRUE) {
        if (value != NULL)
          *value = arr->ptr[id].value;
        PERF_HIST_ADD("HashArray_${KEY}_${VALUE}_get:hit", attempt_id);
        PERF_EXIT("HashArray_${KEY}_${VALUE}_get");
        return TRUE;
      }
    }
    hash = hash_of_u64(hash);
  }
  PERF_EXIT("HashArray_${KEY}_${VALUE}_get");
  return FALSE;
}

u8 HashArray_${KEY}_${VALUE}_has(HashArray_${KEY}_${VALUE} *harr, ${KEY} key) {
  PERF_ENTER("HashArray_${KEY}_${VALUE}_has");
  u8 res = HashArray_${KEY}_${VALUE}_get(harr, key, NULL);
  PERF_EXIT("HashArray_${KEY}_${VALUE}_has");
  return res;
}

#define ${KEY}_${VALUE}_FOREACH(harr, i, code) \
  {\
    u32 i##_id = 0; Pair_${KEY}_${VALUE} *i = &harr.arr.ptr[0];\
    for (; i##_id < harr.arr.size; ++i##_id, ++i) {\
      if (i->hash == 0) \
        continue; \
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