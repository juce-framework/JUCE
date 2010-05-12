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

#include "../jucer_Headers.h"


//==============================================================================
const int64 hashCode64 (const String& s)
{
    return s.hashCode64() + s.length() * s.hashCode() + s.toUpperCase().hashCode();
}

const String createAlphaNumericUID()
{
    String uid;
    static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    Random r (Random::getSystemRandom().nextInt64());

    for (int i = 9; --i >= 0;)
    {
        r.setSeedRandomly();
        uid << (juce_wchar) chars [r.nextInt (sizeof (chars))];
    }

    return uid;
}

const String randomHexString (Random& random, int numChars)
{
    String s;
    const char hexChars[] = "0123456789ABCDEF";

    while (--numChars >= 0)
        s << hexChars [random.nextInt (16)];

    return s;
}

const String hexString8Digits (int value)
{
    return String::toHexString (value).paddedLeft ('0', 8);
}

const String createGUID (const String& seed)
{
    String guid;
    Random r (hashCode64 (seed + "_jucersalt"));
    guid << "{" << randomHexString (r, 8); // (written as separate statements to enforce the order of execution)
    guid << "-" << randomHexString (r, 4);
    guid << "-" << randomHexString (r, 4);
    guid << "-" << randomHexString (r, 4);
    guid << "-" << randomHexString (r, 12) << "}";
    return guid;
}

//==============================================================================
namespace FileUtils
{
    int64 calculateStreamHashCode (InputStream& in)
    {
        int64 t = 0;

        const int bufferSize = 4096;
        HeapBlock <uint8> buffer;
        buffer.malloc (bufferSize);

        for (;;)
        {
            const int num = in.read (buffer, bufferSize);

            if (num <= 0)
                break;

            for (int i = 0; i < num; ++i)
                t = t * 65599 + buffer[i];
        }

        return t;
    }

    int64 calculateFileHashCode (const File& file)
    {
        ScopedPointer <FileInputStream> stream (file.createInputStream());
        return stream != 0 ? calculateStreamHashCode (*stream) : 0;
    }

    bool areFilesIdentical (const File& file1, const File& file2)
    {
        return file1.getSize() == file2.getSize()
                && calculateFileHashCode (file1) == calculateFileHashCode (file2);
    }

    bool overwriteFileWithNewDataIfDifferent (const File& file, const char* data, int numBytes)
    {
        if (file.getSize() == numBytes)
        {
            MemoryInputStream newStream (data, numBytes, false);

            if (calculateStreamHashCode (newStream) == calculateFileHashCode (file))
                return true;
        }

        TemporaryFile temp (file);

        return temp.getFile().appendData (data, numBytes)
                 && temp.overwriteTargetFileWithTemporary();
    }

    bool overwriteFileWithNewDataIfDifferent (const File& file, const MemoryOutputStream& newData)
    {
        return overwriteFileWithNewDataIfDifferent (file, newData.getData(), newData.getDataSize());
    }

    bool overwriteFileWithNewDataIfDifferent (const File& file, const String& newData)
    {
        return overwriteFileWithNewDataIfDifferent (file, newData.toUTF8(), strlen ((const char*) newData.toUTF8()));
    }

    bool containsAnyNonHiddenFiles (const File& folder)
    {
        DirectoryIterator di (folder, false);

        while (di.next())
            if (! di.getFile().isHidden())
                return true;

        return false;
    }

    const String unixStylePath (const String& path)
    {
        return path.replaceCharacter ('\\', '/');
    }

    const String windowsStylePath (const String& path)
    {
        return path.replaceCharacter ('/', '\\');
    }

    const String appendPath (const String& path, const String& subpath)
    {
        if (File::isAbsolutePath (subpath)
             || subpath.startsWithChar ('$')
             || subpath.startsWithChar ('~')
             || (CharacterFunctions::isLetter (subpath[0]) && subpath[1] == ':'))
            return subpath.replaceCharacter ('\\', '/');

        String path1 (path.replaceCharacter ('\\', '/'));
        if (! path1.endsWithChar ('/'))
            path1 << '/';

        return path1 + subpath.replaceCharacter ('\\', '/');
    }

    bool shouldPathsBeRelative (String path1, String path2)
    {
        path1 = unixStylePath (path1);
        path2 = unixStylePath (path2);

        const int len = jmin (path1.length(), path2.length());
        int commonBitLength = 0;

        for (int i = 0; i < len; ++i)
        {
            if (CharacterFunctions::toLowerCase (path1[i]) != CharacterFunctions::toLowerCase (path2[i]))
                break;

            ++commonBitLength;
        }

        return path1.substring (0, commonBitLength).removeCharacters ("/:").isNotEmpty();
    }

    //==============================================================================
    bool isJuceFolder (const File& folder)
    {
        return folder.getFileName().containsIgnoreCase ("juce")
                 && folder.getChildFile ("juce.h").exists()
                 && folder.getChildFile ("juce_Config.h").exists();
    }

    static const File lookInFolderForJuceFolder (const File& folder)
    {
        for (DirectoryIterator di (folder, false, "*juce*", File::findDirectories); di.next();)
        {
            if (isJuceFolder (di.getFile()))
                return di.getFile();
        }

        return File::nonexistent;
    }

    const File findParentJuceFolder (const File& file)
    {
        File f (file);

        while (f.exists() && f.getParentDirectory() != f)
        {
            if (isJuceFolder (f))
                return f;

            File found = lookInFolderForJuceFolder (f);
            if (found.exists())
                return found;

            f = f.getParentDirectory();
        }

        return File::nonexistent;
    }

    const File findDefaultJuceFolder()
    {
        File f = findParentJuceFolder (File::getSpecialLocation (File::currentApplicationFile));

        if (! f.exists())
            f = lookInFolderForJuceFolder (File::getSpecialLocation (File::userHomeDirectory));

        if (! f.exists())
            f = lookInFolderForJuceFolder (File::getSpecialLocation (File::userDocumentsDirectory));

        return f;
    }
}

//==============================================================================
void autoScrollForMouseEvent (const MouseEvent& e)
{
    Viewport* const viewport = e.eventComponent->findParentComponentOfClass ((Viewport*) 0);

    if (viewport != 0)
    {
        const MouseEvent e2 (e.getEventRelativeTo (viewport));
        viewport->autoScroll (e2.x, e2.y, 8, 16);
    }
}

void drawComponentPlaceholder (Graphics& g, int w, int h, const String& text)
{
    g.fillAll (Colours::white.withAlpha (0.4f));
    g.setColour (Colours::grey);
    g.drawRect (0, 0, w, h);

    g.drawLine (0.5f, 0.5f, w - 0.5f, h - 0.5f);
    g.drawLine (0.5f, h - 0.5f, w - 0.5f, 0.5f);

    g.setColour (Colours::black);
    g.setFont (11.0f);
    g.drawFittedText (text, 2, 2, w - 4, h - 4, Justification::centredTop, 2);
}

void drawRecessedShadows (Graphics& g, int w, int h, int shadowSize)
{
    ColourGradient cg (Colours::black.withAlpha (0.15f), 0, 0,
                       Colours::transparentBlack, 0, (float) shadowSize, false);
    cg.addColour (0.4, Colours::black.withAlpha (0.07f));
    cg.addColour (0.6, Colours::black.withAlpha (0.02f));

    g.setGradientFill (cg);
    g.fillRect (0, 0, w, shadowSize);

    cg.point1.setXY (0.0f, (float) h);
    cg.point2.setXY (0.0f, (float) h - shadowSize);
    g.setGradientFill (cg);
    g.fillRect (0, h - shadowSize, w, shadowSize);

    cg.point1.setXY (0.0f, 0.0f);
    cg.point2.setXY ((float) shadowSize, 0.0f);
    g.setGradientFill (cg);
    g.fillRect (0, 0, shadowSize, h);

    cg.point1.setXY ((float) w, 0.0f);
    cg.point2.setXY ((float) w - shadowSize, 0.0f);
    g.setGradientFill (cg);
    g.fillRect (w - shadowSize, 0, shadowSize, h);
}

//==============================================================================
namespace CodeFormatting
{
    const String indent (const String& code, const int numSpaces, bool indentFirstLine)
    {
        if (numSpaces == 0)
            return code;

        const String space (String::repeatedString (" ", numSpaces));

        StringArray lines;
        lines.addLines (code);

        for (int i = (indentFirstLine ? 0 : 1); i < lines.size(); ++i)
        {
            String s (lines[i].trimEnd());
            if (s.isNotEmpty())
                s = space + s;

            lines.set (i, s);
        }

        return lines.joinIntoString (newLine);
    }

    const String makeValidIdentifier (String s, bool capitalise, bool removeColons, bool allowTemplates)
    {
        if (s.isEmpty())
            return "unknown";

        if (removeColons)
            s = s.replaceCharacters (".,;:/@", "______");
        else
            s = s.replaceCharacters (".,;/@", "_____");

        int i;
        for (i = s.length(); --i > 0;)
            if (CharacterFunctions::isLetter (s[i])
                 && CharacterFunctions::isLetter (s[i - 1])
                 && CharacterFunctions::isUpperCase (s[i])
                 && ! CharacterFunctions::isUpperCase (s[i - 1]))
                s = s.substring (0, i) + " " + s.substring (i);

        String allowedChars ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_ 0123456789");
        if (allowTemplates)
            allowedChars += "<>";

        if (! removeColons)
            allowedChars += ":";

        StringArray words;
        words.addTokens (s.retainCharacters (allowedChars), false);
        words.trim();

        String n (words[0]);

        if (capitalise)
            n = n.toLowerCase();

        for (i = 1; i < words.size(); ++i)
        {
            if (capitalise && words[i].length() > 1)
                n << words[i].substring (0, 1).toUpperCase()
                  << words[i].substring (1).toLowerCase();
            else
                n << words[i];
        }

        if (CharacterFunctions::isDigit (n[0]))
            n = "_" + n;

        if (CPlusPlusCodeTokeniser::isReservedKeyword (n))
            n << '_';

        return n;
    }

    const String addEscapeChars (const String& s)
    {
        const int len = s.length();
        String r;
        r.preallocateStorage (len + 2);
        bool lastWasHexEscapeCode = false;

        for (int i = 0; i < len; ++i)
        {
            const juce_wchar c = s[i];

            switch (c)
            {
                case '\t':  r << "\\t";  lastWasHexEscapeCode = false; break;
                case '\r':  r << "\\r";  lastWasHexEscapeCode = false; break;
                case '\n':  r << "\\n";  lastWasHexEscapeCode = false; break;
                case '\\':  r << "\\\\"; lastWasHexEscapeCode = false; break;
                case '\'':  r << "\\\'"; lastWasHexEscapeCode = false; break;
                case '\"':  r << "\\\""; lastWasHexEscapeCode = false; break;

                default:
                    if (c < 128
                         && ! (lastWasHexEscapeCode
                                 && String ("0123456789abcdefABCDEF").containsChar (c))) // (have to avoid following a hex escape sequence with a valid hex digit)
                    {
                        r << c;
                        lastWasHexEscapeCode = false;
                    }
                    else
                    {
                        r << "\\x" << String::toHexString ((int) c);
                        lastWasHexEscapeCode = true;
                    }

                    break;
            }
        }

        return r;
    }

    const String createIncludeStatement (const File& includeFile, const File& targetFile)
    {
        return "#include \"" + FileUtils::unixStylePath (includeFile.getRelativePathFrom (targetFile.getParentDirectory())) + "\"";
    }

    const String makeHeaderGuardName (const File& file)
    {
        return "__" + file.getFileName().toUpperCase()
                                        .replaceCharacters (" .", "__")
                                        .retainCharacters ("_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")
                + "_" + String::toHexString (file.hashCode()).toUpperCase() + "__";
    }

    const String stringLiteral (const String& text)
    {
        if (text.isEmpty())
            return "String::empty";

        return CodeFormatting::addEscapeChars (text).quoted();
    }

    const String boolLiteral (bool b)
    {
        return b ? "true" : "false";
    }

    const String floatLiteral (float v)
    {
        String s ((double) v, 4);

        if (s.containsChar ('.'))
            s << 'f';
        else
            s << ".0f";

        return s;
    }

    const String doubleLiteral (double v)
    {
        String s (v, 7);

        if (! s.containsChar ('.'))
            s << ".0";

        return s;
    }

    const String colourToCode (const Colour& col)
    {
        const Colour colours[] =
        {
            #define COL(col)  Colours::col,
            #include "jucer_Colours.h"
            #undef COL
            Colours::transparentBlack
        };

        static const char* colourNames[] =
        {
            #define COL(col)  #col,
            #include "jucer_Colours.h"
            #undef COL
            0
        };

        for (int i = 0; i < numElementsInArray (colourNames) - 1; ++i)
            if (col == colours[i])
                return "Colours::" + String (colourNames[i]);

        return "Colour (0x" + hexString8Digits ((int) col.getARGB()) + ')';
    }

    const String justificationToCode (const Justification& justification)
    {
        switch (justification.getFlags())
        {
            case Justification::centred:                return "Justification::centred";
            case Justification::centredLeft:            return "Justification::centredLeft";
            case Justification::centredRight:           return "Justification::centredRight";
            case Justification::centredTop:             return "Justification::centredTop";
            case Justification::centredBottom:          return "Justification::centredBottom";
            case Justification::topLeft:                return "Justification::topLeft";
            case Justification::topRight:               return "Justification::topRight";
            case Justification::bottomLeft:             return "Justification::bottomLeft";
            case Justification::bottomRight:            return "Justification::bottomRight";
            case Justification::left:                   return "Justification::left";
            case Justification::right:                  return "Justification::right";
            case Justification::horizontallyCentred:    return "Justification::horizontallyCentred";
            case Justification::top:                    return "Justification::top";
            case Justification::bottom:                 return "Justification::bottom";
            case Justification::verticallyCentred:      return "Justification::verticallyCentred";
            case Justification::horizontallyJustified:  return "Justification::horizontallyJustified";
            default:                                    jassertfalse; break;
        }

        return "Justification (" + String (justification.getFlags()) + ")";
    }

    const String castToFloat (const String& expression)
    {
        if (expression.containsOnly ("0123456789.f"))
        {
            String s (expression.getFloatValue());

            if (s.containsChar (T('.')))
                return s + "f";

            return s + ".0f";
        }

        return "(float) (" + expression + ")";
    }

    void writeDataAsCppLiteral (const MemoryBlock& mb, OutputStream& out)
    {
        const int maxCharsOnLine = 250;

        const unsigned char* data = (const unsigned char*) mb.getData();
        int charsOnLine = 0;

        bool canUseStringLiteral = mb.getSize() < 65535; // MS compilers can't handle strings bigger than 65536 chars..

        if (canUseStringLiteral)
        {
            int numEscaped = 0;

            for (size_t i = 0; i < mb.getSize(); ++i)
            {
                const unsigned int num = (unsigned int) data[i];
                if (! ((num >= 32 && num < 127) || num == '\t' || num == '\r' || num == '\n'))
                {
                    if (++numEscaped > mb.getSize() / 4)
                    {
                        canUseStringLiteral = false;
                        break;
                    }
                }
            }
        }

        if (! canUseStringLiteral)
        {
            out << "{ ";

            for (size_t i = 0; i < mb.getSize(); ++i)
            {
                const int num = (int) (unsigned int) data[i];
                out << num << ',';

                charsOnLine += 2;
                if (num >= 10)
                    ++charsOnLine;
                if (num >= 100)
                    ++charsOnLine;

                if (charsOnLine >= maxCharsOnLine)
                {
                    charsOnLine = 0;
                    out << newLine;
                }
            }

            out << "0,0 };";
        }
        else
        {
            out << "\"";

            for (size_t i = 0; i < mb.getSize(); ++i)
            {
                const unsigned int num = (unsigned int) data[i];

                switch (num)
                {
                    case '\t':   out << "\\t"; break;
                    case '\r':   out << "\\r"; break;
                    case '\n':   out << "\\n"; charsOnLine = maxCharsOnLine; break;
                    case '"':    out << "\\\""; break;
                    case '\\':   out << "\\\\"; break;
                    default:
                        if (num >= 32 && num < 127)
                            out << (char) num;
                        else if (num <= 0x0f)
                            out << "\\x0" << String::toHexString ((int) num);
                        else
                            out << "\\x" << String::toHexString ((int) num);
                        break;
                }

                if (++charsOnLine >= maxCharsOnLine && i < mb.getSize() - 1)
                {
                    charsOnLine = 0;
                    out << "\"" << newLine << "\"";
                }
            }

            out << "\";";
        }
    }
}

//==============================================================================
int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex)
{
    startIndex = jmax (0, startIndex);

    while (startIndex < lines.size())
    {
        if (lines[startIndex].trimStart().startsWithIgnoreCase (text))
            return startIndex;

        ++startIndex;
    }

    return -1;
}


//==============================================================================
PropertyPanelWithTooltips::PropertyPanelWithTooltips()
    : lastComp (0)
{
    addAndMakeVisible (panel = new PropertyPanel());
    startTimer (150);
}

PropertyPanelWithTooltips::~PropertyPanelWithTooltips()
{
    deleteAllChildren();
}

void PropertyPanelWithTooltips::paint (Graphics& g)
{
    g.setColour (Colour::greyLevel (0.15f));
    g.setFont (13.0f);

    TextLayout tl;
    tl.appendText (lastTip, Font (14.0f));
    tl.layout (getWidth() - 10, Justification::left, true); // try to make it look nice
    if (tl.getNumLines() > 3)
        tl.layout (getWidth() - 10, Justification::left, false); // too big, so just squash it in..

    tl.drawWithin (g, 5, panel->getBottom() + 2, getWidth() - 10,
                   getHeight() - panel->getBottom() - 4,
                   Justification::centredLeft);
}

void PropertyPanelWithTooltips::resized()
{
    panel->setBounds (0, 0, getWidth(), jmax (getHeight() - 60, proportionOfHeight (0.6f)));
}

void PropertyPanelWithTooltips::timerCallback()
{
    Component* const newComp = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

    if (newComp != lastComp)
    {
        lastComp = newComp;

        String newTip (findTip (newComp));

        if (newTip != lastTip)
        {
            lastTip = newTip;
            repaint (0, panel->getBottom(), getWidth(), getHeight());
        }
    }
}

const String PropertyPanelWithTooltips::findTip (Component* c)
{
    while (c != 0 && c != this)
    {
        TooltipClient* const tc = dynamic_cast <TooltipClient*> (c);
        if (tc != 0)
        {
            const String tip (tc->getTooltip());

            if (tip.isNotEmpty())
                return tip;
        }

        c = c->getParentComponent();
    }

    return String::empty;
}

//==============================================================================
FloatingLabelComponent::FloatingLabelComponent()
    : font (10.0f)
{
    setInterceptsMouseClicks (false ,false);
}

void FloatingLabelComponent::remove()
{
    if (getParentComponent() != 0)
        getParentComponent()->removeChildComponent (this);
}

void FloatingLabelComponent::update (Component* parent, const String& text, const Colour& textColour, int x, int y, bool toRight, bool below)
{
    colour = textColour;

    Rectangle<int> r;

    if (text != getName())
    {
        setName (text);
        glyphs.clear();
        glyphs.addJustifiedText (font, text, 0, 0, 200.0f, Justification::left);
        glyphs.justifyGlyphs (0, std::numeric_limits<int>::max(), 0, 0, 1000, 1000, Justification::topLeft);

        r = glyphs.getBoundingBox (0, std::numeric_limits<int>::max(), false)
                  .getSmallestIntegerContainer().expanded (2, 2);
    }
    else
    {
        r = getLocalBounds();
    }

    r.setPosition (x + (toRight ? 3 : -(r.getWidth() + 3)), y + (below ? 2 : -(r.getHeight() + 2)));
    setBounds (r);
    parent->addAndMakeVisible (this);
}

void FloatingLabelComponent::paint (Graphics& g)
{
    g.setFont (font);
    g.setColour (Colours::white.withAlpha (0.5f));

    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
            glyphs.draw (g, AffineTransform::translation (1.0f + x, 1.0f + y));

    g.setColour (colour);
    glyphs.draw (g, AffineTransform::translation (1.0f, 1.0f));
}
