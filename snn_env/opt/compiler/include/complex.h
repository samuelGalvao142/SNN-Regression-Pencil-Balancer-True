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

#if defined(__ANDROID__)
    #include <android/api-level.h>
#endif

/* Include_next should be before guard macros in order to at last reach system header */
#if defined(__PURE_SYS_C99_HEADERS__) && !(defined(_WIN32) || defined(_WIN64)) && (!defined(__ANDROID__) || __ANDROID_API__ > 19) && !(defined(__VXWORKS__) && defined(_WRS_KERNEL))
    #include_next <complex.h> /* utilize sys header */
#else

    #if !defined(__PURE_INTEL_C99_HEADERS__) && !(defined(_WIN32) || defined(_WIN64)) && (!defined(__ANDROID__) || __ANDROID_API__ > 19)  && !(defined(__VXWORKS__) && defined(_WRS_KERNEL))
        #include_next <complex.h> /* utilize and expand sys header */
    #endif

    #ifndef __COMPLEX_H_INCLUDED
        #define __COMPLEX_H_INCLUDED

        /* Check usage correctness */
        #if !defined(__INTEL_COMPILER) && !defined(__INTEL_LLVM_COMPILER)
            #error "This Intel <complex.h> is for use with only the Intel(R) compilers!"
        #endif

        #if !defined (__unix__) && !defined (__APPLE__) && (!defined (__STDC_VERSION__) || (defined (__STDC_VERSION__) && (__STDC_VERSION__ < 199901L)))
            #ifndef __cplusplus
                #warning "The /Qstd=c99 compilation option is required to enable C99 support for C programs"
            #endif
        #else

            #include <math_common_define.h>

            #if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
                /* CMPLX macros - for C11 or newer. */
                #ifndef CMPLX
                    #define CMPLX(x, y)  __builtin_complex ((double) (x), (double) (y))
                #endif

                #ifndef CMPLXF
                    #define CMPLXF(x, y) __builtin_complex ((float) (x), (float) (y))
                #endif

                #ifndef CMPLXL
                    #define CMPLXL(x, y) __builtin_complex ((long double) (x), (long double) (y))
                #endif
            #endif

            /*-- Complex functions --*/

            #if !defined(__cplusplus) /* No _Complex or GNU __complex__ types available for C++ */

                #if (!defined(__linux__) && !defined(__APPLE__) && !defined(__NetBSD__) && !defined(__FreeBSD__) && !defined(__QNX__) && !defined(__VXWORKS__)) || defined(__PURE_INTEL_C99_HEADERS__)
                    #define complex _Complex
                    #define _Complex_I (1.0iF)
                    #undef I
                    #define I _Complex_I

                    /* to get around definition of complex macro in math.h on Windows*/
                    #if (defined(_WIN32) || defined(_WIN64)) && (!defined(__cplusplus))
                        #ifndef _COMPLEX_DEFINED
                            #define _COMPLEX_DEFINED
                        #endif /* _COMPLEX_DEFINED */
                    #endif /* (defined(_WIN32) || defined(_WIN64)) && (!defined(__cplusplus)) */
                #endif

                /* Complex trigonometric functions */

                #if (!defined(__linux__) && !defined(__APPLE__)) || defined(__ANDROID__) || defined(__PURE_INTEL_C99_HEADERS__)
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI ccos( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI ccosf( float _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI csin( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI csinf( float _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI ctan( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI ctanf( float _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI cacos( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI cacosf( float _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI casin( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI casinf( float _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI catan( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI catanf( float _Complex __z );
                _LIBIMF_EXTERN_C double          _LIBIMF_PUBAPI carg( double _Complex __z );
                _LIBIMF_EXTERN_C float           _LIBIMF_PUBAPI cargf( float _Complex __z );
                #endif
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI cis( double __x );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI cisf( float __x );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI cisl( long double __x );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI cisd( double __x );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI cisdf( float __x );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI cisdl( long double __x );

                /* Complex exponential functions */

                #if (!defined(__linux__) && !defined(__APPLE__)) || defined(__ANDROID__) || defined(__PURE_INTEL_C99_HEADERS__)
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI cexp( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI cexpf( float _Complex __z );
                #endif
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI cexp2( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI cexp2f( float _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI cexp2l( long double _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI cexp10( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI cexp10f( float _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI cexp10l( long double _Complex __z );
                #if (!defined(__linux__) && !defined(__APPLE__)) || defined(__ANDROID__) || defined(__PURE_INTEL_C99_HEADERS__)
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI ccosh( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI ccoshf( float _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI csinh( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI csinhf( float _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI ctanh( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI ctanhf( float _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI cacosh( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI cacoshf( float _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI casinh( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI casinhf( float _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI catanh( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI catanhf( float _Complex __z );
                #endif

                /* Complex logarithmic functions */

                #if (!defined(__linux__) && !defined(__APPLE__)) || defined(__ANDROID__) || defined(__PURE_INTEL_C99_HEADERS__)
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI clog( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI clogf( float _Complex __z );
                #endif
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI clog2( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI clog2f( float _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI clog2l( long double _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI clog10( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI clog10f( float _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI clog10l( long double _Complex __z );

                /* Complex power/root/abs functions */

                #if (!defined(__linux__) && !defined(__APPLE__)) || defined(__ANDROID__) || defined(__PURE_INTEL_C99_HEADERS__)
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI cpow( double _Complex __z, double _Complex __c );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI cpowf( float _Complex __z, float _Complex __c );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI csqrt( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI csqrtf( float _Complex __z );
                _LIBIMF_EXTERN_C double          _LIBIMF_PUBAPI cabs( double _Complex __z );
                _LIBIMF_EXTERN_C float           _LIBIMF_PUBAPI cabsf( float _Complex __z );
                #endif

                /* Other complex functions */

                #if (!defined(__linux__) && !defined(__APPLE__)) || defined(__ANDROID__) || defined(__PURE_INTEL_C99_HEADERS__)
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI conj( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI conjf( float _Complex __z );
                _LIBIMF_EXTERN_C double _Complex _LIBIMF_PUBAPI cproj( double _Complex __z );
                _LIBIMF_EXTERN_C float _Complex  _LIBIMF_PUBAPI cprojf( float _Complex __z );
                _LIBIMF_EXTERN_C double          _LIBIMF_PUBAPI cimag( double _Complex __z );
                _LIBIMF_EXTERN_C float           _LIBIMF_PUBAPI cimagf( float _Complex __z );
                _LIBIMF_EXTERN_C double          _LIBIMF_PUBAPI creal( double _Complex __z );
                _LIBIMF_EXTERN_C float           _LIBIMF_PUBAPI crealf( float _Complex __z );
                #endif

                #if (!defined(__linux__) && !defined(__APPLE__)) || defined(__ANDROID__) || defined(__PURE_INTEL_C99_HEADERS__)
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI ccosl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI csinl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI ctanl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI cacosl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI casinl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI catanl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double          _LIBIMF_PUBAPI cargl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI cexpl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI ccoshl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI csinhl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI ctanhl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI cacoshl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI casinhl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI catanhl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI clogl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI cpowl( long double _Complex __z, long double _Complex __c );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI csqrtl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double          _LIBIMF_PUBAPI cabsl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI conjl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double _Complex _LIBIMF_PUBAPI cprojl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double          _LIBIMF_PUBAPI cimagl( long double _Complex __z );
                _LIBIMF_EXTERN_C long double          _LIBIMF_PUBAPI creall( long double _Complex __z );
                #endif

                #if (__IMFLONGDOUBLE == 64) && !defined(__ANDROID__)
                    #define cabsl   cabs
                    #define cacoshl cacosh
                    #define cacosl  cacos
                    #define cargl   carg
                    #define casinhl casinh
                    #define casinl  casin
                    #define catanhl catanh
                    #define catanl  catan
                    #define ccoshl  ccosh
                    #define ccosl   ccos
                    #define cexp10l cexp10
                    #define cexp2l  cexp2
                    #define cexpl   cexp
                    #define cimagl  cimag
                    #define cisdl   cisd
                    #define cisl    cis
                    #define clog10l clog10
                    #define clog2l  clog2
                    #define clogl   clog
                    #define conjl   conj
                    #define cpowl   cpow
                    #define cprojl  cproj
                    #define creall  creal
                    #define csinhl  csinh
                    #define csinl   csin
                    #define csqrtl  csqrt
                    #define ctanhl  ctanh
                    #define ctanl   ctan
                #endif

            #endif /*!__cplusplus*/

            #include <math_common_undefine.h>

        #endif
    #endif /*__COMPLEX_H_INCLUDED*/
#endif /* usage of sys headers */
