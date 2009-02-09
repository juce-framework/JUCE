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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#ifdef JUCE_INCLUDED_FILE


//==============================================================================
class FontHelper
{
    NSFont* font;

public:
    String name;
    bool isBold, isItalic;
    float fontSize, totalSize, ascent;
    int refCount;

    FontHelper (const String& name_,
                const bool bold_,
                const bool italic_,
                const float size_)
        : font (0),
          name (name_),
          isBold (bold_),
          isItalic (italic_),
          fontSize (size_),
          refCount (1)
    {
        font = [NSFont fontWithName: juceStringToNS (name_) size: size_];

        if (italic_)
            font = [[NSFontManager sharedFontManager] convertFont: font toHaveTrait: NSItalicFontMask];

        if (bold_)
            font = [[NSFontManager sharedFontManager] convertFont: font toHaveTrait: NSBoldFontMask];

        [font retain];

        ascent = fabsf ([font ascender]);
        totalSize = ascent + fabsf ([font descender]);
    }

    ~FontHelper()
    {
        [font release];
    }

    bool getPathAndKerning (const juce_wchar char1,
                            const juce_wchar char2,
                            Path* path,
                            float& kerning,
                            float* ascent,
                            float* descent)
    {
        const ScopedAutoReleasePool pool;

        if (font == 0 
             || ! [[font coveredCharacterSet] longCharacterIsMember: (UTF32Char) char1])
            return false;

        String chars;
        chars << ' ' << char1 << char2;
        NSTextStorage* textStorage = [[[NSTextStorage alloc] 
              initWithString: juceStringToNS (chars)
                  attributes: [NSDictionary dictionaryWithObject: [NSNumber numberWithInt: 0] 
                                                          forKey: NSLigatureAttributeName]] autorelease];
        NSLayoutManager* layoutManager = [[[NSLayoutManager alloc] init] autorelease];
        NSTextContainer* textContainer = [[[NSTextContainer alloc] init] autorelease];
        [layoutManager addTextContainer: textContainer];
        [textStorage addLayoutManager: layoutManager];
        [textStorage setFont: font];

        unsigned int glyphIndex = [layoutManager glyphRangeForCharacterRange: NSMakeRange (1, 1) 
                                                        actualCharacterRange: 0].location;
        NSPoint p1 = [layoutManager locationForGlyphAtIndex: glyphIndex];
        NSPoint p2 = [layoutManager locationForGlyphAtIndex: glyphIndex + 1];
        kerning = p2.x - p1.x;

        if (ascent != 0)
            *ascent = this->ascent;

        if (descent != 0)
            *descent = fabsf ([font descender]);

        if (path != 0)
        {
            NSBezierPath* bez = [NSBezierPath bezierPath];
            [bez moveToPoint: NSMakePoint (0, 0)];
            [bez appendBezierPathWithGlyph: [layoutManager glyphAtIndex: glyphIndex] 
                                                                 inFont: font];

            for (int i = 0; i < [bez elementCount]; ++i)
            {
                NSPoint p[3];
                switch ([bez elementAtIndex: i associatedPoints: p])
                {
                case NSMoveToBezierPathElement:
                    path->startNewSubPath (p[0].x, -p[0].y);
                    break;
                case NSLineToBezierPathElement:
                    path->lineTo (p[0].x, -p[0].y);
                    break;
                case NSCurveToBezierPathElement:
                    path->cubicTo (p[0].x, -p[0].y, p[1].x, -p[1].y, p[2].x, -p[2].y);
                    break;
                case NSClosePathBezierPathElement:
                    path->closeSubPath();
                    break;
                default:
                    jassertfalse
                    break;
                }
            }
        }

        return kerning != 0;
    }

    juce_wchar getDefaultChar()
    {
        return 0;
    }
};

//==============================================================================
class FontHelperCache  : public Timer,
                         public DeletedAtShutdown
{
    VoidArray cache;

public:
    FontHelperCache()
    {
    }

    ~FontHelperCache()
    {
        for (int i = cache.size(); --i >= 0;)
        {
            FontHelper* const f = (FontHelper*) cache.getUnchecked(i);
            delete f;
        }

        clearSingletonInstance();
    }

    FontHelper* getFont (const String& name,
                         const bool bold,
                         const bool italic,
                         const float size = 1024)
    {
        for (int i = cache.size(); --i >= 0;)
        {
            FontHelper* const f = (FontHelper*) cache.getUnchecked(i);

            if (f->name == name
                && f->isBold == bold
                && f->isItalic == italic
                && f->fontSize == size)
            {
                f->refCount++;
                return f;
            }
        }

        FontHelper* const f = new FontHelper (name, bold, italic, size);
        cache.add (f);
        return f;
    }

    void releaseFont (FontHelper* f)
    {
        for (int i = cache.size(); --i >= 0;)
        {
            FontHelper* const f2 = (FontHelper*) cache.getUnchecked(i);

            if (f == f2)
            {
                f->refCount--;

                if (f->refCount == 0)
                    startTimer (5000);

                break;
            }
        }
    }

    void timerCallback()
    {
        stopTimer();

        for (int i = cache.size(); --i >= 0;)
        {
            FontHelper* const f = (FontHelper*) cache.getUnchecked(i);

            if (f->refCount == 0)
            {
                cache.remove (i);
                delete f;
            }
        }

        if (cache.size() == 0)
            delete this;
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (FontHelperCache)
};

juce_ImplementSingleton_SingleThreaded (FontHelperCache)

//==============================================================================
void Typeface::initialiseTypefaceCharacteristics (const String& fontName,
                                                  bool bold,
                                                  bool italic,
                                                  bool addAllGlyphsToFont) throw()
{
    // This method is only safe to be called from the normal UI thread..
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

    FontHelper* const helper = FontHelperCache::getInstance()
                                  ->getFont (fontName, bold, italic);

    clear();
    setAscent (helper->ascent / helper->totalSize);
    setName (fontName);
    setDefaultCharacter (helper->getDefaultChar());
    setBold (bold);
    setItalic (italic);

    if (addAllGlyphsToFont)
    {
        //xxx
        jassertfalse
    }

    FontHelperCache::getInstance()->releaseFont (helper);
}

bool Typeface::findAndAddSystemGlyph (juce_wchar character) throw()
{
    // This method is only safe to be called from the normal UI thread..
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

    FontHelper* const helper = FontHelperCache::getInstance()
                                  ->getFont (getName(), isBold(), isItalic());

    Path path;
    float width;
    bool foundOne = false;

    if (helper->getPathAndKerning (character, T('I'), &path, width, 0, 0))
    {
        path.applyTransform (AffineTransform::scale (1.0f / helper->totalSize,
                                                     1.0f / helper->totalSize));

        addGlyph (character, path, width / helper->totalSize);

        for (int i = 0; i < glyphs.size(); ++i)
        {
            const TypefaceGlyphInfo* const g = (const TypefaceGlyphInfo*) glyphs.getUnchecked(i);

            float kerning;
            if (helper->getPathAndKerning (character, g->getCharacter(), 0, kerning, 0, 0))
            {
                kerning = (kerning - width) / helper->totalSize;

                if (kerning != 0)
                    addKerningPair (character, g->getCharacter(), kerning);
            }

            if (helper->getPathAndKerning (g->getCharacter(), character, 0, kerning, 0, 0))
            {
                kerning = kerning / helper->totalSize - g->width;

                if (kerning != 0)
                    addKerningPair (g->getCharacter(), character, kerning);
            }
        }

        foundOne = true;
    }

    FontHelperCache::getInstance()->releaseFont (helper);
    return foundOne;
}

//==============================================================================
const StringArray Font::findAllTypefaceNames() throw()
{
    StringArray names;

    const ScopedAutoReleasePool pool;
    NSArray* fonts = [[NSFontManager sharedFontManager] availableFontFamilies];

    for (int i = 0; i < [fonts count]; ++i)
        names.add (nsStringToJuce ((NSString*) [fonts objectAtIndex: i]));

    names.sort (true);
    return names;
}

void Typeface::getDefaultFontNames (String& defaultSans, String& defaultSerif, String& defaultFixed) throw()
{
    defaultSans  = "Lucida Grande";
    defaultSerif = "Times New Roman";
    defaultFixed = "Monaco";
}

#endif
