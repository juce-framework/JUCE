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

#ifndef __JUCER_COMPONENTVIEWER_H_56080E43__
#define __JUCER_COMPONENTVIEWER_H_56080E43__

#include "../jucer_DocumentEditorComponent.h"
#include "../../model/Component/jucer_ComponentDocument.h"


//==============================================================================
/**
*/
class ComponentViewer   : public Component,
                          public OpenDocumentManager::DocumentCloseListener,
                          public AsyncUpdater,
                          public ValueTree::Listener
{
public:
    //==============================================================================
    ComponentViewer (OpenDocumentManager::Document* document, Project* project,
                     ComponentDocument* componentDocument);
    ~ComponentViewer();

    //==============================================================================
    void paint (Graphics& g);

    void documentAboutToClose (OpenDocumentManager::Document* closingDoc);

    void valueTreePropertyChanged (ValueTree&, const Identifier&)         { triggerAsyncUpdate(); }
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)   { triggerAsyncUpdate(); }
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)    {}

    void handleAsyncUpdate();

private:
    //==============================================================================
    Project* project;
    OpenDocumentManager::Document* document;
    ComponentDocument* componentDocument;
    ValueTree documentRoot;

    ScopedPointer<ComponentAutoLayoutManager> layoutManager;
    Colour background;

    ComponentViewer (const ComponentViewer&);
    ComponentViewer& operator= (const ComponentViewer&);
};


#endif  // __JUCER_COMPONENTVIEWER_H_56080E43__
