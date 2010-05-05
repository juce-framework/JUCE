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

#ifndef __JUCER_EDITORPANEL_H_8E192A99__
#define __JUCER_EDITORPANEL_H_8E192A99__


//==============================================================================
class EditorPanelBase  : public Component
{
public:
    EditorPanelBase()
        : infoPanel (0), tree (0), markersVisible (true), snappingEnabled (true)
    {
        addAndMakeVisible (toolbar = new Toolbar());
        toolbar->setStyle (Toolbar::textOnly);

        addAndMakeVisible (viewport = new Viewport());

        addChildComponent (tree = new TreeView());
        tree->setRootItemVisible (true);
        tree->setMultiSelectEnabled (true);
        tree->setDefaultOpenness (true);
        tree->setColour (TreeView::backgroundColourId, Colours::white);
        tree->setIndentSize (15);
    }

    ~EditorPanelBase()
    {
        jassert (infoPanel == 0); // remember to call shutdown()
        deleteAllChildren();
    }

    void initialise (Component* canvas, ToolbarItemFactory& toolbarFactory, TreeViewItem* treeRootItem)
    {
        toolbar->addDefaultItems (toolbarFactory);
        viewport->setViewedComponent (canvas);
        addAndMakeVisible (infoPanel = new InfoPanel (this));
        tree->setRootItem (treeRootItem);
        resized();
    }

    void shutdown()
    {
        tree->deleteRootItem();
        deleteAndZero (infoPanel);
    }

    //==============================================================================
    void showOrHideProperties()
    {
        infoPanel->setVisible (! infoPanel->isVisible());
        resized();
    }

    bool arePropertiesVisible() const        { return infoPanel->isVisible(); }

    void showOrHideTree()
    {
        tree->setVisible (! tree->isVisible());
        resized();
    }

    bool isTreeVisible() const              { return tree->isVisible(); }

    void showOrHideMarkers()
    {
        markersVisible = ! markersVisible;
        commandManager->commandStatusChanged();
    }

    bool areMarkersVisible() const          { return markersVisible; }

    void toggleSnapping()
    {
        snappingEnabled = ! snappingEnabled;
        commandManager->commandStatusChanged();
    }

    bool isSnappingEnabled() const          { return snappingEnabled; }

    //==============================================================================
    virtual SelectedItemSet<String>& getSelection() = 0;
    virtual void getSelectedItemProperties (Array<PropertyComponent*>& newComps) = 0;

    void resized()
    {
        const int toolbarHeight = 22;

        toolbar->setBounds (0, 0, getWidth(), toolbarHeight);

        int contentL = 0, contentR = getWidth();

        if (infoPanel != 0 && infoPanel->isVisible())
        {
            contentR -= 200;
            infoPanel->setBounds (contentR, toolbar->getBottom(), getWidth() - contentR, getHeight() - toolbar->getBottom());
        }

        if (tree->isVisible())
        {
            contentL = 200;
            tree->setBounds (0, toolbar->getBottom(), contentL, getHeight() - toolbar->getBottom());
        }

        viewport->setBounds (contentL, toolbar->getBottom(), contentR - contentL, getHeight() - toolbar->getBottom());
    }

private:
    //==============================================================================
    class InfoPanel  : public Component,
                       public ChangeListener
    {
    public:
        InfoPanel (EditorPanelBase* owner_)
          : owner (owner_)
        {
            setOpaque (true);

            addAndMakeVisible (props = new PropertyPanel());

            owner->getSelection().addChangeListener (this);
        }

        ~InfoPanel()
        {
            owner->getSelection().removeChangeListener (this);
            props->clear();
            deleteAllChildren();
        }

        void changeListenerCallback (void*)
        {
            Array <PropertyComponent*> newComps;
            owner->getSelectedItemProperties (newComps);

            props->clear();
            props->addProperties (newComps);
        }

        void paint (Graphics& g)
        {
            g.fillAll (Colour::greyLevel (0.92f));
        }

        void resized()
        {
            props->setSize (getWidth(), getHeight());
        }

    private:
        EditorPanelBase* owner;
        PropertyPanel* props;
    };

    Toolbar* toolbar;
    Viewport* viewport;
    InfoPanel* infoPanel;
    TreeView* tree;
    bool markersVisible, snappingEnabled;
};


#endif  // __JUCER_EDITORPANEL_H_8E192A99__
