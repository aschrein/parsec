#include <myassert.h>
#include <thread_local.h>
#include <utils.hpp>

enum class Token_t {
  PAREN_L,
  PAREN_R,
  STRING,
  BRACE_L,
  BRACE_R,
  LITERAL,
  I32,
  F32,
  BRACK_L,
  BRACK_R
};

struct string_ref {
  const char *ptr;
  u64 len;
};

struct Token {
  Token_t type;
  union {
    string_ref str;
    f32 fl;
    i32 in;
  };
};

#include <templates/__Token_array.h>
#include <templates/__string_ref_string_ref_hashtable.h>
#include <templates/__u64_array.h>
#include <templates/__u64_u64_hashtable.h>

struct Temporal_Storage {
  u8 *ptr;
  u64 capacity;
  u64 cursor;
  Array_u64 stack;
};

Temporal_Storage Temporal_Storage_new(u64 capacity) {
  Temporal_Storage out;
  out.ptr = (u8 *)malloc(capacity);
  out.capacity = capacity;
  out.cursor = 0;
  out.stack = Array_u64_new(0);
  return out;
}

void Temporal_Storage_delete(Temporal_Storage *ts) {
  ASSERT_DEBUG(ts != NULL);
  ASSERT_DEBUG(ts->ptr != NULL);
  ts->cursor = 0;
  ts->capacity = 0;
  free(ts->ptr);
  Array_u64_delete(&ts->stack);
}

void *Temporal_Storage_alloc(Temporal_Storage *ts, u64 size) {
  ASSERT_DEBUG(ts != NULL);
  void *ptr = (void *)(ts->ptr + ts->cursor);
  size = (size + 15) & (~0xf);
  ts->cursor += size;
  ASSERT_DEBUG(ts->cursor < ts->capacity);
  return ptr;
}

void Temporal_Storage_reset(Temporal_Storage *ts) {
  ASSERT_DEBUG(ts != NULL);
  ts->cursor = 0;
}

struct Thread_Local {
  Temporal_Storage temporal_storage;
  u8 initialized = 0;
  ~Thread_Local() { Temporal_Storage_delete(&temporal_storage); }
};

// TODO(aschrein): Change to __thread?
thread_local Thread_Local g_tl{};

Thread_Local *get_tl() {
  if (g_tl.initialized == 0) {
    g_tl.initialized = 1;
    g_tl.temporal_storage = Temporal_Storage_new(1 << 24);
  }
  return &g_tl;
}

void *tl_alloc_tmp(u64 size) {
  return Temporal_Storage_alloc(&get_tl()->temporal_storage, size);
}

void tl_alloc_tmp_enter() {
  Temporal_Storage *ts = &get_tl()->temporal_storage;
  Array_u64_push(&ts->stack, ts->cursor);
}
void tl_alloc_tmp_exit() {
  Temporal_Storage *ts = &get_tl()->temporal_storage;
  ASSERT_DEBUG(ts->stack.size > 0);
  u64 bookmark = Array_u64_pop(&ts->stack);
  ts->cursor = bookmark;
}

void *tl_alloc(u64 size) { return malloc(size); }

void *tl_realloc(void *ptr, u64 oldsize, u64 newsize) {
  if (oldsize == newsize)
    return ptr;
  u64 min_size = oldsize < newsize ? oldsize : newsize;
  void *new_ptr = NULL;
  if (newsize != 0)
    new_ptr = malloc(newsize);
  if (min_size != 0) {
    memcpy(new_ptr, ptr, min_size);
  }
  if (ptr != NULL)
    free(ptr);
  return new_ptr;
}

void tl_free(void *ptr) { free(ptr); }

Array_Token tokenize(string_ref str) {
  Array_Token out = Array_Token_new(0x100);

  // for (auto &c : str) {
  //   if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
  //     if (!ss.str().empty()) {

  //       std::string str = ss.str();

  //       if (str == "(") {
  //         Token tkn;
  //         tkn.type = Token_t::PAREN_L;
  //         out.push_back(tkn);
  //         continue;
  //       }

  //       if (str == ")") {
  //         Token tkn;
  //         tkn.type = Token_t::PAREN_R;
  //         out.push_back(tkn);
  //         continue;
  //       }

  //       float tmp_f;
  //       int tmp_i;
  //       ss >> tmp_i;
  //       if (!ss.fail()) {
  //         Token tkn;
  //         tkn.type = Token_t::LITERAL;
  //         tkn.ltype = Literal_t::INTEGER;
  //         tkn.it = tmp_i;
  //         out.push_back(tkn);
  //         continue;
  //       }

  //       ss >> tmp_f;
  //       if (!ss.fail()) {
  //         Token tkn;
  //         tkn.type = Token_t::LITERAL;
  //         tkn.ltype = Literal_t::FLOAT;
  //         tkn.fl = tmp_f;
  //         out.push_back(tkn);
  //         continue;
  //       }

  //       std::string tmp_s;
  //       ss >> tmp_s;
  //       Token tkn;
  //       tkn.type = Token_t::LITERAL;
  //       tkn.ltype = Literal_t::NAME;
  //       tkn.str = tmp_s;
  //       out.push_back(tkn);
  //       continue;
  //     }

  //     continue;
  //   }
  //   ss << c;
  // }
  return out;
}

// struct Syntax_Node {
//   Token tkn;
//   std::vector<std::unique_ptr<Syntax_Node>> children;
// };

// std::unique_ptr<Syntax_Node> syntax(std::vector<Token> const &tokens) {
//   // std::vector<std::unique_ptr<Syntax_Node>> out;
//   // (int b 0) (int a 0) (geti a) (geti b) (printi (+ a b))
//   std::deque<Syntax_Node *> node_stack;
//   std::unique_ptr<Syntax_Node> main_parent;
//   {
//     Syntax_Node *parent = new Syntax_Node;
//     parent->tkn = Token{
//         .type = Token_t::LITERAL, .ltype = Literal_t::NAME, .str = "main"};
//     main_parent.reset(parent);
//     node_stack.push_back(parent);
//   }
//   for (auto &tkn : tokens) {
//     if (tkn.type == Token_t::PAREN_L) {
//       node_stack.emplace_back(nullptr);
//       continue;
//     }
//     if (tkn.type == Token_t::PAREN_R) {
//       node_stack.pop_back();
//       continue;
//     }
//     if (tkn.type == Token_t::LITERAL) {
//       // Init function name
//       if (node_stack.back() == nullptr) {
//         assert(tkn.ltype == Literal_t::NAME);
//         Syntax_Node *parent = new Syntax_Node;
//         parent->tkn = tkn;
//         node_stack.pop_back();
//         node_stack.push_back(parent);
//       }
//       // Append argument
//       else {

//         node_stack.back()->children.emplace_back();
//       }
//       continue;
//     }
//     assert(false);
//   }
//   return main_parent;
// }
void test_Arrays();
void test_Hashtables();
int main(int argc, char **argv) {
  test_Arrays();
  test_Hashtables();
  // for (u32 i_id = 0, u64 *i = &arr->ptr[0]; i_id < arr->size; ++i_id, ++i)
  // assert(argc == 2);
  // auto tokens = tokenize(argv[0]);
  // auto stree = syntax(tokens);
  // std::cout << argv[1] << "\n";
  return 0;
}

u64 hash_of_u64(u64 u) {
  u64 v = u * 3935559000370003845 + 2691343689449507681;
  v ^= v >> 21;
  v ^= v << 37;
  v ^= v >> 4;
  v *= 4768777513237032717;
  v ^= v << 20;
  v ^= v >> 41;
  v ^= v << 5;
  return v;
}

u64 hash_of_string_ref(string_ref a) {
  u64 len = a.len;
  u64 const *u64ptr = (u64 const *)a.ptr;
  u64 hash = 0;
  while (len >= 8) {
    u64 a = *(u64 *)u64ptr;
    hash = hash ^ hash_of_u64(a);
    len -= 8;
    ++u64ptr;
  }
  u8 const *u8ptr = (u8 *)u64ptr;
  switch (len) {
  case 7:
    hash = hash ^ hash_of_u64((u64)*u8ptr);
    ++u8ptr;
    OK_FALLTHROGH
  case 6:
    hash = hash ^ hash_of_u64((u64)*u8ptr);
    ++u8ptr;
    OK_FALLTHROGH
  case 5:
    hash = hash ^ hash_of_u64((u64)*u8ptr);
    ++u8ptr;
    OK_FALLTHROGH
  case 4:
    hash = hash ^ hash_of_u64((u64)*u8ptr);
    ++u8ptr;
    OK_FALLTHROGH
  case 3:
    hash = hash ^ hash_of_u64((u64)*u8ptr);
    ++u8ptr;
    OK_FALLTHROGH
  case 2:
    hash = hash ^ hash_of_u64((u64)*u8ptr);
    ++u8ptr;
    OK_FALLTHROGH
  case 1:
    hash = hash ^ hash_of_u64((u64)*u8ptr);
    ++u8ptr;
    OK_FALLTHROGH
  default:
    break;
  }
  return hash;
}

u8 cmp_eq_u64(u64 a, u64 b) { return a == b ? TRUE : FALSE; }
u8 cmp_eq_string_ref(string_ref a, string_ref b) {
  if (a.ptr == NULL || b.ptr == NULL)
    return FALSE;
  return a.len != b.len ? FALSE
                        : strncmp(a.ptr, b.ptr, a.len) == 0 ? TRUE : FALSE;
}

// string view of a static string
string_ref stref_s(char const *static_string) {
  ASSERT_DEBUG(static_string != NULL);
  string_ref out;
  out.ptr = static_string;
  out.len = strlen(static_string);
  ASSERT_DEBUG(out.len != 0);
  return out;
}

// string view of a temporal string
string_ref stref_tmp(char const *tmp_string) {
  ASSERT_DEBUG(tmp_string != NULL);
  string_ref out;
  out.len = strlen(tmp_string);
  ASSERT_DEBUG(out.len != 0);
  char *ptr = (char *)tl_alloc_tmp(out.len);
  memcpy(ptr, tmp_string, out.len);
  out.ptr = (char const *)ptr;

  return out;
}

void test_Hashtables() {
  {
    HashArray_u64_u64 harr = HashArray_u64_u64_new(100);
    defer(HashArray_u64_u64_delete(&harr));
    HashArray_u64_u64_push(&harr, 100, 10);
    ASSERT_ALWAYS(HashArray_u64_u64_has(&harr, 100) == TRUE);
    ASSERT_ALWAYS(HashArray_u64_u64_has(&harr, 99) == FALSE);
  }
  {
    HashArray_u64_u64 harr = HashArray_u64_u64_new(0);
    defer(HashArray_u64_u64_delete(&harr));
    i32 N = 1000;
    ito(N) { HashArray_u64_u64_push(&harr, (u64)i, (u64)i * 3); }
    ito(N) {
      // fprintf(stdout, "checking %i\n", i);
      ASSERT_ALWAYS(HashArray_u64_u64_has(&harr, (u64)i) == TRUE);
    }
    ASSERT_ALWAYS(HashArray_u64_u64_has(&harr, 1000) == FALSE);
    ito(N) {
      // fprintf(stdout, "checking %i\n", i);
      u64 value = 0;
      ASSERT_ALWAYS(HashArray_u64_u64_get(&harr, (u64)i, &value) == TRUE);
      ASSERT_ALWAYS(value == (u64)i * 3);
    }
    ASSERT_ALWAYS(harr.arr.capacity < 2 * harr.item_count);
  }
  {
    HashArray_string_ref_string_ref harr =
        HashArray_string_ref_string_ref_new(0);
    defer(HashArray_string_ref_string_ref_delete(&harr));
    HashArray_string_ref_string_ref_push(&harr, stref_s("test key 7777"),
                                         stref_s("test value"));
    ASSERT_ALWAYS(HashArray_string_ref_string_ref_has(
                      &harr, stref_s("test key 7777")) == TRUE);
    ASSERT_ALWAYS(HashArray_string_ref_string_ref_has(
                      &harr, stref_s("test key 777")) == FALSE);
    ASSERT_ALWAYS(HashArray_string_ref_string_ref_has(
                      &harr, stref_s("test key 77")) == FALSE);
    ASSERT_ALWAYS(HashArray_string_ref_string_ref_has(
                      &harr, stref_s("test key 7")) == FALSE);
    ASSERT_ALWAYS(HashArray_string_ref_string_ref_has(
                      &harr, stref_s("test key")) == FALSE);
    {
      string_ref value;
      ASSERT_ALWAYS(HashArray_string_ref_string_ref_get(
                        &harr, stref_s("test key 7777"), &value) == TRUE);
      ASSERT_ALWAYS(cmp_eq_string_ref(stref_s("test value"), value) == TRUE);
    }
  }
  {
    tl_alloc_tmp_enter();
    defer(tl_alloc_tmp_exit());
    char buf[0x100];
    i32 N = 1000;
    HashArray_string_ref_string_ref harr =
        HashArray_string_ref_string_ref_new(0);
    defer(HashArray_string_ref_string_ref_delete(&harr));
    ito(N) {
      u64 rnd = hash_of_u64((u64)i);
      snprintf(buf, sizeof(buf), "key: %d", rnd);
      string_ref key = stref_tmp(buf);
      snprintf(buf, sizeof(buf), "value: %d", rnd);
      string_ref value = stref_tmp(buf);
      HashArray_string_ref_string_ref_push(&harr, key, value);
    }
    ito(N) {
      u64 rnd = hash_of_u64((u64)i);
      snprintf(buf, sizeof(buf), "key: %d", rnd);
      string_ref key = stref_tmp(buf);
      snprintf(buf, sizeof(buf), "value: %d", rnd);
      string_ref value = stref_tmp(buf);
      string_ref ret_value;
      ASSERT_ALWAYS(HashArray_string_ref_string_ref_get(&harr, key, &ret_value) == TRUE);
      ASSERT_ALWAYS(cmp_eq_string_ref(ret_value, value) == TRUE);
    }
    string_ref_string_ref_FOREACH(harr, i, {
      u64 rnd = 0;
      snprintf(buf, sizeof(buf), "%.*s\0", i->key.len, i->key.ptr);
      // fprintf(stdout, "%s\n", buf);
      sscanf(buf, "key: %d", &rnd);

      snprintf(buf, sizeof(buf), "key: %d\0", rnd);
      string_ref key = stref_tmp(buf);
      snprintf(buf, sizeof(buf), "value: %d\0", rnd);
      string_ref value = stref_tmp(buf);
      // fprintf(stdout, "table[%.*s] = %.*s\n", key.len, key.ptr, value.len,
      //         value.ptr);
      // fprintf(stdout, "table[%.*s] = %.*s\n", i->key.len, i->key.ptr,
      //         i->value.len, i->value.ptr);
      ASSERT_ALWAYS(cmp_eq_string_ref(key, i->key) == TRUE);
      ASSERT_ALWAYS(cmp_eq_string_ref(value, i->value) == TRUE);
    });

    // check for meaningful occupancy
    ASSERT_ALWAYS(harr.arr.capacity < 2 * harr.item_count);
  }

  fprintf(stdout, "[SUCCESS] test_Hashtables\n");
}

void test_Arrays() {
  {
    Array_u64 arr = Array_u64_new(0);
    defer(Array_u64_delete(&arr));
    i32 N = 1000;
    ito(N) { Array_u64_push(&arr, (u64)i); }
    u64_FOREACH(arr, i, {
      ASSERT_ALWAYS(*i == i_id);
      // fprintf(stdout, "arr[%i] = %i, ", i_id, *i);
    });
  }
  {
    Array_u64 arr = Array_u64_new(500);
    defer(Array_u64_delete(&arr));
    i32 N = 1000;
    ito(N) { Array_u64_push(&arr, (u64)i); }
    u64_FOREACH(arr, i, { ASSERT_ALWAYS(*i == i_id); });
  }
  {
    Array_u64 arr = Array_u64_new(0);
    defer(Array_u64_delete(&arr));
    i32 N = 1000;
    ito(N) { Array_u64_push(&arr, (u64)i); }
    ito(N) { ASSERT_ALWAYS(Array_u64_pop(&arr) == N - i - 1); }
    u64_FOREACH(arr, i, { ASSERT_ALWAYS(false && "unreachable!"); });
    ASSERT_ALWAYS(arr.size == 0);
  }
  fprintf(stdout, "[SUCCESS] test_Arrays\n");
}