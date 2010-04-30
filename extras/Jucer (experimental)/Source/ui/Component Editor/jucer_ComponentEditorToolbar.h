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
    enum DemoToolbarItemIds
    {
        createComponent     = 1,
        showInfo            = 2,
        showComponentTree   = 3,
    };

    void getAllToolbarItemIds (Array <int>& ids)
    {
        ids.add (createComponent);
        ids.add (showInfo);
        ids.add (showComponentTree);

        ids.add (separatorBarId);
        ids.add (spacerId);
        ids.add (flexibleSpacerId);
    }

    void getDefaultItemSet (Array <int>& ids)
    {
        ids.add (spacerId);
        ids.add (createComponent);
        ids.add (flexibleSpacerId);
        ids.add (showComponentTree);
        ids.add (showInfo);
        ids.add (spacerId);
    }

    ToolbarItemComponent* createItem (int itemId)
    {
        String name;
        int commandId = 0;

        switch (itemId)
        {
            case createComponent:   name = "new"; break;
            case showInfo:          name = "info"; commandId = CommandIDs::showOrHideProperties; break;
            case showComponentTree: name = "tree"; commandId = CommandIDs::showOrHideTree; break;
            default:                jassertfalse; return 0;
        }

        ToolbarButton* b = new ToolbarButton (itemId, name, new DrawablePath(), 0);
        b->setCommandToTrigger (commandManager, commandId, true);
        return b;
    }

private:
    ComponentEditor& editor;
};


#endif  // __JUCER_COMPONENTEDITORTOOLBAR_H_6B5CA931__
