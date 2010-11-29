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

#ifndef __JUCER_PROJECTCONTENTCOMPONENT_JUCEHEADER__
#define __JUCER_PROJECTCONTENTCOMPONENT_JUCEHEADER__

#include "jucer_Project.h"
#include "../Application/jucer_OpenDocumentManager.h"


//==============================================================================
/**
*/
class ProjectContentComponent  : public Component,
                                 public ApplicationCommandTarget,
                                 public ChangeListener,
                                 public StretchableLayoutManager
{
public:
    //==============================================================================
    ProjectContentComponent();
    ~ProjectContentComponent();

    void paint (Graphics& g);
    void resized();

    void setProject (Project* project);

    bool showEditorForFile (const File& f);
    bool showDocument (OpenDocumentManager::Document* doc);
    void hideDocument (OpenDocumentManager::Document* doc);
    bool setEditorComponent (Component* editor, OpenDocumentManager::Document* doc);

    void updateMissingFileStatuses();

    void changeListenerCallback (ChangeBroadcaster*);
    void hasBeenMoved();

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget();
    void getAllCommands (Array <CommandID>& commands);
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result);
    bool isCommandActive (const CommandID commandID);
    bool perform (const InvocationInfo& info);

private:
    ScopedPointer<TreeView> projectTree;
    Project* project;
    ScopedPointer <Component> contentView;
    OpenDocumentManager::Document* currentDocument;

    ScopedPointer<StretchableLayoutResizerBar> resizerBar;

    void updateMainWindowTitle();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectContentComponent);
};


#endif   // __JUCER_PROJECTCONTENTCOMPONENT_JUCEHEADER__
