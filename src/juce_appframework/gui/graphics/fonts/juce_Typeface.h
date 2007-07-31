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

#ifndef __JUCE_TYPEFACE_JUCEHEADER__
#define __JUCE_TYPEFACE_JUCEHEADER__

#include "../../../../juce_core/containers/juce_ReferenceCountedObject.h"
#include "../../../../juce_core/containers/juce_VoidArray.h"
#include "../../../../juce_core/io/juce_InputStream.h"
#include "../../../../juce_core/io/juce_OutputStream.h"
#include "../geometry/juce_Path.h"
class Font;
class Typeface;


//==============================================================================
/**
    Stores information about the shape and kerning of one of the glyphs in a Typeface.

    @see Typeface, PositionedGlyph, GlyphArrangement
*/
class JUCE_API  TypefaceGlyphInfo
{
public:
    //==============================================================================
    /** Returns the path that describes the glyph's outline.

        This is normalised to a height of 1.0, and its origin is the
        left-hand edge of the glyph's baseline.
    */
    const Path& getPath() const throw()             { return path; }

    /** Returns the unicode character that this glyph represents. */
    juce_wchar getCharacter() const throw()         { return character; }

    bool isWhitespace() const throw()               { return CharacterFunctions::isWhitespace (character); }

    /** Returns the distance to leave between this and a following character.

        The value returned is expressed as a proportion of the font's height.
    */
    float getHorizontalSpacing (const juce_wchar subsequentCharacter) const throw();

    /** Returns the typeface that this glyph belongs to. */
    Typeface* getTypeface() const throw()           { return typeface; }


private:
    //==============================================================================
    friend class Typeface;

    struct KerningPair
    {
        juce_wchar character2;
        float kerningAmount;
    };

    const juce_wchar character;
    const Path path;
    float width;
    MemoryBlock kerningPairs;
    Typeface* const typeface;

    TypefaceGlyphInfo (const juce_wchar character,
                       const Path& shape,
                       const float horizontalSeparation,
                       Typeface* const typeface) throw();
    ~TypefaceGlyphInfo() throw();

    KerningPair& getKerningPair (const int index) const throw();
    int getNumKerningPairs() const throw();

    void addKerningPair (const juce_wchar subsequentCharacter,
                         const float extraKerningAmount) throw();

    const TypefaceGlyphInfo& operator= (const TypefaceGlyphInfo&);
};


//==============================================================================
/**
    Represents a size-independent system font.

    A Font object represents a particular Typeface along with a specific size,
    style, kerning, scale, etc, wheras the Typeface is just a generalised description
    of the shapes of the glyphs and their properties.

*/
class JUCE_API  Typeface  : public ReferenceCountedObject
{
public:
    //==============================================================================
    /** Tries to load a named system font and to initialise all the glyphs
        appropriately from it.

        @param faceName     the name of the typeface, e.g. "Times"
        @param bold         whether to try to find a bold version of the font (may not always be available)
        @param italic       whether to try to find an italicised version of the font (may not always be available)
    */
    Typeface (const String& faceName,
              const bool bold,
              const bool italic);

    /** Creates a copy of another typeface */
    Typeface (const Typeface& other);

    /** Destructor. */
    ~Typeface();

    /** Copies another typeface over this one. */
    const Typeface& operator= (const Typeface& other) throw();

    /** Returns a unique ID for the typeface.

        This is based on the name and style, so can be used to compare two Typeface objects.
    */
    int hashCode() const throw()                    { return hash; }

    //==============================================================================
    /** Returns the name of the typeface, e.g. "Times", "Verdana", etc */
    const String& getName() const throw()           { return typefaceName; }

    /** Returns the font's ascent as a proportion of its height. */
    float getAscent() const throw()                 { return ascent; }

    /** Returns true if the font is flagged as being bold. */
    bool isBold() const throw()                     { return bold; }

    /** Returns true if the typeface's 'italic' flag is set. */
    bool isItalic() const throw()                   { return italic; }

    //==============================================================================
    /** Finds the Path that describes the outline shape of a character.

        The height of the path is normalised to 1.0 (i.e. a distance of 1.0 is the
        height of the font).

        This may return 0 if the typeface has no characters, but if the character
        that is asked for is not found, it will first try to return a default
        character instead.
    */
    const Path* getOutlineForGlyph (const juce_wchar character) throw();

    /** Tries to find the information describing a glyph for this character.

        If there isn't a glyph specifically for the character it will return
        a default glyph instead; if the typeface is empty, it may return a null
        pointer.
    */
    const TypefaceGlyphInfo* getGlyph (const juce_wchar character) throw();

    //==============================================================================
    /** Deletes all the glyphs and kerning data fom the typeface. */
    void clear() throw();

    /** Adds a glyph to the typeface.

        This is typically only called by the platform-specific code that generates
        the typeface from a system font.
    */
    void addGlyph (const juce_wchar character,
                   const Path& path,
                   const float horizontalSpacing) throw();

    /** Adds a kerning distance to the typeface.

        The extra amount passed in is expressed as a proportion of the font's
        height, normalised to 1.0.

        This is typically only called by the platform-specific code that generates
        the typeface from a system font.
    */
    void addKerningPair (const juce_wchar firstChar,
                         const juce_wchar secondChar,
                         const float extraAmount) throw();

    /** Sets the typeface's name.

        This is typically only called by the platform-specific code that generates
        the typeface from a system font. Calling this method won't actually affect
        the underlying font being used.
    */
    void setName (const String& name) throw();

    /** Sets the font's ascent value, as a proportion of the font height.

        This is typically only called by the platform-specific code that generates
        the typeface from a system font.
    */
    void setAscent (const float newAscent) throw();

    /** Sets the typeface's 'bold' flag.

        This is typically only called by the platform-specific code that generates
        the typeface from a system font.
    */
    void setBold (const bool shouldBeBold) throw();

    /** Sets the typeface's 'italic' flag.

        This is typically only called by the platform-specific code that generates
        the typeface from a system font.
    */
    void setItalic (const bool shouldBeItalic) throw();

    /** Changes the character index to use as the default character.

        This is the character that gets returned for characters which don't have a
        glyph set for them.
    */
    void setDefaultCharacter (const juce_wchar newDefaultCharacter) throw();

    //==============================================================================
    /** Creates a typeface from data created using Typeface::serialise().

        This will attempt to load a compressed typeface that was created using
        the Typeface::serialise() method. This is handy if you want to store
        a typeface in your application as a binary blob, and use it without
        having to actually install it on the computer.

        @see Typeface::serialise()
    */
    Typeface (InputStream& serialisedTypefaceStream);

    /** Writes the typeface to a stream (using a proprietary format).

        This lets you save a typeface and reload it using the
        Typeface::Typeface (InputStream&) constructor. The data's saved in
        a compressed format.

        @see Typeface::Typeface (InputStream&)
    */
    void serialise (OutputStream& outputStream);

    //==============================================================================
    /** A handy typedef to make it easy to use ref counted pointers to this class. */
    typedef ReferenceCountedObjectPtr <Typeface> Ptr;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    VoidArray glyphs;
    short lookupTable [128];

    String typefaceName;
    int hash;
    float ascent; // as a proportion of the height
    bool bold, italic, isFullyPopulated;
    juce_wchar defaultCharacter; // the char to use if a matching glyph can't be found.

    Typeface() throw();
    void addGlyphCopy (const TypefaceGlyphInfo* const glyphInfoToCopy) throw();

    friend class Font;
    friend class TypefaceCache;
    friend class FontGlyphAlphaMap;

    static const Ptr getTypefaceFor (const Font& font) throw();

    // this is a platform-dependent method that will look for the given typeface
    // and set up its kerning tables, etc. accordingly.
    // If addAllGlyphsToFont is true, it should also add all the glyphs in the font
    // to the typeface immediately, rather than having to add them later on-demand.
    void initialiseTypefaceCharacteristics (const String& fontName,
                                            bool bold, bool italic,
                                            bool addAllGlyphsToFont) throw();

    // platform-specific routine to look up and add a glyph to this typeface
    void findAndAddSystemGlyph (juce_wchar character) throw();

    void updateHashCode() throw();
};


#endif   // __JUCE_TYPEFACE_JUCEHEADER__
