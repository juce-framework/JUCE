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

const String createIncludeStatement (const File& includeFile, const File& targetFile)
{
    return "#include \"" + unixStylePath (includeFile.getRelativePathFrom (targetFile.getParentDirectory()))
            + "\"";
}

const String makeHeaderGuardName (const File& file)
{
    return "__" + file.getFileName().toUpperCase()
                                    .replaceCharacters (" .", "__")
                                    .retainCharacters ("_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")
            + "_"
            + String::toHexString (file.hashCode()).toUpperCase()
            + "__";
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

//==============================================================================
const String replaceCEscapeChars (const String& s)
{
    const int len = s.length();

    String r;
    r.preallocateStorage (len + 2);
    bool lastWasHexEscapeCode = false;

    for (int i = 0; i < len; ++i)
    {
        const tchar c = s[i];

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

//==============================================================================
const String makeValidCppIdentifier (String s,
                                     const bool capitalise,
                                     const bool removeColons,
                                     const bool allowTemplates)
{
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

    // make sure it's not a reserved c++ keyword..
    static const char* const reservedWords[] =
    {
        "auto", "const", "double", "float", "int", "short", "struct",
        "return", "static", "union", "while", "asm", "dynamic_cast",
        "unsigned", "break", "continue", "else", "for", "long", "signed",
        "switch", "void", "case", "default", "enum", "goto", "register",
        "sizeof", "typedef", "volatile", "char", "do", "extern", "if",
        "namespace", "reinterpret_cast", "try", "bool", "explicit", "new",
        "static_cast", "typeid", "catch", "false", "operator", "template",
        "typename", "class", "friend", "private", "this", "using", "const_cast",
        "inline", "public", "throw", "virtual", "delete", "mutable", "protected",
        "true", "wchar_t", "and", "bitand", "compl", "not_eq", "or_eq",
        "xor_eq", "and_eq", "bitor", "not", "or", "xor", "cin", "endl",
        "INT_MIN", "iomanip", "main", "npos", "std", "cout", "include",
        "INT_MAX", "iostream", "MAX_RAND", "NULL", "string", "id", "std"
    };

    for (i = 0; i < numElementsInArray (reservedWords); ++i)
        if (n == reservedWords[i])
            n << '_';

    return n;
}

//==============================================================================
const String floatToCode (const float v)
{
    String s ((double) (float) v, 4);

    if (s.containsChar ('.'))
        s << 'f';
    else
        s << ".0f";

    return s;
}

const String doubleToCode (const double v)
{
    String s (v, 7);

    if (! s.containsChar ('.'))
        s << ".0";

    return s;
}

const String boolToCode (const bool b)
{
    return b ? "true" : "false";
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

const String indentCode (const String& code, const int numSpaces)
{
    if (numSpaces == 0)
        return code;

    const String space (String::repeatedString (" ", numSpaces));

    StringArray lines;
    lines.addLines (code);

    for (int i = 1; i < lines.size(); ++i)
    {
        String s (lines[i].trimEnd());
        if (s.isNotEmpty())
            s = space + s;

        lines.set (i, s);
    }

    return lines.joinIntoString (newLine);
}

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
    g.setColour (Colours::white);

    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
            glyphs.draw (g, AffineTransform::translation (1.0f + x, 1.0f + y));

    g.setColour (colour);
    glyphs.draw (g, AffineTransform::translation (1.0f, 1.0f));
}
