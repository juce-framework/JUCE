/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef __JUCER_CONFIGPAGE_JUCEHEADER__
#define __JUCER_CONFIGPAGE_JUCEHEADER__

#include "jucer_Project.h"
#include "../Utility/jucer_JucerTreeViewBase.h"

//==============================================================================
JucerTreeViewBase* createProjectConfigTreeViewRoot (Project& project);


//==============================================================================
class PropertyGroup  : public Component
{
public:
    PropertyGroup()  {}

    void setProperties (const PropertyListBuilder& newProps)
    {
        properties.clear();
        properties.addArray (newProps.components);

        for (int i = properties.size(); --i >= 0;)
            addAndMakeVisible (properties.getUnchecked(i));
    }

    int updateSize (int x, int y, int width)
    {
        int height = 38;

        for (int i = 0; i < properties.size(); ++i)
        {
            PropertyComponent* pp = properties.getUnchecked(i);
            pp->setBounds (10, height, width - 20, pp->getPreferredHeight());
            height += pp->getHeight();
        }

        height += 16;
        setBounds (x, y, width, height);
        return height;
    }

    void paint (Graphics& g) override
    {
        const Colour bkg (findColour (mainBackgroundColourId));

        g.setColour (Colours::white.withAlpha (0.35f));
        g.fillRect (0, 30, getWidth(), getHeight() - 38);

        g.setFont (Font (15.0f, Font::bold));
        g.setColour (bkg.contrasting (0.7f));
        g.drawFittedText (getName(), 12, 0, getWidth() - 16, 25, Justification::bottomLeft, 1);
    }

    OwnedArray<PropertyComponent> properties;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyGroup)
};

//==============================================================================
class PropertyPanelViewport  : public Component
{
public:
    PropertyPanelViewport (Component* content)
    {
        addAndMakeVisible (&viewport);
        addAndMakeVisible (&rolloverHelp);
        viewport.setViewedComponent (content, true);
    }

    void paint (Graphics& g) override
    {
        IntrojucerLookAndFeel::fillWithBackgroundTexture (*this, g);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds());
        rolloverHelp.setBounds (r.removeFromBottom (70).reduced (10, 0));
        viewport.setBounds (r);
    }

    Viewport viewport;
    RolloverHelpComp rolloverHelp;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyPanelViewport)
};

//==============================================================================
class SettingsTreeViewItemBase  : public JucerTreeViewBase,
                                  public ValueTree::Listener
{
public:
    SettingsTreeViewItemBase() {}

    void showSettingsPage (Component* content);
    void closeSettingsPage();

    void deleteAllSelectedItems() override
    {
        TreeView* const tree = getOwnerView();
        jassert (tree->getNumSelectedItems() <= 1); // multi-select should be disabled

        if (SettingsTreeViewItemBase* s = dynamic_cast <SettingsTreeViewItemBase*> (tree->getSelectedItem (0)))
            s->deleteItem();
    }

    void itemOpennessChanged (bool isNowOpen) override
    {
        if (isNowOpen)
           refreshSubItems();
    }

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override {}
    void valueTreeChildAdded (ValueTree&, ValueTree&) override {}
    void valueTreeChildRemoved (ValueTree&, ValueTree&) override {}
    void valueTreeChildOrderChanged (ValueTree&) override {}
    void valueTreeParentChanged (ValueTree&) override {}

    virtual bool isProjectSettings() const          { return false; }
    virtual bool isModulesList() const              { return false; }

    static void updateSize (Component& comp, PropertyGroup& group)
    {
        const int width = jmax (550, comp.getParentWidth() - 20);

        int y = 0;
        y += group.updateSize (12, y, width - 12);

        comp.setSize (width, y);
    }
};


#endif   // __JUCER_CONFIGPAGE_JUCEHEADER__
