//===-- vec_complex.h - vector classes of complex number --*- C++ -*--===//
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

#include <hvec.h>
#include <complex>
#include <cstdint>

// Forward decls to avoid some ordering dependencies that would arise.
#if defined(__AVX512FP16__)
class CF16vec4;
class CF16vec8;
class CF16vec16;
#endif // defined(__AVX512FP16__)
class CF32vec4;
class CF32vec8;

namespace intrinsic_overloads {
/// Byte-indexes required to duplicate the 2 lower or 2 upper bytes of a 4-byte
/// block. Note that the array is big enough to do any dup operation up to
/// 512-bit wide.
alignas(64) constexpr uint8_t k_dupReal16bit[64] = {
    0, 1, 0, 1, 4, 5, 4, 5, 8, 9, 8, 9, 12, 13, 12, 13,
    0, 1, 0, 1, 4, 5, 4, 5, 8, 9, 8, 9, 12, 13, 12, 13,
    0, 1, 0, 1, 4, 5, 4, 5, 8, 9, 8, 9, 12, 13, 12, 13,
    0, 1, 0, 1, 4, 5, 4, 5, 8, 9, 8, 9, 12, 13, 12, 13,
};
alignas(64) constexpr uint8_t k_dupImag16bit[64] = {
    2, 3, 2, 3, 6, 7, 6, 7, 10, 11, 10, 11, 14, 15, 14, 15,
    2, 3, 2, 3, 6, 7, 6, 7, 10, 11, 10, 11, 14, 15, 14, 15,
    2, 3, 2, 3, 6, 7, 6, 7, 10, 11, 10, 11, 14, 15, 14, 15,
    2, 3, 2, 3, 6, 7, 6, 7, 10, 11, 10, 11, 14, 15, 14, 15,
};

/// Perform a duplication of the lower bits in each pair of the given size. For
/// epi16 there is no instruction to do this very easily, other than a general
/// purpose shuffle.
template <int NUM_BITS, typename T> T ldup(T v);
template <> inline __m128i ldup<16>(__m128i v) {
  return _mm_shuffle_epi8(v, *(const __m128i *)k_dupReal16bit);
}
template <> inline __m256i ldup<16>(__m256i v) {
  return _mm256_shuffle_epi8(v, *(const __m256i *)k_dupReal16bit);
}
template <> inline __m512i ldup<16>(__m512i v) {
  return _mm512_shuffle_epi8(v, *(const __m512i *)k_dupReal16bit);
}
template <> inline __m256i ldup<32>(__m256i v) {
  return _mm256_castps_si256(_mm256_moveldup_ps(_mm256_castsi256_ps(v)));
}
template <> inline __m512i ldup<32>(__m512i v) {
  return _mm512_castps_si512(_mm512_moveldup_ps(_mm512_castsi512_ps(v)));
}

/// Perform a duplication of the upper bytes in each pair of the given size. For
/// epi16 there is no instruction to do this very easily, other than a general
/// purpose shuffle.
template <int NUM_BITS, typename T> T hdup(T v);
template <> inline __m128i hdup<16>(__m128i v) {
  return _mm_shuffle_epi8(v, *(const __m128i *)k_dupImag16bit);
}
template <> inline __m256i hdup<16>(__m256i v) {
  return _mm256_shuffle_epi8(v, *(const __m256i *)k_dupImag16bit);
}
template <> inline __m512i hdup<16>(__m512i v) {
  return _mm512_shuffle_epi8(v, *(const __m512i *)k_dupImag16bit);
}
template <> inline __m256i hdup<32>(__m256i v) {
  return _mm256_castps_si256(_mm256_movehdup_ps(_mm256_castsi256_ps(v)));
}
template <> inline __m512i hdup<32>(__m512i v) {
  return _mm512_castps_si512(_mm512_movehdup_ps(_mm512_castsi512_ps(v)));
}

/// Given a set of mask bits, combine the two bits from each pair into a single
/// bit by ANDing them together.
inline __mmask8 reduce_bit_pair_and(__mmask8 m) {
  return _mm_cmp_epi16_mask(_mm_movm_epi8(m), _mm_set1_epi16(-1),
                            _MM_CMPINT_EQ);
}
inline __mmask8 reduce_bit_pair_and(__mmask16 m) {
  const auto wholeElements = _mm_movm_epi8(m);
  return _mm_cmp_epi16_mask(wholeElements, _mm_set1_epi16(-1), _MM_CMPINT_EQ);
}
inline __mmask16 reduce_bit_pair_and(__mmask32 m) {
  const auto wholeElements = _mm256_movm_epi8(m);
  return _mm256_cmp_epi16_mask(wholeElements, _mm256_set1_epi16(-1),
                               _MM_CMPINT_EQ);
}

/// Given a set of mask bits, combine the two bits from each pair into a single
/// bit by ORing them together.
inline __mmask8 reduce_bit_pair_or(__mmask8 m) {
  return _mm_cmp_epi16_mask(_mm_movm_epi8(m), __m128i(), _MM_CMPINT_NE);
}
inline __mmask8 reduce_bit_pair_or(__mmask16 m) {
  return _mm_cmp_epi16_mask(_mm_movm_epi8(m), __m128i(), _MM_CMPINT_NE);
}
inline __mmask16 reduce_bit_pair_or(__mmask32 m) {
  return _mm256_cmp_epi16_mask(_mm256_movm_epi8(m), __m256i(), _MM_CMPINT_NE);
}

} // namespace intrinsic_overloads

/// Barton-Nackman mixin for generating various complex operations which match
/// the std::complex API as closely as possible.
template <typename CVEC> struct complex_base : public vec_base<CVEC> {

public:
  /// The type of the individual complex elements.
  using value_type = typename vec_traits<CVEC>::value_type;
  using mask_type = typename vec_traits<CVEC>::mask_type;

  /// A SIMD type containing the same number of bits as this complex type, but
  /// organised into individual elements. For example, CF16vec16 would have a
  /// container type of F16vec32 (i.e., F16, but twice as many elements). This
  /// container type allows operations like blend, add, mul, and so on to be
  /// performed directly on the elements, which aids the construction of complex
  /// operations.
  using vec_container_type = typename vec_traits<CVEC>::vec_container_type;

  /// The underlying native built-in type (e.g., __m512h for CF16vec16).
  using builtin_type = typename vec_base<CVEC>::builtin_type;

protected:
  using vec_size = std::integral_constant<std::size_t, sizeof(builtin_type) /
                                                           sizeof(value_type)>;

public:
  complex_base() = default;

  /// Initialise a complex value directly from an underlying builtin vector
  /// container.
  complex_base(typename vec_base<CVEC>::builtin_type v) : vec_base<CVEC>(v) {}

  /// Build a vector by filling every element with the same complex value.
  explicit constexpr complex_base(std::complex<typename value_type::value_type> c)
      : vec_base<CVEC>(
            intrinsic_overloads::broadcast<typename vec_base<CVEC>::bit_type>(
                __builtin_bit_cast(int32_t, c))) {}

  /// Initialise a complex value from a pair of real/imaginary values. If only a
  /// real value is provided then the imaginary component will be initialised to
  /// zero.
  complex_base(typename value_type::value_type r,
               typename value_type::value_type i = {}) {
    // Create full vectors of the broadcast real/imag values, and then blend
    // real/imag together to form the interleaved complex values.
    const vec_container_type rv(r);
    const vec_container_type iv(i);
    this->simd = blend(vec_traits<CVEC>::real_mask::value, iv, rv);
  }

  /// Build a SIMD vector from a set of complex values. If insufficient values
  /// are provided the remaining values will be zeroed.
  complex_base(std::initializer_list<value_type> init) {
    const mask_type n_bits = static_cast<mask_type>((1ULL << init.size()) - 1);
    this->simd = intrinsic_overloads::mask_load<vec_size::value>(
        CVEC().to_bits(), n_bits, init.begin());
  }

  /// Return a SIMD containing only the real values.
  friend auto real(CVEC value) { return value.real(); }

  /// Return a SIMD containing only the imaginary values.
  friend auto imag(CVEC value) { return value.imag(); }

  /// Equality operator.

  // Note that it is tempting to use a 32-bit comparisons instead which would check for
  // bit-wise equality but wouldn't handle all the special cases (+-0, nan, etc.). Instead, perform
  // conventional comparisons on the individual elements and then combine adjacant mask elements.
  // :TODO: Eventually this should call operator==/!= for the actual container type, but this doesn't
  // work yet because compare_mask is returning the wrong mask type in places (e.g., returning
  // an avx2 element-wise mask).
  typename vec_traits<CVEC>::mask_type friend operator==(CVEC lhs, CVEC rhs) {
    const auto cmpMask = compare_mask(vec_container_type(lhs), vec_container_type(rhs),
                                      std::integral_constant<int, _CMP_EQ_OQ>());
    return intrinsic_overloads::reduce_bit_pair_and(cmpMask);
  }
  typename vec_traits<CVEC>::mask_type friend operator!=(CVEC lhs, CVEC rhs) {
    const auto cmpMask = compare_mask(vec_container_type(lhs), vec_container_type(rhs),
                                      std::integral_constant<int, _CMP_NEQ_OQ>());
    return intrinsic_overloads::reduce_bit_pair_or(cmpMask);
  }

  // Scalar variants of the equality operators.
  typename vec_traits<CVEC>::mask_type friend operator==(value_type lhs, CVEC rhs) {
    return CVEC(lhs) == rhs; }
  typename vec_traits<CVEC>::mask_type friend operator==(CVEC lhs, value_type rhs) {
    return lhs == CVEC(rhs); }
  typename vec_traits<CVEC>::mask_type friend operator!=(value_type lhs, CVEC rhs) {
    return CVEC(lhs) != rhs; }
  typename vec_traits<CVEC>::mask_type friend operator!=(CVEC lhs, value_type rhs) {
    return lhs != CVEC(rhs); }

  /// Copy the real values into the imaginary values.
  friend CVEC duplicateReal(CVEC v) {
    constexpr int k_bitsPerComplexElement = sizeof(value_type) * 8 / 2;
    return CVEC(
        intrinsic_overloads::ldup<k_bitsPerComplexElement>(v.to_bits()));
  }

  /// Duplicate the imaginary values into the imaginary values.
  friend CVEC duplicateImag(CVEC v) {
    constexpr int k_bitsPerComplexElement = sizeof(value_type) * 8 / 2;
    return CVEC(
        intrinsic_overloads::hdup<k_bitsPerComplexElement>(v.to_bits()));
  }

  // Roundscale passes through to underlying container.
  friend CVEC ceil(CVEC value) { return (CVEC)ceil(vec_container_type(value)); }
  friend CVEC floor(CVEC value) { return (CVEC)floor(vec_container_type(value)); }
  friend CVEC trunc(CVEC value) { return (CVEC)trunc(vec_container_type(value)); }
  friend CVEC round(CVEC value) { return (CVEC)round(vec_container_type(value)); }

  /// Return the square of the magnitude of the complex value.
  friend auto norm(CVEC value) { return mulconj(value, value).real(); }
};

/// Barton-Nackman style mixin to endow a complex class with arithmetic
/// operators.  Conventional SIMD-like operations are allowed, as well as mixed
/// SIMD/real-valued-scalar operations. In all cases the operations are deferred to members
/// of the derived class which should provide efficient ways to implement them.
///
/// Note that overloads are provided for mixed SIMD/real-valued-scalar operations. Without
/// the overloads the real-valued type would be promoted to a CVEC using the
/// CVEC(real,imag) constructor, and then a full complex multiply would be
/// performed with the constructed value. This would be inefficient compared to
/// multiplying every element of the complex number by the scalar value.
///
/// Many of the operations defined here are deferred to their non-complex
/// counterparts, with suitable masks and blends.
template <typename CVEC> struct complex_numerics_operators {
private:
  CVEC &derived() { return *static_cast<CVEC *>(this); }
  const CVEC &derived() const { return *static_cast<const CVEC *>(this); }

protected:
  using container = typename vec_traits<CVEC>::vec_container_type;

public:
  /// Define the type of the underlying real values. For example, a CF16vec16
  /// would be _Float16.
  using real_scalar_type = typename vec_traits<CVEC>::value_type::value_type;

private:
  /// Utility function to build a division from the other operations. This isn't
  /// fast, but if the algorithm calling this cares about speed than
  /// alternatives to division should be used anyway.
  static CVEC doDivision(CVEC lhs, CVEC rhs) {
    const auto lhsByConjRhs = lhs * conj(rhs);
    const auto rhsByConjRhs = rhs * conj(rhs); // Real-valued only.

    // rhs*conj(rhs) will be a real-value only, so divide each element on the
    // left by both elements on the right.
    const auto dupReal = container(duplicateReal(rhsByConjRhs));

    return CVEC(container(lhsByConjRhs) / dupReal);
  }

public:
  /// @name complex_operators
  ///@{

  CVEC &operator+=(CVEC rhs) {
    auto &t = derived();
    t = CVEC(container(t) + container(rhs));
    return t;
  }
  CVEC &operator-=(CVEC rhs) {
    auto &t = derived();
    t = CVEC(container(t) - container(rhs));
    return t;
  }
  CVEC &operator*=(CVEC rhs) {
    auto &t = derived();
    t = t * rhs; // Invoke the appropriate complex multipler.
    return t;
  }
  CVEC &operator/=(CVEC rhs) {
    auto &t = derived();
    t = doDivision(t, rhs);
    return t;
  }

  CVEC operator-() const {
    return CVEC() - derived();
  }
  CVEC operator+() const { return derived(); }

  friend CVEC operator+(CVEC lhs, CVEC rhs) {
    return CVEC(container(lhs) + container(rhs));
  }
  friend CVEC operator-(CVEC lhs, CVEC rhs) {
    return CVEC(container(lhs) - container(rhs));
  }
  friend CVEC operator/(CVEC lhs, CVEC rhs) { return doDivision(lhs, rhs); }
  ///@}

private:
  /// @name complex_scalar_operators
  /// Scalar operators can be implemented more efficiently than by calling the
  /// native instructions on full complex values.
  ///@{
  static CVEC realScalarPlus(CVEC lhs, real_scalar_type rhs) {
    return CVEC(blend(vec_traits<CVEC>::real_mask::value, container(lhs),
                      container(lhs) + container(rhs)));
  }

  static CVEC realScalarMinus(CVEC lhs, real_scalar_type rhs) {
    return CVEC(blend(vec_traits<CVEC>::real_mask::value, container(lhs),
                      container(lhs) - container(rhs)));
  }

  static CVEC realScalarMultiplies(CVEC lhs, real_scalar_type rhs) {
    return CVEC(container(lhs) * container(rhs));
  }

  static CVEC realScalarDivision(CVEC lhs, real_scalar_type rhs) {
    return CVEC(container(lhs) / container(rhs));
  }

public:
  CVEC &operator+=(real_scalar_type rhs) {
    auto &t = derived();
    t = realScalarPlus(t, rhs);
    return t;
  }
  CVEC &operator-=(real_scalar_type rhs) {
    auto &t = derived();
    t = realScalarMinus(t, rhs);
    return t;
  }
  CVEC &operator*=(real_scalar_type rhs) {
    auto &t = derived();
    t = realScalarMultiplies(t, rhs);
    return t;
  }
  CVEC &operator/=(real_scalar_type rhs) {
    auto &t = derived();
    t = realScalarDivision(t, rhs);
    return t;
  }

  friend CVEC operator+(CVEC lhs, real_scalar_type rhs) {
    return realScalarPlus(lhs, rhs);
  }
  friend CVEC operator+(real_scalar_type lhs, CVEC rhs) {
    return realScalarPlus(rhs, lhs);
  }

  friend CVEC operator-(CVEC lhs, real_scalar_type rhs) {
    return realScalarMinus(lhs, rhs);
  }
  friend CVEC operator-(real_scalar_type lhs, CVEC rhs) {
    return CVEC(lhs) - rhs;
  }

  friend CVEC operator*(CVEC lhs, real_scalar_type rhs) {
    return realScalarMultiplies(lhs, rhs);
  }
  friend CVEC operator*(real_scalar_type lhs, CVEC rhs) {
    return realScalarMultiplies(rhs, lhs);
  }

  friend CVEC operator/(CVEC lhs, real_scalar_type rhs) {
    return realScalarDivision(lhs, rhs);
  }
  ///@}
};

/// Horizontal operations in complex values are a little bit more complicated as
/// they reduce to a pair of values, not just one.
template <typename CVEC> struct complex_horizontal_vec {
  friend typename vec_traits<CVEC>::value_type add_horizontal(CVEC v) {
    // The real/imag values must be summed separately.
    // :TODO: It would be more efficient to use the same reduction tree as for
    // reduce_add, but miss out the final step which sums the two half-precision
    // values, and leave them as a pair.
    using container = typename vec_traits<CVEC>::vec_container_type;
    const auto real =
        blend(vec_traits<CVEC>::real_mask::value, container(), container(v));
    const auto imag =
        blend(vec_traits<CVEC>::imag_mask::value, container(), container(v));
    return {add_horizontal(real), add_horizontal(imag)};
  }
};

/// Synthesise FMA operations from the underlying ISA when native complex isn't
/// supported.
template <typename CVEC> struct cfma_synthesise_mixin {
  /// FMA type operation.
  friend CVEC fma(CVEC lhs, CVEC rhs, CVEC acc) {
    using container = typename vec_traits<CVEC>::vec_container_type;

    const auto dupLhsR = container(duplicateReal(lhs));
    const auto dupLhsI = container(duplicateImag(lhs));
    const auto swapRhs = container(swapRealImag(rhs));

    const auto tmp = fmaddsub(dupLhsI, swapRhs, container(acc));
    return CVEC(fmaddsub(dupLhsR, container(rhs), tmp));
  }

  /// Multiply a complex value by the conjugate of another complex value, adding
  /// the result to the accumulator value.
  friend CVEC fmaconj(CVEC lhs, CVEC rhs, CVEC acc) {
    // This could be implemented in the same way as fma but reversing the sense
    // of the addsub to be subadd instead, but it's simpler and not particularly
    // slower, to do it this way instead.
    return fma(lhs, conj(rhs), acc);
  }

  /// Plain complex multiplies, synthesised from underlying SIMD operations.
  friend CVEC operator*(CVEC lhs, CVEC rhs) { return fma(lhs, rhs, CVEC()); }

  /// Multiply a complex value by the conjugate of a second complex value.
  friend CVEC mulconj(CVEC lhs, CVEC rhs) { return lhs * conj(rhs); }
};

#if defined(__AVX512FP16__)
template<>
struct vec_traits<CF16vec4> {
  using builtin_type = __m128h;
  using value_type = std::complex<_Float16>;
  using mask_type = __mmask8;
  using bit_type = __m128i;
  using vec_container_type = F16vec8;

  /// Bit-pattern to allow real/imag to be easily masked.
  using real_mask = std::integral_constant <__mmask16, 0x55>;
  using imag_mask = std::integral_constant <__mmask16, 0xAA>;
};

class EMPTY_BASES CF16vec4 :
  public complex_base<CF16vec4>,
  public complex_numerics_operators<CF16vec4>,
  public scalar_numerics_operators<CF16vec4>,
  public load_store_mixin<CF16vec4>,
  public complex_horizontal_vec<CF16vec4>
{
public:

  using complex_base<CF16vec4>::complex_base; // Inherit the complex constructors.
  using complex_numerics_operators<CF16vec4>::operator+=;
  using complex_numerics_operators<CF16vec4>::operator-=;
  using complex_numerics_operators<CF16vec4>::operator*=;
  using complex_numerics_operators<CF16vec4>::operator/=;
  using scalar_numerics_operators<CF16vec4>::operator+=;
  using scalar_numerics_operators<CF16vec4>::operator-=;
  using scalar_numerics_operators<CF16vec4>::operator*=;
  using scalar_numerics_operators<CF16vec4>::operator/=;

  F16vec8 real() const { return F16vec8(_mm_cvtepi32_epi16(to_bits())); }
  F16vec8 imag() const { return F16vec8(_mm_cvtepi32_epi16(_mm_srli_epi32(to_bits(), 16))); }

  /// Set the real values to the given SIMD elements. For 16-bit values this is faster than
  /// using a permutex2var.
  void real(F16vec8 value) {
    const vec_container_type rv(_mm_cvtepu16_epi32(value.to_bits()));
    const vec_container_type iv(*this);
    this->simd = blend(vec_traits<CF16vec4>::real_mask::value, iv, rv);
  }

  /// Set the imaginary values to the given SIMD elements.
  void imag(F16vec8 value) {
    const vec_container_type rv(*this);
    const vec_container_type iv(_mm_slli_epi32(_mm_cvtepi16_epi32(value.to_bits()), 16));
    this->simd = blend(vec_traits<CF16vec4>::real_mask::value, iv, rv);
  }

  /// Directly map some methods to various intrisics.
  friend CF16vec4 operator*(CF16vec4 lhs, CF16vec4 rhs) {
    return _mm_fmadd_pch(lhs, rhs, __m128h()); // Replace by multiply when LLVM handles it properly.
  }
  friend CF16vec4 fma(CF16vec4 lhs, CF16vec4 rhs, CF16vec4 acc) {
    return _mm_fmadd_pch(lhs, rhs, acc);
  }
  friend CF16vec4 fmaconj(CF16vec4 lhs, CF16vec4 rhs, CF16vec4 acc) {
    return _mm_fcmadd_pch(lhs, rhs, acc);
  }
  friend CF16vec4 mulconj(CF16vec4 lhs, CF16vec4 rhs) { return _mm_fcmadd_pch(lhs, rhs, __m128h()); }
  friend CF16vec4 conj(CF16vec4 value) { return _mm_conj_pch(value); }
  friend CF16vec4 swapRealImag(CF16vec4 value) {
    return CF16vec4(_mm_ror_epi32(value.to_bits(), 16));
  }
};

template <> struct vec_traits<CF16vec8> {
  using builtin_type = __m256h;
  using value_type = std::complex<_Float16>;
  using mask_type = __mmask8;
  using bit_type = __m256i;
  using vec_container_type = F16vec16;

  /// Bit-pattern to allow real/imag to be easily masked.
  using real_mask = std::integral_constant<__mmask16, 0x5555>;
  using imag_mask = std::integral_constant<__mmask16, 0xAAAA>;
};

class EMPTY_BASES CF16vec8 : public complex_base<CF16vec8>,
                             public complex_numerics_operators<CF16vec8>,
                             public scalar_numerics_operators<CF16vec8>,
                             public load_store_mixin<CF16vec8>,
                             public complex_horizontal_vec<CF16vec8> {
public:
  using complex_base<CF16vec8>::complex_base; // Inherit the complex
                                              // constructors.
  using complex_numerics_operators<CF16vec8>::operator+=;
  using complex_numerics_operators<CF16vec8>::operator-=;
  using complex_numerics_operators<CF16vec8>::operator*=;
  using complex_numerics_operators<CF16vec8>::operator/=;
  using scalar_numerics_operators<CF16vec8>::operator+=;
  using scalar_numerics_operators<CF16vec8>::operator-=;
  using scalar_numerics_operators<CF16vec8>::operator*=;
  using scalar_numerics_operators<CF16vec8>::operator/=;

  F16vec8 real() const { return F16vec8(_mm256_cvtepi32_epi16(to_bits())); }
  F16vec8 imag() const {
    return F16vec8(_mm256_cvtepi32_epi16(_mm256_srli_epi32(to_bits(), 16)));
  }

  /// Set the real values to the given SIMD elements. For 16-bit values this is
  /// faster than using a permutex2var.
  void real(F16vec8 value) {
    const vec_container_type rv(_mm256_cvtepu16_epi32(value.to_bits()));
    const vec_container_type iv(*this);
    this->simd = blend(vec_traits<CF16vec8>::real_mask::value, iv, rv);
  }

  /// Set the imaginary values to the given SIMD elements.
  void imag(F16vec8 value) {
    const vec_container_type rv(*this);
    const vec_container_type iv(
        _mm256_slli_epi32(_mm256_cvtepi16_epi32(value.to_bits()), 16));
    this->simd = blend(vec_traits<CF16vec8>::real_mask::value, iv, rv);
  }

  /// Directly map some methods to various intrisics.
  friend CF16vec8 operator*(CF16vec8 lhs, CF16vec8 rhs) {
    return _mm256_fmadd_pch(lhs, rhs, __m256h()); // Replace by mul when LLVM handles it properly.
  }
  friend CF16vec8 fma(CF16vec8 lhs, CF16vec8 rhs, CF16vec8 acc) {
    return _mm256_fmadd_pch(lhs, rhs, acc);
  }
  friend CF16vec8 fmaconj(CF16vec8 lhs, CF16vec8 rhs, CF16vec8 acc) {
    return _mm256_fcmadd_pch(lhs, rhs, acc);
  }
  friend CF16vec8 mulconj(CF16vec8 lhs, CF16vec8 rhs) {
    return _mm256_fcmadd_pch(lhs, rhs, __m256h());
  }
  friend CF16vec8 conj(CF16vec8 value) { return _mm256_conj_pch(value); }
  friend CF16vec8 swapRealImag(CF16vec8 value) {
    return CF16vec8(_mm256_ror_epi32(value.to_bits(), 16));
  }
};

template <> struct vec_traits<CF16vec16> {
  using builtin_type = __m512h;
  using value_type = std::complex<_Float16>;
  using mask_type = __mmask16;
  using bit_type = __m512i;
  using vec_container_type = F16vec32;

  /// Bit-pattern to allow real/imag to be easily masked.
  using real_mask = std::integral_constant<__mmask32, 0x55555555>;
  using imag_mask = std::integral_constant<__mmask32, 0xAAAAAAAA>;
};

class EMPTY_BASES CF16vec16 : public complex_base<CF16vec16>,
                              public complex_numerics_operators<CF16vec16>,
                              public scalar_numerics_operators<CF16vec16>,
                              public load_store_mixin<CF16vec16>,
                              public complex_horizontal_vec<CF16vec16> {
public:
  using complex_base<CF16vec16>::complex_base; // Inherit the complex
                                               // constructors.
  using complex_numerics_operators<CF16vec16>::operator+=;
  using complex_numerics_operators<CF16vec16>::operator-=;
  using complex_numerics_operators<CF16vec16>::operator*=;
  using complex_numerics_operators<CF16vec16>::operator/=;
  using scalar_numerics_operators<CF16vec16>::operator+=;
  using scalar_numerics_operators<CF16vec16>::operator-=;
  using scalar_numerics_operators<CF16vec16>::operator*=;
  using scalar_numerics_operators<CF16vec16>::operator/=;

  F16vec16 real() const { return F16vec16(_mm512_cvtepi32_epi16(to_bits())); }
  F16vec16 imag() const {
    return F16vec16(_mm512_cvtepi32_epi16(_mm512_srli_epi32(to_bits(), 16)));
  }

  /// Set the real values to the given SIMD elements. For 16-bit values this is
  /// faster than using a permutex2var.
  void real(F16vec16 value) {
    const vec_container_type rv(_mm512_cvtepu16_epi32(value.to_bits()));
    const vec_container_type iv(*this);
    this->simd = blend(vec_traits<CF16vec16>::real_mask::value, iv, rv);
  }

  /// Set the imaginary values to the given SIMD elements.
  void imag(F16vec16 value) {
    const vec_container_type rv(*this);
    const vec_container_type iv(
        _mm512_slli_epi32(_mm512_cvtepi16_epi32(value.to_bits()), 16));
    this->simd = blend(vec_traits<CF16vec16>::real_mask::value, iv, rv);
  }

  /// Directly map various methods to intrinsics.
  friend CF16vec16 operator*(CF16vec16 lhs, CF16vec16 rhs) {
    return _mm512_fmadd_pch(lhs, rhs, __m512h()); // Replace by multiply when LLVM handles it properly.
  }
  friend CF16vec16 fma(CF16vec16 lhs, CF16vec16 rhs, CF16vec16 acc) {
    return _mm512_fmadd_pch(lhs, rhs, acc);
  }
  friend CF16vec16 fmaconj(CF16vec16 lhs, CF16vec16 rhs, CF16vec16 acc) {
    return _mm512_fcmadd_pch(lhs, rhs, acc);
  }
  friend CF16vec16 mulconj(CF16vec16 lhs, CF16vec16 rhs) {
    return _mm512_fcmadd_pch(lhs, rhs, __m512h());
  }
  friend CF16vec16 conj(CF16vec16 value) { return _mm512_conj_pch(value); }
  friend CF16vec16 swapRealImag(const CF16vec16 a) {
    return CF16vec16(_mm512_ror_epi32(a.to_bits(), 16));
  }
};
#endif // defined(__AVX512FP16__)

template <> struct vec_traits<CF32vec4> {
  using builtin_type = __m256;
  using value_type = std::complex<float>;
  using mask_type = __mmask8;
  using bit_type = __m256i;
  using vec_container_type = F32vec8;

  /// Bit-pattern to allow real/imag to be easily masked.
  using real_mask = std::integral_constant<__mmask8, 0x55>;
  using imag_mask = std::integral_constant<__mmask8, 0xAA>;
};

class EMPTY_BASES CF32vec4 : public complex_base<CF32vec4>,
                             public complex_numerics_operators<CF32vec4>,
                             public scalar_numerics_operators<CF32vec4>,
                             public load_store_mixin<CF32vec4>,
                             public complex_horizontal_vec<CF32vec4>,
                             public cfma_synthesise_mixin<CF32vec4> {
public:
  using complex_base<CF32vec4>::complex_base;
  using complex_numerics_operators<CF32vec4>::operator+=;
  using complex_numerics_operators<CF32vec4>::operator-=;
  using complex_numerics_operators<CF32vec4>::operator*=;
  using complex_numerics_operators<CF32vec4>::operator/=;
  using scalar_numerics_operators<CF32vec4>::operator+=;
  using scalar_numerics_operators<CF32vec4>::operator-=;
  using scalar_numerics_operators<CF32vec4>::operator*=;
  using scalar_numerics_operators<CF32vec4>::operator/=;

  explicit CF32vec4(const std::complex<float>& e)
    : complex_base(_mm256_set1_epi64x(__builtin_bit_cast(int64_t, e))) {}

  F32vec4 real() const {
    return F32vec4(_mm_castsi128_ps(_mm256_cvtepi64_epi32(to_bits())));
  }
  F32vec4 imag() const {
    return F32vec4(_mm_castsi128_ps(
        _mm256_cvtepi64_epi32(_mm256_srli_epi64(to_bits(), 32))));
  }

  void real(F32vec4 value) {
    // Keep the odd indexes (imaginaries), and interleave with the consecutive
    // reals.
    const auto k_idx = _mm256_setr_epi32(8, 1, 9, 3, 10, 5, 11, 7);
    simd = _mm256_permutex2var_ps(simd, k_idx, _mm256_castps128_ps256(value));
  }
  void imag(F32vec4 value) {
    // Keep the even indexes (reals), and interleave with the consecutive
    // imaginaries.
    const auto k_idx = _mm256_setr_epi32(0, 8, 2, 9, 4, 10, 6, 11);
    simd = _mm256_permutex2var_ps(simd, k_idx, _mm256_castps128_ps256(value));
  }

  friend CF32vec4 conj(CF32vec4 value) {
    return _mm256_mask_xor_ps(value, 0xAA, _mm256_set1_ps(-0.0), value);
  }
  friend CF32vec4 swapRealImag(const CF32vec4 a) {
    return CF32vec4(_mm256_ror_epi64(a.to_bits(), 32));
  }
};

template <> struct vec_traits<CF32vec8> {
  using builtin_type = __m512;
  using value_type = std::complex<float>;
  using mask_type = __mmask8;
  using bit_type = __m512i;
  using vec_container_type = F32vec16;

  /// Bit-pattern to allow real/imag to be easily masked.
  using real_mask = std::integral_constant<__mmask16, 0x5555>;
  using imag_mask = std::integral_constant<__mmask16, 0xAAAA>;
};

class EMPTY_BASES CF32vec8 : public complex_base<CF32vec8>,
                             public complex_numerics_operators<CF32vec8>,
                             public scalar_numerics_operators<CF32vec8>,
                             public load_store_mixin<CF32vec8>,
                             public complex_horizontal_vec<CF32vec8>,
                             public cfma_synthesise_mixin<CF32vec8> {
public:
  using complex_base<CF32vec8>::complex_base;
  using complex_numerics_operators<CF32vec8>::operator+=;
  using complex_numerics_operators<CF32vec8>::operator-=;
  using complex_numerics_operators<CF32vec8>::operator*=;
  using complex_numerics_operators<CF32vec8>::operator/=;
  using scalar_numerics_operators<CF32vec8>::operator+=;
  using scalar_numerics_operators<CF32vec8>::operator-=;
  using scalar_numerics_operators<CF32vec8>::operator*=;
  using scalar_numerics_operators<CF32vec8>::operator/=;

  explicit CF32vec8(const std::complex<float>& e)
   : complex_base(_mm512_set1_epi64(__builtin_bit_cast(int64_t, e))) {}

  F32vec8 real() const {
    return F32vec8(_mm256_castsi256_ps(_mm512_cvtepi64_epi32(to_bits())));
  }
  F32vec8 imag() const {
    return F32vec8(_mm256_castsi256_ps(
        _mm512_cvtepi64_epi32(_mm512_srli_epi64(to_bits(), 32))));
  }

  void real(F32vec8 value) {
    // Keep the odd indexes (imaginaries), and interleave with the consecutive
    // reals.
    const auto k_idx = _mm512_setr_epi32(16, 1, 17, 3, 18, 5, 19, 7, 20, 9, 21,
                                         11, 22, 13, 23, 15);
    simd = _mm512_permutex2var_ps(simd, k_idx, _mm512_castps256_ps512(value));
  }
  void imag(F32vec8 value) {
    // Keep the even indexes (reals), and interleave with the consecutive
    // imaginaries.
    const auto k_idx = _mm512_setr_epi32(0, 16, 2, 17, 4, 18, 6, 19, 8, 20, 10,
                                         21, 12, 22, 14, 23);
    simd = _mm512_permutex2var_ps(simd, k_idx, _mm512_castps256_ps512(value));
  }

  friend CF32vec8 conj(CF32vec8 value) {
    return _mm512_mask_xor_ps(value, 0xAAAA, _mm512_set1_ps(-0.0f), value);
  }
  friend CF32vec8 swapRealImag(const CF32vec8 a) {
    return CF32vec8(_mm512_ror_epi64(a.to_bits(), 32));
  }
};

/// Allow the C++ tuple interface to be used on complex SIMD types.
namespace std {
#if defined(__AVX512FP16__)
template <>
struct tuple_size<CF16vec4> : public integral_constant<size_t, 4> {};
template <>
struct tuple_size<CF16vec8> : public integral_constant<size_t, 8> {};
template <>
struct tuple_size<CF16vec16> : public integral_constant<size_t, 16> {};
#endif // defined(__AVX512FP16__)
template <>
struct tuple_size<CF32vec4> : public integral_constant<size_t, 4> {};
template <>
struct tuple_size<CF32vec8> : public integral_constant<size_t, 8> {};
} // namespace std

#if defined(__AVX512FP16__)
/// Conversion functions to and from various complex types. Note that to_int16/to_int32
/// are not provided as there is no suitable CI class yet.
inline CF32vec4 to_float(CF16vec4 v) { return CF32vec4(to_float(F16vec8(v))); }
inline CF32vec8 to_float(CF16vec8 v) { return CF32vec8(to_float(F16vec16(v))); }
inline CF16vec4 to_float16(CF32vec4 v) { return CF16vec4(to_float16(F32vec8(v))); }
inline CF16vec8 to_float16(CF32vec8 v) { return CF16vec8(to_float16(F32vec16(v))); }
#endif

#endif
