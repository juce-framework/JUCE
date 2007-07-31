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

#include "../../../src/juce_core/basics/juce_StandardHeader.h"
#include <ApplicationServices/ApplicationServices.h>

BEGIN_JUCE_NAMESPACE


#include "../../../src/juce_appframework/gui/graphics/fonts/juce_Font.h"
#include "../../../src/juce_appframework/events/juce_Timer.h"
#include "../../../src/juce_appframework/application/juce_DeletedAtShutdown.h"
#include "../../../src/juce_core/basics/juce_Singleton.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"


//==============================================================================
static OSStatus pascal CubicMoveTo (const Float32Point *pt,
                                    void* callBackDataPtr)
{
    Path* const p = (Path*) callBackDataPtr;
    p->startNewSubPath (pt->x, pt->y);

    return noErr;
}

static OSStatus pascal CubicLineTo (const Float32Point *pt,
                                    void* callBackDataPtr)
{
    Path* const p = (Path*) callBackDataPtr;
    p->lineTo (pt->x, pt->y);

    return noErr;
}

static OSStatus pascal CubicCurveTo (const Float32Point *pt1,
                                     const Float32Point *pt2,
                                     const Float32Point *pt3,
                                     void* callBackDataPtr)
{
    Path* const p = (Path*) callBackDataPtr;
    p->cubicTo (pt1->x, pt1->y,
                pt2->x, pt2->y,
                pt3->x, pt3->y);

    return noErr;
}

static OSStatus pascal CubicClosePath (void* callBackDataPtr)
{
    Path* const p = (Path*) callBackDataPtr;
    p->closeSubPath();

    return noErr;
}

//==============================================================================
class ATSFontHelper
{
    ATSUFontID fontId;
    ATSUStyle style;

    ATSCubicMoveToUPP moveToProc;
    ATSCubicLineToUPP lineToProc;
    ATSCubicCurveToUPP curveToProc;
    ATSCubicClosePathUPP closePathProc;

    float totalSize, ascent;

    TextToUnicodeInfo encodingInfo;

public:
    String name;
    bool isBold, isItalic;
    float fontSize;
    int refCount;

    ATSFontHelper (const String& name_,
                   const bool bold_,
                   const bool italic_,
                   const float size_)
        : fontId (0),
          name (name_),
          isBold (bold_),
          isItalic (italic_),
          fontSize (size_),
          refCount (1)
    {
        const char* const nameUtf8 = name_.toUTF8();

        ATSUFindFontFromName (const_cast <char*> (nameUtf8),
                              strlen (nameUtf8),
                              kFontFullName,
                              kFontNoPlatformCode,
                              kFontNoScriptCode,
                              kFontNoLanguageCode,
                              &fontId);

        ATSUCreateStyle (&style);

        ATSUAttributeTag attTypes[] = { kATSUFontTag,
                                        kATSUQDBoldfaceTag,
                                        kATSUQDItalicTag,
                                        kATSUSizeTag };

        ByteCount attSizes[] = { sizeof (ATSUFontID),
                                 sizeof (Boolean),
                                 sizeof (Boolean),
                                 sizeof (Fixed) };

        Boolean bold = bold_, italic = italic_;
        Fixed size = X2Fix (size_);

        ATSUAttributeValuePtr attValues[] = { &fontId,
                                              &bold,
                                              &italic,
                                              &size };

        ATSUSetAttributes (style, 4, attTypes, attSizes, attValues);

        moveToProc = NewATSCubicMoveToUPP (CubicMoveTo);
        lineToProc = NewATSCubicLineToUPP (CubicLineTo);
        curveToProc = NewATSCubicCurveToUPP (CubicCurveTo);
        closePathProc = NewATSCubicClosePathUPP (CubicClosePath);

        ascent = 0.0f;
        float kern, descent = 0.0f;
        getPathAndKerning (T('N'), T('O'), 0, kern, &ascent, &descent);
        totalSize = ascent + descent;
    }

    ~ATSFontHelper()
    {
        ATSUDisposeStyle (style);

        DisposeATSCubicMoveToUPP (moveToProc);
        DisposeATSCubicLineToUPP (lineToProc);
        DisposeATSCubicCurveToUPP (curveToProc);
        DisposeATSCubicClosePathUPP (closePathProc);
    }

    bool getPathAndKerning (const juce_wchar char1,
                            const juce_wchar char2,
                            Path* path,
                            float& kerning,
                            float* ascent,
                            float* descent)
    {
        bool ok = false;

        UniChar buffer[4];
        buffer[0] = T(' ');
        buffer[1] = char1;
        buffer[2] = char2;
        buffer[3] = 0;

        UniCharCount count = kATSUToTextEnd;
        ATSUTextLayout layout;
        OSStatus err = ATSUCreateTextLayoutWithTextPtr (buffer,
                                                        0,
                                                        2,
                                                        2,
                                                        1,
                                                        &count,
                                                        &style,
                                                        &layout);
        if (err == noErr)
        {
            ATSUSetTransientFontMatching (layout, true);

            ATSLayoutRecord* layoutRecords;
            ItemCount numRecords;
            Fixed* deltaYs;
            ItemCount numDeltaYs;

            ATSUDirectGetLayoutDataArrayPtrFromTextLayout (layout,
                                                           0,
                                                           kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
                                                           (void**) &layoutRecords,
                                                           &numRecords);

            ATSUDirectGetLayoutDataArrayPtrFromTextLayout (layout,
                                                           0,
                                                           kATSUDirectDataBaselineDeltaFixedArray,
                                                           (void**) &deltaYs,
                                                           &numDeltaYs);

            if (numRecords > 2)
            {
                kerning = (float) (Fix2X (layoutRecords[2].realPos)
                                   - Fix2X (layoutRecords[1].realPos));

                if (ascent != 0)
                {
                    ATSUTextMeasurement asc;
                    ByteCount actualSize;

                    ATSUGetLineControl (layout,
                                        0,
                                        kATSULineAscentTag,
                                        sizeof (ATSUTextMeasurement),
                                        &asc,
                                        &actualSize);

                    *ascent = (float) Fix2X (asc);
                }

                if (descent != 0)
                {
                    ATSUTextMeasurement desc;
                    ByteCount actualSize;

                    ATSUGetLineControl (layout,
                                        0,
                                        kATSULineDescentTag,
                                        sizeof (ATSUTextMeasurement),
                                        &desc,
                                        &actualSize);

                    *descent = (float) Fix2X (desc);
                }

                if (path != 0)
                {
                    OSStatus callbackResult;

                    ok = (ATSUGlyphGetCubicPaths (style,
                                                  layoutRecords[1].glyphID,
                                                  moveToProc,
                                                  lineToProc,
                                                  curveToProc,
                                                  closePathProc,
                                                  (void*) path,
                                                  &callbackResult) == noErr);

                    if (numDeltaYs > 0 && ok)
                    {
                        const float dy = (float) Fix2X (deltaYs[1]);

                        path->applyTransform (AffineTransform::translation (0.0f, dy));
                    }
                }
                else
                {
                    ok = true;
                }
            }

            if (deltaYs != 0)
                ATSUDirectReleaseLayoutDataArrayPtr (0, kATSUDirectDataBaselineDeltaFixedArray,
                                                     (void**) &deltaYs);

            if (layoutRecords != 0)
                ATSUDirectReleaseLayoutDataArrayPtr (0, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
                                                     (void**) &layoutRecords);

            ATSUDisposeTextLayout (layout);
        }

        return kerning;
    }

    float getAscent()
    {
        return ascent;
    }

    float getTotalHeight()
    {
        return totalSize;
    }

    juce_wchar getDefaultChar()
    {
        return 0;
    }
};

//==============================================================================
class ATSFontHelperCache  : public Timer,
                            public DeletedAtShutdown
{
    VoidArray cache;

public:
    ATSFontHelperCache()
    {
    }

    ~ATSFontHelperCache()
    {
        for (int i = cache.size(); --i >= 0;)
        {
            ATSFontHelper* const f = (ATSFontHelper*) cache[i];
            delete f;
        }

        clearSingletonInstance();
    }

    ATSFontHelper* getFont (const String& name,
                            const bool bold,
                            const bool italic,
                            const float size = 1024)
    {
        for (int i = cache.size(); --i >= 0;)
        {
            ATSFontHelper* const f = (ATSFontHelper*) cache.getUnchecked(i);

            if (f->name == name
                && f->isBold == bold
                && f->isItalic == italic
                && f->fontSize == size)
            {
                f->refCount++;
                return f;
            }
        }

        ATSFontHelper* const f = new ATSFontHelper (name, bold, italic, size);
        cache.add (f);
        return f;
    }

    void releaseFont (ATSFontHelper* f)
    {
        for (int i = cache.size(); --i >= 0;)
        {
            ATSFontHelper* const f2 = (ATSFontHelper*) cache[i];

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
        for (int i = cache.size(); --i >= 0;)
        {
            ATSFontHelper* const f = (ATSFontHelper*) cache[i];

            if (f->refCount > 0)
            {
                stopTimer();
                return;
            }
        }

        delete this;
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (ATSFontHelperCache)
};

juce_ImplementSingleton_SingleThreaded (ATSFontHelperCache)

//==============================================================================
void Typeface::initialiseTypefaceCharacteristics (const String& fontName,
                                                  bool bold,
                                                  bool italic,
                                                  bool addAllGlyphsToFont) throw()
{
    ATSFontHelper* const helper = ATSFontHelperCache::getInstance()
                                    ->getFont (fontName, bold, italic);

    clear();
    setAscent (helper->getAscent() / helper->getTotalHeight());
    setName (fontName);
    setDefaultCharacter (helper->getDefaultChar());
    setBold (bold);
    setItalic (italic);

    if (addAllGlyphsToFont)
    {
        //xxx
        jassertfalse
    }

    ATSFontHelperCache::getInstance()->releaseFont (helper);
}

void Typeface::findAndAddSystemGlyph (juce_wchar character) throw()
{
    ATSFontHelper* const helper = ATSFontHelperCache::getInstance()
                                    ->getFont (getName(), isBold(), isItalic());

    Path path;
    float width;

    if (helper->getPathAndKerning (character, T('I'), &path, width, 0, 0))
    {
        path.applyTransform (AffineTransform::scale (1.0f / helper->getTotalHeight(),
                                                     1.0f / helper->getTotalHeight()));

        addGlyph (character, path, width / helper->getTotalHeight());

        for (int i = 0; i < glyphs.size(); ++i)
        {
            const TypefaceGlyphInfo* const g = (const TypefaceGlyphInfo*) glyphs.getUnchecked(i);

            float kerning;
            if (helper->getPathAndKerning (character, g->getCharacter(), 0, kerning, 0, 0))
            {
                kerning = (kerning - width) / helper->getTotalHeight();

                if (kerning != 0)
                    addKerningPair (character, g->getCharacter(), kerning);
            }

            if (helper->getPathAndKerning (g->getCharacter(), character, 0, kerning, 0, 0))
            {
                kerning = kerning / helper->getTotalHeight() - g->width;

                if (kerning != 0)
                    addKerningPair (g->getCharacter(), character, kerning);
            }
        }
    }

    ATSFontHelperCache::getInstance()->releaseFont (helper);
}

const StringArray Font::findAllTypefaceNames() throw()
{
    StringArray names;
    ATSFontIterator iter;

    if (ATSFontIteratorCreate (kATSFontContextGlobal,
                               0,
                               0,
                               kATSOptionFlagsRestrictedScope,
                               &iter) == noErr)
    {
        ATSFontRef font;

        while (ATSFontIteratorNext (iter, &font) == noErr)
        {
            CFStringRef name;

            if (ATSFontGetName (font,
                                kATSOptionFlagsDefault,
                                &name) == noErr)
            {
                const String nm (PlatformUtilities::cfStringToJuceString (name));

                if (nm.isNotEmpty())
                    names.add (nm);

                CFRelease (name);
            }
        }

        ATSFontIteratorRelease (&iter);
    }

    // Use some totuous logic to eliminate bold/italic versions of fonts that we've already got
    // a plain version of. This is only necessary because of Carbon's total lack of support
    // for dealing with font families...
    for (int j = names.size(); --j >= 0;)
    {
        const char* const endings[] = { " bold", " italic", " bold italic", " bolditalic",
                                        " oblque", " bold oblique", " boldoblique" };

        for (int i = 0; i < numElementsInArray (endings); ++i)
        {
            const String ending (endings[i]);

            if (names[j].endsWithIgnoreCase (ending))
            {
                const String root (names[j].dropLastCharacters (ending.length()).trimEnd());

                if (names.contains (root)
                    || names.contains (root + T(" plain"), true))
                {
                    names.remove (j);
                    break;
                }
            }
        }
    }

    names.sort (true);
    return names;
}

void Font::getDefaultFontNames (String& defaultSans, String& defaultSerif, String& defaultFixed) throw()
{
    defaultSans  = "Lucida Grande";
    defaultSerif = "Times New Roman";
    defaultFixed = "Monaco";
}


END_JUCE_NAMESPACE
