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
    font.setTypefaceName ("Andale Mono");
#else
    Font font (10.0f);
    font.setTypefaceName (Font::getDefaultMonospacedFontName());
#endif
    editor.setFont (font);
}

SourceCodeEditor::~SourceCodeEditor()
{
}

void SourceCodeEditor::resized()
{
    editor.setBounds (0, 0, getWidth(), getHeight());
}

bool SourceCodeEditor::isTextFile (const File& file)
{
    return file.hasFileExtension ("cpp;h;hpp;mm;m;c;cc;cxx;txt;xml;plist;rtf;html;htm;php;py;rb;cs");
}

bool SourceCodeEditor::isCppFile (const File& file)
{
    return file.hasFileExtension (sourceOrHeaderFileExtensions);
}
