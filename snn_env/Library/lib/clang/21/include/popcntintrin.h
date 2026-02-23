/*===---- popcntintrin.h - POPCNT intrinsics -------------------------------=== */
/*
 * Modifications, Copyright (C) 2023 Intel Corporation
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

#ifndef __POPCNTINTRIN_H
#define __POPCNTINTRIN_H

/* Define the default attributes for the functions in this file. */
#if defined(__cplusplus) && (__cplusplus >= 201103L)
#define __DEFAULT_FN_ATTRS                                                     \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("popcnt"))) constexpr
#else
#define __DEFAULT_FN_ATTRS                                                     \
  __attribute__((__always_inline__, __nodebug__, __target__("popcnt")))
#endif

/// Counts the number of bits in the source operand having a value of 1.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the <c> POPCNT </c> instruction.
///
/// \param __A
///    An unsigned 32-bit integer operand.
/// \returns A 32-bit integer containing the number of bits with value 1 in the
///    source operand.
static __inline__ int __DEFAULT_FN_ATTRS
_mm_popcnt_u32(unsigned int __A)
{
  return __builtin_popcount(__A);
}

/* #ifdef __x86_64__ */
/// Counts the number of bits in the source operand having a value of 1.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the <c> POPCNT </c> instruction.
///
/// \param __A
///    An unsigned 64-bit integer operand.
/// \returns A 64-bit integer containing the number of bits with value 1 in the
///    source operand.
static __inline__ long long __DEFAULT_FN_ATTRS
_mm_popcnt_u64(unsigned long long __A)
{
  return __builtin_popcountll(__A);
}
/* #endif */ /* __x86_64__ */

#define _mm_countbits_32 (unsigned int) _mm_popcnt_u32
#define _mm_countbits_64 (unsigned long long) _mm_popcnt_u64

#undef __DEFAULT_FN_ATTRS

#endif /* __POPCNTINTRIN_H */
