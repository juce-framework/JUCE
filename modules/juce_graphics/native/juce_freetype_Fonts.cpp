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

struct FTLibWrapper     : public ReferenceCountedObject
{
    FTLibWrapper() : library (0)
    {
        if (FT_Init_FreeType (&library) != 0)
        {
            library = 0;
            DBG ("Failed to initialize FreeType");
        }
    }

    ~FTLibWrapper()
    {
        if (library != 0)
            FT_Done_FreeType (library);
    }

    FT_Library library;

    typedef ReferenceCountedObjectPtr<FTLibWrapper> Ptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FTLibWrapper)
};

//==============================================================================
struct FTFaceWrapper     : public ReferenceCountedObject
{
    FTFaceWrapper (const FTLibWrapper::Ptr& ftLib, const File& file, int faceIndex)
        : face (0), library (ftLib)
    {
        if (FT_New_Face (ftLib->library, file.getFullPathName().toUTF8(), faceIndex, &face) != 0)
            face = 0;
    }

    FTFaceWrapper (const FTLibWrapper::Ptr& ftLib, const void* data, size_t dataSize, int faceIndex)
        : face (0), library (ftLib), savedFaceData (data, dataSize)
    {
        if (FT_New_Memory_Face (ftLib->library, (const FT_Byte*) savedFaceData.getData(),
                                (FT_Long) savedFaceData.getSize(), faceIndex, &face) != 0)
            face = 0;
    }

    ~FTFaceWrapper()
    {
        if (face != 0)
            FT_Done_Face (face);
    }

    FT_Face face;
    FTLibWrapper::Ptr library;
    MemoryBlock savedFaceData;

    typedef ReferenceCountedObjectPtr<FTFaceWrapper> Ptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FTFaceWrapper)
};

//==============================================================================
class FTTypefaceList  : private DeletedAtShutdown
{
public:
    FTTypefaceList()  : library (new FTLibWrapper())
    {
        scanFontPaths (getDefaultFontDirectories());
    }

    ~FTTypefaceList()
    {
        clearSingletonInstance();
    }

    //==============================================================================
    struct KnownTypeface
    {
        KnownTypeface (const File& f, const int index, const FTFaceWrapper& face)
           : file (f),
             family (face.face->family_name),
             style (face.face->style_name),
             faceIndex (index),
             isMonospaced ((face.face->face_flags & FT_FACE_FLAG_FIXED_WIDTH) != 0),
             isSansSerif (isFaceSansSerif (family))
        {
        }

        const File file;
        const String family, style;
        const int faceIndex;
        const bool isMonospaced, isSansSerif;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnownTypeface)
    };

    //==============================================================================
    static FTFaceWrapper::Ptr selectUnicodeCharmap (FTFaceWrapper* face)
    {
        if (face != nullptr)
            if (FT_Select_Charmap (face->face, ft_encoding_unicode) != 0)
                FT_Set_Charmap (face->face, face->face->charmaps[0]);

        return face;
    }

    FTFaceWrapper::Ptr createFace (const void* data, size_t dataSize, int index)
    {
        return selectUnicodeCharmap (new FTFaceWrapper (library, data, dataSize, index));
    }

    FTFaceWrapper::Ptr createFace (const File& file, int index)
    {
        return selectUnicodeCharmap (new FTFaceWrapper (library, file, index));
    }

    FTFaceWrapper::Ptr createFace (const String& fontName, const String& fontStyle)
    {
        const KnownTypeface* ftFace = matchTypeface (fontName, fontStyle);

        if (ftFace == nullptr)  ftFace = matchTypeface (fontName, "Regular");
        if (ftFace == nullptr)  ftFace = matchTypeface (fontName, String());

        if (ftFace != nullptr)
            return createFace (ftFace->file, ftFace->faceIndex);

        return nullptr;
    }

    //==============================================================================
    StringArray findAllFamilyNames() const
    {
        StringArray s;

        for (int i = 0; i < faces.size(); ++i)
            s.addIfNotAlreadyThere (faces.getUnchecked(i)->family);

        return s;
    }

    static int indexOfRegularStyle (const StringArray& styles)
    {
        int i = styles.indexOf ("Regular", true);

        if (i < 0)
            for (i = 0; i < styles.size(); ++i)
                if (! (styles[i].containsIgnoreCase ("Bold") || styles[i].containsIgnoreCase ("Italic")))
                    break;

        return i;
    }

    StringArray findAllTypefaceStyles (const String& family) const
    {
        StringArray s;

        for (int i = 0; i < faces.size(); ++i)
        {
            const KnownTypeface* const face = faces.getUnchecked(i);

            if (face->family == family)
                s.addIfNotAlreadyThere (face->style);
        }

        // try to get a regular style to be first in the list
        const int regular = indexOfRegularStyle (s);
        if (regular > 0)
            s.strings.swap (0, regular);

        return s;
    }

    void scanFontPaths (const StringArray& paths)
    {
        for (int i = 0; i < paths.size(); ++i)
        {
            DirectoryIterator iter (File::getCurrentWorkingDirectory()
                                       .getChildFile (paths[i]), true);

            while (iter.next())
                if (iter.getFile().hasFileExtension ("ttf;pfb;pcf;otf"))
                    scanFont (iter.getFile());
        }
    }

    void getMonospacedNames (StringArray& monoSpaced) const
    {
        for (int i = 0; i < faces.size(); ++i)
            if (faces.getUnchecked(i)->isMonospaced)
                monoSpaced.addIfNotAlreadyThere (faces.getUnchecked(i)->family);
    }

    void getSerifNames (StringArray& serif) const
    {
        for (int i = 0; i < faces.size(); ++i)
            if (! faces.getUnchecked(i)->isSansSerif)
                serif.addIfNotAlreadyThere (faces.getUnchecked(i)->family);
    }

    void getSansSerifNames (StringArray& sansSerif) const
    {
        for (int i = 0; i < faces.size(); ++i)
            if (faces.getUnchecked(i)->isSansSerif)
                sansSerif.addIfNotAlreadyThere (faces.getUnchecked(i)->family);
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (FTTypefaceList)

private:
    FTLibWrapper::Ptr library;
    OwnedArray<KnownTypeface> faces;

    static StringArray getDefaultFontDirectories();

    void scanFont (const File& file)
    {
        int faceIndex = 0;
        int numFaces = 0;

        do
        {
            FTFaceWrapper face (library, file, faceIndex);

            if (face.face != 0)
            {
                if (faceIndex == 0)
                    numFaces = (int) face.face->num_faces;

                if ((face.face->face_flags & FT_FACE_FLAG_SCALABLE) != 0)
                    faces.add (new KnownTypeface (file, faceIndex, face));
            }

            ++faceIndex;
        }
        while (faceIndex < numFaces);
    }

    const KnownTypeface* matchTypeface (const String& familyName, const String& style) const noexcept
    {
        for (int i = 0; i < faces.size(); ++i)
        {
            const KnownTypeface* const face = faces.getUnchecked(i);

            if (face->family == familyName
                  && (face->style.equalsIgnoreCase (style) || style.isEmpty()))
                return face;
        }

        return nullptr;
    }

    static bool isFaceSansSerif (const String& family)
    {
        static const char* sansNames[] = { "Sans", "Verdana", "Arial", "Ubuntu" };

        for (int i = 0; i < numElementsInArray (sansNames); ++i)
            if (family.containsIgnoreCase (sansNames[i]))
                return true;

        return false;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FTTypefaceList)
};

juce_ImplementSingleton_SingleThreaded (FTTypefaceList)


//==============================================================================
class FreeTypeTypeface   : public CustomTypeface
{
public:
    FreeTypeTypeface (const Font& font)
        : faceWrapper (FTTypefaceList::getInstance()->createFace (font.getTypefaceName(),
                                                                  font.getTypefaceStyle()))
    {
        if (faceWrapper != nullptr)
            initialiseCharacteristics (font.getTypefaceName(),
                                       font.getTypefaceStyle());
    }

    FreeTypeTypeface (const void* data, size_t dataSize)
        : faceWrapper (FTTypefaceList::getInstance()->createFace (data, dataSize, 0))
    {
        if (faceWrapper != nullptr)
            initialiseCharacteristics (faceWrapper->face->family_name,
                                       faceWrapper->face->style_name);
    }

    void initialiseCharacteristics (const String& fontName, const String& fontStyle)
    {
        setCharacteristics (fontName, fontStyle,
                            faceWrapper->face->ascender / (float) (faceWrapper->face->ascender - faceWrapper->face->descender),
                            L' ');
    }

    bool loadGlyphIfPossible (const juce_wchar character)
    {
        if (faceWrapper != nullptr)
        {
            FT_Face face = faceWrapper->face;
            const unsigned int glyphIndex = FT_Get_Char_Index (face, (FT_ULong) character);

            if (FT_Load_Glyph (face, glyphIndex, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP | FT_LOAD_IGNORE_TRANSFORM | FT_LOAD_NO_HINTING) == 0
                  && face->glyph->format == ft_glyph_format_outline)
            {
                const float scale = 1.0f / (float) (face->ascender - face->descender);
                Path destShape;

                if (getGlyphShape (destShape, face->glyph->outline, scale))
                {
                    addGlyph (character, destShape, face->glyph->metrics.horiAdvance * scale);

                    if ((face->face_flags & FT_FACE_FLAG_KERNING) != 0)
                        addKerning (face, (uint32) character, glyphIndex);

                    return true;
                }
            }
        }

        return false;
    }

private:
    FTFaceWrapper::Ptr faceWrapper;

    bool getGlyphShape (Path& destShape, const FT_Outline& outline, const float scaleX)
    {
        const float scaleY = -scaleX;
        const short* const contours = outline.contours;
        const char* const tags = outline.tags;
        const FT_Vector* const points = outline.points;

        for (int c = 0; c < outline.n_contours; ++c)
        {
            const int startPoint = (c == 0) ? 0 : contours [c - 1] + 1;
            const int endPoint = contours[c];

            for (int p = startPoint; p <= endPoint; ++p)
            {
                const float x = scaleX * points[p].x;
                const float y = scaleY * points[p].y;

                if (p == startPoint)
                {
                    if (FT_CURVE_TAG (tags[p]) == FT_Curve_Tag_Conic)
                    {
                        float x2 = scaleX * points [endPoint].x;
                        float y2 = scaleY * points [endPoint].y;

                        if (FT_CURVE_TAG (tags[endPoint]) != FT_Curve_Tag_On)
                        {
                            x2 = (x + x2) * 0.5f;
                            y2 = (y + y2) * 0.5f;
                        }

                        destShape.startNewSubPath (x2, y2);
                    }
                    else
                    {
                        destShape.startNewSubPath (x, y);
                    }
                }

                if (FT_CURVE_TAG (tags[p]) == FT_Curve_Tag_On)
                {
                    if (p != startPoint)
                        destShape.lineTo (x, y);
                }
                else if (FT_CURVE_TAG (tags[p]) == FT_Curve_Tag_Conic)
                {
                    const int nextIndex = (p == endPoint) ? startPoint : p + 1;
                    float x2 = scaleX * points [nextIndex].x;
                    float y2 = scaleY * points [nextIndex].y;

                    if (FT_CURVE_TAG (tags [nextIndex]) == FT_Curve_Tag_Conic)
                    {
                        x2 = (x + x2) * 0.5f;
                        y2 = (y + y2) * 0.5f;
                    }
                    else
                    {
                        ++p;
                    }

                    destShape.quadraticTo (x, y, x2, y2);
                }
                else if (FT_CURVE_TAG (tags[p]) == FT_Curve_Tag_Cubic)
                {
                    const int next1 = p + 1;
                    const int next2 = (p == (endPoint - 1)) ? startPoint : (p + 2);

                    if (p >= endPoint
                         || FT_CURVE_TAG (tags[next1]) != FT_Curve_Tag_Cubic
                         || FT_CURVE_TAG (tags[next2]) != FT_Curve_Tag_On)
                        return false;

                    const float x2 = scaleX * points [next1].x;
                    const float y2 = scaleY * points [next1].y;
                    const float x3 = scaleX * points [next2].x;
                    const float y3 = scaleY * points [next2].y;

                    destShape.cubicTo (x, y, x2, y2, x3, y3);
                    p += 2;
                }
            }

            destShape.closeSubPath();
        }

        return true;
    }

    void addKerning (FT_Face face, const uint32 character, const uint32 glyphIndex)
    {
        const float height = (float) (face->ascender - face->descender);

        uint32 rightGlyphIndex;
        FT_ULong rightCharCode = FT_Get_First_Char (face, &rightGlyphIndex);

        while (rightGlyphIndex != 0)
        {
            FT_Vector kerning;

            if (FT_Get_Kerning (face, glyphIndex, rightGlyphIndex, ft_kerning_unscaled, &kerning) == 0
                   && kerning.x != 0)
                addKerningPair ((juce_wchar) character, (juce_wchar) rightCharCode, kerning.x / height);

            rightCharCode = FT_Get_Next_Char (face, rightCharCode, &rightGlyphIndex);
        }
    }

    JUCE_DECLARE_NON_COPYABLE (FreeTypeTypeface)
};

} // namespace juce
