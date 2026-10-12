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
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (create,          "create",           "(Ljava/lang/String;I)Landroid/graphics/Typeface;") \
 STATICMETHOD (createFromFile,  "createFromFile",   "(Ljava/lang/String;)Landroid/graphics/Typeface;") \
 STATICMETHOD (createFromAsset, "createFromAsset",  "(Landroid/content/res/AssetManager;Ljava/lang/String;)Landroid/graphics/Typeface;")

 DECLARE_JNI_CLASS (TypefaceClass, "android/graphics/Typeface")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (create,                   "<init>",                   "(Ljava/nio/ByteBuffer;)V") \
 METHOD (setTtcIndex,              "setTtcIndex",              "(I)Landroid/graphics/fonts/Font$Builder;") \
 METHOD (setFontVariationSettings, "setFontVariationSettings", "(Ljava/lang/String;)Landroid/graphics/fonts/Font$Builder;") \
 METHOD (build,                    "build",                    "()Landroid/graphics/fonts/Font;")

 DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidFontBuilder, "android/graphics/fonts/Font$Builder", 29)
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

#define JUCE_INTRODUCED_IN_29 \
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wgnu-zero-variadic-macro-arguments") \
    __INTRODUCED_IN (29)                                                       \
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

// Defined in juce_core
std::unique_ptr<InputStream> makeAndroidInputStreamWrapper (LocalRef<jobject> stream);

struct AndroidCachedTypeface
{
    HbFont font;
    GlobalRef javaFont;
    TypefaceVerticalMetrics metrics;
};

//==============================================================================
class AndroidTypeface final : public Typeface,
                              private TypefaceFallbackColourGlyphSupport
{
public:
    static Ptr from (const Font& font)
    {
        const auto blobForFont = getBlobForFont (font);
        const auto& blob = blobForFont.data;
        const auto index = (unsigned int) blobForFont.collectionIndex;

        auto face = FontStyleHelpers::getFaceForBlob ({ static_cast<const char*> (blob.getData()), blob.getSize() }, index);

        if (face == nullptr)
        {
            jassertfalse;
            return {};
        }

        HbFont hbFont { hb_font_create (face.get()), IncrementRef::no };
        FontStyleHelpers::initSynthetics (hbFont.get(), font);

        const auto androidFont = shouldStoreAndroidFont (face.get())
                               ? makeAndroidFont ({ static_cast<const std::byte*> (blob.getData()), blob.getSize() }, index)
                               : GlobalRef{};

        Ptr baseFace = new AndroidTypeface (std::move (hbFont),
                                            blobForFont.metrics,
                                            androidFont);

        if (baseFace->getStyle() == font.getTypefaceStyle())
            return baseFace;

        const auto variables = baseFace->getNamedInstanceConfiguration (font.getTypefaceStyle());

        if (variables.empty())
            return baseFace;

        auto cloned = baseFace->cloneWithVariableSettings (variables);

        if (cloned == nullptr)
            return baseFace;

        return cloned;
    }

    static Ptr from (Span<const std::byte> blob, unsigned int index = 0)
    {
        return fromMemory (blob, index);
    }

    Ptr createSystemFallback (const String& text, const String& language) const override
    {
        if (__builtin_available (android 29, *))
            return matchWithAFontMatcher (text, language);

        // The font-fallback API is only available on Android API level 29+
        jassertfalse;
        return {};
    }

    Ptr cloneWithVariableSettings (Span<const FontVariableSetting> settings) const override
    {
        auto nativeDetails = getNativeDetails();
        auto blob = nativeDetails->getBlob();

        unsigned int blobSize = 0;
        auto* blobData = hb_blob_get_data (blob.get(), &blobSize);
        auto metrics = findNonPortableMetricsForData ({ (const std::byte*) blobData, blobSize });
        auto newFace = FontStyleHelpers::getFaceForBlob ({ blobData, blobSize }, 0);

        if (newFace == nullptr)
        {
            jassertfalse;
            return {};
        }

        HbFont hbFont { hb_font_create (newFace.get()), IncrementRef::no };

        const auto registry = nativeDetails->getVariableRegistry();
        auto sanitisedVariables = registry->sanitiseVariables (settings);

        const auto androidFont = shouldStoreAndroidFont (newFace.get())
                               ? makeAndroidFont ({ reinterpret_cast<const std::byte*> (blobData),
                                                    blobSize },
                                                  0,
                                                  sanitisedVariables)
                               : GlobalRef{};

        return new AndroidTypeface (std::move (hbFont),
                                    metrics,
                                    androidFont,
                                    registry,
                                    std::move (sanitisedVariables));
    }

    const Native* getNativeDetails() const override
    {
        return native.get();
    }

    static Ptr findSystemTypeface()
    {
        if (__builtin_available (android 29, *))
            return findSystemTypefaceWithMatcher();

        return from (FontOptions{}.withName ("Roboto"));
    }

    static JUCE_INTRODUCED_IN_29 auto findAFontWithMatcher (const Font& font)
    {
        struct Destructor
        {
            void operator() (AFont* x) const
            {
                if (x != nullptr)
                    AFont_close (x);
            }
        };

        using AFontPtr = std::unique_ptr<AFont, Destructor>;

        constexpr uint16_t testString[] { 't', 'e', 's', 't' };

        auto* matcher = AFontMatcher_create();
        const ScopeGuard freeMatcher { [&] { AFontMatcher_destroy (matcher); } };
        const auto weight = font.isBold() ? AFONT_WEIGHT_BOLD : AFONT_WEIGHT_NORMAL;
        const auto italic = font.isItalic();
        AFontMatcher_setStyle (matcher, weight, italic);
        return AFontPtr { AFontMatcher_match (matcher,
                                              font.getTypefaceName().toRawUTF8(),
                                              testString,
                                              std::size (testString),
                                              nullptr) };
    }

    static StringArray findAllTypefaceNames()
    {
        std::set<String> results;

        for (const auto& family : SystemFontMap::getInstance().getFamilyNames())
            results.insert (family);

        StringArray s;

        for (const auto& family : results)
            s.add (family);

        return s;
    }

    static StringArray findAllTypefaceStyles (const String& family)
    {
        std::vector<String> results;

        const auto systemStyles = SystemFontMap::getInstance().getStyles (family);
        results.insert (results.end(), systemStyles.begin(), systemStyles.end());

        std::set<String> unique;
        StringArray s;

        for (const auto& style : results)
            if (unique.insert (style).second)
                s.add (style);

        return s;
    }

    static JUCE_INTRODUCED_IN_29 Ptr fromMatchedFont (AFont* matched, uint16_t weight, bool italic)
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

        auto face = loadCompatibleFont ({ matchedFile, (int) matchedIndex });

        static const auto weightTag = FontFeatureTag::fromString ("wght");
        static const auto italicTag = FontFeatureTag::fromString ("ital");

        std::vector<FontVariableSetting> settings;

        for (auto var : face->getSupportedVariables())
        {
            if (var == weightTag)
                settings.emplace_back (weightTag, weight);

            if (var == italicTag)
                settings.emplace_back (italicTag, italic ? 1.0f : 0.0f);
        }

        return face->cloneWithVariableSettings (settings);
    }

private:
    static JUCE_INTRODUCED_IN_29 Ptr findSystemTypefaceWithMatcher()
    {
        const auto afont = findAFontWithMatcher (FontOptions{}.withName ("system-ui"));
        return fromMatchedFont (afont.get(), AFONT_WEIGHT_NORMAL, false);
    }

    JUCE_INTRODUCED_IN_29 Ptr matchWithAFontMatcher (const String& text, const String& language) const
    {
        auto* matcher = AFontMatcher_create();
        const ScopeGuard freeMatcher { [&] { AFontMatcher_destroy (matcher); } };

        const auto weight = hb_style_get_value (native->getFont(), HB_STYLE_TAG_WEIGHT);
        const auto italic = hb_style_get_value (native->getFont(), HB_STYLE_TAG_ITALIC) != 0.0f;
        AFontMatcher_setStyle (matcher, (uint16_t) weight, italic);

        AFontMatcher_setLocales (matcher, language.toRawUTF8());

        const auto utf16 = text.toUTF16();

        auto* matched = AFontMatcher_match (matcher,
                                            getName().toRawUTF8(),
                                            unalignedPointerCast<const uint16_t*> (utf16.getAddress()),
                                            (uint32_t) (utf16.findTerminatingNull().getAddress() - utf16.getAddress()),
                                            nullptr);
        const ScopeGuard freeMatched { [&] { AFont_close (matched); } };

        return fromMatchedFont (matched, (uint16_t) weight, italic);
    }

    static bool shouldStoreAndroidFont (hb_face_t* face)
    {
        return (hb_ot_color_has_svg (face) || hb_ot_color_has_paint (face))
            && ! (hb_ot_color_has_layers (face) || hb_ot_color_has_png (face));
    }

    static GlobalRef makeAndroidFont (Span<const std::byte> blob,
                                      unsigned int index,
                                      Span<const FontVariableSetting> variables = {})
    {
        auto* env = getEnv();

        LocalRef<jbyteArray> bytes { env->NewByteArray ((jint) blob.size()) };
        {
            auto* elements = env->GetByteArrayElements (bytes, nullptr);
            const ScopeGuard scope { [&] { env->ReleaseByteArrayElements (bytes, elements, 0); }};
            std::transform (blob.begin(), blob.end(), elements, [] (auto x) { return (jbyte) x; });
        }

        LocalRef<jobject> byteBuffer { env->CallStaticObjectMethod (JavaByteBuffer,
                                                                    JavaByteBuffer.allocateDirect,
                                                                    (jint) blob.size()) };
        LocalRef { env->CallObjectMethod (byteBuffer, JavaByteBuffer.put, bytes.get()) };

        LocalRef<jobject> builder { env->NewObject (AndroidFontBuilder,
                                                    AndroidFontBuilder.create,
                                                    byteBuffer.get()) };
        LocalRef { env->CallObjectMethod (builder,
                                          AndroidFontBuilder.setTtcIndex,
                                          (jint) index) };

        if (! variables.empty())
        {
            const auto androidVariableSettings = std::invoke ([variables]
            {
                String s;

                for (const auto& variable : variables)
                    s << "\'" << variable.tag.toString() << "\' " << variable.value << ", ";

                return s.dropLastCharacters (2);
            });

            LocalRef { env->CallObjectMethod (builder,
                                              AndroidFontBuilder.setFontVariationSettings,
                                              javaString (androidVariableSettings).get()) };
        }

        LocalRef<jobject> androidFont { env->CallObjectMethod (builder,
                                                               AndroidFontBuilder.build) };

        return GlobalRef { androidFont };
    }

    static Ptr loadCompatibleFont (const TypefaceFileAndIndex& key)
    {
        auto* cache = TypefaceFileCache::getInstance();

        if (cache == nullptr)
            return {}; // Perhaps we're shutting down

        return cache->get (key, [] (auto info) -> Ptr
        {
            FileInputStream stream { info.file };

            if (! stream.openedOk())
                return {};

            MemoryBlock mb;
            stream.readIntoMemoryBlock (mb);

            return fromMemory ({ static_cast<const std::byte*> (mb.getData()), mb.getSize() },
                               (unsigned int) info.index);
        });
    }

    class SystemFontMap
    {
    public:
        static const SystemFontMap& getInstance()
        {
            static const auto result = std::invoke ([&]
            {
                const auto addFontInfo = [&] (SystemFontMap& sfm,
                                              const TypefaceFileAndIndex& typefaceFileAndIndex)
                {
                    const auto face = loadCompatibleFont (typefaceFileAndIndex);

                    auto& mapForFamily = sfm.map[face->getName()];
                    mapForFamily.push_back ({ face->getStyle(), typefaceFileAndIndex });

                    for (const auto& instanceName : face->getInstanceNames())
                        mapForFamily.push_back ({ instanceName, typefaceFileAndIndex });
                };

                if (__builtin_available (android 29, *))
                {
                    if (auto* iterator = ASystemFontIterator_open())
                    {
                        const ScopeGuard closeIterator { [&] { ASystemFontIterator_close (iterator); } };

                        SystemFontMap sfm;

                        while (auto* font = ASystemFontIterator_next (iterator))
                        {
                            const ScopeGuard closeFont { [&] { AFont_close (font); } };
                            const File path { AFont_getFontFilePath (font) };
                            const auto index = AFont_getCollectionIndex (font);
                            addFontInfo (sfm, { path, (int) index });
                        }

                        return sfm;
                    }
                }

                SystemFontMap sfm;

                for (auto& f : File ("/system/fonts").findChildFiles (File::findFiles, false, "*.ttf"))
                    addFontInfo (sfm, { f, 0 });

                return sfm;
            });

            return result;
        }

        std::set<String> getFamilyNames() const
        {
            std::set<String> result;

            for (const auto& pair : map)
                result.insert (pair.first);

            return result;
        }

        std::vector<String> getStyles (const String& family) const
        {
            const auto iter = map.find (family);

            if (iter == map.end())
                return {};

            std::vector<String> result;

            for (const auto& pair : iter->second)
                result.push_back (pair.style);

            return result;
        }

        TypefaceFileAndIndex getTypefaceFileAndIndex (const String& family,
                                                      const String& style) const
        {
            const auto familyIter = map.find (family);

            if (familyIter == map.end())
                return {};

            // If the requested style isn't present, try to find a standard style
            for (const auto& styleToTry : { style, String ("Regular"), String() })
            {
                const auto styleIter = std::find_if (familyIter->second.begin(),
                                                     familyIter->second.end(),
                                                     [&] (const auto& p) { return p.style == styleToTry; });

                if (styleIter != familyIter->second.end())
                    return styleIter->fileAndIndex;
            }

            // If the requested style and the regular style are not present, just get the first style
            if (! familyIter->second.empty())
                return familyIter->second.front().fileAndIndex;

            // Somehow the cache contains a family name with no associated styles!
            jassertfalse;
            return {};
        }

    private:
        SystemFontMap() = default;

        struct FileAndIndexForStyle
        {
            String style;
            TypefaceFileAndIndex fileAndIndex;
        };

        std::map<String, std::vector<FileAndIndexForStyle>> map;
    };

    static Ptr fromMemory (Span<const std::byte> blob, unsigned int index)
    {
        auto hbFace = FontStyleHelpers::getFaceForBlob ({ reinterpret_cast<const char*> (blob.data()), blob.size() }, index);

        if (hbFace == nullptr)
            return {};

        const auto metrics = findNonPortableMetricsForData (blob);
        auto* face = hbFace.get();

        return new AndroidTypeface (HbFont (hb_font_create (face), IncrementRef::no),
                                    metrics,
                                    shouldStoreAndroidFont (face) ? makeAndroidFont (blob, index) : GlobalRef{});
    }

    AndroidTypeface (HbFont fontIn,
                     TypefaceVerticalMetrics nonPortableMetricsIn,
                     GlobalRef javaFontIn,
                     std::shared_ptr<VariableAxisRegistry> variableAxisRegistry = {},
                     std::vector<FontVariableSetting> settings = {})
        : AndroidTypeface (std::make_unique<Native> (TypefaceNativeOptions { fontIn,
                                                                             nonPortableMetricsIn,
                                                                             std::move (settings),
                                                                             this,
                                                                             {},
                                                                             variableAxisRegistry }),
                           javaFontIn)
    {
    }

    AndroidTypeface (std::unique_ptr<Native> nativeIn,
                     GlobalRef javaFontIn)
        : Typeface (nativeIn->getTypefaceName(), nativeIn->getTypefaceStyle()),
          javaFont (std::move (javaFontIn)),
          native (std::move (nativeIn))
    {
    }

    struct BlobForFont
    {
        MemoryBlock data;
        size_t collectionIndex;
        TypefaceVerticalMetrics metrics;
    };

    static BlobForFont getBlobForFont (const Font& font)
    {
        auto memory = loadFontAsset (font.getTypefaceName());

        if (! memory.isEmpty())
            return { memory, 0, findNonPortableMetricsForAsset (font.getTypefaceName()) };

        const auto [file, index] = findFontFileAndIndex (font);

        if (! file.exists())
        {
            // Failed to find file corresponding to this font
            jassertfalse;
            return {};
        }

        FileInputStream stream { file };

        MemoryBlock result;
        stream.readIntoMemoryBlock (result);

        return { stream.isExhausted() ? result : MemoryBlock{},
                 (size_t) index,
                 findNonPortableMetricsForFile (file) };
    }

    static TypefaceFileAndIndex findFontFileAndIndex (const Font& font)
    {
        // We deliberately *don't* just use AFontMatcher_match to try to find a font with the
        // requested family name, since this function only works for 'generic' family names like
        // sans-serif, serif, monospace, system-ui, and so on. Instead, we load all the system fonts
        // and try to match the requested name and style.

        const String styles[] { font.getTypefaceStyle(),
                                FontStyleHelpers::getStyleName (font.isBold(), font.isItalic()),
                                {} };

        for (const auto& style : styles)
        {
            if (const auto pair = SystemFontMap::getInstance().getTypefaceFileAndIndex (font.getTypefaceName(), style);
                pair.file.existsAsFile())
            {
                return pair;
            }
        }

        return {};
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

        const auto key = std::invoke ([&]
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
        });

        const ScopedLock lock (cs);
        auto& mapEntry = cache[key];

        if (mapEntry == File())
        {
            static const File cacheDirectory = std::invoke ([]
            {
                auto appContext = getAppContext();

                if (appContext == nullptr)
                    return File{};

                auto* localEnv = getEnv();

                LocalRef cacheFile (localEnv->CallObjectMethod (appContext.get(), AndroidContext.getCacheDir));
                LocalRef jPath ((jstring) localEnv->CallObjectMethod (cacheFile.get(), JavaFile.getAbsolutePath));

                return File (juceString (localEnv, jPath.get()));
            });

            mapEntry = cacheDirectory.getChildFile ("bindata_" + key);
            mapEntry.replaceWithData (data.data(), data.size());
        }

        return mapEntry;
    }

    static TypefaceVerticalMetrics findNonPortableMetricsForFile (const File& file)
    {
        auto* env = getEnv();
        const LocalRef typeface { env->CallStaticObjectMethod (TypefaceClass,
                                                               TypefaceClass.createFromFile,
                                                               javaString (file.getFullPathName()).get()) };
        return findNonPortableMetricsForTypeface (typeface);
    }

    static TypefaceVerticalMetrics findNonPortableMetricsForData (Span<const std::byte> bytes)
    {
        const auto file = getCacheFileForData (bytes);
        return findNonPortableMetricsForFile (file);
    }

    static TypefaceVerticalMetrics findNonPortableMetricsForAsset (const String& name)
    {
        auto* env = getEnv();

        const LocalRef assetManager { env->CallObjectMethod (getAppContext().get(), AndroidContext.getAssets) };
        const LocalRef typeface { env->CallStaticObjectMethod (TypefaceClass,
                                                               TypefaceClass.createFromAsset,
                                                               assetManager.get(),
                                                               javaString ("fonts/" + name).get()) };
        return findNonPortableMetricsForTypeface (typeface);
    }

    static TypefaceVerticalMetrics findNonPortableMetricsForTypeface (const LocalRef<jobject>& typeface)
    {
        constexpr auto referenceFontSize = 256.0f;

        auto* env = getEnv();

        jint constructorFlags = 1 /*ANTI_ALIAS_FLAG*/
                              | 2 /*FILTER_BITMAP_FLAG*/
                              | 4 /*DITHER_FLAG*/
                              | 128 /*SUBPIXEL_TEXT_FLAG*/;

        const LocalRef paint { env->NewObject (AndroidPaint, AndroidPaint.constructor, constructorFlags) };

        LocalRef { env->CallObjectMethod (paint, AndroidPaint.setTypeface, typeface.get()) };
        env->CallVoidMethod (paint, AndroidPaint.setTextSize, referenceFontSize);

        const auto fullAscent  = std::abs (env->CallFloatMethod (paint, AndroidPaint.ascent));
        const auto fullDescent = std::abs (env->CallFloatMethod (paint, AndroidPaint.descent));

        return TypefaceVerticalMetrics { fullAscent  / referenceFontSize,
                                         fullDescent / referenceFontSize,
                                         0.0f };
    }

    std::vector<GlyphLayer> getFallbackColourGlyphLayers (int glyph,
                                                          const AffineTransform& transform) const override
    {
        // Canvas.drawGlyphs is only available from API 31
        if (getAndroidSDKVersion() < 31)
            return {};

        auto* env = getEnv();

        const auto extents = native->getGlyphExtents ((hb_codepoint_t) glyph);

        if (! extents.has_value())
        {
            // Trying to retrieve an image for a glyph that's not present in the font?
            jassertfalse;
            return {};
        }

        const auto upem = (jint) hb_face_get_upem (hb_font_get_face (native->getFont()));
        constexpr jint referenceSize = 128;

        const jint pixelW = (referenceSize * abs (extents->width))  / upem;
        const jint pixelH = (referenceSize * abs (extents->height)) / upem;
        const jint pixelBearingX = (referenceSize * extents->x_bearing) / upem;
        const jint pixelBearingY = (referenceSize * extents->y_bearing) / upem;

        const jint pixelPadding = 2;

        const auto totalW = (size_t) (pixelW + pixelPadding * 2);
        const auto totalH = (size_t) (pixelH + pixelPadding * 2);

        LocalRef<jobject> bitmapConfig { env->CallStaticObjectMethod (AndroidBitmapConfig,
                                                                      AndroidBitmapConfig.valueOf,
                                                                      javaString ("ARGB_8888").get()) };

        LocalRef<jobject> bitmap { env->CallStaticObjectMethod (AndroidBitmap,
                                                                AndroidBitmap.createBitmap,
                                                                totalW,
                                                                totalH,
                                                                bitmapConfig.get()) };

        LocalRef<jobject> canvas { env->NewObject (AndroidCanvas, AndroidCanvas.create, bitmap.get())};

        const jint glyphIdsIn[] { glyph };
        LocalRef<jintArray> glyphIds { env->NewIntArray (std::size (glyphIdsIn)) };
        env->SetIntArrayRegion (glyphIds, 0, std::size (glyphIdsIn), glyphIdsIn);

        const jfloat pos[] { (float) (pixelPadding - pixelBearingX),
                             (float) (pixelPadding + pixelBearingY) };
        LocalRef<jfloatArray> positions { env->NewFloatArray (std::size (pos)) };
        env->SetFloatArrayRegion (positions, 0, std::size (pos), pos);

        LocalRef<jobject> paint { env->NewObject (AndroidPaint, AndroidPaint.defaultConstructor) };
        env->CallVoidMethod (paint, AndroidPaint.setTextSize, (jfloat) referenceSize);

        env->CallVoidMethod (canvas,
                             AndroidCanvas31.drawGlyphs,
                             glyphIds.get(),
                             0,
                             positions.get(),
                             0,
                             (jint) std::size (glyphIdsIn),
                             javaFont.get(),
                             paint.get());

        LocalRef<jintArray> pixels { env->NewIntArray ((jint) totalW * (jint) totalH) };
        env->CallVoidMethod (bitmap,
                             AndroidBitmap.getPixels,
                             pixels.get(),
                             0,
                             totalW,
                             0,
                             0,
                             totalW,
                             totalH);

        auto* colours = env->GetIntArrayElements (pixels, nullptr);

        ScopeGuard scope { [&] { env->ReleaseIntArrayElements (pixels, colours, JNI_ABORT); } };

        Image resultImage { Image::ARGB, (int) totalW, (int) totalH, false };

        // This image will be upside-down, but we'll use the final transform to flip it
        {
            Image::BitmapData bitmapData { resultImage, Image::BitmapData::writeOnly };

            for (size_t y = 0; y < totalH; ++y)
            {
                for (size_t x = 0; x < totalW; ++x)
                {
                    bitmapData.setPixelColour ((int) x,
                                               (int) y,
                                               Colour ((uint32) colours[x + y * totalW]));
                }
            }
        }

        const auto scaleFactor = (float) upem / (float) referenceSize;
        return { GlyphLayer { ImageLayer { resultImage,
                                           AffineTransform::translation ((float) pixelBearingX,
                                                                         (float) -pixelBearingY)
                                               .scaled (scaleFactor, -scaleFactor)
                                               .followedBy (transform) } } };
    }

    GlobalRef javaFont;
    std::unique_ptr<Native> native;
};

//==============================================================================
auto Typeface::createFromFontImpl (const Font& font) -> Ptr
{
    return AndroidTypeface::from (font);
}

auto Typeface::createFromDataImpl (Span<const std::byte> data) -> Ptr
{
    return AndroidTypeface::from (data);
}

auto Typeface::findSystemTypeface() -> Ptr
{
    return AndroidTypeface::findSystemTypeface();
}

void Typeface::scanFolderForFonts (const File&)
{
    jassertfalse; // not currently available
}

//==============================================================================
StringArray Font::findAllTypefaceNamesImpl()
{
    return AndroidTypeface::findAllTypefaceNames();
}

StringArray Font::findAllTypefaceStylesImpl (const String& family)
{
    return AndroidTypeface::findAllTypefaceStyles (family);
}

Typeface::Ptr Font::Native::getDefaultPlatformTypefaceForFont (const Font& font)
{
    const auto faceName = font.getTypefaceName();

    const auto idealFace = std::invoke ([&]() -> Typeface::Ptr
    {
        if (__builtin_available (android 29, *))
        {
            const auto nameToFind = std::invoke ([&]
            {
                if (faceName == Font::getDefaultSansSerifFontName())  return "sans-serif";
                if (faceName == Font::getDefaultSerifFontName())      return "serif";
                if (faceName == Font::getDefaultMonospacedFontName()) return "monospace";
                return "";
            });

            if (strlen (nameToFind) == 0)
                return nullptr;

            auto copy = font;
            copy.setTypefaceName (nameToFind);
            const auto afont = AndroidTypeface::findAFontWithMatcher (copy);
            const auto weight = font.isBold() ? AFONT_WEIGHT_BOLD : AFONT_WEIGHT_NORMAL;
            const auto italic = font.isItalic();

            return AndroidTypeface::fromMatchedFont (afont.get(), weight, italic);
        }

        return nullptr;
    });

    if (idealFace != nullptr)
        return idealFace;

    Font f (font);
    f.setTypefaceName (std::invoke ([&]() -> String
    {
        if (faceName == Font::getDefaultSansSerifFontName())    return "Roboto";
        if (faceName == Font::getDefaultSerifFontName())        return "Roboto";
        if (faceName == Font::getDefaultMonospacedFontName())   return "Roboto";

        return faceName;
    }));

    return Typeface::createSystemTypefaceFor (f);
}

} // namespace juce
