/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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

//==============================================================================
std::unique_ptr<AccessibilityHandler> GroupComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace juce
