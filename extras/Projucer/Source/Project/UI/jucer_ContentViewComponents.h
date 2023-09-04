/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../../Utility/UI/PropertyComponents/jucer_LabelPropertyComponent.h"

//==============================================================================
struct ContentViewHeader    : public Component
{
    ContentViewHeader (String headerName, Icon headerIcon)
        : name (headerName), icon (headerIcon)
    {
        setTitle (name);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (contentHeaderBackgroundColourId));

        auto bounds = getLocalBounds().reduced (20, 0);

        icon.withColour (Colours::white).draw (g, bounds.toFloat().removeFromRight (30), false);

        g.setColour (Colours::white);
        g.setFont (Font (18.0f));
        g.drawFittedText (name, bounds, Justification::centredLeft, 1);
    }

    String name;
    Icon icon;
};

//==============================================================================
class ListBoxHeader    : public Component
{
public:
    ListBoxHeader (Array<String> columnHeaders)
    {
        for (auto s : columnHeaders)
        {
            addAndMakeVisible (headers.add (new Label (s, s)));
            widths.add (1.0f / (float) columnHeaders.size());
        }

        setSize (200, 40);
    }

    ListBoxHeader (Array<String> columnHeaders, Array<float> columnWidths)
    {
        jassert (columnHeaders.size() == columnWidths.size());

        auto index = 0;
        for (auto s : columnHeaders)
        {
            addAndMakeVisible (headers.add (new Label (s, s)));
            widths.add (columnWidths.getUnchecked (index++));
        }

        recalculateWidths();

        setSize (200, 40);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto width = bounds.getWidth();

        auto index = 0;
        for (auto h : headers)
        {
            auto headerWidth = roundToInt ((float) width * widths.getUnchecked (index));
            h->setBounds (bounds.removeFromLeft (headerWidth));
            ++index;
        }
    }

    void setColumnHeaderWidth (int index, float proportionOfWidth)
    {
        if (! (isPositiveAndBelow (index, headers.size()) && isPositiveAndNotGreaterThan (proportionOfWidth, 1.0f)))
        {
            jassertfalse;
            return;
        }

        widths.set (index, proportionOfWidth);
        recalculateWidths (index);
    }

    int getColumnX (int index)
    {
        auto prop = 0.0f;
        for (int i = 0; i < index; ++i)
            prop += widths.getUnchecked (i);

        return roundToInt (prop * (float) getWidth());
    }

    float getProportionAtIndex (int index)
    {
        jassert (isPositiveAndBelow (index, widths.size()));
        return widths.getUnchecked (index);
    }

private:
    OwnedArray<Label> headers;
    Array<float> widths;

    void recalculateWidths (int indexToIgnore = -1)
    {
        auto total = 0.0f;

        for (auto w : widths)
            total += w;

        if (approximatelyEqual (total, 1.0f))
            return;

        auto diff = 1.0f - total;
        auto amount = diff / static_cast<float> (indexToIgnore == -1 ? widths.size() : widths.size() - 1);

        for (int i = 0; i < widths.size(); ++i)
        {
            if (i != indexToIgnore)
            {
                auto val = widths.getUnchecked (i);
                widths.set (i, val + amount);
            }
        }
    }
};

//==============================================================================
class InfoButton  : public Button
{
public:
    InfoButton (const String& infoToDisplay = {})
        : Button ({})
    {
        setTitle ("Info");

        if (infoToDisplay.isNotEmpty())
            setInfoToDisplay (infoToDisplay);

        setSize (20, 20);
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (2);
        auto& icon = getIcons().info;

        g.setColour (findColour (treeIconColourId).withMultipliedAlpha (isMouseOverButton || isButtonDown ? 1.0f : 0.5f));

        if (isButtonDown)
            g.fillEllipse (bounds);
        else
            g.fillPath (icon, RectanglePlacement (RectanglePlacement::centred)
                        .getTransformToFit (icon.getBounds(), bounds));
    }

    void clicked() override
    {
        auto w = std::make_unique<InfoWindow> (info);
        w->setSize (width, w->getHeight() * numLines + 10);

        CallOutBox::launchAsynchronously (std::move (w), getScreenBounds(), nullptr);
    }

    using Button::clicked;

    void setInfoToDisplay (const String& infoToDisplay)
    {
        if (infoToDisplay.isNotEmpty())
        {
            info = infoToDisplay;

            auto stringWidth = roundToInt (Font (14.0f).getStringWidthFloat (info));
            width = jmin (300, stringWidth);

            numLines += static_cast<int> (stringWidth / width);

            setHelpText (info);
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
            g.drawFittedText (stringToDisplay, getLocalBounds(), Justification::centred, 15, 0.75f);
        }

        String stringToDisplay;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoButton)
};

//==============================================================================
class PropertyGroupComponent  : public Component,
                                private TextPropertyComponent::Listener
{
public:
    PropertyGroupComponent (String name, Icon icon, String desc = {})
        : header (name, icon),
          description (desc)
    {
        addAndMakeVisible (header);
    }

    void setProperties (const PropertyListBuilder& newProps)
    {
        clearProperties();

        if (description.isNotEmpty())
            properties.push_back (std::make_unique<LabelPropertyComponent> (description, 16, Font (16.0f),
                                                                            Justification::centredLeft));

        for (auto* comp : newProps.components)
            properties.push_back (std::unique_ptr<PropertyComponent> (comp));

        for (auto& prop : properties)
        {
            const auto propertyTooltip = prop->getTooltip();

            if (propertyTooltip.isNotEmpty())
            {
                // set the tooltip to empty so it only displays when its button is clicked
                prop->setTooltip ({});

                auto infoButton = std::make_unique<InfoButton> (propertyTooltip);
                infoButton->setAssociatedComponent (prop.get());

                auto propertyAndInfoWrapper = std::make_unique<PropertyAndInfoWrapper> (*prop, *infoButton.get());
                addAndMakeVisible (propertyAndInfoWrapper.get());
                propertyComponentsWithInfo.push_back (std::move (propertyAndInfoWrapper));

                infoButtons.push_back (std::move (infoButton));
            }
            else
            {
                addAndMakeVisible (prop.get());
            }

            if (auto* multiChoice = dynamic_cast<MultiChoicePropertyComponent*> (prop.get()))
                multiChoice->onHeightChange = [this] { updateSize(); };

            if (auto* text = dynamic_cast<TextPropertyComponent*> (prop.get()))
                if (text->isTextEditorMultiLine())
                    text->addListener (this);
        }
    }

    int updateSize (int x, int y, int width)
    {
        header.setBounds (0, 0, width, headerSize);
        auto height = header.getBottom() + 10;

        for (auto& pp : properties)
        {
            const auto propertyHeight = jmax (pp->getPreferredHeight(), getApproximateLabelHeight (*pp));

            auto iter = std::find_if (propertyComponentsWithInfo.begin(), propertyComponentsWithInfo.end(),
                                      [&pp] (const std::unique_ptr<PropertyAndInfoWrapper>& w) { return &w->propertyComponent == pp.get(); });

            if (iter != propertyComponentsWithInfo.end())
                (*iter)->setBounds (0, height, width - 10, propertyHeight);
            else
                pp->setBounds (40, height, width - 50, propertyHeight);

            if (shouldResizePropertyComponent (pp.get()))
                resizePropertyComponent (pp.get());

            height += pp->getHeight() + 10;
        }

        height += 16;

        setBounds (x, y, width, jmax (height, getParentHeight()));

        return height;
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId));
    }

    const std::vector<std::unique_ptr<PropertyComponent>>& getProperties() const noexcept
    {
        return properties;
    }

    void clearProperties()
    {
        propertyComponentsWithInfo.clear();
        infoButtons.clear();
        properties.clear();
    }

private:
    //==============================================================================
    struct PropertyAndInfoWrapper  : public Component
    {
        PropertyAndInfoWrapper (PropertyComponent& c, InfoButton& i)
            : propertyComponent (c),
              infoButton (i)
        {
            setFocusContainerType (FocusContainerType::focusContainer);
            setTitle (propertyComponent.getName());

            addAndMakeVisible (propertyComponent);
            addAndMakeVisible (infoButton);
        }

        void resized() override
        {
            auto bounds = getLocalBounds();

            bounds.removeFromLeft (40);
            bounds.removeFromRight (10);

            propertyComponent.setBounds (bounds);
            infoButton.setCentrePosition (20, bounds.getHeight() / 2);
        }

        PropertyComponent& propertyComponent;
        InfoButton& infoButton;
    };

    //==============================================================================
    void textPropertyComponentChanged (TextPropertyComponent* comp) override
    {
        auto fontHeight = [comp]
        {
            Label tmpLabel;
            return comp->getLookAndFeel().getLabelFont (tmpLabel).getHeight();
        }();

        auto lines = StringArray::fromLines (comp->getText());

        comp->setPreferredHeight (jmax (100, 10 + roundToInt (fontHeight * (float) lines.size())));

        updateSize();
    }

    void updateSize()
    {
        updateSize (getX(), getY(), getWidth());

        if (auto* parent = getParentComponent())
            parent->parentSizeChanged();
    }

    bool shouldResizePropertyComponent (PropertyComponent* p)
    {
        if (auto* textComp = dynamic_cast<TextPropertyComponent*> (p))
            return ! textComp->isTextEditorMultiLine();

        return (dynamic_cast<ChoicePropertyComponent*>  (p) != nullptr
             || dynamic_cast<ButtonPropertyComponent*>  (p) != nullptr
             || dynamic_cast<BooleanPropertyComponent*> (p) != nullptr);
    }

    void resizePropertyComponent (PropertyComponent* pp)
    {
        for (auto i = pp->getNumChildComponents() - 1; i >= 0; --i)
        {
            auto* child = pp->getChildComponent (i);

            auto bounds = child->getBounds();
            child->setBounds (bounds.withSizeKeepingCentre (child->getWidth(), pp->getPreferredHeight()));
        }
    }

    static int getApproximateLabelHeight (const PropertyComponent& pp)
    {
        auto availableTextWidth = ProjucerLookAndFeel::getTextWidthForPropertyComponent (pp);

        if (availableTextWidth == 0)
            return 0;

        const auto font = ProjucerLookAndFeel::getPropertyComponentFont();
        const auto labelWidth = font.getStringWidthFloat (pp.getName());
        const auto numLines = (int) (labelWidth / (float) availableTextWidth) + 1;
        return (int) std::round ((float) numLines * font.getHeight() * 1.1f);
    }

    //==============================================================================
    static constexpr int headerSize = 40;

    std::vector<std::unique_ptr<PropertyComponent>> properties;
    std::vector<std::unique_ptr<InfoButton>> infoButtons;
    std::vector<std::unique_ptr<PropertyAndInfoWrapper>> propertyComponentsWithInfo;

    ContentViewHeader header;
    String description;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyGroupComponent)
};
