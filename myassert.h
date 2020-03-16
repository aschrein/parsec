#ifndef MYASSERT_H
#define MYASSERT_H

#include <stdio.h>

#define ASSERT_ALWAYS(x) { if (!(x)) {fprintf(stderr, "[FAIL] at %s:%i \n", __FILE__, __LINE__); (void)(*(int*)(NULL)=0);} }
#define ASSERT_DEBUG(x) ASSERT_ALWAYS(x)

#endif