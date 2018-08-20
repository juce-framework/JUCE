/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace dsp
{

#ifndef DOXYGEN
 // This class is needed internally.
 template <typename Scalar>
 struct CmplxSIMDOps;
#endif

//==============================================================================
/**
    A wrapper around the platform's native SIMD register type.

    This class is only available on SIMD machines. Use JUCE_USE_SIMD to query
    if SIMD is avaialble for your system.

    SIMDRegister<Type> is a templated class representing the native
    vectorized version of FloatingType. SIMDRegister supports all numerical
    primitive types and std:complex<float> and std::complex<double> supports
    and most operations of the corresponding primitive
    type. Additionally, SIMDRegister can be accessed like an array to extract
    the individual elements.

    If you are using SIMDRegister as a pointer, then you must ensure that the
    memory is suffeciently aligned for SIMD vector operations. Failing to do so
    will result in crashes or very slow code. Use SIMDRegister::isSIMDAligned
    to query if a pointer is suffeciently aligned for SIMD vector operations.

    Note that using SIMDRegister without enabling optimizations will result
    in code with very poor performance.

    @tags{DSP}
*/
template <typename Type>
struct SIMDRegister
{
    //==============================================================================
    /** The type that represents the individual constituents of the SIMD Register */
    using ElementType = Type;

    /** STL compatible value_type definition (same as ElementType). */
    using value_type = ElementType;

    /** The corresponding primitive integer type, for example, this will be int32_t
        if type is a float. */
    using MaskType = typename SIMDInternal::MaskTypeFor<ElementType>::type;

    //==============================================================================
    // Here are some types which are needed internally

    /** The native primitive type (used internally). */
    using PrimitiveType = typename SIMDInternal::PrimitiveType<ElementType>::type;

    /** The native operations for this platform and type combination (used internally) */
    using NativeOps = SIMDNativeOps<PrimitiveType>;

    /** The native type (used internally). */
    using vSIMDType = typename NativeOps::vSIMDType;

    /** The corresponding integer SIMDRegister type (used internally). */
    using vMaskType = SIMDRegister<MaskType>;

    /** The internal native type for the corresponding mask type (used internally). */
    using vMaskSIMDType = typename vMaskType::vSIMDType;

    /** Wrapper for operations which need to be handled differently for complex
        and scalar types (used internally). */
    using CmplxOps = CmplxSIMDOps<ElementType>;

    /** Type which is returned when using the subscript operator. The returned type
        should be used just like the type ElementType. */
    struct ElementAccess;

    //==============================================================================
    /** The size in bytes of this register. */
    static constexpr size_t SIMDRegisterSize = sizeof (vSIMDType);

    /** The number of elements that this vector can hold. */
    static constexpr size_t SIMDNumElements = SIMDRegisterSize / sizeof (ElementType);

    vSIMDType value;

    /** Default constructor. */
    inline SIMDRegister() noexcept {}

    /** Constructs an object from the native SIMD type. */
    inline SIMDRegister (vSIMDType a) noexcept : value (a) {}

    /** Constructs an object from a scalar type by broadcasting it to all elements. */
    inline SIMDRegister (Type s) noexcept  { *this = s; }

    /** Destrutor. */
    inline ~SIMDRegister() noexcept {}

    //==============================================================================
    /** Returns the number of elements in this vector. */
    static constexpr size_t size() noexcept    { return SIMDNumElements; }

    //==============================================================================
    /** Creates a new SIMDRegister from the corresponding scalar primitive.
        The scalar is extended to all elements of the vector. */
    inline static SIMDRegister JUCE_VECTOR_CALLTYPE expand (ElementType s) noexcept         { return {CmplxOps::expand (s)}; }

    /** Creates a new SIMDRegister from the internal SIMD type (for example
        __mm128 for single-precision floating point on SSE architectures). */
    inline static SIMDRegister JUCE_VECTOR_CALLTYPE fromNative (vSIMDType a) noexcept       { return {a}; }

    /** Creates a new SIMDRegister from the first SIMDNumElements of a scalar array. */
    inline static SIMDRegister JUCE_VECTOR_CALLTYPE fromRawArray (const ElementType* a) noexcept
    {
        jassert (isSIMDAligned (a));
        return {CmplxOps::load (a)};
    }

    /** Copies the elements of the SIMDRegister to a scalar array in memory. */
    inline void JUCE_VECTOR_CALLTYPE copyToRawArray (ElementType* a) const noexcept
    {
        jassert (isSIMDAligned (a));
        CmplxOps::store (value, a);
    }

    //==============================================================================
    /** Returns the idx-th element of the receiver. Note that this does not check if idx
        is larger than the native register size. */
    inline ElementType JUCE_VECTOR_CALLTYPE get (size_t idx) const noexcept
    {
        jassert (idx < SIMDNumElements);
        return CmplxOps::get (value, idx);
    }

    /** Sets the idx-th element of the receiver. Note that this does not check if idx
        is larger than the native register size. */
    inline void JUCE_VECTOR_CALLTYPE set (size_t idx, ElementType v) noexcept
    {
        jassert (idx < SIMDNumElements);
        value = CmplxOps::set (value, idx, v);
    }

    //==============================================================================
    /** Returns the idx-th element of the receiver. Note that this does not check if idx
        is larger than the native register size. */
    inline ElementType JUCE_VECTOR_CALLTYPE operator[] (size_t idx) const noexcept
    {
        return get (idx);
    }

    /** Returns the idx-th element of the receiver. Note that this does not check if idx
        is larger than the native register size. */
    inline ElementAccess JUCE_VECTOR_CALLTYPE operator[] (size_t idx) noexcept
    {
        jassert (idx < SIMDNumElements);
        return ElementAccess (*this, idx);
    }

    //==============================================================================
    /** Adds another SIMDRegister to the receiver. */
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator+= (SIMDRegister v) noexcept      { value = NativeOps::add (value, v.value); return *this; }

    /** Subtracts another SIMDRegister to the receiver. */
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator-= (SIMDRegister v) noexcept      { value = NativeOps::sub (value, v.value); return *this; }

    /** Multiplies another SIMDRegister to the receiver. */
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator*= (SIMDRegister v) noexcept      { value = CmplxOps::mul (value, v.value); return *this; }

    //==============================================================================
    /** Broadcasts the scalar to all elements of the receiver. */
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator=  (ElementType s) noexcept       { value  = CmplxOps::expand (s); return *this; }

    /** Adds a scalar to the receiver. */
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator+= (ElementType s) noexcept       { value = NativeOps::add (value, CmplxOps::expand (s)); return *this; }

    /** Subtracts a scalar to the receiver. */
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator-= (ElementType s) noexcept       { value = NativeOps::sub (value, CmplxOps::expand (s)); return *this; }

    /** Multiplies a scalar to the receiver. */
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator*= (ElementType s) noexcept       { value = CmplxOps::mul (value, CmplxOps::expand (s)); return *this; }

    //==============================================================================
    /** Bit-and the reciver with SIMDRegister v and store the result in the receiver. */
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator&= (vMaskType v) noexcept         { value = NativeOps::bit_and (value, toVecType (v.value)); return *this; }

    /** Bit-or the reciver with SIMDRegister v and store the result in the receiver. */
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator|= (vMaskType v) noexcept         { value = NativeOps::bit_or  (value, toVecType (v.value)); return *this; }

    /** Bit-xor the reciver with SIMDRegister v and store the result in the receiver. */
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator^= (vMaskType v) noexcept         { value = NativeOps::bit_xor (value, toVecType (v.value)); return *this; }

    //==============================================================================
    /** Bit-and each element of the reciver with the scalar s and store the result in the receiver.*/
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator&= (MaskType s) noexcept           { value = NativeOps::bit_and (value, toVecType (s)); return *this; }

    /** Bit-or each element of the reciver with the scalar s and store the result in the receiver.*/
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator|= (MaskType s) noexcept           { value = NativeOps::bit_or  (value, toVecType (s)); return *this; }

    /** Bit-xor each element of the reciver with the scalar s and store the result in the receiver.*/
    inline SIMDRegister& JUCE_VECTOR_CALLTYPE operator^= (MaskType s) noexcept           { value = NativeOps::bit_xor (value, toVecType (s)); return *this; }

    //==============================================================================
    /** Returns the sum of the receiver and v.*/
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator+ (SIMDRegister v) const noexcept  { return { NativeOps::add (value, v.value) }; }

    /** Returns the difference of the receiver and v.*/
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator- (SIMDRegister v) const noexcept  { return { NativeOps::sub (value, v.value) }; }

    /** Returns the product of the receiver and v.*/
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator* (SIMDRegister v) const noexcept  { return { CmplxOps::mul (value, v.value) }; }

    //==============================================================================
    /** Returns a vector where each element is the sum of the corresponding element in the receiver and the scalar s.*/
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator+ (ElementType s) const noexcept   { return { NativeOps::add (value, CmplxOps::expand (s)) }; }

    /** Returns a vector where each element is the difference of the corresponding element in the receiver and the scalar s.*/
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator- (ElementType s) const noexcept   { return { NativeOps::sub (value, CmplxOps::expand (s)) }; }

    /** Returns a vector where each element is the product of the corresponding element in the receiver and the scalar s.*/
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator* (ElementType s) const noexcept   { return { CmplxOps::mul (value, CmplxOps::expand (s)) }; }

    //==============================================================================
    /** Returns the bit-and of the receiver and v. */
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator& (vMaskType v) const noexcept     { return { NativeOps::bit_and (value, toVecType (v.value)) }; }

    /** Returns the bit-or of the receiver and v. */
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator| (vMaskType v) const noexcept     { return { NativeOps::bit_or  (value, toVecType (v.value)) }; }

    /** Returns the bit-xor of the receiver and v. */
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator^ (vMaskType v) const noexcept     { return { NativeOps::bit_xor (value, toVecType (v.value)) }; }

    /** Returns a vector where each element is the bit-inverted value of the corresponding element in the receiver.*/
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator~() const noexcept                 { return { NativeOps::bit_not (value) }; }

    //==============================================================================
    /** Returns a vector where each element is the bit-and'd value of the corresponding element in the receiver and the scalar s.*/
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator& (MaskType s) const noexcept      { return { NativeOps::bit_and (value, toVecType (s)) }; }

    /** Returns a vector where each element is the bit-or'd value of the corresponding element in the receiver and the scalar s.*/
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator| (MaskType s) const noexcept      { return { NativeOps::bit_or  (value, toVecType (s)) }; }

    /** Returns a vector where each element is the bit-xor'd value of the corresponding element in the receiver and the scalar s.*/
    inline SIMDRegister JUCE_VECTOR_CALLTYPE operator^ (MaskType s) const noexcept      { return { NativeOps::bit_xor (value, toVecType (s)) }; }

    //==============================================================================
    /** Returns true if all elements-wise comparisons return true. */
    inline bool JUCE_VECTOR_CALLTYPE operator== (SIMDRegister other) const noexcept    { return  NativeOps::allEqual (value, other.value); }

    /** Returns true if any elements-wise comparisons return false. */
    inline bool JUCE_VECTOR_CALLTYPE operator!= (SIMDRegister other) const noexcept    { return ! (*this == other); }

    /** Returns true if all elements are equal to the scalar. */
    inline bool JUCE_VECTOR_CALLTYPE operator== (Type s) const noexcept                { return *this == SIMDRegister::expand (s); }

    /** Returns true if any elements are not equal to the scalar. */
    inline bool JUCE_VECTOR_CALLTYPE operator!= (Type s) const noexcept                { return ! (*this == s); }

    //==============================================================================
    /** Returns a SIMDRegister of the corresponding integral type where each element has each bit set
        if the corresponding element of a is equal to the corresponding element of b, or zero otherwise.
        The result can then be used in bit operations defined above to avoid branches in vector SIMD code. */
    static inline vMaskType JUCE_VECTOR_CALLTYPE equal              (SIMDRegister a, SIMDRegister b) noexcept { return toMaskType (NativeOps::equal (a.value, b.value)); }

    /** Returns a SIMDRegister of the corresponding integral type where each element has each bit set
        if the corresponding element of a is not equal to the corresponding element of b, or zero otherwise.
        The result can then be used in bit operations defined above to avoid branches in vector SIMD code. */
    static inline vMaskType JUCE_VECTOR_CALLTYPE notEqual           (SIMDRegister a, SIMDRegister b) noexcept { return toMaskType (NativeOps::notEqual (a.value, b.value)); }

    /** Returns a SIMDRegister of the corresponding integral type where each element has each bit set
        if the corresponding element of a is less than to the corresponding element of b, or zero otherwise.
        The result can then be used in bit operations defined above to avoid branches in vector SIMD code. */
    static inline vMaskType JUCE_VECTOR_CALLTYPE lessThan           (SIMDRegister a, SIMDRegister b) noexcept { return toMaskType (NativeOps::greaterThan (b.value, a.value)); }

    /** Returns a SIMDRegister of the corresponding integral type where each element has each bit set
        if the corresponding element of a is than or equal to the corresponding element of b, or zero otherwise.
        The result can then be used in bit operations defined above to avoid branches in vector SIMD code. */
    static inline vMaskType JUCE_VECTOR_CALLTYPE lessThanOrEqual    (SIMDRegister a, SIMDRegister b) noexcept { return toMaskType (NativeOps::greaterThanOrEqual (b.value, a.value)); }

    /** Returns a SIMDRegister of the corresponding integral type where each element has each bit set
        if the corresponding element of a is greater than to the corresponding element of b, or zero otherwise.
        The result can then be used in bit operations defined above to avoid branches in vector SIMD code. */
    static inline vMaskType JUCE_VECTOR_CALLTYPE greaterThan        (SIMDRegister a, SIMDRegister b) noexcept { return toMaskType (NativeOps::greaterThan (a.value, b.value)); }

    /** Returns a SIMDRegister of the corresponding integral type where each element has each bit set
        if the corresponding element of a is greater than or equal to the corresponding element of b, or zero otherwise.
        The result can then be used in bit operations defined above to avoid branches in vector SIMD code. */
    static inline vMaskType JUCE_VECTOR_CALLTYPE greaterThanOrEqual (SIMDRegister a, SIMDRegister b) noexcept { return toMaskType (NativeOps::greaterThanOrEqual (a.value, b.value)); }

     //==============================================================================
    /** Returns a new vector where each element is the minimum of the corresponding element of a and b. */
    static inline SIMDRegister JUCE_VECTOR_CALLTYPE min (SIMDRegister a, SIMDRegister b) noexcept    { return { NativeOps::min (a.value, b.value) }; }

    /** Returns a new vector where each element is the maximum of the corresponding element of a and b. */
    static inline SIMDRegister JUCE_VECTOR_CALLTYPE max (SIMDRegister a, SIMDRegister b) noexcept    { return { NativeOps::max (a.value, b.value) }; }

    //==============================================================================
    /** Multiplies b and c and adds the result to a. */
    static inline SIMDRegister JUCE_VECTOR_CALLTYPE multiplyAdd (SIMDRegister a, const SIMDRegister b, SIMDRegister c) noexcept
    {
        return { CmplxOps::muladd (a.value, b.value, c.value) };
    }

    //==============================================================================
    /** Returns a scalar which is the sum of all elements of the receiver. */
    inline ElementType sum() const noexcept          { return CmplxOps::sum (value); }

    //==============================================================================
    /** Checks if the given pointer is suffeciently aligned for using SIMD operations. */
    static inline bool isSIMDAligned (const ElementType* ptr) noexcept
    {
        uintptr_t bitmask = SIMDRegisterSize - 1;
        return (reinterpret_cast<uintptr_t> (ptr) & bitmask) == 0;
    }

    /** Returns the next position in memory where isSIMDAligned returns true.

        If the current position in memory is already aligned then this method
        will simply return the pointer.
    */
    static inline ElementType* getNextSIMDAlignedPtr (ElementType* ptr) noexcept
    {
        return snapPointerToAlignment (ptr, SIMDRegisterSize);
    }

   #ifndef DOXYGEN
    static inline const ElementType* getNextSIMDAlignedPtr (const ElementType* ptr) noexcept
    {
        return snapPointerToAlignment (ptr, SIMDRegisterSize);
    }
   #endif

private:
    static inline vMaskType JUCE_VECTOR_CALLTYPE toMaskType (vSIMDType a) noexcept
    {
        union
        {
            vSIMDType in;
            vMaskSIMDType out;
        } u;

        u.in = a;
        return vMaskType::fromNative (u.out);
    }

    static inline vSIMDType JUCE_VECTOR_CALLTYPE toVecType (vMaskSIMDType a) noexcept
    {
        union
        {
            vMaskSIMDType in;
            vSIMDType out;
        } u;

        u.in = a;
        return u.out;
    }

    static inline vSIMDType JUCE_VECTOR_CALLTYPE toVecType (MaskType a) noexcept
    {
        union
        {
            vMaskSIMDType in;
            vSIMDType out;
        } u;

        u.in = CmplxSIMDOps<MaskType>::expand (a);
        return u.out;
    }
};

} // namespace dsp
} // namespace juce

#ifndef DOXYGEN
 #include "juce_SIMDRegister_Impl.h"
#endif
