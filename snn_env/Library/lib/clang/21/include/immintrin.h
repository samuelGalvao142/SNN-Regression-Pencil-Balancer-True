/*===---- immintrin.h - Intel intrinsics -----------------------------------=== */
/*
 * Modifications, Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/*
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __IMMINTRIN_H
#define __IMMINTRIN_H

/* Turn fp precise on for intrinsics, push state to restore at end. */
#pragma float_control(push)
#pragma float_control(precise, on)

/* Enable fast contract if it was enabled on the command line */
#pragma STDC FP_CONTRACT DEFAULT

/// TODO: Move it into clang/lib/Headers/amxintrin.h when all amx disclosed.

/// AMX tile register size can be configured, the maximum size is 16x64=1024
/// bytes. Since there is no 2D type in llvm IR, we use vector type to
/// represent 2D tile and the fixed size is maximum amx tile register size.
typedef int _tile1024i __attribute__((__vector_size__(1024), __aligned__(64)));
typedef int _tile1024i_1024a __attribute__((__vector_size__(1024), __aligned__(1024)));

/// This struct pack the shape and tile data together for user. We suggest
/// initializing the struct as early as possible, because compiler depends
/// on the shape information to do configure. The constant value is preferred
/// for optimization by compiler.
typedef struct __tile1024i_str {
  const unsigned short row;
  const unsigned short col;
  _tile1024i tile;
} __tile1024i;

#if !defined(__i386__) && !defined(__x86_64__)
#error "This header is only meant to be used on x86 and x64 architecture"
#endif

#include <x86gprintrin.h>

#include <ia32intrin.h>
#include <mmintrin.h>

#include <xmmintrin.h>

#include <emmintrin.h>


#include <pmmintrin.h>

#include <tmmintrin.h>

#include <smmintrin.h>

#include <wmmintrin.h>

#include <clflushoptintrin.h>

#include <clwbintrin.h>

#include <avxintrin.h>

#include <avx2intrin.h>

#include <f16cintrin.h>

#include <bmiintrin.h>

#include <bmi2intrin.h>

#include <lzcntintrin.h>

#include <popcntintrin.h>

#include <fmaintrin.h>

#include <avx512fintrin.h>

#include <avx512vlintrin.h>

#include <avx512bwintrin.h>

#include <avx512bitalgintrin.h>

#include <avx512cdintrin.h>

#include <avx512vpopcntdqintrin.h>

#include <avx512vpopcntdqvlintrin.h>

#include <avx512vnniintrin.h>

#include <avx512vlvnniintrin.h>

#include <avxvnniintrin.h>

#include <avx512dqintrin.h>

#include <avx512vlbitalgintrin.h>

#include <avx512vlbwintrin.h>

#include <avx512vlcdintrin.h>

#include <avx512vldqintrin.h>

#include <avx512ifmaintrin.h>

#include <avx512ifmavlintrin.h>

#include <avxifmaintrin.h>

#include <avx512vbmiintrin.h>

#include <avx512vbmivlintrin.h>

#include <avx512vbmi2intrin.h>

#include <avx512vlvbmi2intrin.h>

#include <avx512fp16intrin.h>

#include <avx512vlfp16intrin.h>

#include <avx512bf16intrin.h>

#include <avx512vlbf16intrin.h>

#include <pkuintrin.h>

#include <vpclmulqdqintrin.h>

#include <vaesintrin.h>

#include <gfniintrin.h>

#include <avxvnniint8intrin.h>

#include <avxneconvertintrin.h>













#include <sha512intrin.h>

#include <sm3intrin.h>

#include <sm4intrin.h>

#include <avxvnniint16intrin.h>

/// Reads the value of the IA32_TSC_AUX MSR (0xc0000103).
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> RDPID </c> instruction.
///
/// \returns The 32-bit contents of the MSR.
static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__, __target__("rdpid")))
_rdpid_u32(void) {
  return __builtin_ia32_rdpid();
}

/// Returns a 16-bit hardware-generated random value.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> RDRAND </c> instruction.
///
/// \param __p
///    A pointer to a 16-bit memory location to place the random value.
/// \returns 1 if the value was successfully generated, 0 otherwise.
static __inline__ int __attribute__((__always_inline__, __nodebug__, __target__("rdrnd")))
_rdrand16_step(unsigned short *__p)
{
  return (int)__builtin_ia32_rdrand16_step(__p);
}

/// Returns a 32-bit hardware-generated random value.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> RDRAND </c> instruction.
///
/// \param __p
///    A pointer to a 32-bit memory location to place the random value.
/// \returns 1 if the value was successfully generated, 0 otherwise.
static __inline__ int __attribute__((__always_inline__, __nodebug__, __target__("rdrnd")))
_rdrand32_step(unsigned int *__p)
{
  return (int)__builtin_ia32_rdrand32_step(__p);
}

/// Returns a 64-bit hardware-generated random value.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> RDRAND </c> instruction.
///
/// \param __p
///    A pointer to a 64-bit memory location to place the random value.
/// \returns 1 if the value was successfully generated, 0 otherwise.
static __inline__ int __attribute__((__always_inline__, __nodebug__, __target__("rdrnd")))
_rdrand64_step(unsigned long long *__p)
{
#ifdef __x86_64__
  return (int)__builtin_ia32_rdrand64_step(__p);
#else
  // We need to emulate the functionality of 64-bit rdrand with 2 32-bit
  // rdrand instructions.
  unsigned int __lo, __hi;
  unsigned int __res_lo = __builtin_ia32_rdrand32_step(&__lo);
  unsigned int __res_hi = __builtin_ia32_rdrand32_step(&__hi);
  if (__res_lo && __res_hi) {
    *__p = ((unsigned long long)__hi << 32) | (unsigned long long)__lo;
    return 1;
  } else {
    *__p = 0;
    return 0;
  }
#endif
}

#ifdef __x86_64__
/// Reads the FS base register.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> RDFSBASE </c> instruction.
///
/// \returns The lower 32 bits of the FS base register.
static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_readfsbase_u32(void)
{
  return __builtin_ia32_rdfsbase32();
}

/// Reads the FS base register.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> RDFSBASE </c> instruction.
///
/// \returns The contents of the FS base register.
static __inline__ unsigned long long __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_readfsbase_u64(void)
{
  return __builtin_ia32_rdfsbase64();
}

/// Reads the GS base register.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> RDGSBASE </c> instruction.
///
/// \returns The lower 32 bits of the GS base register.
static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_readgsbase_u32(void)
{
  return __builtin_ia32_rdgsbase32();
}

/// Reads the GS base register.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> RDGSBASE </c> instruction.
///
/// \returns The contents of the GS base register.
static __inline__ unsigned long long __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_readgsbase_u64(void)
{
  return __builtin_ia32_rdgsbase64();
}

/// Modifies the FS base register.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> WRFSBASE </c> instruction.
///
/// \param __V
///    Value to use for the lower 32 bits of the FS base register.
static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_writefsbase_u32(unsigned int __V)
{
  __builtin_ia32_wrfsbase32(__V);
}

/// Modifies the FS base register.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> WRFSBASE </c> instruction.
///
/// \param __V
///    Value to use for the FS base register.
static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_writefsbase_u64(unsigned long long __V)
{
  __builtin_ia32_wrfsbase64(__V);
}

/// Modifies the GS base register.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> WRGSBASE </c> instruction.
///
/// \param __V
///    Value to use for the lower 32 bits of the GS base register.
static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_writegsbase_u32(unsigned int __V)
{
  __builtin_ia32_wrgsbase32(__V);
}

/// Modifies the GS base register.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> WRFSBASE </c> instruction.
///
/// \param __V
///    Value to use for GS base register.
static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_writegsbase_u64(unsigned long long __V)
{
  __builtin_ia32_wrgsbase64(__V);
}

#endif


/* The structs used below are to force the load/store to be unaligned. This
 * is accomplished with the __packed__ attribute. The __may_alias__ prevents
 * tbaa metadata from being generated based on the struct and the type of the
 * field inside of it.
 */

/// Load a 16-bit value from memory and swap its bytes.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the MOVBE instruction.
///
/// \param __P
///    A pointer to the 16-bit value to load.
/// \returns The byte-swapped value.
static __inline__ short __attribute__((__always_inline__, __nodebug__, __target__("movbe")))
_loadbe_i16(void const * __P) {
  struct __loadu_i16 {
    unsigned short __v;
  } __attribute__((__packed__, __may_alias__));
  return (short)__builtin_bswap16(((const struct __loadu_i16*)__P)->__v);
}

/// Swap the bytes of a 16-bit value and store it to memory.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the MOVBE instruction.
///
/// \param __P
///    A pointer to the memory for storing the swapped value.
/// \param __D
///    The 16-bit value to be byte-swapped.
static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("movbe")))
_storebe_i16(void * __P, short __D) {
  struct __storeu_i16 {
    unsigned short __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __storeu_i16*)__P)->__v = __builtin_bswap16((unsigned short)__D);
}

/// Load a 32-bit value from memory and swap its bytes.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the MOVBE instruction.
///
/// \param __P
///    A pointer to the 32-bit value to load.
/// \returns The byte-swapped value.
static __inline__ int __attribute__((__always_inline__, __nodebug__, __target__("movbe")))
_loadbe_i32(void const * __P) {
  struct __loadu_i32 {
    unsigned int __v;
  } __attribute__((__packed__, __may_alias__));
  return (int)__builtin_bswap32(((const struct __loadu_i32*)__P)->__v);
}

/// Swap the bytes of a 32-bit value and store it to memory.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the MOVBE instruction.
///
/// \param __P
///    A pointer to the memory for storing the swapped value.
/// \param __D
///    The 32-bit value to be byte-swapped.
static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("movbe")))
_storebe_i32(void * __P, int __D) {
  struct __storeu_i32 {
    unsigned int __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __storeu_i32*)__P)->__v = __builtin_bswap32((unsigned int)__D);
}

#ifdef __x86_64__
/// Load a 64-bit value from memory and swap its bytes.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the MOVBE instruction.
///
/// \param __P
///    A pointer to the 64-bit value to load.
/// \returns The byte-swapped value.
static __inline__ long long __attribute__((__always_inline__, __nodebug__, __target__("movbe")))
_loadbe_i64(void const * __P) {
  struct __loadu_i64 {
    unsigned long long __v;
  } __attribute__((__packed__, __may_alias__));
  return (long long)__builtin_bswap64(((const struct __loadu_i64*)__P)->__v);
}

/// Swap the bytes of a 64-bit value and store it to memory.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the MOVBE instruction.
///
/// \param __P
///    A pointer to the memory for storing the swapped value.
/// \param __D
///    The 64-bit value to be byte-swapped.
static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("movbe")))
_storebe_i64(void * __P, long long __D) {
  struct __storeu_i64 {
    unsigned long long __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __storeu_i64*)__P)->__v = __builtin_bswap64((unsigned long long)__D);
}
#endif

#include <rtmintrin.h>
#include <xtestintrin.h>

#include <shaintrin.h>

#include <fxsrintrin.h>

/* No feature check desired due to internal MSC_VER checks */
#include <xsaveintrin.h>

#include <xsaveoptintrin.h>

#include <xsavecintrin.h>

#include <xsavesintrin.h>

#include <cetintrin.h>

/* Intrinsics inside adcintrin.h are available at all times. */
#include <adcintrin.h>

#include <adxintrin.h>

#include <rdseedintrin.h>

#include <wbnoinvdintrin.h>

#include <cldemoteintrin.h>

#include <waitpkgintrin.h>

#include <movdirintrin.h>

#include <movrsintrin.h>

#include <movrs_avx10_2intrin.h>

#include <movrs_avx10_2_512intrin.h>

#include <pconfigintrin.h>

#include <sgxintrin.h>

#include <ptwriteintrin.h>

#include <invpcidintrin.h>



















#include <keylockerintrin.h>

#include <amxintrin.h>

#include <amxfp16intrin.h>

#include <amxcomplexintrin.h>

#include <amxfp8intrin.h>

#include <amxtransposeintrin.h>

#include <amxmovrsintrin.h>

#include <amxmovrstransposeintrin.h>

#include <amxavx512intrin.h>

#include <amxtf32intrin.h>

#include <amxtf32transposeintrin.h>

#include <amxbf16transposeintrin.h>

#include <amxfp16transposeintrin.h>

#include <amxcomplextransposeintrin.h>

#include <avx512vp2intersectintrin.h>

#include <avx512vlvp2intersectintrin.h>




#include <avx10_2bf16intrin.h>
#include <avx10_2convertintrin.h>
#include <avx10_2copyintrin.h>
#include <avx10_2minmaxintrin.h>
#include <avx10_2niintrin.h>
#include <avx10_2satcvtdsintrin.h>
#include <avx10_2satcvtintrin.h>

#include <avx10_2_512bf16intrin.h>
#include <avx10_2_512convertintrin.h>
#include <avx10_2_512minmaxintrin.h>
#include <avx10_2_512niintrin.h>
#include <avx10_2_512satcvtdsintrin.h>
#include <avx10_2_512satcvtintrin.h>

#include <sm4evexintrin.h>

#include <enqcmdintrin.h>

#include <serializeintrin.h>

#include <tsxldtrkintrin.h>

#if defined(_MSC_VER) && __has_extension(gnu_asm)
/* Define the default attributes for these intrinsics */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__))
#ifdef __cplusplus
extern "C" {
#endif
/*----------------------------------------------------------------------------*\
|* Interlocked Exchange HLE
\*----------------------------------------------------------------------------*/
#if defined(__i386__) || defined(__x86_64__)
static __inline__ long __DEFAULT_FN_ATTRS
_InterlockedExchange_HLEAcquire(long volatile *_Target, long _Value) {
  __asm__ __volatile__(".byte 0xf2 ; lock ; xchg {%0, %1|%1, %0}"
                       : "+r" (_Value), "+m" (*_Target) :: "memory");
  return _Value;
}
static __inline__ long __DEFAULT_FN_ATTRS
_InterlockedExchange_HLERelease(long volatile *_Target, long _Value) {
  __asm__ __volatile__(".byte 0xf3 ; lock ; xchg {%0, %1|%1, %0}"
                       : "+r" (_Value), "+m" (*_Target) :: "memory");
  return _Value;
}
#endif
#if defined(__x86_64__)
static __inline__ __int64 __DEFAULT_FN_ATTRS
_InterlockedExchange64_HLEAcquire(__int64 volatile *_Target, __int64 _Value) {
  __asm__ __volatile__(".byte 0xf2 ; lock ; xchg {%0, %1|%1, %0}"
                       : "+r" (_Value), "+m" (*_Target) :: "memory");
  return _Value;
}
static __inline__ __int64 __DEFAULT_FN_ATTRS
_InterlockedExchange64_HLERelease(__int64 volatile *_Target, __int64 _Value) {
  __asm__ __volatile__(".byte 0xf3 ; lock ; xchg {%0, %1|%1, %0}"
                       : "+r" (_Value), "+m" (*_Target) :: "memory");
  return _Value;
}
#endif
/*----------------------------------------------------------------------------*\
|* Interlocked Compare Exchange HLE
\*----------------------------------------------------------------------------*/
#if defined(__i386__) || defined(__x86_64__)
static __inline__ long __DEFAULT_FN_ATTRS
_InterlockedCompareExchange_HLEAcquire(long volatile *_Destination,
                              long _Exchange, long _Comparand) {
  __asm__ __volatile__(".byte 0xf2 ; lock ; cmpxchg {%2, %1|%1, %2}"
                       : "+a" (_Comparand), "+m" (*_Destination)
                       : "r" (_Exchange) : "memory");
  return _Comparand;
}
static __inline__ long __DEFAULT_FN_ATTRS
_InterlockedCompareExchange_HLERelease(long volatile *_Destination,
                              long _Exchange, long _Comparand) {
  __asm__ __volatile__(".byte 0xf3 ; lock ; cmpxchg {%2, %1|%1, %2}"
                       : "+a" (_Comparand), "+m" (*_Destination)
                       : "r" (_Exchange) : "memory");
  return _Comparand;
}
#endif
#if defined(__x86_64__)
static __inline__ __int64 __DEFAULT_FN_ATTRS
_InterlockedCompareExchange64_HLEAcquire(__int64 volatile *_Destination,
                              __int64 _Exchange, __int64 _Comparand) {
  __asm__ __volatile__(".byte 0xf2 ; lock ; cmpxchg {%2, %1|%1, %2}"
                       : "+a" (_Comparand), "+m" (*_Destination)
                       : "r" (_Exchange) : "memory");
  return _Comparand;
}
static __inline__ __int64 __DEFAULT_FN_ATTRS
_InterlockedCompareExchange64_HLERelease(__int64 volatile *_Destination,
                              __int64 _Exchange, __int64 _Comparand) {
  __asm__ __volatile__(".byte 0xf3 ; lock ; cmpxchg {%2, %1|%1, %2}"
                       : "+a" (_Comparand), "+m" (*_Destination)
                       : "r" (_Exchange) : "memory");
  return _Comparand;
}
#endif
#ifdef __cplusplus
}
#endif

#undef __DEFAULT_FN_ATTRS

#endif /* defined(_MSC_VER) && __has_extension(gnu_asm) */


#include <svmlintrin.h>

#if __has_extension(gnu_asm)

#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__))

static __inline__ void __DEFAULT_FN_ATTRS
_clac(void) {
  __asm__ __volatile__ ("clac" : : : "memory");
}

static __inline__ void __DEFAULT_FN_ATTRS
_stac(void) {
  __asm__ __volatile__ ("stac" : : : "memory");
}

static __inline__ void __DEFAULT_FN_ATTRS
_lgdt(void *__ptr) {
  __asm__ __volatile__("lgdt %0" : : "m"(*(short *)(__ptr)) : "memory");
}

static __inline__ void __DEFAULT_FN_ATTRS
_sgdt(void *__ptr) {
  __asm__ __volatile__("sgdt %0" : "=m"(*(short *)(__ptr)) : : "memory");
}

#endif /* __has_extension(gnu_asm) */

#if !defined(_MSC_VER) && !defined(__cpuid) && __has_extension(gnu_asm)
#ifdef __cplusplus
extern "C" {
#endif
void __cpuid(int[4], int);
#ifdef __cplusplus
}
#endif

static __inline__ unsigned long long __DEFAULT_FN_ATTRS
__readmsr(unsigned int __register) {
  // Loads the contents of a 64-bit model specific register (MSR) specified in
  // the ECX register into registers EDX:EAX. The EDX register is loaded with
  // the high-order 32 bits of the MSR and the EAX register is loaded with the
  // low-order 32 bits. If less than 64 bits are implemented in the MSR being
  // read, the values returned to EDX:EAX in unimplemented bit locations are
  // undefined.
  unsigned int __edx;
  unsigned int __eax;
  __asm__ ("rdmsr" : "=d"(__edx), "=a"(__eax) : "c"(__register));
  return (((unsigned long long)__edx) << 32) | (unsigned long long)__eax;
}

static __inline__ void __DEFAULT_FN_ATTRS
__writemsr(unsigned int __register, unsigned long long __data) {
  __asm__ ("wrmsr" : : "d"((unsigned)(__data >> 32)), "a"((unsigned)__data), "c"(__register));
}

#define _readmsr(R) __readmsr(R)
#define _writemsr(R, D) __writemsr(R, D)

#endif /* !defined(_MSC_VER) && __has_extension(gnu_asm) */

#undef __DEFAULT_FN_ATTRS

// clang-format off

/* Definitions of feature list to be used by feature select intrinsics */
#define _FEATURE_GENERIC_IA32        (1ULL     )
#define _FEATURE_FPU                 (1ULL << 1)
#define _FEATURE_CMOV                (1ULL << 2)
#define _FEATURE_MMX                 (1ULL << 3)
#define _FEATURE_FXSAVE              (1ULL << 4)
#define _FEATURE_SSE                 (1ULL << 5)
#define _FEATURE_SSE2                (1ULL << 6)
#define _FEATURE_SSE3                (1ULL << 7)
#define _FEATURE_SSSE3               (1ULL << 8)
#define _FEATURE_SSE4_1              (1ULL << 9)
#define _FEATURE_SSE4_2              (1ULL << 10)
#define _FEATURE_MOVBE               (1ULL << 11)
#define _FEATURE_POPCNT              (1ULL << 12)
#define _FEATURE_PCLMULQDQ           (1ULL << 13)
#define _FEATURE_AES                 (1ULL << 14)
#define _FEATURE_F16C                (1ULL << 15)
#define _FEATURE_AVX                 (1ULL << 16)
#define _FEATURE_RDRND               (1ULL << 17)
#define _FEATURE_FMA                 (1ULL << 18)
#define _FEATURE_BMI                 (1ULL << 19)
#define _FEATURE_LZCNT               (1ULL << 20)
#define _FEATURE_HLE                 (1ULL << 21)
#define _FEATURE_RTM                 (1ULL << 22)
#define _FEATURE_AVX2                (1ULL << 23)
#define _FEATURE_AVX512DQ            (1ULL << 24)
#define _FEATURE_PTWRITE             (1ULL << 25)
#define _FEATURE_AVX512F             (1ULL << 27)
#define _FEATURE_ADX                 (1ULL << 28)
#define _FEATURE_RDSEED              (1ULL << 29)
#define _FEATURE_AVX512IFMA52        (1ULL << 30)
#define _FEATURE_AVX512ER            (1ULL << 32)
#define _FEATURE_AVX512PF            (1ULL << 33)
#define _FEATURE_AVX512CD            (1ULL << 34)
#define _FEATURE_SHA                 (1ULL << 35)
#define _FEATURE_MPX                 (1ULL << 36)
#define _FEATURE_AVX512BW            (1ULL << 37)
#define _FEATURE_AVX512VL            (1ULL << 38)
#define _FEATURE_AVX512VBMI          (1ULL << 39)
#define _FEATURE_AVX512_4FMAPS       (1ULL << 40)
#define _FEATURE_AVX512_4VNNIW       (1ULL << 41)
#define _FEATURE_AVX512_VPOPCNTDQ    (1ULL << 42)
#define _FEATURE_AVX512_BITALG       (1ULL << 43)
#define _FEATURE_AVX512_VBMI2        (1ULL << 44)
#define _FEATURE_GFNI                (1ULL << 45)
#define _FEATURE_VAES                (1ULL << 46)
#define _FEATURE_VPCLMULQDQ          (1ULL << 47)
#define _FEATURE_AVX512_VNNI         (1ULL << 48)
#define _FEATURE_CLWB                (1ULL << 49)
#define _FEATURE_RDPID               (1ULL << 50)
#define _FEATURE_IBT                 (1ULL << 51)
#define _FEATURE_SHSTK               (1ULL << 52)
#define _FEATURE_SGX                 (1ULL << 53)
#define _FEATURE_WBNOINVD            (1ULL << 54)
#define _FEATURE_PCONFIG             (1ULL << 55)
#define _FEATURE_AXV512_VP2INTERSECT (1ULL << 56)
#define _FEATURE_AXV512_FP16         (1ULL << 60)

/* NOTE: Features with bit_pos >= 64 are defined in Page2       */
/* Example of how allow_cpu_features uses those features:       */
/*    __attribute__((allow_cpu_features(0, _FEATURE_CLDEMOTE))) */
#define _FEATURE_CLDEMOTE            (1ULL     )
#define _FEATURE_MOVDIRI             (1ULL << 1)
#define _FEATURE_MOVDIR64B           (1ULL << 2)
#define _FEATURE_WAITPKG             (1ULL << 3)
#define _FEATURE_AVX512_Bf16         (1ULL << 4)
#define _FEATURE_ENQCMD              (1ULL << 5)
#define _FEATURE_AVX_VNNI            (1ULL << 6)
#define _FEATURE_AMX_TILE            (1ULL << 7)
#define _FEATURE_AMX_INT8            (1ULL << 8)
#define _FEATURE_AMX_BF16            (1ULL << 9)
#define _FEATURE_KL                  (1ULL << 10)
#define _FEATURE_WIDE_KL             (1ULL << 11)
#define _FEATURE_HRESET              (1ULL << 12)
#define _FEATURE_UINTR               (1ULL << 13)
#define _FEATURE_PREFETCHI           (1ULL << 14)
#define _FEATURE_AVXVNNIINT8         (1ULL << 15)
#define _FEATURE_CMPCCXADD           (1ULL << 16)
#define _FEATURE_AVXIFMA             (1ULL << 17)
#define _FEATURE_AVXNECONVERT        (1ULL << 18)
#define _FEATURE_RAOINT              (1ULL << 19)
#define _FEATURE_AMX_FP16            (1ULL << 20)
#define _FEATURE_AMX_COMPLEX         (1ULL << 21)
#define _FEATURE_SHA512              (1ULL << 22)
#define _FEATURE_SM3                 (1ULL << 23)
#define _FEATURE_SM4                 (1ULL << 24)
#define _FEATURE_AVXVNNIINT16        (1ULL << 25)
#define _FEATURE_USERMSR             (1ULL << 26)
#define _FEATURE_AVX10_1             (1ULL << 27)
#define _FEATURE_AVX10_2             (1ULL << 28)
#define _FEATURE_APXF                (1ULL << 29)
#define _FEATURE_MSRLIST             (1ULL << 30)
#define _FEATURE_WRMSRNS             (1ULL << 31)
#define _FEATURE_PBNDKB              (1ULL << 32)
#define _FEATURE_AMX_FP8             (1ULL << 33)
#define _FEATURE_AMX_TF32            (1ULL << 34)
#define _FEATURE_AMX_TRANSPOSE       (1ULL << 35)
#define _FEATURE_AMX_MOVRS           (1ULL << 36)
#define _FEATURE_AMX_512             (1ULL << 37)
#define _FEATURE_MOVRS               (1ULL << 38)
#define _FEATURE_INVPCID             (1ULL << 39)
#define _FEATURE_PKU                 (1ULL << 40)
#define _FEATURE_PREFETCHW           (1ULL << 41)
#define _FEATURE_CLFLUSHOPT          (1ULL << 42)
#define _FEATURE_LAHFSAH64           (1ULL << 43)
#define _FEATURE_SERIALIZE           (1ULL << 44)
#define _FEATURE_TSXLDTRK            (1ULL << 45)
#define _FEATURE_FSRM                (1ULL << 46)
#define _FEATURE_ERMSB               (1ULL << 46)
#define _FEATURE_MWAIT               (1ULL << 48)
#define _FEATURE_MONITORLESS_MWAIT   (1ULL << 49)

// clang-format on

#pragma float_control(pop)

#endif /* __IMMINTRIN_H */
