/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCER_COMPONENTDOCUMENT_JUCEHEADER__
#define __JUCER_COMPONENTDOCUMENT_JUCEHEADER__

#include "../jucer_Headers.h"
#include "jucer_Project.h"


//==============================================================================
class ComponentDocument
{
public:
    //==============================================================================
    ComponentDocument (Project* project, const File& cppFile);
    ~ComponentDocument();

    static bool isComponentFile (const File& file);

    bool save();
    bool reload();
    bool hasChangedSinceLastSave();

    typedef SelectedItemSet<uint32> SelectedItems;

    //==============================================================================
    int getNumComponents() const;
    const ValueTree getComponent (int index) const;
    Component* createComponent (int index) const;
    void updateComponent (Component* comp) const;
    bool containsComponent (Component* comp) const;
    const ValueTree getComponentState (Component* comp) const;
    bool isStateForComponent (const ValueTree& storedState, Component* comp) const;

    void addNewComponentMenuItems (PopupMenu& menu) const;
    void performNewComponentMenuItem (int menuResultCode);

    //==============================================================================
    enum ResizeZones
    {
        zoneL = 1,
        zoneR = 2,
        zoneT = 4,
        zoneB = 8
    };

    void beginDrag (const Array<Component*>& items, const MouseEvent& e,
                    const ResizableBorderComponent::Zone& zone);
    void continueDrag (const MouseEvent& e);
    void endDrag (const MouseEvent& e);

    //==============================================================================
    ValueTree& getRoot()                                { return root; }
    UndoManager* getUndoManager() throw()               { return &undoManager; }

private:
    Project* project;
    File cppFile;
    ValueTree root;
    UndoManager undoManager;

    class DragHandler;
    ScopedPointer <DragHandler> dragger;

    void checkRootObject();
    ValueTree getComponentGroup() const;
};


#endif   // __JUCER_COMPONENTDOCUMENT_JUCEHEADER__
