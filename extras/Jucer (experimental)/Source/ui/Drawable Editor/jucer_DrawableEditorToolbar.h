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

#ifndef __JUCER_DRAWABLEEDITORTOOLBAR_H_65D23CDD__
#define __JUCER_DRAWABLEEDITORTOOLBAR_H_65D23CDD__


//==============================================================================
class DrawableEditorToolbarFactory  : public ToolbarItemFactory
{
public:
    DrawableEditorToolbarFactory (DrawableEditor& editor_)
        : editor (editor_)
    {
    }

    ~DrawableEditorToolbarFactory()
    {
    }

    //==============================================================================
    enum ToolbarItemIds
    {
        createShape     = 1,
        showInfo,
        showTree,
        showOrHideMarkers,
        toggleSnapping
    };

    void getAllToolbarItemIds (Array <int>& ids)
    {
        ids.add (createShape);
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
        ids.add (createShape);
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
            case createShape:       return new NewShapeToolbarButton (editor, itemId);
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
    class NewShapeToolbarButton   : public JucerToolbarButton
    {
    public:
        NewShapeToolbarButton (DrawableEditor& editor_, int itemId_)
            : JucerToolbarButton (itemId_, "create..."), editor (editor_)
        {
            setTriggeredOnMouseDown (true);
        }

        void clicked()
        {
            editor.showNewShapeMenu (this);
        }

    private:
        DrawableEditor& editor;
    };

private:
    DrawableEditor& editor;

    DrawableEditorToolbarFactory (const DrawableEditorToolbarFactory&);
    DrawableEditorToolbarFactory& operator= (const DrawableEditorToolbarFactory&);
};


#endif  // __JUCER_DRAWABLEEDITORTOOLBAR_H_65D23CDD__
