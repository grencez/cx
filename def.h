/**
 * \file def.h
 * All important definitions.
 **/
#ifndef DEF_H_
#define DEF_H_
#ifndef __OPENCL_VERSION__
#include <float.h>
#include <limits.h>
#include <stddef.h>
#endif  /* #ifndef __OPENCL_VERSION__ */

typedef int Bit;
//enum Trit { Nil = -1, Yes = 1, May = 0 };
enum Trit { Nil = 0, Yes = 1, May = 2 };
typedef char Sign;
typedef char Bool;
#define MPI_Bool MPI_CHAR

typedef enum Trit Trit;
/** Type of function which compares two objects.
 * Return values should conform to:
 *  -1: lhs < rhs
 *   0: lhs == rhs
 *  +1: lhs > rhs
 **/
typedef Sign (* PosetCmpFn) (const void*, const void*);

    /** Define bool.**/
#if !defined(__cplusplus) && !defined(__OPENCL_VERSION__)
typedef char bool;
#define true 1
#define false 0
#endif

typedef unsigned char byte;
#define NBitsInByte 8
#define NBits_byte 8
#define Max_byte ((byte)0xFF)

#if !defined(__OPENCL_VERSION__)
typedef unsigned int uint;
#endif  /* #ifndef __OPENCL_VERSION__ */
#define Max_uint UINT_MAX
#define NBits_uint (NBits_byte*sizeof(uint))

typedef long int jint;
typedef unsigned long int ujint;
#define Max_ujint (~(ujint)0)
typedef byte ujintlg;
#define NBits_ujint (NBits_byte*sizeof(ujint))
#define Max_ujintlg  Max_byte
#define MaxCk_ujintlg(x)  ((x) == Max_byte)

#if !defined(__OPENCL_VERSION__)
typedef struct uint2 uint2;
struct uint2 { uint s[2]; };
#endif  /* #ifndef __OPENCL_VERSION__ */
typedef struct ujint2 ujint2;
struct ujint2 { ujint s[2]; };

#ifndef uint32
#define uint32 uint
#endif
#ifndef Max_uint32
#define Max_uint32 (0xFFFFFFFFu)
#endif

#if 0
typedef double real;
#define Max_real DBL_MAX
#define Min_real (-DBL_MAX)
#define Small_real DBL_MIN
#define Epsilon_real DBL_EPSILON
#define realPackSz 2
#define GL_REAL GL_DOUBLE

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MPI_real MPI_DOUBLE

#else
typedef float real;
#define Max_real FLT_MAX
#define Min_real (-FLT_MAX)
#define Small_real FLT_MIN
#define Epsilon_real FLT_EPSILON
#define realPackSz 4
#define GL_REAL GL_FLOAT

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define MPI_real MPI_FLOAT

#endif


#ifdef _MSC_VER
    /* Disable warning: 'fopen' unsafe, use fopen_s instead */
    /* REF CMakeLists.txt: Define _CRT_SECURE_NO_WARNINGS */

    /* Disable: conditional expression is constant */
# pragma warning (disable : 4127)
    /* Disable: conversion from 'uint' to 'real' */
# pragma warning (disable : 4244)
    /* Disable: conversion from 'double' to 'float' */
# pragma warning (disable : 4305)
#endif


#if __STDC_VERSION__ < 199901L
#define inline __inline
#ifdef _MSC_VER
#define restrict
#else
#define restrict __restrict
#endif
#endif

#define qual_inline static inline

#ifdef _MSC_VER
# define __FUNC__ __FUNCTION__
#else
# define __FUNC__ __func__
#endif


#ifndef __OPENCL_VERSION__
#include "synhax.h"
#endif

#endif

