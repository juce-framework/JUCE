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

Typeface::Ptr Font::Native::getDefaultPlatformTypefaceForFont (const Font& font)
{
    Font f (font);
    f.setTypefaceName ([&]() -> String
                       {
                           const auto faceName = font.getTypefaceName();

                           if (faceName == Font::getDefaultSansSerifFontName())    return "Roboto";
                           if (faceName == Font::getDefaultSerifFontName())        return "Roboto";
                           if (faceName == Font::getDefaultMonospacedFontName())   return "Roboto";

                           return faceName;
                       }());

    return Typeface::createSystemTypefaceFor (f);
}

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (create,          "create",           "(Ljava/lang/String;I)Landroid/graphics/Typeface;") \
 STATICMETHOD (createFromFile,  "createFromFile",   "(Ljava/lang/String;)Landroid/graphics/Typeface;") \
 STATICMETHOD (createFromAsset, "createFromAsset",  "(Landroid/content/res/AssetManager;Ljava/lang/String;)Landroid/graphics/Typeface;")

 DECLARE_JNI_CLASS (TypefaceClass, "android/graphics/Typeface")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (constructor,          "<init>",           "()V") \
 METHOD (computeBounds,        "computeBounds",     "(Landroid/graphics/RectF;Z)V")

 DECLARE_JNI_CLASS (AndroidPath, "android/graphics/Path")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (constructor,   "<init>",   "()V") \
 FIELD  (left,           "left",     "F") \
 FIELD  (right,          "right",    "F") \
 FIELD  (top,            "top",      "F") \
 FIELD  (bottom,         "bottom",   "F") \
 METHOD (roundOut,       "roundOut", "(Landroid/graphics/Rect;)V")

DECLARE_JNI_CLASS (AndroidRectF, "android/graphics/RectF")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (getInstance, "getInstance", "(Ljava/lang/String;)Ljava/security/MessageDigest;") \
 METHOD       (update,      "update",      "([B)V") \
 METHOD       (digest,      "digest",      "()[B")
DECLARE_JNI_CLASS (JavaMessageDigest, "java/security/MessageDigest")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD       (open,      "open",      "(Ljava/lang/String;)Ljava/io/InputStream;") \

DECLARE_JNI_CLASS (AndroidAssetManager, "android/content/res/AssetManager")
#undef JNI_CLASS_MEMBERS

// Defined in juce_core
std::unique_ptr<InputStream> makeAndroidInputStreamWrapper (LocalRef<jobject> stream);

struct AndroidCachedTypeface
{
    std::shared_ptr<hb_font_t> font;
    TypefaceAscentDescent nonPortableMetrics;
};

//==============================================================================
class MemoryFontCache : public DeletedAtShutdown
{
public:
    using Value = AndroidCachedTypeface;

    ~MemoryFontCache()
    {
        clearSingletonInstance();
    }

    struct Key
    {
        String name, style;
        auto tie() const { return std::tuple (name, style); }
        bool operator< (const Key& other) const { return tie() < other.tie(); }
        bool operator== (const Key& other) const { return tie() == other.tie(); }
    };

    void add (const Key& key, const Value& value)
    {
        const std::scoped_lock lock { mutex };
        cache.emplace (key, value);
    }

    void remove (const Key& p)
    {
        const std::scoped_lock lock { mutex };
        cache.erase (p);
    }

    std::set<String> getAllNames() const
    {
        const std::scoped_lock lock { mutex };
        std::set<String> result;

        for (const auto& item : cache)
            result.insert (item.first.name);

        return result;
    }

    std::set<String> getStylesForFamily (const String& family) const
    {
        const std::scoped_lock lock { mutex };

        const auto lower = std::lower_bound (cache.begin(), cache.end(), family, [] (const auto& a, const String& b)
        {
            return a.first.name < b;
        });
        const auto upper = std::upper_bound (cache.begin(), cache.end(), family, [] (const String& a, const auto& b)
        {
            return a < b.first.name;
        });

        std::set<String> result;

        for (const auto& item : makeRange (lower, upper))
            result.insert (item.first.style);

        return result;
    }

    std::optional<Value> find (const Key& key) const
    {
        const std::scoped_lock lock { mutex };

        const auto iter = cache.find (key);

        if (iter != cache.end())
            return iter->second;

        return {};
    }

    JUCE_DECLARE_SINGLETON_INLINE (MemoryFontCache, true)

private:
    std::map<Key, Value> cache;
    mutable std::mutex mutex;
};

StringArray Font::findAllTypefaceNames()
{
    auto results = [&]
    {
        if (auto* cache = MemoryFontCache::getInstance())
            return cache->getAllNames();

        return std::set<String>{};
    }();

    for (auto& f : File ("/system/fonts").findChildFiles (File::findFiles, false, "*.ttf"))
        results.insert (f.getFileNameWithoutExtension().upToLastOccurrenceOf ("-", false, false));

    StringArray s;

    for (const auto& family : results)
        s.add (family);

    return s;
}

StringArray Font::findAllTypefaceStyles (const String& family)
{
    auto results = [&]
    {
        if (auto* cache = MemoryFontCache::getInstance())
            return cache->getStylesForFamily (family);

        return std::set<String>{};
    }();

    for (auto& f : File ("/system/fonts").findChildFiles (File::findFiles, false, family + "-*.ttf"))
        results.insert (f.getFileNameWithoutExtension().fromLastOccurrenceOf ("-", false, false));

    StringArray s;

    for (const auto& style : results)
        s.add (style);

    return s;
}

//==============================================================================
class AndroidTypeface final : public Typeface
{
public:
    enum class DoCache
    {
        no,
        yes
    };

    static Typeface::Ptr from (const Font& font)
    {
        if (auto* cache = MemoryFontCache::getInstance())
            if (auto result = cache->find ({ font.getTypefaceName(), font.getTypefaceStyle() }))
                return new AndroidTypeface (DoCache::no, result->font, result->nonPortableMetrics, font.getTypefaceName(), font.getTypefaceStyle());

        auto [blob, metrics] = getBlobForFont (font);
        auto face = FontStyleHelpers::getFaceForBlob ({ static_cast<const char*> (blob.getData()), blob.getSize() }, 0);

        if (face == nullptr)
        {
            jassertfalse;
            return {};
        }

        HbFont hbFont { hb_font_create (face.get()) };
        FontStyleHelpers::initSynthetics (hbFont.get(), font);

        return new AndroidTypeface (DoCache::no, std::move (hbFont), metrics, font.getTypefaceName(), font.getTypefaceStyle());
    }

    static Typeface::Ptr from (Span<const std::byte> blob, unsigned int index = 0)
    {
        return fromMemory (DoCache::yes, blob, index);
    }

    Native getNativeDetails() const override
    {
        return Native { hbFont.get(), nonPortableMetrics };
    }

    Typeface::Ptr createSystemFallback (const String& text, const String& language) const override
    {
        if (__builtin_available (android 29, *))
            return matchWithAFontMatcher (text, language);

        // The font-fallback API is only available on Android API level 29+
        jassertfalse;
        return {};
    }

    ~AndroidTypeface() override
    {
        if (doCache == DoCache::yes)
            if (auto* c = MemoryFontCache::getInstance())
                c->remove ({ getName(), getStyle() });
    }

    static Typeface::Ptr findSystemTypeface()
    {
        if (__builtin_available (android 29, *))
            return findSystemTypefaceWithMatcher();

        return from (FontOptions{}.withName ("Roboto"));
    }

private:
    static __INTRODUCED_IN (29) Typeface::Ptr fromMatchedFont (AFont* matched)
    {
        if (matched == nullptr)
        {
            // Unable to find any matching fonts. This should never happen - in the worst case,
            // we should at least get a font with the tofu character.
            jassertfalse;
            return {};
        }

        const File matchedFile { AFont_getFontFilePath (matched) };
        const auto matchedIndex = AFont_getCollectionIndex (matched);

        auto* cache = TypefaceFileCache::getInstance();

        if (cache == nullptr)
            return {}; // Perhaps we're shutting down

        return cache->get ({ matchedFile, (int) matchedIndex }, &loadCompatibleFont);
    }

    static __INTRODUCED_IN (29) Typeface::Ptr findSystemTypefaceWithMatcher()
    {
        using AFontMatcherPtr = std::unique_ptr<AFontMatcher, FunctionPointerDestructor<AFontMatcher_destroy>>;
        using AFontPtr = std::unique_ptr<AFont, FunctionPointerDestructor<AFont_close>>;

        constexpr uint16_t testString[] { 't', 'e', 's', 't' };

        const AFontMatcherPtr matcher { AFontMatcher_create() };
        const AFontPtr matched { AFontMatcher_match (matcher.get(),
                                                     "system-ui",
                                                     testString,
                                                     std::size (testString),
                                                     nullptr) };

        return fromMatchedFont (matched.get());
    }

    __INTRODUCED_IN (29) Typeface::Ptr matchWithAFontMatcher (const String& text, const String& language) const
    {
        using AFontMatcherPtr = std::unique_ptr<AFontMatcher, FunctionPointerDestructor<AFontMatcher_destroy>>;
        using AFontPtr = std::unique_ptr<AFont, FunctionPointerDestructor<AFont_close>>;

        const AFontMatcherPtr matcher { AFontMatcher_create() };

        const auto weight = hb_style_get_value (hbFont.get(), HB_STYLE_TAG_WEIGHT);
        const auto italic = hb_style_get_value (hbFont.get(), HB_STYLE_TAG_ITALIC) != 0.0f;
        AFontMatcher_setStyle (matcher.get(), (uint16_t) weight, italic);

        AFontMatcher_setLocales (matcher.get(), language.toRawUTF8());

        const auto utf16 = text.toUTF16();

        const AFontPtr matched { AFontMatcher_match (matcher.get(),
                                                     readFontName (hb_font_get_face (hbFont.get()),
                                                                   HB_OT_NAME_ID_FONT_FAMILY,
                                                                   nullptr).toRawUTF8(),
                                                     unalignedPointerCast<const uint16_t*> (utf16.getAddress()),
                                                     (uint32_t) (utf16.findTerminatingNull().getAddress() - utf16.getAddress()),
                                                     nullptr) };

        return fromMatchedFont (matched.get());
    }

    static Typeface::Ptr loadCompatibleFont (const TypefaceFileAndIndex& info)
    {
        FileInputStream stream { info.file };

        if (! stream.openedOk())
            return {};

        MemoryBlock mb;
        stream.readIntoMemoryBlock (mb);

        auto result = fromMemory (DoCache::no,
                                  { static_cast<const std::byte*> (mb.getData()), mb.getSize() },
                                  (unsigned int) info.index);

        if (result == nullptr)
            return {};

        const auto tech = result->getColourGlyphFormats();
        const auto hasSupportedColours = (tech & (colourGlyphFormatCOLRv0 | colourGlyphFormatBitmap)) != 0;

        // If the font only uses unsupported colour technologies, assume it's the system emoji font
        // and try to return a compatible version of the font
        if (tech != 0 && ! hasSupportedColours)
            if (auto fallback = from (FontOptions { "NotoColorEmojiLegacy", FontValues::defaultFontHeight, Font::plain }); fallback != nullptr)
                return fallback;

        return result;
    }

    static Typeface::Ptr fromMemory (DoCache cache, Span<const std::byte> blob, unsigned int index = 0)
    {
        auto face = FontStyleHelpers::getFaceForBlob ({ reinterpret_cast<const char*> (blob.data()), blob.size() }, index);

        if (face == nullptr)
            return {};

        const auto metrics = findNonPortableMetricsForData (blob);

        return new AndroidTypeface (cache,
                                    HbFont { hb_font_create (face.get()) },
                                    metrics,
                                    readFontName (face.get(), HB_OT_NAME_ID_FONT_FAMILY, nullptr),
                                    readFontName (face.get(), HB_OT_NAME_ID_FONT_SUBFAMILY, nullptr));
    }

    static String readFontName (hb_face_t* face, hb_ot_name_id_t nameId, hb_language_t language)
    {
        unsigned int textSize{};
        textSize = hb_ot_name_get_utf8 (face, nameId, language, &textSize, nullptr);
        std::vector<char> nameString (textSize + 1, 0);
        textSize = (unsigned int) nameString.size();
        hb_ot_name_get_utf8 (face, nameId, language, &textSize, nameString.data());

        return nameString.data();
    }

    AndroidTypeface (DoCache cache,
                     std::shared_ptr<hb_font_t> fontIn,
                     TypefaceAscentDescent nonPortableMetricsIn,
                     const String& name,
                     const String& style)
        : Typeface (name, style),
          hbFont (std::move (fontIn)),
          doCache (cache),
          nonPortableMetrics (nonPortableMetricsIn)
    {
        if (doCache == DoCache::yes)
            if (auto* c = MemoryFontCache::getInstance())
                c->add ({ name, style }, { hbFont, nonPortableMetrics });
    }

    static std::tuple<MemoryBlock, TypefaceAscentDescent> getBlobForFont (const Font& font)
    {
        auto memory = loadFontAsset (font.getTypefaceName());

        if (! memory.isEmpty())
            return std::tuple (memory, findNonPortableMetricsForAsset (font.getTypefaceName()));

        const auto file = findFontFile (font);

        if (! file.exists())
        {
            // Failed to find file corresponding to this font
            jassertfalse;
            return {};
        }

        FileInputStream stream { file };

        MemoryBlock result;
        stream.readIntoMemoryBlock (result);

        return std::tuple (stream.isExhausted() ? result : MemoryBlock{}, findNonPortableMetricsForFile (file));
    }

    static File findFontFile (const Font& font)
    {
        const String styles[] { font.getTypefaceStyle(),
                                FontStyleHelpers::getStyleName (font.isBold(), font.isItalic()),
                                {} };

        for (const auto& style : styles)
            if (auto file = getFontFile (font.getTypefaceName(), style); file.exists())
                return file;

        for (auto& file : File ("/system/fonts").findChildFiles (File::findFiles, false, "*.ttf"))
            if (file.getFileName().startsWith (font.getTypefaceName()))
                return file;

        return {};
    }

    static File getFontFile (const String& family, const String& fontStyle)
    {
        return "/system/fonts/" + family + (fontStyle.isNotEmpty() ? ("-" + fontStyle) : String{}) + ".ttf";
    }

    static MemoryBlock loadFontAsset (const String& typefaceName)
    {
        auto* env = getEnv();

        const LocalRef assetManager { env->CallObjectMethod (getAppContext().get(), AndroidContext.getAssets) };

        if (assetManager == nullptr)
            return {};

        const LocalRef inputStream { env->CallObjectMethod (assetManager,
                                                            AndroidAssetManager.open,
                                                            javaString ("fonts/" + typefaceName).get()) };

        // Opening an input stream for an asset might throw if the asset isn't found
        jniCheckHasExceptionOccurredAndClear();

        if (inputStream == nullptr)
            return {};

        auto streamWrapper = makeAndroidInputStreamWrapper (inputStream);

        if (streamWrapper == nullptr)
            return {};

        MemoryBlock result;
        streamWrapper->readIntoMemoryBlock (result);

        return streamWrapper->isExhausted() ? result : MemoryBlock{};
    }

    static File getCacheFileForData (Span<const std::byte> data)
    {
        static CriticalSection cs;
        static std::map<String, File> cache;

        JNIEnv* const env = getEnv();

        const auto key = [&]
        {
            LocalRef digest (env->CallStaticObjectMethod (JavaMessageDigest, JavaMessageDigest.getInstance, javaString ("MD5").get()));
            LocalRef bytes (env->NewByteArray ((int) data.size()));

            jboolean ignore;
            auto* jbytes = env->GetByteArrayElements (bytes.get(), &ignore);
            memcpy (jbytes, data.data(), data.size());
            env->ReleaseByteArrayElements (bytes.get(), jbytes, 0);

            env->CallVoidMethod (digest.get(), JavaMessageDigest.update, bytes.get());
            LocalRef result ((jbyteArray) env->CallObjectMethod (digest.get(), JavaMessageDigest.digest));
            auto* md5Bytes = env->GetByteArrayElements (result.get(), &ignore);
            const ScopeGuard scope { [&] { env->ReleaseByteArrayElements (result.get(), md5Bytes, 0); } };

            return String::toHexString (md5Bytes, env->GetArrayLength (result.get()), 0);
        }();

        const ScopedLock lock (cs);
        auto& mapEntry = cache[key];

        if (mapEntry == File())
        {
            static const File cacheDirectory = []
            {
                auto appContext = getAppContext();

                if (appContext == nullptr)
                    return File{};

                auto* localEnv = getEnv();

                LocalRef cacheFile (localEnv->CallObjectMethod (appContext.get(), AndroidContext.getCacheDir));
                LocalRef jPath ((jstring) localEnv->CallObjectMethod (cacheFile.get(), JavaFile.getAbsolutePath));

                return File (juceString (localEnv, jPath.get()));
            }();

            mapEntry = cacheDirectory.getChildFile ("bindata_" + key);
            mapEntry.replaceWithData (data.data(), data.size());
        }

        return mapEntry;
    }

    static TypefaceAscentDescent findNonPortableMetricsForFile (File file)
    {
        auto* env = getEnv();
        const LocalRef typeface { env->CallStaticObjectMethod (TypefaceClass,
                                                               TypefaceClass.createFromFile,
                                                               javaString (file.getFullPathName()).get()) };
        return findNonPortableMetricsForTypeface (typeface);
    }

    static TypefaceAscentDescent findNonPortableMetricsForData (Span<const std::byte> bytes)
    {
        const auto file = getCacheFileForData (bytes);
        return findNonPortableMetricsForFile (file);
    }

    static TypefaceAscentDescent findNonPortableMetricsForAsset (const String& name)
    {
        auto* env = getEnv();

        const LocalRef assetManager { env->CallObjectMethod (getAppContext().get(), AndroidContext.getAssets) };
        const LocalRef typeface { env->CallStaticObjectMethod (TypefaceClass,
                                                               TypefaceClass.createFromAsset,
                                                               assetManager.get(),
                                                               javaString ("fonts/" + name).get()) };
        return findNonPortableMetricsForTypeface (typeface);
    }

    static TypefaceAscentDescent findNonPortableMetricsForTypeface (const LocalRef<jobject>& typeface)
    {
        constexpr auto referenceFontSize = 256.0f;

        auto* env = getEnv();

        jint constructorFlags = 1 /*ANTI_ALIAS_FLAG*/
                              | 2 /*FILTER_BITMAP_FLAG*/
                              | 4 /*DITHER_FLAG*/
                              | 128 /*SUBPIXEL_TEXT_FLAG*/;

        const LocalRef paint { env->NewObject (AndroidPaint, AndroidPaint.constructor, constructorFlags) };

        env->CallObjectMethod (paint, AndroidPaint.setTypeface, typeface.get());
        env->CallVoidMethod (paint, AndroidPaint.setTextSize, referenceFontSize);

        const auto fullAscent  = std::abs (env->CallFloatMethod (paint, AndroidPaint.ascent));
        const auto fullDescent = std::abs (env->CallFloatMethod (paint, AndroidPaint.descent));

        return TypefaceAscentDescent { fullAscent  / referenceFontSize,
                                       fullDescent / referenceFontSize };
    }

    std::shared_ptr<hb_font_t> hbFont;
    DoCache doCache;
    TypefaceAscentDescent nonPortableMetrics;
};

//==============================================================================
Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return AndroidTypeface::from (font);
}

Typeface::Ptr Typeface::createSystemTypefaceFor (Span<const std::byte> data)
{
    return AndroidTypeface::from (data);
}

Typeface::Ptr Typeface::findSystemTypeface()
{
    return AndroidTypeface::findSystemTypeface();
}

void Typeface::scanFolderForFonts (const File&)
{
    jassertfalse; // not currently available
}

} // namespace juce
