/*
   Copyright (C) 1985 Intel Corporation

   This software and the related documents are Intel copyrighted materials, and
   your use of them is governed by the express license under which they were
   provided to you ("License"). Unless the License provides otherwise, you may
   not use, modify, copy, publish, distribute, disclose or transmit this
   software or the related documents without Intel's prior written permission.

   This software and the related documents are provided as is, with no express
   or implied warranties, other than those that are expressly stated in the
   License.
*/

#if defined(__cplusplus)
    #define _LIBIMF_EXTERN_C extern "C"
#else
    #define _LIBIMF_EXTERN_C extern
#endif

#if (defined(_WIN32) || defined(_WIN64))
    #if defined(__cplusplus)
        #if defined (__INTEL_LLVM_COMPILER)
            #define _LIBIMF_FORCEINLINE extern "C" __forceinline
        #else
            #define _LIBIMF_FORCEINLINE extern "C" __forceinline
        #endif
    #else
        #if defined (__INTEL_LLVM_COMPILER)
            #define _LIBIMF_FORCEINLINE __forceinline
        #else
            #define _LIBIMF_FORCEINLINE __forceinline
        #endif
    #endif
#else
    #if defined(__cplusplus)
        #if defined (__INTEL_LLVM_COMPILER)
            #define _LIBIMF_FORCEINLINE extern "C" inline __attribute__((always_inline)) __attribute__((weak))
        #else
            #define _LIBIMF_FORCEINLINE extern "C" inline __attribute__((always_inline))
        #endif
    #else
        #if defined (__INTEL_LLVM_COMPILER)
            #define _LIBIMF_FORCEINLINE inline __attribute__((always_inline)) __attribute__((weak))
        #else
            #define _LIBIMF_FORCEINLINE inline __attribute__((always_inline))
        #endif
    #endif
#endif

#ifndef __IMFLONGDOUBLE
    #if defined(__LONG_DOUBLE_SIZE__) /* Compiler-predefined macros. If defined, should be 128|80|64 */
        #define __IMFLONGDOUBLE (__LONG_DOUBLE_SIZE__)
    #else
        #define __IMFLONGDOUBLE 64
    #endif
#endif

#if defined(__cplusplus) && !(defined(__FreeBSD__) || defined(__ANDROID__))
    #define _LIBIMF_CPP_EXCEPT_SPEC() throw()
#else
    #define _LIBIMF_CPP_EXCEPT_SPEC()
#endif

#if defined(_DLL) && (defined(_WIN32) || defined(_WIN64)) /* Windows DLL */
    #define _LIBIMF_PUBAPI __declspec(dllimport) __cdecl
    #define _LIBIMF_PUBAPI_INL __cdecl
    #define _LIBIMF_PUBVAR __declspec(dllimport)
#elif defined(__unix__) || defined(__APPLE__) || defined(__QNX__) || defined(__VXWORKS__) /* Linux, MacOS or QNX */
    #define _LIBIMF_PUBAPI /* do not change this line! */
    #define _LIBIMF_PUBAPI_INL
    #define _LIBIMF_PUBVAR
#else /* Windows static */
    #define _LIBIMF_PUBAPI __cdecl
    #define _LIBIMF_PUBAPI_INL __cdecl
    #define _LIBIMF_PUBVAR
#endif

#if defined (__APPLE__)
    #define _LIBIMF_DBL_XDBL    long double
#else
    #define _LIBIMF_DBL_XDBL    double
#endif
