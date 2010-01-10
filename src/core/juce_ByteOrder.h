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

#ifndef __JUCE_BYTEORDER_JUCEHEADER__
#define __JUCE_BYTEORDER_JUCEHEADER__


//==============================================================================
/** Contains static methods for converting the byte order between different
    endiannesses.
*/
class JUCE_API  ByteOrder
{
public:
    //==============================================================================
    /** Swaps the upper and lower bytes of a 16-bit integer. */
    static uint16 swap (uint16 value);

    /** Reverses the order of the 4 bytes in a 32-bit integer. */
    static uint32 swap (uint32 value);

    /** Reverses the order of the 8 bytes in a 64-bit integer. */
    static uint64 swap (uint64 value);

    //==============================================================================
    /** Swaps the byte order of a 16-bit int if the CPU is big-endian */
    static uint16 swapIfBigEndian (const uint16 value);

    /** Swaps the byte order of a 32-bit int if the CPU is big-endian */
    static uint32 swapIfBigEndian (const uint32 value);

    /** Swaps the byte order of a 64-bit int if the CPU is big-endian */
    static uint64 swapIfBigEndian (const uint64 value);

    /** Swaps the byte order of a 16-bit int if the CPU is little-endian */
    static uint16 swapIfLittleEndian (const uint16 value);

    /** Swaps the byte order of a 32-bit int if the CPU is little-endian */
    static uint32 swapIfLittleEndian (const uint32 value);

    /** Swaps the byte order of a 64-bit int if the CPU is little-endian */
    static uint64 swapIfLittleEndian (const uint64 value);

    //==============================================================================
    /** Turns 4 bytes into a little-endian integer. */
    static uint32 littleEndianInt (const char* const bytes);

    /** Turns 2 bytes into a little-endian integer. */
    static uint16 littleEndianShort (const char* const bytes);

    /** Turns 4 bytes into a big-endian integer. */
    static uint32 bigEndianInt (const char* const bytes);

    /** Turns 2 bytes into a big-endian integer. */
    static uint16 bigEndianShort (const char* const bytes);

    //==============================================================================
    /** Converts 3 little-endian bytes into a signed 24-bit value (which is sign-extended to 32 bits). */
    static int littleEndian24Bit (const char* const bytes);

    /** Converts 3 big-endian bytes into a signed 24-bit value (which is sign-extended to 32 bits). */
    static int bigEndian24Bit (const char* const bytes);

    /** Copies a 24-bit number to 3 little-endian bytes. */
    static void littleEndian24BitToChars (const int value, char* const destBytes);

    /** Copies a 24-bit number to 3 big-endian bytes. */
    static void bigEndian24BitToChars (const int value, char* const destBytes);

    //==============================================================================
    /** Returns true if the current CPU is big-endian. */
    static bool isBigEndian();
};


//==============================================================================
#if JUCE_USE_INTRINSICS
  #pragma intrinsic (_byteswap_ulong)
#endif

inline uint16 ByteOrder::swap (uint16 n)
{
#if JUCE_USE_INTRINSICSxxx // agh - the MS compiler has an internal error when you try to use this intrinsic!
    return (uint16) _byteswap_ushort (n);
#else
    return (uint16) ((n << 8) | (n >> 8));
#endif
}

inline uint32 ByteOrder::swap (uint32 n)
{
#if JUCE_MAC || JUCE_IPHONE
    return OSSwapInt32 (n);
#elif JUCE_GCC
    asm("bswap %%eax" : "=a"(n) : "a"(n));
    return n;
#elif JUCE_USE_INTRINSICS
    return _byteswap_ulong (n);
#else
    __asm {
        mov eax, n
        bswap eax
        mov n, eax
    }
    return n;
#endif
}

inline uint64 ByteOrder::swap (uint64 value)
{
#if JUCE_MAC || JUCE_IPHONE
    return OSSwapInt64 (value);
#elif JUCE_USE_INTRINSICS
    return _byteswap_uint64 (value);
#else
    return (((int64) swap ((uint32) value)) << 32) | swap ((uint32) (value >> 32));
#endif
}

#if JUCE_LITTLE_ENDIAN
 inline uint16 ByteOrder::swapIfBigEndian (const uint16 v)                                  { return v; }
 inline uint32 ByteOrder::swapIfBigEndian (const uint32 v)                                  { return v; }
 inline uint64 ByteOrder::swapIfBigEndian (const uint64 v)                                  { return v; }
 inline uint16 ByteOrder::swapIfLittleEndian (const uint16 v)                               { return swap (v); }
 inline uint32 ByteOrder::swapIfLittleEndian (const uint32 v)                               { return swap (v); }
 inline uint64 ByteOrder::swapIfLittleEndian (const uint64 v)                               { return swap (v); }
 inline uint32 ByteOrder::littleEndianInt (const char* const bytes)                         { return *(uint32*) bytes; }
 inline uint16 ByteOrder::littleEndianShort (const char* const bytes)                       { return *(uint16*) bytes; }
 inline uint32 ByteOrder::bigEndianInt (const char* const bytes)                            { return swap (*(uint32*) bytes); }
 inline uint16 ByteOrder::bigEndianShort (const char* const bytes)                          { return swap (*(uint16*) bytes); }
 inline bool ByteOrder::isBigEndian()                                                       { return false; }
#else
 inline uint16 ByteOrder::swapIfBigEndian (const uint16 v)                                  { return swap (v); }
 inline uint32 ByteOrder::swapIfBigEndian (const uint32 v)                                  { return swap (v); }
 inline uint64 ByteOrder::swapIfBigEndian (const uint64 v)                                  { return swap (v); }
 inline uint16 ByteOrder::swapIfLittleEndian (const uint16 v)                               { return v; }
 inline uint32 ByteOrder::swapIfLittleEndian (const uint32 v)                               { return v; }
 inline uint64 ByteOrder::swapIfLittleEndian (const uint64 v)                               { return v; }
 inline uint32 ByteOrder::littleEndianInt (const char* const bytes)                         { return swap (*(uint32*) bytes); }
 inline uint16 ByteOrder::littleEndianShort (const char* const bytes)                       { return swap (*(uint16*) bytes); }
 inline uint32 ByteOrder::bigEndianInt (const char* const bytes)                            { return *(uint32*) bytes; }
 inline uint16 ByteOrder::bigEndianShort (const char* const bytes)                          { return *(uint16*) bytes; }
 inline bool ByteOrder::isBigEndian()                                                       { return true; }
#endif

inline int  ByteOrder::littleEndian24Bit (const char* const bytes)                          { return (((int) bytes[2]) << 16) | (((uint32) (uint8) bytes[1]) << 8) | ((uint32) (uint8) bytes[0]); }
inline int  ByteOrder::bigEndian24Bit (const char* const bytes)                             { return (((int) bytes[0]) << 16) | (((uint32) (uint8) bytes[1]) << 8) | ((uint32) (uint8) bytes[2]); }
inline void ByteOrder::littleEndian24BitToChars (const int value, char* const destBytes)    { destBytes[0] = (char)(value & 0xff); destBytes[1] = (char)((value >> 8) & 0xff); destBytes[2] = (char)((value >> 16) & 0xff); }
inline void ByteOrder::bigEndian24BitToChars (const int value, char* const destBytes)       { destBytes[0] = (char)((value >> 16) & 0xff); destBytes[1] = (char)((value >> 8) & 0xff); destBytes[2] = (char)(value & 0xff); }


#endif   // __JUCE_BYTEORDER_JUCEHEADER__
