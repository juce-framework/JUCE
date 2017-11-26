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

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
class OSCLogListBox    : public ListBox, private ListBoxModel, private AsyncUpdater
{
public:

    //==============================================================================
    OSCLogListBox()
    {
        setModel (this);
    }

    //==============================================================================
    ~OSCLogListBox()
    {
    }

    //==============================================================================
    int getNumRows() override
    {
        return oscLogList.size();
    }

    //==============================================================================
    void paintListBoxItem (int row, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        ignoreUnused (rowIsSelected);

        if (isPositiveAndBelow (row, oscLogList.size()))
        {
            g.setColour (Colours::white);

            g.drawText (oscLogList[row],
                        Rectangle<int> (width, height).reduced (4, 0),
                        Justification::centredLeft, true);
        }
    }

    //==============================================================================
    void addOSCMessage (const OSCMessage& message, int level = 0)
    {
        oscLogList.add (getIndentationString (level)
                        + "- osc message, address = '"
                        + message.getAddressPattern().toString()
                        + "', "
                        + String (message.size())
                        + " argument(s)");

        if (! message.isEmpty())
        {
            for (OSCArgument* arg = message.begin(); arg != message.end(); ++arg)
                addOSCMessageArgument (*arg, level + 1);
        }

        triggerAsyncUpdate();
    }

    //==============================================================================
    void addOSCBundle (const OSCBundle& bundle, int level = 0)
    {
        OSCTimeTag timeTag = bundle.getTimeTag();

        oscLogList.add (getIndentationString (level)
                        + "- osc bundle, time tag = "
                        + timeTag.toTime().toString (true, true, true, true));

        for (OSCBundle::Element* element = bundle.begin(); element != bundle.end(); ++element)
        {
            if (element->isMessage())
                addOSCMessage (element->getMessage(), level + 1);
            else if (element->isBundle())
                addOSCBundle (element->getBundle(), level + 1);
        }

        triggerAsyncUpdate();
    }

    //==============================================================================
    void addOSCMessageArgument (const OSCArgument& arg, int level)
    {
        String typeAsString;
        String valueAsString;

        if (arg.isFloat32())
        {
            typeAsString = "float32";
            valueAsString = String (arg.getFloat32());
        }
        else if (arg.isInt32())
        {
            typeAsString = "int32";
            valueAsString = String (arg.getInt32());
        }
        else if (arg.isString())
        {
            typeAsString = "string";
            valueAsString = arg.getString();
        }
        else if (arg.isBlob())
        {
            typeAsString = "blob";
            auto& blob = arg.getBlob();
            valueAsString = String::fromUTF8 ((const char*) blob.getData(), (int) blob.getSize());
        }
        else
        {
            typeAsString = "(unknown)";
        }

        oscLogList.add (getIndentationString (level + 1) + "- " + typeAsString.paddedRight(' ', 12) + valueAsString);
    }

    //==============================================================================
    void addInvalidOSCPacket (const char* /* data */, int dataSize)
    {
        oscLogList.add ("- (" + String(dataSize) + "bytes with invalid format)");
    }

    //==============================================================================
    void clear()
    {
        oscLogList.clear();
        triggerAsyncUpdate();
    }

    //==============================================================================
    void handleAsyncUpdate() override
    {
        updateContent();
        scrollToEnsureRowIsOnscreen (oscLogList.size() - 1);
        repaint();
    }

private:

    //==============================================================================
    String getIndentationString (int level)
    {
        return String().paddedRight (' ', 2 * level);
    }


    //==============================================================================
    StringArray oscLogList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCLogListBox)
};
