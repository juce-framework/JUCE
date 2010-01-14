/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_MATHSFUNCTIONS_JUCEHEADER__
#define __JUCE_MATHSFUNCTIONS_JUCEHEADER__

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
  /** A platform-independent macro for writing 64-bit literals, needed because
      different compilers have different syntaxes for this.

      E.g. writing literal64bit (0x1000000000) will translate to 0x1000000000LL for
      GCC, or 0x1000000000 for MSVC.
  */
  #define literal64bit(longLiteral)     ((__int64) longLiteral)
#else
  /** A platform-independent 64-bit integer type. */
  typedef long long                 int64;
  /** A platform-independent 64-bit unsigned integer type. */
  typedef unsigned long long        uint64;
  /** A platform-independent macro for writing 64-bit literals, needed because
      different compilers have different syntaxes for this.

      E.g. writing literal64bit (0x1000000000) will translate to 0x1000000000LL for
      GCC, or 0x1000000000 for MSVC.
  */
  #define literal64bit(longLiteral)     (longLiteral##LL)
#endif


#if JUCE_64BIT
  /** A signed integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  typedef int64                     pointer_sized_int;
  /** An unsigned integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  typedef uint64                    pointer_sized_uint;
#elif _MSC_VER >= 1300
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

/** A platform-independent unicode character type. */
typedef wchar_t                     juce_wchar;


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
inline Type jmin (const Type a, const Type b)                                               { return (a > b) ? b : a; }

/** Returns the smaller of three values. */
template <typename Type>
inline Type jmin (const Type a, const Type b, const Type c)                                 { return (a > b) ? ((b > c) ? c : b) : ((a > c) ? c : a); }

/** Returns the smaller of four values. */
template <typename Type>
inline Type jmin (const Type a, const Type b, const Type c, const Type d)                   { return jmin (a, jmin (b, c, d)); }


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
                    const Type valueToConstrain) throw()
{
    jassert (lowerLimit <= upperLimit); // if these are in the wrong order, results are unpredictable..

    return (valueToConstrain < lowerLimit) ? lowerLimit
                                           : ((valueToConstrain > upperLimit) ? upperLimit
                                                                              : valueToConstrain);
}

//==============================================================================
/** Handy function to swap two values over.
*/
template <typename Type>
inline void swapVariables (Type& variable1, Type& variable2)
{
    const Type tempVal = variable1;
    variable1 = variable2;
    variable2 = tempVal;
}

/** Handy function for getting the number of elements in a simple const C array.

    E.g.
    @code
    static int myArray[] = { 1, 2, 3 };

    int numElements = numElementsInArray (myArray) // returns 3
    @endcode
*/
template <typename Type>
inline int numElementsInArray (Type& array)         { return (int) (sizeof (array) / sizeof (array[0])); }

//==============================================================================
// Some useful maths functions that aren't always present with all compilers and build settings.

/** Using juce_hypot and juce_hypotf is easier than dealing with all the different
    versions of these functions of various platforms and compilers. */
inline double juce_hypot (double a, double b)
{
  #if JUCE_WINDOWS
    return _hypot (a, b);
  #else
    return hypot (a, b);
  #endif
}

/** Using juce_hypot and juce_hypotf is easier than dealing with all the different
    versions of these functions of various platforms and compilers. */
inline float juce_hypotf (float a, float b)
{
  #if JUCE_WINDOWS
    return (float) _hypot (a, b);
  #else
    return hypotf (a, b);
  #endif
}

/** 64-bit abs function. */
inline int64 abs64 (const int64 n)
{
    return (n >= 0) ? n : -n;
}


//==============================================================================
/** A predefined value for Pi, at double-precision.

    @see float_Pi
*/
const double  double_Pi  = 3.1415926535897932384626433832795;

/** A predefined value for Pi, at sngle-precision.

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
    #else
      return std::isfinite (value);
    #endif
}

//==============================================================================
/** Fast floating-point-to-integer conversion.

    This is faster than using the normal c++ cast to convert a float to an int, and
    it will round the value to the nearest integer, rather than rounding it down
    like the normal cast does.

    Note that this routine gets its speed at the expense of some accuracy, and when
    rounding values whose floating point component is exactly 0.5, odd numbers and
    even numbers will be rounded up or down differently.
*/
template <typename FloatType>
inline int roundToInt (const FloatType value) throw()
{
    union { int asInt[2]; double asDouble; } n;
    n.asDouble = ((double) value) + 6755399441055744.0;

    #if JUCE_BIG_ENDIAN
      return n.asInt [1];
    #else
      return n.asInt [0];
    #endif
}

/** Fast floating-point-to-integer conversion.

    This is a slightly slower and slightly more accurate version of roundDoubleToInt(). It works
    fine for values above zero, but negative numbers are rounded the wrong way.
*/
inline int roundToIntAccurate (const double value) throw()
{
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
inline int roundDoubleToInt (const double value) throw()
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
inline int roundFloatToInt (const float value) throw()
{
    return roundToInt (value);
}


//==============================================================================

#endif   // __JUCE_MATHSFUNCTIONS_JUCEHEADER__
