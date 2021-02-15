/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

struct DefaultFontNames
{
    DefaultFontNames()
        : defaultSans  ("sans"),
          defaultSerif ("serif"),
          defaultFixed ("monospace"),
          defaultFallback ("sans")
    {
    }

    String getRealFontName (const String& faceName) const
    {
        if (faceName == Font::getDefaultSansSerifFontName())    return defaultSans;
        if (faceName == Font::getDefaultSerifFontName())        return defaultSerif;
        if (faceName == Font::getDefaultMonospacedFontName())   return defaultFixed;

        return faceName;
    }

    String defaultSans, defaultSerif, defaultFixed, defaultFallback;
};

Typeface::Ptr Font::getDefaultTypefaceForFont (const Font& font)
{
    static DefaultFontNames defaultNames;

    Font f (font);
    f.setTypefaceName (defaultNames.getRealFontName (font.getTypefaceName()));
    return Typeface::createSystemTypefaceFor (f);
}

//==============================================================================
#if JUCE_USE_FREETYPE

StringArray FTTypefaceList::getDefaultFontDirectories()
{
    return StringArray ("/system/fonts");
}

Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return new FreeTypeTypeface (font);
}

void Typeface::scanFolderForFonts (const File& folder)
{
    FTTypefaceList::getInstance()->scanFontPaths (StringArray (folder.getFullPathName()));
}

StringArray Font::findAllTypefaceNames()
{
    return FTTypefaceList::getInstance()->findAllFamilyNames();
}

StringArray Font::findAllTypefaceStyles (const String& family)
{
    return FTTypefaceList::getInstance()->findAllTypefaceStyles (family);
}

bool TextLayout::createNativeLayout (const AttributedString&)
{
    return false;
}

#else

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

//==============================================================================
StringArray Font::findAllTypefaceNames()
{
    StringArray results;

    for (auto& f : File ("/system/fonts").findChildFiles (File::findFiles, false, "*.ttf"))
        results.addIfNotAlreadyThere (f.getFileNameWithoutExtension().upToLastOccurrenceOf ("-", false, false));

    return results;
}

StringArray Font::findAllTypefaceStyles (const String& family)
{
    StringArray results ("Regular");

    for (auto& f : File ("/system/fonts").findChildFiles (File::findFiles, false, family + "-*.ttf"))
        results.addIfNotAlreadyThere (f.getFileNameWithoutExtension().fromLastOccurrenceOf ("-", false, false));

    return results;
}

const float referenceFontSize = 256.0f;
const float referenceFontToUnits = 1.0f / referenceFontSize;

//==============================================================================
class AndroidTypeface   : public Typeface
{
public:
    AndroidTypeface (const Font& font)
        : Typeface (font.getTypefaceName(), font.getTypefaceStyle()),
          ascent (0), descent (0), heightToPointsFactor (1.0f)
    {
        JNIEnv* const env = getEnv();

        // First check whether there's an embedded asset with this font name:
        typeface = GlobalRef (getTypefaceFromAsset (name));

        if (typeface.get() == nullptr)
        {
            const bool isBold   = style.contains ("Bold");
            const bool isItalic = style.contains ("Italic");

            File fontFile (getFontFile (name, style));

            if (! fontFile.exists())
                fontFile = findFontFile (name, isBold, isItalic);

            if (fontFile.exists())
                typeface = GlobalRef (LocalRef<jobject>(env->CallStaticObjectMethod (TypefaceClass, TypefaceClass.createFromFile,
                                                                                     javaString (fontFile.getFullPathName()).get())));
            else
                typeface = GlobalRef (LocalRef<jobject>(env->CallStaticObjectMethod (TypefaceClass, TypefaceClass.create,
                                                                                     javaString (getName()).get(),
                                                                                     (isBold ? 1 : 0) + (isItalic ? 2 : 0))));
        }

        initialise (env);
    }

    AndroidTypeface (const void* data, size_t size)
        : Typeface (String (static_cast<uint64> (reinterpret_cast<uintptr_t> (data))), String())
    {
        auto* env = getEnv();
        auto cacheFile = getCacheFileForData (data, size);

        typeface = GlobalRef (LocalRef<jobject>(env->CallStaticObjectMethod (TypefaceClass, TypefaceClass.createFromFile,
                                                                             javaString (cacheFile.getFullPathName()).get())));

        initialise (env);
    }

    void initialise (JNIEnv* const env)
    {
        rect = GlobalRef (LocalRef<jobject>(env->NewObject (AndroidRect, AndroidRect.constructor, 0, 0, 0, 0)));

        paint = GlobalRef (GraphicsHelpers::createPaint (Graphics::highResamplingQuality));
        const LocalRef<jobject> ignored (paint.callObjectMethod (AndroidPaint.setTypeface, typeface.get()));

        charArray = GlobalRef (LocalRef<jobject>((jobject) env->NewCharArray (2)));

        paint.callVoidMethod (AndroidPaint.setTextSize, referenceFontSize);

        const float fullAscent = std::abs (paint.callFloatMethod (AndroidPaint.ascent));
        const float fullDescent = paint.callFloatMethod (AndroidPaint.descent);
        const float totalHeight = fullAscent + fullDescent;

        ascent  = fullAscent / totalHeight;
        descent = fullDescent / totalHeight;
        heightToPointsFactor = referenceFontSize / totalHeight;
    }

    float getAscent() const override                 { return ascent; }
    float getDescent() const override                { return descent; }
    float getHeightToPointsFactor() const override   { return heightToPointsFactor; }

    float getStringWidth (const String& text) override
    {
        JNIEnv* env = getEnv();
        auto numChars = CharPointer_UTF16::getBytesRequiredFor (text.getCharPointer());
        jfloatArray widths = env->NewFloatArray ((int) numChars);

        const int numDone = paint.callIntMethod (AndroidPaint.getTextWidths, javaString (text).get(), widths);

        HeapBlock<jfloat> localWidths (static_cast<size_t> (numDone));
        env->GetFloatArrayRegion (widths, 0, numDone, localWidths);
        env->DeleteLocalRef (widths);

        float x = 0;

        for (int i = 0; i < numDone; ++i)
            x += localWidths[i];

        return x * referenceFontToUnits;
    }

    void getGlyphPositions (const String& text, Array<int>& glyphs, Array<float>& xOffsets) override
    {
        JNIEnv* env = getEnv();
        auto numChars = CharPointer_UTF16::getBytesRequiredFor (text.getCharPointer());
        jfloatArray widths = env->NewFloatArray ((int) numChars);

        const int numDone = paint.callIntMethod (AndroidPaint.getTextWidths, javaString (text).get(), widths);

        HeapBlock<jfloat> localWidths (static_cast<size_t> (numDone));
        env->GetFloatArrayRegion (widths, 0, numDone, localWidths);
        env->DeleteLocalRef (widths);

        auto s = text.getCharPointer();

        xOffsets.add (0);
        float x = 0;

        for (int i = 0; i < numDone; ++i)
        {
            const float local = localWidths[i];

            // Android uses jchar (UTF-16) characters
            jchar ch = (jchar) s.getAndAdvance();

            // Android has no proper glyph support, so we have to do
            // a hacky workaround for ligature detection

           #if JUCE_STRING_UTF_TYPE <= 16
            static_assert (sizeof (int) >= (sizeof (jchar) * 2), "Unable store two java chars in one glyph");

            // if the width of this glyph is zero inside the string but has
            // a width on it's own, then it's probably due to ligature
            if (local == 0.0f && glyphs.size() > 0 && getStringWidth (String (ch)) > 0.0f)
            {
                // modify the previous glyph
                int& glyphNumber = glyphs.getReference (glyphs.size() - 1);

                // make sure this is not a three character ligature
                if (glyphNumber < std::numeric_limits<jchar>::max())
                {
                    const unsigned int previousGlyph
                        = static_cast<unsigned int> (glyphNumber) & ((1U << (sizeof (jchar) * 8U)) - 1U);
                    const unsigned int thisGlyph
                        = static_cast<unsigned int> (ch)          & ((1U << (sizeof (jchar) * 8U)) - 1U);

                    glyphNumber = static_cast<int> ((thisGlyph << (sizeof (jchar) * 8U)) | previousGlyph);
                    ch = 0;
                }
            }
           #endif

            glyphs.add ((int) ch);
            x += local;
            xOffsets.add (x * referenceFontToUnits);
        }
    }

    bool getOutlineForGlyph (int /*glyphNumber*/, Path& /*destPath*/) override
    {
        return false;
    }

    EdgeTable* getEdgeTableForGlyph (int glyphNumber, const AffineTransform& t, float /*fontHeight*/) override
    {
       #if JUCE_STRING_UTF_TYPE <= 16
        static_assert (sizeof (int) >= (sizeof (jchar) * 2), "Unable store two jni chars in one int");

        // glyphNumber of zero is used to indicate that the last character was a ligature
        if (glyphNumber == 0) return nullptr;

        jchar ch1 = (static_cast<unsigned int> (glyphNumber) >> 0)                     & ((1U << (sizeof (jchar) * 8U)) - 1U);
        jchar ch2 = (static_cast<unsigned int> (glyphNumber) >> (sizeof (jchar) * 8U)) & ((1U << (sizeof (jchar) * 8U)) - 1U);
       #else
        jchar ch1 = glyphNumber, ch2 = 0;
       #endif
        Rectangle<int> bounds;
        auto* env = getEnv();

        {
            LocalRef<jobject> matrix (GraphicsHelpers::createMatrix (env, AffineTransform::scale (referenceFontToUnits).followedBy (t)));

            jboolean isCopy;
            auto* buffer = env->GetCharArrayElements ((jcharArray) charArray.get(), &isCopy);

            buffer[0] = ch1; buffer[1] = ch2;
            env->ReleaseCharArrayElements ((jcharArray) charArray.get(), buffer, 0);

            LocalRef<jobject> path (env->NewObject (AndroidPath, AndroidPath.constructor));
            LocalRef<jobject> boundsF (env->NewObject (AndroidRectF, AndroidRectF.constructor));


            env->CallVoidMethod (paint.get(), AndroidPaint.getCharsPath, charArray.get(), 0, (ch2 != 0 ? 2 : 1), 0.0f, 0.0f, path.get());

            env->CallVoidMethod (path.get(), AndroidPath.computeBounds, boundsF.get(), 1);

            env->CallBooleanMethod (matrix.get(), AndroidMatrix.mapRect, boundsF.get());

            env->CallVoidMethod (boundsF.get(), AndroidRectF.roundOut, rect.get());

            bounds = Rectangle<int>::leftTopRightBottom (env->GetIntField (rect.get(), AndroidRect.left) - 1,
                                                         env->GetIntField (rect.get(), AndroidRect.top),
                                                         env->GetIntField (rect.get(), AndroidRect.right) + 1,
                                                         env->GetIntField (rect.get(), AndroidRect.bottom));

            auto w = bounds.getWidth();
            auto h = jmax (1, bounds.getHeight());

            LocalRef<jobject> bitmapConfig (env->CallStaticObjectMethod (AndroidBitmapConfig, AndroidBitmapConfig.valueOf, javaString ("ARGB_8888").get()));
            LocalRef<jobject> bitmap (env->CallStaticObjectMethod (AndroidBitmap, AndroidBitmap.createBitmap, w, h, bitmapConfig.get()));
            LocalRef<jobject> canvas (env->NewObject (AndroidCanvas, AndroidCanvas.create, bitmap.get()));

            env->CallBooleanMethod (matrix.get(), AndroidMatrix.postTranslate, (float) -bounds.getX(), (float) -bounds.getY());
            env->CallVoidMethod (canvas.get(), AndroidCanvas.setMatrix, matrix.get());
            env->CallVoidMethod (canvas.get(), AndroidCanvas.drawPath, path.get(), paint.get());

            int requiredRenderArraySize = w * h;
            if (requiredRenderArraySize > lastCachedRenderArraySize)
            {
                cachedRenderArray = GlobalRef (LocalRef<jobject> ((jobject) env->NewIntArray (requiredRenderArraySize)));
                lastCachedRenderArraySize = requiredRenderArraySize;
            }

            env->CallVoidMethod (bitmap.get(), AndroidBitmap.getPixels, cachedRenderArray.get(), 0, w, 0, 0, w, h);
            env->CallVoidMethod (bitmap.get(), AndroidBitmap.recycle);
        }

        EdgeTable* et = nullptr;

        if (! bounds.isEmpty())
        {
            et = new EdgeTable (bounds);

            jint* const maskDataElements = env->GetIntArrayElements ((jintArray) cachedRenderArray.get(), nullptr);
            const jint* mask = maskDataElements;

            for (int y = bounds.getY(); y < bounds.getBottom(); ++y)
            {
               #if JUCE_LITTLE_ENDIAN
                const uint8* const lineBytes = ((const uint8*) mask) + 3;
               #else
                const uint8* const lineBytes = (const uint8*) mask;
               #endif

                et->clipLineToMask (bounds.getX(), y, lineBytes, 4, bounds.getWidth());
                mask += bounds.getWidth();
            }

            env->ReleaseIntArrayElements ((jintArray) cachedRenderArray.get(), maskDataElements, 0);
        }

        return et;
    }

    GlobalRef typeface, paint, rect, charArray, cachedRenderArray;
    float ascent, descent, heightToPointsFactor;
    int lastCachedRenderArraySize = -1;

private:
    static File findFontFile (const String& family,
                              const bool bold, const bool italic)
    {
        File file;

        if (bold || italic)
        {
            String suffix;
            if (bold)   suffix = "Bold";
            if (italic) suffix << "Italic";

            file = getFontFile (family, suffix);

            if (file.exists())
                return file;
        }

        file = getFontFile (family, "Regular");

        if (! file.exists())
            file = getFontFile (family, String());

        return file;
    }

    static File getFontFile (const String& family, const String& fontStyle)
    {
        String path ("/system/fonts/" + family);

        if (fontStyle.isNotEmpty())
            path << '-' << fontStyle;

        return File (path + ".ttf");
    }

    static LocalRef<jobject> getTypefaceFromAsset (const String& typefaceName)
    {
        auto* env = getEnv();

        LocalRef<jobject> assetManager (env->CallObjectMethod (getAppContext().get(), AndroidContext.getAssets));

        if (assetManager == nullptr)
            return LocalRef<jobject>();

        auto assetTypeface = env->CallStaticObjectMethod (TypefaceClass, TypefaceClass.createFromAsset, assetManager.get(),
                                                          javaString ("fonts/" + typefaceName).get());

        // this may throw
        if (env->ExceptionCheck() != 0)
        {
            env->ExceptionClear();
            return LocalRef<jobject>();
        }

        return LocalRef<jobject> (assetTypeface);
    }

    static File getCacheDirectory()
    {
        static File result = []()
        {
            auto appContext = getAppContext();

            if (appContext != nullptr)
            {
                auto* env = getEnv();

                LocalRef<jobject> cacheFile (env->CallObjectMethod (appContext.get(), AndroidContext.getCacheDir));
                LocalRef<jstring> jPath ((jstring) env->CallObjectMethod (cacheFile.get(), JavaFile.getAbsolutePath));

                return File (juceString (env, jPath.get()));
            }

            jassertfalse;
            return File();
        } ();

        return result;
    }

    static HashMap<String, File>& getInMemoryFontCache()
    {
        static HashMap<String, File> cache;
        return cache;
    }

    static File getCacheFileForData (const void* data, size_t size)
    {
        static CriticalSection cs;
        JNIEnv* const env = getEnv();

        String key;
        {
            LocalRef<jobject> digest (env->CallStaticObjectMethod (JavaMessageDigest, JavaMessageDigest.getInstance, javaString("MD5").get()));
            LocalRef<jbyteArray> bytes(env->NewByteArray ((int) size));

            jboolean ignore;
            auto* jbytes = env->GetByteArrayElements(bytes.get(), &ignore);
            memcpy(jbytes, data, size);
            env->ReleaseByteArrayElements(bytes.get(), jbytes, 0);

            env->CallVoidMethod(digest.get(), JavaMessageDigest.update, bytes.get());
            LocalRef<jbyteArray> result((jbyteArray) env->CallObjectMethod(digest.get(), JavaMessageDigest.digest));

            auto* md5Bytes = env->GetByteArrayElements(result.get(), &ignore);
            key = String::toHexString(md5Bytes, env->GetArrayLength(result.get()), 0);
            env->ReleaseByteArrayElements(result.get(), md5Bytes, 0);
        }

        ScopedLock lock (cs);
        auto& mapEntry = getInMemoryFontCache().getReference (key);

        if (mapEntry == File())
        {
            mapEntry = getCacheDirectory().getChildFile ("bindata_" + key);
            mapEntry.replaceWithData (data, size);
        }

        return mapEntry;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidTypeface)
};

//==============================================================================
Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return new AndroidTypeface (font);
}

Typeface::Ptr Typeface::createSystemTypefaceFor (const void* data, size_t size)
{
    return new AndroidTypeface (data, size);
}

void Typeface::scanFolderForFonts (const File&)
{
    jassertfalse; // not available unless using FreeType
}

bool TextLayout::createNativeLayout (const AttributedString&)
{
    return false;
}

#endif

} // namespace juce
