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

#ifndef __JUCER_SOURCECODEEDITOR_JUCEHEADER__
#define __JUCER_SOURCECODEEDITOR_JUCEHEADER__

#include "../Project/jucer_Project.h"
#include "../Application/jucer_DocumentEditorComponent.h"


//==============================================================================
/**
*/
class SourceCodeEditor  : public DocumentEditorComponent
{
public:
    //==============================================================================
    SourceCodeEditor (OpenDocumentManager::Document* document,
                      CodeDocument& codeDocument,
                      CodeTokeniser* const codeTokeniser);

    ~SourceCodeEditor();

    static SourceCodeEditor* createFor (OpenDocumentManager::Document* document,
                                        CodeDocument& codeDocument);

    static CodeTokeniser* getTokeniserFor (const File& file);

    //==============================================================================
    void resized();

    CodeEditorComponent editor;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceCodeEditor);
};


#endif   // __JUCER_SOURCECODEEDITOR_JUCEHEADER__
