/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*
  ==============================================================================

   In accordance with the terms of the JUCE 7 End-Use License Agreement, the
   JUCE Code in SECTION A cannot be removed, changed or otherwise rendered
   ineffective unless you have a JUCE Indie or Pro license, or are using JUCE
   under the GPL v3 license.

   End User License Agreement: www.juce.com/juce-7-licence

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

    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
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
