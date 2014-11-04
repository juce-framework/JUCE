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

#ifndef JUCE_BYTEORDER_H_INCLUDED
#define JUCE_BYTEORDER_H_INCLUDED


//==============================================================================
/** Contains static methods for converting the byte order between different
    endiannesses.
*/
class JUCE_API  ByteOrder
{
public:
    //==============================================================================
    /** Swaps the upper and lower bytes of a 16-bit integer. */
    static uint16 swap (uint16 value) noexcept;

    /** Reverses the order of the 4 bytes in a 32-bit integer. */
    static uint32 swap (uint32 value) noexcept;

    /** Reverses the order of the 8 bytes in a 64-bit integer. */
    static uint64 swap (uint64 value) noexcept;

    //==============================================================================
    /** Swaps the byte order of a 16-bit int if the CPU is big-endian */
    static uint16 swapIfBigEndian (uint16 value) noexcept;

    /** Swaps the byte order of a 32-bit int if the CPU is big-endian */
    static uint32 swapIfBigEndian (uint32 value) noexcept;

    /** Swaps the byte order of a 64-bit int if the CPU is big-endian */
    static uint64 swapIfBigEndian (uint64 value) noexcept;

    /** Swaps the byte order of a 16-bit int if the CPU is little-endian */
    static uint16 swapIfLittleEndian (uint16 value) noexcept;

    /** Swaps the byte order of a 32-bit int if the CPU is little-endian */
    static uint32 swapIfLittleEndian (uint32 value) noexcept;

    /** Swaps the byte order of a 64-bit int if the CPU is little-endian */
    static uint64 swapIfLittleEndian (uint64 value) noexcept;

    //==============================================================================
    /** Turns 4 bytes into a little-endian integer. */
    static uint32 littleEndianInt (const void* bytes) noexcept;

    /** Turns 8 bytes into a little-endian integer. */
    static uint64 littleEndianInt64 (const void* bytes) noexcept;

    /** Turns 2 bytes into a little-endian integer. */
    static uint16 littleEndianShort (const void* bytes) noexcept;

    /** Turns 4 bytes into a big-endian integer. */
    static uint32 bigEndianInt (const void* bytes) noexcept;

    /** Turns 8 bytes into a big-endian integer. */
    static uint64 bigEndianInt64 (const void* bytes) noexcept;

    /** Turns 2 bytes into a big-endian integer. */
    static uint16 bigEndianShort (const void* bytes) noexcept;

    //==============================================================================
    /** Converts 3 little-endian bytes into a signed 24-bit value (which is sign-extended to 32 bits). */
    static int littleEndian24Bit (const void* bytes) noexcept;

    /** Converts 3 big-endian bytes into a signed 24-bit value (which is sign-extended to 32 bits). */
    static int bigEndian24Bit (const void* bytes) noexcept;

    /** Copies a 24-bit number to 3 little-endian bytes. */
    static void littleEndian24BitToChars (int value, void* destBytes) noexcept;

    /** Copies a 24-bit number to 3 big-endian bytes. */
    static void bigEndian24BitToChars (int value, void* destBytes) noexcept;

    //==============================================================================
    /** Returns true if the current CPU is big-endian. */
    static bool isBigEndian() noexcept;

private:
    ByteOrder() JUCE_DELETED_FUNCTION;

    JUCE_DECLARE_NON_COPYABLE (ByteOrder)
};


//==============================================================================
#if JUCE_USE_MSVC_INTRINSICS && ! defined (__INTEL_COMPILER)
 #pragma intrinsic (_byteswap_ulong)
#endif

inline uint16 ByteOrder::swap (uint16 n) noexcept
{
   #if JUCE_USE_MSVC_INTRINSICSxxx // agh - the MS compiler has an internal error when you try to use this intrinsic!
    return static_cast<uint16> (_byteswap_ushort (n));
   #else
    return static_cast<uint16> ((n << 8) | (n >> 8));
   #endif
}

inline uint32 ByteOrder::swap (uint32 n) noexcept
{
   #if JUCE_MAC || JUCE_IOS
    return OSSwapInt32 (n);
   #elif JUCE_GCC && JUCE_INTEL && ! JUCE_NO_INLINE_ASM
    asm("bswap %%eax" : "=a"(n) : "a"(n));
    return n;
   #elif JUCE_USE_MSVC_INTRINSICS
    return _byteswap_ulong (n);
   #elif JUCE_MSVC && ! JUCE_NO_INLINE_ASM
    __asm {
        mov eax, n
        bswap eax
        mov n, eax
    }
    return n;
   #elif JUCE_ANDROID
    return bswap_32 (n);
   #else
    return (n << 24) | (n >> 24) | ((n & 0xff00) << 8) | ((n & 0xff0000) >> 8);
   #endif
}

inline uint64 ByteOrder::swap (uint64 value) noexcept
{
   #if JUCE_MAC || JUCE_IOS
    return OSSwapInt64 (value);
   #elif JUCE_USE_MSVC_INTRINSICS
    return _byteswap_uint64 (value);
   #else
    return (((int64) swap ((uint32) value)) << 32) | swap ((uint32) (value >> 32));
   #endif
}

#if JUCE_LITTLE_ENDIAN
 inline uint16 ByteOrder::swapIfBigEndian (const uint16 v) noexcept                                  { return v; }
 inline uint32 ByteOrder::swapIfBigEndian (const uint32 v) noexcept                                  { return v; }
 inline uint64 ByteOrder::swapIfBigEndian (const uint64 v) noexcept                                  { return v; }
 inline uint16 ByteOrder::swapIfLittleEndian (const uint16 v) noexcept                               { return swap (v); }
 inline uint32 ByteOrder::swapIfLittleEndian (const uint32 v) noexcept                               { return swap (v); }
 inline uint64 ByteOrder::swapIfLittleEndian (const uint64 v) noexcept                               { return swap (v); }
 inline uint32 ByteOrder::littleEndianInt (const void* const bytes) noexcept                         { return *static_cast<const uint32*> (bytes); }
 inline uint64 ByteOrder::littleEndianInt64 (const void* const bytes) noexcept                       { return *static_cast<const uint64*> (bytes); }
 inline uint16 ByteOrder::littleEndianShort (const void* const bytes) noexcept                       { return *static_cast<const uint16*> (bytes); }
 inline uint32 ByteOrder::bigEndianInt (const void* const bytes) noexcept                            { return swap (*static_cast<const uint32*> (bytes)); }
 inline uint64 ByteOrder::bigEndianInt64 (const void* const bytes) noexcept                          { return swap (*static_cast<const uint64*> (bytes)); }
 inline uint16 ByteOrder::bigEndianShort (const void* const bytes) noexcept                          { return swap (*static_cast<const uint16*> (bytes)); }
 inline bool ByteOrder::isBigEndian() noexcept                                                       { return false; }
#else
 inline uint16 ByteOrder::swapIfBigEndian (const uint16 v) noexcept                                  { return swap (v); }
 inline uint32 ByteOrder::swapIfBigEndian (const uint32 v) noexcept                                  { return swap (v); }
 inline uint64 ByteOrder::swapIfBigEndian (const uint64 v) noexcept                                  { return swap (v); }
 inline uint16 ByteOrder::swapIfLittleEndian (const uint16 v) noexcept                               { return v; }
 inline uint32 ByteOrder::swapIfLittleEndian (const uint32 v) noexcept                               { return v; }
 inline uint64 ByteOrder::swapIfLittleEndian (const uint64 v) noexcept                               { return v; }
 inline uint32 ByteOrder::littleEndianInt (const void* const bytes) noexcept                         { return swap (*static_cast<const uint32*> (bytes)); }
 inline uint64 ByteOrder::littleEndianInt64 (const void* const bytes) noexcept                       { return swap (*static_cast<const uint64*> (bytes)); }
 inline uint16 ByteOrder::littleEndianShort (const void* const bytes) noexcept                       { return swap (*static_cast<const uint16*> (bytes)); }
 inline uint32 ByteOrder::bigEndianInt (const void* const bytes) noexcept                            { return *static_cast<const uint32*> (bytes); }
 inline uint64 ByteOrder::bigEndianInt64 (const void* const bytes) noexcept                          { return *static_cast<const uint64*> (bytes); }
 inline uint16 ByteOrder::bigEndianShort (const void* const bytes) noexcept                          { return *static_cast<const uint16*> (bytes); }
 inline bool ByteOrder::isBigEndian() noexcept                                                       { return true; }
#endif

inline int  ByteOrder::littleEndian24Bit (const void* const bytes) noexcept                          { return (((int) static_cast<const int8*> (bytes)[2]) << 16) | (((int) static_cast<const uint8*> (bytes)[1]) << 8) | ((int) static_cast<const uint8*> (bytes)[0]); }
inline int  ByteOrder::bigEndian24Bit (const void* const bytes) noexcept                             { return (((int) static_cast<const int8*> (bytes)[0]) << 16) | (((int) static_cast<const uint8*> (bytes)[1]) << 8) | ((int) static_cast<const uint8*> (bytes)[2]); }
inline void ByteOrder::littleEndian24BitToChars (const int value, void* const destBytes) noexcept    { static_cast<uint8*> (destBytes)[0] = (uint8) value;         static_cast<uint8*> (destBytes)[1] = (uint8) (value >> 8); static_cast<uint8*> (destBytes)[2] = (uint8) (value >> 16); }
inline void ByteOrder::bigEndian24BitToChars (const int value, void* const destBytes) noexcept       { static_cast<uint8*> (destBytes)[0] = (uint8) (value >> 16); static_cast<uint8*> (destBytes)[1] = (uint8) (value >> 8); static_cast<uint8*> (destBytes)[2] = (uint8) value; }


#endif   // JUCE_BYTEORDER_H_INCLUDED
