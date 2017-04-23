#ifndef RV_TYPES_H
#define RV_TYPES_H

#include "re_core_global.h"

#ifdef __cplusplus
extern "C" {
#endif

// re_*
typedef unsigned long re_addr_t;

// elf
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef int32_t register_t;

// PE/win
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;

#ifdef __cplusplus
}
#endif

#endif // RV_TYPES_H
