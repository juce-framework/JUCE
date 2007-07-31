/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
forcedinline int    jmax (const int a, const int b) throw()                                   { return (a < b) ? b : a; }
/** Returns the larger of two values. */
forcedinline int64  jmax (const int64 a, const int64 b) throw()                               { return (a < b) ? b : a; }
/** Returns the larger of two values. */
forcedinline float  jmax (const float a, const float b) throw()                               { return (a < b) ? b : a; }
/** Returns the larger of two values. */
forcedinline double jmax (const double a, const double b) throw()                             { return (a < b) ? b : a; }

/** Returns the larger of three values. */
inline int    jmax (const int a, const int b, const int c) throw()                            { return (a < b) ? ((b < c) ? c : b) : ((a < c) ? c : a); }
/** Returns the larger of three values. */
inline int64  jmax (const int64 a, const int64 b, const int64 c) throw()                      { return (a < b) ? ((b < c) ? c : b) : ((a < c) ? c : a); }
/** Returns the larger of three values. */
inline float  jmax (const float a, const float b, const float c) throw()                      { return (a < b) ? ((b < c) ? c : b) : ((a < c) ? c : a); }
/** Returns the larger of three values. */
inline double jmax (const double a, const double b, const double c) throw()                   { return (a < b) ? ((b < c) ? c : b) : ((a < c) ? c : a); }

/** Returns the larger of four values. */
inline int    jmax (const int a, const int b, const int c, const int d) throw()               { return jmax (a, jmax (b, c, d)); }
/** Returns the larger of four values. */
inline int64  jmax (const int64 a, const int64 b, const int64 c, const int64 d) throw()       { return jmax (a, jmax (b, c, d)); }
/** Returns the larger of four values. */
inline float  jmax (const float a, const float b, const float c, const float d) throw()       { return jmax (a, jmax (b, c, d)); }
/** Returns the larger of four values. */
inline double jmax (const double a, const double b, const double c, const double d) throw()   { return jmax (a, jmax (b, c, d)); }

/** Returns the smaller of two values. */
inline int    jmin (const int a, const int b) throw()                                         { return (a > b) ? b : a; }
/** Returns the smaller of two values. */
inline int64  jmin (const int64 a, const int64 b) throw()                                     { return (a > b) ? b : a; }
/** Returns the smaller of two values. */
inline float  jmin (const float a, const float b) throw()                                     { return (a > b) ? b : a; }
/** Returns the smaller of two values. */
inline double jmin (const double a, const double b) throw()                                   { return (a > b) ? b : a; }

/** Returns the smaller of three values. */
inline int    jmin (const int a, const int b, const int c) throw()                            { return (a > b) ? ((b > c) ? c : b) : ((a > c) ? c : a); }
/** Returns the smaller of three values. */
inline int64  jmin (const int64 a, const int64 b, const int64 c) throw()                      { return (a > b) ? ((b > c) ? c : b) : ((a > c) ? c : a); }
/** Returns the smaller of three values. */
inline float  jmin (const float a, const float b, const float c) throw()                      { return (a > b) ? ((b > c) ? c : b) : ((a > c) ? c : a); }
/** Returns the smaller of three values. */
inline double jmin (const double a, const double b, const double c) throw()                   { return (a > b) ? ((b > c) ? c : b) : ((a > c) ? c : a); }

/** Returns the smaller of four values. */
inline int    jmin (const int a, const int b, const int c, const int d) throw()               { return jmin (a, jmin (b, c, d)); }
/** Returns the smaller of four values. */
inline int64  jmin (const int64 a, const int64 b, const int64 c, const int64 d) throw()       { return jmin (a, jmin (b, c, d)); }
/** Returns the smaller of four values. */
inline float  jmin (const float a, const float b, const float c, const float d) throw()       { return jmin (a, jmin (b, c, d)); }
/** Returns the smaller of four values. */
inline double jmin (const double a, const double b, const double c, const double d) throw()   { return jmin (a, jmin (b, c, d)); }


//==============================================================================
/** Constrains a value to keep it within a given limit.

    Note that this expects that lowerLimit <= upperLimit. If this isn't true,
    the results will be unpredictable.

    @param lowerLimit           the minimum value to return
    @param upperLimit           the maximum value to return
    @param valueToConstrain     the value to try to return
    @returns    the closest value to valueToConstrain which lies between lowerLimit
                and upperLimit (inclusive)
*/
template <class Type>
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
template <class Type>
inline void swapVariables (Type& variable1, Type& variable2) throw()
{
    const Type tempVal = variable1;
    variable1 = variable2;
    variable2 = tempVal;
}

/** Handy macro for getting the number of elements in a simple const C array.

    E.g.
    @code
    static int myArray[] = { 1, 2, 3 };

    int numElements = numElementsInArray (myArray) // returns 3
    @endcode
*/
#define numElementsInArray(a)   ((int) (sizeof (a) / sizeof ((a)[0])))

//==============================================================================
// Some useful maths functions that aren't always present with all compilers and build settings.

#if JUCE_WIN32 || defined (DOXYGEN)
  /** Using juce_hypot and juce_hypotf is easier than dealing with all the different
      versions of these functions of various platforms and compilers. */
  forcedinline double juce_hypot (double a, double b)           { return _hypot (a, b); }

  /** Using juce_hypot and juce_hypotf is easier than dealing with all the different
      versions of these functions of various platforms and compilers. */
  forcedinline float juce_hypotf (float a, float b)             { return (float) _hypot (a, b); }
#elif MACOS_10_2_OR_EARLIER
  /** Using juce_hypot and juce_hypotf is easier than dealing with all the different
      versions of these functions of various platforms and compilers. */
  forcedinline double juce_hypot (double a, double b)           { return hypot (a, b); }

  /** Using juce_hypot and juce_hypotf is easier than dealing with all the different
      versions of these functions of various platforms and compilers. */
  forcedinline float juce_hypotf (float a, float b)             { return (float) hypot (a, b); }
  forcedinline float sinf (const float a)                       { return (float) sin (a); }
  forcedinline float cosf (const float a)                       { return (float) cos (a); }
  forcedinline float tanf (const float a)                       { return (float) tan (a); }
  forcedinline float atan2f (const float a, const float b)      { return (float) atan2 (a, b); }
  forcedinline float sqrtf (const float a)                      { return (float) sqrt (a); }
  forcedinline float logf (const float a)                       { return (float) log (a); }
  forcedinline float powf (const float a, const float b)        { return (float) pow (a, b); }
  forcedinline float expf (const float a)                       { return (float) exp (a); }
#else
  /** Using juce_hypot and juce_hypotf is easier than dealing with all the different
      versions of these functions of various platforms and compilers. */
  forcedinline double juce_hypot (double a, double b)           { return hypot (a, b); }

  /** Using juce_hypot and juce_hypotf is easier than dealing with all the different
      versions of these functions of various platforms and compilers. */
  forcedinline float juce_hypotf (float a, float b)             { return hypotf (a, b); }
#endif

inline int64 abs64 (const int64 n) throw()                      { return (n >= 0) ? n : -n; }


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
/** The isfinite() method seems to vary greatly between platforms, so this is a
    platform-independent macro for it.
*/
#if JUCE_LINUX
  #define juce_isfinite(v)      std::isfinite(v)
#elif JUCE_MAC
  #if MACOS_10_2_OR_EARLIER
    #define juce_isfinite(v)    __isfinite(v)
  #elif MACOS_10_3_OR_EARLIER
    #ifdef isfinite
      #define juce_isfinite(v)    isfinite(v)
    #else
      // no idea why the isfinite macro is sometimes impossible to include, so just copy the built-in one..
      static __inline__ int juce_isfinite (double __x) { return __x == __x && __builtin_fabs (__x) != __builtin_inf(); }
    #endif
  #else
    #define juce_isfinite(v)    std::isfinite(v)
  #endif
#elif JUCE_WIN32 && ! defined (isfinite)
  #define juce_isfinite(v)      _finite(v)
#else
  #define juce_isfinite(v)      isfinite(v)
#endif


//==============================================================================

#endif   // __JUCE_MATHSFUNCTIONS_JUCEHEADER__
