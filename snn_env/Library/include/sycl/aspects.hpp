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
//==--------------- aspects.hpp - SYCL Aspect Enums ------------*- C++ -*---==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ===--------------------------------------------------------------------=== //
#pragma once

#include <sycl/detail/defines.hpp>            // for __SYCL_TYPE
#include <sycl/detail/defines_elementary.hpp> // for __SYCL2020_DEPRECATED

namespace sycl {
inline namespace _V1 {

#define __SYCL_ASPECT(ASPECT, ID) ASPECT = ID,
#define __SYCL_ASPECT_DEPRECATED(ASPECT, ID, MESSAGE)                          \
  ASPECT __SYCL2020_DEPRECATED(MESSAGE) = ID,
#define __SYCL_ASPECT_DEPRECATED_ALIAS(ASPECT, ID, MESSAGE)                    \
  __SYCL_ASPECT_DEPRECATED(ASPECT, ID, MESSAGE)
enum class __SYCL_TYPE(aspect) aspect {
#include <sycl/info/aspects.def>
#include <sycl/info/aspects_deprecated.def>
};
#undef __SYCL_ASPECT_DEPRECATED_ALIAS
#undef __SYCL_ASPECT_DEPRECATED
#undef __SYCL_ASPECT

} // namespace _V1
} // namespace sycl
