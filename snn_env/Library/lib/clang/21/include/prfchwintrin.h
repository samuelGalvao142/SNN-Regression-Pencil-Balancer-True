/*===---- prfchwintrin.h - PREFETCHW intrinsic -----------------------------=== */
/*
 * Modifications, Copyright (C) 2022 Intel Corporation
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

#if !defined(__X86INTRIN_H) && !defined(_MM3DNOW_H_INCLUDED)
#error "Never use <prfchwintrin.h> directly; include <x86intrin.h> instead."
#endif

#ifndef __PRFCHWINTRIN_H
#define __PRFCHWINTRIN_H

#if defined(__cplusplus)
extern "C" {
#endif

/// Loads a memory sequence containing the specified memory address into
///    all data cache levels.
///
///    The cache-coherency state is set to exclusive. Data can be read from
///    and written to the cache line without additional delay.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the \c PREFETCHT0 instruction.
///
/// \param __P
///    A pointer specifying the memory address to be prefetched.
void _m_prefetch(void *__P);

/// Loads a memory sequence containing the specified memory address into
///    the L1 data cache and sets the cache-coherency state to modified.
///
///    This provides a hint to the processor that the cache line will be
///    modified. It is intended for use when the cache line will be written to
///    shortly after the prefetch is performed.
///
///    Note that the effect of this intrinsic is dependent on the processor
///    implementation.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the \c PREFETCHW instruction.
///
/// \param __P
///    A pointer specifying the memory address to be prefetched.
void _m_prefetchw(volatile const void *__P);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif /* __PRFCHWINTRIN_H */
