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
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 STATICMETHOD (create,          "create",           "(Ljava/lang/String;I)Landroid/graphics/Typeface;") \
 STATICMETHOD (createFromFile,  "createFromFile",   "(Ljava/lang/String;)Landroid/graphics/Typeface;") \

DECLARE_JNI_CLASS (TypefaceClass, "android/graphics/Typeface");
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
        typeface = GlobalRef (android.activity.callObjectMethod (JuceAppActivity.getTypeFaceFromAsset,
                                                                 javaString ("fonts/" + name).get()));

        if (typeface.get() == nullptr)
        {
            const bool isBold   = style.contains ("Bold");
            const bool isItalic = style.contains ("Italic");

            File fontFile (getFontFile (name, style));

            if (! fontFile.exists())
                fontFile = findFontFile (name, isBold, isItalic);

            if (fontFile.exists())
                typeface = GlobalRef (env->CallStaticObjectMethod (TypefaceClass, TypefaceClass.createFromFile,
                                                                   javaString (fontFile.getFullPathName()).get()));
            else
                typeface = GlobalRef (env->CallStaticObjectMethod (TypefaceClass, TypefaceClass.create,
                                                                   javaString (getName()).get(),
                                                                   (isBold ? 1 : 0) + (isItalic ? 2 : 0)));
        }

        initialise (env);
    }

    AndroidTypeface (const void* data, size_t size)
        : Typeface (String (static_cast<uint64> (reinterpret_cast<uintptr_t> (data))), String())
    {
        JNIEnv* const env = getEnv();

        LocalRef<jbyteArray> bytes (env->NewByteArray ((jsize) size));
        env->SetByteArrayRegion (bytes, 0, (jsize) size, (const jbyte*) data);

        typeface = GlobalRef (android.activity.callObjectMethod (JuceAppActivity.getTypeFaceFromByteArray, bytes.get()));

        initialise (env);
    }

    void initialise (JNIEnv* const env)
    {
        rect = GlobalRef (env->NewObject (AndroidRectClass, AndroidRectClass.constructor, 0, 0, 0, 0));

        paint = GlobalRef (GraphicsHelpers::createPaint (Graphics::highResamplingQuality));
        const LocalRef<jobject> ignored (paint.callObjectMethod (AndroidPaint.setTypeface, typeface.get()));

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
        const int numChars = text.length();
        jfloatArray widths = env->NewFloatArray (numChars);

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
        const int numChars = text.length();
        jfloatArray widths = env->NewFloatArray (numChars);

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

        JNIEnv* env = getEnv();

        jobject matrix = GraphicsHelpers::createMatrix (env, AffineTransform::scale (referenceFontToUnits).followedBy (t));
        jintArray maskData = (jintArray) android.activity.callObjectMethod (JuceAppActivity.renderGlyph, ch1, ch2, paint.get(), matrix, rect.get());

        env->DeleteLocalRef (matrix);

        const int left   = env->GetIntField (rect.get(), AndroidRectClass.left);
        const int top    = env->GetIntField (rect.get(), AndroidRectClass.top);
        const int right  = env->GetIntField (rect.get(), AndroidRectClass.right);
        const int bottom = env->GetIntField (rect.get(), AndroidRectClass.bottom);

        const Rectangle<int> bounds (left, top, right - left, bottom - top);

        EdgeTable* et = nullptr;

        if (! bounds.isEmpty())
        {
            et = new EdgeTable (bounds);

            jint* const maskDataElements = env->GetIntArrayElements (maskData, 0);
            const jint* mask = maskDataElements;

            for (int y = top; y < bottom; ++y)
            {
               #if JUCE_LITTLE_ENDIAN
                const uint8* const lineBytes = ((const uint8*) mask) + 3;
               #else
                const uint8* const lineBytes = (const uint8*) mask;
               #endif

                et->clipLineToMask (left, y, lineBytes, 4, bounds.getWidth());
                mask += bounds.getWidth();
            }

            env->ReleaseIntArrayElements (maskData, maskDataElements, 0);
        }

        env->DeleteLocalRef (maskData);
        return et;
    }

    GlobalRef typeface, paint, rect;
    float ascent, descent, heightToPointsFactor;

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

    static File getFontFile (const String& family, const String& style)
    {
        String path ("/system/fonts/" + family);

        if (style.isNotEmpty())
            path << '-' << style;

        return File (path + ".ttf");
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
