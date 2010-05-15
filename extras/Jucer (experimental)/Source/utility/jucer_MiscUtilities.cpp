/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../jucer_Headers.h"


//==============================================================================
const int64 hashCode64 (const String& s)
{
    return s.hashCode64() + s.length() * s.hashCode() + s.toUpperCase().hashCode();
}

const String createAlphaNumericUID()
{
    String uid;
    static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    Random r (Random::getSystemRandom().nextInt64());

    for (int i = 9; --i >= 0;)
    {
        r.setSeedRandomly();
        uid << (juce_wchar) chars [r.nextInt (sizeof (chars))];
    }

    return uid;
}

const String randomHexString (Random& random, int numChars)
{
    String s;
    const char hexChars[] = "0123456789ABCDEF";

    while (--numChars >= 0)
        s << hexChars [random.nextInt (16)];

    return s;
}

const String hexString8Digits (int value)
{
    return String::toHexString (value).paddedLeft ('0', 8);
}

const String createGUID (const String& seed)
{
    String guid;
    Random r (hashCode64 (seed + "_jucersalt"));
    guid << "{" << randomHexString (r, 8); // (written as separate statements to enforce the order of execution)
    guid << "-" << randomHexString (r, 4);
    guid << "-" << randomHexString (r, 4);
    guid << "-" << randomHexString (r, 4);
    guid << "-" << randomHexString (r, 12) << "}";
    return guid;
}

//==============================================================================
void autoScrollForMouseEvent (const MouseEvent& e)
{
    Viewport* const viewport = e.eventComponent->findParentComponentOfClass ((Viewport*) 0);

    if (viewport != 0)
    {
        const MouseEvent e2 (e.getEventRelativeTo (viewport));
        viewport->autoScroll (e2.x, e2.y, 8, 16);
    }
}

void drawComponentPlaceholder (Graphics& g, int w, int h, const String& text)
{
    g.fillAll (Colours::white.withAlpha (0.4f));
    g.setColour (Colours::grey);
    g.drawRect (0, 0, w, h);

    g.drawLine (0.5f, 0.5f, w - 0.5f, h - 0.5f);
    g.drawLine (0.5f, h - 0.5f, w - 0.5f, 0.5f);

    g.setColour (Colours::black);
    g.setFont (11.0f);
    g.drawFittedText (text, 2, 2, w - 4, h - 4, Justification::centredTop, 2);
}

void drawRecessedShadows (Graphics& g, int w, int h, int shadowSize)
{
    ColourGradient cg (Colours::black.withAlpha (0.15f), 0, 0,
                       Colours::transparentBlack, 0, (float) shadowSize, false);
    cg.addColour (0.4, Colours::black.withAlpha (0.07f));
    cg.addColour (0.6, Colours::black.withAlpha (0.02f));

    g.setGradientFill (cg);
    g.fillRect (0, 0, w, shadowSize);

    cg.point1.setXY (0.0f, (float) h);
    cg.point2.setXY (0.0f, (float) h - shadowSize);
    g.setGradientFill (cg);
    g.fillRect (0, h - shadowSize, w, shadowSize);

    cg.point1.setXY (0.0f, 0.0f);
    cg.point2.setXY ((float) shadowSize, 0.0f);
    g.setGradientFill (cg);
    g.fillRect (0, 0, shadowSize, h);

    cg.point1.setXY ((float) w, 0.0f);
    cg.point2.setXY ((float) w - shadowSize, 0.0f);
    g.setGradientFill (cg);
    g.fillRect (w - shadowSize, 0, shadowSize, h);
}

//==============================================================================
int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex)
{
    startIndex = jmax (0, startIndex);

    while (startIndex < lines.size())
    {
        if (lines[startIndex].trimStart().startsWithIgnoreCase (text))
            return startIndex;

        ++startIndex;
    }

    return -1;
}


//==============================================================================
PropertyPanelWithTooltips::PropertyPanelWithTooltips()
    : lastComp (0)
{
    addAndMakeVisible (panel = new PropertyPanel());
    startTimer (150);
}

PropertyPanelWithTooltips::~PropertyPanelWithTooltips()
{
    deleteAllChildren();
}

void PropertyPanelWithTooltips::paint (Graphics& g)
{
    g.setColour (Colour::greyLevel (0.15f));
    g.setFont (13.0f);

    TextLayout tl;
    tl.appendText (lastTip, Font (14.0f));
    tl.layout (getWidth() - 10, Justification::left, true); // try to make it look nice
    if (tl.getNumLines() > 3)
        tl.layout (getWidth() - 10, Justification::left, false); // too big, so just squash it in..

    tl.drawWithin (g, 5, panel->getBottom() + 2, getWidth() - 10,
                   getHeight() - panel->getBottom() - 4,
                   Justification::centredLeft);
}

void PropertyPanelWithTooltips::resized()
{
    panel->setBounds (0, 0, getWidth(), jmax (getHeight() - 60, proportionOfHeight (0.6f)));
}

void PropertyPanelWithTooltips::timerCallback()
{
    Component* const newComp = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

    if (newComp != lastComp)
    {
        lastComp = newComp;

        String newTip (findTip (newComp));

        if (newTip != lastTip)
        {
            lastTip = newTip;
            repaint (0, panel->getBottom(), getWidth(), getHeight());
        }
    }
}

const String PropertyPanelWithTooltips::findTip (Component* c)
{
    while (c != 0 && c != this)
    {
        TooltipClient* const tc = dynamic_cast <TooltipClient*> (c);
        if (tc != 0)
        {
            const String tip (tc->getTooltip());

            if (tip.isNotEmpty())
                return tip;
        }

        c = c->getParentComponent();
    }

    return String::empty;
}

//==============================================================================
FloatingLabelComponent::FloatingLabelComponent()
    : font (10.0f)
{
    setInterceptsMouseClicks (false ,false);
}

void FloatingLabelComponent::remove()
{
    if (getParentComponent() != 0)
        getParentComponent()->removeChildComponent (this);
}

void FloatingLabelComponent::update (Component* parent, const String& text, const Colour& textColour, int x, int y, bool toRight, bool below)
{
    colour = textColour;

    Rectangle<int> r;

    if (text != getName())
    {
        setName (text);
        glyphs.clear();
        glyphs.addJustifiedText (font, text, 0, 0, 200.0f, Justification::left);
        glyphs.justifyGlyphs (0, std::numeric_limits<int>::max(), 0, 0, 1000, 1000, Justification::topLeft);

        r = glyphs.getBoundingBox (0, std::numeric_limits<int>::max(), false)
                  .getSmallestIntegerContainer().expanded (2, 2);
    }
    else
    {
        r = getLocalBounds();
    }

    r.setPosition (x + (toRight ? 3 : -(r.getWidth() + 3)), y + (below ? 2 : -(r.getHeight() + 2)));
    setBounds (r);
    parent->addAndMakeVisible (this);
}

void FloatingLabelComponent::paint (Graphics& g)
{
    g.setFont (font);
    g.setColour (Colours::white.withAlpha (0.5f));

    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
            glyphs.draw (g, AffineTransform::translation (1.0f + x, 1.0f + y));

    g.setColour (colour);
    glyphs.draw (g, AffineTransform::translation (1.0f, 1.0f));
}
