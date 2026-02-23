/*
   Copyright (C) 2015 Intel Corporation

   This software and the related documents are Intel copyrighted materials, and
   your use of them is governed by the express license under which they were
   provided to you ("License"). Unless the License provides otherwise, you may
   not use, modify, copy, publish, distribute, disclose or transmit this
   software or the related documents without Intel's prior written permission.

   This software and the related documents are provided as is, with no express
   or implied warranties, other than those that are expressly stated in the
   License.
*/

/* Short-vector random number generators (SVRNG) library */

#ifndef __SVRNG_H__
#define __SVRNG_H__

#if (!defined(__INTEL_COMPILER) && (!defined(__INTEL_CLANG_COMPILER) || !defined(__INTEL_LLVM_COMPILER)))
# error "<svrng.h> is for use with only Intel(R) compilers!"
#endif

#include <stdint.h>
#include <immintrin.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*
    // ERROR HANDLING CONSTANTS
    */
    #define SVRNG_STATUS_OK                  (  0  )
    #define SVRNG_STATUS_ERROR_BAD_PARAM1    ( -1  )
    #define SVRNG_STATUS_ERROR_BAD_PARAM2    ( -2  )
    #define SVRNG_STATUS_ERROR_BAD_PARAM3    ( -3  )
    #define SVRNG_STATUS_ERROR_BAD_PARAM4    ( -4  )
    #define SVRNG_STATUS_ERROR_BAD_PARAMS    ( -10 )
    #define SVRNG_STATUS_ERROR_BAD_ENGINE    ( -11 )
    #define SVRNG_STATUS_ERROR_BAD_DISTR     ( -12 )
    #define SVRNG_STATUS_ERROR_MEMORY_ALLOC  ( -13 )
    #define SVRNG_STATUS_ERROR_UNSUPPORTED   ( -14 )

    /*vectorization declaration macro */
    #define _VECTOR_VARIANT_(impl,len,uargs) __declspec(vector_variant(implements(impl),vectorlength(len),uniform uargs, nomask, processor(_PROCESSOR_)))

    /*
    // CPU-SPECIFIC TYPE DEFINITIONS AND KERNEL FUNCTION DECLARATIONS
    */
    /******************* CORE AVX512 **********************************************************************/
    #if  defined (__AVX512F__) && defined (__AVX512DQ__) && defined (__AVX512BW__) && defined (__AVX512VL__)

        #ifndef _CPU_
            #define _CPU_        coreavx512
        #endif
        #ifndef _PROCESSOR_
            #define _PROCESSOR_  future_cpu_23
        #endif
        #ifndef _ALIGN_
            #define _ALIGN_      __declspec(align(64))
        #endif

        /* unsigned 32-bit integer types */
        typedef __m128i svrng_uint1_t;
        typedef __m128i svrng_uint2_t;
        typedef __m128i svrng_uint4_t;
        typedef __m256i svrng_uint8_t;
        typedef __m512i svrng_uint16_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_{ __m512i r[2]; } svrng_uint32_t;
        #else
            typedef struct _ALIGN_{ __m512i r1; __m512i r2; } svrng_uint32_t;
        #endif

        /* unsigned 64-bit integer types */
        typedef __m128i svrng_ulong1_t;
        typedef __m128i svrng_ulong2_t;
        typedef __m256i svrng_ulong4_t;
        typedef __m512i svrng_ulong8_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_{ __m512i r[2]; } svrng_ulong16_t;
            typedef struct _ALIGN_{ __m512i r[4]; } svrng_ulong32_t;
        #else
            typedef struct _ALIGN_{ __m512i r1; __m512i r2; } svrng_ulong16_t;
            typedef struct _ALIGN_{ __m512i r1; __m512i r2; __m512i r3; __m512i r4; } svrng_ulong32_t;
        #endif

        /* signed 32-bit integer types */
        typedef __m128i svrng_int1_t;
        typedef __m128i svrng_int2_t;
        typedef __m128i svrng_int4_t;
        typedef __m256i svrng_int8_t;
        typedef __m512i svrng_int16_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_{ __m512i r[2]; } svrng_int32_t;
        #else
            typedef struct _ALIGN_{ __m512i r1; __m512i r2; } svrng_int32_t;
        #endif

        /* single precision floating point types */
        typedef __m128 svrng_float1_t;
        typedef __m128 svrng_float2_t;
        typedef __m128 svrng_float4_t;
        typedef __m256 svrng_float8_t;
        typedef __m512 svrng_float16_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_{ __m512 r[2]; } svrng_float32_t;
        #else
            typedef struct _ALIGN_{ __m512 r1; __m512 r2; } svrng_float32_t;
        #endif

        /* double precision floating point types */
        typedef __m128d svrng_double1_t;
        typedef __m128d svrng_double2_t;
        typedef __m256d svrng_double4_t;
        typedef __m512d svrng_double8_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_{ __m512d r[2]; } svrng_double16_t;
            typedef struct _ALIGN_{ __m512d r[4]; } svrng_double32_t;
        #else
            typedef struct _ALIGN_{ __m512d r1; __m512d r2; } svrng_double16_t;
            typedef struct _ALIGN_{ __m512d r1; __m512d r2; __m512d r3; __m512d r4; } svrng_double32_t;
        #endif

        /* vectorization declarations dependent on basic data type sizes */
        /* (4,8) bytes and SIMD vector lengths (1-32) elements */
        /* 32-bit data */
        #define _KERNEL_VECTOR_CONV_coreavx512_4_1(impl,len,uargs)  /* SCALAR BY DEFINITION */
        #define _KERNEL_VECTOR_CONV_coreavx512_4_2(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx512_4_4(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx512_4_8(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx512_4_16(impl,len,uargs) _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx512_4_32(impl,len,uargs) _VECTOR_VARIANT_(impl,len,uargs)
        /* 64-bit data */
        #define _KERNEL_VECTOR_CONV_coreavx512_8_1(impl,len,uargs) /* SCALAR BY DEFINITION */
        #define _KERNEL_VECTOR_CONV_coreavx512_8_2(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx512_8_4(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx512_8_8(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx512_8_16(impl,len,uargs) _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx512_8_32(impl,len,uargs) _VECTOR_VARIANT_(impl,len,uargs)

    /******************* MIC AVX512 or CORE AVX2 **********************************************************************/
    #elif  (defined (__AVX512F__) && defined (__AVX512CD__) && defined (__AVX512ER__) && defined (__AVX512PF__)) || defined (__MIC__) || defined (__AVX2__)

        #ifndef _CPU_
            #define _CPU_        coreavx2
        #endif
        #ifndef _PROCESSOR_
            #define _PROCESSOR_  future_cpu_20
        #endif
        #ifndef _ALIGN_
            #define _ALIGN_      __declspec(align(32))
        #endif

        /* unsigned 32-bit integer types */
        typedef __m128i svrng_uint1_t;
        typedef __m128i svrng_uint2_t;
        typedef __m128i svrng_uint4_t;
        typedef __m256i svrng_uint8_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_  { __m256i r[2]; } svrng_uint16_t;
            typedef struct _ALIGN_  { __m256i r[4]; } svrng_uint32_t;
        #else
            typedef struct _ALIGN_  { __m256i r1; __m256i r2; } svrng_uint16_t;
            typedef struct _ALIGN_  { __m256i r1; __m256i r2; __m256i r3; __m256i r4; } svrng_uint32_t;
        #endif

        /* unsigned 64-bit integer types */
        typedef __m128i svrng_ulong1_t;
        typedef __m128i svrng_ulong2_t;
        typedef __m256i svrng_ulong4_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_  { __m256i r[2]; } svrng_ulong8_t;
            typedef struct _ALIGN_  { __m256i r[4]; } svrng_ulong16_t;
            typedef struct _ALIGN_  { __m256i r[8]; } svrng_ulong32_t;
        #else
            typedef struct _ALIGN_  { __m256i r1; __m256i r2; } svrng_ulong8_t;
            typedef struct _ALIGN_  { __m256i r1; __m256i r2; __m256i r3; __m256i r4; } svrng_ulong16_t;
            typedef struct _ALIGN_  { __m256i r1; __m256i r2; __m256i r3; __m256i r4; __m256i r5; __m256i r6; __m256i r7; __m256i r8; } svrng_ulong32_t;
        #endif

        /* signed 32-bit integer types */
        typedef __m128i svrng_int1_t;
        typedef __m128i svrng_int2_t;
        typedef __m128i svrng_int4_t;
        typedef __m256i svrng_int8_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_  { __m256i r[2]; } svrng_int16_t;
            typedef struct _ALIGN_  { __m256i r[4]; } svrng_int32_t;
        #else
            typedef struct _ALIGN_  { __m256i r1; __m256i r2; } svrng_int16_t;
            typedef struct _ALIGN_  { __m256i r1; __m256i r2; __m256i r3; __m256i r4; } svrng_int32_t;
        #endif

        /* single precision floating point types */
        typedef __m128 svrng_float1_t;
        typedef __m128 svrng_float2_t;
        typedef __m128 svrng_float4_t;
        typedef __m256 svrng_float8_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_  { __m256 r[2]; } svrng_float16_t;
            typedef struct _ALIGN_  { __m256 r[4]; } svrng_float32_t;
        #else
            typedef struct _ALIGN_  { __m256 r1; __m256 r2; } svrng_float16_t;
            typedef struct _ALIGN_  { __m256 r1; __m256 r2; __m256 r3; __m256 r4; } svrng_float32_t;
        #endif

        /* double precision floating point types */
        typedef __m128d svrng_double1_t;
        typedef __m128d svrng_double2_t;
        typedef __m256d svrng_double4_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_  { __m256d r[2]; } svrng_double8_t;
            typedef struct _ALIGN_  { __m256d r[4]; } svrng_double16_t;
            typedef struct _ALIGN_  { __m256d r[8]; } svrng_double32_t;
        #else
            typedef struct _ALIGN_  { __m256d r1; __m256d r2; } svrng_double8_t;
            typedef struct _ALIGN_  { __m256d r1; __m256d r2; __m256d r3; __m256d r4; } svrng_double16_t;
            typedef struct _ALIGN_  { __m256d r1; __m256d r2; __m256d r3; __m256d r4; __m256d r5; __m256d r6; __m256d r7; __m256d r8; } svrng_double32_t;
        #endif

        /* vectorization declarations dependent on basic data type sizes */
        /* (4,8) bytes and SIMD vector lengths (1-32) elements */
        /* 32-bit data */
        #define _KERNEL_VECTOR_CONV_coreavx2_4_1(impl,len,uargs) /* SCALAR BY DEFINITION */
        #define _KERNEL_VECTOR_CONV_coreavx2_4_2(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx2_4_4(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx2_4_8(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx2_4_16(impl,len,uargs) _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx2_4_32(impl,len,uargs) _VECTOR_VARIANT_(impl,len,uargs)
        /* 64-bit data */
        #define _KERNEL_VECTOR_CONV_coreavx2_8_1(impl,len,uargs) /* SCALAR BY DEFINITION */
        #define _KERNEL_VECTOR_CONV_coreavx2_8_2(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx2_8_4(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx2_8_8(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx2_8_16(impl,len,uargs) _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_coreavx2_8_32(impl,len,uargs) _VECTOR_VARIANT_(impl,len,uargs)

    /******************* SSE2 **********************************************************************/
    #else

        #ifndef _CPU_
            #define _CPU_        sse2
        #endif
        #ifndef _PROCESSOR_
            #define _PROCESSOR_  pentium_4_sse3
        #endif
        #ifndef _ALIGN_
            #define _ALIGN_      __declspec(align(16))
        #endif

        /* unsigned 32-bit integer types */
        typedef __m128i svrng_uint1_t;
        typedef __m128i svrng_uint2_t;
        typedef __m128i svrng_uint4_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_ { __m128i r[2]; } svrng_uint8_t;
            typedef struct _ALIGN_ { __m128i r[4]; } svrng_uint16_t;
            typedef struct _ALIGN_ { __m128i r[8]; } svrng_uint32_t;
        #else
            typedef struct _ALIGN_ { __m128i r1; __m128i r2; } svrng_uint8_t;
            typedef struct _ALIGN_ { __m128i r1; __m128i r2; __m128i r3; __m128i r4; } svrng_uint16_t;
            typedef struct _ALIGN_ { __m128i r1; __m128i r2; __m128i r3; __m128i r4; __m128i r5; __m128i r6; __m128i r7; __m128i r8; } svrng_uint32_t;
        #endif

        /* unsigned 64-bit integer types */
        typedef __m128i svrng_ulong1_t;
        typedef __m128i svrng_ulong2_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_ { __m128i r[2]; }  svrng_ulong4_t;
            typedef struct _ALIGN_ { __m128i r[4]; }  svrng_ulong8_t;
            typedef struct _ALIGN_ { __m128i r[8]; }  svrng_ulong16_t;
            typedef struct _ALIGN_ { __m128i r[16]; } svrng_ulong32_t;
        #else
            typedef struct _ALIGN_ { __m128i r1; __m128i r2; }  svrng_ulong4_t;
            typedef struct _ALIGN_ { __m128i r1; __m128i r2; __m128i r3; __m128i r4; }  svrng_ulong8_t;
            typedef struct _ALIGN_ { __m128i r1; __m128i r2; __m128i r3; __m128i r4; __m128i r5; __m128i r6; __m128i r7; __m128i r8; }  svrng_ulong16_t;
            typedef struct _ALIGN_ { __m128i r1; __m128i r2; __m128i r3; __m128i r4; __m128i r5; __m128i r6; __m128i r7; __m128i r8; __m128i r9;
                __m128i r10; __m128i r11; __m128i r12; __m128i r13; __m128i r14; __m128i r15; __m128i r16; } svrng_ulong32_t;
        #endif

        /* signed 32-bit integer types */
        typedef __m128i svrng_int1_t;
        typedef __m128i svrng_int2_t;
        typedef __m128i svrng_int4_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_ { __m128i r[2]; } svrng_int8_t;
            typedef struct _ALIGN_ { __m128i r[4]; } svrng_int16_t;
            typedef struct _ALIGN_ { __m128i r[8]; } svrng_int32_t;
        #else
            typedef struct _ALIGN_ { __m128i r1; __m128i r2; } svrng_int8_t;
            typedef struct _ALIGN_ { __m128i r1; __m128i r2; __m128i r3; __m128i r4; } svrng_int16_t;
            typedef struct _ALIGN_ { __m128i r1; __m128i r2; __m128i r3; __m128i r4; __m128i r5; __m128i r6; __m128i r7; __m128i r8; } svrng_int32_t;
        #endif

        /* single precision floating point types */
        typedef __m128 svrng_float1_t;
        typedef __m128 svrng_float2_t;
        typedef __m128 svrng_float4_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_ { __m128 r[2]; } svrng_float8_t;
            typedef struct _ALIGN_ { __m128 r[4]; } svrng_float16_t;
            typedef struct _ALIGN_ { __m128 r[8]; } svrng_float32_t;
        #else
            typedef struct _ALIGN_ { __m128 r1; __m128 r2; } svrng_float8_t;
            typedef struct _ALIGN_ { __m128 r1; __m128 r2; __m128 r3; __m128 r4; } svrng_float16_t;
            typedef struct _ALIGN_ { __m128 r1; __m128 r2; __m128 r3; __m128 r4; __m128 r5; __m128 r6; __m128 r7; __m128 r8; } svrng_float32_t;
        #endif

        /* double precision floating point types */
        typedef __m128d svrng_double1_t;
        typedef __m128d svrng_double2_t;
        #if (defined(__INTEL_COMPILER))
            typedef struct _ALIGN_ { __m128d r[2]; }  svrng_double4_t;
            typedef struct _ALIGN_ { __m128d r[4]; }  svrng_double8_t;
            typedef struct _ALIGN_ { __m128d r[8]; }  svrng_double16_t;
            typedef struct _ALIGN_ { __m128d r[16]; } svrng_double32_t;
        #else
            typedef struct _ALIGN_ { __m128d r1; __m128d r2; }  svrng_double4_t;
            typedef struct _ALIGN_ { __m128d r1; __m128d r2; __m128d r3; __m128d r4; }  svrng_double8_t;
            typedef struct _ALIGN_ { __m128d r1; __m128d r2; __m128d r3; __m128d r4; __m128d r5; __m128d r6; __m128d r7; __m128d r8; }  svrng_double16_t;
            typedef struct _ALIGN_ { __m128d r1; __m128d r2; __m128d r3; __m128d r4; __m128d r5; __m128d r6; __m128d r7; __m128d r8; __m128d r9;
                __m128d r10; __m128d r11; __m128d r12; __m128d r13; __m128d r14; __m128d r15; __m128d r16; } svrng_double32_t;
        #endif

        /*vectorization declarations dependent on SIMD sizes (4,8) and vector lengths (1-32) */
        /* 32-bit data */
        #define _KERNEL_VECTOR_CONV_sse2_4_1(impl,len,uargs) /* SCALAR BY DEFINITION */
        #define _KERNEL_VECTOR_CONV_sse2_4_2(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_sse2_4_4(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_sse2_4_8(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_sse2_4_16(impl,len,uargs) _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_sse2_4_32(impl,len,uargs) /*  _VECTOR_VARIANT_(impl,len,uargs) */ /* inefficient on SSE2 */
        /* 64-bit data */
        #define _KERNEL_VECTOR_CONV_sse2_8_1(impl,len,uargs) /* SCALAR BY DEFINITION */
        #define _KERNEL_VECTOR_CONV_sse2_8_2(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_sse2_8_4(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_sse2_8_8(impl,len,uargs)  _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_sse2_8_16(impl,len,uargs) _VECTOR_VARIANT_(impl,len,uargs)
        #define _KERNEL_VECTOR_CONV_sse2_8_32(impl,len,uargs) /* _VECTOR_VARIANT_(impl,len,uargs) */ /* inefficient on SSE2 */

    #endif /* #if  defined (__AVX512F__) && defined (__AVX512DQ__) && defined (__AVX512BW__) && defined (__AVX512VL__) */

    /* string construction pre-processor macro */
    #define GLUE(a,b)                    a##b
    #define PASTE(a,b)                   GLUE(a,b)
    #define PASTE2(a,b)                  PASTE(a,b)
    #define PASTE3(a,b,c)                PASTE(PASTE2(a,b),c)
    #define PASTE4(a,b,c,d)              PASTE(PASTE3(a,b,c),d)
    #define PASTE5(a,b,c,d,e)            PASTE(PASTE4(a,b,c,d),e)
    #define PASTE6(a,b,c,d,e,f)          PASTE(PASTE5(a,b,c,d,e),f)

    /* CPU-specific kernel function and type names */
    #define _KERNEL_NAME_(prefix,name)      PASTE4(prefix,_CPU_,_,name)

    /* CPU-specific kernel scalar function conventions */
    #if defined(_WIN32) || defined(_WIN64)
        #define _KERNEL_CONV_    __regcall
    #else
        #define _KERNEL_CONV_    __attribute__((regcall))
    #endif

    /* Interface function conventions */
    #define _API_CONV_      static __forceinline _KERNEL_CONV_

    /* CPU-specific kernel vector function conventions for auto-vectorization */
    #ifndef _NOAUTOVECTOR_
        #define _KERNEL_VECTOR_CONV_(impl,uargs,sz,len)  PASTE6(_KERNEL_VECTOR_CONV_,_CPU_,_,sz,_,len)(impl,len,uargs) _KERNEL_CONV_
    #else
        #define _KERNEL_VECTOR_CONV_(impl,uargs,sz,len) _KERNEL_CONV_
    #endif


    /*
    // DATA TYPES FOR ENGINES AND DISTRIBUTIONS
    */
    typedef void* svrng_engine_t;
    typedef void* svrng_distribution_t;

    /*
    // CPU-SPECIFIC KENREL FUNCTION PROTOTYPES
    */

    /* ---------------------------------- ERROR HANDLER ----------------------------------------- */
    /*
    // int32_t svrng_set_status(int32_t status);
    //      Purpose: Return old status, sets new status.
    //      Parameters: int32_t status - new status
    //      Return: int32_t - old status
    */
     int32_t             _KERNEL_CONV_ _KERNEL_NAME_(svrng_,set_status)(int32_t status);

    /*
    // int32_t svrng_get_status(void);
    //      Purpose: Return current status.
    //      Parameters: none
    //      Return: int32_t - current status
    */
     int32_t             _KERNEL_CONV_ _KERNEL_NAME_(svrng_,get_status)(void);

    /* ------------------------------- ENGINES INITIALIZATION ---------------------------------- */
    /*
    // svrng_engine_t  svrng_new_rand0_engine( uint32_t seed );
    //      Purpose: Create and initialize rand0 engine by single seed value
    //      Parameters: uint32_t seed - seed value
    //      Return: svrng_engine_t - created engine pointer (or NULL on error)
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_rand0_engine)( uint32_t seed );

    /*
    // svrng_engine_t  svrng_new_rand0_engine_ex( int num, uint32_t* pseed );
    //      Purpose: Create and initialize rand0 engine by series of seed values
    //      Parameters: int num - number of seeds, uint32_t* pseed - pointer to seeds array
    //                  Function returns error if num < 0 or num > MAX,
    //                  where MAX is maximum number of 32-bit values to initialize SIMD register on CPU
    //                  MAX = 4 on SSE2, = 8 on AVX, = 16 on MIC, etc..
    //      Return: svrng_engine_t - created engine pointer (or NULL on error)
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_rand0_engine_ex)( int num, uint32_t* pseed );

    /*
    // svrng_engine_t  svrng_new_rand_engine( uint32_t seed );
    //      Note: everything the same as for rand0
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_rand_engine)( uint32_t seed );

    /*
    // svrng_engine_t  svrng_new_rand_engine( int num, uint32_t* pseed );
    //      Note: everything the same as for rand0
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_rand_engine_ex)( int num, uint32_t* pseed );

    /*
    // svrng_engine_t  svrng_new_mcg31m1_engine( uint32_t seed );
    //      Note: everything the same as for rand0
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_mcg31m1_engine)( uint32_t seed );

    /*
    // svrng_engine_t  svrng_new_mcg31m1_engine( int num, uint32_t* pseed );
    //      Note: everything the same as for rand0
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_mcg31m1_engine_ex)( int num, uint32_t* pseed );

    /*
    // svrng_engine_t  svrng_new_mcg59_engine( uint64_t seed );
    //      Purpose: Create and initialize mcg59 engine by single seed value
    //      Parameters: uint64_t seed - seed value
    //                  The seed value is 64-bit here since mcg59 is 64-bit generator engine
    //      Return: svrng_engine_t - created engine pointer (or NULL on error)
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_mcg59_engine)( uint64_t seed );

    /*
    // svrng_engine_t  svrng_new_mcg59_engine_ex( int num, uint64_t* pseed );
    //      Purpose: Create and initialize mcg59 engine by series of seed values
    //      Parameters: int num - number of seeds, uint64_t* pseed - pointer to 64-bit seeds array
    //                  Function returns error if num < 0 or num > MAX,
    //                  where MAX is maximum number of 64-bit values to initialize SIMD register on CPU
    //                  MAX = 2 on SSE2, = 4 on AVX, = 8 on MIC, etc..
    //      Return: svrng_engine_t - created engine pointer (or NULL on error)
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_mcg59_engine_ex)( int num, uint64_t* pseed );

    /*
    // svrng_engine_t  svrng_new_mt19937_engine( uint32_t seed );
    //      Purpose: Create and initialize mt19937 engine by single seed value
    //      Parameters: uint32_t seed - seed value
    //      Return: svrng_engine_t created engine pointer (or NULL on error)
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_mt19937_engine)( uint32_t seed );

    /*
    // svrng_engine_t  svrng_new_mt19937_engine_ex( int num, uint32_t* pseed );
    //      Purpose: Create and initialize mt19937 engine by series of seed values
    //      Parameters: int num - number of seeds, uint32_t* pseed - pointer to seeds array
    //                  Function returns error if num < 0 or num > 624,
    //      Return: svrng_engine_t - created engine pointer (or NULL on error)
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_mt19937_engine_ex)( int num, uint32_t* pseed );

    /*
    // svrng_engine_t  svrng_copy_engine( svrng_engine_t pengine );
    //      Purpose: Create full copy of an engine
    //      Parameters: svrng_engine_t pengine - engine to be copied. Memory allocated for new engine.
    //      Return: svrng_engine_t - created engine pointer (or NULL on error)
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,copy_engine)( svrng_engine_t pengine );

    /*
    // svrng_engine_t  svrng_delete_engine( svrng_engine_t pengine );
    //      Purpose: Delete engine
    //      Parameters: svrng_engine_t pengine - engine to be deleted. Memory is deallocated
    //      Return: svrng_engine_t - NULL pointer
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,delete_engine)( svrng_engine_t pengine );

    /*
    // svrng_engine_t  svrng_skipahead_engine( svrng_engine_t pengine, long long offset );
    //      Purpose: Re-initialize engine parameters for "skipahead" technique parallel computations
    //      Parameters: svrng_engine_t pengine - engine to be skipahead-reinitialized. Memory NOT allocated for new engine.
    //                  long long offset - offset from original seed
    //      Return: svrng_engine_t - skipahead-engine pointer (or NULL on error)
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,skipahead_engine)( svrng_engine_t pengine, long long offset );

    /*
    // svrng_engine_t  svrng_leapfrog_engine( svrng_engine_t pengine, int offset, int stride );
    //      Purpose: Re-initialize engine parameters for "leapfrog" technique parallel computations
    //      Parameters: svrng_engine_t pengine - engine to be leapfrog-reinitialized. Memory NOT allocated for new engine.
    //                  int offset - offset from original seed
    //                  int stride - stride for generated sequence
    //      Return: svrng_engine_t - leapfrog-engine pointer (or NULL on error)
    */
     svrng_engine_t      _KERNEL_CONV_ _KERNEL_NAME_(svrng_,leapfrog_engine)( svrng_engine_t pengine, int offset, int stride );

    /* ------------------------------- DISTRIBUTIONS INITIALIZATION ---------------------------------- */
    /*
    // svrng_distribution_t svrng_new_uniform_distribution_int( int  a, int  b );
    //      Purpose: Create and initialize integer uniform distribution
    //      Parameters: int a, int b - [a,b) interval for integer uniform numbers generation
    //      Return: svrng_distribution_t - created distribution pointer (or NULL on error)
    */
     svrng_distribution_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_uniform_distribution_int)( int  a, int  b );

    /*
    // svrng_distribution_t svrng_new_uniform_distribution_float( float  a, float  b );
    //      Purpose: Create and initialize single precision floating point uniform distribution
    //      Parameters: float a, float b - [a,b) interval for single precision uniform numbers generation
    //      Return: svrng_distribution_t - created distribution pointer (or NULL on error)
    */
     svrng_distribution_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_uniform_distribution_float)( float  a, float  b );

    /*
    // svrng_distribution_t svrng_new_uniform_distribution_double( double  a, double  b );
    //      Purpose: Create and initialize double precision floating point uniform distribution
    //      Parameters: double a, double b - [a,b) interval for double precision uniform numbers generation
    //      Return: svrng_distribution_t - created distribution pointer (or NULL on error)
    */
     svrng_distribution_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_uniform_distribution_double)( double a, double b );

    /*
    // svrng_distribution_t svrng_new_normal_distribution_float( float  mean, float  stddev );
    //      Purpose: Create and initialize single precision floating point normal (Gaussian) distribution
    //      Parameters: float mean - mean value of Gaussian distribution
    //                  float stddev - standard deviation value of Gaussian distribution
    //      Return: svrng_distribution_t - created distribution pointer (or NULL on error)
    */
     svrng_distribution_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_normal_distribution_float)( float  mean, float  stddev );
    /*
    // svrng_distribution_t svrng_new_normal_distribution_double( double  mean, double  stddev );
    //      Purpose: Create and initialize double precision floating point normal (Gaussian) distribution
    //      Parameters: double mean - mean value of Gaussian distribution
    //                  double stddev - standard deviation value of Gaussian distribution
    //      Return: svrng_distribution_t - created distribution pointer (or NULL on error)
    */
     svrng_distribution_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,new_normal_distribution_double)( double mean, double stddev );

    /*
    // svrng_distribution_t svrng_update_uniform_distribution_int( svrng_distribution_t pdistr, int  a, int  b );
    //      Purpose: Update integer uniform distribution parameters
    //      Parameters: svrng_distribution_t pdistr - distribution to be updated. Memory is NOT allocated.
    //                  int a, int b - [a,b) interval for integer uniform numbers generation
    //      Return: svrng_distribution_t - updated distribution pointer (or NULL on error)
    */
     svrng_distribution_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,update_uniform_distribution_int)( svrng_distribution_t pdistr, int  a, int b );

    /*
    // svrng_distribution_t svrng_update_uniform_distribution_float( svrng_distribution_t pdistr, float  a, float  b );
    //      Purpose: Update single precision floating point uniform distribution parameters
    //      Parameters: svrng_distribution_t pdistr - distribution to be updated. Memory is NOT allocated.
    //                  float a, float b - [a,b) interval for single precision uniform numbers generation
    //      Return: svrng_distribution_t - updated distribution pointer (or NULL on error)
    */
     svrng_distribution_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,update_uniform_distribution_float)( svrng_distribution_t pdistr, float  a, float  b );

    /*
    // svrng_distribution_t svrng_update_uniform_distribution_double( svrng_distribution_t pdistr, double  a, double  b );
    //      Purpose: Update double precision floating point uniform distribution parameters
    //      Parameters: svrng_distribution_t pdistr - distribution to be updated. Memory is NOT allocated.
    //                  double a, double b - [a,b) interval for double precision uniform numbers generation
    //      Return: svrng_distribution_t - updated distribution pointer (or NULL on error)
    */
     svrng_distribution_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,update_uniform_distribution_double)( svrng_distribution_t pdistr, double a, double b );

    /*
    // svrng_distribution_t svrng_update_normal_distribution_float( svrng_distribution_t pdistr, float  mean, float  stddev );
    //      Purpose: Update single precision floating point normal (Gaussian) distribution parameters
    //      Parameters: svrng_distribution_t pdistr - distribution to be updated. Memory is NOT allocated.
    //                  float mean - mean value of Gaussian distribution
    //                  float stddev - standard deviation value of Gaussian distribution
    //      Return: svrng_distribution_t - updated distribution pointer (or NULL on error)
    */
     svrng_distribution_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,update_normal_distribution_float)(  svrng_distribution_t  pdistr, float  mean, float  stddev );

    /*
    // svrng_distribution_t svrng_update_normal_distribution_double( svrng_distribution_t pdistr, double  mean, double  stddev );
    //      Purpose: Update double precision floating point normal (Gaussian) distribution parameters
    //      Parameters: svrng_distribution_t pdistr - distribution to be updated. Memory is NOT allocated.
    //                  double mean - mean value of Gaussian distribution
    //                  double stddev - standard deviation value of Gaussian distribution
    //      Return: svrng_distribution_t - updated distribution pointer (or NULL on error)
    */
     svrng_distribution_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,update_normal_distribution_double)( svrng_distribution_t  pdistr, double mean, double stddev );

    /*
    // svrng_distribution_t svrng_delete_distribution( svrng_distribution_t pdistr );
    //      Purpose: Delete distribution
    //      Parameters: svrng_distribution_t pdistr - distribution to be deleted. Memory deallocated.
    //      Return: svrng_distribution_t - NULL pointer
    */
     svrng_distribution_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,delete_distribution)( svrng_distribution_t pdistr );

    /*
    // uint32_t svrng_generate_uint( svrng_engine_t pengine );
    //      Purpose: Generate 32-bit unsigned integer unscaled uniformly distributed random value
    //      Parameters: svrng_engine_t pengine - basic generator engine
    //      Return: uint32_t - 32-bit unsigned integer random value
    //      Note: 64-bit engines (mcg59) causes error
    */
     uint32_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,generate_uint)( svrng_engine_t pengine );

    /*
    // svrng_uint*_t svrng_generate*_uint( svrng_engine_t pengine );
    //      Purpose: Generate 'n' 32-bit unsigned integer unscaled uniformly distributed random values
    //               returned in SIMD vector register(s). 'n' = 1,2,4,8,16,32
    //      Parameters: svrng_engine_t pengine - basic generator engine
    //      Return: svrng_uint*_t - 'n' 32-bit unsigned integer random values
    //      Note: 64-bit engines (mcg59) causes error
    */
    svrng_uint1_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_uint)(svrng_engine_t pengine),(pengine),4,1 ) _KERNEL_NAME_(svrng_,generate1_uint)(  svrng_engine_t pengine );
    svrng_uint2_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_uint)(svrng_engine_t pengine),(pengine),4,2 ) _KERNEL_NAME_(svrng_,generate2_uint)(  svrng_engine_t pengine );
    svrng_uint4_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_uint)(svrng_engine_t pengine),(pengine),4,4 ) _KERNEL_NAME_(svrng_,generate4_uint)(  svrng_engine_t pengine );
    svrng_uint8_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_uint)(svrng_engine_t pengine),(pengine),4,8 ) _KERNEL_NAME_(svrng_,generate8_uint)(  svrng_engine_t pengine );
    svrng_uint16_t _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_uint)(svrng_engine_t pengine),(pengine),4,16) _KERNEL_NAME_(svrng_,generate16_uint)( svrng_engine_t pengine );
    svrng_uint32_t _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_uint)(svrng_engine_t pengine),(pengine),4,32) _KERNEL_NAME_(svrng_,generate32_uint)( svrng_engine_t pengine );

    /*
    // uint64_t svrng_generate_ulong( svrng_engine_t pengine );
    //      Purpose: Generate 64-bit unsigned integer unscaled uniformly distributed random value
    //      Parameters: svrng_engine_t pengine - basic generator engine
    //      Return: uint64_t - 64-bit unsigned integer random value
    //      Note: 32-bit engines (rand0,rand,mcg31m1,mt19937) causes error
    */
     uint64_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,generate_ulong)( svrng_engine_t pengine );

    /*
    // svrng_ulong*_t svrng_generate*_ulong( svrng_engine_t pengine );
    //      Purpose: Generate 'n' 64-bit unsigned integer unscaled uniformly distributed random values
    //               returned in SIMD vector register(s). 'n' = 1,2,4,8,16,32
    //      Parameters: svrng_engine_t pengine - basic generator engine
    //      Return: svrng_ulong*_t - 'n' 64-bit unsigned integer random values
    //      Note: 32-bit engines (rand0,rand,mcg31m1,mt19937) causes error
    */
    svrng_ulong1_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_ulong)(svrng_engine_t pengine),(pengine),8,1 ) _KERNEL_NAME_(svrng_,generate1_ulong)(  svrng_engine_t pengine );
    svrng_ulong2_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_ulong)(svrng_engine_t pengine),(pengine),8,2 ) _KERNEL_NAME_(svrng_,generate2_ulong)(  svrng_engine_t pengine );
    svrng_ulong4_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_ulong)(svrng_engine_t pengine),(pengine),8,4 ) _KERNEL_NAME_(svrng_,generate4_ulong)(  svrng_engine_t pengine );
    svrng_ulong8_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_ulong)(svrng_engine_t pengine),(pengine),8,8 ) _KERNEL_NAME_(svrng_,generate8_ulong)(  svrng_engine_t pengine );
    svrng_ulong16_t _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_ulong)(svrng_engine_t pengine),(pengine),8,16) _KERNEL_NAME_(svrng_,generate16_ulong)( svrng_engine_t pengine );
    svrng_ulong32_t _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_ulong)(svrng_engine_t pengine),(pengine),8,32) _KERNEL_NAME_(svrng_,generate32_ulong)( svrng_engine_t pengine );

    /*
    // int32_t svrng_generate_int( svrng_engine_t pengine, svrng_distribution_t pdistr );
    //      Purpose: Generate 32-bit signed integer random value
    //      Parameters: svrng_engine_t pengine - basic generator engine
    //                  svrng_distribution_t pdistr - distribution
    //      Return: uint32_t - 32-bit integer random value
    //      Note: some distributions (normal) cannot generate integer values, cause error
    */
     int32_t _KERNEL_CONV_ _KERNEL_NAME_(svrng_,generate_int)(   svrng_engine_t pengine, svrng_distribution_t pdistr );

    /*
    // svrng_int*_t svrng_generate*_int( svrng_engine_t pengine, svrng_distribution_t pdistr );
    //      Purpose: Generate 'n' 32-bit signed integer random values
    //               returned in SIMD vector register(s). 'n' = 1,2,4,8,16,32
    //      Parameters: svrng_engine_t pengine - basic generator engine
    //                  svrng_distribution_t pdistr - distribution
    //      Return: svrng_int*_t - 'n' 32-bit integer random values
    //      Note: some distributions (normal) cannot generate integer values, cause error
    */
    svrng_int1_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_int)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),4,1 ) _KERNEL_NAME_(svrng_,generate1_int)(  svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_int2_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_int)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),4,2 ) _KERNEL_NAME_(svrng_,generate2_int)(  svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_int4_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_int)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),4,4 ) _KERNEL_NAME_(svrng_,generate4_int)(  svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_int8_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_int)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),4,8 ) _KERNEL_NAME_(svrng_,generate8_int)(  svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_int16_t _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_int)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),4,16) _KERNEL_NAME_(svrng_,generate16_int)( svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_int32_t _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_int)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),4,32) _KERNEL_NAME_(svrng_,generate32_int)( svrng_engine_t pengine, svrng_distribution_t pdistr );

    /*
    // float svrng_generate_float( svrng_engine_t pengine, svrng_distribution_t pdistr );
    //      Purpose: Generate single precision floating point random value
    //      Parameters: svrng_engine_t pengine - basic generator engine
    //                  svrng_distribution_t pdistr - distribution
    //      Return: float - single precision floating point random value
    */
     float _KERNEL_CONV_ _KERNEL_NAME_(svrng_,generate_float)( svrng_engine_t pengine, svrng_distribution_t pdistr );

    /*
    // svrng_float*_t svrng_generate*_float( svrng_engine_t pengine, svrng_distribution_t pdistr );
    //      Purpose: Generate 'n' single precision floating point random values
    //               returned in SIMD vector register(s). 'n' = 1,2,4,8,16,32
    //      Parameters: svrng_engine_t pengine - basic generator engine
    //                  svrng_distribution_t pdistr - distribution
    //      Return: svrng_float*_t - 'n' single precision floating point random values
    */
    svrng_float1_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_float)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),4,1 ) _KERNEL_NAME_(svrng_,generate1_float)(  svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_float2_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_float)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),4,2 ) _KERNEL_NAME_(svrng_,generate2_float)(  svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_float4_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_float)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),4,4 ) _KERNEL_NAME_(svrng_,generate4_float)(  svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_float8_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_float)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),4,8 ) _KERNEL_NAME_(svrng_,generate8_float)(  svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_float16_t _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_float)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),4,16) _KERNEL_NAME_(svrng_,generate16_float)( svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_float32_t _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_float)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),4,32) _KERNEL_NAME_(svrng_,generate32_float)( svrng_engine_t pengine, svrng_distribution_t pdistr );

    /*
    // double svrng_generate_double( svrng_engine_t pengine, svrng_distribution_t pdistr );
    //      Purpose: Generate double precision floating point random value
    //      Parameters: svrng_engine_t pengine - basic generator engine
    //                  svrng_distribution_t pdistr - distribution
    //      Return: float - double precision floating point random value
    */
     double _KERNEL_CONV_ _KERNEL_NAME_(svrng_,generate_double)( svrng_engine_t pengine, svrng_distribution_t pdistr );

    /*
    // svrng_double*_t svrng_generate*_double( svrng_engine_t pengine, svrng_distribution_t pdistr );
    //      Purpose: Generate 'n' double precision floating point random values
    //               returned in SIMD vector register(s). 'n' = 1,2,4,8,16,32
    //      Parameters: svrng_engine_t pengine - basic generator engine
    //                  svrng_distribution_t pdistr - distribution
    //      Return: svrng_double*_t - 'n' single precision floating point random values
    */
    svrng_double1_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_double)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),8,1 ) _KERNEL_NAME_(svrng_,generate1_double)(  svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_double2_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_double)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),8,2 ) _KERNEL_NAME_(svrng_,generate2_double)(  svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_double4_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_double)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),8,4 ) _KERNEL_NAME_(svrng_,generate4_double)(  svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_double8_t  _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_double)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),8,8 ) _KERNEL_NAME_(svrng_,generate8_double)(  svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_double16_t _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_double)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),8,16) _KERNEL_NAME_(svrng_,generate16_double)( svrng_engine_t pengine, svrng_distribution_t pdistr );
    svrng_double32_t _KERNEL_VECTOR_CONV_(_KERNEL_NAME_(svrng_,generate_double)(svrng_engine_t pengine, svrng_distribution_t pdistr),(pengine,pdistr),8,32) _KERNEL_NAME_(svrng_,generate32_double)( svrng_engine_t pengine, svrng_distribution_t pdistr );


    /*
    // API FUNCTION NAMES (MAPPING TO KERNELS - COMPILE-TIME CPU-DISPATCHER IMPLEMENTATION)
    */
    int32_t _API_CONV_ svrng_set_status( int32_t status )                                                                               { return _KERNEL_NAME_(svrng_,set_status)( status ); }
    int32_t _API_CONV_ svrng_get_status( void )                                                                                         { return _KERNEL_NAME_(svrng_,get_status)( ); }

    svrng_engine_t _API_CONV_ svrng_new_rand0_engine( uint32_t seed )                                                                   { return _KERNEL_NAME_(svrng_,new_rand0_engine)( seed ); }
    svrng_engine_t _API_CONV_ svrng_new_rand0_engine_ex( int num, uint32_t* pseed )                                                     { return _KERNEL_NAME_(svrng_,new_rand0_engine_ex)( num, pseed ); }
    svrng_engine_t _API_CONV_ svrng_new_rand_engine( uint32_t seed )                                                                    { return _KERNEL_NAME_(svrng_,new_rand_engine)( seed ); }
    svrng_engine_t _API_CONV_ svrng_new_rand_engine_ex( int num, uint32_t* pseed )                                                      { return _KERNEL_NAME_(svrng_,new_rand_engine_ex)( num, pseed ); }
    svrng_engine_t _API_CONV_ svrng_new_mcg31m1_engine( uint32_t seed )                                                                 { return _KERNEL_NAME_(svrng_,new_mcg31m1_engine)( seed ); }
    svrng_engine_t _API_CONV_ svrng_new_mcg31m1_engine_ex( int num, uint32_t* pseed )                                                   { return _KERNEL_NAME_(svrng_,new_mcg31m1_engine_ex)( num, pseed ); }
    svrng_engine_t _API_CONV_ svrng_new_mcg59_engine( uint64_t seed )                                                                   { return _KERNEL_NAME_(svrng_,new_mcg59_engine)( seed ); }
    svrng_engine_t _API_CONV_ svrng_new_mcg59_engine_ex( int num, uint64_t* pseed )                                                     { return _KERNEL_NAME_(svrng_,new_mcg59_engine_ex)( num, pseed ); }
    svrng_engine_t _API_CONV_ svrng_new_mt19937_engine( uint32_t seed )                                                                 { return _KERNEL_NAME_(svrng_,new_mt19937_engine)( seed ); }
    svrng_engine_t _API_CONV_ svrng_new_mt19937_engine_ex( int num, uint32_t* pseed )                                                   { return _KERNEL_NAME_(svrng_,new_mt19937_engine_ex)( num, pseed ); }

    svrng_engine_t _API_CONV_ svrng_copy_engine( svrng_engine_t pengine )                                                               { return _KERNEL_NAME_(svrng_,copy_engine)( pengine ); }
    svrng_engine_t _API_CONV_ svrng_delete_engine( svrng_engine_t pengine )                                                             { return _KERNEL_NAME_(svrng_,delete_engine)( pengine ); }
    svrng_engine_t _API_CONV_ svrng_skipahead_engine( svrng_engine_t pengine, long long offset )                                        { return _KERNEL_NAME_(svrng_,skipahead_engine)( pengine, offset ); }
    svrng_engine_t _API_CONV_ svrng_leapfrog_engine( svrng_engine_t pengine, int offset, int stride )                                   { return _KERNEL_NAME_(svrng_,leapfrog_engine)( pengine, offset, stride ); }

    svrng_distribution_t _API_CONV_ svrng_new_uniform_distribution_int( int  a, int  b )                                                { return _KERNEL_NAME_(svrng_,new_uniform_distribution_int)( a, b ); }
    svrng_distribution_t _API_CONV_ svrng_new_uniform_distribution_float( float  a, float  b )                                          { return _KERNEL_NAME_(svrng_,new_uniform_distribution_float)( a, b ); }
    svrng_distribution_t _API_CONV_ svrng_new_uniform_distribution_double( double a, double b )                                         { return _KERNEL_NAME_(svrng_,new_uniform_distribution_double)( a, b ); }
    svrng_distribution_t _API_CONV_ svrng_new_normal_distribution_float( float  mean, float  stddev )                                   { return _KERNEL_NAME_(svrng_,new_normal_distribution_float)( mean, stddev ); }
    svrng_distribution_t _API_CONV_ svrng_new_normal_distribution_double( double mean, double stddev )                                  { return _KERNEL_NAME_(svrng_,new_normal_distribution_double)( mean, stddev ); }
    svrng_distribution_t _API_CONV_ svrng_update_uniform_distribution_int( svrng_distribution_t pdistr, int  a, int b )                 { return _KERNEL_NAME_(svrng_,update_uniform_distribution_int)( pdistr, a, b ); }
    svrng_distribution_t _API_CONV_ svrng_update_uniform_distribution_float( svrng_distribution_t pdistr, float  a, float  b )          { return _KERNEL_NAME_(svrng_,update_uniform_distribution_float)( pdistr, a, b ); }
    svrng_distribution_t _API_CONV_ svrng_update_uniform_distribution_double( svrng_distribution_t pdistr, double a, double b )         { return _KERNEL_NAME_(svrng_,update_uniform_distribution_double)( pdistr, a, b ); }
    svrng_distribution_t _API_CONV_ svrng_update_normal_distribution_float(  svrng_distribution_t  pdistr, float  mean, float  stddev ) { return _KERNEL_NAME_(svrng_,update_normal_distribution_float)( pdistr, mean, stddev ); }
    svrng_distribution_t _API_CONV_ svrng_update_normal_distribution_double( svrng_distribution_t  pdistr, double mean, double stddev ) { return _KERNEL_NAME_(svrng_,update_normal_distribution_double)( pdistr, mean, stddev ); }
    svrng_distribution_t _API_CONV_ svrng_delete_distribution( svrng_distribution_t pdistr )                                            { return _KERNEL_NAME_(svrng_,delete_distribution)( pdistr ); }

    uint32_t       _API_CONV_ svrng_generate_uint( svrng_engine_t pengine )                                                             { return _KERNEL_NAME_(svrng_,generate_uint)( pengine ); }
    svrng_uint1_t  _API_CONV_ svrng_generate1_uint( svrng_engine_t pengine )                                                            { return _KERNEL_NAME_(svrng_,generate1_uint)( pengine ); }
    svrng_uint2_t  _API_CONV_ svrng_generate2_uint( svrng_engine_t pengine )                                                            { return _KERNEL_NAME_(svrng_,generate2_uint)( pengine ); }
    svrng_uint4_t  _API_CONV_ svrng_generate4_uint( svrng_engine_t pengine )                                                            { return _KERNEL_NAME_(svrng_,generate4_uint)( pengine ); }
    svrng_uint8_t  _API_CONV_ svrng_generate8_uint( svrng_engine_t pengine )                                                            { return _KERNEL_NAME_(svrng_,generate8_uint)( pengine ); }
    svrng_uint16_t _API_CONV_ svrng_generate16_uint( svrng_engine_t pengine )                                                           { return _KERNEL_NAME_(svrng_,generate16_uint)( pengine ); }
    svrng_uint32_t _API_CONV_ svrng_generate32_uint( svrng_engine_t pengine )                                                           { return _KERNEL_NAME_(svrng_,generate32_uint)( pengine ); }

    uint64_t        _API_CONV_ svrng_generate_ulong( svrng_engine_t pengine )                                                           { return _KERNEL_NAME_(svrng_,generate_ulong)( pengine ); }
    svrng_ulong1_t  _API_CONV_ svrng_generate1_ulong( svrng_engine_t pengine )                                                          { return _KERNEL_NAME_(svrng_,generate1_ulong)( pengine ); }
    svrng_ulong2_t  _API_CONV_ svrng_generate2_ulong( svrng_engine_t pengine )                                                          { return _KERNEL_NAME_(svrng_,generate2_ulong)( pengine ); }
    svrng_ulong4_t  _API_CONV_ svrng_generate4_ulong( svrng_engine_t pengine )                                                          { return _KERNEL_NAME_(svrng_,generate4_ulong)( pengine ); }
    svrng_ulong8_t  _API_CONV_ svrng_generate8_ulong( svrng_engine_t pengine )                                                          { return _KERNEL_NAME_(svrng_,generate8_ulong)( pengine ); }
    svrng_ulong16_t _API_CONV_ svrng_generate16_ulong( svrng_engine_t pengine )                                                         { return _KERNEL_NAME_(svrng_,generate16_ulong)( pengine ); }
    svrng_ulong32_t _API_CONV_ svrng_generate32_ulong( svrng_engine_t pengine )                                                         { return _KERNEL_NAME_(svrng_,generate32_ulong)( pengine ); }

    int32_t       _API_CONV_ svrng_generate_int( svrng_engine_t pengine, svrng_distribution_t pdistr )                                  { return _KERNEL_NAME_(svrng_,generate_int)( pengine, pdistr ); }
    svrng_int1_t  _API_CONV_ svrng_generate1_int( svrng_engine_t pengine, svrng_distribution_t pdistr )                                 { return _KERNEL_NAME_(svrng_,generate1_int)( pengine, pdistr ); }
    svrng_int2_t  _API_CONV_ svrng_generate2_int( svrng_engine_t pengine, svrng_distribution_t pdistr )                                 { return _KERNEL_NAME_(svrng_,generate2_int)( pengine, pdistr ); }
    svrng_int4_t  _API_CONV_ svrng_generate4_int( svrng_engine_t pengine, svrng_distribution_t pdistr )                                 { return _KERNEL_NAME_(svrng_,generate4_int)( pengine, pdistr ); }
    svrng_int8_t  _API_CONV_ svrng_generate8_int( svrng_engine_t pengine, svrng_distribution_t pdistr )                                 { return _KERNEL_NAME_(svrng_,generate8_int)( pengine, pdistr ); }
    svrng_int16_t _API_CONV_ svrng_generate16_int( svrng_engine_t pengine, svrng_distribution_t pdistr )                                { return _KERNEL_NAME_(svrng_,generate16_int)( pengine, pdistr ); }
    svrng_int32_t _API_CONV_ svrng_generate32_int( svrng_engine_t pengine, svrng_distribution_t pdistr )                                { return _KERNEL_NAME_(svrng_,generate32_int)( pengine, pdistr ); }

    float           _API_CONV_ svrng_generate_float( svrng_engine_t pengine,  svrng_distribution_t pdistr )                             { return _KERNEL_NAME_(svrng_,generate_float)( pengine, pdistr ); }
    svrng_float1_t  _API_CONV_ svrng_generate1_float( svrng_engine_t pengine, svrng_distribution_t pdistr )                             { return _KERNEL_NAME_(svrng_,generate1_float)( pengine, pdistr ); }
    svrng_float2_t  _API_CONV_ svrng_generate2_float( svrng_engine_t pengine, svrng_distribution_t pdistr )                             { return _KERNEL_NAME_(svrng_,generate2_float)( pengine, pdistr ); }
    svrng_float4_t  _API_CONV_ svrng_generate4_float( svrng_engine_t pengine, svrng_distribution_t pdistr )                             { return _KERNEL_NAME_(svrng_,generate4_float)( pengine, pdistr ); }
    svrng_float8_t  _API_CONV_ svrng_generate8_float( svrng_engine_t pengine, svrng_distribution_t pdistr )                             { return _KERNEL_NAME_(svrng_,generate8_float)( pengine, pdistr ); }
    svrng_float16_t _API_CONV_ svrng_generate16_float( svrng_engine_t pengine, svrng_distribution_t pdistr )                            { return _KERNEL_NAME_(svrng_,generate16_float)( pengine, pdistr ); }
    svrng_float32_t _API_CONV_ svrng_generate32_float( svrng_engine_t pengine, svrng_distribution_t pdistr )                            { return _KERNEL_NAME_(svrng_,generate32_float)( pengine, pdistr ); }

    double           _API_CONV_ svrng_generate_double( svrng_engine_t pengine,  svrng_distribution_t pdistr )                           { return _KERNEL_NAME_(svrng_,generate_double)( pengine, pdistr ); }
    svrng_double1_t  _API_CONV_ svrng_generate1_double( svrng_engine_t pengine, svrng_distribution_t pdistr )                           { return _KERNEL_NAME_(svrng_,generate1_double)( pengine, pdistr ); }
    svrng_double2_t  _API_CONV_ svrng_generate2_double( svrng_engine_t pengine, svrng_distribution_t pdistr )                           { return _KERNEL_NAME_(svrng_,generate2_double)( pengine, pdistr ); }
    svrng_double4_t  _API_CONV_ svrng_generate4_double( svrng_engine_t pengine, svrng_distribution_t pdistr )                           { return _KERNEL_NAME_(svrng_,generate4_double)( pengine, pdistr ); }
    svrng_double8_t  _API_CONV_ svrng_generate8_double( svrng_engine_t pengine, svrng_distribution_t pdistr )                           { return _KERNEL_NAME_(svrng_,generate8_double)( pengine, pdistr ); }
    svrng_double16_t _API_CONV_ svrng_generate16_double( svrng_engine_t pengine, svrng_distribution_t pdistr )                          { return _KERNEL_NAME_(svrng_,generate16_double)( pengine, pdistr ); }
    svrng_double32_t _API_CONV_ svrng_generate32_double( svrng_engine_t pengine, svrng_distribution_t pdistr )                          { return _KERNEL_NAME_(svrng_,generate32_double)( pengine, pdistr ); }


#ifdef __cplusplus
}
#endif

#endif /* #define __SVRNG_H__ */
