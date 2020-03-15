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
  u8 *ptr;
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
#include <templates/__u64_array.h>
#include <templates/__u64_u64_hashtable.h>

// struct Thread_Local {
//   Array_u64 GC;
//   u8 initialized = 0;
//   ~Thread_Local() {
//     Array_u64_delete(&GC);
//   }
// };

// // TODO(aschrein): Change to __thread?
// thread_local Thread_Local g_tl{};

// Thread_Local *get_tl() {
//   if (g_tl.initialized == 0) {
//     g_tl.initialized = 1;
//     g_tl.GC = Array_u64_new(0x100);
//   }
//   return &g_tl;
// }

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

u8 cmp_eq_u64(u64 a, u64 b) { return (u8)(a - b); }

void test_Hashtables() {
  {
    HashArray_u64_u64 harr = HashArray_u64_u64_new(100);
    HashArray_u64_u64_push(&harr, 100, 10);
    ASSERT_ALWAYS(HashArray_u64_u64_has(&harr, 100) == TRUE);
    ASSERT_ALWAYS(HashArray_u64_u64_has(&harr, 99) == FALSE);
  }
  {
    HashArray_u64_u64 harr = HashArray_u64_u64_new(0);
    i32 N = 1000;
    ito(N) { HashArray_u64_u64_push(&harr, (u64)i, 10); }
    ito(N) {
      // fprintf(stdout, "checking %i\n", i);
      ASSERT_ALWAYS(HashArray_u64_u64_has(&harr, (u64)i) == TRUE);
    }
    ASSERT_ALWAYS(HashArray_u64_u64_has(&harr, 1000) == FALSE);
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