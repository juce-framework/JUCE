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

/*
  ==============================================================================

   In accordance with the terms of the JUCE 6 End-Use License Agreement, the
   JUCE Code in SECTION A cannot be removed, changed or otherwise rendered
   ineffective unless you have a JUCE Indie or Pro license, or are using JUCE
   under the GPL v3 license.

   End User License Agreement: www.juce.com/juce-6-licence

  ==============================================================================
*/

// BEGIN SECTION A

namespace juce
{

/**
    The standard JUCE splash screen component.

    @tags{GUI}
*/
class JUCE_API  JUCESplashScreen  : public Component,
                                    private Timer,
                                    private DeletedAtShutdown
{
public:
    JUCESplashScreen (Component& parentToAddTo);

    static std::unique_ptr<Drawable> getSplashScreenLogo();

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;
    void paint (Graphics&) override;
    void timerCallback() override;
    void parentSizeChanged() override;
    void parentHierarchyChanged() override;
    bool hitTest (int, int) override;
    void mouseUp (const MouseEvent&) override;

    std::unique_ptr<Drawable> content;
    ComponentAnimator fader;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JUCESplashScreen)
};

// END SECTION A

} // namespace juce
