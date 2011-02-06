/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

// (This file gets included by juce_android_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
const StringArray Font::findAllTypefaceNames()
{
    StringArray results;

    Array<File> fonts;
    File ("/system/fonts").findChildFiles (fonts, File::findFiles, false, "*.ttf");

    for (int i = 0; i < fonts.size(); ++i)
        results.add (fonts.getReference(i).getFileNameWithoutExtension());

    return results;
}

void Font::getPlatformDefaultFontNames (String& defaultSans, String& defaultSerif, String& defaultFixed, String& defaultFallback)
{
    defaultSans     = "sans";
    defaultSerif    = "serif";
    defaultFixed    = "monospace";
    defaultFallback = "sans";
}

//==============================================================================
class AndroidTypeface   : public Typeface
{
public:
    AndroidTypeface (const Font& font)
        : Typeface (font.getTypefaceName()),
          ascent (0),
          descent (0)
    {
        jint flags = 0;
        if (font.isBold()) flags = 1;
        if (font.isItalic()) flags += 2;

        typeface = GlobalRef (getEnv()->CallStaticObjectMethod (android.typefaceClass, android.create,
                                                                javaString (getName()).get(), flags));

        paint = GlobalRef (android.createPaint());
        const LocalRef<jobject> ignored (paint.callObjectMethod (android.setTypeface, typeface.get()));

        const float standardSize = 256.0f;
        paint.callVoidMethod (android.setTextSize, standardSize);
        ascent = std::abs (paint.callFloatMethod (android.ascent)) / standardSize;
        descent = paint.callFloatMethod (android.descent) / standardSize;

        const float height = ascent + descent;
        unitsToHeightScaleFactor = 1.0f / (height * standardSize);
    }

    float getAscent() const    { return ascent; }
    float getDescent() const   { return descent; }

    float getStringWidth (const String& text)
    {
        JNIEnv* env = getEnv();
        const int numChars = text.length();
        jfloatArray widths = env->NewFloatArray (numChars);

        const int numDone = paint.callIntMethod (android.getTextWidths, javaString (text).get(), widths);

        HeapBlock<jfloat> localWidths (numDone);
        env->GetFloatArrayRegion (widths, 0, numDone, localWidths);
        env->DeleteLocalRef (widths);

        float x = 0;
        for (int i = 0; i < numDone; ++i)
            x += localWidths[i];

        return x * unitsToHeightScaleFactor;
    }

    void getGlyphPositions (const String& text, Array<int>& glyphs, Array<float>& xOffsets)
    {
        JNIEnv* env = getEnv();
        const int numChars = text.length();
        jfloatArray widths = env->NewFloatArray (numChars);

        const int numDone = paint.callIntMethod (android.getTextWidths, javaString (text).get(), widths);

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
            xOffsets.add (x * unitsToHeightScaleFactor);
        }
    }

    bool getOutlineForGlyph (int glyphNumber, Path& destPath)
    {
        // TODO
        return false;
    }

    GlobalRef typeface, paint;
    float ascent, descent, unitsToHeightScaleFactor;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidTypeface);
};

//==============================================================================
const Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return new AndroidTypeface (font);
}


#endif
