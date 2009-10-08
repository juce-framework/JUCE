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

#ifndef __JUCE_DATACONVERSIONS_JUCEHEADER__
#define __JUCE_DATACONVERSIONS_JUCEHEADER__

#include "juce_PlatformDefs.h"

#if JUCE_USE_INTRINSICS
  #pragma intrinsic (_byteswap_ulong)
#endif

//==============================================================================
// Endianness conversions..

/** Swaps the byte-order in an integer from little to big-endianness or vice-versa. */
forcedinline uint32 swapByteOrder (uint32 n) throw()
{
#if JUCE_MAC || JUCE_IPHONE
    // Mac version
    return CFSwapInt32 (n);
#elif JUCE_GCC
    // Inpenetrable GCC version..
    asm("bswap %%eax" : "=a"(n) : "a"(n));
    return n;
#elif JUCE_USE_INTRINSICS
    // Win32 intrinsics version..
    return _byteswap_ulong (n);
#else
    // Win32 version..
    __asm {
        mov eax, n
        bswap eax
        mov n, eax
    }
    return n;
#endif
}

/** Swaps the byte-order of a 16-bit short. */
inline uint16 swapByteOrder (const uint16 n) throw()
{
#if JUCE_USE_INTRINSICSxxx // agh - the MS compiler has an internal error when you try to use this intrinsic!
    // Win32 intrinsics version..
    return (uint16) _byteswap_ushort (n);
#else
    return (uint16) ((n << 8) | (n >> 8));
#endif
}

inline uint64 swapByteOrder (const uint64 value) throw()
{
#if JUCE_MAC || JUCE_IPHONE
    return CFSwapInt64 (value);
#elif JUCE_USE_INTRINSICS
    return _byteswap_uint64 (value);
#else
    return (((int64) swapByteOrder ((uint32) value)) << 32)
            | swapByteOrder ((uint32) (value >> 32));
#endif
}

#if JUCE_LITTLE_ENDIAN
  /** Swaps the byte order of a 16-bit int if the CPU is big-endian */
  inline uint16     swapIfBigEndian (const uint16 v) throw()             { return v; }
  /** Swaps the byte order of a 32-bit int if the CPU is big-endian */
  inline uint32     swapIfBigEndian (const uint32 v) throw()             { return v; }
  /** Swaps the byte order of a 64-bit int if the CPU is big-endian */
  inline uint64     swapIfBigEndian (const uint64 v) throw()             { return v; }

  /** Swaps the byte order of a 16-bit int if the CPU is little-endian */
  inline uint16     swapIfLittleEndian (const uint16 v) throw()          { return swapByteOrder (v); }
  /** Swaps the byte order of a 32-bit int if the CPU is little-endian */
  inline uint32     swapIfLittleEndian (const uint32 v) throw()          { return swapByteOrder (v); }
  /** Swaps the byte order of a 64-bit int if the CPU is little-endian */
  inline uint64     swapIfLittleEndian (const uint64 v) throw()          { return swapByteOrder (v); }

  /** Turns 4 bytes into a little-endian integer. */
  inline uint32     littleEndianInt (const char* const bytes) throw()    { return *(uint32*) bytes; }
  /** Turns 2 bytes into a little-endian integer. */
  inline uint16     littleEndianShort (const char* const bytes) throw()  { return *(uint16*) bytes; }

  /** Turns 4 bytes into a big-endian integer. */
  inline uint32     bigEndianInt (const char* const bytes) throw()       { return swapByteOrder (*(uint32*) bytes); }
  /** Turns 2 bytes into a big-endian integer. */
  inline uint16     bigEndianShort (const char* const bytes) throw()     { return swapByteOrder (*(uint16*) bytes); }

#else
  /** Swaps the byte order of a 16-bit int if the CPU is big-endian */
  inline uint16     swapIfBigEndian (const uint16 v) throw()             { return swapByteOrder (v); }
  /** Swaps the byte order of a 32-bit int if the CPU is big-endian */
  inline uint32     swapIfBigEndian (const uint32 v) throw()             { return swapByteOrder (v); }
  /** Swaps the byte order of a 64-bit int if the CPU is big-endian */
  inline uint64     swapIfBigEndian (const uint64 v) throw()             { return swapByteOrder (v); }

  /** Swaps the byte order of a 16-bit int if the CPU is little-endian */
  inline uint16     swapIfLittleEndian (const uint16 v) throw()          { return v; }
  /** Swaps the byte order of a 32-bit int if the CPU is little-endian */
  inline uint32     swapIfLittleEndian (const uint32 v) throw()          { return v; }
  /** Swaps the byte order of a 64-bit int if the CPU is little-endian */
  inline uint64     swapIfLittleEndian (const uint64 v) throw()          { return v; }

  /** Turns 4 bytes into a little-endian integer. */
  inline uint32     littleEndianInt (const char* const bytes) throw()    { return swapByteOrder (*(uint32*) bytes); }
  /** Turns 2 bytes into a little-endian integer. */
  inline uint16     littleEndianShort (const char* const bytes) throw()  { return swapByteOrder (*(uint16*) bytes); }

  /** Turns 4 bytes into a big-endian integer. */
  inline uint32     bigEndianInt (const char* const bytes) throw()       { return *(uint32*) bytes; }
  /** Turns 2 bytes into a big-endian integer. */
  inline uint16     bigEndianShort (const char* const bytes) throw()     { return *(uint16*) bytes; }
#endif

/** Converts 3 little-endian bytes into a signed 24-bit value (which is sign-extended to 32 bits). */
inline int littleEndian24Bit (const char* const bytes) throw()                          { return (((int) bytes[2]) << 16) | (((uint32) (uint8) bytes[1]) << 8) | ((uint32) (uint8) bytes[0]); }
/** Converts 3 big-endian bytes into a signed 24-bit value (which is sign-extended to 32 bits). */
inline int bigEndian24Bit (const char* const bytes) throw()                             { return (((int) bytes[0]) << 16) | (((uint32) (uint8) bytes[1]) << 8) | ((uint32) (uint8) bytes[2]); }

/** Copies a 24-bit number to 3 little-endian bytes. */
inline void littleEndian24BitToChars (const int value, char* const destBytes) throw()   { destBytes[0] = (char)(value & 0xff); destBytes[1] = (char)((value >> 8) & 0xff); destBytes[2] = (char)((value >> 16) & 0xff); }
/** Copies a 24-bit number to 3 big-endian bytes. */
inline void bigEndian24BitToChars (const int value, char* const destBytes) throw()      { destBytes[0] = (char)((value >> 16) & 0xff); destBytes[1] = (char)((value >> 8) & 0xff); destBytes[2] = (char)(value & 0xff); }


//==============================================================================
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
    union { int asInt[2]; double asDouble; } n;
    n.asDouble = value + 6755399441055744.0;

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
inline int roundDoubleToIntAccurate (const double value) throw()
{
    return roundDoubleToInt (value + 1.5e-8);
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
    union { int asInt[2]; double asDouble; } n;
    n.asDouble = value + 6755399441055744.0;

#if JUCE_BIG_ENDIAN
    return n.asInt [1];
#else
    return n.asInt [0];
#endif
}


#endif   // __JUCE_DATACONVERSIONS_JUCEHEADER__
