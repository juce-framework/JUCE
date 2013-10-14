/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_MATHSFUNCTIONS_H_INCLUDED
#define JUCE_MATHSFUNCTIONS_H_INCLUDED

//==============================================================================
/*
    This file sets up some handy mathematical typdefs and functions.
*/

//==============================================================================
// Definitions for the int8, int16, int32, int64 and pointer_sized_int types.

/** A platform-independent 8-bit signed integer type. */
typedef signed char                 int8;
/** A platform-independent 8-bit unsigned integer type. */
typedef unsigned char               uint8;
/** A platform-independent 16-bit signed integer type. */
typedef signed short                int16;
/** A platform-independent 16-bit unsigned integer type. */
typedef unsigned short              uint16;
/** A platform-independent 32-bit signed integer type. */
typedef signed int                  int32;
/** A platform-independent 32-bit unsigned integer type. */
typedef unsigned int                uint32;

#if JUCE_MSVC
  /** A platform-independent 64-bit integer type. */
  typedef __int64                   int64;
  /** A platform-independent 64-bit unsigned integer type. */
  typedef unsigned __int64          uint64;
#else
  /** A platform-independent 64-bit integer type. */
  typedef long long                 int64;
  /** A platform-independent 64-bit unsigned integer type. */
  typedef unsigned long long        uint64;
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
  typedef int64                     pointer_sized_int;
  /** An unsigned integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  typedef uint64                    pointer_sized_uint;
#elif JUCE_MSVC
  /** A signed integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  typedef _W64 int                  pointer_sized_int;
  /** An unsigned integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  typedef _W64 unsigned int         pointer_sized_uint;
#else
  /** A signed integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  typedef int                       pointer_sized_int;
  /** An unsigned integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  typedef unsigned int              pointer_sized_uint;
#endif

#if JUCE_MSVC
  typedef pointer_sized_int ssize_t;
#endif

//==============================================================================
// Some indispensible min/max functions

/** Returns the larger of two values. */
template <typename Type>
inline Type jmax (const Type a, const Type b)                                               { return (a < b) ? b : a; }

/** Returns the larger of three values. */
template <typename Type>
inline Type jmax (const Type a, const Type b, const Type c)                                 { return (a < b) ? ((b < c) ? c : b) : ((a < c) ? c : a); }

/** Returns the larger of four values. */
template <typename Type>
inline Type jmax (const Type a, const Type b, const Type c, const Type d)                   { return jmax (a, jmax (b, c, d)); }

/** Returns the smaller of two values. */
template <typename Type>
inline Type jmin (const Type a, const Type b)                                               { return (b < a) ? b : a; }

/** Returns the smaller of three values. */
template <typename Type>
inline Type jmin (const Type a, const Type b, const Type c)                                 { return (b < a) ? ((c < b) ? c : b) : ((c < a) ? c : a); }

/** Returns the smaller of four values. */
template <typename Type>
inline Type jmin (const Type a, const Type b, const Type c, const Type d)                   { return jmin (a, jmin (b, c, d)); }

/** Scans an array of values, returning the minimum value that it contains. */
template <typename Type>
const Type findMinimum (const Type* data, int numValues)
{
    if (numValues <= 0)
        return Type();

    Type result (*data++);

    while (--numValues > 0) // (> 0 rather than >= 0 because we've already taken the first sample)
    {
        const Type& v = *data++;
        if (v < result)  result = v;
    }

    return result;
}

/** Scans an array of values, returning the maximum value that it contains. */
template <typename Type>
const Type findMaximum (const Type* values, int numValues)
{
    if (numValues <= 0)
        return Type();

    Type result (*values++);

    while (--numValues > 0) // (> 0 rather than >= 0 because we've already taken the first sample)
    {
        const Type& v = *values++;
        if (result < v)  result = v;
    }

    return result;
}

/** Scans an array of values, returning the minimum and maximum values that it contains. */
template <typename Type>
void findMinAndMax (const Type* values, int numValues, Type& lowest, Type& highest)
{
    if (numValues <= 0)
    {
        lowest = Type();
        highest = Type();
    }
    else
    {
        Type mn (*values++);
        Type mx (mn);

        while (--numValues > 0) // (> 0 rather than >= 0 because we've already taken the first sample)
        {
            const Type& v = *values++;

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
    @see jlimit0To, jmin, jmax
*/
template <typename Type>
inline Type jlimit (const Type lowerLimit,
                    const Type upperLimit,
                    const Type valueToConstrain) noexcept
{
    jassert (lowerLimit <= upperLimit); // if these are in the wrong order, results are unpredictable..

    return (valueToConstrain < lowerLimit) ? lowerLimit
                                           : ((upperLimit < valueToConstrain) ? upperLimit
                                                                              : valueToConstrain);
}

/** Returns true if a value is at least zero, and also below a specified upper limit.
    This is basically a quicker way to write:
    @code valueToTest >= 0 && valueToTest < upperLimit
    @endcode
*/
template <typename Type>
inline bool isPositiveAndBelow (Type valueToTest, Type upperLimit) noexcept
{
    jassert (Type() <= upperLimit); // makes no sense to call this if the upper limit is itself below zero..
    return Type() <= valueToTest && valueToTest < upperLimit;
}

template <>
inline bool isPositiveAndBelow (const int valueToTest, const int upperLimit) noexcept
{
    jassert (upperLimit >= 0); // makes no sense to call this if the upper limit is itself below zero..
    return static_cast <unsigned int> (valueToTest) < static_cast <unsigned int> (upperLimit);
}

/** Returns true if a value is at least zero, and also less than or equal to a specified upper limit.
    This is basically a quicker way to write:
    @code valueToTest >= 0 && valueToTest <= upperLimit
    @endcode
*/
template <typename Type>
inline bool isPositiveAndNotGreaterThan (Type valueToTest, Type upperLimit) noexcept
{
    jassert (Type() <= upperLimit); // makes no sense to call this if the upper limit is itself below zero..
    return Type() <= valueToTest && valueToTest <= upperLimit;
}

template <>
inline bool isPositiveAndNotGreaterThan (const int valueToTest, const int upperLimit) noexcept
{
    jassert (upperLimit >= 0); // makes no sense to call this if the upper limit is itself below zero..
    return static_cast <unsigned int> (valueToTest) <= static_cast <unsigned int> (upperLimit);
}

//==============================================================================
/** Handy function to swap two values. */
template <typename Type>
inline void swapVariables (Type& variable1, Type& variable2)
{
    std::swap (variable1, variable2);
}

/** Handy function for getting the number of elements in a simple const C array.
    E.g.
    @code
    static int myArray[] = { 1, 2, 3 };

    int numElements = numElementsInArray (myArray) // returns 3
    @endcode
*/
template <typename Type, int N>
inline int numElementsInArray (Type (&array)[N])
{
    (void) array; // (required to avoid a spurious warning in MS compilers)
    (void) sizeof (0[array]); // This line should cause an error if you pass an object with a user-defined subscript operator
    return N;
}

//==============================================================================
// Some useful maths functions that aren't always present with all compilers and build settings.

/** Using juce_hypot is easier than dealing with the different types of hypot function
    that are provided by the various platforms and compilers. */
template <typename Type>
inline Type juce_hypot (Type a, Type b) noexcept
{
   #if JUCE_MSVC
    return static_cast <Type> (_hypot (a, b));
   #else
    return static_cast <Type> (hypot (a, b));
   #endif
}

/** 64-bit abs function. */
inline int64 abs64 (const int64 n) noexcept
{
    return (n >= 0) ? n : -n;
}

#if JUCE_MSVC && ! defined (DOXYGEN)  // The MSVC libraries omit these functions for some reason...
 template<typename Type> Type asinh (Type x) noexcept  { return std::log (x + std::sqrt (x * x + (Type) 1)); }
 template<typename Type> Type acosh (Type x) noexcept  { return std::log (x + std::sqrt (x * x - (Type) 1)); }
 template<typename Type> Type atanh (Type x) noexcept  { return (std::log (x + (Type) 1) - std::log (((Type) 1) - x)) / (Type) 2; }
#endif

//==============================================================================
/** A predefined value for Pi, at double-precision.
    @see float_Pi
*/
const double  double_Pi  = 3.1415926535897932384626433832795;

/** A predefined value for Pi, at single-precision.
    @see double_Pi
*/
const float   float_Pi   = 3.14159265358979323846f;


//==============================================================================
/** The isfinite() method seems to vary between platforms, so this is a
    platform-independent function for it.
*/
template <typename FloatingPointType>
inline bool juce_isfinite (FloatingPointType value)
{
   #if JUCE_WINDOWS
    return _finite (value);
   #elif JUCE_ANDROID
    return isfinite (value);
   #else
    return std::isfinite (value);
   #endif
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
inline int roundToInt (const FloatType value) noexcept
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

#if JUCE_MSVC
 #ifndef __INTEL_COMPILER
  #pragma float_control (pop)
 #endif
 #pragma optimize ("", on)  // resets optimisations to the project defaults
#endif

/** Fast floating-point-to-integer conversion.

    This is a slightly slower and slightly more accurate version of roundDoubleToInt(). It works
    fine for values above zero, but negative numbers are rounded the wrong way.
*/
inline int roundToIntAccurate (const double value) noexcept
{
   #ifdef __INTEL_COMPILER
    #pragma float_control (pop)
   #endif

    return roundToInt (value + 1.5e-8);
}

/** Fast floating-point-to-integer conversion.

    This is faster than using the normal c++ cast to convert a double to an int, and
    it will round the value to the nearest integer, rather than rounding it down
    like the normal cast does.

    Note that this routine gets its speed at the expense of some accuracy, and when
    rounding values whose floating point component is exactly 0.5, odd numbers and
    even numbers will be rounded up or down differently. For a more accurate conversion,
    see roundDoubleToIntAccurate().
*/
inline int roundDoubleToInt (const double value) noexcept
{
    return roundToInt (value);
}

/** Fast floating-point-to-integer conversion.

    This is faster than using the normal c++ cast to convert a float to an int, and
    it will round the value to the nearest integer, rather than rounding it down
    like the normal cast does.

    Note that this routine gets its speed at the expense of some accuracy, and when
    rounding values whose floating point component is exactly 0.5, odd numbers and
    even numbers will be rounded up or down differently.
*/
inline int roundFloatToInt (const float value) noexcept
{
    return roundToInt (value);
}

//==============================================================================
/** Returns true if the specified integer is a power-of-two.
*/
template <typename IntegerType>
bool isPowerOfTwo (IntegerType value)
{
   return (value & (value - 1)) == 0;
}

/** Returns the smallest power-of-two which is equal to or greater than the given integer.
*/
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
NumericType square (NumericType n) noexcept
{
    return n * n;
}

//==============================================================================
#if (JUCE_INTEL && JUCE_32BIT) || defined (DOXYGEN)
 /** This macro can be applied to a float variable to check whether it contains a denormalised
     value, and to normalise it if necessary.
     On CPUs that aren't vulnerable to denormalisation problems, this will have no effect.
 */
 #define JUCE_UNDENORMALISE(x)   x += 1.0f; x -= 1.0f;
#else
 #define JUCE_UNDENORMALISE(x)
#endif

//==============================================================================
/** This namespace contains a few template classes for helping work out class type variations.
*/
namespace TypeHelpers
{
   #if JUCE_VC8_OR_EARLIER
    #define PARAMETER_TYPE(type) const type&
   #else
    /** The ParameterType struct is used to find the best type to use when passing some kind
        of object as a parameter.

        Of course, this is only likely to be useful in certain esoteric template situations.

        Because "typename TypeHelpers::ParameterType<SomeClass>::type" is a bit of a mouthful, there's
        a PARAMETER_TYPE(SomeClass) macro that you can use to get the same effect.

        E.g. "myFunction (PARAMETER_TYPE (int), PARAMETER_TYPE (MyObject))"
        would evaluate to "myfunction (int, const MyObject&)", keeping any primitive types as
        pass-by-value, but passing objects as a const reference, to avoid copying.
    */
    template <typename Type> struct ParameterType                   { typedef const Type& type; };

   #if ! DOXYGEN
    template <typename Type> struct ParameterType <Type&>           { typedef Type& type; };
    template <typename Type> struct ParameterType <Type*>           { typedef Type* type; };
    template <>              struct ParameterType <char>            { typedef char type; };
    template <>              struct ParameterType <unsigned char>   { typedef unsigned char type; };
    template <>              struct ParameterType <short>           { typedef short type; };
    template <>              struct ParameterType <unsigned short>  { typedef unsigned short type; };
    template <>              struct ParameterType <int>             { typedef int type; };
    template <>              struct ParameterType <unsigned int>    { typedef unsigned int type; };
    template <>              struct ParameterType <long>            { typedef long type; };
    template <>              struct ParameterType <unsigned long>   { typedef unsigned long type; };
    template <>              struct ParameterType <int64>           { typedef int64 type; };
    template <>              struct ParameterType <uint64>          { typedef uint64 type; };
    template <>              struct ParameterType <bool>            { typedef bool type; };
    template <>              struct ParameterType <float>           { typedef float type; };
    template <>              struct ParameterType <double>          { typedef double type; };
   #endif

    /** A helpful macro to simplify the use of the ParameterType template.
        @see ParameterType
    */
    #define PARAMETER_TYPE(a)    typename TypeHelpers::ParameterType<a>::type
   #endif


    /** These templates are designed to take a type, and if it's a double, they return a double
        type; for anything else, they return a float type.
    */
    template <typename Type> struct SmallestFloatType             { typedef float  type; };
    template <>              struct SmallestFloatType <double>    { typedef double type; };
}


//==============================================================================

#endif   // JUCE_MATHSFUNCTIONS_H_INCLUDED
