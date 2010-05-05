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


//==============================================================================
class JucerToolbarButton   : public ToolbarItemComponent
{
public:
    //==============================================================================
    JucerToolbarButton (ComponentEditor& editor_, int itemId_, const String& labelText)
        : ToolbarItemComponent (itemId_, labelText, true),
          editor (editor_)
    {
        setClickingTogglesState (false);
    }

    ~JucerToolbarButton()
    {
    }

    //==============================================================================
    bool getToolbarItemSizes (int toolbarDepth, bool isToolbarVertical, int& preferredSize, int& minSize, int& maxSize)
    {
        preferredSize = minSize = maxSize = 50;
        return true;
    }

    void paintButton (Graphics& g, const bool over, const bool down)
    {
        Path p;
        p.addRoundedRectangle (1.5f, 2.5f, getWidth() - 3.0f, getHeight() - 5.0f, 3.0f);

        if (getToggleState())
        {
            g.setColour (Colours::grey.withAlpha (0.5f));
            g.fillPath (p);
        }

        g.setColour (Colours::darkgrey.withAlpha (0.3f));
        g.strokePath (p, PathStrokeType (1.0f));

        g.setFont (11.0f);
        g.setColour (Colours::black.withAlpha ((over || down) ? 1.0f : 0.7f));
        g.drawFittedText (getButtonText(), 2, 2, getWidth() - 4, getHeight() - 4, Justification::centred, 2);
    }

    void paintButtonArea (Graphics& g, int width, int height, bool isMouseOver, bool isMouseDown)
    {
    }

    void contentAreaChanged (const Rectangle<int>& newBounds)
    {
    }

    juce_UseDebuggingNewOperator

protected:
    ComponentEditor& editor;

private:
    JucerToolbarButton (const JucerToolbarButton&);
    JucerToolbarButton& operator= (const JucerToolbarButton&);
};

//==============================================================================
class NewComponentToolbarButton   : public JucerToolbarButton
{
public:
    NewComponentToolbarButton (ComponentEditor& editor_, int itemId_)
        : JucerToolbarButton (editor_, itemId_, "create...")
    {
        setTriggeredOnMouseDown (true);
    }

    void clicked()
    {
        editor.showNewComponentMenu (this);
    }
};

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
    enum DemoToolbarItemIds
    {
        createComponent     = 1,
        showInfo            = 2,
        showComponentTree   = 3,
        showOrHideMarkers   = 4,
        toggleSnapping      = 5
    };

    void getAllToolbarItemIds (Array <int>& ids)
    {
        ids.add (createComponent);
        ids.add (showInfo);
        ids.add (showComponentTree);
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
        ids.add (flexibleSpacerId);
        ids.add (showOrHideMarkers);
        ids.add (toggleSnapping);
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
            case createComponent:   name = "new"; return new NewComponentToolbarButton (editor, createComponent);
            case showInfo:          name = "info"; commandId = CommandIDs::showOrHideProperties; break;
            case showComponentTree: name = "tree"; commandId = CommandIDs::showOrHideTree; break;
            case showOrHideMarkers: name = "markers"; commandId = CommandIDs::showOrHideMarkers; break;
            case toggleSnapping:    name = "snap"; commandId = CommandIDs::toggleSnapping; break;
            default:                jassertfalse; return 0;
        }

        JucerToolbarButton* b = new JucerToolbarButton (editor, itemId, name);
        b->setCommandToTrigger (commandManager, commandId, true);
        return b;
    }

private:
    ComponentEditor& editor;

    ComponentEditorToolbarFactory (const ComponentEditorToolbarFactory&);
    ComponentEditorToolbarFactory& operator= (const ComponentEditorToolbarFactory&);
};


#endif  // __JUCER_COMPONENTEDITORTOOLBAR_H_6B5CA931__
