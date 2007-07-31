/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_Typeface.h"
#include "juce_Font.h"
#include "../../../../juce_core/io/streams/juce_GZIPDecompressorInputStream.h"
#include "../../../../juce_core/io/streams/juce_GZIPCompressorOutputStream.h"
#include "../../../../juce_core/io/streams/juce_BufferedInputStream.h"
#include "../../../application/juce_DeletedAtShutdown.h"


//==============================================================================
TypefaceGlyphInfo::TypefaceGlyphInfo (const juce_wchar character_,
                                      const Path& shape,
                                      const float horizontalSeparation,
                                      Typeface* const typeface_) throw()
    : character (character_),
      path (shape),
      width (horizontalSeparation),
      typeface (typeface_)
{
}

TypefaceGlyphInfo::~TypefaceGlyphInfo() throw()
{
}

float TypefaceGlyphInfo::getHorizontalSpacing (const juce_wchar subsequentCharacter) const throw()
{
    if (subsequentCharacter != 0)
    {
        const KerningPair* const pairs = (const KerningPair*) kerningPairs.getData();
        const int numPairs = getNumKerningPairs();

        for (int i = 0; i < numPairs; ++i)
            if (pairs [i].character2 == subsequentCharacter)
                return width + pairs [i].kerningAmount;
    }

    return width;
}

void TypefaceGlyphInfo::addKerningPair (const juce_wchar subsequentCharacter,
                                        const float extraKerningAmount) throw()
{
    const int numPairs = getNumKerningPairs();
    kerningPairs.setSize ((numPairs + 1) * sizeof (KerningPair));

    KerningPair& p = getKerningPair (numPairs);
    p.character2 = subsequentCharacter;
    p.kerningAmount = extraKerningAmount;
}

TypefaceGlyphInfo::KerningPair& TypefaceGlyphInfo::getKerningPair (const int index) const throw()
{
    return ((KerningPair*) kerningPairs.getData()) [index];
}

int TypefaceGlyphInfo::getNumKerningPairs() const throw()
{
    return kerningPairs.getSize() / sizeof (KerningPair);
}


//==============================================================================
Typeface::Typeface() throw()
    : hash (0),
      isFullyPopulated (false)
{
    zeromem (lookupTable, sizeof (lookupTable));
}

Typeface::Typeface (const Typeface& other)
  : typefaceName (other.typefaceName),
    ascent (other.ascent),
    bold (other.bold),
    italic (other.italic),
    isFullyPopulated (other.isFullyPopulated),
    defaultCharacter (other.defaultCharacter)
{
    zeromem (lookupTable, sizeof (lookupTable));

    for (int i = 0; i < other.glyphs.size(); ++i)
        addGlyphCopy ((const TypefaceGlyphInfo*) other.glyphs.getUnchecked(i));

    updateHashCode();
}

Typeface::Typeface (const String& faceName,
                    const bool bold,
                    const bool italic)
  : isFullyPopulated (false)
{
    zeromem (lookupTable, sizeof (lookupTable));

    initialiseTypefaceCharacteristics (faceName, bold, italic, false);

    updateHashCode();
}

Typeface::~Typeface()
{
    clear();
}

const Typeface& Typeface::operator= (const Typeface& other) throw()
{
    if (this != &other)
    {
        clear();

        typefaceName = other.typefaceName;
        ascent = other.ascent;
        bold = other.bold;
        italic = other.italic;
        isFullyPopulated = other.isFullyPopulated;
        defaultCharacter = other.defaultCharacter;

        for (int i = 0; i < other.glyphs.size(); ++i)
            addGlyphCopy ((const TypefaceGlyphInfo*) other.glyphs.getUnchecked(i));

        updateHashCode();
    }

    return *this;
}

void Typeface::updateHashCode() throw()
{
    hash = typefaceName.hashCode();

    if (bold)
        hash ^= 0xffff;

    if (italic)
        hash ^= 0xffff0000;
}

void Typeface::clear() throw()
{
    zeromem (lookupTable, sizeof (lookupTable));
    typefaceName = String::empty;
    bold = false;
    italic = false;

    for (int i = glyphs.size(); --i >= 0;)
    {
        TypefaceGlyphInfo* const g = (TypefaceGlyphInfo*) (glyphs.getUnchecked(i));
        delete g;
    }

    glyphs.clear();
    updateHashCode();
}


Typeface::Typeface (InputStream& serialisedTypefaceStream)
{
    zeromem (lookupTable, sizeof (lookupTable));
    isFullyPopulated = true;

    GZIPDecompressorInputStream gzin (&serialisedTypefaceStream, false);
    BufferedInputStream in (&gzin, 32768, false);

    typefaceName = in.readString();
    bold = in.readBool();
    italic = in.readBool();
    ascent = in.readFloat();
    defaultCharacter = (juce_wchar) in.readShort();

    int i, numChars = in.readInt();

    for (i = 0; i < numChars; ++i)
    {
        const juce_wchar c = (juce_wchar) in.readShort();
        const float width = in.readFloat();

        Path p;
        p.loadPathFromStream (in);
        addGlyph (c, p, width);
    }

    const int numKerningPairs = in.readInt();

    for (i = 0; i < numKerningPairs; ++i)
    {
        const juce_wchar char1 = (juce_wchar) in.readShort();
        const juce_wchar char2 = (juce_wchar) in.readShort();

        addKerningPair (char1, char2, in.readFloat());
    }

    updateHashCode();
}

void Typeface::serialise (OutputStream& outputStream)
{
    GZIPCompressorOutputStream out (&outputStream);

    out.writeString (typefaceName);
    out.writeBool (bold);
    out.writeBool (italic);
    out.writeFloat (ascent);
    out.writeShort ((short) (unsigned short) defaultCharacter);
    out.writeInt (glyphs.size());

    int i, numKerningPairs = 0;

    for (i = 0; i < glyphs.size(); ++i)
    {
        const TypefaceGlyphInfo& g = *(const TypefaceGlyphInfo*)(glyphs.getUnchecked (i));
        out.writeShort ((short) (unsigned short) g.character);
        out.writeFloat (g.width);
        g.path.writePathToStream (out);

        numKerningPairs += g.getNumKerningPairs();
    }

    out.writeInt (numKerningPairs);

    for (i = 0; i < glyphs.size(); ++i)
    {
        const TypefaceGlyphInfo& g = *(const TypefaceGlyphInfo*)(glyphs.getUnchecked (i));

        for (int j = 0; j < g.getNumKerningPairs(); ++j)
        {
            const TypefaceGlyphInfo::KerningPair& p = g.getKerningPair (j);
            out.writeShort ((short) (unsigned short) g.character);
            out.writeShort ((short) (unsigned short) p.character2);
            out.writeFloat (p.kerningAmount);
        }
    }
}

const Path* Typeface::getOutlineForGlyph (const juce_wchar character) throw()
{
    const TypefaceGlyphInfo* const g = (const TypefaceGlyphInfo*) getGlyph (character);

    if (g != 0)
        return &(g->path);
    else
        return 0;
}

const TypefaceGlyphInfo* Typeface::getGlyph (const juce_wchar character) throw()
{
    if (character > 0 && character < 128 && lookupTable [character] > 0)
        return (const TypefaceGlyphInfo*) glyphs [(int)lookupTable [character]];

    const TypefaceGlyphInfo* glyph = 0;

    for (int i = 0; i < glyphs.size(); ++i)
    {
        const TypefaceGlyphInfo* const g = (TypefaceGlyphInfo*) glyphs.getUnchecked(i);

        if (g->character == character)
        {
            glyph = g;
            break;
        }
    }

    if (glyph == 0)
    {
        if (! isFullyPopulated)
        {
            findAndAddSystemGlyph (character);

            for (int i = 0; i < glyphs.size(); ++i)
            {
                const TypefaceGlyphInfo* const g = (TypefaceGlyphInfo*) glyphs.getUnchecked(i);

                if (g->character == character)
                {
                    glyph = g;
                    break;
                }
            }
        }

        if (glyph == 0)
        {
            if (CharacterFunctions::isWhitespace (character) && character != L' ')
                glyph = getGlyph (L' ');
            else if (character != defaultCharacter)
                glyph = getGlyph (defaultCharacter);
        }
    }

    return glyph;
}

void Typeface::addGlyph (const juce_wchar character,
                         const Path& path,
                         const float horizontalSpacing) throw()
{
    if (character > 0 && character < 128)
        lookupTable [character] = (short) glyphs.size();

    glyphs.add (new TypefaceGlyphInfo (character,
                                       path,
                                       horizontalSpacing,
                                       this));
}

void Typeface::addGlyphCopy (const TypefaceGlyphInfo* const glyphInfoToCopy) throw()
{
    if (glyphInfoToCopy != 0)
    {
        if (glyphInfoToCopy->character > 0 && glyphInfoToCopy->character < 128)
            lookupTable [glyphInfoToCopy->character] = (short) glyphs.size();

        TypefaceGlyphInfo* const newOne
            = new TypefaceGlyphInfo (glyphInfoToCopy->character,
                                     glyphInfoToCopy->path,
                                     glyphInfoToCopy->width,
                                     this);

        newOne->kerningPairs = glyphInfoToCopy->kerningPairs;
        glyphs.add (newOne);
    }
}

void Typeface::addKerningPair (const juce_wchar char1,
                               const juce_wchar char2,
                               const float extraAmount) throw()
{
    TypefaceGlyphInfo* const g = (TypefaceGlyphInfo*) getGlyph (char1);

    if (g != 0)
        g->addKerningPair (char2, extraAmount);
}

void Typeface::setName (const String& name) throw()
{
    typefaceName = name;
    updateHashCode();
}

void Typeface::setAscent (const float newAscent) throw()
{
    ascent = newAscent;
}

void Typeface::setDefaultCharacter (const juce_wchar newDefaultCharacter) throw()
{
    defaultCharacter = newDefaultCharacter;
}

void Typeface::setBold (const bool shouldBeBold) throw()
{
    bold = shouldBeBold;
    updateHashCode();
}

void Typeface::setItalic (const bool shouldBeItalic) throw()
{
    italic = shouldBeItalic;
    updateHashCode();
}

//==============================================================================
class TypefaceCache;
static TypefaceCache* typefaceCacheInstance = 0;

void clearUpDefaultFontNames() throw(); // in juce_Font.cpp


//==============================================================================
class TypefaceCache  : private DeletedAtShutdown
{
private:
    //==============================================================================
    struct CachedFace
    {
        CachedFace() throw()
            : lastUsageCount (0),
              flags (0)
        {
        }

        String typefaceName;
        int lastUsageCount;
        int flags;
        Typeface::Ptr typeFace;
    };

    int counter;
    OwnedArray <CachedFace> faces;

    TypefaceCache (const TypefaceCache&);
    const TypefaceCache& operator= (const TypefaceCache&);

public:
    //==============================================================================
    TypefaceCache (int numToCache = 10)
        : counter (1),
          faces (2)
    {
        while (--numToCache >= 0)
        {
            CachedFace* const face = new CachedFace();
            face->typeFace = new Typeface();
            faces.add (face);
        }
    }

    ~TypefaceCache()
    {
        faces.clear();
        jassert (typefaceCacheInstance == this);
        typefaceCacheInstance = 0;

        // just a courtesy call to get avoid leaking these strings at shutdown
        clearUpDefaultFontNames();
    }

    //==============================================================================
    static TypefaceCache* getInstance() throw()
    {
        if (typefaceCacheInstance == 0)
            typefaceCacheInstance = new TypefaceCache();

        return typefaceCacheInstance;
    }

    //==============================================================================
    const Typeface::Ptr findTypefaceFor (const Font& font) throw()
    {
        const int flags = font.getStyleFlags() & (Font::bold | Font::italic);

        int i;
        for (i = faces.size(); --i >= 0;)
        {
            CachedFace* const face = faces.getUnchecked(i);

            if (face->flags == flags
                 && face->typefaceName == font.getTypefaceName())
            {
                face->lastUsageCount = ++counter;
                return face->typeFace;
            }
        }

        int replaceIndex = 0;
        int bestLastUsageCount = INT_MAX;

        for (i = faces.size(); --i >= 0;)
        {
            const int lu = faces.getUnchecked(i)->lastUsageCount;

            if (bestLastUsageCount > lu)
            {
                bestLastUsageCount = lu;
                replaceIndex = i;
            }
        }

        CachedFace* const face = faces.getUnchecked (replaceIndex);

        face->typefaceName = font.getTypefaceName();
        face->flags = flags;
        face->lastUsageCount = ++counter;
        face->typeFace = new Typeface (font.getTypefaceName(),
                                       font.isBold(),
                                       font.isItalic());

        return face->typeFace;
    }
};

const Typeface::Ptr Typeface::getTypefaceFor (const Font& font) throw()
{
    return TypefaceCache::getInstance()->findTypefaceFor (font);
}

END_JUCE_NAMESPACE
