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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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
#if JUCE_MSVC
 #pragma pack (push, 1)
#endif

class PixelRGB;
class PixelAlpha;

forcedinline constexpr uint32 maskPixelComponents (uint32 x) noexcept
{
    return (x >> 8) & 0x00ff00ff;
}

forcedinline constexpr uint32 clampPixelComponents (uint32 x) noexcept
{
    return (x | (0x01000100 - maskPixelComponents (x))) & 0x00ff00ff;
}

//==============================================================================
/** Identifiers for specifying the blending mode.

    In the descriptions below R denotes the resulting colour, S is the source colour,
    D is the destination colour, Sa and Da are the alpha components of S and D.

    @see Graphics::setImageBlendMode
*/
enum class BlendMode
{
    sourceOver,     ///< R = S + D * (1 - Sa)
    source,         ///< R = S
    destinationIn,  ///< R = D * Sa
    destinationOut  ///< R = D * (1 - Sa)
};

//==============================================================================
/**
    Represents a 32-bit INTERNAL pixel with premultiplied alpha, and can perform compositing
    operations with it.

    This is used internally by the imaging classes.

    @see PixelRGB

    @tags{Graphics}
*/
class JUCE_API  PixelARGB
{
public:
    /** Creates a pixel without defining its colour. */
    PixelARGB() noexcept = default;

    /** Creates a pixel. */
    forcedinline constexpr PixelARGB (uint8 a, uint8 r, uint8 g, uint8 b) noexcept
        : internal (  shiftedComponent (indexA, a)
                    | shiftedComponent (indexR, r)
                    | shiftedComponent (indexG, g)
                    | shiftedComponent (indexB, b))
    {
    }

    //==============================================================================
    /** Returns a uint32 which represents the pixel in a platform dependent format. */
    forcedinline constexpr uint32 getNativeARGB() const noexcept   { return internal; }

    /** Returns a uint32 which will be in argb order as if constructed with the following mask operation
        ((alpha << 24) | (red << 16) | (green << 8) | blue). */
    forcedinline constexpr uint32 getInARGBMaskOrder() const noexcept
    {
       #if JUCE_ANDROID
        return (uint32) ((getAlpha() << 24) | (getRed() << 16) | (getGreen() << 8) | (getBlue() << 0));
       #else
        return getNativeARGB();
       #endif
    }

    /** Returns a uint32 which when written to memory, will be in the order a, r, g, b. In other words,
        if the return-value is read as a uint8 array then the elements will be in the order of a, r, g, b*/
    forcedinline constexpr uint32 getInARGBMemoryOrder() const noexcept
    {
       #if JUCE_BIG_ENDIAN
        return getInARGBMaskOrder();
       #else
        return (uint32) ((getBlue() << 24) | (getGreen() << 16) | (getRed() << 8) | getAlpha());
       #endif
    }

    /** Return channels with an even index and insert zero bytes between them. This is useful for blending
        operations. The exact channels which are returned is platform dependent. */
    forcedinline constexpr uint32 getEvenBytes() const noexcept { return getEvenBytes (internal); }

    /** Return channels with an odd index and insert zero bytes between them. This is useful for blending
        operations. The exact channels which are returned is platform dependent. */
    forcedinline constexpr uint32 getOddBytes() const noexcept  { return getEvenBytes (internal >> 8); }

    //==============================================================================
    forcedinline constexpr uint8 getAlpha() const noexcept      { return getComponent (indexA); }
    forcedinline constexpr uint8 getRed()   const noexcept      { return getComponent (indexR); }
    forcedinline constexpr uint8 getGreen() const noexcept      { return getComponent (indexG); }
    forcedinline constexpr uint8 getBlue()  const noexcept      { return getComponent (indexB); }

    //==============================================================================
    /** Copies another pixel colour over this one.

        This doesn't blend it - this colour is simply replaced by the other one.
    */
    template <class Pixel>
    forcedinline constexpr void set (Pixel src) noexcept
    {
        internal = src.getNativeARGB();
    }

    //==============================================================================
    /** Sets the pixel's colour from individual components. */
    forcedinline constexpr void setARGB (uint8 a, uint8 r, uint8 g, uint8 b) noexcept
    {
        *this = PixelARGB { a, r, g, b };
    }

    //==============================================================================
    /** Blends another pixel onto this one using the specified blend mode.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    template <class Pixel>
    forcedinline constexpr void blend (Pixel src, BlendMode mode) noexcept
    {
        blend (src, mode, 256);
    }

    /** Blends another pixel onto this one.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    template <class Pixel>
    forcedinline constexpr void blend (Pixel src) noexcept
    {
        blend (src, BlendMode::sourceOver);
    }

    /** Blends another pixel onto this one.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    forcedinline constexpr void blend (PixelRGB src) noexcept;

    /** Blends another pixel onto this one, applying an extra multiplier to its opacity.

        The opacity of the pixel being overlaid is scaled by the extraAlpha factor before
        being used, so this can blend semi-transparently from a PixelRGB argument.

        Scaling with extraAlpha is done using lossy, fixed-point arithmetic, where full opacity
        is represented by 256.
    */
    template <class Pixel>
    forcedinline constexpr void blend (Pixel src, BlendMode mode, uint32 extraAlpha) noexcept
    {
        auto srcRB = src.getEvenBytes();
        auto srcAG = src.getOddBytes();

        if (extraAlpha != 256u)
        {
            srcRB = maskPixelComponents (extraAlpha * srcRB);
            srcAG = maskPixelComponents (extraAlpha * srcAG);
        }

        const auto srcAlpha = srcAG >> 16;

        switch (mode)
        {
            case BlendMode::sourceOver:
            {
                const uint32 invAlpha = 255u - srcAlpha;
                const auto destRB = getEvenBytes() * invAlpha;
                const auto destAG = getOddBytes()  * invAlpha;
                const auto corrRB = (destRB >> 8) & 0x00ff00ffu;
                const auto corrAG = (destAG >> 8) & 0x00ff00ffu;
                const auto newDestRB = srcRB + ((destRB + corrRB + 0x00800080u) >> 8);
                const auto newDestAG = srcAG + ((destAG + corrAG + 0x00800080u) >> 8);

                const auto rb = getEvenBytes (newDestRB);
                const auto ag = getEvenBytes (newDestAG);
                internal = (ag << 8) | rb;
                return;
            }

            case BlendMode::source:
            {
                internal = (srcAG << 8) | srcRB;
                return;
            }

            case BlendMode::destinationIn:
            {
                const auto destRB = getEvenBytes() * srcAlpha;
                const auto destAG = getOddBytes()  * srcAlpha;
                const auto corrRB = (destRB >> 8) & 0x00ff00ffu;
                const auto corrAG = (destAG >> 8) & 0x00ff00ffu;
                const auto newDestRB = (destRB + corrRB + 0x00800080u) >> 8;
                const auto newDestAG = (destAG + corrAG + 0x00800080u) >> 8;

                const auto rb = getEvenBytes (newDestRB);
                const auto ag = getEvenBytes (newDestAG);
                internal = (ag << 8) | rb;
                return;
            }

            case BlendMode::destinationOut:
            {
                const uint32 invAlpha = 255u - srcAlpha;
                const auto destRB = getEvenBytes() * invAlpha;
                const auto destAG = getOddBytes()  * invAlpha;
                const auto corrRB = (destRB >> 8) & 0x00ff00ffu;
                const auto corrAG = (destAG >> 8) & 0x00ff00ffu;
                const auto newDestRB = (destRB + corrRB + 0x00800080u) >> 8;
                const auto newDestAG = (destAG + corrAG + 0x00800080u) >> 8;

                const auto rb = getEvenBytes (newDestRB);
                const auto ag = getEvenBytes (newDestAG);
                internal = (ag << 8) | rb;
                return;
            }
        }
    }

    template <class Pixel>
    forcedinline constexpr void blend (Pixel src, uint32 extraAlpha) noexcept
    {
        blend (src, BlendMode::sourceOver, extraAlpha);
    }

    /** Blends another pixel with this one, creating a colour that is somewhere
        between the two, as specified by the amount.
    */
    template <class Pixel>
    forcedinline constexpr void tween (Pixel src, uint32 amount) noexcept
    {
        auto dEvenBytes = getEvenBytes();
        dEvenBytes += (((src.getEvenBytes() - dEvenBytes) * amount) >> 8);
        dEvenBytes &= 0x00ff00ff;

        auto dOddBytes = getOddBytes();
        dOddBytes += (((src.getOddBytes() - dOddBytes) * amount) >> 8);
        dOddBytes &= 0x00ff00ff;
        dOddBytes <<= 8;

        dOddBytes |= dEvenBytes;
        internal = dOddBytes;
    }

    //==============================================================================
    /** Replaces the colour's alpha value with another one. */
    forcedinline constexpr void setAlpha (uint8 newAlpha) noexcept
    {
        setComponent (indexA, newAlpha);
    }

    /** Multiplies the colour's alpha value with another one. */
    forcedinline constexpr void multiplyAlpha (int multiplier) noexcept
    {
        // increment alpha by 1, so that if multiplier == 255 (full alpha),
        // this function will not change the values.
        ++multiplier;

        internal = ((((uint32) multiplier) * getOddBytes()) & 0xff00ff00)
                | (((((uint32) multiplier) * getEvenBytes()) >> 8) & 0x00ff00ff);
    }

    forcedinline constexpr void multiplyAlpha (float multiplier) noexcept
    {
        multiplyAlpha ((int) (multiplier * 255.0f));
    }


    forcedinline constexpr PixelARGB getUnpremultiplied() const noexcept
    {
        PixelARGB p { *this };
        p.unpremultiply();
        return p;
    }

    /** Premultiplies the pixel's RGB values by its alpha. */
    forcedinline constexpr void premultiply() noexcept
    {
        const auto alpha = getAlpha();

        if (alpha == 0xff)
            return;

        if (alpha == 0)
        {
            setComponent (indexB, 0);
            setComponent (indexG, 0);
            setComponent (indexR, 0);
        }
        else
        {
            setComponent (indexB, (uint8) ((getBlue()  * alpha + 0x7f) >> 8));
            setComponent (indexG, (uint8) ((getGreen() * alpha + 0x7f) >> 8));
            setComponent (indexR, (uint8) ((getRed()   * alpha + 0x7f) >> 8));
        }
    }

    /** Unpremultiplies the pixel's RGB values. */
    forcedinline constexpr void unpremultiply() noexcept
    {
        const auto alpha = getAlpha();

        if (alpha == 0xff)
            return;

        if (alpha == 0)
        {
            setComponent (indexB, 0);
            setComponent (indexG, 0);
            setComponent (indexR, 0);
        }
        else
        {
            setComponent (indexB, (uint8) jmin ((uint32) 0xffu, (getBlue()  * 0xffu) / alpha));
            setComponent (indexG, (uint8) jmin ((uint32) 0xffu, (getGreen() * 0xffu) / alpha));
            setComponent (indexR, (uint8) jmin ((uint32) 0xffu, (getRed()   * 0xffu) / alpha));
        }
    }

    forcedinline constexpr void desaturate() noexcept
    {
        const auto alpha = getAlpha();
        const auto red   = getRed();
        const auto green = getGreen();
        const auto blue  = getBlue();

        if (0 < alpha && alpha < 0xff)
        {
            const auto newUnpremultipliedLevel = (0xff * ((int) red + (int) green + (int) blue) / (3 * alpha));

            const auto newValue = (uint8) ((newUnpremultipliedLevel * alpha + 0x7f) >> 8);
            setComponent (indexB, newValue);
            setComponent (indexG, newValue);
            setComponent (indexR, newValue);
        }
        else
        {
            const auto newValue = (uint8) (((int) red + (int) green + (int) blue) / 3);
            setComponent (indexB, newValue);
            setComponent (indexG, newValue);
            setComponent (indexR, newValue);
        }
    }

    //==============================================================================
    /** The indexes of the different components in the byte layout of this type of colour. */
  #if JUCE_ANDROID
   #if JUCE_BIG_ENDIAN
    enum { indexA = 0, indexR = 3, indexG = 2, indexB = 1 };
   #else
    enum { indexA = 3, indexR = 0, indexG = 1, indexB = 2 };
   #endif
  #else
   #if JUCE_BIG_ENDIAN
    enum { indexA = 0, indexR = 1, indexG = 2, indexB = 3 };
   #else
    enum { indexA = 3, indexR = 2, indexG = 1, indexB = 0 };
   #endif
  #endif

private:
    //==============================================================================
    static forcedinline constexpr int shift (int index) noexcept
    {
       #if JUCE_BIG_ENDIAN
        return (3 - index) * 8;
       #else
        return index * 8;
       #endif
    }

    forcedinline constexpr uint8 getComponent (int index) const noexcept
    {
        return static_cast<uint8> (internal >> shift (index));
    }

    static forcedinline constexpr uint32 shiftedComponent (int index, uint8 value) noexcept
    {
        return static_cast<uint32> (value << shift (index));
    }

    forcedinline constexpr void setComponent (int index, uint8 value) noexcept
    {
        const auto mask = ~ shiftedComponent (index, 0xff);
        internal = (internal & mask) | shiftedComponent (index, value);
    }

    static forcedinline constexpr uint32 getEvenBytes (uint32 input) noexcept
    {
        return input & 0x00ff00ff;
    }

    uint32 internal;
}
/** @cond */
JUCE_PACKED
/** @endcond */
;

//==============================================================================
/**
    Represents a 24-bit RGB pixel, and can perform compositing operations on it.

    This is used internally by the imaging classes.

    @see PixelARGB

    @tags{Graphics}
*/
class JUCE_API  PixelRGB
{
public:
    /** Creates a pixel without defining its colour. */
    PixelRGB() noexcept = default;

    /** Creates a pixel. */
    forcedinline constexpr PixelRGB (uint8 red, uint8 green, uint8 blue) noexcept
       :
        #if JUCE_MAC || JUCE_IOS
         r (red), g (green), b (blue)
        #else
         b (blue), g (green), r (red)
        #endif
    {
    }

    //==============================================================================
    /** Returns a uint32 which represents the pixel in a platform dependent format which is compatible
        with the native format of a PixelARGB.

        @see PixelARGB::getNativeARGB */
    forcedinline constexpr uint32 getNativeARGB() const noexcept
    {
        return PixelARGB { 0xffu, r, g, b }.getNativeARGB();
    }

    /** Returns a uint32 which will be in argb order as if constructed with the following mask operation
        ((alpha << 24) | (red << 16) | (green << 8) | blue). */
    forcedinline constexpr uint32 getInARGBMaskOrder() const noexcept
    {
        return PixelARGB { 0xffu, r, g, b }.getInARGBMaskOrder();
    }

    /** Returns a uint32 which when written to memory, will be in the order a, r, g, b. In other words,
        if the return-value is read as a uint8 array then the elements will be in the order of a, r, g, b*/
    forcedinline constexpr uint32 getInARGBMemoryOrder() const noexcept
    {
        return PixelARGB { 0xffu, r, g, b }.getInARGBMemoryOrder();
    }

    /** Return channels with an even index and insert zero bytes between them. This is useful for blending
        operations. The exact channels which are returned is platform dependent but compatible with the
        return value of getEvenBytes of the PixelARGB class.

        @see PixelARGB::getEvenBytes */
    forcedinline constexpr uint32 getEvenBytes() const noexcept
    {
       #if JUCE_ANDROID
        return (uint32) (r | (b << 16));
       #else
        return (uint32) (b | (r << 16));
       #endif
    }

    /** Return channels with an odd index and insert zero bytes between them. This is useful for blending
        operations. The exact channels which are returned is platform dependent but compatible with the
        return value of getOddBytes of the PixelARGB class.

        @see PixelARGB::getOddBytes */
    forcedinline constexpr uint32 getOddBytes() const noexcept       { return (uint32) 0xff0000 | g; }

    //==============================================================================
    forcedinline constexpr uint8 getAlpha() const noexcept    { return 0xff; }
    forcedinline constexpr uint8 getRed()   const noexcept    { return r; }
    forcedinline constexpr uint8 getGreen() const noexcept    { return g; }
    forcedinline constexpr uint8 getBlue()  const noexcept    { return b; }

    //==============================================================================
    /** Copies another pixel colour over this one.

        This doesn't blend it - this colour is simply replaced by the other one.
        Because PixelRGB has no alpha channel, any alpha value in the source pixel
        is thrown away.
    */
    template <class Pixel>
    forcedinline constexpr void set (Pixel src) noexcept
    {
        *this = PixelRGB { src.getRed(), src.getGreen(), src.getBlue() };
    }

    /** Sets the pixel's colour from individual components. */
    forcedinline constexpr void setARGB (uint8, uint8 red, uint8 green, uint8 blue) noexcept
    {
        *this = PixelRGB { red, green, blue };
    }

    //==============================================================================
    /** Blends another pixel onto this one.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    template <class Pixel>
    forcedinline constexpr void blend (Pixel src) noexcept
    {
        const auto alpha = (uint32) (0x100 - src.getAlpha());

        // getEvenBytes returns 0x00rr00bb on non-android
        const auto rb = clampPixelComponents (src.getEvenBytes() + maskPixelComponents (getEvenBytes() * alpha));
        // getOddBytes returns 0x00aa00gg on non-android
        const auto ag = clampPixelComponents (src.getOddBytes() + ((g * alpha) >> 8));

        g = (uint8) (ag & 0xff);

       #if JUCE_ANDROID
        b = (uint8) (rb >> 16);
        r = (uint8) (rb & 0xff);
       #else
        r = (uint8) (rb >> 16);
        b = (uint8) (rb & 0xff);
       #endif
    }

    forcedinline constexpr void blend (PixelRGB src) noexcept
    {
        set (src);
    }

    /** Blends another pixel onto this one, applying an extra multiplier to its opacity.

        The opacity of the pixel being overlaid is scaled by the extraAlpha factor before
        being used, so this can blend semi-transparently from a PixelRGB argument.
    */
    template <class Pixel>
    forcedinline constexpr void blend (Pixel src, uint32 extraAlpha) noexcept
    {
        auto ag = maskPixelComponents (extraAlpha * src.getOddBytes());
        auto rb = maskPixelComponents (extraAlpha * src.getEvenBytes());

        const auto alpha = 0x100 - (ag >> 16);

        ag = clampPixelComponents (ag + (g * alpha >> 8));
        rb = clampPixelComponents (rb + maskPixelComponents (getEvenBytes() * alpha));

        g = (uint8) (ag & 0xff);

       #if JUCE_ANDROID
        b = (uint8) (rb >> 16);
        r = (uint8) (rb & 0xff);
       #else
        r = (uint8) (rb >> 16);
        b = (uint8) (rb & 0xff);
       #endif
    }

    /** Blends another pixel onto this one with the specified blend mode and applying an
        extra multiplier to its opacity.

        The opacity of the pixel being overlaid is scaled by the extraAlpha factor before
        being used, so this can blend semi-transparently from a PixelRGB argument.

        Scaling with extraAlpha is done using lossy, fixed-point arithmetic, where full opacity
        is represented by 256.
    */
    template <class Pixel>
    forcedinline void blend (const Pixel& src, BlendMode mode, uint32 extraAlpha = 256u) noexcept
    {
        auto Srb = src.getEvenBytes();
        auto Sag = src.getOddBytes();

        if (extraAlpha != 256u)
        {
            Srb = maskPixelComponents (extraAlpha * Srb);
            Sag = maskPixelComponents (extraAlpha * Sag);
        }

        uint32 rb{}, ag{};

        switch (mode)
        {
            case BlendMode::sourceOver:
            {
                const auto oneMinusSa = 0x100 - (Sag >> 16);
                rb = clampPixelComponents (Srb + maskPixelComponents (getEvenBytes() * oneMinusSa));
                ag = clampPixelComponents (Sag + maskPixelComponents (getOddBytes() * oneMinusSa));
                break;
            }

            case BlendMode::source:
            {
                rb = Srb;
                ag = Sag;
                break;
            }

            case BlendMode::destinationIn:
            {
                const auto Sa = (Sag >> 16) + 1;
                rb = maskPixelComponents (getEvenBytes() * Sa);
                ag = maskPixelComponents (getOddBytes() * Sa);
                break;
            }

            case BlendMode::destinationOut:
            {
                const auto oneMinusSa = 0x100 - (Sag >> 16);
                rb = maskPixelComponents (getEvenBytes() * oneMinusSa);
                ag = maskPixelComponents (getOddBytes() * oneMinusSa);
                break;
            }
        }

        g = (uint8) (ag & 0xff);

       #if JUCE_ANDROID
        b = (uint8) (rb >> 16);
        r = (uint8) (rb & 0xff);
       #else
        r = (uint8) (rb >> 16);
        b = (uint8) (rb & 0xff);
       #endif
    }

    /** Blends another pixel with this one, creating a colour that is somewhere
        between the two, as specified by the amount.
    */
    template <class Pixel>
    forcedinline constexpr void tween (Pixel src, uint32 amount) noexcept
    {
        auto dEvenBytes = getEvenBytes();
        dEvenBytes += (((src.getEvenBytes() - dEvenBytes) * amount) >> 8);

        auto dOddBytes = getOddBytes();
        dOddBytes += (((src.getOddBytes() - dOddBytes) * amount) >> 8);

        g = (uint8) (dOddBytes & 0xff);  // dOddBytes =  0x00aa00gg

       #if JUCE_ANDROID
        r = (uint8) (dEvenBytes & 0xff); // dEvenBytes = 0x00bb00rr
        b = (uint8) (dEvenBytes >> 16);
       #else
        b = (uint8) (dEvenBytes & 0xff); // dEvenBytes = 0x00rr00bb
        r = (uint8) (dEvenBytes >> 16);
       #endif
    }

    //==============================================================================
    /** This method is included for compatibility with the PixelARGB class. */
    forcedinline constexpr void setAlpha (uint8) noexcept {}

    /** Multiplies the colour's alpha value with another one. */
    forcedinline constexpr void multiplyAlpha (int) noexcept {}

    /** Multiplies the colour's alpha value with another one. */
    forcedinline constexpr void multiplyAlpha (float) noexcept {}

    /** Premultiplies the pixel's RGB values by its alpha. */
    forcedinline constexpr void premultiply() noexcept {}

    /** Unpremultiplies the pixel's RGB values. */
    forcedinline constexpr void unpremultiply() noexcept {}

    forcedinline constexpr void desaturate() noexcept
    {
        r = g = b = (uint8) (((int) r + (int) g + (int) b) / 3);
    }

    //==============================================================================
    /** The indexes of the different components in the byte layout of this type of colour. */
   #if JUCE_MAC || JUCE_IOS
    enum { indexR = 0, indexG = 1, indexB = 2 };
   #else
    enum { indexR = 2, indexG = 1, indexB = 0 };
   #endif

private:
    //==============================================================================
   #if JUCE_MAC || JUCE_IOS
    uint8 r, g, b;
   #else
    uint8 b, g, r;
   #endif

}
/** @cond */
JUCE_PACKED
/** @endcond */
;

forcedinline constexpr void PixelARGB::blend (PixelRGB src) noexcept
{
    set (src);
}

//==============================================================================
/**
    Represents an 8-bit single-channel pixel, and can perform compositing operations on it.

    This is used internally by the imaging classes.

    @see PixelARGB, PixelRGB

    @tags{Graphics}
*/
class JUCE_API  PixelAlpha
{
public:
    /** Creates a pixel without defining its colour. */
    PixelAlpha() noexcept = default;

    /** Creates a pixel. */
    forcedinline constexpr PixelAlpha (uint8 alpha) noexcept
        : a (alpha)
    {
    }

    //==============================================================================
    /** Returns a uint32 which represents the pixel in a platform dependent format which is compatible
        with the native format of a PixelARGB.

        @see PixelARGB::getNativeARGB */
    forcedinline constexpr uint32 getNativeARGB() const noexcept        { return (uint32) ((a << 24) | (a << 16) | (a << 8) | a); }

    /** Returns a uint32 which will be in argb order as if constructed with the following mask operation
        ((alpha << 24) | (red << 16) | (green << 8) | blue). */
    forcedinline constexpr uint32 getInARGBMaskOrder() const noexcept   { return getNativeARGB(); }

    /** Returns a uint32 which when written to memory, will be in the order a, r, g, b. In other words,
        if the return-value is read as a uint8 array then the elements will be in the order of a, r, g, b*/
    forcedinline uint32 constexpr getInARGBMemoryOrder() const noexcept { return getNativeARGB(); }

    /** Return channels with an even index and insert zero bytes between them. This is useful for blending
        operations. The exact channels which are returned is platform dependent but compatible with the
        return value of getEvenBytes of the PixelARGB class.

        @see PixelARGB::getEvenBytes */
    forcedinline constexpr uint32 getEvenBytes() const noexcept         { return (uint32) ((a << 16) | a); }

    /** Return channels with an odd index and insert zero bytes between them. This is useful for blending
        operations. The exact channels which are returned is platform dependent but compatible with the
        return value of getOddBytes of the PixelARGB class.

        @see PixelARGB::getOddBytes */
    forcedinline constexpr uint32 getOddBytes() const noexcept          { return (uint32) ((a << 16) | a); }

    //==============================================================================
    forcedinline constexpr uint8 getAlpha() const noexcept    { return a; }
    forcedinline constexpr uint8 getRed()   const noexcept    { return 0; }
    forcedinline constexpr uint8 getGreen() const noexcept    { return 0; }
    forcedinline constexpr uint8 getBlue()  const noexcept    { return 0; }

    //==============================================================================
    /** Copies another pixel colour over this one.

        This doesn't blend it - this colour is simply replaced by the other one.
    */
    template <class Pixel>
    forcedinline constexpr void set (Pixel src) noexcept
    {
        a = src.getAlpha();
    }

    /** Sets the pixel's colour from individual components. */
    forcedinline constexpr void setARGB (uint8 a_, uint8, uint8, uint8) noexcept
    {
        a = a_;
    }

    //==============================================================================
    /** Blends another pixel onto this one.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    template <class Pixel>
    forcedinline constexpr void blend (Pixel src) noexcept
    {
        const auto srcA = src.getAlpha();
        a = (uint8) ((a * (0x100 - srcA) >> 8) + srcA);
    }

    /** Blends another pixel onto this one, applying an extra multiplier to its opacity.

        The opacity of the pixel being overlaid is scaled by the extraAlpha factor before
        being used, so this can blend semi-transparently from a PixelRGB argument.
    */
    template <class Pixel>
    forcedinline constexpr void blend (Pixel src, uint32 extraAlpha) noexcept
    {
        ++extraAlpha;
        const auto srcAlpha = (int) ((extraAlpha * src.getAlpha()) >> 8);
        a = (uint8) ((a * (0x100 - srcAlpha) >> 8) + srcAlpha);
    }

    /** Blends another pixel onto this one using the specified blend mode and applying an
        extra multiplier to its opacity.

        The opacity of the pixel being overlaid is scaled by the extraAlpha factor before
        being used, so this can blend semi-transparently from a PixelRGB argument.

        Unlike the similar function of the PixelARGB and PixelRGB classes, full opacity
        is represented by extraAlpha = 255.
    */
    template <class Pixel>
    forcedinline void blend (const Pixel& src, BlendMode mode, uint32 extraAlpha = 255u) noexcept
    {
        auto Sa = (uint32) src.getAlpha();

        if (extraAlpha != 255u)
        {
            ++extraAlpha;
            Sa = (extraAlpha * Sa) >> 8;
        }

        auto& Da = a;

        switch (mode)
        {
            case BlendMode::sourceOver:
            {
                const auto oneMinusSa = 0x100 - Sa;
                Da = (uint8) (Sa + ((Da * oneMinusSa) >> 8));
                return;
            }

            case BlendMode::source:
            {
                Da = (uint8) Sa;
                return;
            }

            case BlendMode::destinationIn:
            {
                Da = (uint8) ((Da * (Sa + 1)) >> 8);
                return;
            }

            case BlendMode::destinationOut:
            {
                const auto oneMinusSa = 0x100 - Sa;
                Da = (uint8) ((Da * oneMinusSa) >> 8);
                return;
            }
        }
        jassertfalse;
    }

    /** Blends another pixel with this one, creating a colour that is somewhere
        between the two, as specified by the amount.
    */
    template <class Pixel>
    forcedinline constexpr void tween (Pixel src, uint32 amount) noexcept
    {
        a += (uint8) (((uint32) (src.getAlpha() - a) * amount) >> 8);
    }

    //==============================================================================
    /** Replaces the colour's alpha value with another one. */
    forcedinline constexpr void setAlpha (uint8 newAlpha) noexcept
    {
        a = newAlpha;
    }

    /** Multiplies the colour's alpha value with another one. */
    forcedinline constexpr void multiplyAlpha (int multiplier) noexcept
    {
        ++multiplier;
        a = (uint8) ((a * multiplier) >> 8);
    }

    forcedinline constexpr void multiplyAlpha (float multiplier) noexcept
    {
        a = (uint8) (a * multiplier);
    }

    /** Premultiplies the pixel's RGB values by its alpha. */
    forcedinline constexpr void premultiply() noexcept {}

    /** Unpremultiplies the pixel's RGB values. */
    forcedinline constexpr void unpremultiply() noexcept {}

    forcedinline constexpr void desaturate() noexcept {}

    //==============================================================================
    /** The indexes of the different components in the byte layout of this type of colour. */
    enum { indexA = 0 };

private:
    //==============================================================================
    uint8 a;
}
/** @cond */
 JUCE_PACKED
/** @endcond */
;

#if JUCE_MSVC
 #pragma pack (pop)
#endif

} // namespace juce
