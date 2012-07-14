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

    editor->moveCaretTo (CodeDocument::Position (&editor->getDocument(), lineNum - 1, characterIndex), false);
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
