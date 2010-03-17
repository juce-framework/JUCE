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
    return path.replaceCharacter (T('\\'), T('/'));
}

const String windowsStylePath (const String& path)
{
    return path.replaceCharacter (T('/'), T('\\'));
}

const String appendPath (const String& path, const String& subpath)
{
    if (File::isAbsolutePath (subpath)
         || subpath.startsWithChar (T('$'))
         || subpath.startsWithChar (T('~'))
         || (CharacterFunctions::isLetter (subpath[0]) && subpath[1] == T(':')))
        return subpath.replaceCharacter (T('\\'), T('/'));

    String path1 (path.replaceCharacter (T('\\'), T('/')));
    if (! path1.endsWithChar (T('/')))
        path1 << '/';

    return path1 + subpath.replaceCharacter (T('\\'), T('/'));
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

    return path1.substring (0, commonBitLength).removeCharacters (T("/:")).isNotEmpty();
}

const String createIncludeStatement (const File& includeFile, const File& targetFile)
{
    return "#include \"" + unixStylePath (includeFile.getRelativePathFrom (targetFile.getParentDirectory()))
            + "\"";
}

const String makeHeaderGuardName (const File& file)
{
    return "__" + file.getFileName().toUpperCase()
                                    .replaceCharacters (T(" ."), T("__"))
                                    .retainCharacters (T("_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"))
            + "_"
            + String::toHexString (file.hashCode()).toUpperCase()
            + "__";
}

//==============================================================================
bool isJuceFolder (const File& folder)
{
    return folder.getFileName().containsIgnoreCase (T("juce"))
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
        case '\t':
            r << T("\\t");
            lastWasHexEscapeCode = false;
            break;
        case '\r':
            r << T("\\r");
            lastWasHexEscapeCode = false;
            break;
        case '\n':
            r <<  T("\\n");
            lastWasHexEscapeCode = false;
            break;
        case '\\':
            r << T("\\\\");
            lastWasHexEscapeCode = false;
            break;
        case '\'':
            r << T("\\\'");
            lastWasHexEscapeCode = false;
            break;
        case '\"':
            r << T("\\\"");
            lastWasHexEscapeCode = false;
            break;

        default:
            if (c < 128 &&
                 ! (lastWasHexEscapeCode
                     && String (T("0123456789abcdefABCDEF")).containsChar (c))) // (have to avoid following a hex escape sequence with a valid hex digit)
            {
                r << c;
                lastWasHexEscapeCode = false;
            }
            else
            {
                lastWasHexEscapeCode = true;
                r << T("\\x") << String::toHexString ((int) c);
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
        s = s.replaceCharacters (T(".,;:/@"), T("______"));
    else
        s = s.replaceCharacters (T(".,;/@"), T("_____"));

    int i;
    for (i = s.length(); --i > 0;)
        if (CharacterFunctions::isLetter (s[i])
             && CharacterFunctions::isLetter (s[i - 1])
             && CharacterFunctions::isUpperCase (s[i])
             && ! CharacterFunctions::isUpperCase (s[i - 1]))
            s = s.substring (0, i) + T(" ") + s.substring (i);

    String allowedChars (T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_ 0123456789"));
    if (allowTemplates)
        allowedChars += T("<>");

    if (! removeColons)
        allowedChars += T(":");

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
        n = T("_") + n;

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

    if (s.containsChar (T('.')))
        s << 'f';
    else
        s << ".0f";

    return s;
}

const String doubleToCode (const double v)
{
    String s (v, 7);

    if (! s.containsChar (T('.')))
        s << ".0";

    return s;
}

const String boolToCode (const bool b)
{
    return b ? T("true") : T("false");
}

const String colourToCode (const Colour& col)
{
    #define COL(col)  Colours::col,

    const Colour colours[] =
    {
        #include "jucer_Colours.h"
        Colours::transparentBlack
    };

    #undef COL
    #define COL(col)  #col,
    static const char* colourNames[] =
    {
        #include "jucer_Colours.h"
        0
    };
    #undef COL

    for (int i = 0; i < numElementsInArray (colourNames) - 1; ++i)
        if (col == colours[i])
            return T("Colours::") + String (colourNames[i]);

    return T("Colour (0x") + hexString8Digits ((int) col.getARGB()) + T(')');
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

    return T("Justification (") + String (justification.getFlags()) + T(")");
}

const String castToFloat (const String& expression)
{
    if (expression.containsOnly (T("0123456789.f")))
    {
        String s (expression.getFloatValue());

        if (s.containsChar (T('.')))
            return s + T("f");

        return s + T(".0f");
    }

    return T("(float) (") + expression + T(")");
}

const String indentCode (const String& code, const int numSpaces)
{
    if (numSpaces == 0)
        return code;

    const String space (String::repeatedString (T(" "), numSpaces));

    StringArray lines;
    lines.addLines (code);

    for (int i = 1; i < lines.size(); ++i)
    {
        String s (lines[i].trimEnd());
        if (s.isNotEmpty())
            s = space + s;

        lines.set (i, s);
    }

    return lines.joinIntoString (T("\n"));
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
