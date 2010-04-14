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
    static const tchar* const reservedWords[] =
    {
        T("auto"), T("const"), T("double"), T("float"), T("int"), T("short"), T("struct"),
        T("return"), T("static"), T("union"), T("while"), T("asm"), T("dynamic_cast"),
        T("unsigned"), T("break"), T("continue"), T("else"), T("for"), T("long"), T("signed"),
        T("switch"), T("void"), T("case"), T("default"), T("enum"), T("goto"), T("register"),
        T("sizeof"), T("typedef"), T("volatile"), T("char"), T("do"), T("extern"), T("if"),
        T("namespace"), T("reinterpret_cast"), T("try"), T("bool"), T("explicit"), T("new"),
        T("static_cast"), T("typeid"), T("catch"), T("false"), T("operator"), T("template"),
        T("typename"), T("class"), T("friend"), T("private"), T("this"), T("using"), T("const_cast"),
        T("inline"), T("public"), T("throw"), T("virtual"), T("delete"), T("mutable"), T("protected"),
        T("true"), T("wchar_t"), T("and"), T("bitand"), T("compl"), T("not_eq"), T("or_eq"),
        T("xor_eq"), T("and_eq"), T("bitor"), T("not"), T("or"), T("xor"), T("cin"), T("endl"),
        T("INT_MIN"), T("iomanip"), T("main"), T("npos"), T("std"), T("cout"), T("include"),
        T("INT_MAX"), T("iostream"), T("MAX_RAND"), T("NULL"), T("string"), T("id")
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

    return lines.joinIntoString ("\n");
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
const char* Coordinate::parentLeftMarkerName   = "parent.left";
const char* Coordinate::parentRightMarkerName  = "parent.right";
const char* Coordinate::parentTopMarkerName    = "parent.top";
const char* Coordinate::parentBottomMarkerName = "parent.bottom";

Coordinate::Coordinate (bool isHorizontal_)
    : value (0), isProportion (false), isHorizontal (isHorizontal_)
{
}

Coordinate::Coordinate (double absoluteDistanceFromOrigin, bool isHorizontal_)
    : value (absoluteDistanceFromOrigin), isProportion (false), isHorizontal (isHorizontal_)
{
}

Coordinate::Coordinate (double absoluteDistance, const String& source, bool isHorizontal_)
    : anchor1 (source), value (absoluteDistance), isProportion (false), isHorizontal (isHorizontal_)
{
}

Coordinate::Coordinate (double relativeProportion, const String& pos1, const String& pos2, bool isHorizontal_)
    : anchor1 (pos1), anchor2 (pos2), value (relativeProportion), isProportion (true), isHorizontal (isHorizontal_)
{
}

Coordinate::~Coordinate()
{
}

const Coordinate Coordinate::getAnchorPoint1() const
{
    return Coordinate (0.0, anchor1, isHorizontal);
}

const Coordinate Coordinate::getAnchorPoint2() const
{
    return Coordinate (0.0, anchor2, isHorizontal);
}

bool Coordinate::isOrigin (const String& name)
{
    return name.isEmpty() || name == parentLeftMarkerName || name == parentTopMarkerName;
}

const String Coordinate::getOriginMarkerName() const
{
    return isHorizontal ? parentLeftMarkerName : parentTopMarkerName;
}

const String Coordinate::getExtentMarkerName() const
{
    return isHorizontal ? parentRightMarkerName : parentBottomMarkerName;
}

const String Coordinate::checkName (const String& name) const
{
    return name.isEmpty() ? getOriginMarkerName() : name;
}

double Coordinate::getPosition (const String& name, MarkerResolver& markerResolver, int recursionCounter) const
{
    if (isOrigin (name))
        return 0.0;

    return markerResolver.findMarker (name, isHorizontal)
                         .resolve (markerResolver, recursionCounter + 1);
}

struct RecursivePositionException
{
};

double Coordinate::resolve (MarkerResolver& markerResolver, int recursionCounter) const
{
    if (recursionCounter > 100)
    {
        jassertfalse
        throw RecursivePositionException();
    }

    const double pos1 = getPosition (anchor1, markerResolver, recursionCounter);

    return isProportion ? pos1 + (getPosition (anchor2, markerResolver, recursionCounter) - pos1) * value
                        : pos1 + value;
}

double Coordinate::resolve (MarkerResolver& markerResolver) const
{
    try
    {
        return resolve (markerResolver, 0);
    }
    catch (RecursivePositionException&)
    {}

    return 0.0;
}

void Coordinate::moveToAbsolute (double newPos, MarkerResolver& markerResolver)
{
    try
    {
        const double pos1 = getPosition (anchor1, markerResolver, 0);

        if (isProportion)
        {
            const double size = getPosition (anchor2, markerResolver, 0) - pos1;

            if (size != 0)
                value = (newPos - pos1) / size;
        }
        else
        {
            value = newPos - pos1;
        }
    }
    catch (RecursivePositionException&)
    {}
}

bool Coordinate::isRecursive (MarkerResolver& markerResolver) const
{
    try
    {
        resolve (markerResolver, 0);
    }
    catch (RecursivePositionException&)
    {
        return true;
    }

    return false;
}

void Coordinate::skipWhitespace (const String& s, int& i)
{
    while (CharacterFunctions::isWhitespace (s[i]))
        ++i;
}

const String Coordinate::readMarkerName (const String& s, int& i)
{
    skipWhitespace (s, i);

    if (CharacterFunctions::isLetter (s[i]) || s[i] == '_')
    {
        int start = i;

        while (CharacterFunctions::isLetterOrDigit (s[i]) || s[i] == '_' || s[i] == '.')
            ++i;

        return s.substring (start, i);
    }

    return String::empty;
}

double Coordinate::readNumber (const String& s, int& i)
{
    skipWhitespace (s, i);

    int start = i;

    if (CharacterFunctions::isDigit (s[i]) || s[i] == '.' || s[i] == '-')
        ++i;

    while (CharacterFunctions::isDigit (s[i]) || s[i] == '.')
        ++i;

    if ((s[i] == 'e' || s[i] == 'E')
         && (CharacterFunctions::isDigit (s[i + 1])
              || s[i + 1] == '-'
              || s[i + 1] == '+'))
    {
        i += 2;

        while (CharacterFunctions::isDigit (s[i]))
            ++i;
    }

    const double value = s.substring (start, i).getDoubleValue();

    while (CharacterFunctions::isWhitespace (s[i]) || s[i] == ',')
        ++i;

    return value;
}

Coordinate::Coordinate (const String& s, bool isHorizontal_)
    : value (0), isProportion (false), isHorizontal (isHorizontal_)
{
    int i = 0;

    anchor1 = readMarkerName (s, i);

    if (anchor1.isNotEmpty())
    {
        skipWhitespace (s, i);

        if (s[i] == '+')
            value = readNumber (s, ++i);
        else if (s[i] == '-')
            value = -readNumber (s, ++i);
    }
    else
    {
        value = readNumber (s, i);
        skipWhitespace (s, i);

        if (s[i] == '%')
        {
            isProportion = true;
            value /= 100.0;
            skipWhitespace (s, ++i);

            if (s[i] == '*')
            {
                anchor1 = readMarkerName (s, ++i);
                skipWhitespace (s, i);

                if (s[i] == '-' && s[i + 1] == '>')
                {
                    i += 2;
                    anchor2 = readMarkerName (s, i);
                }
                else
                {
                    anchor2 = anchor1;
                    anchor1 = getOriginMarkerName();
                }
            }
            else
            {
                anchor1 = getOriginMarkerName();
                anchor2 = getExtentMarkerName();
            }
        }
    }
}

const String Coordinate::toString() const
{
    if (isProportion)
    {
        const String percent (value * 100.0);

        if (isOrigin (anchor1))
        {
            if (anchor2 == parentRightMarkerName || anchor2 == parentBottomMarkerName)
                return percent + "%";
            else
                return percent + "% * " + checkName (anchor2);
        }
        else
            return percent + "% * " + checkName (anchor1) + " -> " + checkName (anchor2);
    }
    else
    {
        if (isOrigin (anchor1))
            return String (value);
        else if (value > 0)
            return checkName (anchor1) + " + " + String (value);
        else if (value < 0)
            return checkName (anchor1) + " - " + String (-value);
        else
            return checkName (anchor1);
    }
}

const double Coordinate::getEditableValue() const
{
    return isProportion ? value * 100.0 : value;
}

void Coordinate::setEditableValue (const double newValue)
{
    value = isProportion ? newValue / 100.0 : newValue;
}

//==============================================================================
RectangleCoordinates::RectangleCoordinates()
    : left (true), right (true), top (false), bottom (false)
{
}

RectangleCoordinates::RectangleCoordinates (const Rectangle<int>& rect)
    : left (rect.getX(), true),
      right (rect.getWidth(), "left", true),
      top (rect.getY(), false),
      bottom (rect.getHeight(), "top", false)
{
}

RectangleCoordinates::RectangleCoordinates (const String& stringVersion)
    : left (true), right (true), top (false), bottom (false)
{
    StringArray tokens;
    tokens.addTokens (stringVersion, ",", String::empty);

    left   = Coordinate (tokens [0], true);
    top    = Coordinate (tokens [1], false);
    right  = Coordinate (tokens [2], true);
    bottom = Coordinate (tokens [3], false);
}

bool RectangleCoordinates::isRecursive (Coordinate::MarkerResolver& markerResolver) const
{
    return left.isRecursive (markerResolver) || right.isRecursive (markerResolver)
              || top.isRecursive (markerResolver) || bottom.isRecursive (markerResolver);
}

const Rectangle<int> RectangleCoordinates::resolve (Coordinate::MarkerResolver& markerResolver) const
{
    const int l = roundToInt (left.resolve (markerResolver));
    const int r = roundToInt (right.resolve (markerResolver));
    const int t = roundToInt (top.resolve (markerResolver));
    const int b = roundToInt (bottom.resolve (markerResolver));

    return Rectangle<int> (l, t, r - l, b - t);
}

void RectangleCoordinates::moveToAbsolute (const Rectangle<int>& newPos, Coordinate::MarkerResolver& markerResolver)
{
    left.moveToAbsolute (newPos.getX(), markerResolver);
    right.moveToAbsolute (newPos.getRight(), markerResolver);
    top.moveToAbsolute (newPos.getY(), markerResolver);
    bottom.moveToAbsolute (newPos.getBottom(), markerResolver);
}

const String RectangleCoordinates::toString() const
{
    return left.toString() + ", " + top.toString() + ", " + right.toString() + ", " + bottom.toString();
}
