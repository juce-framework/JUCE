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

#ifndef __JUCER_COMPONENTEDITORTOOLBAR_H_6B5CA931__
#define __JUCER_COMPONENTEDITORTOOLBAR_H_6B5CA931__

#include "../../utility/jucer_ColourEditorComponent.h"


//==============================================================================
class ComponentEditorToolbarFactory  : public ToolbarItemFactory
{
public:
    ComponentEditorToolbarFactory (ComponentEditor& editor_)
        : editor (editor_)
    {
    }

    ~ComponentEditorToolbarFactory()
    {
    }

    //==============================================================================
    enum ToolbarItemIds
    {
        createComponent     = 1,
        changeBackground,
        showInfo,
        showTree,
        showOrHideMarkers,
        toggleSnapping
    };

    void getAllToolbarItemIds (Array <int>& ids)
    {
        ids.add (createComponent);
        ids.add (changeBackground);
        ids.add (showInfo);
        ids.add (showTree);
        ids.add (showOrHideMarkers);
        ids.add (toggleSnapping);

        ids.add (separatorBarId);
        ids.add (spacerId);
        ids.add (flexibleSpacerId);
    }

    void getDefaultItemSet (Array <int>& ids)
    {
        ids.add (spacerId);
        ids.add (createComponent);
        ids.add (changeBackground);
        ids.add (flexibleSpacerId);
        ids.add (showOrHideMarkers);
        ids.add (toggleSnapping);
        ids.add (flexibleSpacerId);
        ids.add (showTree);
        ids.add (showInfo);
        ids.add (spacerId);
    }

    ToolbarItemComponent* createItem (int itemId)
    {
        String name;
        int commandId = 0;

        switch (itemId)
        {
            case createComponent:   return new NewComponentToolbarButton (editor, itemId);
            case changeBackground:  return new BackgroundColourToolbarButton (editor, itemId);
            case showInfo:          name = "info"; commandId = CommandIDs::showOrHideProperties; break;
            case showTree:          name = "tree"; commandId = CommandIDs::showOrHideTree; break;
            case showOrHideMarkers: name = "markers"; commandId = CommandIDs::showOrHideMarkers; break;
            case toggleSnapping:    name = "snap"; commandId = CommandIDs::toggleSnapping; break;
            default:                jassertfalse; return 0;
        }

        JucerToolbarButton* b = new JucerToolbarButton (itemId, name);
        b->setCommandToTrigger (commandManager, commandId, true);
        return b;
    }

    //==============================================================================
    class NewComponentToolbarButton   : public JucerToolbarButton
    {
    public:
        NewComponentToolbarButton (ComponentEditor& editor_, int itemId_)
            : JucerToolbarButton (itemId_, "create..."), editor (editor_)
        {
            setTriggeredOnMouseDown (true);
        }

        void clicked()
        {
            editor.showNewComponentMenu (this);
        }

    private:
        ComponentEditor& editor;
    };

    //==============================================================================
    class BackgroundColourToolbarButton   : public JucerToolbarButton
    {
    public:
        BackgroundColourToolbarButton (ComponentEditor& editor_, int itemId_)
            : JucerToolbarButton (itemId_, "background"), editor (editor_)
        {
            setTriggeredOnMouseDown (true);
        }

        void clicked()
        {
            editor.getDocument().getUndoManager()->beginNewTransaction();
            PopupColourSelector::showAt (this, editor.getDocument().getBackgroundColour(), Colours::white, true);
        }

    private:
        ComponentEditor& editor;
    };

private:
    ComponentEditor& editor;

    ComponentEditorToolbarFactory (const ComponentEditorToolbarFactory&);
    ComponentEditorToolbarFactory& operator= (const ComponentEditorToolbarFactory&);
};


#endif  // __JUCER_COMPONENTEDITORTOOLBAR_H_6B5CA931__
