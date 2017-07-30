/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../jucer_Headers.h"
#include "jucer_GeneratedCode.h"
#include "jucer_JucerDocument.h"

//==============================================================================
GeneratedCode::GeneratedCode (const JucerDocument* const doc)
    : document (doc), suffix (0)
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
         && ! (parentClass.startsWith ("public ")
                || parentClass.startsWith ("private ")
                || parentClass.startsWith ("protected ")))
    {
        parentClass = "public " + parentClass;
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
    privateMemberDeclarations
        << "Image " << imageMemberName << ";\n";

    if (resourceName.isNotEmpty())
        constructorCode << imageMemberName << " = ImageCache::getFromMemory ("
                        << resourceName << ", " << resourceName << "Size);\n";
}

StringArray GeneratedCode::getExtraParentClasses() const
{
    StringArray s;

    for (int i = 0; i < callbacks.size(); ++i)
    {
        CallbackMethod* const cm = callbacks.getUnchecked(i);
        s.add (cm->requiredParentClass);
    }

    return s;
}

String GeneratedCode::getCallbackDeclarations() const
{
    String s;

    for (int i = 0; i < callbacks.size(); ++i)
    {
        CallbackMethod* const cm = callbacks.getUnchecked(i);

        s << cm->returnType << " " << cm->prototype << " override;\n";
    }

    return s;
}

String GeneratedCode::getCallbackDefinitions() const
{
    String s;

    for (int i = 0; i < callbacks.size(); ++i)
    {
        CallbackMethod* const cm = callbacks.getUnchecked(i);

        const String userCodeBlockName ("User"
            + CodeHelpers::makeValidIdentifier (cm->prototype.upToFirstOccurrenceOf ("(", false, false),
                                                true, true, false).trim());

        if (userCodeBlockName.isNotEmpty() && cm->hasPrePostUserSections)
        {
            s << cm->returnType << " " << className << "::" << cm->prototype
              << "\n{\n    //[" << userCodeBlockName << "_Pre]\n    //[/" << userCodeBlockName
              << "_Pre]\n\n    "
              << CodeHelpers::indent (cm->content.trim(), 4, false)
              << "\n\n    //[" << userCodeBlockName << "_Post]\n    //[/" << userCodeBlockName
              << "_Post]\n}\n\n";
        }
        else
        {
            s << cm->returnType << " " << className << "::" << cm->prototype
              << "\n{\n    "
              << CodeHelpers::indent (cm->content.trim(), 4, false)
              << "\n}\n\n";
        }
    }

    return s;
}

//==============================================================================
String GeneratedCode::getClassDeclaration() const
{
    StringArray parentClassLines;
    parentClassLines.addTokens (parentClasses, ",", StringRef());
    parentClassLines.addArray (getExtraParentClasses());

    parentClassLines = getCleanedStringArray (parentClassLines);

    if (parentClassLines.contains ("public Button", false))
        parentClassLines.removeString ("public Component", false);

    String r ("class ");
    r << className << "  : ";

    r += parentClassLines.joinIntoString (",\n" + String::repeatedString (" ", r.length()));

    return r;
}

String GeneratedCode::getInitialiserList() const
{
    StringArray inits (initialisers);

    if (parentClassInitialiser.isNotEmpty())
        inits.insert (0, parentClassInitialiser);

    inits = getCleanedStringArray (inits);

    String s;

    if (inits.size() == 0)
        return s;

    s << "    : ";

    for (int i = 0; i < inits.size(); ++i)
    {
        String init (inits[i]);

        while (init.endsWithChar (','))
            init = init.dropLastCharacters (1);

        s << init;

        if (i < inits.size() - 1)
            s << ",\n      ";
        else
            s << "\n";
    }

    return s;
}

static String getIncludeFileCode (const Array<File>& files, const File& targetFile)
{
    String s;

    for (int i = 0; i < files.size(); ++i)
        s << CodeHelpers::createIncludeStatement (files.getReference(i), targetFile) << newLine;

    return s;
}

bool GeneratedCode::shouldUseTransMacro() const noexcept
{
    return document->shouldUseTransMacro();
}

//==============================================================================
static void replaceTemplate (String& text, const String& itemName, const String& value)
{
    for (;;)
    {
        const int index = text.indexOf ("%%" + itemName + "%%");

        if (index < 0)
            break;

        int indentLevel = 0;

        for (int i = index; --i >= 0;)
        {
            if (text[i] == '\n')
                break;

            ++indentLevel;
        }

        text = text.replaceSection (index, itemName.length() + 4,
                                    CodeHelpers::indent (value, indentLevel, false));
    }
}

//==============================================================================
static bool getUserSection (const StringArray& lines, const String& tag, StringArray& resultLines)
{
    const int start = indexOfLineStartingWith (lines, "//[" + tag + "]", 0);

    if (start < 0)
        return false;

    const int end = indexOfLineStartingWith (lines, "//[/" + tag + "]", start + 1);

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
        if (dstLines[i].trimStart().startsWith ("//["))
        {
            String tag (dstLines[i].trimStart().substring (3));
            tag = tag.upToFirstOccurrenceOf ("]", false, false);

            jassert (! tag.startsWithChar ('/'));

            if (! tag.startsWithChar ('/'))
            {
                const int endLine = indexOfLineStartingWith (dstLines,
                                                             "//[/" + tag + "]",
                                                             i + 1);

                if (endLine > i)
                {
                    StringArray sourceLines;

                    if (tag != "UserPaintCustomArguments" && getUserSection (srcLines, tag, sourceLines))
                    {
                        for (int j = endLine - i; --j > 0;)
                            dstLines.remove (i + 1);

                        for (int j = 0; j < sourceLines.size(); ++j)
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

    dest = dstLines.joinIntoString ("\n") + "\n";
}

//==============================================================================
void GeneratedCode::applyToCode (String& code,
                                 const File& targetFile,
                                 const String& oldFileWithUserData,
                                 Project* project) const
{
    replaceTemplate (code, "version", JUCEApplicationBase::getInstance()->getApplicationVersion());
    replaceTemplate (code, "creationTime", Time::getCurrentTime().toString (true, true, true));

    replaceTemplate (code, "className", className);
    replaceTemplate (code, "constructorParams", constructorParams);
    replaceTemplate (code, "initialisers", getInitialiserList());

    replaceTemplate (code, "classDeclaration", getClassDeclaration());
    replaceTemplate (code, "privateMemberDeclarations", privateMemberDeclarations);
    replaceTemplate (code, "publicMemberDeclarations", getCallbackDeclarations() + newLine + publicMemberDeclarations);

    replaceTemplate (code, "methodDefinitions", getCallbackDefinitions());

    File juceHeaderFile = project != nullptr ? project->getAppIncludeFile()
                                             : targetFile.getSiblingFile ("JuceHeader.h");

    replaceTemplate (code, "includeJUCEHeader", CodeHelpers::createIncludeStatement (juceHeaderFile, targetFile));

    replaceTemplate (code, "includeFilesH", getIncludeFileCode (includeFilesH, targetFile));
    replaceTemplate (code, "includeFilesCPP", getIncludeFileCode (includeFilesCPP, targetFile));

    replaceTemplate (code, "constructor", constructorCode);
    replaceTemplate (code, "destructor", destructorCode);

    replaceTemplate (code, "metadata", jucerMetadata);
    replaceTemplate (code, "staticMemberDefinitions", staticMemberDefinitions);

    copyAcrossUserSections (code, oldFileWithUserData);
}
