
// NOTE: The majority of these preprocessor defintions conflict with <iostream>
//       and several of the components that make it up.
#define internal static
#define local_persist static
#define global static
#define external extern "C"

// SECTION Data Types
typedef char      i8;
typedef short     i16;
typedef int       i32;
typedef long long i64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef i32     b32;
typedef float   r32;
typedef double  r64;
// !SECTION

// SECTION Bit Manipulation
#define Kilobytes(Value) ((Value) << 10)
#define Megabytes(Value) (Kilobytes(Value) << 10)
#define Gigabytes(Value) (Megabytes(Value) << 10)
#define Terabytes(Value) (Gigabytes(Value) << 10)

#define Bitwise_Bool(b) ((b) == 0 ? (0) : (~0))
// !SECTION

// SECTION Utility
// NOTE: https://www.geeksforgeeks.org/using-sizof-operator-with-array-paratmeters/
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// NOTE: For stripping parentheses from 
#define _Args(...) __VA_ARGS__
#define Strip_(x) x
#define Strip(x) Strip_(_Args x)
// LINK: http://bruceblinn.com/linuxinfo/DoWhile.html
#define Statement(s) do { s } while(0)
// NOTE: Types should be passed inside of parentheses.
#define Swap(t, x, y) Statement(Strip(t) Swap = x; x = y; y = Swap;)
// !SECTION

// SECTION Token pasting for use with combining preprocessor string literals
#define stringify_(s) #s
#define stringify(s) stringify_(s)
#define glue_(a,b) a##b
#define glue(a,b) glue_(a,b)
// !SECTION

// SECTION Switches
#define InvalidDefaultCase default:{}break
// !Section

// SECTION Strings
// NOTE: This is an internal definition of Windows file system max path.
//       It's defined to be semantically identical with Microsoft's MAX_PATH.
//       259 chars, plus one for terminating NUL.
#define ANSI_MAX_PATH 260
// !SECTION