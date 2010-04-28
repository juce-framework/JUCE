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

#include "jucer_CodeGenerator.h"


//==============================================================================
CodeGenerator::CodeGenerator()
{
}

CodeGenerator::~CodeGenerator()
{    
}

int CodeGenerator::getUniqueSuffix()
{
    return ++suffix;
}

//==============================================================================
String& CodeGenerator::getCallbackCode (const String& requiredParentClass,
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

void CodeGenerator::removeCallback (const String& returnType, const String& prototype)
{
    for (int i = callbacks.size(); --i >= 0;)
    {
        CallbackMethod* const cm = callbacks.getUnchecked(i);

        if (cm->returnType == returnType && cm->prototype == prototype)
            callbacks.remove (i);
    }
}

const StringArray CodeGenerator::getExtraParentClasses() const
{
    StringArray s;

    for (int i = 0; i < callbacks.size(); ++i)
    {
        CallbackMethod* const cm = callbacks.getUnchecked(i);
        s.add (cm->requiredParentClass);
    }

    return s;
}

const String CodeGenerator::getCallbackDeclarations() const
{
    String s;

    for (int i = 0; i < callbacks.size(); ++i)
    {
        CallbackMethod* const cm = callbacks.getUnchecked(i);

        s << cm->returnType << " " << cm->prototype << ";" << newLine;
    }

    return s;
}

const String CodeGenerator::getCallbackDefinitions() const
{
    String s;

    for (int i = 0; i < callbacks.size(); ++i)
    {
        CallbackMethod* const cm = callbacks.getUnchecked(i);

        const String userCodeBlockName ("User" + makeValidCppIdentifier (cm->prototype.upToFirstOccurrenceOf ("(", false, false),
                                                                         true, true, false).trim());

        if (userCodeBlockName.isNotEmpty() && cm->hasPrePostUserSections)
        {
            s << cm->returnType << " " << className << "::" << cm->prototype
              << newLine 
              << "{" << newLine 
              << "    //[" << userCodeBlockName << "_Pre]" << newLine
              << "    //[/" << userCodeBlockName
              << "_Pre]" << newLine << newLine 
              << "    " << indentCode (cm->content.trim(), 4) << newLine 
              << newLine 
              << "    //[" << userCodeBlockName << "_Post]" << newLine
              << "    //[/" << userCodeBlockName << "_Post]" << newLine
              << "}" << newLine 
              << newLine;
        }
        else
        {
            s << cm->returnType << " " << className << "::" << cm->prototype << newLine
              << "{" << newLine
              << "    "  << indentCode (cm->content.trim(), 4) << newLine
              << "}" << newLine
              << newLine;
        }
    }

    return s;
}

//==============================================================================
const String CodeGenerator::getClassDeclaration() const
{
    StringArray parentClassLines;
    parentClassLines.addTokens (parentClasses, ",", String::empty);
    parentClassLines.addArray (getExtraParentClasses());

    parentClassLines.trim();
    parentClassLines.removeEmptyStrings();
    parentClassLines.removeDuplicates (false);

    if (parentClassLines.contains ("public Button", false))
        parentClassLines.removeString ("public Component", false);

    String r;
    r << "class " << className << "  : ";
    r << parentClassLines.joinIntoString ("," + String (newLine) + String::repeatedString (" ", r.length()));
    return r;
}

const String CodeGenerator::getInitialiserList() const
{
    String s;
    StringArray inits (memberInitialisers);

    if (parentClassInitialiser.isNotEmpty())
        inits.insert (0, parentClassInitialiser);

    inits.trim();
    inits.removeEmptyStrings();
    inits.removeDuplicates (false);

    if (inits.size() > 0)
    {
        s << "    : ";

        for (int i = 0; i < inits.size(); ++i)
        {
            String init (inits[i]);

            while (init.endsWithChar (','))
                init = init.dropLastCharacters (1);

            s << init;

            if (i < inits.size() - 1)
                s << "," << newLine << "      ";
            else
                s << newLine;
        }
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
        s << "#include \"" << files[i] << "\"" << newLine;

    return s;
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
                                    indentCode (value, indentLevel));
    }
}

//==============================================================================
void CodeGenerator::applyToCode (String& code, const File& targetFile, 
                                 bool isForPreview, Project* project) const
{
    replaceTemplate (code, "juceVersion", SystemStats::getJUCEVersion());
    replaceTemplate (code, "headerGuard", makeHeaderGuardName (targetFile));

    replaceTemplate (code, "className", className);
    replaceTemplate (code, "constructorParams", constructorParams);
    replaceTemplate (code, "initialisers", getInitialiserList());

    replaceTemplate (code, "classDeclaration", getClassDeclaration());
    replaceTemplate (code, "privateMemberDeclarations", privateMemberDeclarations);
    replaceTemplate (code, "publicMemberDeclarations", getCallbackDeclarations() + newLine + publicMemberDeclarations);

    replaceTemplate (code, "methodDefinitions", getCallbackDefinitions());

    if (project != 0)
        replaceTemplate (code, "defaultJuceInclude", createIncludeStatement (project->getAppIncludeFile(), targetFile));
    else
        replaceTemplate (code, "defaultJuceInclude", "#include \"juce_amalgamated.h\"");

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
        replaceTemplate (code, "metadata", "  << Metadata isn't shown in the code preview >>" + String (newLine));
        replaceTemplate (code, "staticMemberDefinitions", "// Static member declarations and resources would go here... (these aren't shown in the code preview)");
    }
}


//==============================================================================
CodeGenerator::CustomCodeList::Iterator::Iterator (const String& documentText, CustomCodeList& customCode_)
    : customCode (customCode_), i (0), codeDocument (0)
{
    lines.addLines (documentText);
    
}

CodeGenerator::CustomCodeList::Iterator::~Iterator()
{
}

bool CodeGenerator::CustomCodeList::Iterator::next()
{
    textBefore = String::empty;
    textAfter = String::empty;
    
    while (i < lines.size())
    {
        textBefore += lines[i] + "\n";

        if (lines[i].trimStart().startsWith ("//["))
        {
            String tag (lines[i].trimStart().substring (3));
            tag = tag.upToFirstOccurrenceOf ("]", false, false).trim();

            if (! (tag.isEmpty() || tag.startsWithChar ('/')))
            {
                const int endLine = indexOfLineStartingWith (lines, "//[/" + tag + "]", i + 1);

                if (endLine > i)
                {
                    sectionName = tag;
                    codeDocument = customCode.getDocumentFor (tag, true);
                    i = endLine;

                    bool isLastTag = true;
                    for (int j = i + 1; j < lines.size(); ++j)
                    {
                        if (lines[j].trimStart().startsWith ("//["))
                        {
                            isLastTag = false;
                            break;
                        }
                    }

                    if (isLastTag)
                    {
                        textAfter = lines.joinIntoString (newLine, i, lines.size() - i);
                        i = lines.size();
                    }

                    return true;
                }
            }
        }
        
        ++i;
    }
    
    return false;
}


//==============================================================================
CodeGenerator::CustomCodeList::CustomCodeList()
{
}

CodeGenerator::CustomCodeList::~CustomCodeList()
{
}

void CodeGenerator::CustomCodeList::reloadFrom (const String& fileContent)
{
    sectionNames.clear();
    sectionContent.clear();
    
    StringArray lines;
    lines.addLines (fileContent);

    for (int i = 0; i < lines.size(); ++i)
    {
        if (lines[i].trimStart().startsWith ("//["))
        {
            String tag (lines[i].trimStart().substring (3));
            tag = tag.upToFirstOccurrenceOf ("]", false, false).trim();

            jassert (! (tag.isEmpty() || tag.startsWithChar ('/')));

            if (! (tag.isEmpty() || tag.startsWithChar ('/')))
            {
                const int endLine = indexOfLineStartingWith (lines, "//[/" + tag + "]", i + 1);

                if (endLine > i)
                {
                    String content (lines.joinIntoString (newLine, i + 1, endLine - i - 1));
                    
                    sectionNames.add (tag);
                    
                    CodeDocumentRef::Ptr doc (new CodeDocumentRef (new CodeDocument()));
                    sectionContent.add (doc);
                    
                    doc->getDocument().replaceAllContent (content);
                    doc->getDocument().clearUndoHistory();
                    doc->getDocument().setSavePoint();

                    i = endLine;
                }
            }
        }
    }
    
    sendSynchronousChangeMessage (this);
}

void CodeGenerator::CustomCodeList::applyTo (String& fileContent) const
{
    StringArray lines;
    lines.addLines (fileContent);

    for (int i = 0; i < lines.size(); ++i)
    {
        if (lines[i].trimStart().startsWith ("//["))
        {
            String tag (lines[i].trimStart().substring (3));
            tag = tag.upToFirstOccurrenceOf ("]", false, false);

            jassert (! tag.startsWithChar ('/'));

            if (! tag.startsWithChar ('/'))
            {
                const int endLine = indexOfLineStartingWith (lines, "//[/" + tag + "]", i + 1);

                if (endLine > i)
                {
                    StringArray sourceLines;
                    sourceLines.addLines (getSectionContent (tag));

                    if (sourceLines.size() > 0)
                    {
                        lines.removeRange (i + 1, endLine - i - 1);

                        for (int j = 0; j < sourceLines.size(); ++j)
                            lines.insert (++i, sourceLines [j].trimEnd());

                        ++i;
                    }
                    else
                    {
                        i = endLine;
                    }
                }
            }
        }

        lines.set (i, lines[i].trimEnd());
    }

    if (lines[lines.size() - 1].isNotEmpty())
        lines.add (String::empty);

    fileContent = lines.joinIntoString (newLine);
}

bool CodeGenerator::CustomCodeList::needsSaving() const
{
    for (int i = sectionContent.size(); --i >= 0;)
        if (sectionContent.getUnchecked(i)->getDocument().hasChangedSinceSavePoint())
            return true;

    return false;
}

int CodeGenerator::CustomCodeList::getNumSections() const
{
    return sectionNames.size();
}

const String CodeGenerator::CustomCodeList::getSectionName (int index) const
{
    return sectionNames [index];
}

const CodeGenerator::CustomCodeList::CodeDocumentRef::Ptr CodeGenerator::CustomCodeList::getDocument (int index) const
{
    return sectionContent [index];
}

const CodeGenerator::CustomCodeList::CodeDocumentRef::Ptr CodeGenerator::CustomCodeList::getDocumentFor (const String& sectionName, bool createIfNotFound)
{
    const int index = sectionNames.indexOf (sectionName);

    if (index >= 0)
        return sectionContent [index];

    if (createIfNotFound)
    {
        sectionNames.add (sectionName);

        const CodeDocumentRef::Ptr doc (new CodeDocumentRef (new CodeDocument()));
        sectionContent.add (doc);
        return doc;
    }

    return 0;
}

const String CodeGenerator::CustomCodeList::getSectionContent (const String& sectionName) const
{
    const int index = sectionNames.indexOf (sectionName);

    if (index >= 0)
        return sectionContent[index]->getDocument().getAllContent();

    return String::empty;
}

void CodeGenerator::CustomCodeList::removeSection (const String& sectionName)
{
    const int index = sectionNames.indexOf (sectionName);

    if (index >= 0)
    {
        sectionNames.remove (index);
        sectionContent.remove (index);
    }
}

