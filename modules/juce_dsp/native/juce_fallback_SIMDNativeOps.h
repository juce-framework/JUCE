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

/** A template specialisation to find corresponding mask type for primitives. */
namespace SIMDInternal
{
    template <typename Primitive> struct MaskTypeFor        { typedef Primitive type; };
    template <> struct MaskTypeFor <float>                  { typedef uint32_t  type; };
    template <> struct MaskTypeFor <double>                 { typedef uint64_t  type; };
    template <> struct MaskTypeFor <char>                   { typedef uint8_t   type; };
    template <> struct MaskTypeFor <int8_t>                 { typedef uint8_t   type; };
    template <> struct MaskTypeFor <int16_t>                { typedef uint16_t  type; };
    template <> struct MaskTypeFor <int32_t>                { typedef uint32_t  type; };
    template <> struct MaskTypeFor <int64_t>                { typedef uint64_t  type; };
    template <> struct MaskTypeFor <std::complex<float> >   { typedef uint32_t  type; };
    template <> struct MaskTypeFor <std::complex<double> >  { typedef uint64_t  type; };

    template <typename Primitive> struct PrimitiveType                           { typedef Primitive type; };
    template <typename Primitive> struct PrimitiveType<std::complex<Primitive> > { typedef Primitive type; };

    template <int n>    struct Log2Helper    { enum { value = Log2Helper<n/2>::value + 1 }; };
    template <>         struct Log2Helper<1> { enum { value = 0 }; };
}

/**
    Useful fallback routines to use if the native SIMD op is not supported. You
    should never need to use this directly. Use juce_SIMDRegister instead.
*/
template <typename ScalarType, typename vSIMDType>
struct SIMDFallbackOps
{
    static constexpr size_t n    =  sizeof (vSIMDType) / sizeof (ScalarType);
    static constexpr size_t mask = (sizeof (vSIMDType) / sizeof (ScalarType)) - 1;
    static constexpr size_t bits = SIMDInternal::Log2Helper<n>::value;

    // corresponding mask type
    typedef typename SIMDInternal::MaskTypeFor<ScalarType>::type MaskType;

    // fallback methods
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept        { return apply<ScalarAdd> (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept        { return apply<ScalarSub> (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept        { return apply<ScalarMul> (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept    { return bitapply<ScalarAnd> (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept    { return bitapply<ScalarOr > (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept    { return bitapply<ScalarXor> (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept { return bitapply<ScalarNot> (a, b); }

    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                { return apply<ScalarMin> (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                { return apply<ScalarMax> (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept              { return cmp<ScalarEq > (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept           { return cmp<ScalarNeq> (a, b); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept        { return cmp<ScalarGt > (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept { return cmp<ScalarGeq> (a, b); }

    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept
    {
        vSIMDType retval;
        auto* dst  = reinterpret_cast<MaskType*> (&retval);
        auto* aSrc = reinterpret_cast<const MaskType*> (&a);

        for (size_t i = 0; i < n; ++i)
            dst [i] = ~aSrc [i];

        return retval;
    }

    static forcedinline ScalarType sum (vSIMDType a) noexcept
    {
        auto retval = static_cast<ScalarType> (0);
        auto* aSrc = reinterpret_cast<const ScalarType*> (&a);

        for (size_t i = 0; i < n; ++i)
            retval += aSrc [i];

        return retval;
    }

    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept
    {
        vSIMDType retval;
        auto* dst  = reinterpret_cast<ScalarType*> (&retval);
        auto* aSrc = reinterpret_cast<const ScalarType*> (&a);
        auto* bSrc = reinterpret_cast<const ScalarType*> (&b);
        auto* cSrc = reinterpret_cast<const ScalarType*> (&c);

        for (size_t i = 0; i < n; ++i)
            dst [i] = aSrc [i] + (bSrc [i] * cSrc [i]);

        return retval;
    }

    //==============================================================================
    static forcedinline vSIMDType cmplxmul (vSIMDType a, vSIMDType b) noexcept
    {
        vSIMDType retval;
        auto* dst  = reinterpret_cast<std::complex<ScalarType>*> (&retval);
        auto* aSrc = reinterpret_cast<const std::complex<ScalarType>*> (&a);
        auto* bSrc = reinterpret_cast<const std::complex<ScalarType>*> (&b);

        const int m = n >> 1;
        for (int i = 0; i < m; ++i)
            dst [i] = aSrc [i] * bSrc [i];

        return retval;
    }

    struct ScalarAdd { static forcedinline ScalarType   op (ScalarType a, ScalarType b)   noexcept { return a + b; } };
    struct ScalarSub { static forcedinline ScalarType   op (ScalarType a, ScalarType b)   noexcept { return a - b; } };
    struct ScalarMul { static forcedinline ScalarType   op (ScalarType a, ScalarType b)   noexcept { return a * b; } };
    struct ScalarMin { static forcedinline ScalarType   op (ScalarType a, ScalarType b)   noexcept { return jmin (a, b); } };
    struct ScalarMax { static forcedinline ScalarType   op (ScalarType a, ScalarType b)   noexcept { return jmax (a, b); } };
    struct ScalarAnd { static forcedinline MaskType     op (MaskType a,   MaskType b)     noexcept { return a & b; } };
    struct ScalarOr  { static forcedinline MaskType     op (MaskType a,   MaskType b)     noexcept { return a | b; } };
    struct ScalarXor { static forcedinline MaskType     op (MaskType a,   MaskType b)     noexcept { return a ^ b; } };
    struct ScalarNot { static forcedinline MaskType     op (MaskType a,   MaskType b)     noexcept { return (~a) & b; } };
    struct ScalarEq  { static forcedinline bool         op (ScalarType a, ScalarType b)   noexcept { return (a == b); } };
    struct ScalarNeq { static forcedinline bool         op (ScalarType a, ScalarType b)   noexcept { return (a != b); } };
    struct ScalarGt  { static forcedinline bool         op (ScalarType a, ScalarType b)   noexcept { return (a >  b); } };
    struct ScalarGeq { static forcedinline bool         op (ScalarType a, ScalarType b)   noexcept { return (a >= b); } };

    // generic apply routines for operations above
    template <typename Op>
    static forcedinline vSIMDType apply (vSIMDType a, vSIMDType b) noexcept
    {
        vSIMDType retval;
        auto* dst  = reinterpret_cast<ScalarType*> (&retval);
        auto* aSrc = reinterpret_cast<const ScalarType*> (&a);
        auto* bSrc = reinterpret_cast<const ScalarType*> (&b);

        for (size_t i = 0; i < n; ++i)
            dst [i] = Op::op (aSrc [i], bSrc [i]);

        return retval;
    }

    template <typename Op>
    static forcedinline vSIMDType cmp (vSIMDType a, vSIMDType b) noexcept
    {
        vSIMDType retval;
        auto* dst  = reinterpret_cast<MaskType*> (&retval);
        auto* aSrc = reinterpret_cast<const ScalarType*> (&a);
        auto* bSrc = reinterpret_cast<const ScalarType*> (&b);

        for (size_t i = 0; i < n; ++i)
            dst [i] = Op::op (aSrc [i], bSrc [i]) ? static_cast<MaskType> (-1) : static_cast<MaskType> (0);

        return retval;
    }

    template <typename Op>
    static forcedinline vSIMDType bitapply (vSIMDType a, vSIMDType b) noexcept
    {
        vSIMDType retval;
        auto* dst  = reinterpret_cast<MaskType*> (&retval);
        auto* aSrc = reinterpret_cast<const MaskType*> (&a);
        auto* bSrc = reinterpret_cast<const MaskType*> (&b);

        for (size_t i = 0; i < n; ++i)
            dst [i] = Op::op (aSrc [i], bSrc [i]);

        return retval;
    }

    static forcedinline vSIMDType expand (ScalarType s) noexcept
    {
        vSIMDType retval;
        auto* dst = reinterpret_cast<ScalarType*> (&retval);

        for (size_t i = 0; i < n; ++i)
            dst [i] = s;

        return retval;
    }

    template <unsigned int shuffle_idx>
    static forcedinline vSIMDType shuffle (vSIMDType a) noexcept
    {
        vSIMDType retval;
        auto* dst  = reinterpret_cast<ScalarType*> (&retval);
        auto* aSrc = reinterpret_cast<const ScalarType*> (&a);

        // the compiler will unroll this loop and the index can
        // be computed at compile-time, so this will be super fast
        for (size_t i = 0; i < n; ++i)
            dst [i] = aSrc [(shuffle_idx >> (bits * i)) & mask];

        return retval;
    }
};

} // namespace dsp
} // namespace juce
