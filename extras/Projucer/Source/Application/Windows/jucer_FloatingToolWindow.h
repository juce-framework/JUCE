/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
struct FloatingToolWindow  : public DialogWindow
{
    FloatingToolWindow (const String& title,
                        const String& windowPosPropertyName,
                        Component* content,
                        std::unique_ptr<Component>& ownerPointer,
                        bool shouldBeResizable,
                        int defaultW, int defaultH,
                        int minW, int minH,
                        int maxW, int maxH)
        : DialogWindow (title, content->findColour (secondaryBackgroundColourId), true, true),
          windowPosProperty (windowPosPropertyName),
          owner (ownerPointer)
    {
        setUsingNativeTitleBar (true);
        setResizable (shouldBeResizable, shouldBeResizable);
        setResizeLimits (minW, minH, maxW, maxH);
        setContentOwned (content, false);

        String windowState;
        if (windowPosProperty.isNotEmpty())
            windowState = getGlobalProperties().getValue (windowPosProperty);

        if (windowState.isNotEmpty())
            restoreWindowStateFromString (windowState);
        else
            centreAroundComponent (Component::getCurrentlyFocusedComponent(), defaultW, defaultH);

        setVisible (true);
        owner.reset (this);
    }

    ~FloatingToolWindow() override
    {
        if (windowPosProperty.isNotEmpty())
            getGlobalProperties().setValue (windowPosProperty, getWindowStateAsString());
    }

    void closeButtonPressed() override
    {
        owner.reset();
    }

    bool escapeKeyPressed() override
    {
        closeButtonPressed();
        return true;
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId));
    }

private:
    String windowPosProperty;
    std::unique_ptr<Component>& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FloatingToolWindow)
};
