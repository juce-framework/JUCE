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
