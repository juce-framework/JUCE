/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/*
    This file sets up some handy mathematical typdefs and functions.
*/

//==============================================================================
// Definitions for the int8, int16, int32, int64 and pointer_sized_int types.

/** A platform-independent 8-bit signed integer type. */
using int8      = signed char;
/** A platform-independent 8-bit unsigned integer type. */
using uint8     = unsigned char;
/** A platform-independent 16-bit signed integer type. */
using int16     = signed short;
/** A platform-independent 16-bit unsigned integer type. */
using uint16    = unsigned short;
/** A platform-independent 32-bit signed integer type. */
using int32     = signed int;
/** A platform-independent 32-bit unsigned integer type. */
using uint32    = unsigned int;

#if JUCE_MSVC
  /** A platform-independent 64-bit integer type. */
  using int64  = __int64;
  /** A platform-independent 64-bit unsigned integer type. */
  using uint64 = unsigned __int64;
#else
  /** A platform-independent 64-bit integer type. */
  using int64  = long long;
  /** A platform-independent 64-bit unsigned integer type. */
  using uint64 = unsigned long long;
#endif

#ifndef DOXYGEN
 /** A macro for creating 64-bit literals.
     Historically, this was needed to support portability with MSVC6, and is kept here
     so that old code will still compile, but nowadays every compiler will support the
     LL and ULL suffixes, so you should use those in preference to this macro.
 */
 #define literal64bit(longLiteral)     (longLiteral##LL)
#endif

#if JUCE_64BIT
  /** A signed integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  using pointer_sized_int  = int64;
  /** An unsigned integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  using pointer_sized_uint = uint64;
#elif JUCE_MSVC
  /** A signed integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  using pointer_sized_int  = _W64 int;
  /** An unsigned integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  using pointer_sized_uint = _W64 unsigned int;
#else
  /** A signed integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  using pointer_sized_int  = int;
  /** An unsigned integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  using pointer_sized_uint = unsigned int;
#endif

#if JUCE_WINDOWS
  using ssize_t = pointer_sized_int;
#endif

//==============================================================================
/** Handy function for avoiding unused variables warning. */
template <typename... Types>
void ignoreUnused (Types&&...) noexcept {}

/** Handy function for getting the number of elements in a simple const C array.
    E.g.
    @code
    static int myArray[] = { 1, 2, 3 };

    int numElements = numElementsInArray (myArray) // returns 3
    @endcode
*/
template <typename Type, size_t N>
constexpr int numElementsInArray (Type (&)[N]) noexcept     { return N; }

//==============================================================================
// Some useful maths functions that aren't always present with all compilers and build settings.

/** Using juce_hypot is easier than dealing with the different types of hypot function
    that are provided by the various platforms and compilers. */
template <typename Type>
Type juce_hypot (Type a, Type b) noexcept
{
   #if JUCE_MSVC
    return static_cast<Type> (_hypot (a, b));
   #else
    return static_cast<Type> (hypot (a, b));
   #endif
}

#ifndef DOXYGEN
template <>
inline float juce_hypot (float a, float b) noexcept
{
   #if JUCE_MSVC
    return _hypotf (a, b);
   #else
    return hypotf (a, b);
   #endif
}
#endif

//==============================================================================
/** Commonly used mathematical constants

    @tags{Core}
*/
template <typename FloatType>
struct MathConstants
{
    /** A predefined value for Pi */
    static constexpr FloatType pi = static_cast<FloatType> (3.141592653589793238L);

    /** A predefined value for 2 * Pi */
    static constexpr FloatType twoPi = static_cast<FloatType> (2 * 3.141592653589793238L);

    /** A predefined value for Pi / 2 */
    static constexpr FloatType halfPi = static_cast<FloatType> (3.141592653589793238L / 2);

    /** A predefined value for Euler's number */
    static constexpr FloatType euler = static_cast<FloatType> (2.71828182845904523536L);

    /** A predefined value for sqrt (2) */
    static constexpr FloatType sqrt2 = static_cast<FloatType> (1.4142135623730950488L);
};

#ifndef DOXYGEN
/** A double-precision constant for pi. */
[[deprecated ("This is deprecated in favour of MathConstants<double>::pi.")]]
const constexpr double  double_Pi  = MathConstants<double>::pi;

/** A single-precision constant for pi. */
[[deprecated ("This is deprecated in favour of MathConstants<float>::pi.")]]
const constexpr float   float_Pi   = MathConstants<float>::pi;
#endif

/** Converts an angle in degrees to radians. */
template <typename FloatType>
constexpr FloatType degreesToRadians (FloatType degrees) noexcept     { return degrees * (MathConstants<FloatType>::pi / FloatType (180)); }

/** Converts an angle in radians to degrees. */
template <typename FloatType>
constexpr FloatType radiansToDegrees (FloatType radians) noexcept     { return radians * (FloatType (180) / MathConstants<FloatType>::pi); }

//==============================================================================
/** The isfinite() method seems to vary between platforms, so this is a
    platform-independent function for it.
*/
template <typename NumericType>
bool juce_isfinite (NumericType value) noexcept
{
    if constexpr (std::numeric_limits<NumericType>::has_infinity
                  || std::numeric_limits<NumericType>::has_quiet_NaN
                  || std::numeric_limits<NumericType>::has_signaling_NaN)
    {
        return std::isfinite (value);
    }
    else
    {
        ignoreUnused (value);
        return true;
    }
}

//==============================================================================
/** Equivalent to operator==, but suppresses float-equality warnings.

    This allows code to be explicit about float-equality checks that are known to have the correct
    semantics.
*/
template <typename Type>
constexpr bool exactlyEqual (Type a, Type b)
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
    return a == b;
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

/** A class encapsulating both relative and absolute tolerances for use in floating-point comparisons.

    @see approximatelyEqual, absoluteTolerance, relativeTolerance

    @tags{Core}
*/
template <typename Type>
class Tolerance
{
public:
    Tolerance() = default;

    /** Returns a copy of this Tolerance object with a new absolute tolerance.

        If you just need a Tolerance object with an absolute tolerance, it might be worth using the
        absoluteTolerance() function.

        @see getAbsolute, absoluteTolerance
    */
    [[nodiscard]] Tolerance withAbsolute (Type newAbsolute)
    {
        return withMember (*this, &Tolerance::absolute, std::abs (newAbsolute));
    }

    /** Returns a copy of this Tolerance object with a new relative tolerance.

        If you just need a Tolerance object with a relative tolerance, it might be worth using the
        relativeTolerance() function.

        @see getRelative, relativeTolerance
    */
    [[nodiscard]] Tolerance withRelative (Type newRelative)
    {
        return withMember (*this, &Tolerance::relative, std::abs (newRelative));
    }

    [[nodiscard]] Type getAbsolute() const { return absolute; }
    [[nodiscard]] Type getRelative() const { return relative; }

private:
    Type absolute{};
    Type relative{};
};

/** Returns a type deduced Tolerance object containing only an absolute tolerance.

    @see Tolerance::withAbsolute, approximatelyEqual
 */
template <typename Type>
static Tolerance<Type> absoluteTolerance (Type tolerance)
{
    return Tolerance<Type>{}.withAbsolute (tolerance);
}

/** Returns a type deduced Tolerance object containing only a relative tolerance.

    @see Tolerance::withRelative, approximatelyEqual
 */
template <typename Type>
static Tolerance<Type> relativeTolerance (Type tolerance)
{
    return Tolerance<Type>{}.withRelative (tolerance);
}


/** Returns true if the two floating-point numbers are approximately equal.

    If either a or b are not finite, returns exactlyEqual (a, b).

    The default absolute tolerance is equal to the minimum normal value. This ensures
    differences that are subnormal are always considered equal. It is highly recommend this
    value is reviewed depending on the calculation being carried out. In general specifying an
    absolute value is useful when considering values close to zero. For example you might
    expect sin (pi) to return 0, but what it actually returns is close to the error of the value pi.
    Therefore, in this example it might be better to set the absolute tolerance to sin (pi).

    The default relative tolerance is equal to the machine epsilon which is the difference between
    1.0 and the next floating-point value that can be represented by Type. In most cases this value
    is probably reasonable. This value is multiplied by the largest absolute value of a and b so as
    to scale relatively according to the input parameters. For example, specifying a relative value
    of 0.05 will ensure values return equal if the difference between them is less than or equal to
    5% of the larger of the two absolute values.

    @param a            The first number to compare.
    @param b            The second number to compare.
    @param tolerance    An object that represents both absolute and relative tolerances
                        when evaluating if a and b are equal.

    @see exactlyEqual
*/
template <typename Type, std::enable_if_t<std::is_floating_point_v<Type>, int> = 0>
constexpr bool approximatelyEqual (Type a, Type b,
                                   Tolerance<Type> tolerance = Tolerance<Type>{}
                                        .withAbsolute (std::numeric_limits<Type>::min())
                                        .withRelative (std::numeric_limits<Type>::epsilon()))
{
    if (! (juce_isfinite (a) && juce_isfinite (b)))
        return exactlyEqual (a, b);

    const auto diff = std::abs (a - b);

    return diff <= tolerance.getAbsolute()
        || diff <= tolerance.getRelative() * std::max (std::abs (a), std::abs (b));
}

/** Special case for non-floating-point types that returns true if both are exactly equal. */
template <typename Type, std::enable_if_t<! std::is_floating_point_v<Type>, int> = 0>
constexpr bool approximatelyEqual (Type a, Type b)
{
    return a == b;
}

//==============================================================================
/** Returns the next representable value by FloatType in the direction of the largest representable value. */
template <typename FloatType>
FloatType nextFloatUp (FloatType value) noexcept
{
    return std::nextafter (value, std::numeric_limits<FloatType>::max());
}

/** Returns the next representable value by FloatType in the direction of the lowest representable value. */
template <typename FloatType>
FloatType nextFloatDown (FloatType value) noexcept
{
    return std::nextafter (value, std::numeric_limits<FloatType>::lowest());
}

//==============================================================================
// Some indispensable min/max functions

/** Returns the larger of two values. */
template <typename Type>
constexpr Type jmax (Type a, Type b)                                   { return a < b ? b : a; }

/** Returns the larger of three values. */
template <typename Type>
constexpr Type jmax (Type a, Type b, Type c)                           { return a < b ? (b < c ? c : b) : (a < c ? c : a); }

/** Returns the larger of four values. */
template <typename Type>
constexpr Type jmax (Type a, Type b, Type c, Type d)                   { return jmax (a, jmax (b, c, d)); }

/** Returns the smaller of two values. */
template <typename Type>
constexpr Type jmin (Type a, Type b)                                   { return b < a ? b : a; }

/** Returns the smaller of three values. */
template <typename Type>
constexpr Type jmin (Type a, Type b, Type c)                           { return b < a ? (c < b ? c : b) : (c < a ? c : a); }

/** Returns the smaller of four values. */
template <typename Type>
constexpr Type jmin (Type a, Type b, Type c, Type d)                   { return jmin (a, jmin (b, c, d)); }

/** Remaps a normalised value (between 0 and 1) to a target range.
    This effectively returns (targetRangeMin + value0To1 * (targetRangeMax - targetRangeMin)).
*/
template <typename Type>
constexpr Type jmap (Type value0To1, Type targetRangeMin, Type targetRangeMax)
{
    return targetRangeMin + value0To1 * (targetRangeMax - targetRangeMin);
}

/** Remaps a value from a source range to a target range. */
template <typename Type>
Type jmap (Type sourceValue, Type sourceRangeMin, Type sourceRangeMax, Type targetRangeMin, Type targetRangeMax)
{
    jassert (! approximatelyEqual (sourceRangeMax, sourceRangeMin)); // mapping from a range of zero will produce NaN!
    return targetRangeMin + ((targetRangeMax - targetRangeMin) * (sourceValue - sourceRangeMin)) / (sourceRangeMax - sourceRangeMin);
}

/** Remaps a normalised value (between 0 and 1) to a logarithmic target range.

    The entire target range must be greater than zero.

    @see mapFromLog10

    @code
    mapToLog10 (0.5, 0.4, 40.0) == 4.0
    @endcode
*/
template <typename Type>
Type mapToLog10 (Type value0To1, Type logRangeMin, Type logRangeMax)
{
    jassert (logRangeMin > 0);
    jassert (logRangeMax > 0);

    auto logMin = std::log10 (logRangeMin);
    auto logMax = std::log10 (logRangeMax);

    return std::pow ((Type) 10.0, value0To1 * (logMax - logMin) + logMin);
}

/** Remaps a logarithmic value in a target range to a normalised value (between 0 and 1).

    The entire target range must be greater than zero.

    @see mapToLog10

    @code
    mapFromLog10 (4.0, 0.4, 40.0) == 0.5
    @endcode
*/
template <typename Type>
Type mapFromLog10 (Type valueInLogRange, Type logRangeMin, Type logRangeMax)
{
    jassert (logRangeMin > 0);
    jassert (logRangeMax > 0);

    auto logMin = std::log10 (logRangeMin);
    auto logMax = std::log10 (logRangeMax);

    return (std::log10 (valueInLogRange) - logMin) / (logMax - logMin);
}

/** Scans an array of values, returning the minimum value that it contains. */
template <typename Type, typename Size>
Type findMinimum (const Type* data, Size numValues)
{
    if (numValues <= 0)
        return Type (0);

    auto result = *data++;

    while (--numValues > 0) // (> 0 rather than >= 0 because we've already taken the first sample)
    {
        auto v = *data++;

        if (v < result)
            result = v;
    }

    return result;
}

/** Scans an array of values, returning the maximum value that it contains. */
template <typename Type, typename Size>
Type findMaximum (const Type* values, Size numValues)
{
    if (numValues <= 0)
        return Type (0);

    auto result = *values++;

    while (--numValues > 0) // (> 0 rather than >= 0 because we've already taken the first sample)
    {
        auto v = *values++;

        if (result < v)
            result = v;
    }

    return result;
}

/** Scans an array of values, returning the minimum and maximum values that it contains. */
template <typename Type>
void findMinAndMax (const Type* values, int numValues, Type& lowest, Type& highest)
{
    if (numValues <= 0)
    {
        lowest = Type (0);
        highest = Type (0);
    }
    else
    {
        auto mn = *values++;
        auto mx = mn;

        while (--numValues > 0) // (> 0 rather than >= 0 because we've already taken the first sample)
        {
            auto v = *values++;

            if (mx < v)  mx = v;
            if (v < mn)  mn = v;
        }

        lowest = mn;
        highest = mx;
    }
}

//==============================================================================
/** Constrains a value to keep it within a given range.

    This will check that the specified value lies between the lower and upper bounds
    specified, and if not, will return the nearest value that would be in-range. Effectively,
    it's like calling jmax (lowerLimit, jmin (upperLimit, value)).

    Note that it expects that lowerLimit <= upperLimit. If this isn't true,
    the results will be unpredictable.

    @param lowerLimit           the minimum value to return
    @param upperLimit           the maximum value to return
    @param valueToConstrain     the value to try to return
    @returns    the closest value to valueToConstrain which lies between lowerLimit
                and upperLimit (inclusive)
    @see jmin, jmax, jmap
*/
template <typename Type>
Type jlimit (Type lowerLimit,
             Type upperLimit,
             Type valueToConstrain) noexcept
{
    jassert (lowerLimit <= upperLimit); // if these are in the wrong order, results are unpredictable..

    return valueToConstrain < lowerLimit ? lowerLimit
                                         : (upperLimit < valueToConstrain ? upperLimit
                                                                          : valueToConstrain);
}

/** Returns true if a value is at least zero, and also below a specified upper limit.
    This is basically a quicker way to write:
    @code valueToTest >= 0 && valueToTest < upperLimit
    @endcode
*/
template <typename Type1, typename Type2>
bool isPositiveAndBelow (Type1 valueToTest, Type2 upperLimit) noexcept
{
    jassert (Type1() <= static_cast<Type1> (upperLimit)); // makes no sense to call this if the upper limit is itself below zero..
    return Type1() <= valueToTest && valueToTest < static_cast<Type1> (upperLimit);
}

template <typename Type>
bool isPositiveAndBelow (int valueToTest, Type upperLimit) noexcept
{
    jassert (upperLimit >= 0); // makes no sense to call this if the upper limit is itself below zero..
    return static_cast<unsigned int> (valueToTest) < static_cast<unsigned int> (upperLimit);
}

/** Returns true if a value is at least zero, and also less than or equal to a specified upper limit.
    This is basically a quicker way to write:
    @code valueToTest >= 0 && valueToTest <= upperLimit
    @endcode
*/
template <typename Type1, typename Type2>
bool isPositiveAndNotGreaterThan (Type1 valueToTest, Type2 upperLimit) noexcept
{
    jassert (Type1() <= static_cast<Type1> (upperLimit)); // makes no sense to call this if the upper limit is itself below zero..
    return Type1() <= valueToTest && valueToTest <= static_cast<Type1> (upperLimit);
}

template <typename Type>
bool isPositiveAndNotGreaterThan (int valueToTest, Type upperLimit) noexcept
{
    jassert (upperLimit >= 0); // makes no sense to call this if the upper limit is itself below zero..
    return static_cast<unsigned int> (valueToTest) <= static_cast<unsigned int> (upperLimit);
}

/** Computes the absolute difference between two values and returns true if it is less than or equal
    to a given tolerance, otherwise it returns false.
*/
template <typename Type>
bool isWithin (Type a, Type b, Type tolerance) noexcept
{
    return std::abs (a - b) <= tolerance;
}

//==============================================================================
#if JUCE_MSVC
 #pragma optimize ("t", off)
 #ifndef __INTEL_COMPILER
  #pragma float_control (precise, on, push)
 #endif
#endif

/** Fast floating-point-to-integer conversion.

    This is faster than using the normal c++ cast to convert a float to an int, and
    it will round the value to the nearest integer, rather than rounding it down
    like the normal cast does.

    Note that this routine gets its speed at the expense of some accuracy, and when
    rounding values whose floating point component is exactly 0.5, odd numbers and
    even numbers will be rounded up or down differently.
*/
template <typename FloatType>
int roundToInt (const FloatType value) noexcept
{
  #ifdef __INTEL_COMPILER
   #pragma float_control (precise, on, push)
  #endif

    union { int asInt[2]; double asDouble; } n;
    n.asDouble = ((double) value) + 6755399441055744.0;

   #if JUCE_BIG_ENDIAN
    return n.asInt [1];
   #else
    return n.asInt [0];
   #endif
}

inline int roundToInt (int value) noexcept
{
    return value;
}

#if JUCE_MSVC
 #ifndef __INTEL_COMPILER
  #pragma float_control (pop)
 #endif
 #pragma optimize ("", on)  // resets optimisations to the project defaults
#endif

/** Fast floating-point-to-integer conversion.

    This is a slightly slower and slightly more accurate version of roundToInt(). It works
    fine for values above zero, but negative numbers are rounded the wrong way.
*/
inline int roundToIntAccurate (double value) noexcept
{
   #ifdef __INTEL_COMPILER
    #pragma float_control (pop)
   #endif

    return roundToInt (value + 1.5e-8);
}

//==============================================================================
/** Truncates a positive floating-point number to an unsigned int.

    This is generally faster than static_cast<unsigned int> (std::floor (x))
    but it only works for positive numbers small enough to be represented as an
    unsigned int.
*/
template <typename FloatType>
unsigned int truncatePositiveToUnsignedInt (FloatType value) noexcept
{
    jassert (value >= static_cast<FloatType> (0));
    jassert (static_cast<FloatType> (value)
             <= static_cast<FloatType> (std::numeric_limits<unsigned int>::max()));

    return static_cast<unsigned int> (value);
}

//==============================================================================
/** Returns true if the specified integer is a power-of-two. */
template <typename IntegerType>
constexpr bool isPowerOfTwo (IntegerType value)
{
   return (value & (value - 1)) == 0;
}

/** Returns the smallest power-of-two which is equal to or greater than the given integer. */
inline int nextPowerOfTwo (int n) noexcept
{
    --n;
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    n |= (n >> 8);
    n |= (n >> 16);
    return n + 1;
}

/** Returns the index of the highest set bit in a (non-zero) number.
    So for n=3 this would return 1, for n=7 it returns 2, etc.
    An input value of 0 is illegal!
*/
int findHighestSetBit (uint32 n) noexcept;

/** Returns the number of bits in a 32-bit integer. */
constexpr int countNumberOfBits (uint32 n) noexcept
{
    n -= ((n >> 1) & 0x55555555);
    n =  (((n >> 2) & 0x33333333) + (n & 0x33333333));
    n =  (((n >> 4) + n) & 0x0f0f0f0f);
    n += (n >> 8);
    n += (n >> 16);
    return (int) (n & 0x3f);
}

/** Returns the number of bits in a 64-bit integer. */
constexpr int countNumberOfBits (uint64 n) noexcept
{
    return countNumberOfBits ((uint32) n) + countNumberOfBits ((uint32) (n >> 32));
}

/** Performs a modulo operation, but can cope with the dividend being negative.
    The divisor must be greater than zero.
*/
template <typename IntegerType>
IntegerType negativeAwareModulo (IntegerType dividend, const IntegerType divisor) noexcept
{
    jassert (divisor > 0);
    dividend %= divisor;
    return (dividend < 0) ? (dividend + divisor) : dividend;
}

/** Returns the square of its argument. */
template <typename NumericType>
inline constexpr NumericType square (NumericType n) noexcept
{
    return n * n;
}

//==============================================================================
/** Writes a number of bits into a memory buffer at a given bit index.
    The buffer is treated as a sequence of 8-bit bytes, and the value is encoded in little-endian order,
    so for example if startBit = 10, and numBits = 11 then the lower 6 bits of the value would be written
    into bits 2-8 of targetBuffer[1], and the upper 5 bits of value into bits 0-5 of targetBuffer[2].

    @see readLittleEndianBitsInBuffer
*/
void writeLittleEndianBitsInBuffer (void* targetBuffer, uint32 startBit, uint32 numBits, uint32 value) noexcept;

/** Reads a number of bits from a buffer at a given bit index.
    The buffer is treated as a sequence of 8-bit bytes, and the value is encoded in little-endian order,
    so for example if startBit = 10, and numBits = 11 then the lower 6 bits of the result would be read
    from bits 2-8 of sourceBuffer[1], and the upper 5 bits of the result from bits 0-5 of sourceBuffer[2].

    @see writeLittleEndianBitsInBuffer
*/
uint32 readLittleEndianBitsInBuffer (const void* sourceBuffer, uint32 startBit, uint32 numBits) noexcept;


//==============================================================================
#if JUCE_INTEL || DOXYGEN
 /** This macro can be applied to a float variable to check whether it contains a denormalised
     value, and to normalise it if necessary.
     On CPUs that aren't vulnerable to denormalisation problems, this will have no effect.
 */
 #define JUCE_UNDENORMALISE(x)   { (x) += 0.1f; (x) -= 0.1f; }
#else
 #define JUCE_UNDENORMALISE(x)
#endif

//==============================================================================
/** This namespace contains a few template classes for helping work out class type variations.
*/
namespace TypeHelpers
{
    /** The ParameterType struct is used to find the best type to use when passing some kind
        of object as a parameter.

        Of course, this is only likely to be useful in certain esoteric template situations.

        E.g. "myFunction (typename TypeHelpers::ParameterType<int>::type, typename TypeHelpers::ParameterType<MyObject>::type)"
        would evaluate to "myfunction (int, const MyObject&)", keeping any primitive types as
        pass-by-value, but passing objects as a const reference, to avoid copying.

        @tags{Core}
    */
    template <typename Type> struct ParameterType                   { using type = const Type&; };

   #ifndef DOXYGEN
    template <typename Type> struct ParameterType <Type&>           { using type = Type&; };
    template <typename Type> struct ParameterType <Type*>           { using type = Type*; };
    template <>              struct ParameterType <char>            { using type = char; };
    template <>              struct ParameterType <unsigned char>   { using type = unsigned char; };
    template <>              struct ParameterType <short>           { using type = short; };
    template <>              struct ParameterType <unsigned short>  { using type = unsigned short; };
    template <>              struct ParameterType <int>             { using type = int; };
    template <>              struct ParameterType <unsigned int>    { using type = unsigned int; };
    template <>              struct ParameterType <long>            { using type = long; };
    template <>              struct ParameterType <unsigned long>   { using type = unsigned long; };
    template <>              struct ParameterType <int64>           { using type = int64; };
    template <>              struct ParameterType <uint64>          { using type = uint64; };
    template <>              struct ParameterType <bool>            { using type = bool; };
    template <>              struct ParameterType <float>           { using type = float; };
    template <>              struct ParameterType <double>          { using type = double; };
   #endif

    /** These templates are designed to take a type, and if it's a double, they return a double
        type; for anything else, they return a float type.

        @tags{Core}
    */
    template <typename Type>
    using SmallestFloatType = std::conditional_t<std::is_same_v<Type, double>, double, float>;

    /** These templates are designed to take an integer type, and return an unsigned int
        version with the same size.

        @tags{Core}
    */
    template <int bytes>     struct UnsignedTypeWithSize            {};

   #ifndef DOXYGEN
    template <>              struct UnsignedTypeWithSize<1>         { using type = uint8; };
    template <>              struct UnsignedTypeWithSize<2>         { using type = uint16; };
    template <>              struct UnsignedTypeWithSize<4>         { using type = uint32; };
    template <>              struct UnsignedTypeWithSize<8>         { using type = uint64; };
   #endif
}

//==============================================================================
#ifndef DOXYGEN
 [[deprecated ("Use roundToInt instead.")]] inline int roundDoubleToInt (double value) noexcept  { return roundToInt (value); }
 [[deprecated ("Use roundToInt instead.")]] inline int roundFloatToInt  (float  value) noexcept  { return roundToInt (value); }
 [[deprecated ("Use std::abs() instead.")]] inline int64 abs64 (int64 n) noexcept                { return std::abs (n); }
#endif

/** Converts an enum to its underlying integral type.
    Similar to std::to_underlying, which is only available in C++23 and above.
*/
template <typename T>
constexpr auto toUnderlyingType (T t) -> std::enable_if_t<std::is_enum_v<T>, std::underlying_type_t<T>>
{
    return static_cast<std::underlying_type_t<T>> (t);
}

} // namespace juce
