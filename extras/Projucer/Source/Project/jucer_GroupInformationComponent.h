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

#include "../Project/jucer_Project.h"

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
class GroupInformationComponent  : public Component,
                                   private ListBoxModel,
                                   private ValueTree::Listener
{
public:
    GroupInformationComponent (const Project::Item& group)
        : item (group),
          header (item.getName(), Icon (getIcons().openFolder, Colours::transparentBlack))
    {
        list.setHeaderComponent (new ListBoxHeader ( { "File", "Binary Resource", "Xcode Resource", "Compile" },
                                                     { 0.4f, 0.2f, 0.2f, 0.2f } ));
        list.setModel (this);
        list.setColour (ListBox::backgroundColourId, Colours::transparentBlack);
        addAndMakeVisible (list);
        list.updateContent();
        list.setRowHeight (30);
        item.state.addListener (this);
        lookAndFeelChanged();

        addAndMakeVisible (header);
    }

    ~GroupInformationComponent()
    {
        item.state.removeListener (this);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.setColour (findColour (secondaryBackgroundColourId));
        g.fillRect (getLocalBounds().reduced (12, 0));
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (12, 0);

        header.setBounds (bounds.removeFromTop (40));
        list.setBounds (bounds.reduced (10, 4));
    }

    void parentSizeChanged() override
    {
        setSize (jmax (550, getParentWidth()), getParentHeight());
    }

    int getNumRows() override
    {
        return item.getNumChildren();
    }

    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour (findColour (rowNumber % 2 == 0 ? widgetBackgroundColourId
                                                    : secondaryWidgetBackgroundColourId));
        g.fillRect (0, 0, width, height - 1);
    }

    Component* refreshComponentForRow (int rowNumber, bool /*isRowSelected*/, Component* existingComponentToUpdate) override
    {
        ScopedPointer<Component> existing (existingComponentToUpdate);

        if (rowNumber < getNumRows())
        {
            Project::Item child (item.getChild (rowNumber));

            if (existingComponentToUpdate == nullptr
                 || dynamic_cast<FileOptionComponent*> (existing.get())->item != child)
            {
                existing = nullptr;
                existing = new FileOptionComponent (child, dynamic_cast<ListBoxHeader*> (list.getHeaderComponent()));
            }
        }

        return existing.release();
    }

    //==============================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override    { itemChanged(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&) override                { itemChanged(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override         { itemChanged(); }
    void valueTreeChildOrderChanged (ValueTree&, int, int) override           { itemChanged(); }
    void valueTreeParentChanged (ValueTree&) override                         { itemChanged(); }

private:
    Project::Item item;
    ListBox list;
    ContentViewHeader header;

    void itemChanged()
    {
        list.updateContent();
        repaint();
    }

    //==============================================================================
    class FileOptionComponent  : public Component
    {
    public:
        FileOptionComponent (const Project::Item& fileItem, ListBoxHeader* listBoxHeader)
            : item (fileItem),
              header (listBoxHeader)
        {
            if (item.isFile())
            {
                addAndMakeVisible (compileButton);
                compileButton.getToggleStateValue().referTo (item.getShouldCompileValue());

                addAndMakeVisible (binaryResourceButton);
                binaryResourceButton.getToggleStateValue().referTo (item.getShouldAddToBinaryResourcesValue());

                addAndMakeVisible (xcodeResourceButton);
                xcodeResourceButton.getToggleStateValue().referTo (item.getShouldAddToXcodeResourcesValue());
            }
        }

        void paint (Graphics& g) override
        {
            if (header != nullptr)
            {
                auto textBounds = getLocalBounds().removeFromLeft (roundToInt (header->getProportionAtIndex (0) * getWidth()));

                auto iconBounds = textBounds.removeFromLeft (25);

                if (item.isImageFile())
                    iconBounds.reduce (5, 5);

                item.getIcon().withColour (findColour (treeIconColourId)).draw (g, iconBounds.toFloat(), item.isIconCrossedOut());

                g.setColour (findColour (widgetTextColourId));

                g.drawText (item.getName(), textBounds, Justification::centredLeft);
            }
        }

        void resized() override
        {
            if (header != nullptr)
            {
                auto bounds = getLocalBounds();
                auto width = getWidth();

                bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (0) * width));

                binaryResourceButton.setBounds (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (1) * width)));
                xcodeResourceButton.setBounds  (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (2) * width)));
                compileButton.setBounds        (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (3) * width)));
            }
        }

        Project::Item item;

    private:
        ListBoxHeader* header;

        ToggleButton compileButton, binaryResourceButton, xcodeResourceButton;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroupInformationComponent)
};
