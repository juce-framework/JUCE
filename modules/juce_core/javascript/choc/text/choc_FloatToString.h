//
//    ██████ ██   ██  ██████   ██████
//   ██      ██   ██ ██    ██ ██            ** Classy Header-Only Classes **
//   ██      ███████ ██    ██ ██
//   ██      ██   ██ ██    ██ ██           https://github.com/Tracktion/choc
//    ██████ ██   ██  ██████   ██████
//
//   CHOC is (C)2022 Tracktion Corporation, and is offered under the terms of the ISC license:
//
//   Permission to use, copy, modify, and/or distribute this software for any purpose with or
//   without fee is hereby granted, provided that the above copyright notice and this permission
//   notice appear in all copies. THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
//   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
//   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
//   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
//   WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
//   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef CHOC_FLOAT_TO_STRING_HEADER_INCLUDED
#define CHOC_FLOAT_TO_STRING_HEADER_INCLUDED

#include <cstring>
#include <string>
#include "../math/choc_MathHelpers.h"

namespace
{
namespace choc::text
{

//==============================================================================
/** Converts a 32-bit float to an accurate, round-trip-safe string.

    The algorithm used is "Grisu3" from the paper "Printing Floating-Point Numbers
    Quickly and Accurately with Integers" by Florian Loitsch.
*/
std::string floatToString (float value);

/** Converts a 64-bit double to an accurate, round-trip-safe string.

    The algorithm used is "Grisu3" from the paper "Printing Floating-Point Numbers
    Quickly and Accurately with Integers" by Florian Loitsch.
*/
std::string floatToString (double value);

//==============================================================================
/** Converts a 32-bit float to an accurate, round-trip-safe string.
    If maxDecimalPlaces is -1, a default is used.
    If omitDecimalPointForRoundNumbers is true, then values such as "2.0" are returned
    without the decimal point, e.g. simply "2".

    The algorithm used is "Grisu3" from the paper "Printing Floating-Point Numbers
    Quickly and Accurately with Integers" by Florian Loitsch.
*/
std::string floatToString (float value, int maxDecimalPlaces, bool omitDecimalPointForRoundNumbers = false);

/** Converts a 64-bit double to an accurate, round-trip-safe string.
    If maxDecimalPlaces is -1, a default is used.
    If omitDecimalPointForRoundNumbers is true, then values such as "2.0" are returned
    without the decimal point, e.g. simply "2".

    The algorithm used is "Grisu3" from the paper "Printing Floating-Point Numbers
    Quickly and Accurately with Integers" by Florian Loitsch.
*/
std::string floatToString (double value, int maxDecimalPlaces, bool omitDecimalPointForRoundNumbers = false);


//==============================================================================
/** Helper class containing its own buffer for converting a float or double to a string.

    The algorithm is "Grisu3" from the paper "Printing Floating-Point Numbers
    Quickly and Accurately with Integers" by Florian Loitsch.

    To use, just construct a FloatToStringBuffer with the value, and use its begin()/end()
    methods to iterate the result. Or use the floatToString() functions to just convert a
    value directly to a std::string.
*/
template <typename FloatOrDouble>
struct FloatToStringBuffer
{
    FloatToStringBuffer (FloatOrDouble value, int maxDecimalPlaces, bool omitPointIfPossible)
       : stringEnd (writeAndGetEnd (storage, value, maxDecimalPlaces, omitPointIfPossible)) {}

    const char* begin() const       { return storage; }
    const char* end() const         { return stringEnd; }

    std::string toString() const    { return std::string (begin(), end()); }

private:
    //==============================================================================
    static_assert (std::is_same<const float, const FloatOrDouble>::value || std::is_same<const double, const FloatOrDouble>::value,
                   "This class can only handle float or double template types");

    char storage[32];
    const char* stringEnd;

    struct MantissaAndExponent
    {
        uint64_t mantissa;
        int32_t exponent;

        static constexpr MantissaAndExponent create (uint64_t floatBits, uint64_t significand)
        {
            constexpr int exponentBias = (sizeof (FloatOrDouble) == 8 ? 0x3ff : 0x7f) + numSignificandBits;
            auto explonentPlusBias = static_cast<int> ((floatBits & exponentMask) >> numSignificandBits);

            return explonentPlusBias == 0 ? MantissaAndExponent { significand, 1 - exponentBias }
                                          : MantissaAndExponent { significand + hiddenBit, explonentPlusBias - exponentBias };
        }

        constexpr MantissaAndExponent operator* (MantissaAndExponent rhs) const
        {
            auto mantissaProduct = math::multiply128 (mantissa, rhs.mantissa);
            return { mantissaProduct.high + (mantissaProduct.low >> 63), exponent + rhs.exponent + 64 };
        }

        constexpr MantissaAndExponent shiftedUp (int numBits) const   { return { mantissa << numBits, exponent - numBits }; }
        constexpr MantissaAndExponent normalized() const              { return shiftedUp (static_cast<int> (math::countUpperClearBits (mantissa))); }
    };

    static uint32_t generateDigits (char* buffer, MantissaAndExponent upperBound, uint64_t mantissaDiff, uint64_t delta, int& K)
    {
        uint32_t length = 0;
        const auto one = MantissaAndExponent { 1ull << -upperBound.exponent, upperBound.exponent };
        auto p1 = static_cast<uint32_t> (upperBound.mantissa >> -one.exponent);
        auto p2 = upperBound.mantissa & (one.mantissa - 1);
        auto numDigits = math::getNumDecimalDigits (p1);

        for (;;)
        {
            auto digit = p1;

            switch (--numDigits)
            {
                case 0:                           p1 = 0;              break;
                case 1:  digit /= powersOf10[1];  p1 %= powersOf10[1]; break;
                case 2:  digit /= powersOf10[2];  p1 %= powersOf10[2]; break;
                case 3:  digit /= powersOf10[3];  p1 %= powersOf10[3]; break;
                case 4:  digit /= powersOf10[4];  p1 %= powersOf10[4]; break;
                case 5:  digit /= powersOf10[5];  p1 %= powersOf10[5]; break;
                case 6:  digit /= powersOf10[6];  p1 %= powersOf10[6]; break;
                case 7:  digit /= powersOf10[7];  p1 %= powersOf10[7]; break;
                case 8:  digit /= powersOf10[8];  p1 %= powersOf10[8]; break;
                default: break;
            }

            writeDigitIfNotLeadingZero (buffer, length, digit);
            auto rest = p2 + (static_cast<uint64_t> (p1) << -one.exponent);

            if (rest <= delta)
            {
                K += numDigits;
                roundFinalDigit (buffer, length, delta, rest, static_cast<uint64_t> (powersOf10[numDigits]) << -one.exponent, mantissaDiff);
                return length;
            }

            if (numDigits == 0)
            {
                for (;;)
                {
                    delta *= 10;
                    p2 *= 10;
                    --numDigits;
                    writeDigitIfNotLeadingZero (buffer, length, static_cast<uint32_t> (p2 >> -one.exponent));
                    p2 &= one.mantissa - 1;

                    if (p2 < delta)
                    {
                        K += numDigits;
                        roundFinalDigit (buffer, length, delta, p2, one.mantissa, numDigits > -9 ? mantissaDiff * powersOf10[-numDigits] : 0);
                        return length;
                    }
                }
            }
        }
    }

    static void roundFinalDigit (char* buffer, uint32_t length, uint64_t delta, uint64_t rest, uint64_t tenToPowerNumDigits, uint64_t diff)
    {
        while (rest < diff && delta - rest >= tenToPowerNumDigits
                && (rest + tenToPowerNumDigits < diff || diff - rest > rest + tenToPowerNumDigits - diff))
        {
            --(buffer[length - 1]);
            rest += tenToPowerNumDigits;
        }
    }

    [[nodiscard]] static char* write (char* dest, char c)                                     { *dest = c; return dest + 1; }
    template <typename... Chars> static char* write (char* dest, char first, Chars... others) { return write (write (dest, first), others...); }
    [[nodiscard]] static char* writeDigit (char* dest, int digit)                             { return write (dest, static_cast<char> (digit + '0')); }
    template <typename... Chars> static char* writeDigit (char* dest, int d, Chars... others) { return writeDigit (writeDigit (dest, d), others...); }
    [[nodiscard]] static char* writeZero (char* dest)                                         { return write (dest, '0', '.', '0'); }
    [[nodiscard]] static char* writeExponent (char* dest, int e)                              { return writeShortInteger (write (dest, 'e'), e); }
    static void writeDigitIfNotLeadingZero (char* dest, uint32_t& length, uint32_t digit)     { if (digit != 0 || length != 0) dest[length++] = static_cast<char> (digit + '0'); }

    [[nodiscard]] static char* writeShortInteger (char* dest, int n)
    {
        if (n < 0)    return writeShortInteger (write (dest, '-'), -n);
        if (n >= 100) return writeDigit (dest, n / 100, (n / 10) % 10, n % 10);
        if (n >= 10)  return writeDigit (dest, n / 10,  n % 10);

        return writeDigit (dest, n);
    }

    static void insertChar (char* dest, uint32_t length, char charToInsert, uint32_t numRepetitions)
    {
        std::memmove (dest + numRepetitions, dest, (size_t) length);

        for (uint32_t i = 0; i < numRepetitions; ++i)
            dest[i] = charToInsert;
    }

    static char* writeAsExponentNotation (char* dest, uint32_t totalLength, int exponent)
    {
        if (totalLength == 1)
            return writeExponent (dest + 1, exponent);

        insertChar (dest + 1, totalLength - 1, '.', 1);

        while (dest[totalLength] == '0' && totalLength > 2)
            --totalLength;

        return writeExponent (dest + (totalLength + 1), exponent);
    }

    static char* writeWithoutExponentLessThan1 (char* dest, uint32_t length, int mantissaDigits, int maxDecimalPlaces)
    {
        auto numPaddingZeros = static_cast<uint32_t> (2 - mantissaDigits);
        insertChar (dest, length, '0', numPaddingZeros);
        dest[1] = '.';

        if (static_cast<int> (length) > maxDecimalPlaces + mantissaDigits)
        {
            for (int i = maxDecimalPlaces + 1; i > 2; --i)
                if (dest[i] != '0')
                    return dest + (i + 1);

            return dest + 3;
        }

        length += numPaddingZeros;

        while (dest[length - 1] == '0' && length > 3)
            --length;

        return dest + length;
    }

    static char* writeWithoutExponentGreaterThan1 (char* dest, uint32_t totalLength, uint32_t mantissaLength, int maxDecimalPlaces, int K)
    {
        if (K >= 0)
        {
            dest += totalLength;

            for (auto i = totalLength; i < mantissaLength; ++i)
                dest = write (dest, '0');

            return write (dest, '.', '0');
        }

        insertChar (dest + mantissaLength, totalLength - mantissaLength, '.', 1);

        if (K + maxDecimalPlaces >= 0)
            return dest + (totalLength + 1);

        for (auto i = static_cast<int> (mantissaLength) + maxDecimalPlaces; i > static_cast<int> (mantissaLength + 1); --i)
            if (dest[i] != '0')
                return dest + (i + 1);

        return dest + (mantissaLength + 2);
    }

    struct Limits
    {
        constexpr Limits (MantissaAndExponent value)
        {
            upper = { (value.mantissa << 1) + 1, value.exponent - 1 };

            while ((upper.mantissa & (hiddenBit << 1)) == 0)
                upper = upper.shiftedUp (1);

            upper = upper.shiftedUp (static_cast<int> (sizeof (upper.mantissa) * 8 - numSignificandBits - 2));

            lower = value.mantissa == hiddenBit ? MantissaAndExponent { (value.mantissa << 2) - 1, value.exponent - 2 }
                                                : MantissaAndExponent { (value.mantissa << 1) - 1, value.exponent - 1 };
            lower.mantissa <<= lower.exponent - upper.exponent;
            lower.exponent = upper.exponent;
        }

        MantissaAndExponent lower, upper;
    };

    static const char* writeAndGetEnd (char* pos, FloatOrDouble value, int maxDecimalPlaces, bool omitPointIfPossible)
    {
        auto startPos = pos;
        auto floatBits = getFloatBits (value);

        if ((floatBits & signMask) == 0)
        {
            if (isZero (floatBits))  return writeZero (pos);
        }
        else
        {
            pos = write (pos, '-');

            if (isZero (floatBits))  return writeZero (pos);

            value = -value;
            floatBits &= ~signMask;
        }

        if (floatBits == nanBits)  return write (pos, 'n', 'a', 'n');
        if (floatBits == infBits)  return write (pos, 'i', 'n', 'f');

        auto v = MantissaAndExponent::create (floatBits, floatBits & significandMask);
        Limits limits (v);

        int K;
        auto powerOf10 = createPowerOf10 (limits.upper.exponent, K);
        auto w = powerOf10 * v.normalized();
        auto upperBound = powerOf10 * limits.upper;
        upperBound.mantissa--;
        auto lowerBound = powerOf10 * limits.lower;
        lowerBound.mantissa++;

        auto totalLength = generateDigits (pos, upperBound, upperBound.mantissa - w.mantissa, upperBound.mantissa - lowerBound.mantissa, K);
        auto end = addDecimalPointAndExponent (pos, totalLength, K, maxDecimalPlaces < 0 ? defaultNumDecimalPlaces : maxDecimalPlaces);

        if (omitPointIfPossible && end > startPos + 1 && end[-1] == '0' && end[-2] == '.')
            end -= 2;

        return end;
    }

    static const char* addDecimalPointAndExponent (char* pos, uint32_t totalLength, int K, int maxDecimalPlaces)
    {
        auto mantissaDigits = static_cast<int> (totalLength) + K;

        if (mantissaDigits < -maxDecimalPlaces)          return writeZero (pos);
        if (mantissaDigits <= 0 && mantissaDigits > -6)  return writeWithoutExponentLessThan1 (pos, totalLength, mantissaDigits, maxDecimalPlaces);
        if (mantissaDigits > 0 && mantissaDigits <= 21)  return writeWithoutExponentGreaterThan1 (pos, totalLength, static_cast<uint32_t> (mantissaDigits), maxDecimalPlaces, K);

        return writeAsExponentNotation (pos, totalLength, mantissaDigits - 1);
    }

    static uint64_t getFloatBits (double value)    { uint64_t i; memcpy (&i, &value, sizeof (i)); return i; }
    static uint64_t getFloatBits (float value)     { uint32_t i; memcpy (&i, &value, sizeof (i)); return i; }
    static bool isZero (uint64_t floatBits)        { return (floatBits & (exponentMask | significandMask)) == 0; }

    static constexpr int       defaultNumDecimalPlaces  = 324;
    static constexpr int       numSignificandBits       = sizeof (FloatOrDouble) == 8 ? 52 : 23;
    static constexpr uint64_t  signMask                 = 1ull << (sizeof (FloatOrDouble) * 8 - 1);
    static constexpr uint64_t  hiddenBit                = 1ull << numSignificandBits;
    static constexpr uint64_t  significandMask          = hiddenBit - 1;
    static constexpr uint64_t  exponentMask             = sizeof (FloatOrDouble) == 8 ? 0x7ff0000000000000ull : 0x7f800000ull;
    static constexpr uint64_t  nanBits                  = sizeof (FloatOrDouble) == 8 ? 0x7ff8000000000000ull : 0x7fc00000ull;
    static constexpr uint64_t  infBits                  = sizeof (FloatOrDouble) == 8 ? 0x7ff0000000000000ull : 0x7f800000ull;
    static constexpr uint32_t  powersOf10[]             = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

    static MantissaAndExponent createPowerOf10 (int exponentBase2, int& K)
    {
        static constexpr MantissaAndExponent powerOf10List[] =
        {
            { 0xfa8fd5a0081c0288ull, -1220 }, { 0xbaaee17fa23ebf76ull, -1193 }, { 0x8b16fb203055ac76ull, -1166 }, { 0xcf42894a5dce35eaull, -1140 }, { 0x9a6bb0aa55653b2dull, -1113 },
            { 0xe61acf033d1a45dfull, -1087 }, { 0xab70fe17c79ac6caull, -1060 }, { 0xff77b1fcbebcdc4full, -1034 }, { 0xbe5691ef416bd60cull, -1007 }, { 0x8dd01fad907ffc3cull,  -980 },
            { 0xd3515c2831559a83ull,  -954 }, { 0x9d71ac8fada6c9b5ull,  -927 }, { 0xea9c227723ee8bcbull,  -901 }, { 0xaecc49914078536dull,  -874 }, { 0x823c12795db6ce57ull,  -847 },
            { 0xc21094364dfb5637ull,  -821 }, { 0x9096ea6f3848984full,  -794 }, { 0xd77485cb25823ac7ull,  -768 }, { 0xa086cfcd97bf97f4ull,  -741 }, { 0xef340a98172aace5ull,  -715 },
            { 0xb23867fb2a35b28eull,  -688 }, { 0x84c8d4dfd2c63f3bull,  -661 }, { 0xc5dd44271ad3cdbaull,  -635 }, { 0x936b9fcebb25c996ull,  -608 }, { 0xdbac6c247d62a584ull,  -582 },
            { 0xa3ab66580d5fdaf6ull,  -555 }, { 0xf3e2f893dec3f126ull,  -529 }, { 0xb5b5ada8aaff80b8ull,  -502 }, { 0x87625f056c7c4a8bull,  -475 }, { 0xc9bcff6034c13053ull,  -449 },
            { 0x964e858c91ba2655ull,  -422 }, { 0xdff9772470297ebdull,  -396 }, { 0xa6dfbd9fb8e5b88full,  -369 }, { 0xf8a95fcf88747d94ull,  -343 }, { 0xb94470938fa89bcfull,  -316 },
            { 0x8a08f0f8bf0f156bull,  -289 }, { 0xcdb02555653131b6ull,  -263 }, { 0x993fe2c6d07b7facull,  -236 }, { 0xe45c10c42a2b3b06ull,  -210 }, { 0xaa242499697392d3ull,  -183 },
            { 0xfd87b5f28300ca0eull,  -157 }, { 0xbce5086492111aebull,  -130 }, { 0x8cbccc096f5088ccull,  -103 }, { 0xd1b71758e219652cull,   -77 }, { 0x9c40000000000000ull,   -50 },
            { 0xe8d4a51000000000ull,   -24 }, { 0xad78ebc5ac620000ull,     3 }, { 0x813f3978f8940984ull,    30 }, { 0xc097ce7bc90715b3ull,    56 }, { 0x8f7e32ce7bea5c70ull,    83 },
            { 0xd5d238a4abe98068ull,   109 }, { 0x9f4f2726179a2245ull,   136 }, { 0xed63a231d4c4fb27ull,   162 }, { 0xb0de65388cc8ada8ull,   189 }, { 0x83c7088e1aab65dbull,   216 },
            { 0xc45d1df942711d9aull,   242 }, { 0x924d692ca61be758ull,   269 }, { 0xda01ee641a708deaull,   295 }, { 0xa26da3999aef774aull,   322 }, { 0xf209787bb47d6b85ull,   348 },
            { 0xb454e4a179dd1877ull,   375 }, { 0x865b86925b9bc5c2ull,   402 }, { 0xc83553c5c8965d3dull,   428 }, { 0x952ab45cfa97a0b3ull,   455 }, { 0xde469fbd99a05fe3ull,   481 },
            { 0xa59bc234db398c25ull,   508 }, { 0xf6c69a72a3989f5cull,   534 }, { 0xb7dcbf5354e9beceull,   561 }, { 0x88fcf317f22241e2ull,   588 }, { 0xcc20ce9bd35c78a5ull,   614 },
            { 0x98165af37b2153dfull,   641 }, { 0xe2a0b5dc971f303aull,   667 }, { 0xa8d9d1535ce3b396ull,   694 }, { 0xfb9b7cd9a4a7443cull,   720 }, { 0xbb764c4ca7a44410ull,   747 },
            { 0x8bab8eefb6409c1aull,   774 }, { 0xd01fef10a657842cull,   800 }, { 0x9b10a4e5e9913129ull,   827 }, { 0xe7109bfba19c0c9dull,   853 }, { 0xac2820d9623bf429ull,   880 },
            { 0x80444b5e7aa7cf85ull,   907 }, { 0xbf21e44003acdd2dull,   933 }, { 0x8e679c2f5e44ff8full,   960 }, { 0xd433179d9c8cb841ull,   986 }, { 0x9e19db92b4e31ba9ull,  1013 },
            { 0xeb96bf6ebadf77d9ull,  1039 }, { 0xaf87023b9bf0ee6bull,  1066 }
        };

        auto dk = (exponentBase2 + 61) * -0.30102999566398114;
        auto ik = static_cast<int> (dk);
        auto index = ((ik + (dk > ik ? 348 : 347)) >> 3) + 1;
        K = 348 - (index << 3);
        return powerOf10List[index];
    }
};

inline std::string floatToString (float value)                                              { return FloatToStringBuffer<float>  (value, -1, false).toString(); }
inline std::string floatToString (double value)                                             { return FloatToStringBuffer<double> (value, -1, false).toString(); }
inline std::string floatToString (float value, int maxDecimals, bool omitPointIfPossible)   { return FloatToStringBuffer<float>  (value, maxDecimals, omitPointIfPossible).toString(); }
inline std::string floatToString (double value, int maxDecimals, bool omitPointIfPossible)  { return FloatToStringBuffer<double> (value, maxDecimals, omitPointIfPossible).toString(); }

} // namespace choc::text
} // anonymous namespace

#endif
