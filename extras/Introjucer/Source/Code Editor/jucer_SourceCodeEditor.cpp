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
SourceCodeEditor::SourceCodeEditor (OpenDocumentManager::Document* document_,
                                    CodeDocument& codeDocument)
    : DocumentEditorComponent (document_)
{
    addAndMakeVisible (editor = createEditor (codeDocument));

   #if JUCE_MAC
    Font font (13.0f);
    font.setTypefaceName ("Menlo");
   #else
    Font font (12.0f);
    font.setTypefaceName (Font::getDefaultMonospacedFontName());
   #endif
    editor->setFont (font);

    editor->setTabSize (4, true);

    updateColourScheme();
    getAppSettings().appearance.settings.addListener (this);
}

SourceCodeEditor::~SourceCodeEditor()
{
    getAppSettings().appearance.settings.removeListener (this);
}

CodeEditorComponent* SourceCodeEditor::createEditor (CodeDocument& codeDocument)
{
    if (document->getFile().hasFileExtension (sourceOrHeaderFileExtensions))
        return new CppCodeEditorComponent (codeDocument);

    return new CodeEditorComponent (codeDocument, nullptr);
}

//==============================================================================
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
