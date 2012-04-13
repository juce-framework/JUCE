/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

namespace FontValues
{
    static float limitFontHeight (const float height) noexcept
    {
        return jlimit (0.1f, 10000.0f, height);
    }

    const float defaultFontHeight = 14.0f;
    String fallbackFont;
}

typedef Typeface::Ptr (*GetTypefaceForFont) (const Font&);
GetTypefaceForFont juce_getTypefaceForFont = nullptr;

//==============================================================================
class TypefaceCache  : public DeletedAtShutdown
{
public:
    TypefaceCache()
        : counter (0)
    {
        setSize (10);
    }

    ~TypefaceCache()
    {
        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (TypefaceCache);

    void setSize (const int numToCache)
    {
        faces.clear();
        faces.insertMultiple (-1, CachedFace(), numToCache);
    }

    Typeface::Ptr findTypefaceFor (const Font& font)
    {
        const int flags = font.getStyleFlags() & (Font::bold | Font::italic);
        const String faceName (font.getTypefaceName());

        int i;
        for (i = faces.size(); --i >= 0;)
        {
            CachedFace& face = faces.getReference(i);

            if (face.flags == flags
                 && face.typefaceName == faceName
                 && face.typeface->isSuitableForFont (font))
            {
                face.lastUsageCount = ++counter;
                return face.typeface;
            }
        }

        int replaceIndex = 0;
        size_t bestLastUsageCount = std::numeric_limits<int>::max();

        for (i = faces.size(); --i >= 0;)
        {
            const size_t lu = faces.getReference(i).lastUsageCount;

            if (bestLastUsageCount > lu)
            {
                bestLastUsageCount = lu;
                replaceIndex = i;
            }
        }

        CachedFace& face = faces.getReference (replaceIndex);
        face.typefaceName = faceName;
        face.flags = flags;
        face.lastUsageCount = ++counter;

        if (juce_getTypefaceForFont == nullptr)
            face.typeface = Font::getDefaultTypefaceForFont (font);
        else
            face.typeface = juce_getTypefaceForFont (font);

        jassert (face.typeface != nullptr); // the look and feel must return a typeface!

        if (defaultFace == nullptr && font == Font())
            defaultFace = face.typeface;

        return face.typeface;
    }

    Typeface::Ptr getDefaultTypeface() const noexcept
    {
        return defaultFace;
    }

private:
    struct CachedFace
    {
        CachedFace() noexcept
            : lastUsageCount (0), flags (-1)
        {
        }

        // Although it seems a bit wacky to store the name here, it's because it may be a
        // placeholder rather than a real one, e.g. "<Sans-Serif>" vs the actual typeface name.
        // Since the typeface itself doesn't know that it may have this alias, the name under
        // which it was fetched needs to be stored separately.
        String typefaceName;
        size_t lastUsageCount;
        int flags;
        Typeface::Ptr typeface;
    };

    Array <CachedFace> faces;
    Typeface::Ptr defaultFace;
    size_t counter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TypefaceCache);
};

juce_ImplementSingleton_SingleThreaded (TypefaceCache)

void Typeface::setTypefaceCacheSize (int numFontsToCache)
{
    TypefaceCache::getInstance()->setSize (numFontsToCache);
}

//==============================================================================
class Font::SharedFontInternal  : public SingleThreadedReferenceCountedObject
{
public:
    SharedFontInternal (const float height_, const int styleFlags_) noexcept
        : typefaceName (Font::getDefaultSansSerifFontName()),
          height (height_),
          horizontalScale (1.0f),
          kerning (0),
          ascent (0),
          styleFlags (styleFlags_),
          typeface ((styleFlags_ & (Font::bold | Font::italic)) == 0
                        ? TypefaceCache::getInstance()->getDefaultTypeface() : nullptr)
    {
    }

    SharedFontInternal (const String& typefaceName_, const float height_, const int styleFlags_) noexcept
        : typefaceName (typefaceName_),
          height (height_),
          horizontalScale (1.0f),
          kerning (0),
          ascent (0),
          styleFlags (styleFlags_),
          typeface (nullptr)
    {
    }

    SharedFontInternal (const Typeface::Ptr& typeface_) noexcept
        : typefaceName (typeface_->getName()),
          height (FontValues::defaultFontHeight),
          horizontalScale (1.0f),
          kerning (0),
          ascent (0),
          styleFlags (Font::plain),
          typeface (typeface_)
    {
    }

    SharedFontInternal (const SharedFontInternal& other) noexcept
        : typefaceName (other.typefaceName),
          height (other.height),
          horizontalScale (other.horizontalScale),
          kerning (other.kerning),
          ascent (other.ascent),
          styleFlags (other.styleFlags),
          typeface (other.typeface)
    {
    }

    bool operator== (const SharedFontInternal& other) const noexcept
    {
        return height == other.height
                && styleFlags == other.styleFlags
                && horizontalScale == other.horizontalScale
                && kerning == other.kerning
                && typefaceName == other.typefaceName;
    }

    String typefaceName;
    float height, horizontalScale, kerning, ascent;
    int styleFlags;
    Typeface::Ptr typeface;
};

//==============================================================================
Font::Font()
    : font (new SharedFontInternal (FontValues::defaultFontHeight, Font::plain))
{
}

Font::Font (const float fontHeight, const int styleFlags)
    : font (new SharedFontInternal (FontValues::limitFontHeight (fontHeight), styleFlags))
{
}

Font::Font (const String& typefaceName, const float fontHeight, const int styleFlags)
    : font (new SharedFontInternal (typefaceName, FontValues::limitFontHeight (fontHeight), styleFlags))
{
}

Font::Font (const Typeface::Ptr& typeface)
    : font (new SharedFontInternal (typeface))
{
}

Font::Font (const Font& other) noexcept
    : font (other.font)
{
}

Font& Font::operator= (const Font& other) noexcept
{
    font = other.font;
    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
Font::Font (Font&& other) noexcept
    : font (static_cast <ReferenceCountedObjectPtr <SharedFontInternal>&&> (other.font))
{
}

Font& Font::operator= (Font&& other) noexcept
{
    font = static_cast <ReferenceCountedObjectPtr <SharedFontInternal>&&> (other.font);
    return *this;
}
#endif

Font::~Font() noexcept
{
}

bool Font::operator== (const Font& other) const noexcept
{
    return font == other.font
            || *font == *other.font;
}

bool Font::operator!= (const Font& other) const noexcept
{
    return ! operator== (other);
}

void Font::dupeInternalIfShared()
{
    if (font->getReferenceCount() > 1)
        font = new SharedFontInternal (*font);
}

//==============================================================================
const String& Font::getDefaultSansSerifFontName()
{
    static const String name ("<Sans-Serif>");
    return name;
}

const String& Font::getDefaultSerifFontName()
{
    static const String name ("<Serif>");
    return name;
}

const String& Font::getDefaultMonospacedFontName()
{
    static const String name ("<Monospaced>");
    return name;
}

const String& Font::getTypefaceName() const noexcept
{
    return font->typefaceName;
}

void Font::setTypefaceName (const String& faceName)
{
    if (faceName != font->typefaceName)
    {
        dupeInternalIfShared();
        font->typefaceName = faceName;
        font->typeface = nullptr;
        font->ascent = 0;
    }
}

Typeface* Font::getTypeface() const
{
    if (font->typeface == nullptr)
        font->typeface = TypefaceCache::getInstance()->findTypefaceFor (*this);

    return font->typeface;
}

//==============================================================================
const String& Font::getFallbackFontName()
{
    return FontValues::fallbackFont;
}

void Font::setFallbackFontName (const String& name)
{
    FontValues::fallbackFont = name;

   #if JUCE_MAC || JUCE_IOS
    jassertfalse; // Note that use of a fallback font isn't currently implemented in OSX..
   #endif
}

//==============================================================================
float Font::getHeight() const noexcept
{
    return font->height;
}

Font Font::withHeight (const float newHeight) const
{
    Font f (*this);
    f.setHeight (newHeight);
    return f;
}

void Font::setHeight (float newHeight)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (font->height != newHeight)
    {
        dupeInternalIfShared();
        font->height = newHeight;
    }
}

void Font::setHeightWithoutChangingWidth (float newHeight)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (font->height != newHeight)
    {
        dupeInternalIfShared();
        font->horizontalScale *= (font->height / newHeight);
        font->height = newHeight;
    }
}

int Font::getStyleFlags() const noexcept
{
    return font->styleFlags;
}

Font Font::withStyle (const int newFlags) const
{
    Font f (*this);
    f.setStyleFlags (newFlags);
    return f;
}

void Font::setStyleFlags (const int newFlags)
{
    if (font->styleFlags != newFlags)
    {
        dupeInternalIfShared();
        font->styleFlags = newFlags;
        font->typeface = nullptr;
        font->ascent = 0;
    }
}

void Font::setSizeAndStyle (float newHeight,
                            const int newStyleFlags,
                            const float newHorizontalScale,
                            const float newKerningAmount)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (font->height != newHeight
         || font->horizontalScale != newHorizontalScale
         || font->kerning != newKerningAmount)
    {
        dupeInternalIfShared();
        font->height = newHeight;
        font->horizontalScale = newHorizontalScale;
        font->kerning = newKerningAmount;
    }

    setStyleFlags (newStyleFlags);
}

float Font::getHorizontalScale() const noexcept
{
    return font->horizontalScale;
}

Font Font::withHorizontalScale (const float newHorizontalScale) const
{
    Font f (*this);
    f.setHorizontalScale (newHorizontalScale);
    return f;
}

void Font::setHorizontalScale (const float scaleFactor)
{
    dupeInternalIfShared();
    font->horizontalScale = scaleFactor;
}

float Font::getExtraKerningFactor() const noexcept
{
    return font->kerning;
}

Font Font::withExtraKerningFactor (const float extraKerning) const
{
    Font f (*this);
    f.setExtraKerningFactor (extraKerning);
    return f;
}

void Font::setExtraKerningFactor (const float extraKerning)
{
    dupeInternalIfShared();
    font->kerning = extraKerning;
}

Font Font::boldened() const             { return withStyle (font->styleFlags | bold); }
Font Font::italicised() const           { return withStyle (font->styleFlags | italic); }

bool Font::isBold() const noexcept      { return (font->styleFlags & bold) != 0; }
bool Font::isItalic() const noexcept    { return (font->styleFlags & italic) != 0; }

void Font::setBold (const bool shouldBeBold)
{
    setStyleFlags (shouldBeBold ? (font->styleFlags | bold)
                                : (font->styleFlags & ~bold));
}

void Font::setItalic (const bool shouldBeItalic)
{
    setStyleFlags (shouldBeItalic ? (font->styleFlags | italic)
                                  : (font->styleFlags & ~italic));
}

void Font::setUnderline (const bool shouldBeUnderlined)
{
    setStyleFlags (shouldBeUnderlined ? (font->styleFlags | underlined)
                                      : (font->styleFlags & ~underlined));
}

bool Font::isUnderlined() const noexcept
{
    return (font->styleFlags & underlined) != 0;
}

float Font::getAscent() const
{
    if (font->ascent == 0)
        font->ascent = getTypeface()->getAscent();

    return font->height * font->ascent;
}

float Font::getDescent() const
{
    return font->height - getAscent();
}

int Font::getStringWidth (const String& text) const
{
    return roundToInt (getStringWidthFloat (text));
}

float Font::getStringWidthFloat (const String& text) const
{
    float w = getTypeface()->getStringWidth (text);

    if (font->kerning != 0)
        w += font->kerning * text.length();

    return w * font->height * font->horizontalScale;
}

void Font::getGlyphPositions (const String& text, Array <int>& glyphs, Array <float>& xOffsets) const
{
    getTypeface()->getGlyphPositions (text, glyphs, xOffsets);

    const int num = xOffsets.size();

    if (num > 0)
    {
        const float scale = font->height * font->horizontalScale;
        float* const x = xOffsets.getRawDataPointer();

        if (font->kerning != 0)
        {
            for (int i = 0; i < num; ++i)
                x[i] = (x[i] + i * font->kerning) * scale;
        }
        else
        {
            for (int i = 0; i < num; ++i)
                x[i] *= scale;
        }
    }
}

void Font::findFonts (Array<Font>& destArray)
{
    const StringArray names (findAllTypefaceNames());

    for (int i = 0; i < names.size(); ++i)
        destArray.add (Font (names[i], FontValues::defaultFontHeight, Font::plain));
}

//==============================================================================
String Font::toString() const
{
    String s (getTypefaceName());

    if (s == getDefaultSansSerifFontName())
        s = String::empty;
    else
        s += "; ";

    s += String (getHeight(), 1);

    if (isBold())      s += " bold";
    if (isItalic())    s += " italic";

    return s;
}

Font Font::fromString (const String& fontDescription)
{
    const int separator = fontDescription.indexOfChar (';');
    String name;

    if (separator > 0)
        name = fontDescription.substring (0, separator).trim();

    if (name.isEmpty())
        name = getDefaultSansSerifFontName();

    String sizeAndStyle (fontDescription.substring (separator + 1));

    float height = sizeAndStyle.getFloatValue();
    if (height <= 0)
        height = 10.0f;

    int flags = Font::plain;
    if (sizeAndStyle.containsIgnoreCase ("bold"))       flags |= Font::bold;
    if (sizeAndStyle.containsIgnoreCase ("italic"))     flags |= Font::italic;

    return Font (name, height, flags);
}
