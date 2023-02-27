#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include <stdint.h>

#define global        static
#define internal      static
#define local_persist static

typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef i32       b32;
typedef float     r32;
typedef double    r64;

#define ArraySize(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Stringify_(s) #s
#define Stringify(s) Stringify_(s)
#define Join_(a,b) a##b
#define Join(a,b) Join_(a,b)

#define InvalidDefaultCase default:{}break

#endif
