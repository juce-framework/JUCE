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

#include "jucer_SourceCodeEditor.h"
#include "../Application/jucer_OpenDocumentManager.h"

//==============================================================================
SourceCodeDocument::SourceCodeDocument (Project* project_, const File& file_)
    : modDetector (file_), project (project_)
{
}

CodeDocument& SourceCodeDocument::getCodeDocument()
{
    if (codeDoc == nullptr)
    {
        codeDoc = new CodeDocument();
        reloadInternal();
    }

    return *codeDoc;
}

Component* SourceCodeDocument::createEditor()
{
    SourceCodeEditor* e = new SourceCodeEditor (this);
    e->createEditor (getCodeDocument());
    applyLastState (*(e->editor));
    return e;
}

void SourceCodeDocument::reloadFromFile()
{
    getCodeDocument();
    reloadInternal();
}

void SourceCodeDocument::reloadInternal()
{
    jassert (codeDoc != nullptr);
    modDetector.updateHash();

    ScopedPointer <InputStream> in (modDetector.getFile().createInputStream());

    if (in != nullptr)
        codeDoc->loadFromStream (*in);
}

bool SourceCodeDocument::save()
{
    TemporaryFile temp (modDetector.getFile());

    {
        FileOutputStream fo (temp.getFile());

        if (! (fo.openedOk() && getCodeDocument().writeToStream (fo)))
            return false;
    }

    if (! temp.overwriteTargetFileWithTemporary())
        return false;

    getCodeDocument().setSavePoint();
    modDetector.updateHash();
    return true;
}

void SourceCodeDocument::updateLastState (CodeEditorComponent& editor)
{
    lastState = new CodeEditorComponent::State (editor);
}

void SourceCodeDocument::applyLastState (CodeEditorComponent& editor) const
{
    if (lastState != nullptr)
        lastState->restoreState (editor);
}

//==============================================================================
SourceCodeEditor::SourceCodeEditor (OpenDocumentManager::Document* document_)
    : DocumentEditorComponent (document_)
{
}

SourceCodeEditor::~SourceCodeEditor()
{
    getAppSettings().appearance.settings.removeListener (this);

    SourceCodeDocument* doc = dynamic_cast <SourceCodeDocument*> (getDocument());

    if (doc != nullptr)
        doc->updateLastState (*editor);
}

void SourceCodeEditor::createEditor (CodeDocument& codeDocument)
{
    if (document->getFile().hasFileExtension (sourceOrHeaderFileExtensions))
        setEditor (new CppCodeEditorComponent (codeDocument));
    else
        setEditor (new CodeEditorComponent (codeDocument, nullptr));
}

void SourceCodeEditor::setEditor (CodeEditorComponent* newEditor)
{
    addAndMakeVisible (editor = newEditor);

    editor->setFont (AppearanceSettings::getDefaultCodeFont());
    editor->setTabSize (4, true);

    updateColourScheme();
    getAppSettings().appearance.settings.addListener (this);
}

void SourceCodeEditor::highlightLine (int lineNum, int characterIndex)
{
    if (lineNum <= editor->getFirstLineOnScreen()
         || lineNum >= editor->getFirstLineOnScreen() + editor->getNumLinesOnScreen() - 1)
    {
        editor->scrollToLine (jmax (0, jmin (lineNum - editor->getNumLinesOnScreen() / 3,
                                             editor->getDocument().getNumLines() - editor->getNumLinesOnScreen())));
    }

    editor->moveCaretTo (CodeDocument::Position (editor->getDocument(), lineNum - 1, characterIndex), false);
}

void SourceCodeEditor::resized()
{
    editor->setBounds (getLocalBounds());
}

void SourceCodeEditor::updateColourScheme()     { getAppSettings().appearance.applyToCodeEditor (*editor); }

void SourceCodeEditor::valueTreePropertyChanged (ValueTree&, const Identifier&)   { updateColourScheme(); }
void SourceCodeEditor::valueTreeChildAdded (ValueTree&, ValueTree&)               { updateColourScheme(); }
void SourceCodeEditor::valueTreeChildRemoved (ValueTree&, ValueTree&)             { updateColourScheme(); }
void SourceCodeEditor::valueTreeChildOrderChanged (ValueTree&)                    { updateColourScheme(); }
void SourceCodeEditor::valueTreeParentChanged (ValueTree&)                        { updateColourScheme(); }
void SourceCodeEditor::valueTreeRedirected (ValueTree&)                           { updateColourScheme(); }


//==============================================================================
namespace CppUtils
{
    static CPlusPlusCodeTokeniser* getCppTokeniser()
    {
        static CPlusPlusCodeTokeniser cppTokeniser;
        return &cppTokeniser;
    }

    static String getLeadingWhitespace (String line)
    {
        line = line.removeCharacters ("\r\n");
        const String::CharPointerType endOfLeadingWS (line.getCharPointer().findEndOfWhitespace());
        return String (line.getCharPointer(), endOfLeadingWS);
    }

    static int getBraceCount (String::CharPointerType line)
    {
        int braces = 0;

        for (;;)
        {
            const juce_wchar c = line.getAndAdvance();

            if (c == 0)                         break;
            else if (c == '{')                  ++braces;
            else if (c == '}')                  --braces;
            else if (c == '/')                  { if (*line == '/') break; }
            else if (c == '"' || c == '\'')     { while (! (line.isEmpty() || line.getAndAdvance() == c)) {} }
        }

        return braces;
    }

    static bool getIndentForCurrentBlock (CodeDocument::Position pos, String& whitespace)
    {
        int braceCount = 0;

        while (pos.getLineNumber() > 0)
        {
            pos = pos.movedByLines (-1);

            const String line (pos.getLineText());
            const String trimmedLine (line.trimStart());

            braceCount += getBraceCount (trimmedLine.getCharPointer());

            if (braceCount > 0)
            {
                whitespace = getLeadingWhitespace (line);
                return true;
            }
        }

        return false;
    }
}

CppCodeEditorComponent::CppCodeEditorComponent (CodeDocument& codeDocument)
    : CodeEditorComponent (codeDocument, CppUtils::getCppTokeniser())
{
}

void CppCodeEditorComponent::handleReturnKey()
{
    CodeEditorComponent::handleReturnKey();

    CodeDocument::Position pos (getCaretPos());

    if (pos.getLineNumber() > 0 && pos.getLineText().trim().isEmpty())
    {
        const String previousLine (pos.movedByLines (-1).getLineText());
        const String trimmedPreviousLine (previousLine.trim());

        if (trimmedPreviousLine.endsWithChar ('{')
             || ((trimmedPreviousLine.startsWith ("if ")
                  || trimmedPreviousLine.startsWith ("for ")
                  || trimmedPreviousLine.startsWith ("while "))
                  && trimmedPreviousLine.endsWithChar (')')))
        {
            const String leadingWhitespace (CppUtils::getLeadingWhitespace (previousLine));
            insertTextAtCaret (leadingWhitespace);
            insertTabAtCaret();
        }
        else
        {
            while (pos.getLineNumber() > 0)
            {
                pos = pos.movedByLines (-1);

                if (pos.getLineText().trimStart().isNotEmpty())
                {
                    insertTextAtCaret (CppUtils::getLeadingWhitespace (pos.getLineText()));
                    break;
                }
            }
        }
    }
}

void CppCodeEditorComponent::insertTextAtCaret (const String& newText)
{
    if (getHighlightedRegion().isEmpty())
    {
        const CodeDocument::Position pos (getCaretPos());

        if ((newText == "{" || newText == "}")
             && pos.getLineNumber() > 0
             && pos.getLineText().trim().isEmpty())
        {
            moveCaretToStartOfLine (true);

            String whitespace;
            if (CppUtils::getIndentForCurrentBlock (pos, whitespace))
            {
                CodeEditorComponent::insertTextAtCaret (whitespace);

                if (newText == "{")
                    insertTabAtCaret();
            }
        }
        else if (newText == getDocument().getNewLineCharacters()
                  && pos.getLineNumber() > 0)
        {
            const String remainderOfLine (pos.getLineText().substring (pos.getIndexInLine()));

            if (remainderOfLine.startsWithChar ('{') || remainderOfLine.startsWithChar ('}'))
            {
                String whitespace;
                if (CppUtils::getIndentForCurrentBlock (pos, whitespace))
                {
                    CodeEditorComponent::insertTextAtCaret (newText + whitespace);

                    if (remainderOfLine.startsWithChar ('{'))
                        insertTabAtCaret();

                    return;
                }
            }
        }
    }

    CodeEditorComponent::insertTextAtCaret (newText);
}
