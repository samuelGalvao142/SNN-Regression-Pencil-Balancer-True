//===-------- hvec.h - vector classes of half -----*- C++ -*---------===//
//  Copyright (c) 2021 Intel Corporation.
//
//  SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.

///
/// \file
///
/// Concept: A C++ abstraction of simd intrinsics designed to
/// improve programmer productivity. Speed and accuracy are
/// sacrificed for utility. Facilitates an easy transition to
/// compiler intrinsics or assembly language.
///
///

#if defined(__INTEL_COMPILER)
#error This header file is only supported by icx/icpx now. If you are using icc, please transfer to icx/icpx.
#else

#pragma once

#include <algorithm>
#include <assert.h>
#include <complex>
#include <immintrin.h>
#include <iostream>
#include <numeric>
#include <tuple>
#include <type_traits>

#include <dvec.h>

/// In some non-linux compilers, each empty mixin class will still be allocated
/// space, making the class bigger than its equivalent built-in SIMD type.
/// Override this where necessary to allow the mixins to have incur additional
/// size.
#ifndef __linux__
#define EMPTY_BASES __declspec(empty_bases)
#else
#define EMPTY_BASES
#endif

/* Figure out whether to define the output operators */
#if defined(_IOSTREAM_) || defined(_CPP_IOSTREAM) ||                           \
    defined(_GLIBCXX_IOSTREAM) || defined(_LIBCPP_IOSTREAM)
#define VEC_DEFINE_OUTPUT_OPERATORS
#endif

#if defined(__AVX512FP16__)
#ifdef VEC_DEFINE_OUTPUT_OPERATORS
inline std::ostream &operator<<(std::ostream &stream, _Float16 v) {
  stream << (float)v;
  return stream;
}

inline std::ostream &operator<<(std::ostream &stream,
                                std::complex<_Float16> v) {
  // Force the use of the normal std::complex, so that it is consistent with
  // other types.
  stream << std::complex<float>(float(v.real()), float(v.imag()));
  return stream;
}
#endif // VEC_DEFINE_OUTPUT_OPERATORS
#endif // defined(__AVX512FP16__)

/// Forward declaration of class which provides types such as built_in,
/// value_type, etc.
template <typename VEC> class vec_traits {};

/// This namespace maps from intrinsics which have hard-coded sizes or types,
/// into overloaded functions with identical names. For example _mm_add_ps,
/// _mm256_add_ps, a_mm256_add_ph and so on could all map to a plain add
/// function which chooses the appropriate intrinsic based upon the type, not
/// the intrinsic name. The use of overloading makes writing the vec classes
/// considerably easier, since templates can be used to generate the appropriate
/// code, without caring about the actual number of SIMD elements that are being
/// processed.
namespace intrinsic_overloads {
/// Blend two bit-buckets of different sizes, and with different numbers of
/// elements. The template specialisations below control how many elements are
/// present in each bit-bucket, and hence how big each element is (e.g., 4
/// elements in a 256-bit bucket implies 64-bit elements).
template <int NUM_ELEMENTS>
inline __m128i blend(__mmask16 m, __m128i src0, __m128i src1);
template <> inline __m128i blend<2>(__mmask16 m, __m128i src0, __m128i src1) {
  return _mm_mask_blend_epi64(__mmask8(m), src0, src1);
}
template <> inline __m128i blend<4>(__mmask16 m, __m128i src0, __m128i src1) {
  return _mm_mask_blend_epi32(__mmask8(m), src0, src1);
}
template <> inline __m128i blend<8>(__mmask16 m, __m128i src0, __m128i src1) {
  return _mm_mask_blend_epi16(__mmask8(m), src0, src1);
}
template <> inline __m128i blend<16>(__mmask16 m, __m128i src0, __m128i src1) {
  return _mm_mask_blend_epi8(m, src0, src1);
}

template <int NUM_ELEMENTS>
inline __m256i blend(__mmask32 m, __m256i src0, __m256i src1);
template <> inline __m256i blend<4>(__mmask32 m, __m256i src0, __m256i src1) {
  return _mm256_mask_blend_epi64(__mmask8(m), src0, src1);
}
template <> inline __m256i blend<8>(__mmask32 m, __m256i src0, __m256i src1) {
  return _mm256_mask_blend_epi32(__mmask8(m), src0, src1);
}
template <> inline __m256i blend<16>(__mmask32 m, __m256i src0, __m256i src1) {
  return _mm256_mask_blend_epi16(__mmask16(m), src0, src1);
}
template <> inline __m256i blend<32>(__mmask32 m, __m256i src0, __m256i src1) {
  return _mm256_mask_blend_epi8(m, src0, src1);
}

template <int NUM_ELEMENTS>
inline __m512i blend(__mmask64 m, __m512i src0, __m512i src1);
template <> inline __m512i blend<8>(__mmask64 m, __m512i src0, __m512i src1) {
  return _mm512_mask_blend_epi64(__mmask8(m), src0, src1);
}
template <> inline __m512i blend<16>(__mmask64 m, __m512i src0, __m512i src1) {
  return _mm512_mask_blend_epi32(__mmask16(m), src0, src1);
}
template <> inline __m512i blend<32>(__mmask64 m, __m512i src0, __m512i src1) {
  return _mm512_mask_blend_epi16(__mmask32(m), src0, src1);
}
template <> inline __m512i blend<64>(__mmask64 m, __m512i src0, __m512i src1) {
  return _mm512_mask_blend_epi8(m, src0, src1);
}

template <int NUM_ELEMENTS>
inline __m128i mask_load(__m128i src0, __mmask16 m, void const *p);
template <>
inline __m128i mask_load<2>(__m128i src0, __mmask16 m, void const *p) {
  return _mm_mask_loadu_epi64(src0, __mmask8(m), p);
}
template <>
inline __m128i mask_load<4>(__m128i src0, __mmask16 m, void const *p) {
  return _mm_mask_loadu_epi32(src0, __mmask8(m), p);
}
template <>
inline __m128i mask_load<8>(__m128i src0, __mmask16 m, void const *p) {
  return _mm_mask_loadu_epi16(src0, __mmask8(m), p);
}
template <>
inline __m128i mask_load<16>(__m128i src0, __mmask16 m, void const *p) {
  return _mm_mask_loadu_epi8(src0, m, p);
}

template <int NUM_ELEMENTS>
inline __m256i mask_load(__m256i src0, __mmask32 m, void const *p);
template <>
inline __m256i mask_load<4>(__m256i src0, __mmask32 m, void const *p) {
  return _mm256_mask_loadu_epi64(src0, __mmask8(m), p);
}
template <>
inline __m256i mask_load<8>(__m256i src0, __mmask32 m, void const *p) {
  return _mm256_mask_loadu_epi32(src0, __mmask8(m), p);
}
template <>
inline __m256i mask_load<16>(__m256i src0, __mmask32 m, void const *p) {
  return _mm256_mask_loadu_epi16(src0, __mmask16(m), p);
}
template <>
inline __m256i mask_load<32>(__m256i src0, __mmask32 m, void const *p) {
  return _mm256_mask_loadu_epi8(src0, m, p);
}

template <int NUM_ELEMENTS>
inline __m512i mask_load(__m512i src0, __mmask64 m, void const *p);
template <>
inline __m512i mask_load<8>(__m512i src0, __mmask64 m, void const *p) {
  return _mm512_mask_loadu_epi64(src0, __mmask8(m), p);
}
template <>
inline __m512i mask_load<16>(__m512i src0, __mmask64 m, void const *p) {
  return _mm512_mask_loadu_epi32(src0, __mmask16(m), p);
}
template <>
inline __m512i mask_load<32>(__m512i src0, __mmask64 m, void const *p) {
  return _mm512_mask_loadu_epi16(src0, __mmask32(m), p);
}
template <>
inline __m512i mask_load<64>(__m512i src0, __mmask64 m, void const *p) {
  return _mm512_mask_loadu_epi8(src0, m, p);
}

// Broadcast a single set of bits in a register container. This is agnostic
// of the type of element to allow it to be used anywhere that bits need to be repeated.
// The classes themselves (e.g., F16vec8) must define their own type-specific broadcast
// if they wish to use them.
template <typename SIMD, typename ELEMENT> constexpr SIMD broadcast(ELEMENT h);
template <> constexpr inline __m128i broadcast(int32_t i) {
  return (__m128i)(__v4si){i, i, i, i};
}
template <> constexpr inline __m256i broadcast(int32_t i) {
  return (__m256i)(__v8si){i, i, i, i, i, i, i, i};
}
#if defined(__AVX512F__)
template <> constexpr inline __m512i broadcast(int32_t i) {
  return (__m512i)(__v16si){i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i};
}
#endif

// Non-temporal store instructions.
inline void store_nta(__m128i *p, __m128i v) { _mm_stream_si128(p, v); }
inline void store_nta(__m256i *p, __m256i v) { _mm256_stream_si256(p, v); }
inline void store_nta(__m512i *p, __m512i v) {
  _mm512_stream_si512((void *)p, v);
}

/// Masked store instructions. Write to memory those elements of the SIMD
/// register whose mask flag is set to true. All other elements in memory will
/// remain unaltered. This function allows the number of mask elements to be
/// specified, which for a given container size will imply how many bits each
/// element contains. This allows masked stores to be generated for any SIMD
/// data type by converting it to the underling bit-container, and specifying
/// how many elements it contains.
template <int NUM_ELEMENTS>
inline void mask_store(void *p, __mmask16 m, __m128i v);
template <> inline void mask_store<2>(void *p, __mmask16 m, __m128i v) {
  _mm_mask_storeu_epi64(p, __mmask8(m), v);
}
template <> inline void mask_store<4>(void *p, __mmask16 m, __m128i v) {
  _mm_mask_storeu_epi32(p, __mmask8(m), v);
}
template <> inline void mask_store<8>(void *p, __mmask16 m, __m128i v) {
  _mm_mask_storeu_epi16(p, __mmask8(m), v);
}
template <> inline void mask_store<16>(void *p, __mmask16 m, __m128i v) {
  _mm_mask_storeu_epi8(p, m, v);
}

template <int NUM_ELEMENTS>
inline void mask_store(void *p, __mmask32 m, __m256i v);
template <> inline void mask_store<4>(void *p, __mmask32 m, __m256i v) {
  _mm256_mask_storeu_epi64(p, __mmask8(m), v);
}
template <> inline void mask_store<8>(void *p, __mmask32 m, __m256i v) {
  _mm256_mask_storeu_epi32(p, __mmask8(m), v);
}
template <> inline void mask_store<16>(void *p, __mmask32 m, __m256i v) {
  _mm256_mask_storeu_epi16(p, __mmask16(m), v);
}
template <> inline void mask_store<32>(void *p, __mmask32 m, __m256i v) {
  _mm256_mask_storeu_epi8(p, m, v);
}

template <int NUM_ELEMENTS>
inline void mask_store(void *p, __mmask64 m, __m512i v);
template <> inline void mask_store<8>(void *p, __mmask64 m, __m512i v) {
  _mm512_mask_storeu_epi64(p, __mmask8(m), v);
}
template <> inline void mask_store<16>(void *p, __mmask64 m, __m512i v) {
  _mm512_mask_storeu_epi32(p, __mmask16(m), v);
}
template <> inline void mask_store<32>(void *p, __mmask64 m, __m512i v) {
  _mm512_mask_storeu_epi16(p, __mmask32(m), v);
}
template <> inline void mask_store<64>(void *p, __mmask64 m, __m512i v) {
  _mm512_mask_storeu_epi8(p, m, v);
}

/// Invert a bit mask. Uses intrinsics to get the minimal instruction sequence,
/// rather than relying on the compiler converting to int, inverting, and then
/// converting back.
inline __mmask8 mask_not(__mmask8 m) { return _knot_mask8(m); }
inline __mmask16 mask_not(__mmask16 m) { return _knot_mask16(m); }
inline __mmask32 mask_not(__mmask32 m) { return _knot_mask32(m); }
inline __mmask64 mask_not(__mmask64 m) { return _knot_mask64(m); }

/// Generate a mask which contains exactly the number of requested bits. This
/// uses the bzhi instruction which clears all bits after the given index, so by
/// using this with an all-one input, just the right number of bits are left
/// behind.
template <typename MASK_TYPE> inline MASK_TYPE get_nbit_mask(unsigned num_set_bits) {
  return MASK_TYPE(_bzhi_u32(~0UL, num_set_bits));
}

} // namespace intrinsic_overloads

/// Main vec class from which all other vec classes are derived. This class uses
/// the curiously recurring template pattern
/// (https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) to
/// allow a set of uniform utility API functions to be inherited by specific
/// concrete classes. This allows the classes such as F16vec16 to provide
/// support for named intrinsics, while allowing a much more generic base class
/// to provide everything else written in terms of those derived classes.
///
/// Mixins are used to provide different behaviours for derived classes. For
/// example, if Intel® Advanced Vector Extensions 512 has a special instruction
/// which provides a feature not available in other ISAs, that behaviour will be
/// abstracted out into a mixin which provides alternative behaviours.
template <typename VEC> class vec_base {
public:
  using value_type =
      typename vec_traits<VEC>::value_type; /// Type of each individual element.
  using reference_type = value_type &; /// Reference to an individual element.
  using builtin_type =
      typename vec_traits<VEC>::builtin_type; /// Underlying simd type, such as
                                              /// __m512.
  using mask_type =
      typename vec_traits<VEC>::mask_type; /// Bit-mask of appropriate size, one
                                           /// bit per vector element.
  using bit_type =
      typename vec_traits<VEC>::bit_type; /// Plain bucket-of-bits container.

protected:
  /// The underlying built-in type representing this SIMD data type.
  builtin_type simd;

  /// Fixed-size constant representing the number of elements in the vector.
  using vec_size = std::integral_constant<std::size_t, sizeof(builtin_type) /
                                                           sizeof(value_type)>;

  VEC &derived() { return *static_cast<VEC *>(this); }
  const VEC &derived() const { return *static_cast<const VEC *>(this); }

public:
  vec_base() = default;

  /// Allow any vector class to be built from the direct underlying type.
  constexpr vec_base(builtin_type v) : simd(v) {}

  /// Allow the vec class to be conveniently converted back to its underlying
  /// type. This makes interactions of this class with intrinsics easy to
  /// manage.
  operator builtin_type() const { return simd; }

  /// Convert a VEC value into a plain bucket-of-bits which can be used in
  /// type-independent operations.
  constexpr bit_type to_bits() const noexcept { return simd; }

  /// Does the initialiser get forward directly to the simd member?
  vec_base(std::initializer_list<value_type> init) : vec_base() {
    const mask_type n_bits = static_cast<mask_type>((1ULL << init.size()) - 1);
    this->simd = intrinsic_overloads::mask_load<vec_size::value>(
        VEC().to_bits(), n_bits, init.begin());
  }

  /// @name container
  /// Standard C++ container-like API.
  ///@{
  static constexpr std::size_t size() noexcept { return vec_size::value; }
  constexpr std::size_t max_size() const noexcept { return vec_size::value; }
  constexpr bool empty() const noexcept { return false; }
  constexpr void fill(value_type value) { derived() = VEC(value); }
  void swap(VEC &v) {
    const auto t = v.simd;
    v.simd = derived().simd;
    derived().simd = t;
  }
  friend void swap(VEC &lhs, VEC &rhs) { lhs.swap(rhs); }
  ///@}

  /// @name oldvec_compat
  /// Compatability API with existing vec classes.
  ///@{
  /// Negate the value. Equivalent to unary-.
  VEC &flip_sign() {
    auto &t = derived();
    t = -t;
    return t;
  }

  /// Set the SIMD value to all zeros. Not particularly useful as there are
  /// other easier ways to achieve this (e.g., empty brace initialisation), but
  /// it's part of the original vec API.
  void set_zero() { simd = {}; }

  /// Return true if the SIMD value is completely zeroed (i.e., every element is
  /// zero).
  bool is_zero() const {
    const mask_type allSet =
        intrinsic_overloads::get_nbit_mask<mask_type>(vec_size::value);
    return (derived() == VEC()) == allSet;
  }

  ///@}

  /// @name element_access
  /// API for accessing individual elements
  constexpr const value_type &at(std::size_t pos) const {
    assert(pos < vec_size::value);
    const value_type *elements = (const value_type *)this;
    return elements[pos];
  }

  constexpr value_type &at(std::size_t pos) {
    assert(pos < vec_size::value);
    value_type *elements = (value_type *)this;
    return elements[pos];
  }

  constexpr const value_type &operator[](std::size_t pos) const {
    return at(pos);
  }
  constexpr value_type &operator[](std::size_t pos) { return at(pos); }
  ///@}

  /// @name selection
  /// API for combining two different vectors on an element-by-element basis,
  /// using some selection criteria. Note that select and blend both do the same
  /// basic operation, but using the mask in opposite senses. Both are provided
  /// because select is compatible with existing vec convention, and blend is
  /// compatible with existing intrinsic convention.
  /// @{
  /// Given two vectors, choose one or other element as output for each element
  /// position. select works like (is_bit_set ? a : b) for each element. When
  /// the corresponding bit is set in the mask the output element is chosen to
  /// be from a, otherwise from b. The second operand defaults to zero if no
  /// value is supplied. Note that select has the opposite ordering sense to
  /// blend.
  friend VEC select(mask_type m, VEC value_if_set, VEC value_if_clear = {}) {
    return VEC(intrinsic_overloads::blend<vec_size::value>(
        m, value_if_clear.to_bits(), value_if_set.to_bits()));
  }

  /// Given two vectors, choose one or other element as output for each element
  /// position. Blend works like the intrinsics of the underlying ISA (e.g.,
  /// mask_blend_ph), where the bit represents the source index to choose - 0
  /// or 1.  Note that blend has the opposite ordering sense to select. No
  /// defaults are provided since a maskz type operation would want to default
  /// the first operand, which wouldn't be valid C++, so don't provide that
  /// option at all.
  friend VEC blend(typename vec_traits<VEC>::mask_type m, VEC value_if_0,
                   VEC value_if_1) {
    return VEC(intrinsic_overloads::blend<vec_size::value>(
        m, value_if_0.to_bits(), value_if_1.to_bits()));
  }

  /// Select N elements from the first operand, with the remaining elements
  /// coming from the second operand. The second operand defaults to zero if no
  /// value is supplied.
  friend VEC select_n(int n, VEC op0, VEC op1 = {}) {
    const auto n_bits = intrinsic_overloads::get_nbit_mask<mask_type>(n);
    return select(n_bits, op0, op1);
  }

  /// Original vec-compatible selection functions. Given two variables, compare
  /// them to each other and generate an output whose respective elements rely
  /// on that comparison. This assumes the SIMD types can be compared to each
  /// other using standard comparison operators.
  friend VEC select_eq(VEC a, VEC b, VEC c, VEC d) {
    return blend(a == b, d, c);
  }
  friend VEC select_neq(VEC a, VEC b, VEC c, VEC d) {
    return blend(a != b, d, c);
  }
  /// @}

#ifdef VEC_DEFINE_OUTPUT_OPERATORS
  /// Output the contents of the SIMD vector to a standard stream,
  /// element-by-element.
  friend std::ostream &operator<<(std::ostream &stream, vec_base value) {
    // Note that the values are printed in reverse order.
    for (std::size_t i = 0; i < value.size(); ++i) {
      const int ri = (value.size() - 1) - i;
      if (i > 0)
        stream << ' ';
      stream << '[' << ri << "]:" << value[ri];
    }
    return stream;
  }
#endif // VEC_DEFINE_OUTPUT_OPERATORS

  friend VEC simd_min(VEC lhs, VEC rhs) { return min(lhs, rhs); }
  friend VEC simd_max(VEC lhs, VEC rhs) { return max(lhs, rhs); }
};

/// Barton-Nackman mixin to provide basic numeric operators which defer their
/// operation to the underlying builtin class. This isn't part of the vec_base
/// class since the numerics operations will be different for complex vec. These
/// operators all work on vector data. Operators which work on mixed scalar-vector
/// inputs are defined in the scalar_numerics_operators mixin.
template <typename VEC> struct simd_numerics_operators {
private:
  VEC &derived() { return *static_cast<VEC *>(this); }
  const VEC &derived() const { return *static_cast<const VEC *>(this); }

protected:
  using value_type = typename vec_traits<VEC>::value_type;
  using builtin_type = typename vec_traits<VEC>::builtin_type;

public:
  VEC &operator+=(VEC rhs) {
    auto &t = derived();
    t = t + rhs;
    return t;
  }
  VEC &operator-=(VEC rhs) {
    auto &t = derived();
    t = t - rhs;
    return t;
  }
  VEC &operator*=(VEC rhs) {
    auto &t = derived();
    t = t * rhs;
    return t;
  }
  VEC &operator/=(VEC rhs) {
    auto &t = derived();
    t = t / rhs;
    return t;
  }

  VEC operator-() const { return VEC() - derived(); }
  VEC operator+() const { return derived(); }

  friend VEC operator+(VEC lhs, VEC rhs) {
    return builtin_type(lhs) + builtin_type(rhs); // Defer to LLVM's own vector support
  }
  friend VEC operator-(VEC lhs, VEC rhs) {
    return builtin_type(lhs) - builtin_type(rhs); // Defer to LLVM's own vector support
  }
  friend VEC operator*(VEC lhs, VEC rhs) {
    return builtin_type(lhs) * builtin_type(rhs); // Defer to LLVM's own vector support
  }
  friend VEC operator/(VEC lhs, VEC rhs) {
    return builtin_type(lhs) / builtin_type(rhs); // Defer to LLVM's own vector support
  }

};


/// Barton-Nackman mixin to provide mixed SIMD/scalar operators.
template <typename VEC> struct scalar_numerics_operators {
private:
  VEC &derived() { return *static_cast<VEC *>(this); }
  const VEC &derived() const { return *static_cast<const VEC *>(this); }

protected:
  using value_type = typename vec_traits<VEC>::value_type;

public:

  VEC &operator+=(value_type rhs) {
    auto &t = derived();
    t += VEC(rhs);
    return t;
  }
  VEC &operator-=(value_type rhs) {
    auto &t = derived();
    t -= VEC(rhs);
    return t;
  }
  VEC &operator*=(value_type rhs) {
    auto &t = derived();
    t *= VEC(rhs);
    return t;
  }
  VEC &operator/=(value_type rhs) {
    auto &t = derived();
    t /= VEC(rhs);
    return t;
  }

  friend VEC operator+(VEC lhs, value_type rhs) { return lhs  + VEC(rhs); }
  friend VEC operator+(value_type lhs, VEC rhs) { return VEC(lhs) + rhs; }
  friend VEC operator-(VEC lhs, value_type rhs) { return lhs - VEC(rhs); }
  friend VEC operator-(value_type lhs, VEC rhs) { return VEC(lhs) - rhs; }
  friend VEC operator*(VEC lhs, value_type rhs) { return lhs * VEC(rhs); }
  friend VEC operator*(value_type lhs, VEC rhs) { return VEC(lhs) * rhs; }
  friend VEC operator/(VEC lhs, value_type rhs) { return lhs / VEC(rhs); }
  friend VEC operator/(value_type lhs, VEC rhs) { return VEC(lhs) / rhs; }
};

/// Mixin to provide length-type operations (length, normalize, etc.) for real-valued types.
template<typename VEC>
struct vec_length_mixin
{
protected:
  VEC& derived() { return *static_cast<VEC*>(this); }
  const VEC& derived() const { return *static_cast<const VEC*>(this); }

public:

  /// Compute the dot product of the two vectors (i.e., an element-wise multiply, following by
  /// a sum over all elements).
  friend typename vec_traits<VEC>::value_type dot(VEC lhs, VEC rhs) { return add_horizontal(lhs * rhs); }
  void dot(typename vec_traits<VEC>::value_type & result, VEC rhs) const { result = add_horizontal(derived() * rhs); }
  typename vec_traits<VEC>::value_type dot(VEC rhs) const { return add_horizontal(derived() * rhs); }

  /// Compute the square of the length of a vector (i.e., the dot product with itself).
  typename vec_traits<VEC>::value_type length_sqr() const { return derived().dot(derived()); }

  /// Compute the length of the vector (i.e., the sqrt of the dot product with itself).
  /// :TODO: Use a reduced width to avoid redundant calculations in big vectors.
  typename vec_traits<VEC>::value_type length() const {
    const VEC sqrtLs = sqrt(VEC(length_sqr()));
    return sqrtLs[0];
  }

  /// Scale the vector so that it's length is 1. Note that the original dvec returns a bool which is
  /// always true. Not sure why, but respect it.
  bool normalize() {
    const VEC rsqrtLs = rsqrt(VEC(length_sqr()));
    derived() *= rsqrtLs;
    return true;
  }

  // :TODO: In the modern dvec provide friend functions for length and normalize.
};

/// Mixin to provide commonly used rounding functions. The mixin dispatches
/// to a templated function to perform the round-scale for a given constant
/// op. Alternative mixins are required for older ISAs which don't have the
/// roundscale instruction.
template <typename VEC> struct roundscale_mixin {
  friend VEC ceil(const VEC v) {
    return roundscale(v, std::integral_constant<int, _MM_FROUND_CEIL>());
  }
  friend VEC floor(const VEC v) {
    return roundscale(v, std::integral_constant<int, _MM_FROUND_FLOOR>());
  }
  friend VEC trunc(const VEC v) {
    return roundscale(v, std::integral_constant<int, _MM_FROUND_TO_ZERO>());
  }

  // Note that roundscale is subtly different to std::round. On boundary
  // conditions (e.g., 0.5, 1.5, 2.5, 3.5) the hardware rounds towards the
  // nearest even (i.e., 0, 2, 2, 4). Should round be called that since it
  // doesn't match std::round?  Maybe not, but since it's only the boundary
  // conditions which vary, and half of them do round to the expected number
  // anyway, I think that round's behaviour is close enough to what is expected
  // to deserve to be called by this name.
  friend VEC round(const VEC v) {
    return roundscale(v, std::integral_constant<int, _MM_FROUND_TO_NEAREST_INT>());
  }
};

/// Mixin to provide comparison operations. Older ISA which don't have the
/// compare_mask instruction should use alternative mixin.
template <typename VEC> struct compare_operator_mixin {
protected:
  using value_type = typename vec_traits<VEC>::value_type;

public:

  friend typename vec_traits<VEC>::mask_type operator==(const VEC lhs,
                                                        const VEC rhs) {
    return compare_mask(lhs, rhs, std::integral_constant<int, _CMP_EQ_OQ>());
  }
  friend typename vec_traits<VEC>::mask_type operator!=(const VEC lhs,
                                                        const VEC rhs) {
    return compare_mask(lhs, rhs, std::integral_constant<int, _CMP_NEQ_OQ>());
  }
  friend typename vec_traits<VEC>::mask_type operator<(const VEC lhs,
                                                       const VEC rhs) {
    return compare_mask(lhs, rhs, std::integral_constant<int, _CMP_LT_OQ>());
  }
  friend typename vec_traits<VEC>::mask_type operator<=(const VEC lhs,
                                                        const VEC rhs) {
    return compare_mask(lhs, rhs, std::integral_constant<int, _CMP_LE_OQ>());
  }
  friend typename vec_traits<VEC>::mask_type operator>(const VEC lhs,
                                                       const VEC rhs) {
    return compare_mask(lhs, rhs, std::integral_constant<int, _CMP_GT_OQ>());
  }
  friend typename vec_traits<VEC>::mask_type operator>=(const VEC lhs,
                                                        const VEC rhs) {
    return compare_mask(lhs, rhs, std::integral_constant<int, _CMP_GE_OQ>());
  }

  /// Scalar comparisons. Both LHS and RHS scalars are handled.
  friend typename vec_traits<VEC>::mask_type operator==(const value_type lhs, const VEC rhs) {
    return VEC(lhs) == rhs; }
  friend typename vec_traits<VEC>::mask_type operator==(const VEC lhs, const value_type rhs) {
    return lhs == VEC(rhs); }
  friend typename vec_traits<VEC>::mask_type operator!=(const value_type lhs, const VEC rhs) {
    return VEC(lhs) != rhs; }
  friend typename vec_traits<VEC>::mask_type operator!=(const VEC lhs, const value_type rhs) {
    return lhs != VEC(rhs); }
  friend typename vec_traits<VEC>::mask_type operator<(const value_type lhs, const VEC rhs) {
    return VEC(lhs) < rhs; }
  friend typename vec_traits<VEC>::mask_type operator<(const VEC lhs, const value_type rhs) {
    return lhs < VEC(rhs); }
  friend typename vec_traits<VEC>::mask_type operator<=(const value_type lhs, const VEC rhs) {
    return VEC(lhs) <= rhs; }
  friend typename vec_traits<VEC>::mask_type operator<=(const VEC lhs, const value_type rhs) {
    return lhs <= VEC(rhs); }
  friend typename vec_traits<VEC>::mask_type operator>(const value_type lhs, const VEC rhs) {
    return VEC(lhs) > rhs; }
  friend typename vec_traits<VEC>::mask_type operator>(const VEC lhs, const value_type rhs) {
    return lhs > VEC(rhs); }
  friend typename vec_traits<VEC>::mask_type operator>=(const value_type lhs, const VEC rhs) {
    return VEC(lhs) >= rhs; }
  friend typename vec_traits<VEC>::mask_type operator>=(const VEC lhs, const value_type rhs) {
    return lhs >= VEC(rhs); }

  /// Original vec-compatible selection functions. Given two variables, compare
  /// them to each other and generate an output whose respective elements rely
  /// on that comparison. This assumes the SIMD types can be compared to each
  /// other using standard operators. Note that the equality/inequality
  /// selection will always be provided as part of the base vec, so that it
  /// defers to whatever operator is created.
  friend VEC select_lt(VEC a, VEC b, VEC c, VEC d) {
    return blend(a < b, d, c);
  }
  friend VEC select_gt(VEC a, VEC b, VEC c, VEC d) {
    return blend(a > b, d, c);
  }
  friend VEC select_le(VEC a, VEC b, VEC c, VEC d) {
    return blend(a <= b, d, c);
  }
  friend VEC select_ge(VEC a, VEC b, VEC c, VEC d) {
    return blend(a >= b, d, c);
  }
};

/// Mixin to handle floating-point classifications.  Specialisations of the
/// classification operation are provided to dispatch to the appropriate
/// intrinsic.  Older ISA which don't have the fpclass
//// instruction should use an alternative mixin.
template <typename VEC> struct fpclass_mixin {
protected:

  using mask_type= typename vec_traits<VEC>::mask_type;

public:

  enum FP_CLASSIFY {
    QUIET_NAN = 0x01,
    POS_ZERO = 0x02,
    NEG_ZERO = 0x04,
    POS_INF = 0x08,
    NEG_INF = 0x10,
    DENORM = 0x20,
    NEG = 0x40,
    SIGNAL_NAN = 0x80
  };

  friend mask_type is_inf(const VEC v) {
    return fpclassify_mask(v, std::integral_constant<int, POS_INF | NEG_INF>());
  }
  friend mask_type is_nan(const VEC v) {
    return fpclassify_mask(v, std::integral_constant<int, QUIET_NAN | SIGNAL_NAN>());
  }
  friend mask_type is_finite(const VEC v) {
    constexpr int invalidClasses = QUIET_NAN | SIGNAL_NAN | POS_INF | NEG_INF;
    return mask_type(~fpclassify_mask(v, std::integral_constant<int, invalidClasses>()));
  }
  friend mask_type is_denormal(const VEC v) {
    return fpclassify_mask(v, std::integral_constant<int, DENORM>());
  }
};

/// Barton-Nackman style mixin to handle numeric clamping operations. 
/// By default this works using min and max operations, but some
/// Intel® Advanced Vector Extensions 512 ISAs can use the range intrinsic
/// instead to do this more efficiently.
template <typename VEC> struct clamp_mixin {
  /// Clamp the input values to be contained within the range [low,high]. If a
  /// value is less than low it will return low. If a value is greater than high
  /// it will return high. Other values are unmodified. \param values The values
  /// to clamp. \param low The low value in the clamping range. \param high The
  /// high value in the clamping range.
  friend VEC clamp(VEC values, VEC low, VEC high) {
    return min(max(values, low), high);
  }

  /// Clamp the input values to the range [-high, high].
  /// \param values The values to clamp.
  /// \param high The magnitude to which to clamp the values. This must be a
  /// positive value.
  friend VEC clamp(VEC values, VEC high) { return clamp(values, -high, high); }
};

/// Specialised load/store operations, such as masked operations or streaming
/// (NTA).
template <class VEC> struct load_store_mixin {
public:
  /// Loads and stores are always emitted as unaligned anyway, but for non-Intel
  /// compilers this could change to an intrinsic overload, or use a suitable
  /// attribute to mark the pointer as unaligned.
  friend void loadu(VEC &v, const typename vec_traits<VEC>::value_type *p) {
    v = *reinterpret_cast<const VEC *>(p);
  }
  friend void storeu(typename vec_traits<VEC>::value_type *p, VEC v) {
    *reinterpret_cast<VEC *>(p) = v;
  }

  /// Non-temporal store into the memory system. Data must be properly aligned.
  friend void store_nta(const typename vec_traits<VEC>::value_type *p, VEC v) {
    intrinsic_overloads::store_nta((typename vec_traits<VEC>::bit_type *)p,
                                   v.to_bits());
  }

  /// Overwrite selected elements from a vector with values taken from memory.
  friend void maskload(VEC &a, const typename vec_traits<VEC>::value_type *p,
                       typename vec_traits<VEC>::mask_type mask) {
    a = intrinsic_overloads::mask_load<std::tuple_size<VEC>::value>(a, mask, p);
  }

  /// Store selected elements from the given value to memory.
  friend void maskstore(typename vec_traits<VEC>::value_type *p,
                        const typename vec_traits<VEC>::mask_type mask, VEC v) {
    const auto rawBits = v.to_bits();
    intrinsic_overloads::mask_store<std::tuple_size<VEC>::value>(p, mask,
                                                                 rawBits);
  }
};

/// Synthesise some of the more unusual add/sub operations when they aren't provided.
template <class VEC> struct addsub_mixin {
  friend VEC addsub(VEC lhs, VEC rhs) {
    return fmaddsub(lhs, VEC(1.0f), rhs);
  }
  friend VEC subadd(VEC lhs, VEC rhs) {
    return fmsubadd(lhs, VEC(1.0f), rhs);
  }
};

#if defined(__AVX512FP16__)
class F16vec8;

template <> struct vec_traits<F16vec8> {
  using builtin_type = __m128h;
  using value_type = _Float16;
  using mask_type = __mmask8;
  using bit_type = __m128i;
};

class EMPTY_BASES F16vec8 : public vec_base<F16vec8>,
                            public simd_numerics_operators<F16vec8>,
                            public scalar_numerics_operators<F16vec8>,
                            public roundscale_mixin<F16vec8>,
                            public vec_length_mixin<F16vec8>,
                            public compare_operator_mixin<F16vec8>,
                            public fpclass_mixin<F16vec8>,
                            public clamp_mixin<F16vec8>,
                            public load_store_mixin<F16vec8>,
                            public addsub_mixin<F16vec8> {
public:
  using vec_base<F16vec8>::vec_base;
  using simd_numerics_operators<F16vec8>::operator+=;
  using simd_numerics_operators<F16vec8>::operator-=;
  using simd_numerics_operators<F16vec8>::operator*=;
  using simd_numerics_operators<F16vec8>::operator/=;
  using scalar_numerics_operators<F16vec8>::operator+=;
  using scalar_numerics_operators<F16vec8>::operator-=;
  using scalar_numerics_operators<F16vec8>::operator*=;
  using scalar_numerics_operators<F16vec8>::operator/=;

  explicit constexpr F16vec8(_Float16 h) : vec_base((__m128h){h, h, h, h, h, h, h, h}) {}

  friend F16vec8 abs(const F16vec8 v) { return _mm_abs_ph(v); }
  friend F16vec8 rcp(const F16vec8 v) { return _mm_rcp_ph(v); }
  friend F16vec8 rsqrt(const F16vec8 v) { return _mm_rsqrt_ph(v); }
  friend F16vec8 sqrt(const F16vec8 v) { return _mm_sqrt_ph(v); }

  friend F16vec8 min(const F16vec8 lhs, const F16vec8 rhs) {
    return _mm_min_ph(lhs, rhs);
  }
  friend F16vec8 max(const F16vec8 lhs, const F16vec8 rhs) {
    return _mm_max_ph(lhs, rhs);
  }

  template <int OP> friend mask_type
  compare_mask(const F16vec8 lhs, const F16vec8 rhs, std::integral_constant<int, OP>) {
    return _mm_cmp_ph_mask(lhs, rhs, OP);
  }

  template <int OP> friend mask_type
  fpclassify_mask(const F16vec8 v, std::integral_constant<int, OP>) {
    return _mm_fpclass_ph_mask(v, OP);
  }

  template <int OP> friend F16vec8
  roundscale(const F16vec8 v, std::integral_constant<int, OP>) {
    return _mm_roundscale_ph(v, OP);
  }

  friend value_type add_horizontal(F16vec8 v) { return _mm_reduce_add_ph(v); }
  friend value_type mul_horizontal(F16vec8 v) { return _mm_reduce_mul_ph(v); }
  friend value_type min_horizontal(F16vec8 v) { return _mm_reduce_min_ph(v); }
  friend value_type max_horizontal(F16vec8 v) { return _mm_reduce_max_ph(v); }

  friend F16vec8 fma(F16vec8 lhs, F16vec8 rhs, F16vec8 acc) {
    return _mm_fmadd_ph(lhs, rhs, acc);
  }
  friend F16vec8 fmaddsub(F16vec8 lhs, F16vec8 rhs, F16vec8 acc) {
    return _mm_fmaddsub_ph(lhs, rhs, acc);
  }
  friend F16vec8 fmsubadd(F16vec8 lhs, F16vec8 rhs, F16vec8 acc) {
    return _mm_fmsubadd_ph(lhs, rhs, acc);
  }

  /// @name convert
  /// Perform a type conversion to the given element type. Each element in the
  /// source vector is converted to the equivalent value in the destination
  /// element type. Only conversions which result in the same number of output
  /// elements are allowed.
  ///@{
  // TODO: integer conversions should allow a rounding flag to be passed in to,
  // and map to cvt_roundph_epi16.
  friend Is16vec8 to_int16(F16vec8 v) { return _mm_cvtph_epi16(v); }
  friend Is32vec8 to_int32(F16vec8 v) { return _mm256_cvtph_epi32(v); }
  friend F32vec8 to_float(F16vec8 v) { return _mm256_cvtxph_ps(v); }
  ///@}
};

class F16vec16;

template <> struct vec_traits<F16vec16> {
  using builtin_type = __m256h;
  using value_type = _Float16;
  using mask_type = __mmask16;
  using bit_type = __m256i;
};

class EMPTY_BASES F16vec16 : public vec_base<F16vec16>,
                             public simd_numerics_operators<F16vec16>,
                             public scalar_numerics_operators<F16vec16>,
                             public roundscale_mixin<F16vec16>,
                             public vec_length_mixin<F16vec16>,
                             public compare_operator_mixin<F16vec16>,
                             public fpclass_mixin<F16vec16>,
                             public clamp_mixin<F16vec16>,
                             public load_store_mixin<F16vec16>,
                             public addsub_mixin<F16vec16> {
public:
  using vec_base<F16vec16>::vec_base;
  using simd_numerics_operators<F16vec16>::operator+=;
  using simd_numerics_operators<F16vec16>::operator-=;
  using simd_numerics_operators<F16vec16>::operator*=;
  using simd_numerics_operators<F16vec16>::operator/=;
  using scalar_numerics_operators<F16vec16>::operator+=;
  using scalar_numerics_operators<F16vec16>::operator-=;
  using scalar_numerics_operators<F16vec16>::operator*=;
  using scalar_numerics_operators<F16vec16>::operator/=;

  explicit constexpr F16vec16(_Float16 h) : vec_base((__m256h){h, h, h, h, h, h, h, h,
                                                               h, h, h, h, h, h, h, h}) {}

  friend F16vec16 abs(const F16vec16 v) { return _mm256_abs_ph(v); }
  friend F16vec16 rcp(const F16vec16 v) { return _mm256_rcp_ph(v); }
  friend F16vec16 rsqrt(const F16vec16 v) { return _mm256_rsqrt_ph(v); }
  friend F16vec16 sqrt(const F16vec16 v) { return _mm256_sqrt_ph(v); }
  friend F16vec16 min(const F16vec16 lhs, const F16vec16 rhs) {
    return _mm256_min_ph(lhs, rhs);
  }
  friend F16vec16 max(const F16vec16 lhs, const F16vec16 rhs) {
    return _mm256_max_ph(lhs, rhs);
  }

  template <int OP> friend mask_type
  compare_mask(const F16vec16 lhs, const F16vec16 rhs, std::integral_constant<int, OP>) {
    return _mm256_cmp_ph_mask(lhs, rhs, OP);
  }

  template <int OP> friend mask_type
  fpclassify_mask(const F16vec16 v, std::integral_constant<int, OP>) {
    return _mm256_fpclass_ph_mask(v, OP);
  }

  template <int OP> friend F16vec16
  roundscale(const F16vec16 v, std::integral_constant<int, OP>) {
    return _mm256_roundscale_ph(v, OP);
  }

  friend value_type add_horizontal(F16vec16 v) {
    return _mm256_reduce_add_ph(v);
  }
  friend value_type mul_horizontal(F16vec16 v) {
    return _mm256_reduce_mul_ph(v);
  }
  friend value_type min_horizontal(F16vec16 v) {
    return _mm256_reduce_min_ph(v);
  }
  friend value_type max_horizontal(F16vec16 v) {
    return _mm256_reduce_max_ph(v);
  }

  friend F16vec16 fma(F16vec16 lhs, F16vec16 rhs, F16vec16 acc) {
    return _mm256_fmadd_ph(lhs, rhs, acc);
  }
  friend F16vec16 fmaddsub(F16vec16 lhs, F16vec16 rhs, F16vec16 acc) {
    return _mm256_fmaddsub_ph(lhs, rhs, acc);
  }
  friend F16vec16 fmsubadd(F16vec16 lhs, F16vec16 rhs, F16vec16 acc) {
    return _mm256_fmsubadd_ph(lhs, rhs, acc);
  }

  /// @name convert
  /// Perform a type conversion to the given element type. Each element in the
  /// source vector is converted to the equivalent value in the destination
  /// element type. Only conversions which result in the same number of output
  /// elements are allowed.
  // TODO: integer conversions should allow a rounding flag to be passed in to,
  // and map to cvt_roundph_epi16.
  friend Is16vec16 to_int16(F16vec16 v) { return _mm256_cvtph_epi16(v); }
  friend Is32vec16 to_int32(F16vec16 v) { return _mm512_cvtph_epi32(v); }
  friend F32vec16 to_float(F16vec16 v) { return _mm512_cvtxph_ps(v); }
  ///@{
};

class F16vec32;

template <> struct vec_traits<F16vec32> {
  using builtin_type = __m512h;
  using value_type = _Float16;
  using mask_type = __mmask32;
  using bit_type = __m512i;
};

class EMPTY_BASES F16vec32 : public vec_base<F16vec32>,
                             public simd_numerics_operators<F16vec32>,
                             public scalar_numerics_operators<F16vec32>,
                             public roundscale_mixin<F16vec32>,
                             public vec_length_mixin<F16vec32>,
                             public compare_operator_mixin<F16vec32>,
                             public fpclass_mixin<F16vec32>,
                             public clamp_mixin<F16vec32>,
                             public load_store_mixin<F16vec32>,
                             public addsub_mixin<F16vec32> {
public:
  using vec_base<F16vec32>::vec_base;
  using simd_numerics_operators<F16vec32>::operator+=;
  using simd_numerics_operators<F16vec32>::operator-=;
  using simd_numerics_operators<F16vec32>::operator*=;
  using simd_numerics_operators<F16vec32>::operator/=;
  using scalar_numerics_operators<F16vec32>::operator+=;
  using scalar_numerics_operators<F16vec32>::operator-=;
  using scalar_numerics_operators<F16vec32>::operator*=;
  using scalar_numerics_operators<F16vec32>::operator/=;

  explicit constexpr F16vec32(_Float16 h) : vec_base((__m512h){h, h, h, h, h, h, h, h,
                                                               h, h, h, h, h, h, h, h,
                                                               h, h, h, h, h, h, h, h,
                                                               h, h, h, h, h, h, h, h}) {}

  friend F16vec32 abs(const F16vec32 v) { return _mm512_abs_ph(v); }
  friend F16vec32 rcp(const F16vec32 v) { return _mm512_rcp_ph(v); }
  friend F16vec32 rsqrt(const F16vec32 v) { return _mm512_rsqrt_ph(v); }
  friend F16vec32 sqrt(const F16vec32 v) { return _mm512_sqrt_ph(v); }

  friend F16vec32 min(const F16vec32 lhs, const F16vec32 rhs) {
    return _mm512_min_ph(lhs, rhs);
  }
  friend F16vec32 max(const F16vec32 lhs, const F16vec32 rhs) {
    return _mm512_max_ph(lhs, rhs);
  }

  template <int OP> friend mask_type
  compare_mask(const F16vec32 lhs, const F16vec32 rhs, std::integral_constant<int, OP>) {
    return _mm512_cmp_ph_mask(lhs, rhs, OP);
  }

  template <int OP> friend mask_type
  fpclassify_mask(const F16vec32 v, std::integral_constant<int, OP>) {
    return _mm512_fpclass_ph_mask(v, OP);
  }

  template <int OP> friend F16vec32
  roundscale(const F16vec32 v, std::integral_constant<int, OP>) {
    return _mm512_roundscale_ph(v, OP);
  }

  friend value_type add_horizontal(F16vec32 v) {
    return _mm512_reduce_add_ph(v);
  }
  friend value_type mul_horizontal(F16vec32 v) {
    return _mm512_reduce_mul_ph(v);
  }
  friend value_type min_horizontal(F16vec32 v) {
    return _mm512_reduce_min_ph(v);
  }
  friend value_type max_horizontal(F16vec32 v) {
    return _mm512_reduce_max_ph(v);
  }

  friend F16vec32 fma(F16vec32 lhs, F16vec32 rhs, F16vec32 acc) {
    return _mm512_fmadd_ph(lhs, rhs, acc);
  }
  friend F16vec32 fmaddsub(F16vec32 lhs, F16vec32 rhs, F16vec32 acc) {
    return _mm512_fmaddsub_ph(lhs, rhs, acc);
  }
  friend F16vec32 fmsubadd(F16vec32 lhs, F16vec32 rhs, F16vec32 acc) {
    return _mm512_fmsubadd_ph(lhs, rhs, acc);
  }

  /// @name convert
  /// Perform a type conversion to the given element type. Each element in the
  /// source vector is converted to the equivalent value in the destination
  /// element type. Only conversions which result in the same number of output
  /// elements are allowed.
  ///@{
  // TODO: integer conversions should allow a rounding flag to be passed in to,
  // and map to cvt_roundph_epi16.
  friend Is16vec32 to_int16(F16vec32 v) { return _mm512_cvtph_epi16(v); }
  ///@}
};
#endif // defined(__AVX512FP16__)

/// Allow the C++ tuple interface to be used on vec types.
namespace std {
#if defined(__AVX512FP16__)
template <> struct tuple_size<F16vec8> : public integral_constant<size_t, 8> {};
template <>
struct tuple_size<F16vec16> : public integral_constant<size_t, 16> {};
template <>
struct tuple_size<F16vec32> : public integral_constant<size_t, 32> {};
#endif // defined(__AVX512FP16__)
template <> struct tuple_size<F32vec4> : public integral_constant<size_t, 4> {};
template <> struct tuple_size<F32vec8> : public integral_constant<size_t, 8> {};
template <>
struct tuple_size<F32vec16> : public integral_constant<size_t, 16> {};
} // namespace std

template <> struct vec_traits<F32vec4> {
  using builtin_type = __m128;
  using value_type = float;
  using bit_type = __m128i;
  using mask_type = __mmask8; // :TODO: Not in AVX2 compiler mode.
};

template <> struct vec_traits<F32vec8> {
  using builtin_type = __m256;
  using value_type = float;
  using bit_type = __m256i;
  using mask_type = __mmask8; // :TODO: Not in AVX2 compiler mode.
};

template <> struct vec_traits<F32vec16> {
  using builtin_type = __m512;
  using value_type = float;
  using bit_type = __m512i;
  using mask_type = __mmask16;
};

/// Provide bonus features for a few of the original vec classes.
// TODO: Put API belows to classes in old i/f/dvec.h.
inline F32vec8 blend(__mmask8 m, F32vec8 value_if_0, F32vec8 value_if_1) {
  return _mm256_mask_blend_ps(m, value_if_0, value_if_1);
}
inline F32vec16 blend(__mmask16 m, F32vec16 value_if_0, F32vec16 value_if_1) {
  return _mm512_mask_blend_ps(m, value_if_0, value_if_1);
}

inline F32vec8 fmaddsub(F32vec8 lhs, F32vec8 rhs, F32vec8 acc) {
  return _mm256_fmaddsub_ps(lhs, rhs, acc);
}
inline F32vec16 fmaddsub(F32vec16 lhs, F32vec16 rhs, F32vec16 acc) {
  return _mm512_fmaddsub_ps(lhs, rhs, acc);
}

inline F32vec8 fmsubadd(F32vec8 lhs, F32vec8 rhs, F32vec8 acc) {
  return _mm256_fmsubadd_ps(lhs, rhs, acc);
}
inline F32vec16 fmsubadd(F32vec16 lhs, F32vec16 rhs, F32vec16 acc) {
  return _mm512_fmsubadd_ps(lhs, rhs, acc);
}

inline F32vec16 addsub(F32vec16 lhs, F32vec16 rhs) {
  return _mm512_fmaddsub_ps(lhs, F32vec16(1.0f), rhs);
}
inline F32vec16 subadd(F32vec16 lhs, F32vec16 rhs) {
  return _mm512_fmsubadd_ps(lhs, F32vec16(1.0f), rhs);
}

#if defined(__AVX512FP16__)
/// @name convert_old_vec_to_FP16_vec
/// Provide extra conversion functions to allow older vec types to be turned
/// into the new types.
///@{
// TODO: Put API belows to classes in old i/f/dvec.h.
inline F16vec8 to_float16(Is16vec8 v) { return _mm_cvtepi16_ph(v); }
inline F16vec16 to_float16(Is16vec16 v) { return _mm256_cvtepi16_ph(v); }
inline F16vec32 to_float16(Is16vec32 v) { return _mm512_cvtepi16_ph(v); }

inline F16vec8 to_float16(Is32vec8 v) { return _mm256_cvtepi32_ph(v); }
inline F16vec16 to_float16(Is32vec16 v) { return _mm512_cvtepi32_ph(v); }

inline F16vec8 to_float16(F32vec8 v) { return _mm256_cvtxps_ph(v); }
inline F16vec16 to_float16(F32vec16 v) { return _mm512_cvtxps_ph(v); }
///@}

/// @name length_api
/// Provide some of the length-related API calls to older vec types.
inline float dot(F32vec4 lhs, F32vec4 rhs) { return lhs.dot(rhs); }
inline float dot(F32vec8 lhs, F32vec8 rhs) { return lhs.dot(rhs); }
inline float dot(F32vec16 lhs, F32vec16 rhs) { return lhs.dot(rhs); }
///@}

#endif // defined(__AVX512FP16__)

#if defined(__AVX512F__)
template <int OP> inline __mmask8
compare_mask(const F32vec8 lhs, const F32vec8 rhs,
             std::integral_constant<int, OP>) {
  return _mm256_cmp_ps_mask(lhs, rhs, OP);
}

template <int OP> inline __mmask16
compare_mask(const F32vec16 lhs, const F32vec16 rhs,
             std::integral_constant<int, OP>) {
  return _mm512_cmp_ps_mask(lhs, rhs, OP);
}

template <int OP> inline F32vec8
roundscale(const F32vec8 v, std::integral_constant<int, OP>) {
  return _mm256_roundscale_ps(v, OP);
}
template <int OP> inline F32vec16
roundscale(const F32vec16 v, std::integral_constant<int, OP>) {
  return _mm512_roundscale_ps(v, OP);
}
#endif // defined(__AVX512F__)

#endif
