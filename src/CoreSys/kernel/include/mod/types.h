#pragma once

#include <stddef.h>
#include <stdint.h>

typedef int ret_t; // Return
typedef int num_t; // Number
typedef char let_t; // Letter
typedef uint8_t unum8_t; // unsigned number 8
typedef uint16_t unum16_t; // unsigned number 16
typedef uint32_t unum32_t; // unsigned number 32
typedef uint64_t unum64_t; // unsigned number 64
typedef int8_t num8_t; // signed number 64
typedef int16_t num16_t; // signed number 16
typedef int32_t num32_t; // signed number 32
typedef int64_t num64_t; // signed number 64
typedef unsigned uncon_t; // Unsiged
typedef signed con_t; // Signed 
typedef size_t cap_t; // Capacity

#define set_t const
#define nret_t void // No Return
#define vol_t volatile // Voliatile