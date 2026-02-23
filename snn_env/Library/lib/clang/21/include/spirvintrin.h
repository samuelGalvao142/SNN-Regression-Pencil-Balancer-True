//===-- spirvintrin.h - SPIRV intrinsic functions -------------------------===//
//
// Modifications, Copyright (C) 2025 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef __SPIRVINTRIN_H
#define __SPIRVINTRIN_H

#ifndef __SPIRV__
#error "This file is intended for SPIRV targets or offloading to SPIRV"
#endif

#ifndef __GPUINTRIN_H
#error "Never use <amdgpuintrin.h> directly; include <gpuintrin.h> instead"
#endif

#include <stdint.h>

#if !defined(__cplusplus)
_Pragma("push_macro(\"bool\")");
#define bool _Bool
#endif

_Pragma("omp begin declare target device_type(nohost)");
_Pragma("omp begin declare variant match(device = {arch(spirv64)})");

// Type aliases to the address spaces used by the SPIR-V backend.
//
// TODO: FIX
#define __gpu_private  __attribute__((address_space(0)))
#define __gpu_constant
#define __gpu_local
#define __gpu_global __attribute__((address_space(1)))
#define __gpu_generic

// Attribute to declare a function as a kernel.
#define __gpu_kernel __attribute__((spir_kernel, visibility("protected")))

#define __SPIRV_VAR_QUALIFIERS extern const
extern uint64_t __attribute__((overloadable)) __spirv_BuiltInLocalInvocationId(int dimindx);
extern uint64_t __attribute__((overloadable)) __spirv_BuiltInNumWorkgroups(int dimindx);
extern uint64_t __attribute__((overloadable)) __spirv_BuiltInWorkgroupId(int dimindx);
extern uint64_t __attribute__((overloadable)) __spirv_BuiltInWorkgroupSize(int dimindx);
extern int  __attribute__((overloadable)) __spirv_SubgroupShuffleINTEL(int, unsigned int);
__SPIRV_VAR_QUALIFIERS uint32_t __spirv_BuiltInSubgroupLocalInvocationId;
__SPIRV_VAR_QUALIFIERS uint32_t __spirv_BuiltInSubgroupSize;

// Returns the number of blocks in the 'x' dimension.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_num_blocks_x(void) {
   return __spirv_BuiltInNumWorkgroups(0);
}

// Returns the number of blocks in the 'y' dimension.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_num_blocks_y(void) {
   return __spirv_BuiltInNumWorkgroups(1);
}

// Returns the number of blocks in the 'z' dimension.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_num_blocks_z(void) {
   return __spirv_BuiltInNumWorkgroups(2);
}

// Returns the 'x' dimension of the current block's id.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_block_id_x(void) {
  return __spirv_BuiltInWorkgroupId(0);
}

// Returns the 'y' dimension of the current block's id.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_block_id_y(void) {
  return __spirv_BuiltInWorkgroupId(1);
}

// Returns the 'z' dimension of the current block's id.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_block_id_z(void) {
  return __spirv_BuiltInWorkgroupId(2);
}

// Returns the number of threads in the 'x' dimension.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_num_threads_x(void) {
  return __spirv_BuiltInWorkgroupSize(0);
}

// Returns the number of threads in the 'y' dimension.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_num_threads_y(void) {
  return __spirv_BuiltInWorkgroupSize(1);
}

// Returns the number of threads in the 'z' dimension.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_num_threads_z(void) {
  return __spirv_BuiltInWorkgroupSize(2);
}

// Returns the 'x' dimension id of the thread in the current block.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_thread_id_x(void) {
  return __spirv_BuiltInLocalInvocationId(0);
}

// Returns the 'y' dimension id of the thread in the current block.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_thread_id_y(void) {
  return __spirv_BuiltInLocalInvocationId(1);
}

// Returns the 'z' dimension id of the thread in the current block.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_thread_id_z(void) {
  return __spirv_BuiltInLocalInvocationId(2);
}

// Returns the size of a warp, always 32 on NVIDIA hardware.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_num_lanes(void) { return __spirv_BuiltInSubgroupSize; }

// Returns the id of the thread inside of a warp executing together.
_DEFAULT_FN_ATTRS static __inline__ uint32_t __gpu_lane_id(void) { return __spirv_BuiltInSubgroupLocalInvocationId; }

// Returns the bit-mask of active threads in the current warp.
_DEFAULT_FN_ATTRS static __inline__ uint64_t __gpu_lane_mask(void) { 
  uint32_t Size = __gpu_num_lanes();
  return ((uint64_t)1 << Size) - (uint64_t)1;
}

// Copies the value from the first active thread in the warp to the rest.
_DEFAULT_FN_ATTRS static __inline__ uint32_t
__gpu_read_first_lane_u32(uint64_t __lane_mask, uint32_t __x) {
  return 0;
}

// Returns a bitmask of threads in the current lane for which \p x is true.
_DEFAULT_FN_ATTRS static __inline__ uint64_t __gpu_ballot(uint64_t __lane_mask,
                                                          bool __x) {
  // TODO Implement it for Intel GPUs
  return __lane_mask;
}

// Waits for all the threads in the block to converge and issues a fence.
_DEFAULT_FN_ATTRS static __inline__ void __gpu_sync_threads(void) {
  //__syncthreads();
}

// Waits for all threads in the warp to reconverge for independent scheduling.
_DEFAULT_FN_ATTRS static __inline__ void __gpu_sync_lane(uint64_t __lane_mask) {
  //__nvvm_bar_warp_sync((uint32_t)__lane_mask);
}

// Shuffles the the lanes inside the warp according to the given index.
_DEFAULT_FN_ATTRS static __inline__ uint32_t
__gpu_shuffle_idx_u32(uint64_t __lane_mask, uint32_t __idx, uint32_t __x,
                      uint32_t __width) {
  int Self = __gpu_lane_id();
  int Index = __idx + (Self & ~(__width - 1));
  return __spirv_SubgroupShuffleINTEL(__x, Index << 2);
}

// Returns true if the flat pointer points to 'shared' memory.
_DEFAULT_FN_ATTRS static __inline__ bool __gpu_is_ptr_local(void *ptr) {
  // TODO Need to implement this function for Intel GPU
  return false;
}

// Returns true if the flat pointer points to 'local' memory.
_DEFAULT_FN_ATTRS static __inline__ bool __gpu_is_ptr_private(void *ptr) {
  return 0;
}

// Terminates execution of the calling thread.
_DEFAULT_FN_ATTRS [[noreturn]] static __inline__ void __gpu_exit(void) {
  //__nvvm_exit();
}

// Suspend the thread briefly to assist the scheduler during busy loops.
_DEFAULT_FN_ATTRS static __inline__ void __gpu_thread_suspend(void) {}

_Pragma("omp end declare variant");
_Pragma("omp end declare target");

#if !defined(__cplusplus)
_Pragma("pop_macro(\"bool\")");
#endif

#endif // __SPIRVINTRIN_H
