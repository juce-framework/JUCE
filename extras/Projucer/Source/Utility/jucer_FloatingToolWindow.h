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

#ifndef JUCER_FLOATINGTOOLWINDOW_H_INCLUDED
#define JUCER_FLOATINGTOOLWINDOW_H_INCLUDED


//==============================================================================
struct FloatingToolWindow  : public DialogWindow
{
    FloatingToolWindow (const String& title,
                        const String& windowPosPropertyName,
                        Component* content,
                        ScopedPointer<Component>& ownerPointer,
                        int defaultW, int defaultH,
                        int minW, int minH,
                        int maxW, int maxH)
        : DialogWindow (title, Colours::darkgrey, true, true),
          windowPosProperty (windowPosPropertyName),
          owner (ownerPointer)
    {
        setUsingNativeTitleBar (true);
        setResizable (true, true);
        setResizeLimits (minW, minH, maxW, maxH);
        setContentOwned (content, false);

        const String windowState (getGlobalProperties().getValue (windowPosProperty));

        if (windowState.isNotEmpty())
            restoreWindowStateFromString (windowState);
        else
            centreAroundComponent (Component::getCurrentlyFocusedComponent(), defaultW, defaultH);

        setVisible (true);
        owner = this;
    }

    ~FloatingToolWindow()
    {
        getGlobalProperties().setValue (windowPosProperty, getWindowStateAsString());
    }

    void closeButtonPressed() override
    {
        owner = nullptr;
    }

    bool escapeKeyPressed() override
    {
        closeButtonPressed();
        return true;
    }

private:
    String windowPosProperty;
    ScopedPointer<Component>& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FloatingToolWindow)
};


#endif  // JUCER_FLOATINGTOOLWINDOW_H_INCLUDED
