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

#pragma once


//==============================================================================
struct ContentViewHeader    : public Component
{
    ContentViewHeader (String headerName, Icon headerIcon)
        : name (headerName), icon (headerIcon)
    {
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
            widths.add (1.0f / columnHeaders.size());
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
            auto headerWidth = roundToInt (width * widths.getUnchecked (index));
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
        for (auto i = 0; i < index; ++i)
            prop += widths.getUnchecked (i);

        return roundToInt (prop * getWidth());
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

        if (total == 1.0f)
            return;

        auto diff = 1.0f - total;
        auto amount = diff / static_cast<float> (indexToIgnore == -1 ? widths.size() : widths.size() - 1);

        for (auto i = 0; i < widths.size(); ++i)
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
    PropertyGroupComponent (String name, Icon icon, String desc = {})
        : header (name, icon),
          description (desc)
    {
        addAndMakeVisible (header);

        description.setFont (Font (16.0f));
        description.setColour (getLookAndFeel().findColour (defaultTextColourId));
        description.setLineSpacing (5.0f);
        description.setJustification (Justification::centredLeft);
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
        header.setBounds (0, 0, width, headerSize);
        auto height = header.getBottom() + 10;

        descriptionLayout.createLayout (description, (float) (width - 40));
        const auto descriptionHeight = (int) descriptionLayout.getHeight();

        if (descriptionHeight > 0)
            height += (int) descriptionLayout.getHeight() + 25;

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

        auto textArea = getLocalBounds().toFloat()
                                        .withTop ((float) headerSize)
                                        .reduced (20.0f, 10.0f)
                                        .withHeight (descriptionLayout.getHeight());
        descriptionLayout.draw (g, textArea);
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

private:
    OwnedArray<InfoButton> infoButtons;
    ContentViewHeader header;
    AttributedString description;
    TextLayout descriptionLayout;
    const int headerSize = 40;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyGroupComponent)
};
