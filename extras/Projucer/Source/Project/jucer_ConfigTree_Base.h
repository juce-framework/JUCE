/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

//==============================================================================
class InfoButton    : public Button
{
public:
    InfoButton (const String& infoToDisplay = String())
        : Button (String())
    {
        if (infoToDisplay.isNotEmpty())
            setInfoToDisplay (infoToDisplay);
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (2);
        const auto& icon = getIcons().info;

        g.setColour (findColour (treeIconColourId).withMultipliedAlpha (isMouseOverButton || isButtonDown ? 1.0f : 0.5f));

        if (isButtonDown)
            g.fillEllipse (bounds);
        else
            g.fillPath (icon, RectanglePlacement (RectanglePlacement::centred)
                              .getTransformToFit (icon.getBounds(), bounds));
    }

    void clicked() override
    {
        auto* w = new InfoWindow (info);
        w->setSize (width, w->getHeight() * numLines + 10);

        CallOutBox::launchAsynchronously (w, getScreenBounds(), nullptr);
    }

    void setInfoToDisplay (const String& infoToDisplay)
    {
        if (infoToDisplay.isNotEmpty())
        {
            info = infoToDisplay;

            auto stringWidth = roundToInt (Font (14.0f).getStringWidthFloat (info));
            width = jmin (300, stringWidth);

            numLines += static_cast<int> (stringWidth / width);
        }
    }

    void setAssociatedComponent (Component* comp)    { associatedComponent = comp; }
    Component* getAssociatedComponent()              { return associatedComponent; }

private:
    String info;
    Component* associatedComponent = nullptr;
    int width;
    int numLines = 1;

    //==============================================================================
    struct InfoWindow    : public Component
    {
        InfoWindow (const String& s)
            : stringToDisplay (s)
        {
            setSize (150, 14);
        }

        void paint (Graphics& g) override
        {
            g.fillAll (findColour (secondaryBackgroundColourId));

            g.setColour (findColour (defaultTextColourId));
            g.setFont (Font (14.0f));
            g.drawFittedText (stringToDisplay, getLocalBounds(), Justification::centred, 10, 1.0f);
        }

        String stringToDisplay;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoButton)
};

//==============================================================================
class PropertyGroupComponent  : public Component
{
public:
    PropertyGroupComponent (String name, Icon icon)
        : header (name, icon)
    {
        addAndMakeVisible (header);
    }

    void setProperties (const PropertyListBuilder& newProps)
    {
        infoButtons.clear();
        properties.clear();
        properties.addArray (newProps.components);

        for (auto i = properties.size(); --i >= 0;)
        {
            auto* prop = properties.getUnchecked (i);

            addAndMakeVisible (prop);

            if (! prop->getTooltip().isEmpty())
            {
                addAndMakeVisible (infoButtons.add (new InfoButton (prop->getTooltip())));
                infoButtons.getLast()->setAssociatedComponent (prop);
                prop->setTooltip (String()); // set the tooltip to empty so it only displays when its button is clicked
            }
        }
    }

    int updateSize (int x, int y, int width)
    {
        header.setBounds (0, 0, width, 40);

        auto height = header.getHeight() + 5;

        for (auto i = 0; i < properties.size(); ++i)
        {
            auto* pp = properties.getUnchecked (i);
            auto propertyHeight = pp->getPreferredHeight() + (getHeightMultiplier (pp) * pp->getPreferredHeight());

            InfoButton* buttonToUse = nullptr;

            for (auto* b : infoButtons)
                if (b->getAssociatedComponent() == pp)
                    buttonToUse = b;

            if (buttonToUse != nullptr)
            {
                buttonToUse->setSize (20, 20);
                buttonToUse->setCentrePosition (20, height + (propertyHeight / 2));
            }

            pp->setBounds (40, height, width - 50, propertyHeight);

            resizePropertyComponent (pp);

            height += pp->getHeight() + 10;
        }

        height += 16;

        setBounds (x, y, width, jmax (height, getParentHeight()));

        return height;
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (secondaryBackgroundColourId));
        g.fillRect (getLocalBounds());
    }

    int getHeightMultiplier (PropertyComponent* pp)
    {
        auto availableTextWidth = ProjucerLookAndFeel::getTextWidthForPropertyComponent (pp);

        auto font = ProjucerLookAndFeel::getPropertyComponentFont();
        auto nameWidth = font.getStringWidthFloat (pp->getName());

        return static_cast<int> (nameWidth / availableTextWidth);
    }

    void resizePropertyComponent (PropertyComponent* pp)
    {
        if (pp->getName() == "Dependencies")
            return;

        for (int i = pp->getNumChildComponents() - 1; i >= 0; --i)
        {
            auto* child = pp->getChildComponent (i);

            auto bounds = child->getBounds();
            child->setBounds (bounds.withSizeKeepingCentre (child->getWidth(), pp->getPreferredHeight()));
        }
    }

    OwnedArray<PropertyComponent> properties;
    OwnedArray<InfoButton> infoButtons;
    ContentViewHeader header;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyGroupComponent)
};

//==============================================================================
class ConfigTreeItemBase  : public JucerTreeViewBase,
                            public ValueTree::Listener
{
public:
    ConfigTreeItemBase() {}

    void showSettingsPage (Component* content)
    {
        content->setComponentID (getUniqueName());

        ScopedPointer<Component> comp (content);

        if (ProjectContentComponent* pcc = getProjectContentComponent())
            pcc->setEditorComponent (comp.release(), nullptr);
    }

    void closeSettingsPage()
    {
        if (auto* pcc = getProjectContentComponent())
        {
            if (auto* content = dynamic_cast<Viewport*> (pcc->getEditorComponent()->getChildComponent (0)))
                if (content->getViewedComponent()->getComponentID() == getUniqueName())
                    pcc->hideEditor();
        }
    }

    void deleteAllSelectedItems() override
    {
        TreeView* const tree = getOwnerView();
        jassert (tree->getNumSelectedItems() <= 1); // multi-select should be disabled

        if (ConfigTreeItemBase* s = dynamic_cast<ConfigTreeItemBase*> (tree->getSelectedItem (0)))
            s->deleteItem();
    }

    void itemOpennessChanged (bool isNowOpen) override
    {
        if (isNowOpen)
           refreshSubItems();
    }

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override {}
    void valueTreeChildAdded (ValueTree&, ValueTree&) override {}
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override {}
    void valueTreeChildOrderChanged (ValueTree&, int, int) override {}
    void valueTreeParentChanged (ValueTree&) override {}

    virtual bool isProjectSettings() const          { return false; }
    virtual bool isModulesList() const              { return false; }

    static void updateSize (Component& comp, PropertyGroupComponent& group)
    {
        const auto width = jmax (550, comp.getParentWidth() - 12);

        auto y = 0;
        y += group.updateSize (12, y, width - 12);

        y = jmax (comp.getParentHeight(), y);

        comp.setSize (width, y);
    }

private:
};
