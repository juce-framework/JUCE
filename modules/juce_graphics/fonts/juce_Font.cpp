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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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

class Font::Native
{
public:
    HbFont font{};

    static Typeface::Ptr getDefaultPlatformTypefaceForFont (const Font&);
};

using GetTypefaceForFont = Typeface::Ptr (*)(const Font&);
GetTypefaceForFont juce_getTypefaceForFont = nullptr;

float Font::getDefaultMinimumHorizontalScaleFactor() noexcept                { return FontValues::minimumHorizontalScale; }
void Font::setDefaultMinimumHorizontalScaleFactor (float newValue) noexcept  { FontValues::minimumHorizontalScale = newValue; }

//==============================================================================
class TypefaceCache final : private DeletedAtShutdown
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

    JUCE_DECLARE_SINGLETON_INLINE (TypefaceCache, false)

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
        const Key key { font.getTypefaceName(), font.getTypefaceStyle() };

        jassert (key.name.isNotEmpty());

        {
            const ScopedReadLock slr (lock);

            const auto range = makeRange (std::make_reverse_iterator (faces.end()),
                                          std::make_reverse_iterator (faces.begin()));

            for (auto& face : range)
            {
                if (face.key == key && face.typeface != nullptr)
                {
                    face.lastUsageCount = ++counter;
                    return face.typeface;
                }
            }
        }

        const ScopedWriteLock slw (lock);

        auto newFace = CachedFace { key,
                                    ++counter,
                                    juce_getTypefaceForFont != nullptr
                                        ? juce_getTypefaceForFont (font)
                                        : Font::getDefaultTypefaceForFont (font) };

        if (newFace.typeface == nullptr)
            return nullptr;

        const auto replaceIter = std::min_element (faces.begin(),
                                                   faces.end(),
                                                   [] (const auto& a, const auto& b)
                                                   {
                                                       return a.lastUsageCount < b.lastUsageCount;
                                                   });

        jassert (replaceIter != faces.end());
        auto& face = *replaceIter;

        face = std::move (newFace);

        if (defaultFace == nullptr && key == Key{})
            defaultFace = face.typeface;

        return face.typeface;
    }

    Typeface::Ptr getDefaultFace() const noexcept
    {
        const ScopedReadLock slr (lock);
        return defaultFace;
    }

private:
    struct Key
    {
        String name = Font::getDefaultSansSerifFontName(), style = Font::getDefaultStyle();

        bool operator== (const Key& other) const
        {
            const auto tie = [] (const auto& x) { return std::tie (x.name, x.style); };
            return tie (*this) == tie (other);
        }

        bool operator!= (const Key& other) const
        {
            return ! operator== (other);
        }
    };

    struct CachedFace
    {
        // Although it seems a bit wacky to store the name here, it's because it may be a
        // placeholder rather than a real one, e.g. "<Sans-Serif>" vs the actual typeface name.
        // Since the typeface itself doesn't know that it may have this alias, the name under
        // which it was fetched needs to be stored separately.
        Key key;
        size_t lastUsageCount = 0;
        Typeface::Ptr typeface;
    };

    Typeface::Ptr defaultFace;
    ReadWriteLock lock;
    Array<CachedFace> faces;
    size_t counter = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TypefaceCache)
};

void Typeface::setTypefaceCacheSize (int numFontsToCache)
{
    TypefaceCache::getInstance()->setSize (numFontsToCache);
}

void (*clearOpenGLGlyphCache)() = nullptr;

void Typeface::clearTypefaceCache()
{
    TypefaceCache::getInstance()->clear();

    RenderingHelpers::SoftwareRendererSavedState::clearGlyphCache();

    NullCheckedInvocation::invoke (clearOpenGLGlyphCache);
}

//==============================================================================
class Font::SharedFontInternal  : public ReferenceCountedObject
{
public:
    explicit SharedFontInternal (FontOptions x)
        : options (x.getName().isEmpty() ? x.withName (getDefaultSansSerifFontName()) : std::move (x))
    {
    }

    ReferenceCountedObjectPtr<SharedFontInternal> copy() const
    {
        const ScopedLock lock (mutex);
        return new SharedFontInternal (typeface, options);
    }

    Typeface::Ptr getTypefacePtr (const Font& f)
    {
        const ScopedLock lock (mutex);

        if (typeface == nullptr)
            typeface = options.getTypeface() != nullptr ? options.getTypeface() : TypefaceCache::getInstance()->findTypefaceFor (f);

        return typeface;
    }

    HbFont getFontPtr (const Font& f)
    {
        const ScopedLock lock (mutex);

        if (auto ptr = getTypefacePtr (f))
            return ptr->getNativeDetails().getFontAtPointSizeAndScale (f.getHeightInPoints(), f.getHorizontalScale());

        return {};
    }

    TypefaceAscentDescent getAscentDescent (const Font& f)
    {
        const ScopedLock lock (mutex);

        if (auto ptr = getTypefacePtr (f))
        {
            const auto ascentDescent = ptr->getNativeDetails().getAscentDescent (f.getMetricsKind());

            auto adjusted = ascentDescent;
            adjusted.ascent = getAscentOverride().value_or (adjusted.ascent);
            adjusted.descent = getDescentOverride().value_or (adjusted.descent);
            return adjusted;
        }

        return {};
    }

    void resetTypeface()
    {
        const ScopedLock lock (mutex);
        typeface = nullptr;
    }

    /*  We do not need to lock in these functions, as it's guaranteed
        that these data members can only change if there is a single Font
        instance referencing the shared state.
    */

    StringArray getFallbackFamilies() const
    {
        const auto fallbacks = options.getFallbacks();
        return StringArray (fallbacks.data(), (int) fallbacks.size());
    }

    String getTypefaceName() const               { return options.getName(); }
    String getTypefaceStyle() const              { return options.getStyle(); }
    float getHeight() const                      { return options.getHeight(); }
    float getPointHeight() const                 { return options.getPointHeight(); }
    float getHorizontalScale() const             { return options.getHorizontalScale(); }
    float getKerning() const                     { return options.getKerningFactor(); }
    bool getUnderline() const                    { return options.getUnderline(); }
    bool getFallbackEnabled() const              { return options.getFallbackEnabled(); }
    TypefaceMetricsKind getMetricsKind() const   { return options.getMetricsKind(); }
    auto getFeatureSettings() const              { return options.getFeatureSettings(); }

    void setFeatureSetting (const FontFeatureSetting& feature)
    {
        jassert (getReferenceCount() == 1);
        options = options.withFeatureSetting (feature);
    }

    void removeFeatureSetting (FontFeatureTag feature)
    {
        jassert (getReferenceCount() == 1);
        options = options.withFeatureRemoved (feature);
    }

    std::optional<float> getAscentOverride() const  { return options.getAscentOverride(); }
    std::optional<float> getDescentOverride() const { return options.getDescentOverride(); }

    /*  This shared state may be shared between two or more Font instances that are being
        read/modified from multiple threads.
        Before modifying a shared instance you *must* call dupeInternalIfShared to
        ensure that only one Font instance is pointing to the SharedFontInternal instance
        during the modification.
    */

    void setTypeface (Typeface::Ptr newTypeface)
    {
        jassert (getReferenceCount() == 1);
        typeface = newTypeface;

        if (typeface != nullptr)
            options = options.withTypeface (nullptr).withName ("").withStyle ("");

        options = options.withTypeface (typeface);
    }

    void setTypefaceName (String x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withName (x);
    }

    void setTypefaceStyle (String x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withStyle (x);
    }

    void setHeight (float x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withHeight (x);
    }

    void setPointHeight (float x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withPointHeight (x);
    }

    void setHorizontalScale (float x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withHorizontalScale (x);
    }

    void setKerning (float x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withKerningFactor (x);
    }

    void setAscentOverride (std::optional<float> x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withAscentOverride (x);
    }

    void setDescentOverride (std::optional<float> x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withDescentOverride (x);
    }

    void setUnderline (bool x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withUnderline (x);
    }

    void setFallbackFamilies (const StringArray& x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withFallbacks ({ x.begin(), x.end() });
    }

    void setFallback (bool x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withFallbackEnabled (x);
    }

    bool operator== (const SharedFontInternal& other) const
    {
        return options == other.options;
    }

    bool operator<  (const SharedFontInternal& other) const
    {
        return options < other.options;
    }

private:
    SharedFontInternal (Typeface::Ptr t, FontOptions o)
        : typeface (t), options (std::move (o))
    {
    }

    Typeface::Ptr typeface;
    FontOptions options;
    CriticalSection mutex;
};

//==============================================================================
Font::Font (FontOptions opt)
    : font (new SharedFontInternal (std::move (opt)))
{
}

template <typename... Args>
auto legacyArgs (Args&&... args)
{
    auto result = FontOptions { std::forward<Args> (args)... }.withMetricsKind (TypefaceMetricsKind::legacy);

    if (result.getName().isEmpty())
        result = result.withName (Font::getDefaultSansSerifFontName());

    return result;
}

Font::Font()                                : font (new SharedFontInternal (legacyArgs())) {}
Font::Font (const Typeface::Ptr& typeface)  : font (new SharedFontInternal (legacyArgs (typeface))) {}
Font::Font (const Font& other) noexcept     : font (other.font) {}

Font::Font (float fontHeight, int styleFlags)
    : font (new SharedFontInternal (legacyArgs (fontHeight, styleFlags)))
{
}

Font::Font (const String& typefaceName, float fontHeight, int styleFlags)
    : font (new SharedFontInternal (legacyArgs (typefaceName, fontHeight, styleFlags)))
{
}

Font::Font (const String& typefaceName, const String& typefaceStyle, float fontHeight)
    : font (new SharedFontInternal (legacyArgs (typefaceName, typefaceStyle, fontHeight)))
{
}

Font& Font::operator= (const Font& other) noexcept
{
    Font copy { other };
    std::swap (copy.font, font);
    return *this;
}

Font::Font (Font&& other) noexcept
    : font (std::exchange (other.font, {}))
{
}

Font& Font::operator= (Font&& other) noexcept
{
    Font copy { std::move (other) };
    std::swap (copy.font, font);
    return *this;
}

Font::~Font() noexcept = default;

bool Font::operator== (const Font& other) const noexcept
{
    return font == other.font
            || *font == *other.font;
}

bool Font::operator!= (const Font& other) const noexcept
{
    return ! operator== (other);
}

bool Font::compare (const Font& a, const Font& b) noexcept
{
    return *a.font < *b.font;
}

void Font::dupeInternalIfShared()
{
    if (font->getReferenceCount() > 1)
        font = font->copy();
}

//==============================================================================
struct FontPlaceholderNames
{
    String sans     = "<Sans-Serif>",
           serif    = "<Serif>",
           mono     = "<Monospaced>",
           regular  = "<Regular>",
           systemUi = "system-ui";
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
const String& Font::getSystemUIFontName()               { return getFontPlaceholderNames().systemUi; }
const String& Font::getDefaultSerifFontName()           { return getFontPlaceholderNames().serif; }
const String& Font::getDefaultMonospacedFontName()      { return getFontPlaceholderNames().mono; }
const String& Font::getDefaultStyle()                   { return getFontPlaceholderNames().regular; }

String Font::getTypefaceName() const noexcept           { return font->getTypefaceName(); }
String Font::getTypefaceStyle() const noexcept          { return font->getTypefaceStyle(); }

void Font::setTypefaceName (const String& faceName)
{
    if (faceName != font->getTypefaceName())
    {
        jassert (faceName.isNotEmpty());

        dupeInternalIfShared();
        font->setTypeface (nullptr);
        font->setTypefaceName (faceName);
    }
}

void Font::setTypefaceStyle (const String& typefaceStyle)
{
    if (typefaceStyle != font->getTypefaceStyle())
    {
        dupeInternalIfShared();
        font->setTypeface (nullptr);
        font->setTypefaceStyle (typefaceStyle);
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
    return findAllTypefaceStyles (getTypefacePtr()->getName());
}

void Font::setPreferredFallbackFamilies (const StringArray& fallbacks)
{
    if (getPreferredFallbackFamilies() != fallbacks)
    {
        dupeInternalIfShared();
        font->setFallbackFamilies (fallbacks);
    }
}

StringArray Font::getPreferredFallbackFamilies() const
{
    return font->getFallbackFamilies();
}

void Font::setFallbackEnabled (bool enabled)
{
    if (getFallbackEnabled() != enabled)
    {
        dupeInternalIfShared();
        font->setFallback (enabled);
    }
}

bool Font::getFallbackEnabled() const
{
    return font->getFallbackEnabled();
}

Typeface::Ptr Font::getTypefacePtr() const
{
    return font->getTypefacePtr (*this);
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
    return font->getAscentDescent (*this).getHeightToPointsFactor();
}

Font Font::withPointHeight (float heightInPoints) const
{
    Font f (*this);
    f.setPointHeight (heightInPoints);
    return f;
}

void Font::setHeight (float newHeight)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (! approximatelyEqual (font->getHeight(), newHeight))
    {
        dupeInternalIfShared();
        font->setHeight (newHeight);
        font->resetTypeface();
    }
}

void Font::setPointHeight (float newHeight)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (! approximatelyEqual (font->getPointHeight(), newHeight))
    {
        dupeInternalIfShared();
        font->setPointHeight (newHeight);
        font->resetTypeface();
    }
}

void Font::setHeightWithoutChangingWidth (float newHeight)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (! approximatelyEqual (font->getHeight(), newHeight))
    {
        dupeInternalIfShared();
        font->setHorizontalScale (font->getHorizontalScale() * (font->getHeight() / newHeight));
        font->setHeight (newHeight);
        font->resetTypeface();
    }
}

int Font::getStyleFlags() const noexcept
{
    int styleFlags = font->getUnderline() ? underlined : plain;

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
        font->setTypeface (nullptr);
        font->setTypefaceStyle (FontStyleHelpers::getStyleName (newFlags));
        font->setUnderline ((newFlags & underlined) != 0);
    }
}

void Font::setSizeAndStyle (float newHeight,
                            const int newStyleFlags,
                            const float newHorizontalScale,
                            const float newKerningAmount)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (! approximatelyEqual (font->getHeight(), newHeight)
         || ! approximatelyEqual (font->getHorizontalScale(), newHorizontalScale)
         || ! approximatelyEqual (font->getKerning(), newKerningAmount))
    {
        dupeInternalIfShared();
        font->setHeight (newHeight);
        font->setHorizontalScale (newHorizontalScale);
        font->setKerning (newKerningAmount);
        font->resetTypeface();
    }

    setStyleFlags (newStyleFlags);
}

void Font::setSizeAndStyle (float newHeight,
                            const String& newStyle,
                            const float newHorizontalScale,
                            const float newKerningAmount)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (! approximatelyEqual (font->getHeight(), newHeight)
         || ! approximatelyEqual (font->getHorizontalScale(), newHorizontalScale)
         || ! approximatelyEqual (font->getKerning(), newKerningAmount))
    {
        dupeInternalIfShared();
        font->setHeight (newHeight);
        font->setHorizontalScale (newHorizontalScale);
        font->setKerning (newKerningAmount);
        font->resetTypeface();
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
    font->setHorizontalScale (scaleFactor);
    font->resetTypeface();
}

float Font::getHorizontalScale() const noexcept
{
    return font->getHorizontalScale();
}

float Font::getExtraKerningFactor() const noexcept
{
    return font->getKerning();
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
    font->setKerning (extraKerning);
    font->resetTypeface();
}

std::optional<float> Font::getAscentOverride() const noexcept
{
    return font->getAscentOverride();
}

void Font::setAscentOverride (std::optional<float> x)
{
    dupeInternalIfShared();
    font->setAscentOverride (x);
}

std::optional<float> Font::getDescentOverride() const noexcept
{
    return font->getDescentOverride();
}

void Font::setDescentOverride (std::optional<float> x)
{
    dupeInternalIfShared();
    font->setDescentOverride (x);
}

Font Font::boldened() const                 { return withStyle (getStyleFlags() | bold); }
Font Font::italicised() const               { return withStyle (getStyleFlags() | italic); }

bool Font::isBold() const noexcept          { return FontStyleHelpers::isBold   (font->getTypefaceStyle()); }
bool Font::isItalic() const noexcept        { return FontStyleHelpers::isItalic (font->getTypefaceStyle()); }
bool Font::isUnderlined() const noexcept    { return font->getUnderline(); }

TypefaceMetricsKind Font::getMetricsKind() const noexcept { return font->getMetricsKind(); }

Span<const FontFeatureSetting> Font::getFeatureSettings() const&
{
    return font->getFeatureSettings();
}

void Font::setFeatureSetting (FontFeatureSetting featureSetting)
{
    dupeInternalIfShared();
    font->setFeatureSetting (featureSetting);
}

void Font::removeFeatureSetting (FontFeatureTag featureToRemove)
{
    dupeInternalIfShared();
    font->removeFeatureSetting (featureToRemove);
}

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
    font->setUnderline (shouldBeUnderlined);
    font->resetTypeface();
}

float Font::getAscent() const
{
    return font->getAscentDescent (*this).getScaledAscent() * getHeight();
}

float Font::getHeight() const noexcept
{
    jassert ((font->getHeight() > 0.0f) != (font->getPointHeight() > 0.0f));
    const auto height = font->getHeight();
    return height > 0.0f ? height : font->getPointHeight() * font->getAscentDescent (*this).getPointsToHeightFactor();
}

float Font::getDescent() const              { return getHeight() - getAscent(); }

float Font::getHeightInPoints() const
{
    jassert ((font->getHeight() > 0.0f) != (font->getPointHeight() > 0.0f));
    const auto pointHeight = font->getPointHeight();

    if (pointHeight > 0.0f)
        return pointHeight;

    const auto factor = font->getAscentDescent (*this).getPointsToHeightFactor();

    if (factor > 0.0f)
        return font->getHeight() / factor;

    jassertfalse;
    return 0.0f;
}

float Font::getAscentInPoints() const       { return font->getAscentDescent (*this).ascent  * getHeightInPoints(); }
float Font::getDescentInPoints() const      { return font->getAscentDescent (*this).descent * getHeightInPoints(); }

int Font::getStringWidth (const String& text) const
{
    JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS
    return (int) std::ceil (getStringWidthFloat (text));
    JUCE_END_IGNORE_DEPRECATION_WARNINGS
}

float Font::getStringWidthFloat (const String& text) const
{
    if (auto typeface = getTypefacePtr())
    {
        const auto w = typeface->getStringWidth (getMetricsKind(), text, getHeight(), getHorizontalScale());
        return w + (getHeight() * getHorizontalScale() * getExtraKerningFactor() * (float) text.length());
    }

    return 0;
}

void Font::findFonts (Array<Font>& destArray)
{
    for (auto& name : findAllTypefaceNames())
    {
        auto styles = findAllTypefaceStyles (name);

        String style ("Regular");

        if (! styles.contains (style, true))
            style = styles[0];

        destArray.add (FontOptions (name, style, FontValues::defaultFontHeight));
    }
}

static bool characterNotRendered (uint32_t c)
{
    constexpr uint32_t points[]
    {
        // Control points
        0x0000, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x001A, 0x001B, 0x0085,

        // BIDI control points
        0x061C, 0x200E, 0x200F, 0x202A, 0x202B, 0x202C, 0x202D, 0x202E, 0x2066, 0x2067, 0x2068, 0x2069
    };

    return std::find (std::begin (points), std::end (points), c) != std::end (points);
}

static bool isFontSuitableForCodepoint (const Font& font, juce_wchar c)
{
    const auto& hbFont = font.getNativeDetails().font;

    if (hbFont == nullptr)
        return false;

    hb_codepoint_t glyph{};

    return characterNotRendered ((uint32_t) c)
           || hb_font_get_nominal_glyph (hbFont.get(), (hb_codepoint_t) c, &glyph);
}

static bool isFontSuitableForText (const Font& font, const String& str)
{
    for (const auto c : str)
        if (! isFontSuitableForCodepoint (font, c))
            return false;

    return true;
}

Font Font::findSuitableFontForText (const String& text, const String& language) const
{
    if (! getFallbackEnabled() || isFontSuitableForText (*this, text))
        return *this;

    for (const auto& fallback : getPreferredFallbackFamilies())
    {
        auto copy = *this;
        copy.setTypefaceName (fallback);

        if (isFontSuitableForText (copy, text))
            return copy;
    }

    const auto fallbackTypefacePtr = std::invoke ([&]
    {
        if (auto current = getTypefacePtr())
            return current;

        auto copy = *this;
        copy.setTypefaceName (Font::getDefaultSansSerifFontName());
        return copy.getTypefacePtr();
    });

    if (fallbackTypefacePtr != nullptr)
    {
        if (auto suggested = fallbackTypefacePtr->createSystemFallback (text, language))
        {
            auto copy = *this;

            if (copy.getTypefacePtr() != suggested)
            {
                copy.dupeInternalIfShared();
                copy.font->setTypeface (suggested);
            }

            return copy;
        }
    }

    return *this;
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

    return FontOptions (name, style, height);
}

Font::Native Font::getNativeDetails() const
{
    return { font->getFontPtr (*this) };
}

Typeface::Ptr Font::getDefaultTypefaceForFont (const Font& font)
{
    const auto resolvedTypeface = [&]() -> Typeface::Ptr
    {
        if (font.getTypefaceName() != getSystemUIFontName())
            return {};

        const auto systemTypeface = Typeface::findSystemTypeface();

        if (systemTypeface == nullptr)
            return {};

        if (systemTypeface->getStyle() == font.getTypefaceStyle())
            return systemTypeface;

        auto copy = font;
        copy.setTypefaceName (systemTypeface->getName());
        return getDefaultTypefaceForFont (copy);
    }();

    if (resolvedTypeface != nullptr)
        return resolvedTypeface;

    return Native::getDefaultPlatformTypefaceForFont (font);
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class FontTests : public UnitTest
{
public:
    FontTests() : UnitTest ("Font", UnitTestCategories::graphics) {}

    void runTest() override
    {
        const Span data { FontBinaryData::Karla_Regular_Typo_On_Offsets_Off };
        const auto face = Typeface::createSystemTypefaceFor (data.data(), data.size());

        beginTest ("Old constructor from Typeface");
        {
            JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS
            Font f { face };
            JUCE_END_IGNORE_DEPRECATION_WARNINGS

            expect (f.getTypefaceName() == face->getName());
            expect (f.getTypefaceStyle() == face->getStyle());
            expect (f.getTypefacePtr() == face);

            f.setTypefaceStyle ("Italic");

            expect (f.getTypefaceName() == face->getName());
            expect (f.getTypefaceStyle() == "Italic");
            expect (f.getTypefacePtr() != face);
        }

        beginTest ("FontOptions constructor from Typeface");
        {
            const FontOptions opt { face };
            expect (opt.getName() == face->getName());
            expect (opt.getStyle() == face->getStyle());
            expect (opt.getTypeface() == face);

            Font f { opt };

            expect (f.getTypefaceName() == face->getName());
            expect (f.getTypefaceStyle() == face->getStyle());
            expect (f.getTypefacePtr() == face);

            f.setTypefaceStyle ("Italic");

            expect (f.getTypefaceName() == face->getName());
            expect (f.getTypefaceStyle() == "Italic");
            expect (f.getTypefacePtr() != face);
        }

        beginTest ("FontOptions constructor from Typeface with style and name set");
        {
            const auto opt = FontOptions { face }.withName ("placeholder").withStyle ("Italic");
            expect (opt.getName() == face->getName());
            expect (opt.getStyle() == face->getStyle());
            expect (opt.getTypeface() == face);

            Font f { opt };

            expect (f.getTypefaceName() == face->getName());
            expect (f.getTypefaceStyle() == face->getStyle());
            expect (f.getTypefacePtr() == face);

            f.setTypefaceStyle ("Italic");

            expect (f.getTypefaceName() == face->getName());
            expect (f.getTypefaceStyle() == "Italic");
            expect (f.getTypefacePtr() != face);
        }

        auto a = FontOptions().withName ("placeholder").withStyle ("Italic");

        beginTest ("Setting Typeface on FontOptions replaces previous name/style");
        {
            auto b = a.withTypeface (face);

            expect (b.getName() == face->getName());
            expect (b.getStyle() == face->getStyle());
        }

        beginTest ("Setting a name or style on a FontOptions holding a typeface has no effect");
        {
            auto b = a.withTypeface (face).withName ("name").withStyle ("style");
            expect (b.getName() == face->getName());
            expect (b.getStyle() == face->getStyle());

            auto c = b.withTypeface (nullptr).withName ("name").withStyle ("style");
            expect (c.getName() == "name");
            expect (c.getStyle() == "style");
        }
    }
};

static FontTests fontTests;

#endif

} // namespace juce
