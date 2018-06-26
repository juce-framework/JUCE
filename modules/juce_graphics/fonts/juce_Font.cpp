/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace FontValues
{
    static float limitFontHeight (const float height) noexcept
    {
        return jlimit (0.1f, 10000.0f, height);
    }

    const float defaultFontHeight = 14.0f;
    float minimumHorizontalScale = 0.7f;
    String fallbackFont;
    String fallbackFontStyle;
}

using GetTypefaceForFont = Typeface::Ptr (*)(const Font&);
GetTypefaceForFont juce_getTypefaceForFont = nullptr;

float Font::getDefaultMinimumHorizontalScaleFactor() noexcept                { return FontValues::minimumHorizontalScale; }
void Font::setDefaultMinimumHorizontalScaleFactor (float newValue) noexcept  { FontValues::minimumHorizontalScale = newValue; }

//==============================================================================
class TypefaceCache  : private DeletedAtShutdown
{
public:
    TypefaceCache()
    {
        setSize (10);
    }

    ~TypefaceCache()
    {
        clearSingletonInstance();
    }

    JUCE_DECLARE_SINGLETON (TypefaceCache, false)

    void setSize (const int numToCache)
    {
        const ScopedWriteLock sl (lock);

        faces.clear();
        faces.insertMultiple (-1, CachedFace(), numToCache);
    }

    void clear()
    {
        const ScopedWriteLock sl (lock);

        setSize (faces.size());
        defaultFace = nullptr;
    }

    Typeface::Ptr findTypefaceFor (const Font& font)
    {
        const ScopedReadLock slr (lock);

        auto faceName = font.getTypefaceName();
        auto faceStyle = font.getTypefaceStyle();

        jassert (faceName.isNotEmpty());

        for (int i = faces.size(); --i >= 0;)
        {
            CachedFace& face = faces.getReference(i);

            if (face.typefaceName == faceName
                 && face.typefaceStyle == faceStyle
                 && face.typeface != nullptr
                 && face.typeface->isSuitableForFont (font))
            {
                face.lastUsageCount = ++counter;
                return face.typeface;
            }
        }

        const ScopedWriteLock slw (lock);
        int replaceIndex = 0;
        auto bestLastUsageCount = std::numeric_limits<size_t>::max();

        for (int i = faces.size(); --i >= 0;)
        {
            auto lu = faces.getReference(i).lastUsageCount;

            if (bestLastUsageCount > lu)
            {
                bestLastUsageCount = lu;
                replaceIndex = i;
            }
        }

        auto& face = faces.getReference (replaceIndex);
        face.typefaceName = faceName;
        face.typefaceStyle = faceStyle;
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

    Typeface::Ptr defaultFace;

private:
    struct CachedFace
    {
        CachedFace() noexcept {}

        // Although it seems a bit wacky to store the name here, it's because it may be a
        // placeholder rather than a real one, e.g. "<Sans-Serif>" vs the actual typeface name.
        // Since the typeface itself doesn't know that it may have this alias, the name under
        // which it was fetched needs to be stored separately.
        String typefaceName, typefaceStyle;
        size_t lastUsageCount = 0;
        Typeface::Ptr typeface;
    };

    ReadWriteLock lock;
    Array<CachedFace> faces;
    size_t counter = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TypefaceCache)
};

JUCE_IMPLEMENT_SINGLETON (TypefaceCache)

void Typeface::setTypefaceCacheSize (int numFontsToCache)
{
    TypefaceCache::getInstance()->setSize (numFontsToCache);
}

void (*clearOpenGLGlyphCache)() = nullptr;

void Typeface::clearTypefaceCache()
{
    TypefaceCache::getInstance()->clear();

    RenderingHelpers::SoftwareRendererSavedState::clearGlyphCache();

    if (clearOpenGLGlyphCache != nullptr)
        clearOpenGLGlyphCache();
}

//==============================================================================
class Font::SharedFontInternal  : public ReferenceCountedObject
{
public:
    SharedFontInternal() noexcept
        : typeface (TypefaceCache::getInstance()->defaultFace),
          typefaceName (Font::getDefaultSansSerifFontName()),
          typefaceStyle (Font::getDefaultStyle()),
          height (FontValues::defaultFontHeight)
    {
    }

    SharedFontInternal (int styleFlags, float fontHeight) noexcept
        : typefaceName (Font::getDefaultSansSerifFontName()),
          typefaceStyle (FontStyleHelpers::getStyleName (styleFlags)),
          height (fontHeight),
          underline ((styleFlags & underlined) != 0)
    {
        if (styleFlags == plain)
            typeface = TypefaceCache::getInstance()->defaultFace;
    }

    SharedFontInternal (const String& name, int styleFlags, float fontHeight) noexcept
        : typefaceName (name),
          typefaceStyle (FontStyleHelpers::getStyleName (styleFlags)),
          height (fontHeight),
          underline ((styleFlags & underlined) != 0)
    {
        if (styleFlags == plain && typefaceName.isEmpty())
            typeface = TypefaceCache::getInstance()->defaultFace;
    }

    SharedFontInternal (const String& name, const String& style, float fontHeight) noexcept
        : typefaceName (name), typefaceStyle (style), height (fontHeight)
    {
        if (typefaceName.isEmpty())
            typefaceName = Font::getDefaultSansSerifFontName();
    }

    explicit SharedFontInternal (const Typeface::Ptr& face) noexcept
        : typeface (face),
          typefaceName (face->getName()),
          typefaceStyle (face->getStyle()),
          height (FontValues::defaultFontHeight)
    {
        jassert (typefaceName.isNotEmpty());
    }

    SharedFontInternal (const SharedFontInternal& other) noexcept
        : ReferenceCountedObject(),
          typeface (other.typeface),
          typefaceName (other.typefaceName),
          typefaceStyle (other.typefaceStyle),
          height (other.height),
          horizontalScale (other.horizontalScale),
          kerning (other.kerning),
          ascent (other.ascent),
          underline (other.underline)
    {
    }

    bool operator== (const SharedFontInternal& other) const noexcept
    {
        return height == other.height
                && underline == other.underline
                && horizontalScale == other.horizontalScale
                && kerning == other.kerning
                && typefaceName == other.typefaceName
                && typefaceStyle == other.typefaceStyle;
    }

    Typeface::Ptr typeface;
    String typefaceName, typefaceStyle;
    float height, horizontalScale = 1.0f, kerning = 0, ascent = 0;
    bool underline = false;
};

//==============================================================================
Font::Font()                                : font (new SharedFontInternal()) {}
Font::Font (const Typeface::Ptr& typeface)  : font (new SharedFontInternal (typeface)) {}
Font::Font (const Font& other) noexcept     : font (other.font) {}

Font::Font (float fontHeight, int styleFlags)
    : font (new SharedFontInternal (styleFlags, FontValues::limitFontHeight (fontHeight)))
{
}

Font::Font (const String& typefaceName, float fontHeight, int styleFlags)
    : font (new SharedFontInternal (typefaceName, styleFlags, FontValues::limitFontHeight (fontHeight)))
{
}

Font::Font (const String& typefaceName, const String& typefaceStyle, float fontHeight)
    : font (new SharedFontInternal (typefaceName, typefaceStyle, FontValues::limitFontHeight (fontHeight)))
{
}

Font& Font::operator= (const Font& other) noexcept
{
    font = other.font;
    return *this;
}

Font::Font (Font&& other) noexcept
    : font (static_cast<ReferenceCountedObjectPtr<SharedFontInternal>&&> (other.font))
{
}

Font& Font::operator= (Font&& other) noexcept
{
    font = static_cast<ReferenceCountedObjectPtr<SharedFontInternal>&&> (other.font);
    return *this;
}

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

void Font::checkTypefaceSuitability()
{
    if (font->typeface != nullptr && ! font->typeface->isSuitableForFont (*this))
        font->typeface = nullptr;
}

//==============================================================================
struct FontPlaceholderNames
{
    String sans    { "<Sans-Serif>" },
           serif   { "<Serif>" },
           mono    { "<Monospaced>" },
           regular { "<Regular>" };
};

static const FontPlaceholderNames& getFontPlaceholderNames()
{
    static FontPlaceholderNames names;
    return names;
}

#if JUCE_MSVC
// This is a workaround for the lack of thread-safety in MSVC's handling of function-local
// statics - if multiple threads all try to create the first Font object at the same time,
// it can cause a race-condition in creating these placeholder strings.
struct FontNamePreloader { FontNamePreloader() { getFontPlaceholderNames(); } };
static FontNamePreloader fnp;
#endif

const String& Font::getDefaultSansSerifFontName()       { return getFontPlaceholderNames().sans; }
const String& Font::getDefaultSerifFontName()           { return getFontPlaceholderNames().serif; }
const String& Font::getDefaultMonospacedFontName()      { return getFontPlaceholderNames().mono; }
const String& Font::getDefaultStyle()                   { return getFontPlaceholderNames().regular; }

const String& Font::getTypefaceName() const noexcept    { return font->typefaceName; }
const String& Font::getTypefaceStyle() const noexcept   { return font->typefaceStyle; }

void Font::setTypefaceName (const String& faceName)
{
    if (faceName != font->typefaceName)
    {
        jassert (faceName.isNotEmpty());

        dupeInternalIfShared();
        font->typefaceName = faceName;
        font->typeface = nullptr;
        font->ascent = 0;
    }
}

void Font::setTypefaceStyle (const String& typefaceStyle)
{
    if (typefaceStyle != font->typefaceStyle)
    {
        dupeInternalIfShared();
        font->typefaceStyle = typefaceStyle;
        font->typeface = nullptr;
        font->ascent = 0;
    }
}

Font Font::withTypefaceStyle (const String& newStyle) const
{
    Font f (*this);
    f.setTypefaceStyle (newStyle);
    return f;
}

StringArray Font::getAvailableStyles() const
{
    return findAllTypefaceStyles (getTypeface()->getName());
}

Typeface* Font::getTypeface() const
{
    if (font->typeface == nullptr)
    {
        font->typeface = TypefaceCache::getInstance()->findTypefaceFor (*this);
        jassert (font->typeface != nullptr);
    }

    return font->typeface.get();
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

const String& Font::getFallbackFontStyle()
{
    return FontValues::fallbackFontStyle;
}

void Font::setFallbackFontStyle (const String& style)
{
    FontValues::fallbackFontStyle = style;

   #if JUCE_MAC || JUCE_IOS
    jassertfalse; // Note that use of a fallback font isn't currently implemented in OSX..
   #endif
}

//==============================================================================
Font Font::withHeight (const float newHeight) const
{
    Font f (*this);
    f.setHeight (newHeight);
    return f;
}

float Font::getHeightToPointsFactor() const
{
    return getTypeface()->getHeightToPointsFactor();
}

Font Font::withPointHeight (float heightInPoints) const
{
    Font f (*this);
    f.setHeight (heightInPoints / getHeightToPointsFactor());
    return f;
}

void Font::setHeight (float newHeight)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (font->height != newHeight)
    {
        dupeInternalIfShared();
        font->height = newHeight;
        checkTypefaceSuitability();
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
        checkTypefaceSuitability();
    }
}

int Font::getStyleFlags() const noexcept
{
    int styleFlags = font->underline ? underlined : plain;

    if (isBold())    styleFlags |= bold;
    if (isItalic())  styleFlags |= italic;

    return styleFlags;
}

Font Font::withStyle (const int newFlags) const
{
    Font f (*this);
    f.setStyleFlags (newFlags);
    return f;
}

void Font::setStyleFlags (const int newFlags)
{
    if (getStyleFlags() != newFlags)
    {
        dupeInternalIfShared();
        font->typeface = nullptr;
        font->typefaceStyle = FontStyleHelpers::getStyleName (newFlags);
        font->underline = (newFlags & underlined) != 0;
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
        checkTypefaceSuitability();
    }

    setStyleFlags (newStyleFlags);
}

void Font::setSizeAndStyle (float newHeight,
                            const String& newStyle,
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
        checkTypefaceSuitability();
    }

    setTypefaceStyle (newStyle);
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
    checkTypefaceSuitability();
}

float Font::getHorizontalScale() const noexcept
{
    return font->horizontalScale;
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
    checkTypefaceSuitability();
}

Font Font::boldened() const                 { return withStyle (getStyleFlags() | bold); }
Font Font::italicised() const               { return withStyle (getStyleFlags() | italic); }

bool Font::isBold() const noexcept          { return FontStyleHelpers::isBold   (font->typefaceStyle); }
bool Font::isItalic() const noexcept        { return FontStyleHelpers::isItalic (font->typefaceStyle); }
bool Font::isUnderlined() const noexcept    { return font->underline; }

void Font::setBold (const bool shouldBeBold)
{
    auto flags = getStyleFlags();
    setStyleFlags (shouldBeBold ? (flags | bold)
                                : (flags & ~bold));
}

void Font::setItalic (const bool shouldBeItalic)
{
    auto flags = getStyleFlags();
    setStyleFlags (shouldBeItalic ? (flags | italic)
                                  : (flags & ~italic));
}

void Font::setUnderline (const bool shouldBeUnderlined)
{
    dupeInternalIfShared();
    font->underline = shouldBeUnderlined;
    checkTypefaceSuitability();
}

float Font::getAscent() const
{
    if (font->ascent == 0.0f)
        font->ascent = getTypeface()->getAscent();

    return font->height * font->ascent;
}

float Font::getHeight() const noexcept      { return font->height; }
float Font::getDescent() const              { return font->height - getAscent(); }

float Font::getHeightInPoints() const       { return getHeight()  * getHeightToPointsFactor(); }
float Font::getAscentInPoints() const       { return getAscent()  * getHeightToPointsFactor(); }
float Font::getDescentInPoints() const      { return getDescent() * getHeightToPointsFactor(); }

int Font::getStringWidth (const String& text) const
{
    return (int) std::ceil (getStringWidthFloat (text));
}

float Font::getStringWidthFloat (const String& text) const
{
    // This call isn't thread-safe when there's a message thread running
    jassert (MessageManager::getInstanceWithoutCreating() == nullptr
               || MessageManager::getInstanceWithoutCreating()->currentThreadHasLockedMessageManager());

    auto w = getTypeface()->getStringWidth (text);

    if (font->kerning != 0.0f)
        w += font->kerning * text.length();

    return w * font->height * font->horizontalScale;
}

void Font::getGlyphPositions (const String& text, Array<int>& glyphs, Array<float>& xOffsets) const
{
    // This call isn't thread-safe when there's a message thread running
    jassert (MessageManager::getInstanceWithoutCreating() == nullptr
               || MessageManager::getInstanceWithoutCreating()->currentThreadHasLockedMessageManager());

    getTypeface()->getGlyphPositions (text, glyphs, xOffsets);

    if (auto num = xOffsets.size())
    {
        auto scale = font->height * font->horizontalScale;
        auto* x = xOffsets.getRawDataPointer();

        if (font->kerning != 0.0f)
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
    for (auto& name : findAllTypefaceNames())
    {
        auto styles = findAllTypefaceStyles (name);

        String style ("Regular");

        if (! styles.contains (style, true))
            style = styles[0];

        destArray.add (Font (name, style, FontValues::defaultFontHeight));
    }
}

//==============================================================================
String Font::toString() const
{
    String s;

    if (getTypefaceName() != getDefaultSansSerifFontName())
        s << getTypefaceName() << "; ";

    s << String (getHeight(), 1);

    if (getTypefaceStyle() != getDefaultStyle())
        s << ' ' << getTypefaceStyle();

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

    String sizeAndStyle (fontDescription.substring (separator + 1).trimStart());

    float height = sizeAndStyle.getFloatValue();
    if (height <= 0)
        height = 10.0f;

    const String style (sizeAndStyle.fromFirstOccurrenceOf (" ", false, false));

    return Font (name, style, height);
}

} // namespace juce
