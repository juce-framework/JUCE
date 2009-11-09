/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_TYPEFACE_JUCEHEADER__
#define __JUCE_TYPEFACE_JUCEHEADER__

#include "../../../containers/juce_ReferenceCountedObject.h"
#include "../../../containers/juce_OwnedArray.h"
#include "../../../io/streams/juce_InputStream.h"
#include "../../../io/streams/juce_OutputStream.h"
#include "../geometry/juce_Path.h"
class Font;
class CustomTypefaceGlyphInfo;


//==============================================================================
/** A typeface represents a size-independent font.

    This base class is abstract, but calling createSystemTypefaceFor() will return
    a platform-specific subclass that can be used.

    The CustomTypeface subclass allow you to build your own typeface, and to
    load and save it in the Juce typeface format.

    Normally you should never need to deal directly with Typeface objects - the Font
    class does everything you typically need for rendering text.

    @see CustomTypeface, Font
*/
class JUCE_API  Typeface  : public ReferenceCountedObject
{
public:
    //==============================================================================
    /** A handy typedef for a pointer to a typeface. */
    typedef ReferenceCountedObjectPtr <Typeface> Ptr;

    //==============================================================================
    /** Returns the name of the typeface.
        @see Font::getTypefaceName
    */
    const String getName() const throw()       { return name; }

    //==============================================================================
    /** Creates a new system typeface. */
    static const Ptr createSystemTypefaceFor (const Font& font);

    //==============================================================================
    /** Destructor. */
    virtual ~Typeface();

    /** Returns the ascent of the font, as a proportion of its height.
        The height is considered to always be normalised as 1.0, so this will be a
        value less that 1.0, indicating the proportion of the font that lies above
        its baseline.
    */
    virtual float getAscent() const = 0;

    /** Returns the descent of the font, as a proportion of its height.
        The height is considered to always be normalised as 1.0, so this will be a
        value less that 1.0, indicating the proportion of the font that lies below
        its baseline.
    */
    virtual float getDescent() const = 0;

    /** Measures the width of a line of text.

        The distance returned is based on the font having an normalised height of 1.0.

        You should never need to call this directly! Use Font::getStringWidth() instead!
    */
    virtual float getStringWidth (const String& text) = 0;

    /** Converts a line of text into its glyph numbers and their positions.

        The distances returned are based on the font having an normalised height of 1.0.

        You should never need to call this directly! Use Font::getGlyphPositions() instead!
    */
    virtual void getGlyphPositions (const String& text, Array <int>& glyphs, Array<float>& xOffsets) = 0;

    /** Returns the outline for a glyph.

        The path returned will be normalised to a font height of 1.0.
    */
    virtual bool getOutlineForGlyph (int glyphNumber, Path& path) = 0;


    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    String name;

    Typeface (const String& name) throw();

private:
    Typeface (const Typeface&);
    const Typeface& operator= (const Typeface&);
};

//==============================================================================
/** A typeface that can be populated with custom glyphs.

    You can create a CustomTypeface if you need one that contains your own glyphs,
    or if you need to load a typeface from a Juce-formatted binary stream.

    If you want to create a copy of a native face, you can use addGlyphsFromOtherTypeface()
    to copy glyphs into this face.

    @see Typeface, Font
*/
class JUCE_API  CustomTypeface  : public Typeface
{
public:
    //==============================================================================
    /** Creates a new, empty typeface. */
    CustomTypeface();

    /** Loads a typeface from a previously saved stream.
        The stream must have been created by writeToStream().
        @see writeToStream
    */
    CustomTypeface (InputStream& serialisedTypefaceStream);

    /** Destructor. */
    ~CustomTypeface();

    //==============================================================================
    /** Resets this typeface, deleting all its glyphs and settings. */
    void clear();

    /** Sets the vital statistics for the typeface.
        @param name     the typeface's name
        @param ascent   the ascent - this is normalised to a height of 1.0 and this is
                        the value that will be returned by Typeface::getAscent(). The
                        descent is assumed to be (1.0 - ascent)
        @param isBold   should be true if the typeface is bold
        @param isItalic should be true if the typeface is italic
        @param defaultCharacter     the character to be used as a replacement if there's
                        no glyph available for the character that's being drawn
    */
    void setCharacteristics (const String& name, const float ascent,
                             const bool isBold, const bool isItalic,
                             const juce_wchar defaultCharacter) throw();

    /** Adds a glyph to the typeface.

        The path that is passed in is normalised so that the font height is 1.0, and its
        origin is the anchor point of the character on its baseline.

        The width is the nominal width of the character, and any extra kerning values that
        are specified will be added to this width.
    */
    void addGlyph (const juce_wchar character, const Path& path, const float width) throw();

    /** Specifies an extra kerning amount to be used between a pair of characters.
        The amount will be added to the nominal width of the first character when laying out a string.
    */
    void addKerningPair (const juce_wchar char1, const juce_wchar char2, const float extraAmount) throw();

    /** Adds a range of glyphs from another typeface.
        This will attempt to pull in the paths and kerning information from another typeface and
        add it to this one.
    */
    void addGlyphsFromOtherTypeface (Typeface& typefaceToCopy, juce_wchar characterStartIndex, int numCharacters) throw();

    /** Saves this typeface as a Juce-formatted font file.
        A CustomTypeface can be created to reload the data that is written - see the CustomTypeface
        constructor.
    */
    bool writeToStream (OutputStream& outputStream);

    //==============================================================================
    // The following methods implement the basic Typeface behaviour.
    float getAscent() const;
    float getDescent() const;
    float getStringWidth (const String& text);
    void getGlyphPositions (const String& text, Array <int>& glyphs, Array<float>& xOffsets);
    bool getOutlineForGlyph (int glyphNumber, Path& path);
    int getGlyphForCharacter (juce_wchar character);

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    juce_wchar defaultCharacter;
    float ascent;
    bool isBold, isItalic;

    //==============================================================================
    /** If a subclass overrides this, it can load glyphs into the font on-demand.
        When methods such as getGlyphPositions() or getOutlineForGlyph() are asked for a
        particular character and there's no corresponding glyph, they'll call this
        method so that a subclass can try to add that glyph, returning true if it
        manages to do so.
    */
    virtual bool loadGlyphIfPossible (const juce_wchar characterNeeded);

private:
    //==============================================================================
    OwnedArray <CustomTypefaceGlyphInfo> glyphs;
    short lookupTable [128];

    CustomTypeface (const CustomTypeface&);
    const CustomTypeface& operator= (const CustomTypeface&);

    CustomTypefaceGlyphInfo* findGlyph (const juce_wchar character, const bool loadIfNeeded) throw();
    CustomTypefaceGlyphInfo* findGlyphSubstituting (const juce_wchar character) throw();
};

#endif   // __JUCE_TYPEFACE_JUCEHEADER__
