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

class ColourSelector::ColourComponentSlider  : public Slider
{
public:
    ColourComponentSlider (const String& name)
        : Slider (name)
    {
        setRange (0.0, 255.0, 1.0);
    }

    String getTextFromValue (double value)
    {
        return String::toHexString ((int) value).toUpperCase().paddedLeft ('0', 2);
    }

    double getValueFromText (const String& text)
    {
        return (double) text.getHexValue32();
    }

private:
    JUCE_DECLARE_NON_COPYABLE (ColourComponentSlider)
};

//==============================================================================
class ColourSelector::ColourSpaceMarker  : public Component
{
public:
    ColourSpaceMarker()
    {
        setInterceptsMouseClicks (false, false);
    }

    void paint (Graphics& g) override
    {
        g.setColour (Colour::greyLevel (0.1f));
        g.drawEllipse (1.0f, 1.0f, getWidth() - 2.0f, getHeight() - 2.0f, 1.0f);
        g.setColour (Colour::greyLevel (0.9f));
        g.drawEllipse (2.0f, 2.0f, getWidth() - 4.0f, getHeight() - 4.0f, 1.0f);
    }

private:
    JUCE_DECLARE_NON_COPYABLE (ColourSpaceMarker)
};

//==============================================================================
class ColourSelector::ColourSpaceView  : public Component
{
public:
    ColourSpaceView (ColourSelector& cs, float& hue, float& sat, float& val, const int edgeSize)
        : owner (cs), h (hue), s (sat), v (val), lastHue (0.0f), edge (edgeSize)
    {
        addAndMakeVisible (marker);
        setMouseCursor (MouseCursor::CrosshairCursor);
    }

    void paint (Graphics& g) override
    {
        if (colours.isNull())
        {
            const int width = getWidth() / 2;
            const int height = getHeight() / 2;
            colours = Image (Image::RGB, width, height, false);

            Image::BitmapData pixels (colours, Image::BitmapData::writeOnly);

            for (int y = 0; y < height; ++y)
            {
                const float val = 1.0f - y / (float) height;

                for (int x = 0; x < width; ++x)
                {
                    const float sat = x / (float) width;
                    pixels.setPixelColour (x, y, Colour (h, sat, val, 1.0f));
                }
            }
        }

        g.setOpacity (1.0f);
        g.drawImageTransformed (colours,
                                RectanglePlacement (RectanglePlacement::stretchToFit)
                                    .getTransformToFit (colours.getBounds().toFloat(),
                                                        getLocalBounds().reduced (edge).toFloat()),
                                false);
    }

    void mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        const float sat = (e.x - edge) / (float) (getWidth() - edge * 2);
        const float val = 1.0f - (e.y - edge) / (float) (getHeight() - edge * 2);

        owner.setSV (sat, val);
    }

    void updateIfNeeded()
    {
        if (lastHue != h)
        {
            lastHue = h;
            colours = Image();
            repaint();
        }

        updateMarker();
    }

    void resized() override
    {
        colours = Image();
        updateMarker();
    }

private:
    ColourSelector& owner;
    float& h;
    float& s;
    float& v;
    float lastHue;
    ColourSpaceMarker marker;
    const int edge;
    Image colours;

    void updateMarker()
    {
        marker.setBounds (roundToInt ((getWidth() - edge * 2) * s),
                          roundToInt ((getHeight() - edge * 2) * (1.0f - v)),
                          edge * 2, edge * 2);
    }

    JUCE_DECLARE_NON_COPYABLE (ColourSpaceView)
};

//==============================================================================
class ColourSelector::HueSelectorMarker  : public Component
{
public:
    HueSelectorMarker()
    {
        setInterceptsMouseClicks (false, false);
    }

    void paint (Graphics& g) override
    {
        const float cw = (float) getWidth();
        const float ch = (float) getHeight();

        Path p;
        p.addTriangle (1.0f, 1.0f,
                       cw * 0.3f, ch * 0.5f,
                       1.0f, ch - 1.0f);

        p.addTriangle (cw - 1.0f, 1.0f,
                       cw * 0.7f, ch * 0.5f,
                       cw - 1.0f, ch - 1.0f);

        g.setColour (Colours::white.withAlpha (0.75f));
        g.fillPath (p);

        g.setColour (Colours::black.withAlpha (0.75f));
        g.strokePath (p, PathStrokeType (1.2f));
    }

private:
    JUCE_DECLARE_NON_COPYABLE (HueSelectorMarker)
};

//==============================================================================
class ColourSelector::HueSelectorComp  : public Component
{
public:
    HueSelectorComp (ColourSelector& cs, float& hue, const int edgeSize)
        : owner (cs), h (hue), edge (edgeSize)
    {
        addAndMakeVisible (marker);
    }

    void paint (Graphics& g) override
    {
        ColourGradient cg;
        cg.isRadial = false;
        cg.point1.setXY (0.0f, (float) edge);
        cg.point2.setXY (0.0f, (float) (getHeight() - edge));

        for (float i = 0.0f; i <= 1.0f; i += 0.02f)
            cg.addColour (i, Colour (i, 1.0f, 1.0f, 1.0f));

        g.setGradientFill (cg);
        g.fillRect (getLocalBounds().reduced (edge));
    }

    void resized() override
    {
        marker.setBounds (0, roundToInt ((getHeight() - edge * 2) * h), getWidth(), edge * 2);
    }

    void mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        owner.setHue ((e.y - edge) / (float) (getHeight() - edge * 2));
    }

    void updateIfNeeded()
    {
        resized();
    }

private:
    ColourSelector& owner;
    float& h;
    HueSelectorMarker marker;
    const int edge;

    JUCE_DECLARE_NON_COPYABLE (HueSelectorComp)
};

//==============================================================================
class ColourSelector::SwatchComponent   : public Component
{
public:
    SwatchComponent (ColourSelector& cs, int itemIndex)
        : owner (cs), index (itemIndex)
    {
    }

    void paint (Graphics& g) override
    {
        const Colour c (owner.getSwatchColour (index));

        g.fillCheckerBoard (getLocalBounds(), 6, 6,
                            Colour (0xffdddddd).overlaidWith (c),
                            Colour (0xffffffff).overlaidWith (c));
    }

    void mouseDown (const MouseEvent&) override
    {
        PopupMenu m;
        m.addItem (1, TRANS("Use this swatch as the current colour"));
        m.addSeparator();
        m.addItem (2, TRANS("Set this swatch to the current colour"));

        m.showMenuAsync (PopupMenu::Options().withTargetComponent (this),
                         ModalCallbackFunction::forComponent (menuStaticCallback, this));
    }

private:
    ColourSelector& owner;
    const int index;

    static void menuStaticCallback (int result, SwatchComponent* comp)
    {
        if (comp != nullptr)
        {
            if (result == 1)
                comp->setColourFromSwatch();
            else if (result == 2)
                comp->setSwatchFromColour();
        }
    }

    void setColourFromSwatch()
    {
        owner.setCurrentColour (owner.getSwatchColour (index));
    }

    void setSwatchFromColour()
    {
        if (owner.getSwatchColour (index) != owner.getCurrentColour())
        {
            owner.setSwatchColour (index, owner.getCurrentColour());
            repaint();
        }
    }

    JUCE_DECLARE_NON_COPYABLE (SwatchComponent)
};

//==============================================================================
ColourSelector::ColourSelector (const int sectionsToShow, const int edge, const int gapAroundColourSpaceComponent)
    : colour (Colours::white),
      flags (sectionsToShow),
      edgeGap (edge)
{
    // not much point having a selector with no components in it!
    jassert ((flags & (showColourAtTop | showSliders | showColourspace)) != 0);

    updateHSV();

    if ((flags & showSliders) != 0)
    {
        addAndMakeVisible (sliders[0] = new ColourComponentSlider (TRANS ("red")));
        addAndMakeVisible (sliders[1] = new ColourComponentSlider (TRANS ("green")));
        addAndMakeVisible (sliders[2] = new ColourComponentSlider (TRANS ("blue")));
        addChildComponent (sliders[3] = new ColourComponentSlider (TRANS ("alpha")));

        sliders[3]->setVisible ((flags & showAlphaChannel) != 0);

        for (int i = 4; --i >= 0;)
            sliders[i]->addListener (this);
    }

    if ((flags & showColourspace) != 0)
    {
        addAndMakeVisible (colourSpace = new ColourSpaceView (*this, h, s, v, gapAroundColourSpaceComponent));
        addAndMakeVisible (hueSelector = new HueSelectorComp (*this, h,  gapAroundColourSpaceComponent));
    }

    update();
}

ColourSelector::~ColourSelector()
{
    dispatchPendingMessages();
    swatchComponents.clear();
}

//==============================================================================
Colour ColourSelector::getCurrentColour() const
{
    return ((flags & showAlphaChannel) != 0) ? colour : colour.withAlpha ((uint8) 0xff);
}

void ColourSelector::setCurrentColour (Colour c)
{
    if (c != colour)
    {
        colour = ((flags & showAlphaChannel) != 0) ? c : c.withAlpha ((uint8) 0xff);

        updateHSV();
        update();
    }
}

void ColourSelector::setHue (float newH)
{
    newH = jlimit (0.0f, 1.0f, newH);

    if (h != newH)
    {
        h = newH;
        colour = Colour (h, s, v, colour.getFloatAlpha());
        update();
    }
}

void ColourSelector::setSV (float newS, float newV)
{
    newS = jlimit (0.0f, 1.0f, newS);
    newV = jlimit (0.0f, 1.0f, newV);

    if (s != newS || v != newV)
    {
        s = newS;
        v = newV;
        colour = Colour (h, s, v, colour.getFloatAlpha());
        update();
    }
}

//==============================================================================
void ColourSelector::updateHSV()
{
    colour.getHSB (h, s, v);
}

void ColourSelector::update()
{
    if (sliders[0] != nullptr)
    {
        sliders[0]->setValue ((int) colour.getRed());
        sliders[1]->setValue ((int) colour.getGreen());
        sliders[2]->setValue ((int) colour.getBlue());
        sliders[3]->setValue ((int) colour.getAlpha());
    }

    if (colourSpace != nullptr)
    {
        colourSpace->updateIfNeeded();
        hueSelector->updateIfNeeded();
    }

    if ((flags & showColourAtTop) != 0)
        repaint (previewArea);

    sendChangeMessage();
}

//==============================================================================
void ColourSelector::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));

    if ((flags & showColourAtTop) != 0)
    {
        const Colour currentColour (getCurrentColour());

        g.fillCheckerBoard (previewArea, 10, 10,
                            Colour (0xffdddddd).overlaidWith (currentColour),
                            Colour (0xffffffff).overlaidWith (currentColour));

        g.setColour (Colours::white.overlaidWith (currentColour).contrasting());
        g.setFont (Font (14.0f, Font::bold));
        g.drawText (currentColour.toDisplayString ((flags & showAlphaChannel) != 0),
                    previewArea, Justification::centred, false);
    }

    if ((flags & showSliders) != 0)
    {
        g.setColour (findColour (labelTextColourId));
        g.setFont (11.0f);

        for (int i = 4; --i >= 0;)
        {
            if (sliders[i]->isVisible())
                g.drawText (sliders[i]->getName() + ":",
                            0, sliders[i]->getY(),
                            sliders[i]->getX() - 8, sliders[i]->getHeight(),
                            Justification::centredRight, false);
        }
    }
}

void ColourSelector::resized()
{
    const int swatchesPerRow = 8;
    const int swatchHeight = 22;

    const int numSliders = ((flags & showAlphaChannel) != 0) ? 4 : 3;
    const int numSwatches = getNumSwatches();

    const int swatchSpace = numSwatches > 0 ? edgeGap + swatchHeight * ((numSwatches + 7) / swatchesPerRow) : 0;
    const int sliderSpace = ((flags & showSliders) != 0)  ? jmin (22 * numSliders + edgeGap, proportionOfHeight (0.3f)) : 0;
    const int topSpace = ((flags & showColourAtTop) != 0) ? jmin (30 + edgeGap * 2, proportionOfHeight (0.2f)) : edgeGap;

    previewArea.setBounds (edgeGap, edgeGap, getWidth() - edgeGap * 2, topSpace - edgeGap * 2);

    int y = topSpace;

    if ((flags & showColourspace) != 0)
    {
        const int hueWidth = jmin (50, proportionOfWidth (0.15f));

        colourSpace->setBounds (edgeGap, y,
                                getWidth() - hueWidth - edgeGap - 4,
                                getHeight() - topSpace - sliderSpace - swatchSpace - edgeGap);

        hueSelector->setBounds (colourSpace->getRight() + 4, y,
                                getWidth() - edgeGap - (colourSpace->getRight() + 4),
                                colourSpace->getHeight());

        y = getHeight() - sliderSpace - swatchSpace - edgeGap;
    }

    if ((flags & showSliders) != 0)
    {
        const int sliderHeight = jmax (4, sliderSpace / numSliders);

        for (int i = 0; i < numSliders; ++i)
        {
            sliders[i]->setBounds (proportionOfWidth (0.2f), y,
                                   proportionOfWidth (0.72f), sliderHeight - 2);

            y += sliderHeight;
        }
    }

    if (numSwatches > 0)
    {
        const int startX = 8;
        const int xGap = 4;
        const int yGap = 4;
        const int swatchWidth = (getWidth() - startX * 2) / swatchesPerRow;
        y += edgeGap;

        if (swatchComponents.size() != numSwatches)
        {
            swatchComponents.clear();

            for (int i = 0; i < numSwatches; ++i)
            {
                SwatchComponent* const sc = new SwatchComponent (*this, i);
                swatchComponents.add (sc);
                addAndMakeVisible (sc);
            }
        }

        int x = startX;

        for (int i = 0; i < swatchComponents.size(); ++i)
        {
            SwatchComponent* const sc = swatchComponents.getUnchecked(i);

            sc->setBounds (x + xGap / 2,
                           y + yGap / 2,
                           swatchWidth - xGap,
                           swatchHeight - yGap);

            if (((i + 1) % swatchesPerRow) == 0)
            {
                x = startX;
                y += swatchHeight;
            }
            else
            {
                x += swatchWidth;
            }
        }
    }
}

void ColourSelector::sliderValueChanged (Slider*)
{
    if (sliders[0] != nullptr)
        setCurrentColour (Colour ((uint8) sliders[0]->getValue(),
                                  (uint8) sliders[1]->getValue(),
                                  (uint8) sliders[2]->getValue(),
                                  (uint8) sliders[3]->getValue()));
}

//==============================================================================
int ColourSelector::getNumSwatches() const
{
    return 0;
}

Colour ColourSelector::getSwatchColour (const int) const
{
    jassertfalse; // if you've overridden getNumSwatches(), you also need to implement this method
    return Colours::black;
}

void ColourSelector::setSwatchColour (const int, const Colour&) const
{
    jassertfalse; // if you've overridden getNumSwatches(), you also need to implement this method
}
