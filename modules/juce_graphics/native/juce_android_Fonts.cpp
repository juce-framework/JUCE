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

    Array<File> fonts;
    File ("/system/fonts").findChildFiles (fonts, File::findFiles, false, "*.ttf");

    for (int i = 0; i < fonts.size(); ++i)
        results.addIfNotAlreadyThere (fonts.getReference(i).getFileNameWithoutExtension()
                                        .upToLastOccurrenceOf ("-", false, false));

    return results;
}

StringArray Font::findAllTypefaceStyles (const String& family)
{
    StringArray results ("Regular");

    Array<File> fonts;
    File ("/system/fonts").findChildFiles (fonts, File::findFiles, false, family + "-*.ttf");

    for (int i = 0; i < fonts.size(); ++i)
        results.addIfNotAlreadyThere (fonts.getReference(i).getFileNameWithoutExtension()
                                        .fromLastOccurrenceOf ("-", false, false));

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
        : Typeface (String(), String())
    {
        JNIEnv* const env = getEnv();

        LocalRef<jbyteArray> bytes (env->NewByteArray (size));
        env->SetByteArrayRegion (bytes, 0, size, (const jbyte*) data);

        typeface = GlobalRef (android.activity.callObjectMethod (JuceAppActivity.getTypeFaceFromByteArray, bytes.get()));

        initialise (env);
    }

    void initialise (JNIEnv* const env)
    {
        rect = GlobalRef (env->NewObject (RectClass, RectClass.constructor, 0, 0, 0, 0));

        paint = GlobalRef (GraphicsHelpers::createPaint (Graphics::highResamplingQuality));
        const LocalRef<jobject> ignored (paint.callObjectMethod (Paint.setTypeface, typeface.get()));

        paint.callVoidMethod (Paint.setTextSize, referenceFontSize);

        const float fullAscent = std::abs (paint.callFloatMethod (Paint.ascent));
        const float fullDescent = paint.callFloatMethod (Paint.descent);
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

        const int numDone = paint.callIntMethod (Paint.getTextWidths, javaString (text).get(), widths);

        HeapBlock<jfloat> localWidths (numDone);
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

        const int numDone = paint.callIntMethod (Paint.getTextWidths, javaString (text).get(), widths);

        HeapBlock<jfloat> localWidths (numDone);
        env->GetFloatArrayRegion (widths, 0, numDone, localWidths);
        env->DeleteLocalRef (widths);

        String::CharPointerType s (text.getCharPointer());

        xOffsets.add (0);

        float x = 0;
        for (int i = 0; i < numDone; ++i)
        {
            glyphs.add ((int) s.getAndAdvance());
            x += localWidths[i];
            xOffsets.add (x * referenceFontToUnits);
        }
    }

    bool getOutlineForGlyph (int /*glyphNumber*/, Path& /*destPath*/) override
    {
        return false;
    }

    EdgeTable* getEdgeTableForGlyph (int glyphNumber, const AffineTransform& t, float /*fontHeight*/) override
    {
        JNIEnv* env = getEnv();

        jobject matrix = GraphicsHelpers::createMatrix (env, AffineTransform::scale (referenceFontToUnits).followedBy (t));
        jintArray maskData = (jintArray) android.activity.callObjectMethod (JuceAppActivity.renderGlyph, (jchar) glyphNumber, paint.get(), matrix, rect.get());

        env->DeleteLocalRef (matrix);

        const int left   = env->GetIntField (rect.get(), RectClass.left);
        const int top    = env->GetIntField (rect.get(), RectClass.top);
        const int right  = env->GetIntField (rect.get(), RectClass.right);
        const int bottom = env->GetIntField (rect.get(), RectClass.bottom);

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
