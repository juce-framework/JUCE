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
#include "jucer_GeneratedCode.h"


//==============================================================================
GeneratedCode::GeneratedCode (const JucerDocument* const document_)
    : document (document_),
      suffix (0)
{
}

GeneratedCode::~GeneratedCode()
{
}

int GeneratedCode::getUniqueSuffix()
{
    return ++suffix;
}

//==============================================================================
String& GeneratedCode::getCallbackCode (const String& requiredParentClass,
                                        const String& returnType,
                                        const String& prototype,
                                        const bool hasPrePostUserSections)
{
    String parentClass (requiredParentClass);
    if (parentClass.isNotEmpty()
         && ! (parentClass.startsWith (T("public "))
                || parentClass.startsWith (T("private "))
                || parentClass.startsWith (T("protected "))))
    {
        parentClass = T("public ") + parentClass;
    }

    for (int i = callbacks.size(); --i >= 0;)
    {
        CallbackMethod* const cm = callbacks.getUnchecked(i);

        if (cm->requiredParentClass == parentClass
             && cm->returnType == returnType
             && cm->prototype == prototype)
            return cm->content;
    }

    CallbackMethod* const cm = new CallbackMethod();
    callbacks.add (cm);

    cm->requiredParentClass = parentClass;
    cm->returnType = returnType;
    cm->prototype = prototype;
    cm->hasPrePostUserSections = hasPrePostUserSections;
    return cm->content;
}

void GeneratedCode::removeCallback (const String& returnType, const String& prototype)
{
    for (int i = callbacks.size(); --i >= 0;)
    {
        CallbackMethod* const cm = callbacks.getUnchecked(i);

        if (cm->returnType == returnType && cm->prototype == prototype)
            callbacks.remove (i);
    }
}

void GeneratedCode::addImageResourceLoader (const String& imageMemberName, const String& resourceName)
{
    const String initialiser (imageMemberName + T(" (0)"));

    if (! initialisers.contains (initialiser, false))
    {
        initialisers.add (initialiser);

        privateMemberDeclarations
            << "Image* " << imageMemberName << ";\n";

        if (resourceName.isNotEmpty())
        {
            constructorCode
                << imageMemberName << " = ImageCache::getFromMemory ("
                << resourceName << ", " << resourceName << "Size);\n";

            destructorCode
                << "ImageCache::release (" << imageMemberName << ");\n";
        }
    }
}

const StringArray GeneratedCode::getExtraParentClasses() const
{
    StringArray s;

    for (int i = 0; i < callbacks.size(); ++i)
    {
        CallbackMethod* const cm = callbacks.getUnchecked(i);
        s.add (cm->requiredParentClass);
    }

    return s;
}

const String GeneratedCode::getCallbackDeclarations() const
{
    String s;

    for (int i = 0; i < callbacks.size(); ++i)
    {
        CallbackMethod* const cm = callbacks.getUnchecked(i);

        s << cm->returnType << " " << cm->prototype << ";\n";
    }

    return s;
}

const String GeneratedCode::getCallbackDefinitions() const
{
    String s;

    for (int i = 0; i < callbacks.size(); ++i)
    {
        CallbackMethod* const cm = callbacks.getUnchecked(i);

        const String userCodeBlockName (T("User")
            + makeValidCppIdentifier (cm->prototype.upToFirstOccurrenceOf (T("("), false, false),
                                      true, true, false).trim());

        if (userCodeBlockName.isNotEmpty() && cm->hasPrePostUserSections)
        {
            s << cm->returnType << " " << className << "::" << cm->prototype
              << "\n{\n    //[" << userCodeBlockName << "_Pre]\n    //[/" << userCodeBlockName
              << "_Pre]\n\n    "
              << indentCode (cm->content.trim(), 4)
              << "\n\n    //[" << userCodeBlockName << "_Post]\n    //[/" << userCodeBlockName
              << "_Post]\n}\n\n";
        }
        else
        {
            s << cm->returnType << " " << className << "::" << cm->prototype
              << "\n{\n    "
              << indentCode (cm->content.trim(), 4)
              << "\n}\n\n";
        }
    }

    return s;
}

//==============================================================================
const String GeneratedCode::getClassDeclaration() const
{
    StringArray parentClassLines;
    parentClassLines.addTokens (parentClasses, T(","), 0);
    parentClassLines.addArray (getExtraParentClasses());

    parentClassLines.trim();
    parentClassLines.removeEmptyStrings();
    parentClassLines.removeDuplicates (false);

    if (parentClassLines.contains (T("public Button"), false))
        parentClassLines.removeString (("public Component"), false);

    String r (T("class "));
    r << className << T("  : ");

    r += parentClassLines.joinIntoString (T(",\n") + String::repeatedString (T(" "), r.length()));

    return r;
}

const String GeneratedCode::getInitialiserList() const
{
    StringArray inits (initialisers);

    if (parentClassInitialiser.isNotEmpty())
        inits.insert (0, parentClassInitialiser);

    inits.trim();
    inits.removeEmptyStrings();
    inits.removeDuplicates (false);

    String s;

    if (inits.size() == 0)
        return s;

    s << "    : ";

    for (int i = 0; i < inits.size(); ++i)
    {
        String init (inits[i]);

        while (init.endsWithChar (T(',')))
            init = init.dropLastCharacters (1);

        s << init;

        if (i < inits.size() - 1)
            s << ",\n      ";
        else
            s << "\n";
    }

    return s;
}

static const String getIncludeFileCode (StringArray files)
{
    files.trim();
    files.removeEmptyStrings();
    files.removeDuplicates (false);

    String s;

    for (int i = 0; i < files.size(); ++i)
        s << T("#include \"") << files[i] << T("\"\n");

    return s;
}

//==============================================================================
static void replaceTemplate (String& text, const String& itemName, const String& value)
{
    for (;;)
    {
        const int index = text.indexOf (T("%%") + itemName + T("%%"));

        if (index < 0)
            break;

        int indentLevel = 0;

        for (int i = index; --i >= 0;)
        {
            if (text[i] == T('\n'))
                break;

            ++indentLevel;
        }

        text = text.replaceSection (index, itemName.length() + 4,
                                    indentCode (value, indentLevel));
    }
}

//==============================================================================
static bool getUserSection (const StringArray& lines, const String& tag, StringArray& resultLines)
{
    const int start = indexOfLineStartingWith (lines, T("//[") + tag + T("]"), 0);

    if (start < 0)
        return false;

    const int end = indexOfLineStartingWith (lines, T("//[/") + tag + T("]"), start + 1);

    for (int i = start + 1; i < end; ++i)
        resultLines.add (lines [i]);

    return true;
}

static void copyAcrossUserSections (String& dest, const String& src)
{
    StringArray srcLines, dstLines;
    srcLines.addLines (src);
    dstLines.addLines (dest);

    for (int i = 0; i < dstLines.size(); ++i)
    {
        if (dstLines[i].trimStart().startsWith (T("//[")))
        {
            String tag (dstLines[i].trimStart().substring (3));
            tag = tag.upToFirstOccurrenceOf (T("]"), false, false);

            jassert (! tag.startsWithChar (T('/')));

            if (! tag.startsWithChar (T('/')))
            {
                const int endLine = indexOfLineStartingWith (dstLines,
                                                             T("//[/") + tag + T("]"),
                                                             i + 1);

                if (endLine > i)
                {
                    StringArray sourceLines;

                    if (getUserSection (srcLines, tag, sourceLines))
                    {
                        int j;
                        for (j = endLine - i; --j > 0;)
                            dstLines.remove (i + 1);

                        for (j = 0; j < sourceLines.size(); ++j)
                            dstLines.insert (++i, sourceLines [j].trimEnd());

                        ++i;
                    }
                    else
                    {
                        i = endLine;
                    }
                }
            }
        }

        dstLines.set (i, dstLines[i].trimEnd());
    }

    dest = dstLines.joinIntoString (T("\n")) + T("\n");
}

//==============================================================================
void GeneratedCode::applyToCode (String& code,
                                 const String& fileNameRoot,
                                 const bool isForPreview,
                                 const String& oldFileWithUserData) const
{
    // header guard..
    String headerGuard ("__JUCER_HEADER_");
    headerGuard << className.toUpperCase().retainCharacters (T("ABCDEFGHIJKLMNOPQRSTUVWXYZ"))
                << "_" << fileNameRoot.toUpperCase().retainCharacters (T("ABCDEFGHIJKLMNOPQRSTUVWXYZ"))
                << "_" << String::toHexString (Random::getSystemRandom().nextInt()).toUpperCase()
                << "__";
    replaceTemplate (code, "headerGuard", headerGuard);

    replaceTemplate (code, "creationTime", Time::getCurrentTime().toString (true, true, true));

    replaceTemplate (code, "className", className);
    replaceTemplate (code, "constructorParams", constructorParams);
    replaceTemplate (code, "initialisers", getInitialiserList());

    replaceTemplate (code, "classDeclaration", getClassDeclaration());
    replaceTemplate (code, "privateMemberDeclarations", privateMemberDeclarations);
    replaceTemplate (code, "publicMemberDeclarations", getCallbackDeclarations() + "\n" + publicMemberDeclarations);

    replaceTemplate (code, "methodDefinitions", getCallbackDefinitions());

    replaceTemplate (code, "includeFilesH", getIncludeFileCode (includeFilesH));
    replaceTemplate (code, "includeFilesCPP", getIncludeFileCode (includeFilesCPP));

    replaceTemplate (code, "constructor", constructorCode);
    replaceTemplate (code, "destructor", destructorCode);

    if (! isForPreview)
    {
        replaceTemplate (code, "metadata", jucerMetadata);
        replaceTemplate (code, "staticMemberDefinitions", staticMemberDefinitions);
    }
    else
    {
        replaceTemplate (code, "metadata", T("  << Metadata isn't shown in the code preview >>\n"));
        replaceTemplate (code, "staticMemberDefinitions", T("// Static member declarations and resources would go here... (these aren't shown in the code preview)"));
    }

    copyAcrossUserSections (code, oldFileWithUserData);
}
