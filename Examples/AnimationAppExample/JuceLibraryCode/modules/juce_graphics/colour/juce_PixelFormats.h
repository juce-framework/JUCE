/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_PIXELFORMATS_H_INCLUDED
#define JUCE_PIXELFORMATS_H_INCLUDED


//==============================================================================
#if JUCE_MSVC
 #pragma pack (push, 1)
#endif

class PixelRGB;
class PixelAlpha;

inline uint32 maskPixelComponents (uint32 x) noexcept
{
    return (x >> 8) & 0x00ff00ff;
}

inline uint32 clampPixelComponents (uint32 x) noexcept
{
    return (x | (0x01000100 - maskPixelComponents (x))) & 0x00ff00ff;
}

//==============================================================================
/**
    Represents a 32-bit ARGB pixel with premultiplied alpha, and can perform compositing
    operations with it.

    This is used internally by the imaging classes.

    @see PixelRGB
*/
class JUCE_API  PixelARGB
{
public:
    /** Creates a pixel without defining its colour. */
    PixelARGB() noexcept {}
    ~PixelARGB() noexcept {}

    /** Creates a pixel from a 32-bit argb value.
    */
    PixelARGB (const uint32 argbValue) noexcept
        : argb (argbValue)
    {
    }

    PixelARGB (const uint8 a, const uint8 r, const uint8 g, const uint8 b) noexcept
    {
        components.b = b;
        components.g = g;
        components.r = r;
        components.a = a;
    }

    forcedinline uint32 getARGB() const noexcept                { return argb; }
    forcedinline uint32 getUnpremultipliedARGB() const noexcept { PixelARGB p (argb); p.unpremultiply(); return p.getARGB(); }

    forcedinline uint32 getRB() const noexcept      { return 0x00ff00ff & argb; }
    forcedinline uint32 getAG() const noexcept      { return 0x00ff00ff & (argb >> 8); }

    forcedinline uint8 getAlpha() const noexcept    { return components.a; }
    forcedinline uint8 getRed() const noexcept      { return components.r; }
    forcedinline uint8 getGreen() const noexcept    { return components.g; }
    forcedinline uint8 getBlue() const noexcept     { return components.b; }

   #if JUCE_GCC && ! JUCE_CLANG
    // NB these are here as a workaround because GCC refuses to bind to packed values.
    forcedinline uint8& getAlpha() noexcept         { return comps [indexA]; }
    forcedinline uint8& getRed() noexcept           { return comps [indexR]; }
    forcedinline uint8& getGreen() noexcept         { return comps [indexG]; }
    forcedinline uint8& getBlue() noexcept          { return comps [indexB]; }
   #else
    forcedinline uint8& getAlpha() noexcept         { return components.a; }
    forcedinline uint8& getRed() noexcept           { return components.r; }
    forcedinline uint8& getGreen() noexcept         { return components.g; }
    forcedinline uint8& getBlue() noexcept          { return components.b; }
   #endif

    /** Blends another pixel onto this one.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    template <class Pixel>
    forcedinline void blend (const Pixel& src) noexcept
    {
        const uint32 alpha = 0x100 - src.getAlpha();
        uint32 rb = src.getRB() + maskPixelComponents (getRB() * alpha);
        uint32 ag = src.getAG() + maskPixelComponents (getAG() * alpha);
        argb = clampPixelComponents (rb) + (clampPixelComponents (ag) << 8);
    }

    /** Blends another pixel onto this one.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    forcedinline void blend (const PixelRGB src) noexcept;


    /** Blends another pixel onto this one, applying an extra multiplier to its opacity.

        The opacity of the pixel being overlaid is scaled by the extraAlpha factor before
        being used, so this can blend semi-transparently from a PixelRGB argument.
    */
    template <class Pixel>
    forcedinline void blend (const Pixel& src, uint32 extraAlpha) noexcept
    {
        uint32 ag = maskPixelComponents (extraAlpha * src.getAG());
        const uint32 alpha = 0x100 - (ag >> 16);
        ag += maskPixelComponents (getAG() * alpha);

        uint32 rb = maskPixelComponents (extraAlpha * src.getRB())
                     + maskPixelComponents (getRB() * alpha);

        argb = clampPixelComponents(rb) + (clampPixelComponents (ag) << 8);
    }

    /** Blends another pixel with this one, creating a colour that is somewhere
        between the two, as specified by the amount.
    */
    template <class Pixel>
    forcedinline void tween (const Pixel& src, const uint32 amount) noexcept
    {
        uint32 drb = getRB();
        drb += (((src.getRB() - drb) * amount) >> 8);
        drb &= 0x00ff00ff;

        uint32 dag = getAG();
        dag += (((src.getAG() - dag) * amount) >> 8);
        dag &= 0x00ff00ff;
        dag <<= 8;

        dag |= drb;
        argb = dag;
    }

    /** Copies another pixel colour over this one.

        This doesn't blend it - this colour is simply replaced by the other one.
    */
    template <class Pixel>
    forcedinline void set (const Pixel& src) noexcept
    {
        argb = src.getARGB();
    }

    /** Replaces the colour's alpha value with another one. */
    forcedinline void setAlpha (const uint8 newAlpha) noexcept
    {
        components.a = newAlpha;
    }

    /** Multiplies the colour's alpha value with another one. */
    forcedinline void multiplyAlpha (int multiplier) noexcept
    {
        ++multiplier;

        argb = ((((uint32) multiplier) * getAG()) & 0xff00ff00)
                | (((((uint32) multiplier) * getRB()) >> 8) & 0x00ff00ff);
    }

    forcedinline void multiplyAlpha (const float multiplier) noexcept
    {
        multiplyAlpha ((int) (multiplier * 255.0f));
    }

    /** Sets the pixel's colour from individual components. */
    void setARGB (const uint8 a, const uint8 r, const uint8 g, const uint8 b) noexcept
    {
        components.b = b;
        components.g = g;
        components.r = r;
        components.a = a;
    }

    /** Premultiplies the pixel's RGB values by its alpha. */
    forcedinline void premultiply() noexcept
    {
        const uint32 alpha = components.a;

        if (alpha < 0xff)
        {
            if (alpha == 0)
            {
                components.b = 0;
                components.g = 0;
                components.r = 0;
            }
            else
            {
                components.b = (uint8) ((components.b * alpha + 0x7f) >> 8);
                components.g = (uint8) ((components.g * alpha + 0x7f) >> 8);
                components.r = (uint8) ((components.r * alpha + 0x7f) >> 8);
            }
        }
    }

    /** Unpremultiplies the pixel's RGB values. */
    forcedinline void unpremultiply() noexcept
    {
        const uint32 alpha = components.a;

        if (alpha < 0xff)
        {
            if (alpha == 0)
            {
                components.b = 0;
                components.g = 0;
                components.r = 0;
            }
            else
            {
                components.b = (uint8) jmin ((uint32) 0xff, (components.b * 0xff) / alpha);
                components.g = (uint8) jmin ((uint32) 0xff, (components.g * 0xff) / alpha);
                components.r = (uint8) jmin ((uint32) 0xff, (components.r * 0xff) / alpha);
            }
        }
    }

    forcedinline void desaturate() noexcept
    {
        if (components.a < 0xff && components.a > 0)
        {
            const int newUnpremultipliedLevel = (0xff * ((int) components.r + (int) components.g + (int) components.b) / (3 * components.a));

            components.r = components.g = components.b
                = (uint8) ((newUnpremultipliedLevel * components.a + 0x7f) >> 8);
        }
        else
        {
            components.r = components.g = components.b
                = (uint8) (((int) components.r + (int) components.g + (int) components.b) / 3);
        }
    }

    /** Returns a uint32 which when written to memory, will be in the order r, g, b, a. */
    inline uint32 getInRGBAMemoryOrder() const noexcept
    {
       #if JUCE_BIG_ENDIAN
        return (((uint32) components.r) << 24) | (((uint32) components.g) << 16) | (((uint32) components.b) << 8) | components.a;
       #else
        return (((uint32) components.a) << 24) | (((uint32) components.b) << 16) | (((uint32) components.g) << 8) | components.r;
       #endif
    }

    //==============================================================================
    /** The indexes of the different components in the byte layout of this type of colour. */
   #if JUCE_BIG_ENDIAN
    enum { indexA = 0, indexR = 1, indexG = 2, indexB = 3 };
   #else
    enum { indexA = 3, indexR = 2, indexG = 1, indexB = 0 };
   #endif

private:
    //==============================================================================
    struct Components
    {
       #if JUCE_BIG_ENDIAN
        uint8 a, r, g, b;
       #else
        uint8 b, g, r, a;
       #endif
    } JUCE_PACKED;

    union
    {
        uint32 argb;
        Components components;
       #if JUCE_GCC
        uint8 comps[4];
       #endif
    };
}
#ifndef DOXYGEN
 JUCE_PACKED
#endif
;


//==============================================================================
/**
    Represents a 24-bit RGB pixel, and can perform compositing operations on it.

    This is used internally by the imaging classes.

    @see PixelARGB
*/
class JUCE_API  PixelRGB
{
public:
    /** Creates a pixel without defining its colour. */
    PixelRGB() noexcept {}
    ~PixelRGB() noexcept {}

    /** Creates a pixel from a 32-bit argb value.

        (The argb format is that used by PixelARGB)
    */
    PixelRGB (const uint32 argb) noexcept
    {
        r = (uint8) (argb >> 16);
        g = (uint8) (argb >> 8);
        b = (uint8) (argb);
    }

    forcedinline uint32 getARGB() const noexcept                { return 0xff000000 | b | (((uint32) g) << 8) | (((uint32) r) << 16); }
    forcedinline uint32 getUnpremultipliedARGB() const noexcept { return getARGB(); }

    forcedinline uint32 getRB() const noexcept      { return b | (uint32) (r << 16); }
    forcedinline uint32 getAG() const noexcept      { return (uint32) (0xff0000 | g); }

    forcedinline uint8 getAlpha() const noexcept    { return 0xff; }
    forcedinline uint8 getRed() const noexcept      { return r; }
    forcedinline uint8 getGreen() const noexcept    { return g; }
    forcedinline uint8 getBlue() const noexcept     { return b; }

    forcedinline uint8& getRed() noexcept           { return r; }
    forcedinline uint8& getGreen() noexcept         { return g; }
    forcedinline uint8& getBlue() noexcept          { return b; }

    /** Blends another pixel onto this one.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    template <class Pixel>
    forcedinline void blend (const Pixel& src) noexcept
    {
        const uint32 alpha = 0x100 - src.getAlpha();

        uint32 rb = clampPixelComponents (src.getRB() + maskPixelComponents (getRB() * alpha));
        uint32 ag = src.getAG() + (g * alpha >> 8);

        r = (uint8) (rb >> 16);
        g = (uint8) clampPixelComponents (ag);
        b = (uint8) rb;
    }

    forcedinline void blend (const PixelRGB src) noexcept
    {
        set (src);
    }

    /** Blends another pixel onto this one, applying an extra multiplier to its opacity.

        The opacity of the pixel being overlaid is scaled by the extraAlpha factor before
        being used, so this can blend semi-transparently from a PixelRGB argument.
    */
    template <class Pixel>
    forcedinline void blend (const Pixel& src, uint32 extraAlpha) noexcept
    {
        uint32 ag = maskPixelComponents (extraAlpha * src.getAG());
        const uint32 alpha = 0x100 - (ag >> 16);
        ag += g * alpha >> 8;

        uint32 rb = clampPixelComponents (maskPixelComponents (extraAlpha * src.getRB())
                                           + maskPixelComponents (getRB() * alpha));

        b = (uint8) rb;
        g = (uint8) clampPixelComponents (ag);
        r = (uint8) (rb >> 16);
    }

    /** Blends another pixel with this one, creating a colour that is somewhere
        between the two, as specified by the amount.
    */
    template <class Pixel>
    forcedinline void tween (const Pixel& src, const uint32 amount) noexcept
    {
        uint32 drb = getRB();
        drb += (((src.getRB() - drb) * amount) >> 8);

        uint32 dag = getAG();
        dag += (((src.getAG() - dag) * amount) >> 8);

        b = (uint8) drb;
        g = (uint8) dag;
        r = (uint8) (drb >> 16);
    }

    /** Copies another pixel colour over this one.

        This doesn't blend it - this colour is simply replaced by the other one.
        Because PixelRGB has no alpha channel, any alpha value in the source pixel
        is thrown away.
    */
    template <class Pixel>
    forcedinline void set (const Pixel& src) noexcept
    {
        b = src.getBlue();
        g = src.getGreen();
        r = src.getRed();
    }

    /** This method is included for compatibility with the PixelARGB class. */
    forcedinline void setAlpha (const uint8) noexcept {}

    /** Multiplies the colour's alpha value with another one. */
    forcedinline void multiplyAlpha (int) noexcept {}

    /** Multiplies the colour's alpha value with another one. */
    forcedinline void multiplyAlpha (float) noexcept {}

    /** Sets the pixel's colour from individual components. */
    void setARGB (const uint8, const uint8 red, const uint8 green, const uint8 blue) noexcept
    {
        r = red;
        g = green;
        b = blue;
    }

    /** Premultiplies the pixel's RGB values by its alpha. */
    forcedinline void premultiply() noexcept {}

    /** Unpremultiplies the pixel's RGB values. */
    forcedinline void unpremultiply() noexcept {}

    forcedinline void desaturate() noexcept
    {
        r = g = b = (uint8) (((int) r + (int) g + (int) b) / 3);
    }

    //==============================================================================
    /** The indexes of the different components in the byte layout of this type of colour. */
   #if JUCE_MAC
    enum { indexR = 0, indexG = 1, indexB = 2 };
   #else
    enum { indexR = 2, indexG = 1, indexB = 0 };
   #endif

private:
    //==============================================================================
   #if JUCE_MAC
    uint8 r, g, b;
   #else
    uint8 b, g, r;
   #endif

}
#ifndef DOXYGEN
 JUCE_PACKED
#endif
;

forcedinline void PixelARGB::blend (const PixelRGB src) noexcept
{
    set (src);
}

//==============================================================================
/**
    Represents an 8-bit single-channel pixel, and can perform compositing operations on it.

    This is used internally by the imaging classes.

    @see PixelARGB, PixelRGB
*/
class JUCE_API  PixelAlpha
{
public:
    /** Creates a pixel without defining its colour. */
    PixelAlpha() noexcept {}
    ~PixelAlpha() noexcept {}

    /** Creates a pixel from a 32-bit argb value.

        (The argb format is that used by PixelARGB)
    */
    PixelAlpha (const uint32 argb) noexcept
    {
        a = (uint8) (argb >> 24);
    }

    forcedinline uint32 getARGB() const noexcept                { return (((uint32) a) << 24) | (((uint32) a) << 16) | (((uint32) a) << 8) | a; }
    forcedinline uint32 getUnpremultipliedARGB() const noexcept { return (((uint32) a) << 24) | 0xffffff; }

    forcedinline uint32 getRB() const noexcept      { return (((uint32) a) << 16) | a; }
    forcedinline uint32 getAG() const noexcept      { return (((uint32) a) << 16) | a; }

    forcedinline uint8 getAlpha() const noexcept    { return a; }
    forcedinline uint8& getAlpha() noexcept         { return a; }

    forcedinline uint8 getRed() const noexcept      { return 0; }
    forcedinline uint8 getGreen() const noexcept    { return 0; }
    forcedinline uint8 getBlue() const noexcept     { return 0; }

    /** Blends another pixel onto this one.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    template <class Pixel>
    forcedinline void blend (const Pixel& src) noexcept
    {
        const int srcA = src.getAlpha();
        a = (uint8) ((a * (0x100 - srcA) >> 8) + srcA);
    }

    /** Blends another pixel onto this one, applying an extra multiplier to its opacity.

        The opacity of the pixel being overlaid is scaled by the extraAlpha factor before
        being used, so this can blend semi-transparently from a PixelRGB argument.
    */
    template <class Pixel>
    forcedinline void blend (const Pixel& src, uint32 extraAlpha) noexcept
    {
        ++extraAlpha;
        const int srcAlpha = (int) ((extraAlpha * src.getAlpha()) >> 8);
        a = (uint8) ((a * (0x100 - srcAlpha) >> 8) + srcAlpha);
    }

    /** Blends another pixel with this one, creating a colour that is somewhere
        between the two, as specified by the amount.
    */
    template <class Pixel>
    forcedinline void tween (const Pixel& src, const uint32 amount) noexcept
    {
        a += ((src.getAlpha() - a) * amount) >> 8;
    }

    /** Copies another pixel colour over this one.

        This doesn't blend it - this colour is simply replaced by the other one.
    */
    template <class Pixel>
    forcedinline void set (const Pixel& src) noexcept
    {
        a = src.getAlpha();
    }

    /** Replaces the colour's alpha value with another one. */
    forcedinline void setAlpha (const uint8 newAlpha) noexcept
    {
        a = newAlpha;
    }

    /** Multiplies the colour's alpha value with another one. */
    forcedinline void multiplyAlpha (int multiplier) noexcept
    {
        ++multiplier;
        a = (uint8) ((a * multiplier) >> 8);
    }

    forcedinline void multiplyAlpha (const float multiplier) noexcept
    {
        a = (uint8) (a * multiplier);
    }

    /** Sets the pixel's colour from individual components. */
    forcedinline void setARGB (const uint8 a_, const uint8 /*r*/, const uint8 /*g*/, const uint8 /*b*/) noexcept
    {
        a = a_;
    }

    /** Premultiplies the pixel's RGB values by its alpha. */
    forcedinline void premultiply() noexcept {}

    /** Unpremultiplies the pixel's RGB values. */
    forcedinline void unpremultiply() noexcept {}

    forcedinline void desaturate() noexcept {}

    //==============================================================================
    /** The indexes of the different components in the byte layout of this type of colour. */
    enum { indexA = 0 };

private:
    //==============================================================================
    uint8 a;
}
#ifndef DOXYGEN
 JUCE_PACKED
#endif
;

#if JUCE_MSVC
 #pragma pack (pop)
#endif

#endif   // JUCE_PIXELFORMATS_H_INCLUDED
