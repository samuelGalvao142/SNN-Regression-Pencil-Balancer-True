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

#if (defined(_WIN32) || defined(_WIN64))
    /* Include_next should be before guard macros _FLOAT_H__ in order to at last reach system header */
    #if defined(__INTEL_COMPILER) || defined( __INTEL_LLVM_COMPILER)
        #include_next <float.h> /* utilize system header */
    #else
        #if _MSC_VER >= 1900  /* MSVS2015+ */
            /* Location of float.h has been changed starting with VS2015 */
            #include <../ucrt/float.h>
        #elif _MSC_VER >= 1400 /* Previous versions of MSVS are not supported. */
            /*
               Here, the #include <../../vc/include/header.h> is used as the
               equivalent of #include_next<header.h> working for MS C compiler
               of MSVS 2005, 2008, 2010 with default installation paths. The
               equivalent works correctly when Intel(R) compiler header is not
               located in ../../vc/include subfolder for any searched include
               path, and MS header is.
               In case of non standard location of MS headers, say in
               C:/PROGRA~1/MSVS/NEW_VC/INCLUDE folder, proper
               __MS_VC_INSTALL_PATH macro should be defined in command line -D
               option like -D__MS_VC_INSTALL_PATH=C:/PROGRA~1/MSVS/NEW_VC.
            */
            #ifndef __MS_VC_INSTALL_PATH
            #define __MS_VC_INSTALL_PATH    ../../vc
            #endif

            #define __TMP_GLUE(a,b)         a##b
            #define __TMP_PASTE2(a,b)       __TMP_GLUE(a,b)
            #define __TMP_ANGLE_BRACKETS(x) <x>

            #include __TMP_ANGLE_BRACKETS(__TMP_PASTE2(__MS_VC_INSTALL_PATH,/include/float.h))

            #undef __TMP_GLUE
            #undef __TMP_PASTE2
            #undef __TMP_ANGLE_BRACKETS
        #endif
    #endif
#endif

/* float.h standard header -- IEEE 754 version */
#if defined(__INTEL_COMPILER) || defined( __INTEL_LLVM_COMPILER)
#ifndef _FLOAT_H__
#define _FLOAT_H__

#ifdef  __cplusplus
extern "C" {
#endif

#if (defined(_WIN32) || defined(_WIN64)) && defined(_DLL) /* Windows DLL */
    extern int __declspec(dllimport) __cdecl __libm_flt_rounds (void) ;
#elif (defined(_WIN32) || defined(_WIN64)) /* Windows static */
    extern int __cdecl __libm_flt_rounds (void) ;
#else
    extern int __libm_flt_rounds (void) ;
#endif

#ifdef  __cplusplus
}
#endif

/* common properties */
#undef  FLT_RADIX
#define FLT_RADIX   2

#undef  FLT_ROUNDS
#if defined(__ANDROID__) && (__STDC_HOSTED__ == 0)
    #define FLT_ROUNDS 1
#else
    #define FLT_ROUNDS  (__libm_flt_rounds())
#endif

/* float properties */
#undef FLT_EPSILON
#undef FLT_MAX
#undef FLT_MIN
#undef FLT_DIG
#undef FLT_MANT_DIG
#undef FLT_MAX_10_EXP
#undef FLT_MAX_EXP
#undef FLT_MIN_10_EXP
#undef FLT_MIN_EXP

#define FLT_EPSILON         ((float) 1.1920928955078125e-7)
#if (defined(_WIN32) || defined(_WIN64))
#define FLT_MAX             ((float) 340282346638528859811704183484516925440.0)
#else
#define FLT_MAX             ((float) 340282346638528859811704183484516925440.0)
#endif
#define FLT_MIN             ((float) 1.1754943508222875079687365372222456778186655567720875215087517062784172594547271728515625e-38)
#define FLT_DIG             6
#define FLT_MANT_DIG        24
#define FLT_MAX_10_EXP      38
#define FLT_MAX_EXP         128
#define FLT_MIN_10_EXP      (-37)
#define FLT_MIN_EXP         (-125)

/* double properties */
#undef DBL_EPSILON
#undef DBL_MAX
#undef DBL_MIN
#undef DBL_DIG
#undef DBL_MANT_DIG
#undef DBL_MAX_10_EXP
#undef DBL_MAX_EXP
#undef DBL_MIN_10_EXP
#undef DBL_MIN_EXP

#define DBL_EPSILON         ((double) 2.220446049250313080847263336181640625e-16)
#define DBL_MAX             ((double) 1.7976931348623157e+308)
#define DBL_MIN             ((double) 2.2250738585072014e-308)
#define DBL_DIG             15
#define DBL_MANT_DIG        53
#define DBL_MAX_10_EXP      308
#define DBL_MAX_EXP         1024
#define DBL_MIN_10_EXP      (-307)
#define DBL_MIN_EXP         (-1021)

/* long double properties */
#undef LDBL_EPSILON
#undef LDBL_MAX
#undef LDBL_MIN
#undef LDBL_DIG
#undef LDBL_MANT_DIG
#undef LDBL_MAX_10_EXP
#undef LDBL_MAX_EXP
#undef LDBL_MIN_10_EXP
#undef LDBL_MIN_EXP

#ifndef __IMFLONGDOUBLE
    #if defined(__LONG_DOUBLE_SIZE__) /* Compiler-predefined macros. If defined, should be 128|80|64 */
        #define __IMFLONGDOUBLE (__LONG_DOUBLE_SIZE__)
    #else
        #define __IMFLONGDOUBLE 64
    #endif
#endif

/* __LDBL_EPSILON__ macro and companions are defined automatically
 * by modern Intel(R), GNU* and Clang* compilers for non-Windows* targets */
#if defined(__LDBL_EPSILON__)
    #define LDBL_EPSILON         __LDBL_EPSILON__
    #define LDBL_MAX             __LDBL_MAX__
    #define LDBL_MIN             __LDBL_MIN__
    #define LDBL_DIG             __LDBL_DIG__
    #define LDBL_MANT_DIG        __LDBL_MANT_DIG__
    #define LDBL_MAX_10_EXP      __LDBL_MAX_10_EXP__
    #define LDBL_MAX_EXP         __LDBL_MAX_EXP__
    #define LDBL_MIN_10_EXP      __LDBL_MIN_10_EXP__
    #define LDBL_MIN_EXP         __LDBL_MIN_EXP__
#else
#if (__IMFLONGDOUBLE == 64)
    #define LDBL_EPSILON         ((long double) 2.220446049250313080847263336181640625e-16)
    #define LDBL_MAX             ((long double) 1.79769313486231570814527423731704356798070567525845e308)
    #define LDBL_MIN             ((long double) 2.2250738585072013830902327173324040642192159804623e-308)
    #define LDBL_DIG             15
    #define LDBL_MANT_DIG        53
    #define LDBL_MAX_10_EXP      308
    #define LDBL_MAX_EXP         1024
    #define LDBL_MIN_10_EXP      (-307)
    #define LDBL_MIN_EXP         (-1021)
#else
    #define LDBL_EPSILON        ((long double) ((double) 1.08420217248550443400745280086994171142578125e-19))
    #define LDBL_MAX            (((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.0715086071862673e301)) * \
                                 ((long double) ((double) 1.067993517960455e96)) *  \
                                 ((long double) ((double) 36893488147419099136.0)) + \
                                  ((long double) ((double) 4094.0)))
    #define LDBL_MIN            (((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 9.332636185032188789900895447238171696170914463717e-302)) * \
                                 ((long double) ((double) 1.01517673492625968930960982233221741872944753988044e-115)))
    #define LDBL_DIG            18
    #define LDBL_MANT_DIG       64
    #define LDBL_MAX_10_EXP     4932
    #define LDBL_MAX_EXP        16384
    #define LDBL_MIN_10_EXP     (-4931)
    #define LDBL_MIN_EXP        (-16381)
#endif
#endif

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    /* Here if ISO/IEC 9899:1999 (ISO C99) or newer. */

    #undef DECIMAL_DIG
    #undef FLT_EVAL_METHOD

    #define DECIMAL_DIG 21
    #ifdef __FLT_EVAL_METHOD__
        #define FLT_EVAL_METHOD __FLT_EVAL_METHOD__
    #else
        #define FLT_EVAL_METHOD -1
    #endif

#endif  /* #if (__STDC_VERSION__ >= 199901L) */

#if (defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) || (defined (__cplusplus) && (__cplusplus >= 201703L))
    /* Here if ISO/IEC 9899:2011 (ISO C11) (or newer) OR ISO/IEC 14882:2017 (C++17) (or newer). */

    #undef  FLT_HAS_SUBNORM
    #undef  DBL_HAS_SUBNORM
    #undef LDBL_HAS_SUBNORM
    #undef  FLT_DECIMAL_DIG
    #undef  DBL_DECIMAL_DIG
    #undef LDBL_DECIMAL_DIG
    #undef  FLT_TRUE_MIN
    #undef  DBL_TRUE_MIN
    #undef LDBL_TRUE_MIN

    /* Binary format parameters. */

    /* P parameter of binary format FPbin(p), mantissa length, in bits: */
    #define  __LIBM_FP16_MANTISSA_BITS      11
    #define  __LIBM_FP32_MANTISSA_BITS      24
    #define  __LIBM_FP64_MANTISSA_BITS      53
    #define  __LIBM_FP80_MANTISSA_BITS      64
    #define  __LIBM_FP128_MANTISSA_BITS     113
    /* Emin parameter of binary format FPbin(p): */
    #define  __LIBM_FP16_EMIN               (-14)
    #define  __LIBM_FP32_EMIN               (-126)
    #define  __LIBM_FP64_EMIN               (-1022)
    #define  __LIBM_FP80_EMIN               (-16382)
    #define  __LIBM_FP128_EMIN              (-16382)
    /* Minimal positive subnormal of binary format FPbin(p), 2^(Emin-p+1): */
    #define  __LIBM_FP16_MIN_SUBNORMAL (((float) 0x1.0p-12) *                \
                                        ((float) 0x1.0p-12))
    #define  __LIBM_FP32_MIN_SUBNORMAL (((float) 0x1.0p-74) *                \
                                        ((float) 0x1.0p-75))
    #define  __LIBM_FP64_MIN_SUBNORMAL (((double) 0x1.0p-537) *              \
                                        ((double) 0x1.0p-537))
    #define  __LIBM_FP80_MIN_SUBNORMAL (((long double) 0x1.0p-445) *         \
                                        (((((((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500)) *   \
                                            (((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500))) *  \
                                           ((((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500)) *   \
                                            (((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500)))) * \
                                          (((((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500)) *   \
                                            (((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500))) *  \
                                           ((((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500)) *   \
                                            (((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500))))) *\
                                         ((((((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500)) *   \
                                            (((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500))) *  \
                                           ((((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500)) *   \
                                            (((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500)))) * \
                                          (((((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500)) *   \
                                            (((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500))) *  \
                                           ((((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500)) *   \
                                            (((long double) 0x1.0p-500) *    \
                                             ((long double) 0x1.0p-500)))))))
    #define  __LIBM_FP128_MIN_SUBNORMAL (((_Float128) 0x1.0p-494) *          \
                                         (((((((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500)) *    \
                                             (((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500))) *   \
                                            ((((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500)) *    \
                                             (((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500)))) *  \
                                           (((((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500)) *    \
                                             (((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500))) *   \
                                            ((((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500)) *    \
                                             (((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500))))) * \
                                          ((((((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500)) *    \
                                             (((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500))) *   \
                                            ((((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500)) *    \
                                             (((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500)))) *  \
                                           (((((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500)) *    \
                                             (((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500))) *   \
                                            ((((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500)) *    \
                                             (((_Float128) 0x1.0p-500) *     \
                                              ((_Float128) 0x1.0p-500)))))))
    /* Number of decimal digits d in decimal format FPdec(d), such that two   */
    /* conversions (in round to nearest mode) of initial number n from        */
    /* FPbin(p) format to FPdec(d) format and back to FPbin(p) format would   */
    /* recover initial number N:                                              */
    #define  __LIBM_FP16_RECOVERING_DECIMAL_DIGITS      5
    #define  __LIBM_FP32_RECOVERING_DECIMAL_DIGITS      9
    #define  __LIBM_FP64_RECOVERING_DECIMAL_DIGITS      17
    #define  __LIBM_FP80_RECOVERING_DECIMAL_DIGITS      21
    #define  __LIBM_FP128_RECOVERING_DECIMAL_DIGITS     36

    /* ISO C11, Paragraph 5.2.4.2.2, clause 10 */
    #define  FLT_HAS_SUBNORM    1
    #define  DBL_HAS_SUBNORM    1
    #define LDBL_HAS_SUBNORM    1

    /* ISO C11, Paragraph 5.2.4.2.2, Clause 11 */
    #define     FLT_DECIMAL_DIG   __LIBM_FP32_RECOVERING_DECIMAL_DIGITS
    #define     DBL_DECIMAL_DIG   __LIBM_FP64_RECOVERING_DECIMAL_DIGITS
    #if   (__IMFLONGDOUBLE==64)
        #define LDBL_DECIMAL_DIG  __LIBM_FP64_RECOVERING_DECIMAL_DIGITS
    #elif (__IMFLONGDOUBLE==80)
        #define LDBL_DECIMAL_DIG  __LIBM_FP80_RECOVERING_DECIMAL_DIGITS
    #elif (__IMFLONGDOUBLE==128)
        #define LDBL_DECIMAL_DIG  __LIBM_FP128_RECOVERING_DECIMAL_DIGITS
    #endif

    /* ISO C11, Paragraph 5.2.4.2.2, Clause 11 */
    #define     FLT_TRUE_MIN       __LIBM_FP32_MIN_SUBNORMAL
    #define     DBL_TRUE_MIN       __LIBM_FP64_MIN_SUBNORMAL
    #if   (__IMFLONGDOUBLE==64)
        #define LDBL_TRUE_MIN       __LIBM_FP64_MIN_SUBNORMAL
    #elif (__IMFLONGDOUBLE==80)
        #define LDBL_TRUE_MIN       __LIBM_FP80_MIN_SUBNORMAL
    #elif (__IMFLONGDOUBLE==128)
        #define LDBL_TRUE_MIN       __LIBM_FP128_MIN_SUBNORMAL
    #endif
#endif /* #if (__STDC_VERSION__ >= 201000L) */

#ifdef __STDC_WANT_DEC_FP__

#ifdef __DEC_EVAL_METHOD__
#define DEC_EVAL_METHOD __DEC_EVAL_METHOD__
#else
#define DEC_EVAL_METHOD -1
#endif

#ifndef DEC32_MANT_DIG
#define DEC32_MANT_DIG __DEC32_MANT_DIG__
#endif
#ifndef DEC64_MANT_DIG
#define DEC64_MANT_DIG __DEC64_MANT_DIG__
#endif
#ifndef DEC128_MANT_DIG
#define DEC128_MANT_DIG __DEC128_MANT_DIG__
#endif
#ifndef DEC32_MIN_EXP
#define DEC32_MIN_EXP __DEC32_MIN_EXP__
#endif
#ifndef DEC64_MIN_EXP
#define DEC64_MIN_EXP __DEC64_MIN_EXP__
#endif
#ifndef DEC128_MIN_EXP
#define DEC128_MIN_EXP __DEC128_MIN_EXP__
#endif
#ifndef DEC32_MAX_EXP
#define DEC32_MAX_EXP __DEC32_MAX_EXP__
#endif
#ifndef DEC64_MAX_EXP
#define DEC64_MAX_EXP __DEC64_MAX_EXP__
#endif
#ifndef DEC128_MAX_EXP
#define DEC128_MAX_EXP __DEC128_MAX_EXP__
#endif
#ifndef DEC32_EPSILON
#define DEC32_EPSILON __DEC32_EPSILON__
#endif
#ifndef DEC64_EPSILON
#define DEC64_EPSILON __DEC64_EPSILON__
#endif
#ifndef DEC128_EPSILON
#define DEC128_EPSILON __DEC128_EPSILON__
#endif
#ifndef DEC32_MIN
#define DEC32_MIN __DEC32_MIN__
#endif
#ifndef DEC64_MIN
#define DEC64_MIN __DEC64_MIN__
#endif
#ifndef DEC128_MIN
#define DEC128_MIN __DEC128_MIN__
#endif
#ifndef DEC32_MAX
#define DEC32_MAX __DEC32_MAX__
#endif
#ifndef DEC64_MAX
#define DEC64_MAX __DEC64_MAX__
#endif
#ifndef DEC128_MAX
#define DEC128_MAX __DEC128_MAX__
#endif
#ifndef DEC32_DEN
#define DEC32_DEN __DEC32_SUBNORMAL_MIN__
#endif
#ifndef DEC64_DEN
#define DEC64_DEN __DEC64_SUBNORMAL_MIN__
#endif
#ifndef DEC128_DEN
#define DEC128_DEN __DEC128_SUBNORMAL_MIN__
#endif

#endif /* __STDC_WANT_DEC_FP__ */

#endif /* #ifndef _FLOAT_H__ */

#else /* !(__INTEL_COMPILER || __INTEL_LLVM_COMPILER) */
#if defined(__has_include_next) // in case compiler doesn't have include_next (e.g. MSVC)
#include_next <float.h>
#endif
#endif /* __INTEL_COMPILER || __INTEL_LLVM_COMPILER */
