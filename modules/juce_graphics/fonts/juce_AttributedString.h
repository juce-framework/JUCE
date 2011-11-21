/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_ATTRIBUTEDSTRING_JUCEHEADER__
#define __JUCE_ATTRIBUTEDSTRING_JUCEHEADER__


//==============================================================================
/**
    A text string with a set of colour/font settings that are associated with sub-ranges
    of the text.

    An attributed string lets you create a string with varied fonts, colours, word-wrapping,
    layout, etc., and draw it using AttributedString::draw().
*/
class JUCE_API  AttributedString
{
public:
    /** Creates an empty attributed string. */
    AttributedString();

    /** Creates an attributed string with the given text. */
    explicit AttributedString (const String& text);

    AttributedString (const AttributedString& other);
    AttributedString& operator= (const AttributedString& other);

    /** Destructor. */
    ~AttributedString();

    //==============================================================================
    /** Returns the complete text of this attributed string. */
    const String& getText() const noexcept                  { return text; }

    /** Sets the text.
        This will change the text, but won't affect any of the attributes that have
        been added.
    */
    void setText (const String& newText);

    //==============================================================================
    /** Draws this string within the given area.
        The layout of the string within the rectangle is controlled by the justification
        value passed to setJustification().
    */
    void draw (Graphics& g, const Rectangle<float>& area) const;

    //==============================================================================
    /** Returns the justification that should be used for laying-out the text.
        This may include both vertical and horizontal flags.
    */
    Justification getJustification() const noexcept         { return justification; }

    /** Sets the justification that should be used for laying-out the text.
        This may include both vertical and horizontal flags.
    */
    void setJustification (const Justification& newJustification) noexcept;

    //==============================================================================
    /** Types of word-wrap behaviour.
        @see getWordWrap, setWordWrap
    */
    enum WordWrap
    {
        none,   /**< No word-wrapping: lines extend indefinitely. */
        byWord, /**< Lines are wrapped on a word boundary. */
        byChar, /**< Lines are wrapped on a character boundary. */
    };

    /** Returns the word-wrapping behaviour. */
    WordWrap getWordWrap() const noexcept                   { return wordWrap; }

    /** Sets the word-wrapping behaviour. */
    void setWordWrap (WordWrap newWordWrap) noexcept;

    //==============================================================================
    /** Types of reading direction that can be used.
        @see getReadingDirection, setReadingDirection
    */
    enum ReadingDirection
    {
        natural,
        leftToRight,
        rightToLeft,
    };

    /** Returns the reading direction for the text. */
    ReadingDirection getReadingDirection() const noexcept   { return readingDirection; }

    /** Sets the reading direction that should be used for the text. */
    void setReadingDirection (ReadingDirection newReadingDirection) noexcept;

    //==============================================================================
    /** Returns the extra line-spacing distance. */
    float getLineSpacing() const noexcept                   { return lineSpacing; }

    /** Sets an extra line-spacing distance. */
    void setLineSpacing (float newLineSpacing) noexcept;

    //==============================================================================
    /** An attribute that has been applied to a range of characters in an AttributedString. */
    class JUCE_API  Attribute
    {
    public:
        /** Creates an attribute that changes the colour for a range of characters.
            @see AttributedString::setColour()
        */
        Attribute (const Range<int>& range, const Colour& colour);

        /** Creates an attribute that changes the font for a range of characters.
            @see AttributedString::setFont()
        */
        Attribute (const Range<int>& range, const Font& font);

        Attribute (const Attribute&);
        ~Attribute();

        /** If this attribute specifies a font, this returns it; otherwise it returns nullptr. */
        const Font* getFont() const noexcept            { return font; }

        /** If this attribute specifies a colour, this returns it; otherwise it returns nullptr. */
        const Colour* getColour() const noexcept        { return colour; }

        /** The range of characters to which this attribute should be applied. */
        const Range<int> range;

    private:
        ScopedPointer<Font> font;
        ScopedPointer<Colour> colour;

        Attribute& operator= (const Attribute&);
    };

    /** Returns the number of attributes that have been added to this string. */
    int getNumAttributes() const noexcept                       { return attributes.size(); }

    /** Returns one of the string's attributes.
        The index provided must be less than getNumAttributes(), and >= 0.
    */
    const Attribute* getAttribute (int index) const noexcept    { return attributes.getUnchecked (index); }

    //==============================================================================
    /** Adds a colour attribute for the specified range. */
    void setColour (const Range<int>& range, const Colour& colour);

    /** Adds a font attribute for the specified range. */
    void setFont (const Range<int>& range, const Font& font);

private:
    String text;
    float lineSpacing;
    Justification justification;
    WordWrap wordWrap;
    ReadingDirection readingDirection;
    OwnedArray<Attribute> attributes;
};


//==============================================================================
/**

*/
class JUCE_API  GlyphLayout
{
public:
    /** Creates an empty layout. */
    GlyphLayout();

    /** Destructor. */
    ~GlyphLayout();

    //==============================================================================
    /** Creates a layout from the given attributed string.
        This will replace any data that is currently stored in the layout.
    */
    void setText (const AttributedString& text, float maxWidth);

    /** Draws the layout within the specified area.
        The position of the text within the rectangle is controlled by the justification
        flags set in the original AttributedString that was used to create this layout.
    */
    void draw (Graphics& g, const Rectangle<float>& area) const;

    //==============================================================================
    /** A positioned glyph. */
    class JUCE_API  Glyph
    {
    public:
        Glyph (int glyphCode, const Point<float>& anchor) noexcept;
        ~Glyph();

        const int glyphCode;
        const Point<float> anchor;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Glyph);
    };

    //==============================================================================
    /** A sequence of glyphs. */
    class JUCE_API  Run
    {
    public:
        Run();
        Run (const Range<int>& range, int numGlyphsToPreallocate);
        ~Run();

        int getNumGlyphs() const noexcept       { return glyphs.size(); }
        const Font& getFont() const noexcept    { return font; }
        const Colour& getColour() const         { return colour; }
        Glyph& getGlyph (int index) const;

        void setStringRange (const Range<int>& newStringRange) noexcept;
        void setFont (const Font& newFont);
        void setColour (const Colour& newColour) noexcept;

        void addGlyph (Glyph* glyph);
        void ensureStorageAllocated (int numGlyphsNeeded);

    private:
        OwnedArray<Glyph> glyphs;
        Range<int> stringRange;
        Font font;
        Colour colour;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Run);
    };

    //==============================================================================
    /** A line containing a sequence of glyph-runs. */
    class JUCE_API  Line
    {
    public:
        Line() noexcept;
        Line (const Range<int>& stringRange, const Point<float>& lineOrigin,
              float ascent, float descent, float leading, int numRunsToPreallocate);
        ~Line();

        const Point<float>& getLineOrigin() const noexcept      { return lineOrigin; }

        float getAscent() const noexcept                        { return ascent; }
        float getDescent() const noexcept                       { return descent; }
        float getLeading() const noexcept                       { return leading; }

        int getNumRuns() const noexcept                         { return runs.size(); }
        Run& getRun (int index) const noexcept;

        void setStringRange (const Range<int>& newStringRange) noexcept;
        void setLineOrigin (const Point<float>& newLineOrigin) noexcept;
        void setLeading (float newLeading) noexcept;
        void increaseAscentDescent (float newAscent, float newDescent) noexcept;

        void addRun (Run* run);

    private:
        OwnedArray<Run> runs;
        Range<int> stringRange;
        Point<float> lineOrigin;
        float ascent, descent, leading;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Line);
    };

    //==============================================================================
    /** Returns the maximum width of the content. */
    float getWidth() const noexcept     { return width; }

    /** Returns the maximum height of the content. */
    float getHeight() const noexcept;

    /** Returns the number of lines in the layout. */
    int getNumLines() const noexcept    { return lines.size(); }

    /** Returns one of the lines. */
    Line& getLine (int index) const;

    /** Adds a line to the layout. The object passed-in will be owned and deleted by the layout
        when it is no longer needed.
    */
    void addLine (Line* line);

    /** Pre-allocates space for the specified number of lines. */
    void ensureStorageAllocated (int numLinesNeeded);

private:
    OwnedArray<Line> lines;
    float width;
    Justification justification;

    void createStandardLayout (const AttributedString&);
    bool createNativeLayout (const AttributedString&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphLayout);
};

#endif   // __JUCE_ATTRIBUTEDSTRING_JUCEHEADER__
