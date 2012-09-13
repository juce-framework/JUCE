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
        codeDoc->clearUndoHistory();
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
    codeDoc->applyChanges (modDetector.getFile().loadFileAsString());
    codeDoc->setSavePoint();
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
        setEditor (new CppCodeEditorComponent (document->getFile(), codeDocument));
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

void SourceCodeEditor::scrollToKeepRangeOnScreen (const Range<int>& range)
{
    const int space = jmin (10, editor->getNumLinesOnScreen() / 3);
    const CodeDocument::Position start (editor->getDocument(), range.getStart());
    const CodeDocument::Position end   (editor->getDocument(), range.getEnd());

    editor->scrollToKeepLinesOnScreen (Range<int> (start.getLineNumber() - space, end.getLineNumber() + space));
}

void SourceCodeEditor::highlight (const Range<int>& range, bool cursorAtStart)
{
    scrollToKeepRangeOnScreen (range);

    if (cursorAtStart)
    {
        editor->moveCaretTo (CodeDocument::Position (editor->getDocument(), range.getEnd()),   false);
        editor->moveCaretTo (CodeDocument::Position (editor->getDocument(), range.getStart()), true);
    }
    else
    {
        editor->setHighlightedRegion (range);
    }
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
static CPlusPlusCodeTokeniser cppTokeniser;

CppCodeEditorComponent::CppCodeEditorComponent (const File& f, CodeDocument& codeDocument)
    : CodeEditorComponent (codeDocument, &cppTokeniser), file (f)
{
}

void CppCodeEditorComponent::handleReturnKey()
{
    CodeEditorComponent::handleReturnKey();

    CodeDocument::Position pos (getCaretPos());

    String blockIndent, lastLineIndent;
    CodeHelpers::getIndentForCurrentBlock (pos, getTabString (getTabSize()), blockIndent, lastLineIndent);

    const String remainderOfBrokenLine (pos.getLineText());
    const int numLeadingWSChars = CodeHelpers::getLeadingWhitespace (remainderOfBrokenLine).length();

    if (numLeadingWSChars > 0)
        getDocument().deleteSection (pos, pos.movedBy (numLeadingWSChars));

    if (remainderOfBrokenLine.trimStart().startsWithChar ('}'))
        insertTextAtCaret (blockIndent);
    else
        insertTextAtCaret (lastLineIndent);

    const String previousLine (pos.movedByLines (-1).getLineText());
    const String trimmedPreviousLine (previousLine.trim());

    if ((trimmedPreviousLine.startsWith ("if ")
          || trimmedPreviousLine.startsWith ("if(")
          || trimmedPreviousLine.startsWith ("for ")
          || trimmedPreviousLine.startsWith ("for(")
          || trimmedPreviousLine.startsWith ("while(")
          || trimmedPreviousLine.startsWith ("while "))
         && trimmedPreviousLine.endsWithChar (')'))
    {
        insertTabAtCaret();
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

            String blockIndent, lastLineIndent;
            if (CodeHelpers::getIndentForCurrentBlock (pos, getTabString (getTabSize()), blockIndent, lastLineIndent))
            {
                CodeEditorComponent::insertTextAtCaret (blockIndent);

                if (newText == "{")
                    insertTabAtCaret();
            }
        }
    }

    CodeEditorComponent::insertTextAtCaret (newText);
}

enum { showInFinderID = 0x2fe821e3 };

void CppCodeEditorComponent::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    menu.addItem (showInFinderID,
                 #if JUCE_MAC
                  "Reveal " + file.getFileName() + " in Finder");
                 #else
                  "Reveal " + file.getFileName() + " in Explorer");
                 #endif
    menu.addSeparator();

    CodeEditorComponent::addPopupMenuItems (menu, e);
}

void CppCodeEditorComponent::performPopupMenuAction (int menuItemID)
{
    if (menuItemID == showInFinderID)
        file.revealToUser();
    else
        CodeEditorComponent::performPopupMenuAction (menuItemID);
}
