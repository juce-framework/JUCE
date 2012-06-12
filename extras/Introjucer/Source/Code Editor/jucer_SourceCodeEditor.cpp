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
                                    CodeDocument& codeDocument,
                                    CodeTokeniser* const codeTokeniser)
    : DocumentEditorComponent (document_),
      editor (codeDocument, codeTokeniser)
{
    addAndMakeVisible (&editor);

   #if JUCE_MAC
    Font font (10.6f);
    font.setTypefaceName ("Menlo");
   #else
    Font font (10.0f);
    font.setTypefaceName (Font::getDefaultMonospacedFontName());
   #endif
    editor.setFont (font);

    editor.setTabSize (4, true);
}

SourceCodeEditor::~SourceCodeEditor()
{
}

void SourceCodeEditor::resized()
{
    editor.setBounds (getLocalBounds());
}

CodeTokeniser* SourceCodeEditor::getTokeniserFor (const File& file)
{
    if (file.hasFileExtension (sourceOrHeaderFileExtensions))
    {
        static CPlusPlusCodeTokeniser cppTokeniser;
        return &cppTokeniser;
    }

    return nullptr;
}

SourceCodeEditor* SourceCodeEditor::createFor (OpenDocumentManager::Document* document,
                                               CodeDocument& codeDocument)
{
    return new SourceCodeEditor (document, codeDocument,
                                 getTokeniserFor (document->getFile()));
}
