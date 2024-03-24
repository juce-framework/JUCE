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

namespace juce
{

static Typeface::Ptr getTypefaceForFontFromLookAndFeel (const Font& font)
{
    return LookAndFeel::getDefaultLookAndFeel().getTypefaceForFont (font);
}

using GetTypefaceForFont = Typeface::Ptr (*)(const Font&);
extern GetTypefaceForFont juce_getTypefaceForFont;

//==============================================================================
LookAndFeel::LookAndFeel()
{
    /* if this fails it means you're trying to create a LookAndFeel object before
       the static Colours have been initialised. That ain't gonna work. It probably
       means that you're using a static LookAndFeel object and that your compiler has
       decided to initialise it before the Colours class.
    */
    jassert (Colours::white == Colour (0xffffffff));

    juce_getTypefaceForFont = getTypefaceForFontFromLookAndFeel;
}

LookAndFeel::~LookAndFeel()
{
    /* This assertion is triggered if you try to delete a LookAndFeel object while something
       is still using it!

       Reasons may be:
         - it's still being used as the default LookAndFeel; or
         - it's set as a Component's current lookandfeel; or
         - there's a WeakReference to it somewhere else in your code

       Generally the fix for this will be to make sure you call
       Component::setLookAndFeel (nullptr) on any components that were still using
       it before you delete it, or call LookAndFeel::setDefaultLookAndFeel (nullptr)
       if you had set it up to be the default one. This assertion can also be avoided by
       declaring your LookAndFeel object before any of the Components that use it as
       the Components will be destroyed before the LookAndFeel.

       Deleting a LookAndFeel is unlikely to cause a crash since most things will use a
       safe WeakReference to it, but it could cause some unexpected graphical behaviour,
       so it's advisable to clear up any references before destroying them!
    */
    jassert (masterReference.getNumActiveWeakReferences() == 0
              || (masterReference.getNumActiveWeakReferences() == 1
                   && this == &getDefaultLookAndFeel()));
}

//==============================================================================
Colour LookAndFeel::findColour (int colourID) const noexcept
{
    const ColourSetting c = { colourID, Colour() };
    auto index = colours.indexOf (c);

    if (index >= 0)
        return colours[index].colour;

    jassertfalse;
    return Colours::black;
}

void LookAndFeel::setColour (int colourID, Colour newColour) noexcept
{
    const ColourSetting c = { colourID, newColour };
    auto index = colours.indexOf (c);

    if (index >= 0)
        colours.getReference (index).colour = newColour;
    else
        colours.add (c);
}

bool LookAndFeel::isColourSpecified (const int colourID) const noexcept
{
    const ColourSetting c = { colourID, Colour() };
    return colours.contains (c);
}

//==============================================================================
LookAndFeel& LookAndFeel::getDefaultLookAndFeel() noexcept
{
    return Desktop::getInstance().getDefaultLookAndFeel();
}

void LookAndFeel::setDefaultLookAndFeel (LookAndFeel* newDefaultLookAndFeel) noexcept
{
    Desktop::getInstance().setDefaultLookAndFeel (newDefaultLookAndFeel);
}

//==============================================================================
Typeface::Ptr LookAndFeel::getTypefaceForFont (const Font& font)
{
    if (font.getTypefaceName() == Font::getDefaultSansSerifFontName())
    {
        if (defaultTypeface != nullptr)
            return defaultTypeface;

        if (defaultSans.isNotEmpty())
        {
            Font f (font);
            f.setTypefaceName (defaultSans);
            return Typeface::createSystemTypefaceFor (f);
        }
    }

    return Font::getDefaultTypefaceForFont (font);
}

void LookAndFeel::setDefaultSansSerifTypeface (Typeface::Ptr newDefaultTypeface)
{
    if (defaultTypeface != newDefaultTypeface)
    {
        defaultTypeface = newDefaultTypeface;
        Typeface::clearTypefaceCache();
    }
}

void LookAndFeel::setDefaultSansSerifTypefaceName (const String& newName)
{
    if (defaultSans != newName)
    {
        defaultTypeface.reset();
        Typeface::clearTypefaceCache();
        defaultSans = newName;
    }
}

//==============================================================================
MouseCursor LookAndFeel::getMouseCursorFor (Component& component)
{
    auto cursor = component.getMouseCursor();

    for (auto* parent = component.getParentComponent();
         parent != nullptr && cursor == MouseCursor::ParentCursor;
         parent = parent->getParentComponent())
    {
        cursor = parent->getMouseCursor();
    }

    return cursor;
}

std::unique_ptr<LowLevelGraphicsContext> LookAndFeel::createGraphicsContext (const Image& imageToRenderOn,
                                                                             Point<int> origin,
                                                                             const RectangleList<int>& initialClip)
{
    return std::make_unique<LowLevelGraphicsSoftwareRenderer> (imageToRenderOn, origin, initialClip);
}

//==============================================================================
void LookAndFeel::setUsingNativeAlertWindows (bool shouldUseNativeAlerts)
{
    useNativeAlertWindows = shouldUseNativeAlerts;
}

bool LookAndFeel::isUsingNativeAlertWindows()
{
   #if JUCE_LINUX || JUCE_BSD
    return false; // not available currently..
   #else
    return useNativeAlertWindows;
   #endif
}

} // namespace juce
