/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

//==============================================================================
class InfoButton    : public Button
{
public:
    InfoButton (PropertyComponent& comp)
        : Button (String()),
          associatedComponent (comp)
    {
        tooltip = associatedComponent.getTooltip();
        auto stringWidth = Font (14.0f).getStringWidthFloat (tooltip);

        int maxWidth = 300;

        if (stringWidth > maxWidth)
        {
            width = maxWidth;
            numLines += static_cast<int> (stringWidth / width);
        }
        else
        {
            width = roundToInt (stringWidth);
        }
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
        auto* w = new InfoWindow (tooltip);
        w->setSize (width, w->getHeight() * numLines + 10);

        CallOutBox::launchAsynchronously (w, getScreenBounds(), nullptr);
    }

    PropertyComponent& associatedComponent;

private:
    String tooltip;
    int width;
    int numLines = 1;

    //==============================================================================
    struct InfoWindow    : public Component
    {
        InfoWindow (String s)
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
                addAndMakeVisible (infoButtons.add (new InfoButton (*prop)));
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
                if (&b->associatedComponent == pp)
                    buttonToUse = b;

            if (buttonToUse != nullptr)
            {
                buttonToUse->setSize (20, 20);
                buttonToUse->setCentrePosition (20, height + (propertyHeight / 2));
            }

            pp->setBounds (40, height, width - 50, propertyHeight);

            resizeContentIfChoicePropertyComponent (pp);

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

    void resizeContentIfChoicePropertyComponent (PropertyComponent* pp)
    {
        if (auto* choiceComp = dynamic_cast<ChoicePropertyComponent*> (pp))
        {
            auto* box = choiceComp->getChildComponent (0);
            auto bounds = box->getBounds();

            box->setBounds (bounds.withSizeKeepingCentre (box->getWidth(), pp->getPreferredHeight()));
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
        if (ProjectContentComponent* pcc = getProjectContentComponent())
        {
            if (auto* content = dynamic_cast<Viewport*> (pcc->getEditorComponent()))
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
