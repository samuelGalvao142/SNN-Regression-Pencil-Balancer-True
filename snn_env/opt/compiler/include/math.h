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



#if !(defined(__INTEL_COMPILER) || defined(__INTEL_LLVM_COMPILER)) && \
    !defined(__PURE_SYS_C99_HEADERS__)
    #define __TOGGLED_PURE_SYS_C99_HEADERS__
    #define __PURE_SYS_C99_HEADERS__
#endif


#if defined(__PURE_SYS_C99_HEADERS__)

    #if (defined(_WIN32) || defined(_WIN64))
        #if defined(__INTEL_COMPILER) || defined(__INTEL_LLVM_COMPILER)
            #include_next <math.h> /* utilize system header */
        #else
            #if _MSC_VER >= 1900 /* MSVS2015+ */
                /* Location of math.h has been changed starting with VS2015 */
                #include <../ucrt/math.h>
            #elif _MSC_VER >= 1400 /* Previous versions of MSVS are not supported. */
                /*
                   Here, the #include <../../vc/include/header.h> is used as the
                   equivalent of #include_next<header.h> working for MS C compiler
                   of MSVS 2005, 2008, 2010 with default installation paths.
                   The equivalent works correctly when Intel(R) compiler header is not
                   located in ../../vc/include subfolder for any searched include path,
                   and MS header is.
                   In case of non standard location of MS headers, say in
                   C:/PROGRA~1/MSVS/NEW_VC/INCLUDE folder, proper __MS_VC_INSTALL_PATH
                   macro should be defined in command line -D option
                   like -D__MS_VC_INSTALL_PATH=C:/PROGRA~1/MSVS/NEW_VC.
                */
                #ifndef __MS_VC_INSTALL_PATH
                    #define __MS_VC_INSTALL_PATH    ../../vc
                #endif

                #define __TMP_GLUE(a,b)         a##b
                #define __TMP_PASTE2(a,b)       __TMP_GLUE(a,b)
                #define __TMP_ANGLE_BRACKETS(x) <x>
                #include __TMP_ANGLE_BRACKETS(__TMP_PASTE2(__MS_VC_INSTALL_PATH,/include/math.h))
                #undef __TMP_GLUE
                #undef __TMP_PASTE2
                #undef __TMP_ANGLE_BRACKETS
            #endif
        #endif
    #else
        #include_next <math.h>
    #endif

    #if defined(__TOGGLED_PURE_SYS_C99_HEADERS__)
        #undef __TOGGLED_PURE_SYS_C99_HEADERS__
        #undef __PURE_SYS_C99_HEADERS__
    #endif

#else /* (__PURE_SYS_C99_HEADERS__) */

    /* To avoid conflicts, we mangle in the sys header file the names we then declare */
    #if !defined(__PURE_INTEL_C99_HEADERS__) /* utilize and expand sys header */

        /* Use these definitions to get around inline implementations in MS header file */
        #if (defined(_WIN32) || defined(_WIN64))

            #if !defined (_M_X64)
                #define acosf     __MS_acosf
                #define asinf     __MS_asinf
                #define atan2f    __MS_atan2f
                #define atanf     __MS_atanf
                #define ceilf     __MS_ceilf
                #define coshf     __MS_coshf
                #define cosf      __MS_cosf
                #define expf      __MS_expf
            #endif /* _M_X64 */

            #define fabsf         __MS_fabsf

            #if !defined (_M_X64)
                #define floorf    __MS_floorf
                #define fmodf     __MS_fmodf
            #endif /* _M_X64 */

            #define frexpf        __MS_frexpf
            #define hypotf        __MS_hypotf
            #define _matherr      __MS_matherr

            #if !defined(__INTEL_LLVM_COMPILER) || (defined(__AVX512F__) && defined(__INTEL_LLVM_COMPILER))
                #define ldexpf    __MS_ldexpf
            #else
                #define _MS_LDEXPF_INLINED_
            #endif

            #if _MSC_VER < 1900 /* MSVS2015- */
                /* prior to MSVS2015, hypot is inlined in MS header file */
                #define hypot     __MS_hypot
            #endif /* _MSC_VER < 1900 */

            #if !defined (_M_X64)
                #define log10f    __MS_log10f
                #define logf      __MS_logf
                #define modff     __MS_modff

                #if !defined(__INTEL_LLVM_COMPILER)
                    #define powf  __MS_powf
                #endif

                #define sinhf     __MS_sinhf
                #define sinf      __MS_sinf
                #define sqrtf     __MS_sqrtf
                #define tanhf     __MS_tanhf
                #define tanf      __MS_tanf
            #endif /* _M_X64 */

            #define acosl         __MS_acosl
            #define asinl         __MS_asinl
            #define atan2l        __MS_atan2l
            #define atanl         __MS_atanl
            #define ceill         __MS_ceill
            #define coshl         __MS_coshl
            #define cosl          __MS_cosl
            #define expl          __MS_expl
            #define fabsl         __MS_fabsl
            #define floorl        __MS_floorl
            #define fmodl         __MS_fmodl
            #define frexpl        __MS_frexpl
            #define hypotl        __MS_hypotl
            #define ldexpl        __MS_ldexpl
            #define logl          __MS_logl
            #define log10l        __MS_log10l
            #define modfl         __MS_modfl
            #define powl          __MS_powl
            #define sinhl         __MS_sinhl
            #define sinl          __MS_sinl
            #define sqrtl         __MS_sqrtl
            #define tanhl         __MS_tanhl
            #define tanl          __MS_tanl

            /* Define the macro to get around definition of complex macro in MS math.h */
            #ifndef _COMPLEX_DEFINED
                /* Define _complex struct to be compatible with math.h on Windows*/
                struct _complex {
                    double x, y; /* real and imaginary parts */
                };
                #define _COMPLEX_DEFINED
            #endif /* _COMPLEX_DEFINED */

            /* User should include complex.h to get definition of complex macro and cabs */
            #if defined(__INTEL_COMPILER)
                #define cabs    __MS_cabs
            #endif

            #if defined(__INTEL_COMPILER) || defined(__INTEL_LLVM_COMPILER)
                #include_next <math.h> /* utilize system header */
            #else
                #if _MSC_VER >= 1900 /* MSVS2015+ */
                    /* Location of math.h has been changed starting with VS2015 */
                    #include <../ucrt/math.h>
                #elif _MSC_VER >= 1400 /* Previous versions of MSVS are not supported. */
                    /*
                       Here, the #include <../../vc/include/header.h> is used as the
                       equivalent of #include_next<header.h> working for MS C compiler
                       of MSVS 2005, 2008, 2010 with default installation paths.
                       The equivalent works correctly when Intel(R) compiler header is not
                       located in ../../vc/include subfolder for any searched include path,
                       and MS header is.
                       In case of non standard location of MS headers, say in
                       C:/PROGRA~1/MSVS/NEW_VC/INCLUDE folder, proper __MS_VC_INSTALL_PATH
                       macro should be defined in command line -D option
                       like -D__MS_VC_INSTALL_PATH=C:/PROGRA~1/MSVS/NEW_VC.
                    */
                    #ifndef __MS_VC_INSTALL_PATH
                        #define __MS_VC_INSTALL_PATH ../../vc
                    #endif

                    #define __TMP_GLUE(a,b)         a##b
                    #define __TMP_PASTE2(a,b)       __TMP_GLUE(a,b)
                    #define __TMP_ANGLE_BRACKETS(x) <x>
                    #include __TMP_ANGLE_BRACKETS(__TMP_PASTE2(__MS_VC_INSTALL_PATH,/include/math.h))
                    #undef __TMP_GLUE
                    #undef __TMP_PASTE2
                    #undef __TMP_ANGLE_BRACKETS
                #endif
            #endif
        #else
            #include_next <math.h>
        #endif
    #endif

    /* Undefining symbols we define that the system header file might define as macros */
    #undef acosf
    #undef acosl
    #undef asinf
    #undef asinl
    #undef atan2f
    #undef atan2l
    #undef atanf
    #undef atanl
    #undef ceilf
    #undef ceill
    #undef cosf
    #undef cosl
    #undef coshf
    #undef coshl
    #undef expf
    #undef expl
    #undef fabsf
    #undef fabsl
    #undef floorf
    #undef floorl
    #undef fmodf
    #undef fmodl
    #undef frexpf
    #undef frexpl
    #undef hypotf
    #undef hypot
    #undef hypotl
    #undef ldexpf
    #undef ldexpl
    #undef log10f
    #undef log10l
    #undef logf
    #undef logl
    #undef modff
    #undef modfl
    #undef powf
    #undef powl
    #undef sinf
    #undef sinl
    #undef sinhf
    #undef sinhl
    #undef sqrtf
    #undef sqrtl
    #undef tanf
    #undef tanl
    #undef tanhf
    #undef tanhl
    #undef _matherr
    #if defined(__INTEL_COMPILER)
        #undef cabs
    #endif

    #ifndef __MATH_H_INCLUDED
        #define __MATH_H_INCLUDED

        #include <math_common_define.h>

        #if (!((defined(__GNUC__) && (__GNUC__ >= 6) && defined(__cplusplus) && (__cplusplus >= 201103L)) || \
               ((defined(__clang__) && defined(__cplusplus)) && \
                ((__clang_major__ >= 9) || defined(__ANDROID__) || defined(__APPLE__)))))
            #undef fpclassify
            #undef finite
            #undef isnan
            #undef isinf
            #undef isnormal
            #undef isfinite
            #undef signbit
            #undef isgreater
            #undef isless
            #undef isgreaterequal
            #undef islessequal
            #undef islessgreater
            #undef isunordered
        #endif

        #undef MATH_ERRNO
        #undef MATH_ERREXCEPT
        #undef math_errhandling

        #define MATH_ERRNO        1
        #define MATH_ERREXCEPT    2
        #define math_errhandling (MATH_ERRNO | MATH_ERREXCEPT)

        /* NOTE: This should be properly defined by the system header file */
        #if (!defined (__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__FreeBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) && !defined(_MSC_FULL_VER) || (defined(_MSC_FULL_VER) && (_MSC_FULL_VER < 180021005)) || defined (__PURE_INTEL_C99_HEADERS__)
            #if defined (FLT_EVAL_METHOD) && (FLT_EVAL_METHOD == 0)
                typedef float  float_t;
                typedef double double_t;
            #elif defined (FLT_EVAL_METHOD) && (FLT_EVAL_METHOD == 1)
                typedef double float_t;
                typedef double double_t;
            #elif defined (FLT_EVAL_METHOD) && (FLT_EVAL_METHOD == 2)
                typedef long double float_t;
                typedef long double double_t;
            #else
                typedef float  float_t;
                typedef double double_t;
            #endif
        #endif

        #if defined(__PURE_INTEL_C99_HEADERS__) || !(defined(__unix__) || defined(__APPLE__) || defined(__QNX__) || defined(__VXWORKS__)) /* We need to define FP_ILOGB0, FP_ILOGBNAN */
            #if (defined(__FreeBSD__)) && !(defined(__ECL) || defined(__ECC)) /* FreeBSD - for other unix macro are defined in included math.h*/
                #define FP_ILOGB0   (-2147483647 - 1)
                #define FP_ILOGBNAN (-2147483647 - 1)
            #else /* Windows and Intel(R) Itanium(R) architecture */
                #ifndef FP_ILOGB0
                    #define FP_ILOGB0   (-2147483647 - 1)
                #endif /* FP_ILOGB0 */

                #ifndef FP_ILOGBNAN
                    #define FP_ILOGBNAN 2147483647
                #endif /* FP_ILOGBNAN */
            #endif
        #endif

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__FreeBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)

            #ifndef NAN
                static unsigned int __libm_qnan[] = {0x7fc00000};
                #define NAN (*((float *)__libm_qnan))
            #endif /* NAN */

            #ifndef HUGE_VALF
                static const unsigned int __libm_huge_valf[] = {0x7f800000};
                #define HUGE_VALF (*((float *)__libm_huge_valf))
            #endif /* HUGE_VALF */

            #ifndef HUGE_VALL
                #if (__IMFLONGDOUBLE ==  64)
                    static const unsigned int __libm_huge_vall[] = { 0, 0x7ff00000 };
                #elif (__IMFLONGDOUBLE ==  128)
                    static const unsigned int __libm_huge_vall[] = { 0, 0, 0, 0x7fff0000 };
                #else /* (__IMFLONGDOUBLE ==  80 */
                    static const unsigned int __libm_huge_vall[] = { 0x00000000, 0x80000000, 0x00007fff, 0 };
                #endif

                #define HUGE_VALL (*((long double *)__libm_huge_vall))
            #endif /* HUGE_VALL */

            #ifndef HUGE_VAL
                static const unsigned int __libm_huge_val[] = {0, 0x7ff00000};
                #define HUGE_VAL (*((double *)__libm_huge_val))
            #endif /* HUGE_VAL */

            #ifndef INFINITY
                static const unsigned int __libm_infinity[] = {0x7f800000};
                #define INFINITY (*((float *)__libm_infinity))
            #endif /* INFINITY */
        #endif

        /* Classification macros */

        #if defined (__IWMMXT__) || defined(__PURE_INTEL_C99_HEADERS__) || defined(_WIN32) || defined(_WIN64)

            #undef FP_NAN
            #undef FP_INFINITE
            #undef FP_ZERO
            #undef FP_SUBNORMAL
            #undef FP_NORMAL

            #if defined(__APPLE__)
                #define FP_NAN       (1)
                #define FP_INFINITE  (2)
                #define FP_ZERO      (3)
                #define FP_NORMAL    (4)
                #define FP_SUBNORMAL (5)
            #elif defined (__FreeBSD__) || defined(__ANDROID__) || (defined (__QNX__) && defined(__x86_64__))
                #define FP_NAN       (2)
                #define FP_INFINITE  (1)
                #define FP_ZERO      (16)
                #define FP_SUBNORMAL (8)
                #define FP_NORMAL    (4)
            #elif defined (__VXWORKS__) || (defined(__QNX__) && !defined(__x86_64__)) || defined(_WIN32) || defined(_WIN64)
                #define FP_NAN       (2)
                #define FP_INFINITE  (1)
                #define FP_ZERO      (0)
                #define FP_SUBNORMAL (-2)
                #define FP_NORMAL    (-1)
            #else
                #define FP_NAN       (0)
                #define FP_INFINITE  (1)
                #define FP_ZERO      (2)
                #define FP_SUBNORMAL (3)
                #define FP_NORMAL    (4)
            #endif
        #endif

        #if !defined FP_NAN
            #define FP_NAN       (0)
        #endif

        #if !defined FP_INFINITE
            #define FP_INFINITE  (1)
        #endif

        #if !defined FP_ZERO
            #define FP_ZERO      (2)
        #endif

        #if !defined FP_SUBNORMAL
            #define FP_SUBNORMAL (3)
        #endif

        #if !defined FP_NORMAL
            #define FP_NORMAL    (4)
        #endif

        #if !(defined(__VXWORKS__)) || !(defined(__cplusplus))
            _LIBIMF_EXTERN_C int fpclassifyf    ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #if !defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__)
            _LIBIMF_EXTERN_C int fpclassify     ( _LIBIMF_DBL_XDBL   __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #endif
            _LIBIMF_EXTERN_C int fpclassifyd    ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            _LIBIMF_EXTERN_C int fpclassifyl    ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();

            _LIBIMF_EXTERN_C int isinff         ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #if !defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__)
            _LIBIMF_EXTERN_C int isinf          ( _LIBIMF_DBL_XDBL   __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #endif
            _LIBIMF_EXTERN_C int isinfd         ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            _LIBIMF_EXTERN_C int isinfl         ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();

            _LIBIMF_EXTERN_C int isnanf         ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #if !defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__)
            _LIBIMF_EXTERN_C int isnan          ( _LIBIMF_DBL_XDBL   __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #endif
            _LIBIMF_EXTERN_C int isnand         ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            _LIBIMF_EXTERN_C int isnanl         ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();

            _LIBIMF_EXTERN_C int isnormalf      ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #if !defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__)
            _LIBIMF_EXTERN_C int isnormal       ( _LIBIMF_DBL_XDBL   __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #endif
            _LIBIMF_EXTERN_C int isnormald      ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            _LIBIMF_EXTERN_C int isnormall      ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();

            _LIBIMF_EXTERN_C int isfinitef      ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #if !defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__)
            _LIBIMF_EXTERN_C int isfinite       ( _LIBIMF_DBL_XDBL   __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #endif
            _LIBIMF_EXTERN_C int isfinited      ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            _LIBIMF_EXTERN_C int isfinitel      ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();

            _LIBIMF_EXTERN_C int finitef        ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #if !defined(__APPLE__) || defined(__PURE_INTEL_C99_HEADERS__)
            _LIBIMF_EXTERN_C int finite         ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #endif
            _LIBIMF_EXTERN_C int finited        ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            _LIBIMF_EXTERN_C int finitel        ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();

            _LIBIMF_EXTERN_C int signbitf       ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #if !defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__)
            _LIBIMF_EXTERN_C int signbit        ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            #endif
            _LIBIMF_EXTERN_C int signbitd       ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            _LIBIMF_EXTERN_C int signbitl       ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        #endif

        #if !defined(__APPLE__) || defined(__PURE_INTEL_C99_HEADERS__)
            _LIBIMF_EXTERN_C int __fpclassifyf  ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            _LIBIMF_EXTERN_C int __fpclassify   ( _LIBIMF_DBL_XDBL   __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            _LIBIMF_EXTERN_C int __fpclassifyd  ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
            _LIBIMF_EXTERN_C int __fpclassifyl  ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        #endif

        _LIBIMF_EXTERN_C int __isinff       ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __isinf        ( _LIBIMF_DBL_XDBL   __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __isinfd       ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __isinfl       ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();

        _LIBIMF_EXTERN_C int __isnanf       ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __isnan        ( _LIBIMF_DBL_XDBL   __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __isnand       ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __isnanl       ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();

        _LIBIMF_EXTERN_C int __isnormalf    ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __isnormal     ( _LIBIMF_DBL_XDBL   __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __isnormald    ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __isnormall    ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();

        _LIBIMF_EXTERN_C int __isfinitef    ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __isfinite     ( _LIBIMF_DBL_XDBL   __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __isfinited    ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __isfinitel    ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();

        _LIBIMF_EXTERN_C int __finitef      ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __finite       ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __finited      ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __finitel      ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();

        _LIBIMF_EXTERN_C int __signbitf     ( float              __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __signbit      ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __signbitd     ( double             __x ) _LIBIMF_CPP_EXCEPT_SPEC();
        _LIBIMF_EXTERN_C int __signbitl     ( long double        __x ) _LIBIMF_CPP_EXCEPT_SPEC();

#define __IMFC99MACRO1ARG_ALL( __x__, __func__, __fprefix__, __fsuffix__, \
                                                __dprefix__, __dsuffix__, \
                                                __lprefix__, __lsuffix__) \
    (( sizeof( __x__ ) > sizeof( double ))                                \
     ? __lprefix__##__func__##__lsuffix__( (long double)(__x__) )         \
     : (( sizeof( __x__ ) == sizeof( float ))                             \
        ? __fprefix__##__func__##__fsuffix__(    (float)(__x__) )         \
        : __dprefix__##__func__##__dsuffix__(   (double)(__x__) )         \
       )                                                                  \
    )


        #undef __IMFUSE_MACRO_FOR_CLASSIFY_AND_COMPARE

        #if defined(__GNUC__)
            #if defined(__APPLE__)
                #if !defined(__clang__)
                    #define __IMFUSE_MACRO_FOR_CLASSIFY_AND_COMPARE
                #else /* defined(__clang__) */
                    #if !defined(__cplusplus) && (__clang_major__ < 9)
                        #define __IMFUSE_MACRO_FOR_CLASSIFY_AND_COMPARE
                    #endif
                #endif
            #else /* !defined(__APPLE__) */
                #if (__GNUC__ < 6)
                    #if defined(__clang__)
                        #if !defined(__cplusplus) || (defined(__cplusplus) && (__cplusplus < 201103L))
                            #define __IMFUSE_MACRO_FOR_CLASSIFY_AND_COMPARE
                        #endif
                    #else /* !defined(__clang__) */
                        #define __IMFUSE_MACRO_FOR_CLASSIFY_AND_COMPARE
                    #endif
                #endif
            #endif
        #endif

        #if !defined(__cplusplus) || defined(__IMFUSE_MACRO_FOR_CLASSIFY_AND_COMPARE)
            #if defined (__FreeBSD__)
                #define fpclassify( __x__ ) __IMFC99MACRO1ARG_ALL( __x__, fpclassify, __, f, __, d, __, l)
                #define isinf( __x__ )      __IMFC99MACRO1ARG_ALL( __x__, isinf,      __, f, __,  , __, l)
                #define isnan( __x__ )      __IMFC99MACRO1ARG_ALL( __x__, isnan,        , f, __,  , __, l)
                #define isnormal( __x__ )   __IMFC99MACRO1ARG_ALL( __x__, isnormal,   __, f, __,  , __, l)
                #define isfinite( __x__ )   __IMFC99MACRO1ARG_ALL( __x__, isfinite,   __, f, __,  , __, l)
                #define signbit( __x__ )    __IMFC99MACRO1ARG_ALL( __x__, signbit,    __, f, __,  , __, l)
            #elif defined (__APPLE__)
                #define fpclassify( __x__ ) __IMFC99MACRO1ARG_ALL( __x__, fpclassify, __, f, __, d, __, l)
                #define isinf( __x__ )      __IMFC99MACRO1ARG_ALL( __x__, isinf,      __, f, __, d, __,  )
                #define isnan( __x__ )      __IMFC99MACRO1ARG_ALL( __x__, isnan,      __, f, __, d, __,  )
                #define isnormal( __x__ )   __IMFC99MACRO1ARG_ALL( __x__, isnormal,   __, f, __, d, __,  )
                #define isfinite( __x__ )   __IMFC99MACRO1ARG_ALL( __x__, isfinite,   __, f, __, d, __, l)
                #define signbit( __x__ )    __IMFC99MACRO1ARG_ALL( __x__, signbit,    __, f, __, d, __, l)
            #else
                #define fpclassify( __x__ ) __IMFC99MACRO1ARG_ALL( __x__, fpclassify, __, f, __,  , __, l)
                #define isinf( __x__ )      __IMFC99MACRO1ARG_ALL( __x__, isinf,      __, f, __,  , __, l)
                #define isnan( __x__ )      __IMFC99MACRO1ARG_ALL( __x__, isnan,      __, f, __,  , __, l)
                #define isnormal( __x__ )   __IMFC99MACRO1ARG_ALL( __x__, isnormal,   __, f, __,  , __, l)
                #define isfinite( __x__ )   __IMFC99MACRO1ARG_ALL( __x__, finite,     __, f, __,  , __, l)
                #define signbit( __x__ )    __IMFC99MACRO1ARG_ALL( __x__, signbit,    __, f, __,  , __, l)
            #endif
        #endif

        /* Comparison macros */

        _LIBIMF_EXTERN_C int isgreaterf( float __xf, float __yf );
        #if !defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C int isgreater( double __xd, double __yd );
        #endif
        _LIBIMF_EXTERN_C int isgreaterl( long double __xl, long double __yl );
        _LIBIMF_EXTERN_C int __isgreaterf( float __xf, float __yf );
        _LIBIMF_EXTERN_C int __isgreater( double __xd, double __yd );
        _LIBIMF_EXTERN_C int __isgreaterl( long double __xl, long double __yl );

        _LIBIMF_EXTERN_C int isgreaterequalf( float __xf, float __yf );
        #if !defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C int isgreaterequal( double __xd, double __yd );
        #endif
        _LIBIMF_EXTERN_C int isgreaterequall( long double __xl, long double __yl );
        _LIBIMF_EXTERN_C int __isgreaterequalf( float __xf, float __yf );
        _LIBIMF_EXTERN_C int __isgreaterequal( double __xd, double __yd );
        _LIBIMF_EXTERN_C int __isgreaterequall( long double __xl, long double __yl );

        _LIBIMF_EXTERN_C int islessf( float __xf, float __yf );
        #if !defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C int isless( double __xd, double __yd );
        #endif
        _LIBIMF_EXTERN_C int islessl( long double __xl, long double __yl );
        _LIBIMF_EXTERN_C int __islessf( float __xf, float __yf );
        _LIBIMF_EXTERN_C int __isless( double __xd, double __yd );
        _LIBIMF_EXTERN_C int __islessl( long double __xl, long double __yl );

        int islessequalf( float __xf, float __yf );
        #if !defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C int islessequal( double __xd, double __yd );
        #endif
        _LIBIMF_EXTERN_C int islessequall( long double __xl, long double __yl );
        _LIBIMF_EXTERN_C int __islessequalf( float __xf, float __yf );
        _LIBIMF_EXTERN_C int __islessequal( double __xd, double __yd );
        _LIBIMF_EXTERN_C int __islessequall( long double __xl, long double __yl );

        _LIBIMF_EXTERN_C int islessgreaterf( float __xf, float __yf );
        #if !defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C int islessgreater( double __xd, double __yd );
        #endif
        _LIBIMF_EXTERN_C int islessgreaterl( long double __xl, long double __yl );
        _LIBIMF_EXTERN_C int __islessgreaterf( float __xf, float __yf );
        _LIBIMF_EXTERN_C int __islessgreater( double __xd, double __yd );
        _LIBIMF_EXTERN_C int __islessgreaterl( long double __xl, long double __yl );

        _LIBIMF_EXTERN_C int isunorderedf( float __xf, float __yf );
        #if !defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C int isunordered( double __xd, double __yd );
        #endif
        _LIBIMF_EXTERN_C int isunorderedl( long double __xl, long double __yl );
        _LIBIMF_EXTERN_C int __isunorderedf( float __xf, float __yf );
        _LIBIMF_EXTERN_C int __isunordered( double __xd, double __yd );
        _LIBIMF_EXTERN_C int __isunorderedl( long double __xl, long double __yl );

#define __IMFC99MACRO2ARG( __x__, __y__, __func__ ) \
    ((( sizeof( __x__ ) > sizeof( double )) || ( sizeof( __y__ ) > sizeof( double ))) \
     ? __func__##l( (long double)(__x__), (long double)(__y__) ) \
     : ((( sizeof( __x__ ) + sizeof( __y__ )) == (2*sizeof( float ))) \
        ? __func__##f( (float)(__x__), (float)(__y__) ) \
        : __func__( (double)(__x__), (double)(__y__) )))


        #if !defined(__cplusplus) || defined(__IMFUSE_MACRO_FOR_CLASSIFY_AND_COMPARE)
            #define isgreater( __x__, __y__ )       __IMFC99MACRO2ARG( __x__, __y__, __isgreater )
            #define isgreaterequal( __x__, __y__ )  __IMFC99MACRO2ARG( __x__, __y__, __isgreaterequal )
            #define isless( __x__, __y__ )          __IMFC99MACRO2ARG( __x__, __y__, __isless )
            #define islessequal( __x__, __y__ )     __IMFC99MACRO2ARG( __x__, __y__, __islessequal )
            #define islessgreater( __x__, __y__ )   __IMFC99MACRO2ARG( __x__, __y__, __islessgreater )
            #define isunordered( __x__, __y__ )     __IMFC99MACRO2ARG( __x__, __y__, __isunordered )
        #endif

        #undef __IMFUSE_MACRO_FOR_CLASSIFY_AND_COMPARE

        /* Real functions */

        /* Radian argument trigonometric functions */

        #if defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI acos( double __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI asin( double __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI atan( double __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI_INL atan2( double __y, double __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI cos( double __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI sin( double __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI tan( double __x );
        #endif  /*__PURE_INTEL_C99_HEADERS__*/

        #if (!defined(__unix__) && !defined(__APPLE__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI acosf( float __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI asinf( float __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI atanf( float __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI_INL atan2f( float __y, float __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI cosf( float __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI sinf( float __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI tanf( float __x );
        #endif

        /* Hyperbolic functions */

        #if (!defined(__unix__) && !defined(__APPLE__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI acosh( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI acoshf( float __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI asinh( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI asinhf( float __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI atanh( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI atanhf( float __x );
        #endif

        #if defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI cosh( double __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI sinh( double __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI tanh( double __x );
        #endif /*__PURE_INTEL_C99_HEADERS__*/

        #if (!defined(__unix__) && !defined(__APPLE__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI coshf( float __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI sinhf( float __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI tanhf( float __x );
        #endif

        /* Exponential functions */

        #if defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI exp( double __x );
        #endif /*__PURE_INTEL_C99_HEADERS__*/

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI expf( float __x );
        #endif

        #if (!defined (__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__) && !(defined(__linux__) && defined(__USE_ISOC99))) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI expm1( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI expm1f( float __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI exp2( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI exp2f( float __x );
        #endif

        #if (!defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__) && !(defined(__linux__) && defined(__USE_ISOC99))) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI exp10( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI exp10f( float __x );
        #endif

        #if defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI frexp( double __x, int *__exp );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI ldexp( double __x, int __exp );
        #endif  /*__PURE_INTEL_C99_HEADERS__*/

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI frexpf( float __x, int *__exp );
        #if !(defined _MS_LDEXPF_INLINED_)
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI ldexpf( float __x, int __exp );
        #endif
        #endif

        #if (!defined(__unix__) && !defined(__APPLE__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI nanf( const char* __tagp );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI nan ( const char* __tagp );
        _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI nanl( const char* __tagp );
        #endif

        #if (!defined(__APPLE__) && !defined(__QNX__) && !defined(__NetBSD__) && !defined(__VXWORKS__) && (!defined(__linux__) || !defined (__USE_ISOC99) || (defined (__USE_ISOC99) && !defined (__USE_MISC) && !defined(__USE_XOPEN_EXTENDED)))) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI scalb( double __x, double __y );
        #endif

        #if !defined(__QNX__) && !defined(__NetBSD__) && !defined(__VXWORKS__) && (!defined(__linux__) || !defined (__USE_ISOC99) || (defined (__USE_ISOC99) && !defined (__USE_MISC) && !defined (__USE_XOPEN_EXTENDED)) || defined(__PURE_INTEL_C99_HEADERS__))
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI scalbf( float __x, float __y );
        #endif

        #if (!defined(__linux__) && !defined(__NetBSD__) && !defined(__APPLE__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI scalbn( double __x, int __n );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI scalbnf( float __x, int __n );
        #endif

        #if (!defined (__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__) && !(defined(__linux__) && defined(__USE_ISOC99))) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI scalbln( double __x, long int __n );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI scalblnf( float __x, long int __n );
        #endif

        /* Logarithmic functions */

        #if defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI log( double __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI log10( double __x );
        #endif /*__PURE_INTEL_C99_HEADERS__*/

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI logf( float __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI log10f( float __x );
        #endif

        #if (!defined (__APPLE__) && !defined(__NetBSD__) && !defined (__QNX__) && !defined(__VXWORKS__) && !(defined(__linux__) && defined(__USE_ISOC99))) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI log2( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI log2f( float __x );
        #endif

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI log1p( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI log1pf( float __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI logb( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI logbf( float __x );
        _LIBIMF_EXTERN_C int      _LIBIMF_PUBAPI ilogb( double __x );
        _LIBIMF_EXTERN_C int      _LIBIMF_PUBAPI ilogbf( float __x );
        #endif

        #if defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI modf( double __x, double *__iptr );
        #endif /*__PURE_INTEL_C99_HEADERS__*/

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI modff( float __x, float *__iptr );
        #endif

        /* Power/root/abs functions */

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI cbrt( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI cbrtf( float __x );
        #endif

        #if defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI_INL fabs( double __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI pow( double __x, double __y );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI_INL sqrt( double __x );
        #endif /*__PURE_INTEL_C99_HEADERS__*/

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI_INL fabsf( float __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI hypot( double __x, double __y );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI hypotf( float __x, float __y );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI powf( float __x, float __y );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI_INL sqrtf( float __x );
        #endif

        /* Error and gamma functions */

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI erf( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI erff( float __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI erfc( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI erfcf( float __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI lgamma( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI lgammaf( float __x );
        #endif

        #if (!defined(__cplusplus) || defined(__PURE_INTEL_C99_HEADERS__) || defined(_WIN32) || defined(_WIN64))
        /* Obsolete alias for lgamma */
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI gamma( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI gammaf( float __x );
        /* reentrant gamma functions */
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI lgamma_r(double __x, int *__signgam);
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI lgammaf_r( float __x, int *__signgam );
        #endif

        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI gamma_r( double __x, int *__signgam );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI gammaf_r( float __x, int *__signgam );

        #if (!defined (__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__) && !(defined(__linux__) && defined(__USE_ISOC99))) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI tgamma( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI tgammaf( float __x );
        #endif

        /* Nearest integer functions */

        #if defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI ceil( double __x );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI floor( double __x );
        #endif /*__PURE_INTEL_C99_HEADERS__*/

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI ceilf( float __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI floorf( float __x );
        #endif

        #if (!defined (__APPLE__) && !defined(__QNX__) && !defined(__NetBSD__) && !defined(__VXWORKS__) && !(defined(__linux__) && defined(__USE_ISOC99))) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI nearbyint( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI nearbyintf( float __x );
        #endif

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI rint( double __x );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI rintf( float __x );
        #endif

        #if (!defined (__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__) && !(defined(__linux__) && defined(__USE_ISOC99))) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C long int      _LIBIMF_PUBAPI lrint( double __x );
        _LIBIMF_EXTERN_C long int      _LIBIMF_PUBAPI lrintf( float __x );
        _LIBIMF_EXTERN_C long long int _LIBIMF_PUBAPI llrint( double __x );
        _LIBIMF_EXTERN_C long long int _LIBIMF_PUBAPI llrintf( float __x );
        _LIBIMF_EXTERN_C double        _LIBIMF_PUBAPI round( double __x );
        _LIBIMF_EXTERN_C float         _LIBIMF_PUBAPI roundf( float __x );
        _LIBIMF_EXTERN_C long int      _LIBIMF_PUBAPI lround( double __x );
        _LIBIMF_EXTERN_C long int      _LIBIMF_PUBAPI lroundf( float __x );
        _LIBIMF_EXTERN_C long long int _LIBIMF_PUBAPI llround( double __x );
        _LIBIMF_EXTERN_C long long int _LIBIMF_PUBAPI llroundf( float __x );
        _LIBIMF_EXTERN_C double        _LIBIMF_PUBAPI trunc( double __x );
        _LIBIMF_EXTERN_C float         _LIBIMF_PUBAPI truncf( float __x );
        #endif

        /* Remainder functions */

        #if defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI fmod( double __x, double __y );
        #endif /*__PURE_INTEL_C99_HEADERS__*/

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI fmodf( float __x, float __y );
        #endif

        #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI remainder( double __x, double __y );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI remainderf( float __x, float __y );
        #endif

        #if (!defined (__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__) && !(defined(__linux__) && defined(__USE_ISOC99))) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI remquo( double __x, double __y, int *__quo );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI remquof( float __x, float __y, int *__quo );
        #endif

        /* Manipulation functions */

        #if (!defined(__linux__) && !defined(__NetBSD__) && !defined(__APPLE__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI copysign( double __x, double __y );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI copysignf( float __x, float __y );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI nextafter( double __x, double __y );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI nextafterf( float __x, float __y );
        #endif

        #if (__IMFLONGDOUBLE == 64) /* MS compatibility */
            #if !defined(_MSC_VER) || (defined(_MSC_VER) && (_MSC_VER<1800))
            _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI __libm_nexttoward64 ( double __x, double __y );
            #else
            _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI __libm_nexttoward64 ( double __x, long double __y );
            #endif
            _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI __libm_nexttoward64f( float  __x, double __y );
            _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI __libm_nexttoward64l( double __x, double __y );
        #endif

        /* Maximum, minimum, and positive difference functions */

        #if (!defined (__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__) && !(defined(__linux__) && defined(__USE_ISOC99))) || defined(__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI fdim( double __x, double __y );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI fdimf( float __x, float __y );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI fmax( double __x, double __y );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI fmaxf( float __x, float __y );
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI fmin( double __x, double __y );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI fminf( float __x, float __y );
        /* Floating multiply-add */
        _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI fma( double __x, double __y, double __z );
        _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI fmaf( float __x, float __y, float __z );
        #endif

        #if (__IMFLONGDOUBLE == 64) && !defined(__ANDROID__) /* MS compatibility */

        #if (!defined(__linux__))
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL exp10l( long double __x ) {return (long double) exp10((double) __x);}
        #endif
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL acosl( long double __x ) {return (long double) acos((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL asinl( long double __x ) {return (long double) asin((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL atan2l( long double __y, long double __x ) {return (long double) atan2((double)__y, (double) __x );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL atanl( long double __x ) {return (long double) atan((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL ceill( long double __x ) {return (long double) ceil((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL cosl( long double __x ) {return (long double) cos((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL coshl( long double __x ) {return (long double) cosh((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL expl( long double __x ) {return (long double) exp((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL fabsl( long double __x ) {return (long double) fabs((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL floorl( long double __x ) {return (long double) floor((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL fmodl( long double __x, long double __y ) {return (long double) fmod((double)__x, (double) __y );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL frexpl( long double __x, int *__exp ) {return (long double) frexp((double)__x, __exp );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL hypotl( long double __x, long double __y ) {return (long double) hypot((double)__x, (double) __y );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL ldexpl( long double __x, int __exp ) {return (long double) ldexp((double)__x, __exp );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL logl( long double __x ) {return (long double) log((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL log10l( long double __x ) {return (long double) log10((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL modfl( long double __x, long double *__iptr ) {return (long double) modf((double)__x, (double *) __iptr );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL powl( long double __x, long double __y ) {return (long double) pow((double)__x, (double) __y );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL sinhl( long double __x ) {return (long double) sinh((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL sinl( long double __x ) {return (long double) sin((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL sqrtl( long double __x ) {return (long double) sqrt((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL tanl( long double __x ) {return (long double) tan((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL tanhl( long double __x ) {return (long double) tanh((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL acoshl( long double __x ) {return (long double) acosh((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL asinhl( long double __x ) {return (long double) asinh((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL atanhl( long double __x ) {return (long double) atanh((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL cbrtl( long double __x ) {return (long double) cbrt((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL copysignl( long double __x, long double __y ) {return (long double) copysign((double)__x, (double) __y );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL erfcl( long double __x ) {return (long double) erfc((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL erfl( long double __x ) {return (long double) erf((double) __x);}
        _LIBIMF_FORCEINLINE int          _LIBIMF_PUBAPI_INL ilogbl( long double __x ) {return (long double) ilogb((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL gammal( long double __x ) {return (long double) gamma((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL gammal_r( long double __x, int *__signgam ) {return (long double) gamma_r((double) __x, __signgam);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL lgammal( long double __x ) {return (long double) lgamma((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL lgammal_r( long double __x, int *__signgam ) {return (long double) lgamma_r((double) __x, __signgam);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL log1pl( long double __x ) {return (long double) log1p((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL logbl( long double __x ) {return (long double) logb((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL nextafterl( long double __x, long double __y ) {return (long double) nextafter((double)__x, (double) __y );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL remainderl( long double __x, long double __y ) {return (long double) remainder((double)__x, (double) __y );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL rintl( long double __x ) {return (long double) rint((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL scalbnl( long double __x, int __n ) {return (long double) scalbn((double)__x, __n );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL exp2l( long double __x ) {return (long double) exp2((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL expm1l( long double __x ) {return (long double) expm1((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL fdiml( long double __x, long double __y ) {return (long double) fdim((double)__x, (double) __y );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL fmal( long double __x, long double __y, long double __z ) {return (long double) fma((double) __x, (double) __y, (double) __z);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL fmaxl( long double __x, long double __y ) {return (long double) fmax((double)__x, (double) __y );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL fminl( long double __x, long double __y ) {return (long double) fmin((double)__x, (double) __y );}
        _LIBIMF_FORCEINLINE long long int _LIBIMF_PUBAPI_INL llrintl( long double __x ) {return (long double) llrint((double) __x);}
        _LIBIMF_FORCEINLINE long long int _LIBIMF_PUBAPI_INL llroundl( long double __x ) {return (long double) llround((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL log2l( long double __x ) {return (long double) log2((double) __x);}
        _LIBIMF_FORCEINLINE long int     _LIBIMF_PUBAPI_INL lrintl( long double __x ) {return (long double) lrint((double) __x);}
        _LIBIMF_FORCEINLINE long int     _LIBIMF_PUBAPI_INL lroundl( long double __x ) {return (long double) lround((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL nearbyintl( long double __x ) {return (long double) nearbyint((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL nexttowardl( long double __x, long double __y ) {return (long double)  __libm_nexttoward64l((double)__x, (double) __y );}
        _LIBIMF_FORCEINLINE double       _LIBIMF_PUBAPI_INL nexttoward( double __x, long double __y ) {return __libm_nexttoward64l(__x, (double) __y );}
        _LIBIMF_FORCEINLINE float        _LIBIMF_PUBAPI_INL nexttowardf( float __x, long double __y ) {return __libm_nexttoward64f(__x, (double) __y );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL remquol( long double __x, long double __y, int *__quo ) {return (long double) remquo((double) __x, (double) __y, __quo);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL roundl( long double __x ) {return (long double) round((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL scalbl( long double __x, long double __y ) {return (long double) scalb((double)__x, (double) __y );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL scalblnl( long double __x, long int __n ) {return (long double) scalbln((double)__x, __n );}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL tgammal( long double __x ) {return (long double) tgamma((double) __x);}
        _LIBIMF_FORCEINLINE long double  _LIBIMF_PUBAPI_INL truncl( long double __x ) {return (long double) trunc((double) __x);}

        #else /*(__IMFLONGDOUBLE == 64)*/

            #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI acoshl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI acosl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI asinhl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI asinl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI atan2l( long double __y, long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI atanhl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI atanl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI cbrtl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI ceill( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI copysignl( long double __x, long double __y );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI coshl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI cosl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI erfcl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI erfl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI exp2l( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI exp10l( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI expl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI fabsl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI floorl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI fmodl( long double __x, long double __y );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI frexpl( long double __x, int *__exp );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI hypotl( long double __x, long double __y );
            _LIBIMF_EXTERN_C int          _LIBIMF_PUBAPI ilogbl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI ldexpl( long double __x, int __exp );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI gammal( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI gammal_r( long double __x, int *__signgam );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI lgammal( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI lgammal_r( long double __x, int *__signgam );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI log2l( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI log10l( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI log1pl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI logbl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI logl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI modfl( long double __x, long double *__iptr );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI nextafterl( long double __x, long double __y );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI powl( long double __x, long double __y );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI remainderl( long double __x, long double __y );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI rintl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI scalbnl( long double __x, int __n );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI sinhl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI sinl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI sqrtl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI tanhl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI tanl( long double __x );
            #endif

            #if (!defined (__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__) && !(defined(__linux__) && defined(__USE_ISOC99))) || defined(__PURE_INTEL_C99_HEADERS__)
            _LIBIMF_EXTERN_C double   _LIBIMF_PUBAPI nexttoward( double __x, long double __y );
            _LIBIMF_EXTERN_C float    _LIBIMF_PUBAPI nexttowardf( float __x, long double __y );
            #endif

            #if (!defined(__QNX__) && !defined(__VXWORKS__) && !(defined(__unix__) && defined(__USE_ISOC99))) || defined(__PURE_INTEL_C99_HEADERS__)
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI expm1l( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI fdiml( long double __x, long double __y );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI fmal( long double __x, long double __y, long double __z );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI fmaxl( long double __x, long double __y );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI fminl( long double __x, long double __y );
            _LIBIMF_EXTERN_C long long int _LIBIMF_PUBAPI llrintl( long double __x );
            _LIBIMF_EXTERN_C long long int _LIBIMF_PUBAPI llroundl( long double __x );
            _LIBIMF_EXTERN_C long int     _LIBIMF_PUBAPI lrintl( long double __x );
            _LIBIMF_EXTERN_C long int     _LIBIMF_PUBAPI lroundl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI nearbyintl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI nexttowardl( long double __x, long double __y );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI remquol( long double __x, long double __y, int *__quo );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI roundl( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI scalblnl( long double __x, long int __n );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI tgammal( long double __x );
            _LIBIMF_EXTERN_C long double  _LIBIMF_PUBAPI truncl( long double __x );
            #endif

        #endif /*(__IMFLONGDOUBLE == 64)*/

        /* MS compatible exception handling */

        /* Exception type passed in the type field of exception struct */
        #define _DOMAIN    1 /* argument domain error */
        #define _SING      2 /* argument singularity */
        #define _OVERFLOW  3 /* overflow range error */
        #define _UNDERFLOW 4 /* underflow range error */

        #if !defined(__linux__) || defined(__PURE_INTEL_C99_HEADERS__)
            #define _TLOSS 5 /* total loss of precision */
            #define _PLOSS 6 /* partial loss of precision */
        #endif

        typedef struct ____exception {
            int        type;
            const char *name;
            double     arg1;
            double     arg2;
            double     retval;
        } ___exception;

        #if defined(__unix__) || defined(__linux__) || defined(__APPLE__) || defined(__NetBSD__) || defined(__QNX__) || defined(__VXWORKS__)
            #if defined(__cplusplus)
                #define __exception ____exception /* map 'struct __exception'  to 'struct ____exception'  */
            #else /*__cplusplus*/
                #define exceptionf ____exceptionf /* map 'struct   exceptionf' to 'struct ____exceptionf' */
                #define exceptionl ____exceptionl /* map 'struct   exceptionl' to 'struct ____exceptionl' */
            #endif /*__cplusplus*/
        #else /* Win32 or Win64 */
            #define _exception ____exception
            #define _exceptionf ____exceptionf
            #define _exceptionl ____exceptionl
        #endif

        typedef struct ____exceptionf {
            int        type;
            const char *name;
            float      arg1;
            float      arg2;
            float      retval;
        } ___exceptionf;

        typedef struct ____exceptionl {
            int         type;
            const char  *name;
            long double arg1;
            long double arg2;
            long double retval;
        } ___exceptionl;

        #if !defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__QNX__) && !defined(__VXWORKS__) && defined (__PURE_INTEL_C99_HEADERS__)
        _LIBIMF_EXTERN_C int _LIBIMF_PUBAPI _matherr( struct ____exception  *__e );
        #endif
        _LIBIMF_EXTERN_C int _LIBIMF_PUBAPI matherrf( struct ____exceptionf *__e );
        _LIBIMF_EXTERN_C int _LIBIMF_PUBAPI matherrl( struct ____exceptionl *__e );

        /*
           User-installable exception handlers

           Static redefinition of matherr() is useful only for statically linked
           libraries. When Libm is built as a DLL, the Libm's matherr() is already
           loaded into the DLL and (statically) linked.  In this case, the only way
           to replace the library default matherr() with your matherr() is to use
           the matherr() exchange functions (see description below).

           1. In user code, implement your own substitute matherr() function.
           2. To install it, call __libm_setusermatherr(), with your
              function as an argument. Note that the __libm_setusermatherr()
              returns the address of previously defined matherr. If you save
              the address, you can use it later to restore the original matherr().
           3. Your matherr() will now be installed! Your matherr() will be called
              instead of the default matherr().
        */

        typedef int ( _LIBIMF_PUBAPI_INL *___pmatherr )( struct ____exception  *__e );
        typedef int ( _LIBIMF_PUBAPI_INL *___pmatherrf )( struct ____exceptionf *__e );
        typedef int ( _LIBIMF_PUBAPI_INL *___pmatherrl )( struct ____exceptionl *__e );

        _LIBIMF_EXTERN_C ___pmatherr  _LIBIMF_PUBAPI __libm_setusermatherr( ___pmatherr  __user_matherr );
        _LIBIMF_EXTERN_C ___pmatherrf _LIBIMF_PUBAPI __libm_setusermatherrf( ___pmatherrf __user_matherrf );
        _LIBIMF_EXTERN_C ___pmatherrl _LIBIMF_PUBAPI __libm_setusermatherrl( ___pmatherrl __user_matherrl );

        /* Standard conformance support */
        #undef __IMFUSE_VERSIONIMF_TYPE

        #if (!defined(__linux__) || !defined(__USE_MISC)) && !defined(__NetBSD__) || defined (__PURE_INTEL_C99_HEADERS__)
            #define __IMFUSE_VERSIONIMF_TYPE
        #endif


        #if defined(__GLIBC__)
            #if defined(__GLIBC_PREREQ)
                #if __GLIBC_PREREQ(2,27)
                    #define __IMFUSE_VERSIONIMF_TYPE
                #endif
            #endif
        #endif

        #ifdef __IMFUSE_VERSIONIMF_TYPE
            typedef enum ___LIB_VERSIONIMF_TYPE {
                 _IEEE_ = -1    /* IEEE-like behavior    */
                ,_SVID_         /* SysV, Rel. 4 behavior */
                ,_XOPEN_        /* Unix98                */
                ,_POSIX_LIBIMF_ /* Posix                 */
                ,_ISOC_         /* ISO C9X               */
            } _LIB_VERSIONIMF_TYPE;
        #else
            #define _LIB_VERSIONIMF_TYPE _LIB_VERSION_TYPE
        #endif

        _LIBIMF_EXTERN_C _LIB_VERSIONIMF_TYPE _LIBIMF_PUBVAR _LIB_VERSIONIMF;
        #include <math_common_undefine.h>

    #endif /* __MATH_H_INCLUDED */
#endif /* usage of sys headers */
