/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

//==============================================================================
static Typeface::Ptr getTypefaceForFontFromLookAndFeel (const Font& font)
{
    return LookAndFeel::getDefaultLookAndFeel().getTypefaceForFont (font);
}

typedef Typeface::Ptr (*GetTypefaceForFont) (const Font&);
extern GetTypefaceForFont juce_getTypefaceForFont;

//==============================================================================
LookAndFeel::LookAndFeel()
    : useNativeAlertWindows (false)
{
    /* if this fails it means you're trying to create a LookAndFeel object before
       the static Colours have been initialised. That ain't gonna work. It probably
       means that you're using a static LookAndFeel object and that your compiler has
       decided to intialise it before the Colours class.
    */
    jassert (Colours::white == Colour (0xffffffff));

    juce_getTypefaceForFont = getTypefaceForFontFromLookAndFeel;
}

LookAndFeel::~LookAndFeel()
{
    masterReference.clear();
}

//==============================================================================
Colour LookAndFeel::findColour (int colourID) const noexcept
{
    const ColourSetting c = { colourID, Colour() };
    const int index = colours.indexOf (c);

    if (index >= 0)
        return colours.getReference (index).colour;

    jassertfalse;
    return Colours::black;
}

void LookAndFeel::setColour (int colourID, Colour newColour) noexcept
{
    const ColourSetting c = { colourID, newColour };
    const int index = colours.indexOf (c);

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
    if (defaultSans.isNotEmpty() && font.getTypefaceName() == Font::getDefaultSansSerifFontName())
    {
        Font f (font);
        f.setTypefaceName (defaultSans);
        return Typeface::createSystemTypefaceFor (f);
    }

    return Font::getDefaultTypefaceForFont (font);
}

void LookAndFeel::setDefaultSansSerifTypefaceName (const String& newName)
{
    if (defaultSans != newName)
    {
        Typeface::clearTypefaceCache();
        defaultSans = newName;
    }
}

//==============================================================================
MouseCursor LookAndFeel::getMouseCursorFor (Component& component)
{
    MouseCursor m (component.getMouseCursor());

    Component* parent = component.getParentComponent();
    while (parent != nullptr && m == MouseCursor::ParentCursor)
    {
        m = parent->getMouseCursor();
        parent = parent->getParentComponent();
    }

    return m;
}

LowLevelGraphicsContext* LookAndFeel::createGraphicsContext (const Image& imageToRenderOn, const Point<int>& origin,
                                                             const RectangleList<int>& initialClip)
{
    return new LowLevelGraphicsSoftwareRenderer (imageToRenderOn, origin, initialClip);
}

//==============================================================================
void LookAndFeel::setUsingNativeAlertWindows (bool shouldUseNativeAlerts)
{
    useNativeAlertWindows = shouldUseNativeAlerts;
}

bool LookAndFeel::isUsingNativeAlertWindows()
{
   #if JUCE_LINUX
    return false; // not available currently..
   #else
    return useNativeAlertWindows;
   #endif
}
