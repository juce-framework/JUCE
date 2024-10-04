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

#include FT_TRUETYPE_TABLES_H
#include FT_GLYPH_H
#include FT_COLOR_H

namespace juce
{

#if JUCE_USE_FONTCONFIG
using FcConfigPtr  = std::unique_ptr<FcConfig,  FunctionPointerDestructor<FcConfigDestroy>>;
using FcPatternPtr = std::unique_ptr<FcPattern, FunctionPointerDestructor<FcPatternDestroy>>;
using FcCharSetPtr = std::unique_ptr<FcCharSet, FunctionPointerDestructor<FcCharSetDestroy>>;
using FcLangSetPtr = std::unique_ptr<FcLangSet, FunctionPointerDestructor<FcLangSetDestroy>>;
#endif

struct FTLibWrapper final : public ReferenceCountedObject
{
    FTLibWrapper()
    {
        if (FT_Init_FreeType (&library) != 0)
        {
            library = {};
            DBG ("Failed to initialize FreeType");
        }
    }

    ~FTLibWrapper()
    {
        if (library != nullptr)
            FT_Done_FreeType (library);
    }

   #if JUCE_USE_FONTCONFIG
    const FcConfigPtr fcConfig { FcInitLoadConfigAndFonts() };
   #endif

    FT_Library library = {};

    using Ptr = ReferenceCountedObjectPtr<FTLibWrapper>;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FTLibWrapper)
};

//==============================================================================
struct FTFaceWrapper final : public ReferenceCountedObject
{
    using Ptr = ReferenceCountedObjectPtr<FTFaceWrapper>;

    static FTFaceWrapper::Ptr selectUnicodeCharmap (FTFaceWrapper::Ptr face)
    {
        if (face != nullptr)
            if (FT_Select_Charmap (face->face, ft_encoding_unicode) != 0)
                FT_Set_Charmap (face->face, face->face->charmaps[0]);

        return face;
    }

    static FTFaceWrapper::Ptr from (const FTLibWrapper::Ptr& ftLib, const File& file, int faceIndex)
    {
        FT_Face result{};

        if (FT_New_Face (ftLib->library, file.getFullPathName().toUTF8(), faceIndex, &result) != 0)
            return {};

        return selectUnicodeCharmap (new FTFaceWrapper (ftLib, result));
    }

    static FTFaceWrapper::Ptr from (const FTLibWrapper::Ptr& ftLib, const void* data, size_t dataSize, int faceIndex)
    {
        MemoryBlock storage (data, dataSize);
        FT_Face result{};
        if (FT_New_Memory_Face (ftLib->library,
                                static_cast<const FT_Byte*> (storage.getData()),
                                (FT_Long) storage.getSize(),
                                faceIndex,
                                &result) != 0)
            return {};

        return selectUnicodeCharmap (new FTFaceWrapper (ftLib, result, std::move (storage)));
    }

    FTFaceWrapper (const FTLibWrapper::Ptr& ftLib, FT_Face faceIn, MemoryBlock mb = {})
        : library (ftLib), savedFaceData (std::move (mb)), face (faceIn) {}

    ~FTFaceWrapper()
    {
        if (face != nullptr)
            FT_Done_Face (face);
    }

    FTLibWrapper::Ptr library;
    MemoryBlock savedFaceData;
    FT_Face face = {};

    JUCE_LEAK_DETECTOR (FTFaceWrapper)
};

//==============================================================================
class FTTypefaceList final : private DeletedAtShutdown
{
public:
    FTTypefaceList()
    {
        scanFontPaths (getDefaultFontDirectories());
    }

    ~FTTypefaceList() override
    {
        clearSingletonInstance();
    }

    //==============================================================================
    struct KnownTypeface
    {
        explicit KnownTypeface (const FTFaceWrapper& face)
           : family (face.face->family_name),
             style (face.face->style_name),
             faceIndex ((int) face.face->face_index),
             flags (((face.face->style_flags & FT_STYLE_FLAG_BOLD) ? bold : 0)
                  | ((face.face->style_flags & FT_STYLE_FLAG_ITALIC) ? italic : 0)
                  | ((face.face->face_flags & FT_FACE_FLAG_FIXED_WIDTH) ? monospaced : 0)
                  | (isFaceSansSerif (family) ? sansSerif : 0))
        {
        }

        virtual ~KnownTypeface() = default;
        virtual FTFaceWrapper::Ptr create (FTLibWrapper::Ptr) const = 0;
        virtual bool holdsFace (FTFaceWrapper::Ptr) const { return false; }

        enum Flag
        {
            bold = 1 << 0,
            italic = 1 << 1,
            monospaced = 1 << 2,
            sansSerif = 1 << 3,
        };

        const String family, style;
        const int faceIndex;
        const int flags;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnownTypeface)
    };

    struct FileTypeface : public KnownTypeface
    {
        FileTypeface (const FTFaceWrapper& face, const File& fileIn)
            : KnownTypeface (face), file (fileIn) {}

        FTFaceWrapper::Ptr create (FTLibWrapper::Ptr lib) const override
        {
            return FTFaceWrapper::from (lib, file, faceIndex);
        }

        const File file;
    };

    struct CachedTypeface : public KnownTypeface
    {
        explicit CachedTypeface (FTFaceWrapper::Ptr ptr)
            : KnownTypeface (*ptr), face (ptr) {}

        FTFaceWrapper::Ptr create (FTLibWrapper::Ptr) const override
        {
            return face;
        }

        bool holdsFace (FTFaceWrapper::Ptr p) const override
        {
            return face == p;
        }

        FTFaceWrapper::Ptr face;
    };

    //==============================================================================
    FTFaceWrapper::Ptr createFace (const void* data, size_t dataSize, int index)
    {
        return FTFaceWrapper::from (library, data, dataSize, index);
    }

    FTFaceWrapper::Ptr createFace (const File& file, int index)
    {
        return FTFaceWrapper::from (library, file, index);
    }

    FTFaceWrapper::Ptr createFace (const String& fontName, const String& fontStyle)
    {
        auto ftFace = matchTypeface (fontName, fontStyle);

        if (ftFace == nullptr)  ftFace = matchTypeface (fontName, "Regular");
        if (ftFace == nullptr)  ftFace = matchTypeface (fontName, {});

        if (ftFace != nullptr)
            return ftFace->create (library);

        return nullptr;
    }

    //==============================================================================
    StringArray findAllFamilyNames() const
    {
        std::set<String> set;

        for (const auto& face : faces)
            set.insert (face->family);

        StringArray s;

        for (const auto& family : set)
            s.add (family);

        return s;
    }

    StringArray findAllTypefaceStyles (const String& family) const
    {
        StringArray s;

        for (const auto& face : faces)
            if (face->family == family)
                s.addIfNotAlreadyThere (face->style);

        // scanFontPaths ensures that regular styles are ordered before other styles
        return s;
    }

    void scanFontPaths (const StringArray& paths)
    {
        for (auto& path : paths)
        {
            for (const auto& iter : RangedDirectoryIterator (File::getCurrentWorkingDirectory().getChildFile (path), true))
            {
                if (iter.getFile().hasFileExtension ("ttf;pfb;pcf;otf"))
                    scanFont (iter.getFile());
            }
        }

        std::sort (faces.begin(), faces.end(), [] (const auto& a, const auto& b)
        {
            const auto tie = [] (const KnownTypeface& t)
            {
                return std::make_tuple (t.family,
                                        t.flags,
                                        t.style,
                                        t.faceIndex);
            };

            return tie (*a) < tie (*b);
        });
    }

    void getMonospacedNames (StringArray& monoSpaced) const
    {
        for (const auto& face : faces)
            if (face->flags & KnownTypeface::monospaced)
                monoSpaced.addIfNotAlreadyThere (face->family);
    }

    void getSerifNames (StringArray& serif) const
    {
        for (const auto& face : faces)
            if ((face->flags & (KnownTypeface::sansSerif | KnownTypeface::monospaced)) == 0)
                serif.addIfNotAlreadyThere (face->family);
    }

    void getSansSerifNames (StringArray& sansSerif) const
    {
        for (const auto& face : faces)
            if (face->flags & KnownTypeface::sansSerif)
                sansSerif.addIfNotAlreadyThere (face->family);
    }

    void addMemoryFace (FTFaceWrapper::Ptr ptr)
    {
        faces.insert (faces.begin(), std::make_unique<CachedTypeface> (ptr));
    }

    void removeMemoryFace (FTFaceWrapper::Ptr ptr)
    {
        const auto iter = std::find_if (faces.begin(), faces.end(), [&] (const auto& face)
        {
            return face->holdsFace (ptr);
        });

        if (iter != faces.end())
            faces.erase (iter);
    }

    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL (FTTypefaceList)

    FTLibWrapper::Ptr getLibrary() const { return library; }

private:
    FTLibWrapper::Ptr library = new FTLibWrapper;
    std::vector<std::unique_ptr<KnownTypeface>> faces;

    static StringArray getDefaultFontDirectories();

    void scanFont (const File& file)
    {
        int faceIndex = 0;
        int numFaces = 0;

        do
        {
            if (auto face = FTFaceWrapper::from (library, file, faceIndex))
            {
                if (face->face != nullptr)
                {
                    if (faceIndex == 0)
                        numFaces = (int) face->face->num_faces;

                    faces.push_back (std::make_unique<FileTypeface> (*face, file));
                }
            }

            ++faceIndex;
        }
        while (faceIndex < numFaces);
    }

    const KnownTypeface* matchTypeface (const String& familyName, const String& style) const noexcept
    {
        for (const auto& face : faces)
            if (face->family == familyName
                  && (face->style.equalsIgnoreCase (style) || style.isEmpty()))
                return face.get();

        return nullptr;
    }

    static bool isFaceSansSerif (const String& family)
    {
        static const char* sansNames[] = { "Sans", "Verdana", "Arial", "Ubuntu" };

        for (auto* name : sansNames)
            if (family.containsIgnoreCase (name))
                return true;

        return false;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FTTypefaceList)
};

JUCE_IMPLEMENT_SINGLETON (FTTypefaceList)

//==============================================================================
class FreeTypeTypeface final : public Typeface
{
    using Ptr = ReferenceCountedObjectPtr<FreeTypeTypeface>;
    enum class DoCache
    {
        no,
        yes,
    };

public:
    static Typeface::Ptr from (const Font& font)
    {
        const auto name = font.getTypefaceName();
        const auto style = font.getTypefaceStyle();

        auto face = FTTypefaceList::getInstance()->createFace (name, style);

        if (face == nullptr)
            return {};

        auto* hbFace = hb_ft_face_create_referenced (face->face);
        const ScopeGuard scope { [&] { hb_face_destroy (hbFace); } };

        HbFont hb { hb_font_create (hbFace) };

        if (hb == nullptr)
            return {};

        FontStyleHelpers::initSynthetics (hb.get(), font);
        return new FreeTypeTypeface (DoCache::no, face, std::move (hb), name, style);
    }

    static Typeface::Ptr from (Span<const std::byte> data, int index = 0)
    {
        auto face = FTTypefaceList::getInstance()->createFace (data.data(), data.size(), index);

        if (face == nullptr)
            return {};

        auto* hbFace = hb_ft_face_create_referenced (face->face);
        const ScopeGuard scope { [&] { hb_face_destroy (hbFace); } };

        HbFont hb { hb_font_create (hbFace) };

        if (hb == nullptr)
            return {};

        return new FreeTypeTypeface (DoCache::yes, face, std::move (hb), face->face->family_name, face->face->style_name);
    }

    Native getNativeDetails() const override
    {
        return Native { hb.get(), nonPortableMetrics };
    }

    Typeface::Ptr createSystemFallback ([[maybe_unused]] const String& text,
                                        [[maybe_unused]] const String& language) const override
    {
       #if JUCE_USE_FONTCONFIG
        auto* cache = TypefaceFileCache::getInstance();

        if (cache == nullptr)
            return {};

        FcPatternPtr pattern { FcPatternCreate() };

        {
            FcValue value{};
            value.type = FcTypeString;
            value.u.s = unalignedPointerCast<const FcChar8*> (ftFace->face->family_name);
            FcPatternAddWeak (pattern.get(), FC_FAMILY, value, FcFalse);
        }

        {
            FcValue value{};
            value.type = FcTypeString;
            value.u.s = unalignedPointerCast<const FcChar8*> (ftFace->face->style_name);
            FcPatternAddWeak (pattern.get(), FC_STYLE, value, FcFalse);
        }

        {
            const FcCharSetPtr charset { FcCharSetCreate() };
            for (const auto& character : text)
                FcCharSetAddChar (charset.get(), (FcChar32) character);
            FcPatternAddCharSet (pattern.get(), FC_CHARSET, charset.get());
        }

        if (language.isNotEmpty())
        {
            const FcLangSetPtr langset { FcLangSetCreate() };
            FcLangSetAdd (langset.get(), unalignedPointerCast<const FcChar8*> (language.toRawUTF8()));
            FcPatternAddLangSet (pattern.get(), FC_LANG, langset.get());
        }

        return fromPattern (pattern.get());
       #else
        // Font substitution will not work unless fontconfig is enabled.
        jassertfalse;
        return nullptr;
       #endif
    }

    ~FreeTypeTypeface() override
    {
        if (doCache == DoCache::yes)
            if (auto* list = FTTypefaceList::getInstanceWithoutCreating())
                list->removeMemoryFace (ftFace);
    }

    static Typeface::Ptr findSystemTypeface()
    {
       #if JUCE_USE_FONTCONFIG
        FcPatternPtr pattern { FcNameParse (unalignedPointerCast<const FcChar8*> ("system-ui")) };
        return fromPattern (pattern.get());
       #else
        return nullptr;
       #endif
    }

private:
   #if JUCE_USE_FONTCONFIG
    static Typeface::Ptr fromPattern (FcPattern* pattern)
    {
        auto* cache = TypefaceFileCache::getInstance();

        if (cache == nullptr)
            return {};

        const auto library = FTTypefaceList::getInstance()->getLibrary();

        FcConfigSubstitute (library->fcConfig.get(), pattern, FcMatchPattern);
        FcDefaultSubstitute (pattern);

        FcResult result{};
        const FcPatternPtr matched { FcFontMatch (library->fcConfig.get(), pattern, &result) };

        if (result != FcResultMatch)
            return {};

        FcChar8* fileString{};
        if (FcPatternGetString (matched.get(), FC_FILE, 0, &fileString) != FcResultMatch)
            return {};

        int index{};
        if (FcPatternGetInteger (matched.get(), FC_INDEX, 0, &index) != FcResultMatch)
            return {};

        const File file { String { CharPointer_UTF8 { unalignedPointerCast<const char*> (fileString) } } };

        return cache->get ({ file, index }, [] (const TypefaceFileAndIndex& f) -> Typeface::Ptr
        {
            auto face = FTTypefaceList::getInstance()->createFace (f.file, f.index);

            if (face == nullptr)
                return {};

            const HbFace hbFace { hb_ft_face_create_referenced (face->face) };
            HbFont cachedFont { hb_font_create (hbFace.get()) };

            if (cachedFont == nullptr)
                return {};

            return new FreeTypeTypeface (DoCache::no, face, std::move (cachedFont), face->face->family_name, face->face->style_name);
        });
    }
   #endif

    FreeTypeTypeface (DoCache cache,
                      FTFaceWrapper::Ptr ftFaceIn,
                      HbFont hbIn,
                      const String& nameIn,
                      const String& styleIn)
        : Typeface (nameIn, styleIn),
          ftFace (ftFaceIn),
          hb (std::move (hbIn)),
          doCache (cache)
    {
        if (doCache == DoCache::yes)
            if (auto* list = FTTypefaceList::getInstance())
                list->addMemoryFace (ftFace);
    }

    FTFaceWrapper::Ptr ftFace;
    HbFont hb;
    DoCache doCache;
    TypefaceAscentDescent nonPortableMetrics { (float) std::abs (ftFace->face->ascender)  / (float) ftFace->face->units_per_EM,
                                               (float) std::abs (ftFace->face->descender) / (float) ftFace->face->units_per_EM };

    JUCE_DECLARE_NON_COPYABLE (FreeTypeTypeface)
};

Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return FreeTypeTypeface::from (font);
}

Typeface::Ptr Typeface::createSystemTypefaceFor (Span<const std::byte> data)
{
    return FreeTypeTypeface::from (data);
}

Typeface::Ptr Typeface::findSystemTypeface()
{
    return FreeTypeTypeface::findSystemTypeface();
}

} // namespace juce
