/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

GroupComponent::GroupComponent (const String& name,
                                const String& labelText)
    : Component (name),
      text (labelText),
      justification (Justification::left)
{
    setInterceptsMouseClicks  (false, true);
}

GroupComponent::~GroupComponent() {}

void GroupComponent::setText (const String& newText)
{
    if (text != newText)
    {
        text = newText;
        repaint();
    }
}

String GroupComponent::getText() const
{
    return text;
}

void GroupComponent::setTextLabelPosition (Justification newJustification)
{
    if (justification != newJustification)
    {
        justification = newJustification;
        repaint();
    }
}

void GroupComponent::paint (Graphics& g)
{
    getLookAndFeel().drawGroupComponentOutline (g, getWidth(), getHeight(),
                                                text, justification, *this);
}

void GroupComponent::enablementChanged()    { repaint(); }
void GroupComponent::colourChanged()        { repaint(); }

} // namespace juce
