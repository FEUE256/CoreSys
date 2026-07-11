#pragma once

#include <drivers/sf/main.h>
#include <mod/types.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define assert(expr) do {                     \
    if (!(expr)) {                            \
        k_sf("ASSERT: " #expr                 \
             " at " __FILE__ ":" STR(__LINE__)); \
    }                                         \
} while (0)

#define assert_hard(expr) do { \
    k_sf("ASSERT: " #expr " at " __FILE__ ":" STR(__LINE__)); \
} while (0)
