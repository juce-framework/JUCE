/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  1 Dec 2009 9:04:27 pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "RenderingTestComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
class RenderingTestCanvas   : public Component,
                              public Timer
{
public:
    RenderingTestCanvas (RenderingTestComponent& owner_)
        : owner (owner_)
    {
        setOpaque (true);
        averageTime = 0;

        rgbImage = ImageFileFormat::loadFrom (RenderingTestComponent::demoJpeg_jpg, RenderingTestComponent::demoJpeg_jpgSize);
        argbImage = ImageFileFormat::loadFrom (RenderingTestComponent::demoPng_png, RenderingTestComponent::demoPng_pngSize);
        createSVGDrawable();

        glyphs.addFittedText (Font (20.0f), "The Quick Brown Fox Jumped Over The Lazy Dog",
                              -120, -50, 240, 100, Justification::centred, 2, 1.0f);

        int i;
        for (i = 0; i < numElementsInArray (bouncingPointX); ++i)
        {
            bouncingPointX[i] = (float) Random::getSystemRandom().nextInt (200);
            bouncingPointY[i] = (float) Random::getSystemRandom().nextInt (200);
            bouncingPointDX[i] = (Random::getSystemRandom().nextFloat() - 0.5f) * 6.0f;
            bouncingPointDY[i] = (Random::getSystemRandom().nextFloat() - 0.5f) * 6.0f;
        }

        for (i = 0; i < numElementsInArray (bouncingNumber); ++i)
        {
            bouncingNumber[i] = Random::getSystemRandom().nextFloat();
            bouncingNumberDelta[i] = (Random::getSystemRandom().nextFloat() - 0.5f) * 0.03f;
        }

        for (i = 0; i < numElementsInArray (speeds); ++i)
            speeds[i] = 0.02f;

        int redrawFramesPerSecond = 60;
        startTimer (1000 / redrawFramesPerSecond);
    }

    ~RenderingTestCanvas()
    {
        delete rgbImage;
        delete argbImage;
        delete svgDrawable;
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::white);
        g.setImageResamplingQuality (owner.highQualityToggle->getToggleState() ? Graphics::highResamplingQuality
                                                                               : Graphics::lowResamplingQuality);

        double startTime = Time::getMillisecondCounterHiRes();

        if (owner.clipToRectangleToggle->getToggleState())
            clipToRectangle (g);

        if (owner.clipToPathToggle->getToggleState())
            clipToPath (g);

        if (owner.clipToImageToggle->getToggleState())
            clipToImage (g);

        g.fillCheckerBoard (0, 0, getWidth(), getHeight(), 50, 50,
                            Colour (0xffdddddd), Colours::transparentBlack);

        switch (owner.testTypeComboBox->getSelectedId())
        {
            case 1:  drawPaths (g, true, false, false); break;
            case 2:  drawPaths (g, false, true, false); break;
            case 3:  drawPaths (g, false, false, true); break;
            case 4:  drawStroke (g); break;
            case 5:  drawImages (g, rgbImage); break;
            case 6:  drawImages (g, argbImage);  break;
            case 7:  drawTiling (g, rgbImage); break;
            case 8:  drawTiling (g, argbImage);  break;
            case 9:  drawGlyphs (g); break;
            case 10: drawSVG (g); break;
            case 11: drawLines (g); break;
        }

        double endTime = Time::getMillisecondCounterHiRes();
        double timeTaken = endTime - startTime;
        averageTime += (timeTaken - averageTime) * 0.1;
    }

    void timerCallback()
    {
        repaint();

        int i;
        for (i = 0; i < numElementsInArray (bouncingPointX); ++i)
        {
            bounce (bouncingPointX[i], bouncingPointDX[i], (float) getWidth());
            bounce (bouncingPointY[i], bouncingPointDY[i], (float) getHeight());
        }

        for (i = 0; i < numElementsInArray (bouncingNumber); ++i)
            bounce (bouncingNumber[i], bouncingNumberDelta[i], 1.0f);

        owner.speedLabel->setText (String (getWidth()) + "x" + String (getHeight())
                                    + " - Render time: " + String (averageTime, 2) + "ms", false);

        if (owner.animatePositionToggle->getToggleState())
        {
            bounce (owner.xSlider, speeds[0]);
            bounce (owner.ySlider, speeds[1]);
        }

        if (owner.animateSizeToggle->getToggleState())
        {
            bounce (owner.sizeSlider, speeds[2]);
        }

        if (owner.animateRotationToggle->getToggleState())
        {
            bounce (owner.angleSlider, speeds[3]);
        }
    }

private:
    RenderingTestComponent& owner;
    double averageTime;

    Image* rgbImage;
    Image* argbImage;
    DrawableComposite* svgDrawable;
    GlyphArrangement glyphs;
    ColourGradient linearGradient, radialGradient;
    float bouncingPointX[10], bouncingPointY[10], bouncingPointDX[10], bouncingPointDY[10];
    float bouncingNumber[8], bouncingNumberDelta[8];
    float speeds[8];
    Time lastSVGLoadTime;

    const AffineTransform getTransform()
    {
        return AffineTransform::rotation ((float) owner.angleSlider->getValue() / (180.0f / float_Pi))
                                .scaled ((float) owner.sizeSlider->getValue(),
                                         (float) owner.sizeSlider->getValue())
                                .translated (getWidth() * 0.5f + (float) owner.xSlider->getValue(),
                                             getHeight() * 0.5f + (float) owner.ySlider->getValue());
    }

    void clipToRectangle (Graphics& g)
    {
        g.reduceClipRegion ((int) bouncingPointX[0] / 2, (int) bouncingPointY[0] / 2,
                            getWidth() / 2, getHeight() / 2);
    }

    void clipToPath (Graphics& g)
    {
        float size = (float) jmin (getWidth(), getHeight());

        Path p;
        p.addStar (bouncingPointX[1], bouncingPointY[1], 7,
                   size * jmax (0.6f, bouncingNumber[4]),
                   size * jmax (0.7f, bouncingNumber[5]),
                   bouncingNumber[4]);

        g.reduceClipRegion (p, AffineTransform::identity);
    }

    void clipToImage (Graphics& g)
    {
        AffineTransform transform (AffineTransform::translation (argbImage->getWidth() / -2.0f, argbImage->getHeight() / -2.0f)
                                                   .rotated (bouncingNumber[3])
                                                   .scaled (bouncingNumber[2] + 4.0f, bouncingNumber[2] + 4.0f)
                                                   .translated (bouncingPointX[2], bouncingPointY[2]));
        g.reduceClipRegion (*argbImage,
                            Rectangle (0, 0, argbImage->getWidth(), argbImage->getHeight()),
                            transform);
    }

    void drawPaths (Graphics& g, bool solid, bool linearGradient, bool radialGradient)
    {
        Path p;
        p.addRectangle (-50, 0, 100, 100);
        p.addStar (100.0f, 0.0f, 7, 30.0f, 70.0f, 0.1f);
        p.addStar (-100.0f, 0.0f, 6, 40.0f, 70.0f, 0.1f);
        p.addEllipse (-60.0f, -100.0f, 120.0f, 90.0f);

        if (linearGradient || radialGradient)
        {
            Colour c1 (bouncingNumber[0], bouncingNumber[1], bouncingNumber[2], 1.0f);
            Colour c2 (bouncingNumber[4], bouncingNumber[5], bouncingNumber[6], 1.0f);
            Colour c3 (bouncingNumber[3], bouncingNumber[7], bouncingNumber[1], 1.0f);

            float x1 = getWidth() * 0.25f;
            float y1 = getHeight() * 0.25f;
            float x2 = getWidth() * 0.7f;
            float y2 = getHeight() * 0.75f;
            float intermediatePos = 0.5f;

            if (owner.animateFillToggle->getToggleState())
            {
                x1 = bouncingPointX[0];
                y1 = bouncingPointY[0];
                x2 = bouncingPointX[1];
                y2 = bouncingPointY[1];
                intermediatePos = bouncingNumber[0];
            }

            ColourGradient gradient (c1, x1, y1,
                                     c2, x2, y2,
                                     radialGradient);

            gradient.addColour (intermediatePos, c3);

            g.setGradientFill (gradient);
        }
        else
        {
            g.setColour (Colours::blue);
        }

        g.setOpacity ((float) owner.opacitySlider->getValue());
        g.fillPath (p, getTransform());
    }

    void drawStroke (Graphics& g)
    {
        Path p;
        p.startNewSubPath (bouncingPointX[0], bouncingPointY[0]);

        for (int i = 1; i < numElementsInArray (bouncingPointX) - 1; i += 2)
        {
            p.quadraticTo (bouncingPointX[i], bouncingPointY[i],
                           bouncingPointX[i + 1], bouncingPointY[i + 1]);
        }

        p.closeSubPath();

        PathStrokeType stroke (5.0f * (float) owner.sizeSlider->getValue());
        g.setColour (Colours::purple.withAlpha ((float) owner.opacitySlider->getValue()));
        g.strokePath (p, stroke, AffineTransform::identity);
    }

    void drawImages (Graphics& g, Image* image)
    {
        AffineTransform transform (AffineTransform::translation ((float) (image->getWidth() / -2),
                                                                 (float) (image->getHeight() / -2))
                                                   .followedBy (getTransform()));

        g.setOpacity ((float) owner.opacitySlider->getValue());
        g.drawImageTransformed (image, 0, 0,
                                image->getWidth(),
                                image->getHeight(),
                                transform, false);
    }

    void drawTiling (Graphics& g, Image* image)
    {
        AffineTransform transform (AffineTransform::translation ((float) (image->getWidth() / -2),
                                                                 (float) (image->getHeight() / -2))
                                                   .followedBy (getTransform()));

        FillType fill (*image, transform);
        fill.setOpacity ((float) owner.opacitySlider->getValue());
        g.setFillType (fill);
        g.fillAll();
    }

    void drawGlyphs (Graphics& g)
    {
        g.setColour (Colours::black.withAlpha ((float) owner.opacitySlider->getValue()));

        glyphs.draw (g, getTransform());
    }

    void drawSVG (Graphics& g)
    {
        if (Time::getCurrentTime().toMilliseconds() > lastSVGLoadTime.toMilliseconds() + 3000)
        {
            lastSVGLoadTime = Time::getCurrentTime();
            createSVGDrawable();
        }

        svgDrawable->draw (g, (float) owner.opacitySlider->getValue(), getTransform());
    }

    void drawLines (Graphics& g)
    {
        g.setColour (Colours::blue.withAlpha ((float) owner.opacitySlider->getValue()));

        for (int x = 0; x < getWidth(); ++x)
        {
            float y = getHeight() * 0.3f;
            float width = y * fabsf (sinf (x / 100.0f + 2.0f * bouncingNumber[1]));
            g.drawVerticalLine (x, y - width, y + width);
        }

        g.setColour (Colours::green.withAlpha ((float) owner.opacitySlider->getValue()));

        for (int y = 0; y < getHeight(); ++y)
        {
            float x = getWidth() * 0.3f;
            float width = x * fabsf (sinf (y / 100.0f + 2.0f * bouncingNumber[2]));
            g.drawHorizontalLine (y, x - width, x + width);
        }

        g.setColour (Colours::yellow.withAlpha ((float) owner.opacitySlider->getValue()));
        g.drawLine (bouncingPointX[0], bouncingPointY[0], bouncingPointX[1], bouncingPointY[1]);
        g.drawLine (getWidth() - bouncingPointX[0], getHeight() - bouncingPointY[0],
                    getWidth() - bouncingPointX[1], getHeight() - bouncingPointY[1]);
    }

    void createSVGDrawable()
    {
        svgDrawable = 0;

        MemoryInputStream iconsFileStream (BinaryData::icons_zip, BinaryData::icons_zipSize, false);
        ZipFile icons (&iconsFileStream, false);

        // Load a random SVG file from our embedded icons.zip file.
        InputStream* svgFileStream
            = icons.createStreamForEntry (Random::getSystemRandom().nextInt (icons.getNumEntries()));

        if (svgFileStream != 0)
        {
            Drawable* loadedSVG = Drawable::createFromImageDataStream (*svgFileStream);
            delete svgFileStream;

            if (loadedSVG != 0)
            {
                // to make our icon the right size, we'll put it inside a DrawableComposite, and apply
                // a transform to get it to the size we want.

                float x, y, w, h;
                loadedSVG->getBounds (x, y, w, h);
                const float scaleFactor = 300.0f / jmax (w, h);

                svgDrawable = new DrawableComposite();
                svgDrawable->insertDrawable (loadedSVG, AffineTransform::translation (-(x + w * 0.5f), -(y + h * 0.5f))
                                                                        .scaled (scaleFactor, scaleFactor));
            }
        }
    }

    static void bounce (float& x, float& dx, float maxX)
    {
        x += dx;

        if (x <= 0)
            dx = fabsf (dx);
        else if (x >= maxX)
            dx = -fabsf (dx);

        x = jlimit (0.0f, maxX, x);
    }

    static void bounce (Slider* slider, float& speed)
    {
        double v = slider->getValue() + speed * (slider->getMaximum() - slider->getMinimum());

        if (v <= slider->getMinimum())
            speed = fabsf (speed);
        else if (v >= slider->getMaximum())
            speed = -fabsf (speed);

        slider->setValue (v, false);
    }
};

Component* createRenderingDemo()
{
    return new RenderingTestComponent();
}

//[/MiscUserDefs]

//==============================================================================
RenderingTestComponent::RenderingTestComponent ()
    : testTypeComboBox (0),
      testTypeLabel (0),
      speedLabel (0),
      testCanvas (0),
      opacitySlider (0),
      highQualityToggle (0),
      animateSizeToggle (0),
      animateRotationToggle (0),
      animatePositionToggle (0),
      animateFillToggle (0),
      opacityLabel (0),
      xSlider (0),
      ySlider (0),
      sizeSlider (0),
      angleSlider (0),
      xSliderLabel (0),
      ySliderLabel (0),
      sizeSliderLabel (0),
      angleSliderLabel (0),
      clipToRectangleToggle (0),
      clipToPathToggle (0),
      clipToImageToggle (0)
{
    addAndMakeVisible (testTypeComboBox = new ComboBox (String::empty));
    testTypeComboBox->setEditableText (false);
    testTypeComboBox->setJustificationType (Justification::centredLeft);
    testTypeComboBox->setTextWhenNothingSelected (String::empty);
    testTypeComboBox->setTextWhenNoChoicesAvailable (T("(no choices)"));
    testTypeComboBox->addItem (T("Paths - Solid"), 1);
    testTypeComboBox->addItem (T("Paths - Linear gradient"), 2);
    testTypeComboBox->addItem (T("Paths - Radial gradient"), 3);
    testTypeComboBox->addItem (T("Paths - Stroked"), 4);
    testTypeComboBox->addItem (T("Images - RGB"), 5);
    testTypeComboBox->addItem (T("Images - ARGB"), 6);
    testTypeComboBox->addItem (T("Tiled Images - RGB"), 7);
    testTypeComboBox->addItem (T("Tiled Images - ARGB"), 8);
    testTypeComboBox->addItem (T("Glyphs"), 9);
    testTypeComboBox->addItem (T("SVG"), 10);
    testTypeComboBox->addItem (T("Lines"), 11);
    testTypeComboBox->addListener (this);

    addAndMakeVisible (testTypeLabel = new Label (String::empty,
                                                  T("Test type:")));
    testTypeLabel->setFont (Font (15.0000f, Font::plain));
    testTypeLabel->setJustificationType (Justification::centredRight);
    testTypeLabel->setEditable (false, false, false);
    testTypeLabel->setColour (TextEditor::textColourId, Colours::black);
    testTypeLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (speedLabel = new Label (String::empty,
                                               T("speed")));
    speedLabel->setFont (Font (15.0000f, Font::plain));
    speedLabel->setJustificationType (Justification::centredLeft);
    speedLabel->setEditable (false, false, false);
    speedLabel->setColour (TextEditor::textColourId, Colours::black);
    speedLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (testCanvas = new RenderingTestCanvas (*this));

    addAndMakeVisible (opacitySlider = new Slider (String::empty));
    opacitySlider->setRange (0, 1, 0.001);
    opacitySlider->setSliderStyle (Slider::LinearHorizontal);
    opacitySlider->setTextBoxStyle (Slider::TextBoxLeft, false, 70, 20);
    opacitySlider->addListener (this);

    addAndMakeVisible (highQualityToggle = new ToggleButton (String::empty));
    highQualityToggle->setButtonText (T("Higher quality image interpolation"));

    addAndMakeVisible (animateSizeToggle = new ToggleButton (String::empty));
    animateSizeToggle->setButtonText (T("Animate size"));

    addAndMakeVisible (animateRotationToggle = new ToggleButton (String::empty));
    animateRotationToggle->setButtonText (T("Animate rotation"));

    addAndMakeVisible (animatePositionToggle = new ToggleButton (String::empty));
    animatePositionToggle->setButtonText (T("Animate position"));

    addAndMakeVisible (animateFillToggle = new ToggleButton (String::empty));
    animateFillToggle->setButtonText (T("Animate gradient"));

    addAndMakeVisible (opacityLabel = new Label (String::empty,
                                                 T("Opacity:")));
    opacityLabel->setFont (Font (15.0000f, Font::plain));
    opacityLabel->setJustificationType (Justification::centredRight);
    opacityLabel->setEditable (false, false, false);
    opacityLabel->setColour (TextEditor::textColourId, Colours::black);
    opacityLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (xSlider = new Slider (String::empty));
    xSlider->setRange (-100, 100, 0.1);
    xSlider->setSliderStyle (Slider::LinearHorizontal);
    xSlider->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
    xSlider->addListener (this);

    addAndMakeVisible (ySlider = new Slider (String::empty));
    ySlider->setRange (-100, 100, 0.1);
    ySlider->setSliderStyle (Slider::LinearHorizontal);
    ySlider->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
    ySlider->addListener (this);

    addAndMakeVisible (sizeSlider = new Slider (String::empty));
    sizeSlider->setRange (0.01, 10, 0.01);
    sizeSlider->setSliderStyle (Slider::LinearHorizontal);
    sizeSlider->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
    sizeSlider->addListener (this);
    sizeSlider->setSkewFactor (0.4);

    addAndMakeVisible (angleSlider = new Slider (String::empty));
    angleSlider->setRange (-180, 180, 0.1);
    angleSlider->setSliderStyle (Slider::LinearHorizontal);
    angleSlider->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
    angleSlider->addListener (this);

    addAndMakeVisible (xSliderLabel = new Label (String::empty,
                                                 T("X offset:")));
    xSliderLabel->setFont (Font (15.0000f, Font::plain));
    xSliderLabel->setJustificationType (Justification::centredRight);
    xSliderLabel->setEditable (false, false, false);
    xSliderLabel->setColour (TextEditor::textColourId, Colours::black);
    xSliderLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (ySliderLabel = new Label (String::empty,
                                                 T("Y offset:")));
    ySliderLabel->setFont (Font (15.0000f, Font::plain));
    ySliderLabel->setJustificationType (Justification::centredRight);
    ySliderLabel->setEditable (false, false, false);
    ySliderLabel->setColour (TextEditor::textColourId, Colours::black);
    ySliderLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (sizeSliderLabel = new Label (String::empty,
                                                    T("Size:")));
    sizeSliderLabel->setFont (Font (15.0000f, Font::plain));
    sizeSliderLabel->setJustificationType (Justification::centredRight);
    sizeSliderLabel->setEditable (false, false, false);
    sizeSliderLabel->setColour (TextEditor::textColourId, Colours::black);
    sizeSliderLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (angleSliderLabel = new Label (String::empty,
                                                     T("Angle:")));
    angleSliderLabel->setFont (Font (15.0000f, Font::plain));
    angleSliderLabel->setJustificationType (Justification::centredRight);
    angleSliderLabel->setEditable (false, false, false);
    angleSliderLabel->setColour (TextEditor::textColourId, Colours::black);
    angleSliderLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (clipToRectangleToggle = new ToggleButton (String::empty));
    clipToRectangleToggle->setButtonText (T("Clip to rectangle"));

    addAndMakeVisible (clipToPathToggle = new ToggleButton (String::empty));
    clipToPathToggle->setButtonText (T("Clip to path"));

    addAndMakeVisible (clipToImageToggle = new ToggleButton (String::empty));
    clipToImageToggle->setButtonText (T("Clip to image"));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);

    //[Constructor] You can add your own custom stuff here..
    testTypeComboBox->setSelectedId (1);
    sizeSlider->setValue (1.0, false);
    opacitySlider->setValue (1.0, false);
    highQualityToggle->setToggleState (true, false);
    //[/Constructor]
}

RenderingTestComponent::~RenderingTestComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (testTypeComboBox);
    deleteAndZero (testTypeLabel);
    deleteAndZero (speedLabel);
    deleteAndZero (testCanvas);
    deleteAndZero (opacitySlider);
    deleteAndZero (highQualityToggle);
    deleteAndZero (animateSizeToggle);
    deleteAndZero (animateRotationToggle);
    deleteAndZero (animatePositionToggle);
    deleteAndZero (animateFillToggle);
    deleteAndZero (opacityLabel);
    deleteAndZero (xSlider);
    deleteAndZero (ySlider);
    deleteAndZero (sizeSlider);
    deleteAndZero (angleSlider);
    deleteAndZero (xSliderLabel);
    deleteAndZero (ySliderLabel);
    deleteAndZero (sizeSliderLabel);
    deleteAndZero (angleSliderLabel);
    deleteAndZero (clipToRectangleToggle);
    deleteAndZero (clipToPathToggle);
    deleteAndZero (clipToImageToggle);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void RenderingTestComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void RenderingTestComponent::resized()
{
    testTypeComboBox->setBounds (proportionOfWidth (0.1652f), 16, proportionOfWidth (0.3425f), 24);
    testTypeLabel->setBounds (proportionOfWidth (0.0275f), 16, proportionOfWidth (0.1291f), 24);
    speedLabel->setBounds (proportionOfWidth (0.5370f), 16, proportionOfWidth (0.4303f), 24);
    testCanvas->setBounds (20, 56, getWidth() - 40, getHeight() - 215);
    opacitySlider->setBounds (proportionOfWidth (0.5990f), getHeight() - 141, proportionOfWidth (0.3787f), 24);
    highQualityToggle->setBounds (proportionOfWidth (0.0224f), getHeight() - 141, proportionOfWidth (0.4458f), 24);
    animateSizeToggle->setBounds (proportionOfWidth (0.7573f), getHeight() - 85, proportionOfWidth (0.2237f), 24);
    animateRotationToggle->setBounds (proportionOfWidth (0.7573f), getHeight() - 61, proportionOfWidth (0.2237f), 24);
    animatePositionToggle->setBounds (proportionOfWidth (0.7573f), getHeight() - 109, proportionOfWidth (0.2237f), 24);
    animateFillToggle->setBounds (proportionOfWidth (0.7573f), getHeight() - 37, proportionOfWidth (0.2341f), 24);
    opacityLabel->setBounds ((proportionOfWidth (0.5990f)) + -66, getHeight() - 141, 64, 24);
    xSlider->setBounds (proportionOfWidth (0.3614f), getHeight() - 109, proportionOfWidth (0.3787f), 24);
    ySlider->setBounds (proportionOfWidth (0.3614f), getHeight() - 85, proportionOfWidth (0.3787f), 24);
    sizeSlider->setBounds (proportionOfWidth (0.3614f), getHeight() - 61, proportionOfWidth (0.3787f), 24);
    angleSlider->setBounds (proportionOfWidth (0.3614f), getHeight() - 37, proportionOfWidth (0.3787f), 24);
    xSliderLabel->setBounds (proportionOfWidth (0.2496f), getHeight() - 109, proportionOfWidth (0.1067f), 24);
    ySliderLabel->setBounds (proportionOfWidth (0.2496f), getHeight() - 85, proportionOfWidth (0.1067f), 24);
    sizeSliderLabel->setBounds (proportionOfWidth (0.2496f), getHeight() - 61, proportionOfWidth (0.1067f), 24);
    angleSliderLabel->setBounds (proportionOfWidth (0.2496f), getHeight() - 37, proportionOfWidth (0.1067f), 24);
    clipToRectangleToggle->setBounds (proportionOfWidth (0.0224f), getHeight() - 109, 144, 24);
    clipToPathToggle->setBounds (proportionOfWidth (0.0224f), getHeight() - 85, 144, 24);
    clipToImageToggle->setBounds (proportionOfWidth (0.0224f), getHeight() - 61, 144, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void RenderingTestComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == testTypeComboBox)
    {
        //[UserComboBoxCode_testTypeComboBox] -- add your combo box handling code here..
        //[/UserComboBoxCode_testTypeComboBox]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void RenderingTestComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == opacitySlider)
    {
        //[UserSliderCode_opacitySlider] -- add your slider handling code here..
        //[/UserSliderCode_opacitySlider]
    }
    else if (sliderThatWasMoved == xSlider)
    {
        //[UserSliderCode_xSlider] -- add your slider handling code here..
        //[/UserSliderCode_xSlider]
    }
    else if (sliderThatWasMoved == ySlider)
    {
        //[UserSliderCode_ySlider] -- add your slider handling code here..
        //[/UserSliderCode_ySlider]
    }
    else if (sliderThatWasMoved == sizeSlider)
    {
        //[UserSliderCode_sizeSlider] -- add your slider handling code here..
        //[/UserSliderCode_sizeSlider]
    }
    else if (sliderThatWasMoved == angleSlider)
    {
        //[UserSliderCode_angleSlider] -- add your slider handling code here..
        //[/UserSliderCode_angleSlider]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="RenderingTestComponent" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffffffff"/>
  <COMBOBOX name="" id="216a392b47348589" memberName="testTypeComboBox" virtualName=""
            explicitFocusOrder="0" pos="16.469% 16 34.256% 24" editable="0"
            layout="33" items="Paths - Solid&#10;Paths - Linear gradient&#10;Paths - Radial gradient&#10;Paths - Stroked&#10;Images - RGB&#10;Images - ARGB&#10;Tiled Images - RGB&#10;Tiled Images - ARGB&#10;Glyphs&#10;SVG&#10;Lines"
            textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <LABEL name="" id="193cb8e961baa02a" memberName="testTypeLabel" virtualName=""
         explicitFocusOrder="0" pos="2.767% 16 12.912% 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Test type:" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
  <LABEL name="" id="c4977cdfea8776fb" memberName="speedLabel" virtualName=""
         explicitFocusOrder="0" pos="53.755% 16 43.083% 24" edTextCol="ff000000"
         edBkgCol="0" labelText="speed" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="" id="e21621e19906eff" memberName="testCanvas" virtualName=""
                    explicitFocusOrder="0" pos="20 56 40M 215M" class="RenderingTestCanvas"
                    params="*this"/>
  <SLIDER name="" id="e970a33ca991909e" memberName="opacitySlider" virtualName=""
          explicitFocusOrder="0" pos="59.947% 141R 37.813% 24" min="0"
          max="1" int="0.001" style="LinearHorizontal" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="70" textBoxHeight="20" skewFactor="1"/>
  <TOGGLEBUTTON name="" id="2d368b2ffc99beef" memberName="highQualityToggle"
                virtualName="" explicitFocusOrder="0" pos="2.24% 141R 44.532% 24"
                buttonText="Higher quality image interpolation" connectedEdges="0"
                needsCallback="0" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="" id="3b7c06ef24935a72" memberName="animateSizeToggle"
                virtualName="" explicitFocusOrder="0" pos="75.758% 85R 22.398% 24"
                buttonText="Animate size" connectedEdges="0" needsCallback="0"
                radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="" id="acf40ac0130d68eb" memberName="animateRotationToggle"
                virtualName="" explicitFocusOrder="0" pos="75.758% 61R 22.398% 24"
                buttonText="Animate rotation" connectedEdges="0" needsCallback="0"
                radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="" id="afe7d31210a544cb" memberName="animatePositionToggle"
                virtualName="" explicitFocusOrder="0" pos="75.758% 109R 22.398% 24"
                buttonText="Animate position" connectedEdges="0" needsCallback="0"
                radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="" id="20466306ead4c6c2" memberName="animateFillToggle"
                virtualName="" explicitFocusOrder="0" pos="75.758% 37R 23.452% 24"
                buttonText="Animate gradient" connectedEdges="0" needsCallback="0"
                radioGroupId="0" state="0"/>
  <LABEL name="" id="ff3fb4acd2101aa5" memberName="opacityLabel" virtualName=""
         explicitFocusOrder="0" pos="-66 141R 64 24" posRelativeX="e970a33ca991909e"
         edTextCol="ff000000" edBkgCol="0" labelText="Opacity:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <SLIDER name="" id="549cfd1459f09c12" memberName="xSlider" virtualName=""
          explicitFocusOrder="0" pos="36.1% 109R 37.813% 24" min="-100"
          max="100" int="0.1" style="LinearHorizontal" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="" id="49b53bab0eca9967" memberName="ySlider" virtualName=""
          explicitFocusOrder="0" pos="36.1% 85R 37.813% 24" min="-100"
          max="100" int="0.1" style="LinearHorizontal" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="" id="d89d3e0269c1aef4" memberName="sizeSlider" virtualName=""
          explicitFocusOrder="0" pos="36.1% 61R 37.813% 24" min="0.01"
          max="10" int="0.01" style="LinearHorizontal" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="0.4"/>
  <SLIDER name="" id="a68c75ae0f41c437" memberName="angleSlider" virtualName=""
          explicitFocusOrder="0" pos="36.1% 37R 37.813% 24" min="-180"
          max="180" int="0.1" style="LinearHorizontal" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <LABEL name="" id="61972b44db6093d7" memberName="xSliderLabel" virtualName=""
         explicitFocusOrder="0" pos="24.901% 109R 10.672% 24" edTextCol="ff000000"
         edBkgCol="0" labelText="X offset:" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
  <LABEL name="" id="fd87229f56908c79" memberName="ySliderLabel" virtualName=""
         explicitFocusOrder="0" pos="24.901% 85R 10.672% 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Y offset:" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
  <LABEL name="" id="889901f3d351ac41" memberName="sizeSliderLabel" virtualName=""
         explicitFocusOrder="0" pos="24.901% 61R 10.672% 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Size:" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
  <LABEL name="" id="98c096221f161097" memberName="angleSliderLabel" virtualName=""
         explicitFocusOrder="0" pos="24.901% 37R 10.672% 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Angle:" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
  <TOGGLEBUTTON name="" id="dc21f241b7188003" memberName="clipToRectangleToggle"
                virtualName="" explicitFocusOrder="0" pos="2.24% 109R 144 24"
                buttonText="Clip to rectangle" connectedEdges="0" needsCallback="0"
                radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="" id="e242a0decedf4fbd" memberName="clipToPathToggle" virtualName=""
                explicitFocusOrder="0" pos="2.24% 85R 144 24" buttonText="Clip to path"
                connectedEdges="0" needsCallback="0" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="" id="2c40de62d77841ae" memberName="clipToImageToggle"
                virtualName="" explicitFocusOrder="0" pos="2.24% 61R 144 24"
                buttonText="Clip to image" connectedEdges="0" needsCallback="0"
                radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: demoJpeg_jpg, 111719, "../../../../../../../../Users/jules/Desktop/DemoJPEG.jpg"
static const unsigned char resource_RenderingTestComponent_demoJpeg_jpg[] = { 255,216,255,224,0,16,74,70,73,70,0,1,1,1,0,180,0,180,0,0,255,226,5,88,73,67,67,95,80,82,79,70,73,76,69,0,1,1,0,0,5,72,97,112,
112,108,2,32,0,0,115,99,110,114,82,71,66,32,88,89,90,32,7,211,0,7,0,1,0,0,0,0,0,0,97,99,115,112,65,80,80,76,0,0,0,0,97,112,112,108,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,246,214,0,1,0,0,0,0,211,45,97,112,
112,108,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,114,88,89,90,0,0,1,8,0,0,0,20,103,88,89,90,0,0,1,28,0,0,0,20,98,88,89,90,0,0,1,48,0,0,0,20,119,
116,112,116,0,0,1,68,0,0,0,20,99,104,97,100,0,0,1,88,0,0,0,44,114,84,82,67,0,0,1,132,0,0,0,14,103,84,82,67,0,0,1,132,0,0,0,14,98,84,82,67,0,0,1,132,0,0,0,14,100,101,115,99,0,0,4,216,0,0,0,110,99,112,114,
116,0,0,4,148,0,0,0,65,100,115,99,109,0,0,1,148,0,0,2,254,88,89,90,32,0,0,0,0,0,0,116,75,0,0,62,29,0,0,3,203,88,89,90,32,0,0,0,0,0,0,90,115,0,0,172,166,0,0,23,38,88,89,90,32,0,0,0,0,0,0,40,24,0,0,21,87,
0,0,184,51,88,89,90,32,0,0,0,0,0,0,243,82,0,1,0,0,0,1,22,207,115,102,51,50,0,0,0,0,0,1,12,66,0,0,5,222,255,255,243,38,0,0,7,146,0,0,253,145,255,255,251,162,255,255,253,163,0,0,3,220,0,0,192,108,99,117,
114,118,0,0,0,0,0,0,0,1,2,51,0,0,109,108,117,99,0,0,0,0,0,0,0,15,0,0,0,12,101,110,85,83,0,0,0,36,0,0,2,158,101,115,69,83,0,0,0,44,0,0,1,76,100,97,68,75,0,0,0,52,0,0,1,218,100,101,68,69,0,0,0,44,0,0,1,
152,102,105,70,73,0,0,0,40,0,0,0,196,102,114,70,85,0,0,0,60,0,0,2,194,105,116,73,84,0,0,0,44,0,0,2,114,110,108,78,76,0,0,0,36,0,0,2,14,110,111,78,79,0,0,0,32,0,0,1,120,112,116,66,82,0,0,0,40,0,0,2,74,
115,118,83,69,0,0,0,42,0,0,0,236,106,97,74,80,0,0,0,28,0,0,1,22,107,111,75,82,0,0,0,24,0,0,2,50,122,104,84,87,0,0,0,26,0,0,1,50,122,104,67,78,0,0,0,22,0,0,1,196,0,75,0,97,0,109,0,101,0,114,0,97,0,110,
0,32,0,82,0,71,0,66,0,45,0,112,0,114,0,111,0,102,0,105,0,105,0,108,0,105,0,82,0,71,0,66,0,45,0,112,0,114,0,111,0,102,0,105,0,108,0,32,0,102,0,246,0,114,0,32,0,75,0,97,0,109,0,101,0,114,0,97,48,171,48,
225,48,233,0,32,0,82,0,71,0,66,0,32,48,215,48,237,48,213,48,161,48,164,48,235,101,120,79,77,118,248,106,95,0,32,0,82,0,71,0,66,0,32,130,114,95,105,99,207,143,240,0,80,0,101,0,114,0,102,0,105,0,108,0,32,
0,82,0,71,0,66,0,32,0,112,0,97,0,114,0,97,0,32,0,67,0,225,0,109,0,97,0,114,0,97,0,82,0,71,0,66,0,45,0,107,0,97,0,109,0,101,0,114,0,97,0,112,0,114,0,111,0,102,0,105,0,108,0,82,0,71,0,66,0,45,0,80,0,114,
0,111,0,102,0,105,0,108,0,32,0,102,0,252,0,114,0,32,0,75,0,97,0,109,0,101,0,114,0,97,0,115,118,248,103,58,0,32,0,82,0,71,0,66,0,32,99,207,143,240,101,135,78,246,0,82,0,71,0,66,0,45,0,98,0,101,0,115,0,
107,0,114,0,105,0,118,0,101,0,108,0,115,0,101,0,32,0,116,0,105,0,108,0,32,0,75,0,97,0,109,0,101,0,114,0,97,0,82,0,71,0,66,0,45,0,112,0,114,0,111,0,102,0,105,0,101,0,108,0,32,0,67,0,97,0,109,0,101,0,114,
0,97,206,116,186,84,183,124,0,32,0,82,0,71,0,66,0,32,213,4,184,92,211,12,199,124,0,80,0,101,0,114,0,102,0,105,0,108,0,32,0,82,0,71,0,66,0,32,0,100,0,101,0,32,0,67,0,226,0,109,0,101,0,114,0,97,0,80,0,114,
0,111,0,102,0,105,0,108,0,111,0,32,0,82,0,71,0,66,0,32,0,70,0,111,0,116,0,111,0,99,0,97,0,109,0,101,0,114,0,97,0,67,0,97,0,109,0,101,0,114,0,97,0,32,0,82,0,71,0,66,0,32,0,80,0,114,0,111,0,102,0,105,0,
108,0,101,0,80,0,114,0,111,0,102,0,105,0,108,0,32,0,82,0,86,0,66,0,32,0,100,0,101,0,32,0,108,32,25,0,97,0,112,0,112,0,97,0,114,0,101,0,105,0,108,0,45,0,112,0,104,0,111,0,116,0,111,0,0,116,101,120,116,
0,0,0,0,67,111,112,121,114,105,103,104,116,32,50,48,48,51,32,65,112,112,108,101,32,67,111,109,112,117,116,101,114,32,73,110,99,46,44,32,97,108,108,32,114,105,103,104,116,115,32,114,101,115,101,114,118,
101,100,46,0,0,0,0,100,101,115,99,0,0,0,0,0,0,0,19,67,97,109,101,114,97,32,82,71,66,32,80,114,111,102,105,108,101,0,0,0,0,0,0,0,0,0,0,0,19,67,97,109,101,114,97,32,82,71,66,32,80,114,111,102,105,108,101,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,225,2,128,69,120,105,102,0,0,77,77,0,42,0,0,0,8,0,8,1,15,0,2,0,0,0,6,0,0,0,110,1,16,0,2,0,0,
0,22,0,0,0,116,1,18,0,3,0,0,0,1,0,1,0,0,1,26,0,5,0,0,0,1,0,0,0,138,1,27,0,5,0,0,0,1,0,0,0,146,1,40,0,3,0,0,0,1,0,2,0,0,1,50,0,2,0,0,0,20,0,0,0,154,135,105,0,4,0,0,0,1,0,0,0,174,0,0,0,0,67,97,110,111,110,
0,67,97,110,111,110,32,80,111,119,101,114,83,104,111,116,32,83,68,53,53,48,0,0,0,0,180,0,0,0,1,0,0,0,180,0,0,0,1,50,48,48,57,58,48,52,58,49,50,32,49,49,58,51,48,58,52,49,0,0,27,130,154,0,5,0,0,0,1,0,0,
1,248,130,157,0,5,0,0,0,1,0,0,2,0,136,39,0,3,0,0,0,1,0,50,0,0,144,0,0,7,0,0,0,4,48,50,50,48,144,3,0,2,0,0,0,20,0,0,2,8,144,4,0,2,0,0,0,20,0,0,2,28,145,2,0,5,0,0,0,1,0,0,2,48,146,1,0,10,0,0,0,1,0,0,2,56,
146,2,0,5,0,0,0,1,0,0,2,64,146,4,0,10,0,0,0,1,0,0,2,72,146,5,0,5,0,0,0,1,0,0,2,80,146,7,0,3,0,0,0,1,0,5,0,0,146,9,0,3,0,0,0,1,0,16,0,0,146,10,0,5,0,0,0,1,0,0,2,88,160,0,0,7,0,0,0,4,48,49,48,48,160,1,0,
3,0,0,0,1,0,1,0,0,160,2,0,4,0,0,0,1,0,0,2,128,160,3,0,4,0,0,0,1,0,0,1,224,162,14,0,5,0,0,0,1,0,0,2,96,162,15,0,5,0,0,0,1,0,0,2,104,162,16,0,3,0,0,0,1,0,2,0,0,162,23,0,3,0,0,0,1,0,2,0,0,164,1,0,3,0,0,0,
1,0,0,0,0,164,2,0,3,0,0,0,1,0,0,0,0,164,3,0,3,0,0,0,1,0,1,0,0,164,4,0,5,0,0,0,1,0,0,2,112,164,6,0,3,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,160,0,0,0,71,0,0,0,10,50,48,48,57,58,48,52,58,49,50,32,49,49,58,
51,48,58,52,49,0,50,48,48,57,58,48,52,58,49,50,32,49,49,58,51,48,58,52,49,0,0,0,0,3,0,0,0,1,0,0,0,117,0,0,0,16,0,0,0,181,0,0,0,32,0,0,0,0,0,0,0,1,0,0,0,107,0,0,0,32,0,0,8,209,0,0,0,250,0,7,208,0,0,0,0,
71,0,7,208,0,0,0,0,71,0,0,0,1,0,0,0,1,255,219,0,67,0,3,2,3,3,3,2,3,3,3,3,4,4,3,4,5,9,6,5,5,5,5,11,8,8,6,9,13,11,13,13,12,11,12,12,14,16,20,17,14,15,19,15,12,12,17,24,18,19,21,21,22,23,22,14,17,25,26,24,
22,26,20,22,22,22,255,219,0,67,1,4,4,4,5,5,5,10,6,6,10,22,14,12,14,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,
22,22,22,22,22,255,192,0,17,8,1,224,2,128,3,1,34,0,2,17,1,3,17,1,255,196,0,31,0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,255,196,0,181,16,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,125,1,2,3,0,4,17,
5,18,33,49,65,6,19,81,97,7,34,113,20,50,129,145,161,8,35,66,177,193,21,82,209,240,36,51,98,114,130,9,10,22,23,24,25,26,37,38,39,40,41,42,52,53,54,55,56,57,58,67,68,69,70,71,72,73,74,83,84,85,86,87,88,
89,90,99,100,101,102,103,104,105,106,115,116,117,118,119,120,121,122,131,132,133,134,135,136,137,138,146,147,148,149,150,151,152,153,154,162,163,164,165,166,167,168,169,170,178,179,180,181,182,183,184,
185,186,194,195,196,197,198,199,200,201,202,210,211,212,213,214,215,216,217,218,225,226,227,228,229,230,231,232,233,234,241,242,243,244,245,246,247,248,249,250,255,196,0,31,1,0,3,1,1,1,1,1,1,1,1,1,0,0,
0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,255,196,0,181,17,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,119,0,1,2,3,17,4,5,33,49,6,18,65,81,7,97,113,19,34,50,129,8,20,66,145,161,177,193,9,35,51,82,240,21,98,114,209,10,22,36,52,
225,37,241,23,24,25,26,38,39,40,41,42,53,54,55,56,57,58,67,68,69,70,71,72,73,74,83,84,85,86,87,88,89,90,99,100,101,102,103,104,105,106,115,116,117,118,119,120,121,122,130,131,132,133,134,135,136,137,138,
146,147,148,149,150,151,152,153,154,162,163,164,165,166,167,168,169,170,178,179,180,181,182,183,184,185,186,194,195,196,197,198,199,200,201,202,210,211,212,213,214,215,216,217,218,226,227,228,229,230,
231,232,233,234,242,243,244,245,246,247,248,249,250,255,218,0,12,3,1,0,2,17,3,17,0,63,0,225,74,156,240,56,163,111,165,57,69,24,231,165,126,204,126,90,32,30,221,105,192,10,7,106,59,255,0,129,165,96,2,1,
250,138,0,201,239,79,193,164,238,5,2,5,28,116,253,41,66,253,104,199,60,254,52,163,191,52,12,7,92,116,20,99,156,210,251,113,70,123,138,4,3,25,52,187,105,64,207,34,150,129,137,142,122,117,167,1,71,211,154,
94,222,180,0,40,237,210,148,1,233,73,239,78,192,245,52,128,48,8,20,187,71,225,75,199,3,189,59,233,72,6,129,147,208,82,128,51,75,143,122,95,206,152,6,51,74,163,183,165,3,160,205,59,241,164,0,64,165,0,122,
80,61,61,123,210,142,40,0,3,29,185,167,12,116,239,73,248,126,116,236,82,0,0,102,151,30,212,1,138,49,245,160,5,0,113,197,0,126,180,162,140,119,253,40,1,64,246,165,199,20,82,253,40,176,92,48,49,75,140,209,
210,143,229,64,11,128,41,64,233,64,233,78,237,64,128,1,233,75,142,104,24,165,234,104,176,197,2,158,163,131,197,39,243,237,78,28,208,0,7,173,56,15,254,185,160,12,241,218,138,67,23,214,151,25,163,28,81,
64,11,142,59,82,168,246,160,126,148,225,193,235,64,7,210,148,129,233,64,165,224,208,32,32,122,117,165,0,115,71,113,218,151,185,20,0,157,190,180,163,250,209,239,218,148,112,125,168,16,236,12,125,40,30,
244,30,41,69,22,0,3,233,78,192,201,164,237,222,156,57,29,40,0,29,5,41,239,210,129,215,165,3,214,128,29,131,64,25,232,40,3,245,165,235,142,105,128,0,49,239,78,3,154,23,167,181,40,227,173,32,14,167,56,165,
0,125,69,24,29,59,82,208,1,128,64,165,192,161,113,200,167,1,78,192,32,25,165,61,105,70,57,197,24,246,52,88,0,138,48,57,167,98,151,175,24,160,67,113,211,138,92,118,165,199,62,244,160,99,243,164,2,1,144,
56,250,208,5,59,30,180,224,6,104,1,155,70,61,233,64,226,157,129,207,29,41,113,64,13,35,142,252,82,226,157,143,151,165,41,28,116,160,4,3,52,180,163,131,239,71,124,80,51,202,0,4,103,61,41,192,0,41,122,26,
49,154,216,4,3,219,189,47,249,205,56,99,20,128,122,119,164,49,125,9,163,129,74,7,74,80,51,218,128,19,181,40,7,242,163,184,25,165,3,222,128,19,243,165,232,115,138,92,113,207,235,75,64,0,7,31,253,122,6,
51,206,105,121,197,40,238,123,210,1,7,78,166,148,3,197,40,24,227,20,184,200,160,4,250,83,189,121,231,181,0,115,143,233,75,130,71,90,0,0,235,222,156,48,114,57,160,12,82,254,84,128,13,56,113,218,142,223,
214,130,190,249,160,0,10,80,48,59,81,131,159,122,118,61,248,52,128,5,3,189,24,20,239,229,235,64,6,41,113,74,40,250,80,1,143,202,151,20,15,204,210,209,168,5,46,62,106,49,193,52,184,230,128,14,244,96,103,
220,210,128,56,165,20,0,0,56,167,1,214,128,61,169,71,90,0,76,122,83,128,233,75,233,74,56,63,225,64,10,1,165,3,138,49,138,127,92,82,1,162,156,5,40,20,188,231,220,208,2,123,102,140,210,241,233,205,40,29,
168,24,10,92,243,214,128,59,230,148,116,231,20,128,81,250,83,134,113,239,64,24,28,12,210,243,64,0,165,7,181,40,28,208,40,16,15,124,80,58,117,165,192,227,20,1,197,0,3,241,167,122,26,7,74,80,61,57,166,32,
28,82,140,138,7,165,47,57,160,0,82,245,230,143,199,154,80,56,160,5,28,208,49,64,7,52,224,51,205,0,32,235,138,118,56,20,159,94,148,240,61,168,0,252,41,69,20,184,197,0,41,235,237,74,58,117,163,20,171,64,
9,223,52,252,12,250,82,15,167,52,163,60,247,160,0,125,41,71,175,243,160,118,245,167,98,128,19,28,210,128,127,10,80,56,246,245,167,96,241,64,132,199,92,10,80,51,235,78,0,103,165,24,207,20,0,128,117,24,
165,29,105,192,123,210,129,205,3,27,140,118,230,143,122,144,12,125,104,219,250,80,3,0,61,105,113,158,106,93,135,52,109,160,44,69,142,120,165,197,63,109,0,26,0,242,127,98,41,113,74,123,82,156,26,212,4,
239,210,142,121,165,62,230,129,214,128,0,57,205,56,96,247,231,233,70,41,125,51,72,4,239,214,140,103,210,157,200,52,167,166,49,64,8,71,231,89,186,229,243,233,246,177,74,151,17,199,35,74,160,43,91,180,190,
111,63,112,99,166,122,100,244,173,50,208,160,221,44,241,68,184,234,239,138,241,239,140,190,37,142,27,148,177,183,158,228,44,35,46,20,237,141,219,177,207,92,142,163,177,6,188,44,227,51,161,67,15,82,154,
154,231,182,215,215,240,216,244,112,24,58,181,107,65,242,251,189,206,232,248,138,230,125,86,37,182,75,104,108,94,76,1,40,44,224,12,229,72,31,197,145,242,242,51,156,243,93,53,180,137,61,180,83,169,80,178,
174,237,190,96,59,61,137,207,81,210,190,108,209,181,73,190,212,109,99,89,24,198,251,130,137,50,88,129,211,131,131,159,92,117,175,65,181,181,138,245,97,183,187,112,182,214,215,107,115,117,44,67,17,76,165,
11,109,102,227,29,8,244,228,230,190,55,3,196,88,202,19,113,126,245,251,221,159,69,137,201,232,212,138,183,187,110,199,170,140,21,4,28,169,228,17,78,237,211,138,230,245,95,21,173,157,181,179,93,104,178,
131,113,114,45,161,120,202,130,78,9,220,224,28,0,120,0,14,152,231,173,110,61,221,180,90,100,218,133,212,159,101,182,133,11,200,103,249,10,129,235,239,95,113,133,206,40,86,77,77,56,73,110,154,253,79,155,
173,128,171,77,222,62,242,238,139,75,199,52,157,234,29,42,238,214,252,196,33,105,85,228,206,99,120,136,104,240,51,243,231,238,240,51,244,199,168,171,32,28,100,3,138,237,161,139,161,136,77,210,149,255,
0,224,156,213,104,85,165,101,56,216,6,58,80,1,239,82,67,27,202,225,17,11,200,221,0,28,154,204,125,103,72,138,63,53,175,162,150,17,40,138,70,138,64,194,54,57,198,227,219,161,199,173,78,35,27,135,195,175,
222,205,46,191,118,225,74,133,74,191,2,185,162,56,230,151,28,100,147,77,243,109,150,100,134,75,133,89,157,182,42,227,57,56,7,7,29,56,35,243,169,63,207,21,116,177,20,171,95,217,201,59,118,38,116,167,11,
115,171,92,65,138,81,210,140,117,231,154,112,234,49,91,16,29,197,40,29,104,28,210,226,128,12,115,71,20,160,126,31,90,120,28,255,0,42,0,110,41,113,74,7,108,125,105,113,197,0,32,7,189,40,20,160,117,165,
247,160,4,34,156,7,79,90,7,231,78,199,52,0,131,244,167,1,193,160,123,10,120,28,125,40,1,184,226,156,163,177,233,75,130,113,138,80,61,168,1,49,249,211,128,167,1,237,75,143,173,72,196,236,56,20,119,247,
167,99,3,52,123,208,2,127,42,59,211,143,98,122,208,63,26,3,112,250,210,142,77,0,115,158,41,192,208,0,7,126,230,156,59,209,143,126,105,122,80,32,238,41,122,158,104,52,189,69,0,0,100,81,233,138,92,122,154,
92,30,105,136,79,74,112,28,116,160,82,129,211,142,105,0,156,80,58,211,177,222,148,14,13,48,5,197,47,81,64,28,31,231,78,2,128,0,61,184,165,227,20,162,157,129,233,72,99,112,59,83,143,95,74,6,49,158,244,
236,123,211,1,59,123,82,128,61,120,250,82,251,118,165,198,58,129,64,132,3,138,119,122,80,5,40,25,233,64,9,248,113,75,223,165,47,165,46,40,1,0,246,167,1,159,173,46,41,64,160,0,14,122,127,245,169,192,82,
129,245,167,40,233,239,64,13,199,0,83,128,224,117,226,151,25,57,167,237,160,6,1,222,156,6,59,83,246,224,98,156,0,25,254,180,12,98,175,229,79,0,103,154,94,195,138,117,33,13,164,199,38,158,58,209,142,158,
180,12,102,59,82,1,205,72,71,20,135,173,23,19,60,144,3,249,80,7,52,243,244,164,3,154,216,3,29,177,210,128,61,58,210,145,74,7,229,64,13,0,230,157,130,112,0,167,122,226,168,107,151,15,107,164,93,207,26,
72,242,36,103,106,160,36,147,248,115,89,213,155,132,28,173,178,42,49,188,146,238,94,81,244,170,186,133,245,181,138,196,215,62,96,89,24,168,40,187,185,3,56,192,245,172,207,10,222,220,203,100,35,213,102,
79,180,194,129,115,128,169,12,96,12,41,99,203,57,36,147,236,43,140,241,63,137,94,109,31,80,47,111,29,218,91,78,225,224,136,16,90,60,128,165,93,78,65,3,44,122,113,95,63,95,62,160,176,202,78,92,147,125,
55,251,252,159,161,234,83,203,42,123,110,84,185,146,249,20,62,46,120,180,217,92,88,219,89,68,50,200,222,97,147,42,202,78,54,224,142,171,214,188,167,91,214,133,217,130,21,104,103,100,44,75,24,137,4,159,
188,113,158,79,97,211,24,21,159,45,221,213,205,140,17,173,178,45,172,110,201,15,92,198,172,114,19,29,128,237,93,207,132,109,116,205,67,64,187,134,238,214,24,245,8,85,55,94,44,103,48,16,216,31,48,63,54,
65,253,121,233,95,3,152,98,222,34,187,196,77,106,125,102,19,13,28,61,37,4,102,105,118,44,109,173,35,91,182,130,229,12,123,34,159,1,85,177,130,219,135,208,0,61,235,208,116,135,104,180,235,59,107,127,45,
38,142,8,98,212,150,92,200,89,212,226,72,241,145,146,78,84,55,184,56,199,20,154,78,153,103,106,247,214,87,150,170,215,246,202,64,104,102,100,105,0,229,27,60,241,208,49,35,229,201,192,167,88,216,234,26,
117,138,217,45,213,171,106,111,28,81,197,44,3,228,157,130,109,59,185,251,216,81,243,103,145,142,120,227,201,117,249,90,146,150,167,107,140,90,180,153,179,172,234,22,151,107,13,216,181,186,182,186,134,
80,96,146,39,82,209,5,220,9,100,108,130,199,229,35,35,144,61,234,182,181,172,201,97,111,170,218,155,168,46,100,37,90,103,104,203,5,35,107,6,3,248,193,7,161,228,14,7,65,84,224,142,234,223,84,178,182,158,
120,44,110,100,79,222,185,82,234,203,252,71,140,228,231,25,237,207,21,79,196,175,105,10,43,52,118,211,77,18,182,229,80,75,4,31,47,3,159,151,11,144,120,56,168,120,154,174,167,52,164,219,51,80,130,141,146,
208,232,188,58,154,166,161,28,87,247,141,115,30,151,99,32,27,173,163,249,165,114,225,124,173,163,57,228,130,73,231,110,57,244,232,188,107,173,106,90,38,171,100,45,110,76,22,238,162,103,89,81,88,13,164,
134,25,7,149,32,253,115,214,188,238,203,198,23,49,233,247,81,91,171,217,217,33,204,80,35,156,205,25,0,97,135,240,122,6,235,211,166,43,151,241,102,177,62,171,16,182,188,212,22,27,20,59,126,205,2,230,116,
207,204,79,247,73,44,73,219,220,224,158,181,234,211,204,102,176,238,133,59,197,183,171,190,253,175,165,244,245,57,103,132,140,171,41,201,94,199,99,168,252,67,109,82,218,75,104,202,88,65,18,111,251,68,
111,178,71,101,108,130,172,15,65,199,202,58,159,165,113,182,146,95,204,233,120,215,143,113,52,147,9,84,128,170,93,71,241,50,240,9,82,79,211,214,170,220,120,110,83,111,114,218,149,188,150,214,3,203,49,
134,81,230,176,36,240,54,240,61,72,35,156,117,238,106,219,219,71,123,115,21,132,109,228,165,180,74,233,189,142,223,47,39,160,207,200,50,7,29,13,114,86,175,82,163,114,169,81,201,174,175,95,196,222,157,
10,112,86,166,146,71,178,248,6,222,79,237,123,219,221,104,197,37,249,186,220,93,165,35,114,28,24,202,30,248,27,87,220,241,210,189,38,241,226,183,73,101,150,68,138,222,48,88,179,176,0,40,238,107,197,52,
75,216,237,230,210,111,28,102,230,210,49,246,119,43,143,49,49,140,13,220,19,195,116,174,151,72,150,29,75,93,103,191,191,123,207,179,72,242,183,152,185,221,185,125,72,218,177,42,128,115,156,22,237,156,
215,187,144,231,235,8,189,141,71,100,237,171,217,125,223,240,15,47,51,203,126,177,105,199,167,222,207,71,82,10,171,43,6,86,0,130,59,131,78,198,49,239,205,114,154,167,136,13,197,234,90,232,83,219,136,4,
141,12,247,147,72,129,17,250,124,185,28,227,150,244,226,168,221,248,146,56,181,107,155,171,86,141,109,238,160,62,85,212,251,118,16,136,2,227,7,32,111,96,73,234,107,234,231,197,24,24,219,150,87,93,246,
249,219,215,185,225,199,39,175,43,182,172,119,99,233,75,138,231,252,21,127,5,214,139,109,9,188,150,230,237,19,231,158,81,204,216,3,115,103,191,57,250,129,197,116,17,176,116,86,86,202,176,200,62,162,189,
108,191,48,161,141,164,167,77,235,213,95,99,139,21,132,169,135,155,140,150,157,251,142,20,225,211,154,76,96,85,123,155,187,123,105,45,196,237,182,57,24,172,146,2,8,135,229,98,11,14,184,37,113,248,215,
70,35,19,75,15,79,218,85,149,145,141,42,83,171,46,88,43,178,215,24,160,117,230,178,109,53,205,42,230,81,29,189,236,114,19,128,8,56,203,28,97,71,169,230,182,23,159,254,181,78,27,25,67,19,23,42,50,82,75,
177,85,104,84,164,210,168,173,113,56,20,160,82,138,48,107,164,196,49,255,0,215,167,80,41,192,119,160,4,3,175,165,56,14,244,127,42,81,64,11,220,115,74,7,190,104,31,133,58,144,192,125,41,195,175,52,10,48,
51,199,227,64,10,0,35,158,212,162,142,254,212,15,203,214,144,1,246,165,245,163,161,206,105,64,245,197,2,19,29,189,169,203,211,154,0,246,167,99,154,96,3,35,165,41,3,165,47,29,122,210,245,237,72,4,198,105,
113,207,90,92,81,200,237,64,131,30,244,188,82,99,189,56,103,182,105,216,4,246,167,81,205,56,26,6,32,165,29,41,64,56,227,165,42,140,246,160,44,32,31,90,112,28,80,5,59,30,131,52,0,99,3,181,47,189,40,25,
205,40,160,4,29,115,78,2,140,103,233,252,233,192,15,74,0,76,30,49,197,40,28,245,165,31,67,74,1,245,160,4,29,61,169,216,227,218,156,189,169,192,113,147,140,208,33,20,82,129,206,105,216,165,11,64,8,23,158,
41,193,120,247,167,99,244,167,96,140,230,129,141,11,235,210,158,23,138,7,74,119,110,40,0,0,98,151,250,210,83,198,40,0,163,3,252,105,113,232,41,69,0,55,185,230,158,14,59,210,119,206,41,79,227,64,7,227,
249,81,248,243,73,199,122,95,76,230,139,8,57,199,90,59,82,145,158,180,148,1,228,184,205,47,122,118,59,227,138,49,147,210,182,16,223,229,74,56,237,79,199,3,210,129,250,210,24,157,170,190,163,112,182,150,
19,220,50,72,254,90,18,4,96,22,39,181,91,0,238,227,53,194,248,230,247,89,176,215,24,67,31,219,52,207,179,133,146,209,35,33,148,156,229,217,135,65,211,24,231,53,228,231,24,250,120,60,51,148,229,102,244,
93,239,111,51,179,5,134,157,122,169,69,94,199,53,171,220,79,63,134,146,203,236,87,66,246,41,18,66,211,190,245,140,203,247,130,30,189,51,180,224,130,13,115,215,83,232,150,130,209,237,36,107,41,157,91,237,
17,21,102,44,220,2,219,135,13,192,193,29,250,122,213,198,185,125,91,72,184,143,80,158,250,56,209,188,184,212,72,84,71,156,20,98,192,100,225,128,27,106,182,147,171,89,203,167,217,60,211,90,177,134,225,
225,158,222,249,183,77,176,227,253,89,61,78,6,113,145,220,230,191,35,196,226,234,86,248,223,220,146,253,15,186,165,70,52,213,151,82,133,158,137,164,89,234,122,134,163,168,90,195,38,140,219,38,73,183,5,
72,163,110,184,0,246,193,36,147,145,211,158,149,112,233,218,110,152,154,151,149,8,22,197,86,43,119,105,194,172,249,0,128,192,156,134,32,117,252,123,214,54,168,144,92,11,205,54,234,123,151,133,97,205,162,
187,136,203,101,242,25,176,123,99,32,116,199,90,210,240,38,149,60,250,124,147,75,45,146,69,25,100,105,166,249,183,140,140,4,57,249,73,3,131,208,244,28,215,52,165,39,15,121,155,168,183,177,208,233,151,
77,53,174,151,119,109,27,60,48,249,145,179,50,109,145,212,176,97,238,121,207,36,227,142,149,35,105,198,239,195,151,18,105,87,113,148,91,255,0,179,206,165,55,73,20,108,73,42,165,190,94,62,96,199,219,21,
145,225,171,179,28,208,234,186,108,113,34,18,65,142,87,44,37,228,9,54,228,240,184,231,145,83,106,126,39,188,102,212,174,44,175,237,109,172,152,56,145,23,7,39,11,131,147,235,142,61,121,247,172,91,149,244,
69,90,202,227,224,212,45,116,203,217,44,167,211,229,251,52,73,57,220,240,148,44,202,113,30,57,233,220,142,227,189,62,27,221,54,220,27,169,97,118,146,228,226,92,184,95,57,64,37,85,71,83,223,142,1,29,122,
98,185,219,221,94,246,207,68,73,238,228,185,105,217,182,51,186,46,200,193,63,187,218,71,204,188,158,71,0,158,7,90,75,102,13,226,11,15,236,198,84,188,242,60,163,125,177,136,64,216,249,128,97,193,199,127,
122,181,30,103,169,22,119,54,52,249,114,117,59,141,55,79,48,88,133,19,5,154,45,207,188,227,42,67,115,140,47,110,50,43,51,90,34,60,93,91,218,71,109,171,223,204,36,147,204,125,238,16,227,37,114,50,138,70,
27,25,235,208,86,246,185,112,183,141,125,22,158,141,164,7,137,109,237,111,228,157,196,178,200,50,36,57,56,193,108,124,160,12,3,156,147,154,207,240,183,132,173,30,117,17,222,89,95,73,247,26,113,49,64,163,
110,9,42,221,14,226,6,210,120,239,90,69,43,189,127,224,149,107,43,178,181,229,204,182,250,95,153,21,224,119,120,1,249,84,49,99,158,121,60,13,163,29,51,159,210,160,177,138,231,92,184,184,111,179,69,36,
183,50,43,172,80,157,225,95,24,59,148,116,5,87,56,232,49,156,102,179,38,211,37,134,123,184,46,46,226,147,236,242,98,72,150,110,87,45,198,222,62,99,193,227,183,21,165,124,183,145,92,94,173,164,127,101,
105,39,77,166,220,49,102,92,142,146,125,15,60,14,135,56,53,110,138,133,245,42,41,90,232,233,26,59,157,71,90,211,247,37,173,181,165,132,102,25,139,238,27,58,178,146,189,242,112,63,30,107,18,27,203,11,203,
41,237,245,0,165,174,148,44,146,219,202,202,254,102,230,37,112,56,218,191,47,92,231,158,213,38,185,103,115,168,218,221,132,157,227,138,15,145,97,50,54,90,44,224,150,219,215,56,206,27,32,215,63,115,97,
106,186,116,17,146,191,105,76,52,137,231,19,243,19,180,225,64,224,119,224,224,231,147,89,69,38,181,11,38,61,39,176,210,146,40,44,154,55,50,31,46,11,162,238,194,18,113,146,87,167,61,50,59,138,179,116,215,
23,246,183,17,92,62,245,71,71,150,112,21,81,24,3,180,28,117,206,51,210,185,233,140,54,215,129,18,23,242,97,112,102,87,110,64,221,128,14,56,234,8,4,86,207,133,160,91,184,118,198,12,145,190,246,49,128,54,
147,147,131,38,15,205,128,126,156,250,138,213,197,124,67,140,35,173,206,183,192,178,93,199,34,72,145,24,236,99,80,215,0,31,48,18,58,228,49,3,7,25,3,61,56,227,60,250,142,151,173,221,157,66,107,47,41,39,
180,141,129,23,210,73,134,101,111,155,121,64,50,61,21,49,201,227,60,87,8,154,180,223,98,146,218,210,75,108,66,140,214,182,136,134,66,91,0,224,244,200,192,199,67,201,61,64,172,125,63,198,232,182,27,181,
104,162,123,153,230,12,109,237,208,43,48,43,183,46,220,244,201,253,49,93,184,44,124,176,210,230,166,218,111,183,249,117,56,177,56,120,86,86,148,110,125,3,112,201,110,29,165,145,21,23,171,51,96,123,115,
94,39,241,94,91,214,241,45,165,165,210,180,11,42,121,159,232,131,62,122,169,0,100,158,164,18,62,95,122,167,226,207,26,182,167,163,46,157,149,16,1,181,18,64,172,206,187,130,156,159,226,32,30,9,250,214,
60,222,36,146,102,182,188,212,218,41,46,109,163,242,213,34,203,121,39,162,54,15,5,248,201,56,199,74,250,44,110,127,71,48,195,251,41,193,197,171,117,235,215,79,200,242,112,217,85,76,45,111,104,154,107,
94,133,175,10,94,197,104,255,0,105,91,134,142,244,180,176,60,140,187,86,5,0,29,184,39,229,113,142,164,112,79,182,43,215,7,138,108,44,52,223,46,228,192,183,95,242,194,37,144,147,130,185,80,249,231,254,
5,220,215,130,77,127,109,40,136,180,139,47,153,146,92,199,151,103,111,155,158,249,201,35,222,186,13,35,89,186,217,246,171,201,18,73,124,183,140,33,27,157,72,39,104,43,142,64,39,129,219,222,188,12,54,59,
17,133,169,207,73,180,122,181,240,148,235,195,150,106,231,208,26,61,223,219,244,245,187,242,36,129,93,142,196,144,97,136,29,200,171,191,40,43,185,209,73,56,93,204,6,126,153,175,59,179,241,25,134,210,207,
80,213,175,238,160,91,201,35,85,89,211,253,68,0,103,146,7,82,125,107,62,246,107,189,75,196,86,146,201,115,230,219,67,3,201,27,231,203,97,24,1,28,236,254,13,196,228,147,146,84,112,57,175,187,193,231,114,
120,88,233,239,249,187,191,157,146,232,124,205,124,178,213,223,72,249,30,170,57,0,227,168,167,126,181,143,99,169,125,191,85,16,233,162,25,116,171,101,43,115,56,144,22,89,7,11,16,3,243,39,211,3,215,27,
62,149,239,225,177,17,175,14,104,234,187,247,244,60,154,212,93,41,89,238,6,151,156,244,205,46,59,103,20,184,174,131,33,56,167,40,244,235,64,167,98,128,16,10,120,30,212,157,233,216,252,232,16,130,148,14,
180,160,115,74,104,1,61,59,210,140,211,133,24,252,232,0,199,175,74,95,195,138,81,205,46,56,160,4,244,205,56,14,69,39,165,56,80,2,17,154,80,41,113,201,163,4,208,2,1,143,165,56,14,41,64,226,151,29,205,0,
32,224,102,156,6,79,61,232,20,184,231,233,64,0,239,75,138,41,216,199,210,128,16,83,177,210,128,14,113,78,247,160,3,158,41,113,207,90,0,234,105,216,160,4,198,57,29,253,233,192,30,41,64,199,90,82,56,160,
64,0,199,20,160,102,148,14,5,60,14,115,64,198,129,211,61,105,227,52,160,26,93,191,231,52,0,131,174,113,78,3,173,46,57,167,0,40,0,199,165,40,28,119,167,125,40,160,4,193,252,13,46,7,3,189,40,20,160,113,
64,0,224,210,158,162,133,28,210,254,20,0,116,20,118,165,230,156,7,31,206,128,19,235,75,239,138,80,56,160,250,80,2,17,219,20,116,20,180,114,105,136,65,72,7,61,177,79,199,20,157,186,115,72,25,229,24,197,
46,50,5,41,28,210,173,106,194,229,25,47,237,19,80,22,47,43,45,198,207,51,5,14,208,160,18,78,238,157,143,233,76,151,83,211,227,10,90,246,18,90,65,18,0,249,220,228,100,1,142,188,115,199,108,213,127,20,104,
255,0,218,246,81,70,151,115,90,92,69,32,120,166,136,242,190,163,29,8,35,177,175,19,241,101,222,189,164,235,3,65,183,97,115,112,144,169,182,182,182,218,188,128,118,179,99,166,221,164,227,56,57,175,3,31,
140,199,225,42,221,69,74,15,109,53,244,209,254,140,245,112,152,108,54,34,60,183,180,151,159,226,116,158,56,248,133,121,164,202,186,101,205,191,217,47,210,236,249,211,66,141,34,172,0,229,89,65,0,146,113,
200,235,201,198,120,53,193,197,172,195,170,207,171,189,174,161,122,36,36,200,130,70,27,174,87,0,109,0,252,196,158,120,29,135,181,112,87,250,187,223,89,76,151,158,101,214,167,112,225,190,213,60,155,138,
224,156,5,61,123,156,131,192,207,21,185,164,106,218,92,22,208,173,254,159,228,106,131,106,164,241,169,28,238,5,93,215,169,233,128,71,78,190,181,249,238,99,137,171,138,105,205,182,150,221,108,125,110,19,
9,10,17,209,106,247,58,249,110,180,200,45,252,137,205,242,196,178,44,145,68,173,181,200,28,21,207,28,30,78,127,16,51,89,23,55,214,179,207,52,246,145,207,111,102,249,69,89,176,166,68,126,9,227,161,232,
56,235,89,186,178,249,183,115,92,220,8,237,217,238,25,112,138,194,53,27,55,224,99,57,28,129,233,146,121,200,197,92,209,110,94,241,30,97,111,115,44,146,67,178,37,133,64,10,220,150,12,57,220,91,142,70,49,
131,215,53,229,251,23,163,238,118,164,186,157,94,177,246,69,189,179,186,154,27,77,94,1,10,44,162,86,9,113,24,229,74,147,142,153,236,115,200,246,169,172,202,193,113,19,89,159,54,217,225,30,82,91,74,175,
230,54,9,0,150,56,252,185,227,138,168,64,181,182,123,57,197,130,92,206,139,113,36,182,178,135,222,63,133,119,140,146,195,0,3,193,227,7,52,92,203,29,220,91,172,118,109,251,62,68,242,226,49,19,161,28,34,
144,119,19,146,115,88,202,148,224,236,183,252,63,49,221,45,142,43,236,178,105,50,195,253,165,52,230,246,111,223,65,2,35,103,4,150,39,126,120,109,192,130,58,142,114,57,21,181,166,218,222,107,112,219,221,
221,73,28,63,106,156,60,87,76,185,138,52,235,146,7,222,60,241,253,234,154,246,11,5,153,254,213,43,223,74,145,197,36,5,164,39,201,218,119,54,242,113,147,243,3,223,145,201,168,53,24,190,215,176,92,106,13,
246,215,156,109,137,78,196,216,196,226,76,142,248,95,110,188,85,185,236,186,141,61,52,42,220,181,236,110,98,185,188,197,161,76,187,172,69,66,39,85,220,164,253,206,224,243,140,28,115,93,57,189,134,214,
9,109,35,83,60,165,209,151,43,184,109,42,2,156,12,156,30,190,189,177,70,140,229,52,255,0,58,235,116,169,99,19,152,98,117,87,218,3,245,98,127,131,7,167,39,145,138,206,138,75,75,75,216,46,237,163,105,60,
199,121,36,184,154,66,11,169,224,128,63,133,50,120,94,15,3,154,135,52,211,86,216,46,173,169,209,217,220,167,151,52,43,8,189,101,68,116,6,81,195,231,230,11,187,238,241,158,159,74,110,161,165,217,91,219,
94,239,181,149,39,105,93,145,18,79,45,93,152,2,0,35,32,49,4,156,244,200,193,34,177,173,180,230,184,142,57,45,238,214,89,99,44,238,210,202,2,192,222,102,229,0,99,39,219,57,171,126,36,214,172,161,177,186,
107,107,197,185,154,117,62,84,81,177,97,9,192,25,249,135,43,130,221,59,142,216,169,139,146,126,239,81,114,221,106,98,222,75,45,204,19,75,114,240,152,109,230,88,109,145,89,66,101,114,219,119,142,78,6,58,
243,131,197,69,97,45,245,228,115,68,151,10,242,164,15,34,249,155,135,150,156,22,10,199,176,235,239,208,243,84,52,216,162,8,243,223,218,199,53,148,160,41,199,0,227,174,49,206,70,65,36,99,62,245,209,233,
250,76,77,12,50,56,184,75,35,185,161,85,99,178,224,142,112,121,220,0,97,90,77,197,43,11,148,177,225,184,174,111,97,241,6,163,164,67,53,228,118,118,194,221,126,210,255,0,52,187,184,144,133,207,11,247,78,
57,199,24,25,174,115,65,211,236,47,117,203,123,73,183,90,234,45,152,201,157,11,35,72,126,232,35,183,110,79,114,43,208,224,184,142,102,177,130,51,21,171,153,188,203,121,160,140,175,155,212,190,87,141,205,
184,245,62,220,145,80,199,165,91,195,169,73,170,92,189,141,197,218,177,242,100,150,96,29,192,24,12,84,123,99,36,242,56,246,52,161,81,43,176,189,153,193,29,7,82,117,15,127,11,52,114,179,9,4,78,71,204,62,
240,44,57,200,224,156,146,69,119,255,0,97,125,39,79,153,102,99,61,198,161,242,78,176,74,137,229,224,16,55,96,96,12,40,25,92,28,159,92,154,161,170,234,113,105,41,44,72,141,25,146,86,0,73,33,89,51,180,100,
0,14,64,61,61,121,247,174,123,254,19,45,66,109,54,230,218,88,162,180,19,78,162,45,184,84,142,94,132,99,186,144,70,122,115,130,48,105,169,58,138,201,21,204,173,170,23,196,190,35,186,131,80,146,222,218,
205,108,45,248,50,194,164,73,208,101,122,14,9,7,28,118,39,165,121,253,238,165,45,196,252,124,136,0,8,160,159,221,131,206,1,244,25,174,171,74,182,146,229,53,107,17,116,194,237,142,214,24,194,110,36,109,
222,195,36,12,147,201,227,211,166,42,132,222,19,72,174,173,230,147,81,143,236,210,74,18,70,131,46,98,93,191,55,160,36,30,49,208,240,115,218,182,166,225,23,102,66,106,229,91,8,46,34,16,204,109,192,70,38,
40,129,144,102,70,3,126,0,206,65,199,228,13,77,37,172,223,105,158,227,112,16,128,37,147,115,146,14,120,30,156,231,2,181,167,210,181,75,123,123,144,44,227,254,205,141,85,214,127,190,174,248,194,128,114,
78,72,201,228,99,39,175,74,219,210,224,75,45,53,100,185,93,142,233,242,229,188,194,168,79,202,219,143,114,119,13,189,115,199,78,106,249,244,230,136,222,165,205,11,69,134,239,73,254,214,154,41,66,164,107,
37,188,67,1,163,100,56,109,195,163,183,13,199,108,119,32,86,183,151,108,247,119,55,42,187,109,213,146,103,141,211,127,148,72,43,229,129,145,131,149,57,36,224,12,122,213,4,123,93,52,173,230,249,110,36,
186,59,254,204,178,110,242,240,112,167,36,141,204,199,140,118,4,213,75,152,22,233,18,217,211,251,51,75,185,128,76,215,19,147,137,20,176,249,207,160,45,140,100,246,227,34,186,170,85,169,63,129,47,235,171,
254,145,140,82,142,237,157,61,134,175,123,246,157,144,207,4,154,100,155,229,184,251,59,51,179,64,23,230,32,100,239,61,6,51,158,73,7,140,86,196,49,69,97,29,219,53,208,183,43,18,51,135,218,112,174,65,86,
126,115,242,240,160,113,238,15,90,194,135,205,22,146,67,99,23,153,105,167,41,23,64,40,133,148,16,14,220,156,109,108,50,144,59,14,79,92,30,227,69,48,172,173,61,244,94,106,70,235,50,11,117,243,1,14,0,17,
179,14,128,2,62,163,154,244,240,24,122,154,202,122,221,61,119,245,235,247,28,56,154,144,73,69,111,117,161,177,30,174,246,162,70,179,134,222,67,46,200,224,130,99,229,22,96,126,103,102,232,73,4,96,99,60,
26,236,200,228,96,96,227,159,175,181,87,22,208,49,84,145,99,154,72,79,14,85,75,14,227,36,122,3,193,244,171,43,243,97,129,221,158,115,156,230,190,235,44,195,74,148,92,165,83,154,255,0,215,161,242,184,234,
208,169,43,40,114,216,0,227,20,224,57,233,64,24,235,74,43,212,56,3,138,92,122,82,175,78,148,163,160,160,4,2,151,183,65,78,192,165,30,148,92,67,113,74,62,148,236,80,58,251,208,1,211,154,92,118,227,235,
64,233,74,61,123,80,1,75,138,95,167,52,189,79,61,104,1,0,246,165,199,34,148,15,202,151,28,122,80,1,218,151,30,212,119,198,105,123,210,0,29,169,192,115,211,165,0,102,148,15,65,76,5,252,59,209,142,104,0,
210,226,128,12,116,235,64,20,224,41,122,10,0,20,125,105,212,128,83,135,214,128,12,103,240,167,1,131,199,90,6,114,61,41,223,74,4,32,226,157,142,105,64,246,165,198,49,64,192,10,120,29,105,163,128,42,76,
119,160,4,199,62,152,165,30,244,83,148,126,116,0,14,184,205,59,189,32,167,125,77,0,40,207,231,222,148,80,189,71,90,119,52,0,131,173,56,125,104,0,241,78,29,104,4,38,208,41,113,233,75,211,255,0,213,64,20,
0,131,218,156,49,211,189,46,7,110,180,17,222,128,3,212,226,142,113,197,59,7,20,157,201,20,192,49,237,64,24,160,3,233,75,207,65,214,129,0,252,77,33,25,233,197,40,25,233,214,158,7,52,129,158,75,142,115,
222,151,166,41,194,160,189,18,181,156,235,4,162,41,138,29,142,71,221,56,235,90,73,242,166,193,106,76,65,205,115,62,42,91,251,11,73,238,180,29,46,210,107,185,201,107,150,113,130,112,188,19,199,205,208,
47,229,233,94,109,123,241,35,86,208,238,44,172,53,59,37,184,242,156,45,237,204,99,168,201,225,51,193,224,3,159,175,165,107,248,159,226,190,149,101,106,23,79,130,73,239,36,68,120,149,212,224,171,12,228,
227,191,78,43,198,169,156,224,165,78,92,211,229,107,79,63,145,232,71,47,196,198,113,180,110,159,220,120,198,163,166,173,236,247,119,83,69,19,222,125,163,115,90,188,136,21,24,146,31,1,123,2,87,229,62,156,
231,56,164,241,102,140,235,99,107,121,115,38,253,209,136,165,129,80,49,115,206,25,85,70,118,158,249,231,208,81,127,226,121,239,154,250,89,85,77,197,221,218,51,176,140,2,241,175,204,160,158,204,24,31,195,
25,206,42,221,184,146,235,80,109,78,210,68,69,51,49,103,50,97,155,185,1,64,36,140,116,39,29,235,225,177,19,165,85,254,234,246,210,255,0,171,243,62,186,130,171,20,185,213,187,12,49,90,45,188,246,211,172,
207,40,85,142,105,102,118,86,80,192,54,56,24,193,225,118,158,188,86,173,190,161,165,233,50,92,73,107,97,52,214,174,66,125,178,39,118,86,60,12,47,97,140,48,245,60,85,235,136,162,210,60,61,52,151,11,12,
48,223,195,189,165,45,184,74,229,177,128,223,194,135,29,127,150,113,82,106,39,193,186,107,183,246,253,237,206,101,82,209,197,102,193,85,142,7,203,176,28,146,8,28,227,35,62,230,159,212,227,86,54,85,18,
93,111,165,183,249,254,31,50,157,121,67,226,139,111,203,91,156,219,217,234,31,105,125,69,33,45,99,36,130,88,100,141,11,191,150,79,30,98,125,229,32,231,59,186,96,243,138,214,214,181,111,38,36,133,218,206,
22,9,149,217,56,103,61,219,28,29,185,61,171,19,93,212,100,188,190,12,247,214,230,63,53,101,181,186,133,188,173,131,110,214,87,96,192,20,25,231,35,215,174,77,115,55,44,243,95,73,109,117,43,125,160,72,21,
102,114,119,35,125,15,80,114,57,56,199,210,188,140,69,8,65,242,197,221,127,94,108,238,165,46,120,171,171,29,70,150,182,174,202,39,188,128,15,227,89,67,22,37,250,48,3,157,189,70,239,99,90,218,53,132,87,
19,220,25,46,4,177,71,33,242,212,161,6,101,3,31,128,224,118,207,183,53,230,183,55,83,11,179,29,187,180,76,132,198,36,70,251,199,248,135,167,61,56,173,109,7,91,187,142,207,81,102,89,100,178,16,19,114,118,
174,230,228,124,191,49,0,3,146,24,250,99,138,231,149,41,53,238,151,123,108,119,215,179,253,134,9,45,224,182,150,53,185,14,238,138,21,114,184,10,113,150,251,184,235,142,195,166,107,41,116,249,231,150,214,
230,85,112,8,41,52,177,191,57,3,190,78,2,244,252,189,107,51,77,186,105,94,246,27,187,86,155,65,142,93,130,72,208,204,22,85,85,101,141,59,170,224,142,113,129,207,92,241,121,117,63,180,91,193,13,149,188,
200,82,228,49,149,28,151,219,183,229,203,47,25,44,48,123,99,241,172,149,25,165,116,129,232,108,174,169,20,58,114,44,214,65,237,160,249,166,22,238,3,48,199,240,169,224,245,235,159,198,176,245,155,197,88,
152,67,44,34,217,37,10,142,9,49,202,66,6,36,131,200,35,56,32,113,207,90,138,41,6,163,112,100,179,62,86,154,228,193,36,107,144,89,127,139,44,114,23,56,60,118,200,53,65,52,75,111,179,95,77,119,116,208,206,
132,181,172,5,73,50,39,98,112,114,199,3,30,153,230,148,84,99,190,229,38,145,179,165,56,212,174,147,82,54,77,5,178,178,219,8,99,182,116,134,68,0,22,41,198,213,221,243,12,147,129,208,99,147,93,235,189,133,
174,142,46,45,244,198,140,32,121,22,237,217,113,18,169,199,114,57,29,54,14,114,58,115,92,30,149,123,115,44,122,100,86,51,220,66,16,121,101,22,66,1,76,130,27,4,237,1,121,0,142,113,93,51,105,186,141,196,
151,58,125,206,166,36,158,120,136,100,4,52,97,178,50,202,72,0,228,0,125,136,205,103,85,165,160,220,209,165,168,171,92,219,44,196,121,144,18,28,71,19,9,102,144,147,206,88,49,216,61,23,234,115,129,81,234,
50,71,160,73,107,115,117,109,28,114,92,43,42,186,182,246,68,4,2,65,35,31,197,157,205,207,21,95,71,176,180,210,109,111,52,216,227,154,59,137,46,182,207,120,202,14,192,20,149,216,7,163,96,100,30,153,7,138,
142,249,238,47,90,59,117,45,36,133,84,219,60,112,147,215,150,45,198,70,64,57,39,39,233,138,193,180,165,101,177,156,172,115,186,253,237,237,141,253,196,151,170,209,139,136,243,20,172,160,150,94,118,158,
224,176,224,87,51,165,233,250,164,177,67,113,107,44,123,10,25,84,205,48,249,240,48,221,65,245,35,30,181,232,82,91,92,196,130,214,123,107,75,169,35,148,203,12,111,137,30,61,199,145,134,198,71,205,193,29,
125,170,158,185,225,93,104,172,115,233,122,89,158,212,74,3,36,10,174,35,44,191,120,160,60,227,3,161,207,3,140,115,93,81,172,190,21,111,208,74,205,152,190,31,137,158,75,203,105,173,154,219,85,85,202,221,
139,166,95,48,103,42,141,26,157,142,8,28,30,188,246,32,26,98,70,211,94,136,164,150,43,107,56,129,5,220,101,34,102,4,96,143,94,49,88,150,26,118,169,53,204,182,205,59,218,236,102,79,222,2,165,164,86,218,
16,14,160,147,145,159,111,74,223,211,45,109,180,251,171,116,213,13,188,178,220,194,101,67,43,57,91,98,51,184,149,28,177,206,209,159,94,213,110,43,155,114,210,53,239,18,123,205,53,167,107,104,109,96,137,
145,140,136,165,176,164,96,124,167,162,224,31,199,233,89,90,50,88,79,168,201,53,245,220,194,214,23,121,254,232,38,70,86,6,51,35,19,128,112,1,3,219,156,17,75,96,97,212,214,238,107,151,9,109,36,110,100,
46,27,100,74,126,85,105,0,249,67,124,189,72,227,38,153,30,166,154,29,181,134,156,101,130,228,67,27,71,36,98,32,119,46,70,215,221,143,152,186,145,147,252,59,77,56,123,169,164,85,173,208,235,117,49,98,124,
68,67,199,56,177,3,41,186,17,153,85,176,219,74,227,56,251,223,66,6,14,42,212,154,181,191,136,46,245,29,81,111,151,76,129,35,242,11,42,181,201,62,95,10,176,160,5,119,40,25,0,231,39,56,28,115,159,225,181,
184,215,35,91,168,239,77,164,16,168,97,109,25,243,119,170,158,3,49,7,169,199,3,211,222,186,121,254,220,201,108,214,133,140,155,212,203,28,112,249,77,25,99,140,6,231,114,99,161,254,16,78,73,173,35,137,
148,29,180,249,153,74,17,111,87,177,212,89,92,105,107,165,165,170,220,199,117,13,219,172,44,76,63,235,153,190,246,252,242,91,157,221,120,228,26,175,101,121,21,158,175,168,199,105,60,86,50,170,7,99,4,187,
183,109,108,32,60,13,163,159,211,182,43,156,189,130,120,98,142,245,146,59,56,231,145,163,154,225,101,251,65,70,222,67,8,192,39,156,243,187,223,53,53,142,158,32,187,89,14,171,190,213,230,70,147,98,135,
119,24,7,98,100,28,19,144,79,226,43,174,89,133,74,146,87,178,251,222,159,141,190,70,11,15,78,41,219,83,185,184,107,201,45,224,183,145,161,77,48,207,243,60,19,44,109,118,252,49,220,127,133,122,228,247,
198,59,215,161,196,16,68,158,94,223,47,104,219,183,238,227,182,61,171,202,244,187,83,168,223,89,205,119,246,153,108,173,110,36,13,109,18,29,170,219,130,128,195,141,192,112,79,97,94,178,20,40,85,80,21,
84,109,80,58,0,58,1,95,107,195,80,171,21,62,123,91,109,219,127,240,223,51,229,179,169,65,184,242,239,253,126,35,113,210,156,6,61,105,113,211,38,148,87,213,158,8,218,112,20,225,215,154,49,64,7,229,74,57,
165,192,227,34,129,64,128,253,41,192,115,64,30,180,224,58,122,80,49,191,78,180,99,142,148,240,58,102,140,80,2,1,129,237,71,225,78,3,7,154,92,10,0,64,49,75,138,81,215,29,233,222,158,148,0,220,127,156,82,
143,167,90,83,210,151,30,212,0,1,206,113,78,20,123,245,165,20,0,189,251,226,128,7,56,165,20,160,115,219,52,5,192,0,14,41,113,207,74,92,82,128,40,16,159,206,151,20,225,158,56,163,190,40,1,64,252,105,105,
64,61,250,82,226,129,128,20,224,6,49,64,7,52,225,211,154,0,69,20,236,80,7,6,158,7,106,0,110,220,83,177,254,69,40,24,227,52,160,113,64,8,62,180,224,56,30,148,189,185,167,1,158,212,0,0,105,192,123,10,41,
77,0,24,192,165,3,190,104,3,222,151,235,64,9,142,105,216,162,151,28,100,211,0,160,115,245,197,3,161,165,30,188,14,40,21,197,28,208,7,165,46,61,233,192,80,3,118,140,82,99,154,147,30,163,154,49,131,218,
128,35,2,164,3,138,0,205,60,41,244,160,15,35,3,160,61,43,11,197,122,36,186,178,91,53,190,169,115,99,61,179,239,86,135,4,63,170,176,61,65,232,113,131,142,226,183,255,0,157,40,21,85,105,70,164,121,100,180,
28,37,40,75,153,110,124,169,169,120,127,196,80,77,37,133,245,168,104,108,101,146,65,113,114,118,194,80,231,104,86,228,160,29,0,231,174,61,235,153,251,30,167,115,153,218,217,252,253,162,73,38,192,253,214,
58,32,35,128,115,219,168,175,173,124,79,162,233,154,212,105,111,168,201,38,60,183,1,82,114,153,83,193,200,7,145,211,173,113,150,186,111,133,162,240,100,112,233,86,171,48,177,153,165,130,43,150,101,50,
74,167,248,207,83,147,200,254,247,189,124,150,43,135,40,202,165,249,237,167,125,127,46,136,250,44,62,113,37,11,56,254,31,240,79,157,44,244,167,150,105,110,94,113,20,8,55,23,192,36,103,185,31,95,230,43,
84,89,221,91,91,220,92,180,119,31,101,183,144,36,184,137,130,174,72,25,57,224,140,246,175,98,215,188,58,52,253,7,251,104,155,141,33,124,229,185,186,138,24,4,242,204,248,192,12,160,16,160,119,3,39,175,
122,224,252,87,30,187,225,93,58,246,45,91,101,238,131,168,218,180,22,168,147,177,42,205,134,29,176,48,57,231,147,234,59,249,184,140,162,150,22,55,154,118,93,122,121,93,111,107,232,122,52,179,41,85,118,
131,87,237,249,235,181,206,119,75,212,244,253,51,87,134,7,150,41,45,139,31,50,233,100,96,185,9,196,74,171,199,212,182,112,71,165,90,213,252,83,166,71,12,246,26,42,21,182,148,47,157,113,11,24,228,145,128,
97,180,41,232,8,99,198,120,35,223,142,114,93,67,79,95,11,93,216,67,101,39,218,46,176,207,119,36,159,36,97,121,225,64,228,228,12,142,62,180,154,134,130,246,30,67,50,162,91,221,66,179,34,199,41,153,209,
72,29,78,6,79,57,192,236,112,51,138,242,235,212,110,147,140,18,113,234,210,218,253,46,255,0,175,51,182,156,61,251,205,180,253,119,243,178,57,235,201,158,53,82,138,230,53,98,67,18,58,158,231,29,241,197,
54,37,50,92,198,205,33,18,149,206,84,242,79,160,247,197,50,235,236,241,205,136,48,172,9,4,224,168,35,232,121,7,218,164,182,101,70,136,199,186,77,199,46,168,48,64,207,221,4,114,50,51,205,121,77,232,122,
10,230,168,157,108,33,183,212,98,150,3,44,103,133,154,21,146,60,1,144,49,140,48,199,4,26,154,205,37,213,172,204,211,166,157,167,219,203,43,188,79,21,144,72,89,202,231,104,8,126,94,252,224,168,252,113,
82,89,219,234,183,142,99,180,138,59,91,98,14,124,233,65,84,7,243,203,113,215,25,61,200,174,151,66,240,190,161,127,166,203,112,222,33,182,37,35,62,116,114,132,253,203,147,202,178,227,131,140,30,49,187,
43,130,51,88,202,164,98,174,217,87,107,118,101,90,106,246,54,51,220,53,188,19,219,90,205,2,171,71,231,121,176,153,128,10,236,172,57,41,133,24,206,88,115,219,24,143,77,51,93,90,42,136,200,210,222,80,187,
35,124,74,248,232,204,221,20,125,71,114,59,213,175,18,248,99,87,182,50,252,139,45,156,134,51,39,217,227,84,118,56,200,100,66,73,4,247,7,211,243,201,240,205,228,150,182,146,171,201,7,153,25,253,218,75,
129,180,244,194,241,203,103,128,15,122,110,179,148,45,23,116,129,197,59,205,110,118,218,48,179,187,189,91,37,154,72,163,137,64,133,54,128,72,193,193,108,141,184,192,35,36,100,241,90,214,250,93,204,87,
51,91,139,82,229,212,132,145,164,221,26,38,114,65,29,142,126,238,127,10,225,98,158,225,38,146,235,80,148,130,131,104,141,164,196,140,71,99,220,16,65,227,219,21,175,107,226,248,228,210,54,181,179,249,174,
134,41,228,141,207,239,143,92,184,233,158,252,114,59,87,37,74,50,123,43,145,165,142,139,86,101,211,68,17,188,13,182,86,196,18,132,27,100,61,202,231,145,215,29,125,234,221,181,253,236,86,150,247,115,218,
20,179,182,38,1,22,192,75,46,51,177,143,92,114,6,78,56,174,115,195,215,119,247,119,159,99,192,188,182,150,100,17,76,50,33,65,252,67,44,112,24,40,234,72,226,155,226,107,230,178,214,100,177,186,186,120,
173,3,240,82,64,4,152,39,3,113,25,239,159,229,81,236,36,236,158,172,167,31,180,116,99,77,107,155,184,98,211,245,133,190,127,47,206,150,217,85,195,219,158,62,94,126,240,249,177,187,156,250,84,154,133,204,
16,69,52,151,183,115,45,220,50,2,138,141,183,120,35,45,145,233,180,128,15,92,147,145,92,205,190,180,109,109,243,14,182,20,2,26,219,236,241,128,170,167,146,119,15,155,1,187,18,79,122,200,242,93,166,251,
116,122,156,210,79,54,75,156,101,152,147,206,15,67,215,159,79,194,167,217,57,61,93,133,163,118,71,101,101,36,119,154,61,192,189,131,107,249,43,20,113,196,75,180,67,130,160,156,100,30,231,62,221,43,66,
61,118,246,202,202,91,100,180,143,247,6,54,140,238,96,17,115,199,124,16,114,125,122,215,63,162,220,216,190,150,175,13,228,141,112,177,156,67,254,172,175,28,124,219,142,227,140,146,56,226,186,75,11,125,
77,180,213,186,212,172,96,146,9,25,89,28,0,22,72,246,231,32,169,193,207,7,30,160,116,205,99,82,14,47,85,160,220,26,86,102,126,183,107,105,170,107,113,207,61,201,118,132,164,215,3,115,72,94,66,48,9,97,
192,228,1,183,30,157,137,168,164,209,180,150,190,210,74,172,177,106,45,27,153,102,101,49,228,6,36,124,141,144,216,231,36,99,175,2,167,187,136,92,106,146,79,107,228,90,233,229,4,34,230,224,249,104,66,242,
67,2,121,228,224,3,82,93,125,129,45,45,12,23,12,204,83,1,29,213,176,115,134,86,201,37,79,113,218,169,74,166,150,37,69,189,77,8,181,67,111,126,139,113,36,108,80,143,40,20,93,184,11,242,179,112,50,132,231,
241,207,122,171,15,134,244,187,253,92,92,93,233,240,52,140,187,165,145,228,242,193,235,232,114,216,3,24,228,227,28,241,86,244,27,235,67,106,246,87,191,102,146,121,224,97,2,221,48,127,37,123,100,48,206,
114,114,0,231,184,2,177,46,111,13,149,184,129,228,223,118,219,160,78,254,102,211,143,48,123,118,193,0,251,99,154,184,169,199,222,129,86,186,208,232,14,161,106,247,171,29,133,255,0,217,238,157,2,9,96,76,
112,9,109,155,142,49,220,238,31,74,165,105,58,221,64,211,92,222,198,182,106,190,89,49,54,38,153,201,200,71,246,28,225,113,220,87,53,121,171,203,113,185,133,184,72,165,149,91,122,0,137,14,70,24,5,28,250,
224,103,191,189,67,103,127,13,181,251,44,118,77,37,200,36,197,2,100,8,19,24,4,147,147,212,240,57,235,207,21,126,201,181,168,185,17,221,201,4,90,133,188,58,119,217,141,188,16,68,66,71,230,21,27,24,12,41,
29,55,6,201,206,58,102,180,124,22,247,23,254,38,91,87,123,136,101,179,145,99,242,6,215,81,199,86,32,228,99,11,200,227,230,226,171,104,73,21,170,203,113,169,74,146,253,177,4,22,168,211,152,142,71,14,248,
25,18,130,0,92,116,24,245,56,175,160,126,16,216,248,67,92,240,87,137,29,101,177,176,241,13,148,182,235,167,204,193,34,119,66,196,74,133,70,73,0,115,242,131,128,9,28,146,43,92,10,162,241,17,141,87,104,
220,138,177,154,131,228,213,219,66,174,153,99,109,97,106,176,90,194,34,76,150,101,7,248,143,44,127,19,154,186,7,110,213,191,255,0,8,206,175,42,179,216,193,6,165,26,228,150,211,174,82,227,104,247,80,119,
255,0,227,181,137,60,50,193,43,69,60,50,67,42,156,20,145,10,48,250,131,131,95,180,97,113,56,106,208,94,194,106,73,118,105,254,71,231,184,188,46,42,140,223,183,131,139,125,211,95,153,30,5,40,24,38,151,
165,40,21,214,113,133,28,83,135,95,106,81,249,80,49,160,82,227,61,169,216,25,165,199,83,64,132,31,90,118,40,20,163,154,6,3,143,255,0,85,40,20,171,215,129,78,235,64,13,199,229,70,56,167,129,159,173,46,
58,208,3,113,216,209,140,211,128,165,250,10,0,106,131,205,40,167,113,214,148,125,51,64,8,0,199,189,40,28,117,165,230,156,40,1,20,83,148,10,49,200,165,80,73,205,0,40,228,116,160,10,112,29,241,71,190,40,
0,3,154,118,56,237,65,28,102,151,165,0,3,176,167,99,138,64,56,201,167,99,156,10,0,6,51,74,49,156,102,151,28,154,114,128,64,160,4,28,10,112,250,210,227,181,40,25,237,64,9,138,92,115,75,143,214,148,15,122,
0,80,14,113,218,148,15,122,7,74,94,244,0,184,227,145,75,214,128,48,59,210,208,2,99,138,81,199,74,82,41,64,166,3,113,199,122,120,20,160,103,240,167,128,40,1,128,113,138,80,51,234,105,219,115,218,159,142,
128,127,58,4,52,10,120,7,243,167,129,197,59,30,212,134,51,31,228,81,128,122,226,158,112,14,41,164,119,244,160,87,5,3,181,56,1,72,41,217,227,154,0,242,16,57,52,58,229,72,245,24,205,72,138,78,2,130,196,
246,3,38,185,43,143,23,105,162,9,60,150,221,120,46,100,182,142,25,50,155,221,6,72,206,48,56,231,158,212,234,214,167,79,73,187,21,26,115,159,194,174,121,223,198,159,183,233,240,216,221,72,243,172,54,70,
77,151,49,171,126,238,60,12,22,32,241,156,242,122,113,210,185,47,1,106,183,122,35,61,228,86,190,109,195,34,129,4,131,239,231,156,134,63,116,245,206,121,193,226,189,35,227,63,136,34,131,64,181,91,43,237,
234,242,98,230,24,151,112,153,25,79,200,79,190,65,31,74,240,75,139,77,58,234,77,54,198,59,185,21,238,163,73,124,203,176,217,121,73,11,176,231,161,31,116,19,198,107,224,115,122,242,142,55,218,225,106,93,
175,157,155,211,79,215,183,220,125,110,91,77,75,13,201,90,22,76,245,43,223,136,186,133,221,180,86,207,99,104,254,72,34,239,207,220,164,63,76,169,28,117,62,248,175,52,241,31,136,174,231,181,75,75,192,161,
4,129,154,48,8,81,128,6,226,7,4,159,94,188,10,231,175,228,142,215,83,49,202,178,181,189,188,132,13,179,228,238,86,195,114,56,206,71,35,181,103,207,190,238,73,156,180,164,60,133,156,179,2,50,114,71,79,
111,167,57,175,26,190,63,27,81,56,214,169,116,122,84,48,56,106,122,211,133,142,179,195,246,246,144,218,220,192,66,94,182,166,128,71,19,0,124,182,82,72,61,120,61,112,71,227,81,221,233,250,146,196,237,4,
145,36,87,50,169,40,114,91,112,78,19,44,70,213,239,200,234,70,43,46,195,78,176,54,121,103,31,189,86,71,27,254,88,143,240,179,119,252,58,86,223,246,196,70,222,214,202,66,37,100,96,158,97,82,85,199,66,14,
122,245,60,250,116,174,7,82,164,173,24,200,235,229,130,119,104,163,39,134,96,183,138,51,125,44,162,243,206,38,103,92,24,18,30,48,204,253,143,106,183,107,29,186,189,188,80,88,192,96,24,104,218,241,68,146,
34,14,235,206,57,201,32,117,31,133,116,250,145,210,174,34,242,244,249,175,101,212,100,194,190,228,219,19,2,2,177,201,244,192,199,182,115,207,53,149,125,167,205,165,40,134,55,137,102,140,151,46,219,119,
198,24,225,73,101,57,40,70,112,56,57,235,214,166,173,25,198,252,205,63,71,161,84,234,70,90,236,117,154,107,104,218,38,145,37,213,193,205,156,209,150,14,248,2,54,102,194,149,126,84,100,19,193,61,243,215,
138,231,124,72,207,6,187,5,255,0,217,228,130,225,144,42,35,194,20,20,200,32,22,233,201,32,231,145,201,199,173,91,240,151,141,228,240,211,207,27,234,147,197,116,246,210,24,227,129,4,133,157,184,220,65,
202,199,140,158,79,39,181,100,94,235,55,90,180,243,182,150,223,102,144,40,107,151,229,195,185,108,4,85,28,150,201,57,98,126,94,245,199,10,19,115,218,230,207,150,215,50,181,109,86,229,149,98,147,204,140,
70,251,21,25,247,121,64,49,200,61,216,99,238,254,62,181,148,14,233,164,191,104,151,48,202,29,16,219,7,142,92,29,192,74,165,186,28,96,142,224,144,106,198,181,97,169,193,116,136,232,211,200,240,121,128,
168,225,151,28,144,9,201,97,143,78,123,102,166,179,179,212,47,44,216,89,192,38,116,69,73,10,72,160,178,228,224,5,234,121,31,83,93,212,240,178,191,42,78,254,134,78,170,135,188,172,117,49,105,190,22,241,
94,128,117,115,171,38,149,226,11,123,131,246,221,28,219,145,107,113,25,57,89,45,92,156,67,129,193,141,137,25,251,184,234,104,216,233,80,217,203,111,28,246,151,75,111,117,24,109,54,121,246,133,188,140,
41,57,4,124,167,142,70,48,79,166,57,161,32,190,179,70,75,32,36,10,194,35,25,70,150,57,155,0,177,4,240,10,253,15,57,21,171,101,103,173,45,228,173,121,113,155,123,149,105,30,204,218,137,132,129,99,102,80,
65,28,109,108,116,198,123,251,100,233,215,132,218,236,91,169,74,165,158,198,37,196,250,102,149,115,166,73,62,155,167,221,180,22,239,5,252,19,194,74,221,62,84,164,129,127,190,6,120,227,25,219,200,205,111,
220,217,52,47,230,206,109,225,186,96,215,16,180,82,5,116,12,156,54,121,4,30,129,58,2,14,43,145,40,117,217,211,54,119,49,107,86,241,39,157,104,172,242,133,8,63,121,47,207,130,164,18,159,47,32,3,198,77,
110,248,82,198,214,251,72,184,212,174,161,55,79,12,235,107,42,249,132,161,201,202,149,3,25,227,140,147,142,73,233,87,10,85,49,21,82,189,159,95,144,170,201,209,167,174,171,252,206,106,230,75,141,66,252,
223,249,137,44,184,2,87,156,140,183,163,48,0,1,154,142,70,54,86,152,128,93,155,215,101,50,50,54,216,227,33,137,216,136,6,79,27,73,102,228,17,242,227,38,186,155,89,238,244,125,66,242,238,211,73,131,236,
177,204,37,120,103,194,110,66,202,80,130,160,144,0,193,61,185,197,71,127,173,197,173,24,162,176,211,175,226,191,123,119,89,230,202,249,43,158,114,160,97,176,0,11,130,20,245,232,77,116,74,140,121,27,115,
247,151,70,153,148,106,74,234,209,208,171,224,33,45,216,214,183,233,243,106,48,221,162,64,251,36,219,185,183,130,239,180,240,28,46,10,178,144,203,201,25,206,43,160,187,179,190,182,211,237,230,134,194,
88,109,98,115,28,126,89,50,227,177,94,184,221,158,11,117,39,147,129,92,150,146,154,165,172,47,115,165,89,201,25,139,202,249,94,82,24,73,43,20,71,68,124,100,150,199,67,242,146,59,86,172,30,37,180,177,211,
175,129,184,213,33,213,32,101,138,43,89,100,243,85,176,197,100,204,152,60,130,9,201,35,60,5,220,122,112,86,165,62,171,70,116,166,167,165,246,58,121,44,1,180,186,182,183,152,162,74,195,23,6,78,38,140,170,
157,231,31,119,7,114,144,112,120,172,65,111,29,139,34,222,89,220,206,182,242,159,180,97,78,215,102,225,50,192,231,28,130,14,126,189,234,49,226,153,181,11,71,71,181,138,121,36,87,19,219,92,91,22,119,135,
35,238,12,3,34,128,57,244,235,222,173,106,250,174,133,62,159,10,44,193,96,89,113,11,9,11,170,244,251,236,64,200,192,207,183,210,185,189,156,225,43,91,66,109,169,153,170,155,91,34,218,132,83,255,0,164,
200,85,97,135,25,114,9,195,62,238,248,244,62,248,170,183,177,95,93,36,154,197,181,172,240,64,35,67,37,212,233,183,205,193,218,191,48,24,35,39,3,191,92,243,197,59,87,183,179,184,179,73,108,141,182,223,
63,202,17,44,131,228,29,216,247,218,123,28,117,233,192,170,246,169,117,52,43,111,20,55,41,45,188,130,56,144,76,193,73,244,41,247,75,103,167,4,231,210,183,87,177,94,69,173,92,222,193,12,26,116,104,144,
18,254,116,142,147,229,215,56,32,103,161,220,57,192,233,236,107,161,240,189,161,251,36,55,23,91,98,140,58,68,89,19,113,141,73,192,32,247,206,6,79,90,161,107,167,206,103,176,49,186,253,174,112,206,86,249,
118,170,236,251,217,108,28,245,60,30,248,29,235,165,187,120,137,180,183,88,48,11,33,17,134,85,221,38,50,118,159,238,241,192,39,53,18,157,149,144,142,175,74,178,181,71,82,210,220,77,111,28,133,11,195,14,
74,134,124,130,171,206,209,145,198,57,235,220,87,69,123,99,10,165,132,112,199,190,121,101,37,200,36,36,43,206,11,17,147,130,78,73,28,238,199,56,174,118,61,85,39,211,30,33,24,107,233,72,243,192,46,30,32,
7,59,78,56,231,249,254,53,151,107,120,192,150,109,66,72,208,198,86,210,9,36,217,230,56,3,105,4,240,62,96,58,144,43,134,113,156,165,176,238,227,243,61,243,72,210,60,40,146,232,246,49,248,171,83,210,53,
79,177,205,20,198,85,204,17,204,27,247,23,109,130,28,161,229,26,61,227,112,195,46,10,147,93,61,158,139,175,92,216,91,234,51,248,138,223,86,179,125,59,237,73,45,212,143,246,150,42,216,150,20,200,0,170,
110,12,141,147,189,15,170,146,126,102,209,214,230,206,104,103,213,12,239,228,187,60,242,200,23,115,54,6,211,147,247,64,57,0,143,230,107,215,252,19,227,89,97,212,90,227,82,183,154,56,110,247,194,18,238,
101,146,56,32,35,13,141,223,112,22,40,224,14,73,25,227,21,211,151,226,171,96,49,112,175,9,90,207,95,75,234,190,227,208,171,137,163,142,194,75,9,137,138,181,180,242,118,118,127,121,214,99,6,128,63,10,115,
169,71,42,196,18,56,36,16,65,250,17,193,160,122,116,175,221,163,37,36,164,181,76,252,118,81,113,147,139,221,8,62,180,224,56,226,140,17,75,129,158,42,137,15,67,78,3,147,70,41,202,56,197,0,52,15,241,165,
167,99,20,184,200,207,244,164,3,122,83,150,151,28,240,120,167,15,228,41,134,162,82,224,103,173,56,15,202,151,28,102,144,13,35,57,245,160,117,205,56,142,120,165,2,152,13,3,156,247,167,96,26,112,3,25,199,
214,151,30,189,40,16,208,41,121,199,52,240,40,3,2,144,198,129,219,210,156,7,62,212,160,123,83,130,243,76,6,227,138,112,28,245,167,1,212,245,160,12,26,0,0,237,75,142,112,105,64,245,167,10,0,110,7,78,249,
167,129,129,237,70,13,63,140,116,230,128,26,6,127,26,80,8,52,224,57,20,160,122,208,3,79,78,166,148,15,95,255,0,85,56,47,113,214,156,7,28,208,3,69,47,241,116,165,197,58,128,16,230,129,158,180,240,7,227,
75,142,121,160,6,129,78,193,205,56,12,158,41,251,104,2,52,20,253,159,141,56,10,118,41,128,192,189,41,225,121,226,156,7,20,239,74,0,104,28,82,129,205,56,14,148,99,189,33,11,210,142,212,170,49,75,138,6,
39,97,154,49,255,0,214,165,25,206,0,167,99,181,2,27,142,40,3,138,120,24,28,138,49,64,30,21,226,219,180,177,208,238,36,123,166,181,119,253,218,76,160,252,140,122,18,71,65,239,94,5,226,219,157,87,89,184,
33,229,181,176,89,221,166,220,114,191,105,41,132,47,145,198,226,49,141,216,207,21,236,62,58,211,133,234,226,231,81,251,12,113,198,179,203,130,118,121,139,156,17,234,112,112,65,235,197,120,238,173,171,
43,218,93,230,194,222,226,13,70,19,105,47,217,223,17,149,63,117,99,199,42,61,115,223,113,205,124,30,123,137,133,92,75,231,90,108,190,235,235,111,59,126,39,213,101,152,121,83,165,238,239,253,33,154,138,
132,208,32,26,53,244,237,115,117,229,219,58,54,209,185,71,203,212,241,200,227,35,167,173,114,77,224,233,47,108,53,11,227,169,90,109,183,145,81,247,57,101,148,183,27,23,211,104,198,226,115,244,239,91,214,
215,22,54,118,48,233,208,198,146,52,136,81,230,119,249,173,240,121,101,92,124,204,115,128,63,31,99,216,248,99,78,211,244,13,59,87,91,43,139,159,62,242,209,167,134,214,245,149,65,200,192,77,167,134,36,
134,235,200,32,242,43,203,194,198,53,163,106,150,217,235,174,157,180,209,30,165,89,202,149,220,126,238,253,207,38,184,209,244,120,226,184,182,142,206,123,139,215,64,139,178,114,137,111,193,1,215,35,50,
29,216,249,73,193,4,125,42,27,61,52,121,82,89,79,104,159,218,2,77,132,40,10,67,30,2,231,163,103,244,197,39,136,103,181,184,9,44,17,164,81,74,232,4,0,182,235,119,29,71,185,200,39,112,227,166,15,25,162,
210,41,212,173,229,163,205,36,16,237,86,155,97,112,142,1,56,45,206,27,7,56,61,65,239,94,85,120,201,73,171,223,208,239,166,249,146,43,248,130,56,116,125,83,236,14,129,202,40,243,26,61,200,25,72,36,43,119,
4,19,219,131,88,246,111,137,93,210,50,246,198,66,6,78,65,56,224,250,247,235,90,90,228,178,92,74,82,102,73,34,86,249,113,156,28,243,198,78,112,122,243,222,159,225,61,54,231,251,106,105,226,182,179,158,
227,78,180,154,232,67,36,184,89,93,118,164,113,227,140,146,238,8,28,100,168,244,165,78,246,75,169,105,34,125,34,245,236,213,181,25,109,230,123,56,88,91,59,198,202,3,74,202,91,110,11,100,225,112,72,0,140,
103,56,53,66,230,254,235,80,185,181,219,22,205,69,176,140,209,57,204,199,128,9,237,199,3,216,12,147,199,25,87,240,220,105,215,33,46,65,251,76,89,66,172,62,101,198,1,36,250,146,50,105,150,247,151,80,77,
112,97,149,97,73,226,41,50,109,222,178,39,7,99,3,212,31,195,165,104,169,70,50,247,208,115,221,123,167,113,4,80,203,98,246,183,178,219,25,237,110,29,174,68,22,251,133,193,31,42,162,200,14,29,185,32,144,
0,92,96,147,214,174,234,254,46,91,157,79,68,138,45,58,4,138,206,17,3,218,199,108,219,84,7,194,147,140,134,194,114,80,140,2,7,61,235,31,78,241,62,137,97,160,220,233,237,165,79,51,20,49,71,153,132,113,172,
77,201,218,23,148,98,126,241,30,213,196,233,183,154,149,172,114,203,105,52,234,54,148,105,81,79,1,142,14,91,31,46,122,118,246,175,79,235,30,194,10,52,165,123,165,127,85,183,245,255,0,14,112,251,47,107,
54,231,27,37,183,204,246,101,93,42,218,194,77,102,120,214,123,136,228,151,205,150,99,137,38,44,153,92,109,225,64,0,109,108,99,223,140,215,29,155,19,168,189,220,175,246,152,162,97,40,156,91,6,89,208,0,
23,97,24,35,230,224,228,118,247,205,101,248,127,93,190,210,117,104,239,1,13,58,68,80,25,6,245,101,199,85,207,1,135,99,219,211,154,211,241,23,137,97,187,180,209,208,65,111,5,192,14,215,38,56,192,114,51,
242,2,7,80,70,122,253,107,74,181,176,245,168,221,89,53,210,219,190,247,190,164,70,149,106,117,44,245,79,175,99,167,211,53,155,88,103,107,56,139,8,166,63,191,116,199,203,146,25,130,30,112,51,239,242,244,
237,93,14,163,169,120,122,227,79,150,193,153,45,229,104,75,72,209,79,132,140,5,35,32,143,155,60,228,145,249,87,47,224,49,163,234,55,32,234,58,159,246,117,196,214,248,138,56,97,253,236,204,51,208,17,180,
145,183,169,28,138,191,166,106,87,86,10,219,231,181,216,103,95,176,188,214,138,145,180,96,21,102,124,2,87,35,24,94,122,242,78,112,49,132,239,59,212,146,106,221,147,183,110,186,125,229,74,147,229,247,110,
159,221,255,0,14,115,127,17,181,3,46,163,105,168,165,198,46,221,21,29,161,109,172,168,170,2,131,140,55,36,19,131,255,0,214,166,104,126,36,151,70,208,134,159,102,251,46,46,46,37,154,229,166,137,30,56,220,
160,142,57,33,35,144,124,188,131,187,63,55,32,98,151,199,122,165,246,177,225,199,185,159,200,34,222,252,29,222,87,150,219,152,178,124,190,171,206,118,243,142,58,98,178,188,19,103,105,168,234,115,65,168,
69,52,202,214,114,132,88,155,105,73,50,161,36,206,70,118,231,238,247,207,67,92,181,167,39,136,246,148,231,102,250,237,249,92,235,164,151,213,249,42,198,233,116,223,252,143,76,248,87,225,71,213,108,181,
13,86,238,68,150,233,224,83,27,77,33,49,162,146,0,222,65,201,36,169,192,244,207,122,200,241,188,11,28,173,105,103,188,93,65,20,127,106,137,155,144,55,56,59,79,86,193,7,114,246,24,199,165,101,104,122,187,
248,31,89,212,180,233,26,223,80,179,151,104,145,161,156,163,68,64,39,122,130,8,13,206,8,57,24,233,92,118,189,171,221,234,154,132,250,133,220,242,207,129,255,0,45,66,169,42,7,66,23,2,179,120,138,126,201,
65,66,210,234,251,148,176,243,117,57,148,175,30,139,177,185,168,92,136,44,39,187,180,215,224,185,99,38,196,132,110,105,24,178,252,204,129,184,9,131,243,55,174,0,230,178,60,59,125,105,97,115,246,153,45,
224,190,186,64,85,35,123,141,166,62,62,240,198,78,236,241,143,66,107,189,211,60,43,225,216,66,143,237,253,55,91,212,76,171,182,27,41,75,192,202,84,54,3,15,188,195,144,65,252,178,43,218,180,25,124,53,253,
157,109,162,220,75,166,89,94,219,219,129,111,20,237,21,188,179,160,80,192,70,238,0,13,142,118,177,7,167,28,209,71,15,25,115,78,164,148,82,235,167,221,191,249,178,170,85,80,106,17,139,147,126,191,229,255,
0,0,242,118,179,241,159,138,116,163,20,250,69,148,108,170,28,73,51,48,157,80,96,5,85,219,206,51,208,144,72,61,43,156,187,211,245,187,104,173,52,123,173,50,214,107,191,180,180,86,215,141,115,229,197,27,
101,178,146,17,149,82,14,64,13,234,62,99,94,213,15,138,231,138,214,226,61,18,104,167,213,94,54,186,221,230,164,175,29,186,224,9,30,32,65,82,50,2,240,122,242,13,70,103,182,211,60,50,134,230,22,188,73,164,
101,186,178,138,6,39,124,156,190,73,239,252,69,143,35,38,189,10,89,125,44,71,193,54,244,187,125,187,117,191,169,195,60,109,74,31,20,45,228,120,132,177,40,150,70,109,49,173,99,75,108,102,213,68,144,49,
64,67,49,193,249,93,136,0,168,200,227,61,234,198,143,120,36,186,182,180,183,189,180,211,37,57,140,27,214,88,221,65,239,185,177,129,145,140,231,173,116,215,118,150,162,105,229,210,116,211,165,205,19,108,
158,210,21,200,150,85,69,144,158,120,41,206,7,96,125,9,53,159,226,139,232,175,117,243,99,127,20,82,201,9,70,185,123,61,164,149,63,236,18,10,49,7,166,9,200,252,252,202,248,71,79,71,249,104,119,83,175,26,
158,242,95,230,90,179,142,57,225,251,68,247,83,51,192,207,20,109,19,134,137,192,59,89,184,4,2,74,140,143,207,61,107,169,211,244,118,188,241,75,90,205,110,22,230,100,202,40,82,168,0,76,133,94,138,184,3,
61,185,24,29,107,143,178,211,52,43,91,193,62,141,125,137,252,149,79,46,123,87,25,149,115,144,100,219,130,78,64,11,219,25,173,225,169,106,90,133,180,209,111,183,185,73,110,55,71,22,162,238,237,4,139,143,
248,246,184,206,81,14,57,66,8,56,193,224,230,188,252,69,25,211,149,180,127,137,172,39,25,255,0,193,46,106,176,222,92,234,51,89,72,227,251,90,218,38,13,121,61,199,217,227,72,212,130,76,206,220,113,187,
182,78,15,70,197,71,246,104,175,32,111,42,93,51,237,48,72,1,72,175,188,196,113,129,145,200,229,73,251,167,31,54,238,112,43,32,217,232,207,120,23,113,142,230,213,247,53,168,64,178,38,193,203,47,167,39,
35,28,128,122,243,93,198,149,225,123,237,93,146,222,211,83,103,146,220,98,220,200,139,178,95,49,136,17,97,88,124,249,207,94,112,57,61,13,98,172,151,152,117,216,183,225,166,93,74,49,104,246,247,23,115,
77,41,137,162,181,127,54,87,96,62,249,255,0,103,113,35,140,12,14,61,107,220,252,23,225,127,11,105,254,14,187,186,187,214,180,203,155,183,182,146,234,77,57,165,205,211,21,7,96,98,20,160,198,55,183,60,114,
5,123,15,192,143,129,218,31,132,188,47,101,111,172,219,105,242,248,170,234,57,4,177,253,176,129,26,231,34,52,216,234,225,202,144,79,202,217,7,35,56,197,120,151,198,182,213,188,13,110,235,166,232,154,110,
160,13,204,210,22,123,7,188,188,176,128,135,27,75,47,49,70,87,12,101,101,35,28,101,79,34,121,100,154,126,127,240,77,21,227,170,103,35,171,120,194,206,206,234,202,207,78,242,161,133,161,206,37,129,176,
157,118,168,85,233,140,100,231,177,245,174,191,65,188,251,109,130,74,55,145,140,9,36,77,158,119,28,186,169,228,12,250,215,206,250,59,139,166,41,115,53,178,92,92,55,156,247,126,115,150,81,180,96,46,126,
234,245,62,231,214,189,59,194,210,220,79,172,196,250,122,110,183,75,112,230,119,186,102,73,144,12,224,18,48,184,109,223,47,94,121,233,95,107,151,103,248,152,226,239,90,87,82,182,159,229,211,205,237,234,
124,190,51,41,164,232,126,237,89,174,167,167,99,142,104,3,138,205,208,255,0,180,4,50,13,82,234,214,91,135,145,140,98,1,133,84,236,185,201,220,64,32,18,43,84,14,43,244,10,53,85,74,106,107,175,245,208,249,
42,148,220,36,226,196,29,0,165,199,28,26,80,57,167,99,233,90,144,32,28,226,151,191,35,154,7,74,119,122,0,64,56,252,41,216,246,167,99,215,242,160,10,64,38,223,106,112,7,147,131,74,1,235,218,148,47,30,244,
0,152,165,3,242,165,197,59,20,0,220,119,165,57,245,167,14,148,184,166,3,112,56,167,1,197,46,41,87,31,133,0,32,20,160,114,57,167,116,199,2,148,119,29,232,16,152,227,29,233,64,237,78,3,138,92,115,74,227,
26,41,192,103,235,74,7,231,74,121,60,211,16,128,96,83,128,252,232,3,210,156,1,160,97,245,52,229,245,160,125,105,64,160,1,64,247,167,98,128,63,10,119,173,3,16,140,210,129,205,0,127,147,78,29,123,208,33,
49,147,79,192,231,173,0,125,41,123,208,0,59,98,156,58,250,210,1,235,210,156,58,208,2,119,233,79,20,1,222,156,41,128,0,105,71,175,242,160,99,140,147,78,0,118,233,72,66,98,143,90,118,51,74,0,237,199,189,
0,39,243,165,199,30,244,224,7,227,78,219,64,13,0,210,133,57,57,167,142,148,184,246,162,227,27,142,105,216,165,218,122,98,148,41,244,160,15,143,188,87,226,157,67,251,27,81,151,236,233,29,148,100,8,89,246,
101,155,186,21,4,243,158,164,113,138,241,223,17,107,48,94,220,72,32,188,179,146,70,117,55,50,70,161,26,66,122,231,29,72,24,29,142,1,173,187,157,114,211,86,215,222,214,103,185,158,212,204,211,175,153,10,
33,27,62,84,11,140,16,57,201,61,242,71,78,145,216,27,31,17,221,90,219,221,219,91,18,139,176,152,229,85,88,16,129,181,67,21,27,177,212,169,60,100,147,145,95,153,87,203,225,94,171,112,169,232,159,174,186,
244,220,251,186,85,229,78,11,158,62,182,48,237,117,8,124,213,253,250,45,201,144,24,229,144,110,105,112,114,0,99,223,142,63,60,215,71,225,63,20,217,220,77,45,246,166,238,26,27,83,4,105,41,222,36,206,6,
73,61,206,73,39,156,156,147,235,92,95,136,116,249,44,124,65,45,131,201,26,195,185,86,7,146,80,98,104,207,59,140,171,144,164,96,131,199,24,232,120,203,117,155,75,107,24,110,32,73,44,111,46,93,83,38,209,
145,132,125,219,124,185,224,5,201,94,135,174,65,206,43,133,194,165,59,183,178,118,251,142,212,225,82,214,234,67,173,233,218,117,134,174,182,54,90,141,189,204,1,80,7,70,6,53,59,122,117,236,71,95,127,198,
168,46,169,119,103,43,11,121,34,88,202,128,78,220,113,142,252,245,252,235,38,242,222,1,108,242,139,139,136,103,141,3,24,166,131,11,57,39,31,187,117,56,28,96,252,248,207,56,53,12,7,106,161,112,72,192,32,
237,233,205,99,57,41,73,201,43,106,105,8,184,69,38,238,111,88,75,13,205,244,109,49,146,101,57,19,46,220,176,80,49,187,208,118,21,169,38,181,99,13,146,216,217,66,45,231,156,20,55,7,97,218,139,243,14,217,
220,91,140,244,198,14,115,197,101,216,169,130,202,224,229,129,186,64,27,106,228,176,25,32,214,102,153,117,109,101,44,246,183,177,60,118,211,42,171,48,24,49,145,243,2,123,227,166,113,219,233,81,26,210,
141,249,65,211,140,183,33,211,245,39,143,247,222,76,78,175,135,44,224,187,43,127,121,91,60,55,61,121,205,87,188,158,32,196,42,144,198,21,86,25,206,88,117,110,122,103,142,7,76,123,214,158,161,102,198,75,
137,158,66,178,70,224,204,229,73,80,91,59,15,3,3,118,14,61,235,158,49,200,102,44,70,224,189,72,60,98,159,180,115,75,83,69,4,181,64,14,242,217,56,56,200,56,206,106,229,144,5,227,222,73,136,252,166,48,228,
121,152,236,64,63,206,170,137,118,200,4,96,5,147,11,185,215,145,207,81,253,107,173,240,252,54,150,58,125,222,167,46,152,53,91,113,32,141,89,240,35,66,15,127,226,82,78,70,71,24,224,243,84,154,90,222,194,
105,152,23,27,2,204,145,164,173,16,127,221,184,82,66,240,56,39,167,175,189,22,210,92,69,44,50,176,101,254,37,59,1,200,245,25,171,196,165,221,181,199,151,112,150,214,102,69,49,35,18,202,79,114,0,28,144,
63,79,173,107,248,39,76,211,181,47,19,181,133,253,220,145,89,178,126,230,233,208,40,85,94,172,200,199,129,131,144,57,192,6,147,125,68,147,48,167,186,185,184,159,116,247,47,112,129,113,33,114,88,96,245,
227,223,189,90,159,83,186,189,120,158,105,201,134,28,0,210,202,66,144,184,237,208,96,15,211,214,186,207,11,104,146,127,101,53,225,22,211,233,215,215,13,184,244,50,197,20,133,67,6,254,20,224,19,215,33,
179,88,154,238,150,222,26,241,58,53,203,121,118,235,38,246,81,25,34,62,251,126,111,188,48,71,61,197,43,180,181,47,145,59,6,191,44,82,217,70,21,130,160,124,60,73,39,153,30,115,187,204,71,238,27,35,215,
24,173,29,46,242,222,13,20,91,253,131,55,77,184,194,196,29,199,220,118,234,122,243,142,41,60,87,169,105,183,250,93,212,186,93,188,112,171,74,26,68,80,0,85,56,28,14,51,206,79,182,106,15,12,135,158,41,245,
9,46,34,38,205,75,136,228,98,62,69,198,118,143,109,192,129,223,21,140,214,154,178,227,212,189,54,137,28,30,19,212,245,43,172,189,226,77,0,86,46,91,134,112,164,126,163,147,255,0,214,173,159,128,86,22,119,
62,46,188,184,191,49,170,89,219,25,98,18,48,10,206,73,28,143,226,0,2,72,30,162,177,245,221,85,46,52,71,136,91,36,75,35,168,82,168,1,114,173,147,146,15,56,92,14,107,31,76,188,142,4,183,104,173,96,127,44,
19,112,237,41,83,40,235,140,49,192,192,227,43,85,66,175,179,168,167,109,135,203,207,73,196,250,78,235,193,154,63,138,165,155,87,251,67,35,239,41,101,54,156,158,84,177,40,254,45,221,198,72,198,115,144,
49,142,107,144,241,55,246,191,128,180,153,147,88,185,177,215,244,77,86,23,176,105,110,20,173,220,109,134,144,9,137,39,204,1,119,3,200,192,197,79,224,191,30,61,197,141,151,135,116,27,37,18,77,28,142,174,
160,174,89,79,77,146,99,112,10,24,147,187,168,252,185,221,67,197,118,222,35,248,153,225,137,245,88,33,131,69,209,162,50,72,234,227,202,186,108,100,48,198,71,32,174,209,146,126,248,205,123,47,19,131,146,
78,58,84,239,209,95,127,185,95,127,145,231,170,88,136,187,63,131,183,167,249,179,209,188,57,167,248,43,88,176,181,209,35,208,230,182,214,35,34,248,216,221,233,194,11,194,27,159,54,53,199,41,208,134,140,
227,128,114,107,155,185,182,185,211,180,237,98,104,175,230,213,52,136,200,103,19,204,210,220,91,198,88,43,76,153,249,165,198,210,25,25,178,7,43,130,48,110,95,248,159,73,185,181,155,69,54,47,125,225,219,
102,43,96,203,112,209,222,104,46,225,66,189,172,224,238,22,252,0,208,3,128,15,201,140,21,36,87,26,176,210,173,82,77,82,213,227,211,26,76,38,163,99,231,25,17,178,63,215,196,85,140,120,193,42,219,134,73,
233,210,159,215,101,82,170,140,162,174,182,229,118,249,249,174,233,180,71,213,160,161,120,201,217,244,122,252,191,224,157,23,135,224,240,69,216,212,53,107,75,246,186,134,36,50,174,62,100,137,118,242,14,
220,224,12,14,79,78,231,173,121,167,136,181,157,43,81,215,44,109,46,44,34,138,214,198,17,26,221,196,152,153,27,229,57,44,14,14,89,65,3,39,167,225,84,45,244,251,221,74,201,161,23,150,218,117,182,156,38,
100,91,75,98,102,154,62,64,9,142,177,157,171,149,59,135,202,185,25,20,203,191,10,235,107,127,30,149,170,106,81,73,119,124,241,75,0,137,252,239,180,59,130,72,115,129,178,85,69,201,66,49,140,0,106,115,12,
230,114,167,24,89,71,191,159,202,237,143,9,149,123,238,113,109,254,154,17,248,82,250,246,47,22,13,70,238,235,237,114,198,88,92,43,134,120,167,98,8,32,168,24,247,13,142,217,174,243,75,254,204,62,21,184,
91,134,144,221,74,229,86,37,193,116,96,195,46,196,30,58,112,70,122,144,195,38,185,19,225,219,139,13,17,182,221,93,221,65,105,36,134,73,173,177,28,110,202,126,114,36,92,182,6,211,252,95,33,249,78,14,50,
200,109,161,212,45,46,239,237,188,219,121,88,169,220,210,151,96,73,24,7,57,36,147,143,152,227,223,210,190,118,173,103,81,110,122,107,15,236,222,171,83,166,210,109,244,203,141,203,171,46,159,168,71,144,
76,55,9,230,121,142,185,80,50,71,35,144,123,116,30,245,215,105,26,54,137,59,69,5,173,168,208,173,238,173,155,19,218,124,142,146,112,18,119,207,12,199,169,7,142,57,205,121,206,141,53,254,139,113,12,33,
26,234,101,185,193,221,18,183,154,3,126,245,24,129,128,2,16,87,104,7,32,2,121,174,186,210,254,43,203,27,157,91,83,189,179,72,33,130,82,30,232,176,23,5,79,17,34,112,17,207,56,28,129,245,174,89,169,45,83,
208,21,210,209,158,165,225,75,89,172,252,81,113,164,105,122,86,141,230,235,187,39,125,64,94,9,46,12,101,138,196,99,186,112,46,18,69,28,21,14,26,60,128,9,0,19,233,222,62,240,159,136,124,39,166,235,254,
26,211,116,173,67,87,176,120,162,188,190,214,45,172,173,17,150,13,165,228,253,226,41,121,16,176,94,62,108,114,91,214,190,103,176,178,215,188,82,150,55,62,13,209,245,139,235,68,140,91,221,79,36,74,210,
89,170,158,88,149,194,128,192,240,15,36,133,231,166,62,229,211,252,89,241,15,225,239,192,189,19,64,213,188,57,109,115,174,33,107,27,197,188,191,22,234,144,58,55,150,35,251,193,240,167,145,144,195,110,
49,210,148,167,59,174,103,242,52,139,189,221,143,207,251,75,229,147,81,146,95,178,199,246,101,185,111,182,71,12,123,50,167,31,42,168,227,24,0,253,79,29,107,209,31,125,253,163,37,180,159,102,178,8,26,214,
216,21,15,33,42,121,101,60,109,0,158,253,69,36,46,179,106,26,20,218,38,147,37,197,130,229,5,204,246,173,39,158,234,192,124,216,249,112,10,158,91,210,186,40,46,108,174,238,244,144,12,109,231,205,35,69,
12,49,229,71,80,88,128,62,92,19,140,158,58,99,174,107,233,150,93,25,74,212,231,206,175,109,22,154,219,204,241,167,139,148,99,251,200,114,187,95,93,255,0,34,183,195,203,221,45,239,226,211,236,96,187,118,
177,129,163,89,166,112,64,66,221,135,108,144,125,199,67,94,140,56,170,26,14,149,167,105,22,11,105,166,219,172,86,234,78,0,231,36,158,114,123,243,90,88,205,126,141,150,225,39,133,195,170,115,146,111,201,
88,248,172,118,34,53,235,57,197,89,121,187,136,51,74,7,229,78,52,99,240,174,243,144,74,112,2,148,12,30,156,210,129,198,104,16,99,158,41,105,195,160,61,233,113,199,20,0,152,246,165,197,56,10,59,122,26,
0,110,56,233,74,7,78,41,216,197,56,14,41,131,27,138,112,28,210,129,74,7,38,128,19,28,82,129,199,90,120,28,209,138,1,8,7,90,0,32,211,192,165,198,105,92,4,3,154,80,41,113,205,59,28,226,128,26,7,74,80,57,
165,29,169,192,83,1,0,207,90,92,125,41,113,201,246,165,3,52,12,64,41,72,228,82,254,20,225,192,250,208,33,0,235,78,244,207,90,6,125,41,195,131,64,8,5,46,61,127,90,80,1,61,233,192,117,230,128,17,71,74,118,
211,214,156,22,148,80,3,113,215,57,52,160,30,58,211,177,211,190,41,87,233,64,9,143,74,117,40,30,212,224,7,95,90,4,32,25,52,160,83,177,207,20,236,82,24,213,25,244,167,40,165,192,57,165,2,152,6,61,127,157,
24,237,78,2,156,171,243,80,49,187,114,13,73,26,254,84,228,83,138,149,19,165,38,194,195,66,28,83,150,51,140,227,138,120,29,170,80,56,169,108,171,31,146,106,22,208,78,39,145,140,160,249,126,127,94,220,1,
156,30,184,31,202,157,182,104,204,226,71,156,192,121,152,44,69,67,140,119,110,128,115,140,14,127,10,146,59,171,159,237,11,91,139,83,29,172,103,122,71,44,112,38,224,118,144,197,178,14,225,219,119,190,120,
164,186,184,185,150,207,200,102,105,85,14,236,99,1,62,131,61,123,30,189,59,87,228,73,78,46,232,253,35,70,136,224,158,117,111,44,22,130,16,161,60,190,0,41,232,216,254,85,92,106,87,45,5,213,187,207,33,183,
151,238,198,129,66,231,118,236,158,50,70,114,113,239,233,197,45,229,180,209,193,29,214,217,36,130,80,35,46,16,231,204,32,157,129,15,204,78,7,80,49,239,89,206,12,118,193,140,23,43,42,182,230,109,160,70,
16,129,143,124,231,244,167,239,197,221,238,59,38,173,208,176,210,52,215,81,91,194,94,93,237,149,88,225,44,199,169,63,40,201,233,158,149,210,89,233,80,101,99,145,110,218,91,146,12,74,200,84,66,128,146,
204,249,234,8,224,17,142,126,181,205,105,14,97,191,183,185,180,146,116,191,73,135,144,233,242,128,221,185,235,156,156,96,118,60,245,174,222,127,32,234,215,247,73,113,125,52,255,0,101,55,119,77,36,160,
21,147,163,161,3,33,192,36,96,12,246,3,165,79,178,148,189,235,217,9,181,22,150,236,196,189,129,86,99,60,78,100,183,141,71,155,193,249,48,72,233,215,39,32,226,185,121,163,121,217,254,249,94,115,223,245,
174,178,43,137,110,91,229,137,222,30,64,41,208,149,237,199,242,169,244,125,19,95,241,22,175,103,225,187,24,108,98,187,145,101,44,166,80,190,66,238,221,153,223,144,57,63,46,7,32,242,51,209,210,135,51,229,
234,55,43,106,202,122,35,94,106,211,77,162,201,42,171,220,194,176,59,151,220,160,33,44,173,236,65,199,34,173,106,126,19,109,50,198,57,174,239,45,218,115,25,149,161,183,151,204,149,16,28,23,116,218,54,
33,57,32,229,178,1,173,232,109,44,173,218,8,52,189,34,1,36,112,5,185,156,222,52,134,92,182,55,162,255,0,116,159,186,73,207,94,0,21,170,241,219,219,221,79,112,79,216,230,71,218,239,117,55,201,18,136,240,
98,36,156,96,224,177,198,127,74,39,70,245,21,167,167,123,117,242,216,35,86,209,209,24,190,20,211,225,142,3,117,37,133,164,214,240,163,45,220,87,22,174,110,1,42,219,82,53,60,18,72,70,224,116,32,250,102,
149,223,133,252,73,168,64,151,254,108,45,115,6,4,202,136,97,75,69,219,184,19,35,0,10,240,6,6,79,0,16,107,209,237,60,53,53,236,87,58,165,202,121,41,28,1,146,40,201,251,65,76,12,48,1,142,8,206,57,29,49,
156,118,225,245,61,14,254,215,67,75,139,233,92,104,151,151,35,201,184,73,252,229,144,19,156,158,203,129,206,127,190,64,238,40,120,105,193,190,104,59,216,191,110,154,178,103,25,119,45,194,92,170,222,67,
4,177,18,75,70,23,110,55,97,153,130,140,97,176,115,159,90,211,211,96,184,212,46,238,52,159,14,195,169,221,155,148,216,81,118,9,100,129,121,100,42,62,86,39,61,136,39,160,235,138,236,124,39,225,111,17,92,
220,171,93,198,182,169,228,75,49,136,133,55,78,145,72,19,27,217,136,86,57,3,60,129,158,221,106,165,223,135,34,210,101,138,59,107,139,171,125,116,78,12,94,76,129,196,74,199,10,28,245,39,169,12,15,214,177,
157,41,198,55,45,78,13,218,71,119,225,125,86,88,252,35,170,120,126,104,230,187,180,180,187,219,60,17,193,229,148,182,242,241,150,103,228,28,144,10,144,27,229,61,123,203,107,172,71,168,53,220,15,107,107,
111,228,73,229,93,72,201,146,27,102,35,36,145,237,141,217,246,174,15,91,190,241,118,147,226,36,151,197,83,29,66,217,145,45,165,185,226,120,164,136,100,4,222,128,18,71,204,48,227,39,158,189,107,173,240,
76,122,102,166,242,93,223,132,147,78,189,191,31,36,44,200,37,110,0,221,16,228,96,13,160,159,126,195,146,148,172,237,55,103,248,4,163,45,92,117,71,147,248,210,207,77,183,91,73,236,11,197,113,43,58,93,219,
150,44,160,131,149,117,39,177,228,21,232,8,24,227,147,39,133,45,94,230,104,224,68,145,139,19,129,24,27,186,118,253,43,127,227,94,157,109,99,226,59,107,75,24,160,142,212,19,28,75,18,178,170,70,49,180,29,
220,150,249,152,238,239,249,87,77,163,248,187,194,250,39,194,157,107,195,51,248,126,67,171,106,104,110,44,117,12,1,52,44,129,68,71,35,63,124,134,59,184,85,20,170,203,147,101,114,233,37,36,238,236,121,
37,226,203,45,233,82,174,133,89,147,100,135,37,72,56,108,251,146,15,21,214,248,103,78,177,22,147,253,185,109,34,185,88,252,200,158,235,39,97,3,56,85,232,73,252,255,0,90,230,222,101,184,212,4,206,236,242,
187,249,147,22,29,91,57,39,223,36,215,171,124,53,181,209,117,77,110,91,107,152,89,34,17,75,35,221,220,141,230,44,32,43,181,127,188,112,113,199,31,165,77,56,185,201,45,133,86,92,177,247,70,248,34,194,225,
236,244,219,216,111,45,116,171,203,91,177,123,106,237,25,50,101,87,116,101,72,82,79,66,187,7,222,7,28,130,69,17,248,110,99,169,234,235,97,162,92,62,149,185,238,28,65,114,97,243,149,129,117,101,87,24,66,
11,28,41,24,234,48,43,169,186,154,61,27,70,176,212,244,233,99,73,60,246,179,138,38,231,22,237,32,118,89,88,243,146,16,114,48,202,120,25,92,230,244,119,22,118,246,73,104,175,116,34,103,18,203,34,207,176,
73,185,152,176,82,188,21,81,128,164,114,64,231,158,107,121,80,141,57,171,73,75,175,151,245,234,99,26,156,240,213,52,114,30,27,209,34,88,60,251,43,121,46,85,203,198,183,48,194,134,82,118,22,200,141,190,
235,40,12,14,65,232,120,7,138,185,225,169,174,124,61,111,20,145,88,219,223,104,107,165,164,215,17,71,16,153,90,25,203,48,13,11,96,100,128,14,57,199,6,171,67,168,218,88,223,181,247,135,158,233,46,188,209,
36,187,174,153,80,166,8,11,129,209,183,18,73,234,71,29,43,49,110,239,225,181,150,250,9,60,182,83,182,226,104,201,35,119,56,86,244,200,4,14,163,142,199,21,203,94,172,27,92,138,221,255,0,224,26,83,230,142,
236,233,245,79,17,92,93,120,86,222,239,68,22,246,122,134,232,226,132,249,68,24,240,11,31,187,128,193,135,62,190,156,129,137,96,255,0,132,106,235,68,185,134,71,191,182,214,39,184,141,175,229,187,185,111,
145,112,8,104,176,54,130,175,180,131,156,0,57,0,181,112,119,55,239,21,189,133,227,217,92,203,163,54,100,186,49,41,62,83,171,124,228,1,194,245,200,83,215,36,142,185,171,154,213,253,164,90,141,229,174,110,
193,72,131,197,49,67,57,112,234,118,239,81,156,169,233,198,71,7,211,142,87,135,109,111,185,221,79,29,40,202,238,41,171,91,95,235,70,119,126,43,241,5,237,223,135,100,147,81,184,158,91,107,38,48,199,36,
75,26,11,188,144,89,101,85,218,169,41,10,188,129,134,227,156,100,87,99,240,187,195,26,61,254,158,46,108,101,54,230,123,127,150,73,46,151,22,242,177,36,7,25,255,0,87,33,192,25,25,25,61,107,201,180,200,
180,27,216,237,45,172,175,217,97,188,181,9,52,175,251,168,196,128,28,171,169,206,112,65,228,112,120,197,79,170,91,255,0,100,95,216,207,117,169,90,60,214,171,246,86,154,199,114,92,108,224,136,203,140,28,
129,243,115,219,142,120,172,231,134,106,46,42,86,58,86,99,78,85,148,170,194,235,207,252,206,167,196,250,54,149,166,139,171,121,180,219,219,29,74,218,65,117,97,28,174,146,121,224,18,175,27,103,230,224,
130,114,84,18,25,122,128,107,135,240,214,182,246,222,35,139,81,102,146,233,229,50,51,195,42,252,136,229,2,164,138,127,188,163,33,129,4,17,131,237,87,252,81,125,21,197,140,87,31,218,196,24,157,153,81,174,
204,151,81,71,156,40,121,15,204,225,250,134,39,118,67,102,178,116,237,106,218,226,234,107,201,110,22,56,172,8,54,22,194,4,127,39,32,110,234,6,73,193,201,239,158,153,201,167,78,50,132,117,212,230,175,86,
141,73,221,43,47,235,208,250,111,224,119,196,79,8,124,56,241,93,148,182,218,35,77,119,113,111,10,121,44,219,208,76,65,99,54,65,37,144,168,82,188,103,112,60,14,163,59,196,159,24,124,97,226,191,16,107,105,
226,59,77,57,180,205,66,249,77,196,159,100,105,124,181,28,36,65,247,15,44,5,200,228,54,9,45,212,215,139,201,226,59,219,77,38,37,121,244,203,165,138,64,214,203,53,151,157,13,178,147,151,69,78,50,113,130,
6,120,32,115,198,43,186,240,190,163,225,203,203,244,210,165,251,122,203,122,230,56,210,40,247,51,71,207,239,221,206,72,144,28,110,24,249,190,181,131,218,237,21,203,74,77,40,206,223,215,204,210,240,246,
183,118,243,207,163,233,186,140,214,250,99,146,30,218,222,114,103,17,162,252,167,116,124,185,24,206,9,195,116,53,54,155,114,186,149,222,155,36,66,233,180,168,32,47,190,224,139,119,40,88,226,70,43,141,
129,176,66,134,201,25,199,106,239,188,53,240,235,64,208,110,36,183,181,55,55,30,33,49,198,26,114,185,146,49,158,169,8,206,18,80,142,81,164,32,6,27,78,107,191,142,218,210,41,223,108,86,206,99,77,239,43,
68,133,66,131,134,119,24,17,70,9,70,89,20,238,100,108,53,121,245,49,156,146,126,206,232,250,140,54,77,86,173,21,10,246,211,213,221,118,233,248,18,218,89,120,88,105,182,233,22,159,169,216,194,34,13,111,
246,105,124,241,42,12,6,219,28,163,204,37,50,55,99,177,4,18,51,136,198,143,97,52,146,69,103,175,90,137,162,147,203,120,174,227,104,12,111,128,66,179,141,209,134,42,67,1,187,144,65,21,49,17,164,140,232,
211,195,153,54,177,89,25,94,89,35,3,131,33,253,228,210,162,198,78,212,1,101,140,227,154,134,222,212,199,109,109,108,12,4,219,192,169,28,86,240,172,97,120,59,4,81,103,108,49,150,76,199,36,132,180,109,149,
60,87,179,132,227,12,223,11,21,31,107,204,187,73,39,248,239,248,152,226,184,43,36,197,93,186,92,178,254,237,215,225,176,73,225,141,109,99,89,33,177,55,113,50,238,89,44,228,89,213,135,168,216,73,199,225,
88,179,196,240,72,98,157,26,41,7,240,72,165,79,228,112,107,102,56,97,105,142,161,24,219,44,114,72,237,118,174,68,106,249,117,103,149,215,13,41,200,2,88,147,133,96,24,99,173,110,71,168,235,80,162,216,189,
253,244,248,44,12,23,145,199,113,32,228,28,56,113,177,2,131,145,147,153,35,201,25,97,207,210,97,124,68,159,252,196,80,79,206,46,223,131,191,230,124,214,47,195,42,122,188,53,118,188,164,175,248,171,28,
78,15,113,74,6,114,1,7,30,148,159,12,62,37,248,3,198,127,16,181,47,12,106,118,250,78,159,61,186,180,182,247,182,44,203,5,200,140,47,154,20,134,42,70,79,202,204,23,112,207,0,142,123,255,0,23,234,254,23,
213,245,101,182,183,182,54,182,118,177,178,71,113,110,185,116,85,25,102,148,116,192,206,118,156,54,211,184,113,156,123,171,142,48,10,164,35,56,74,41,238,221,180,237,162,110,231,205,190,2,204,28,102,233,
212,140,156,118,74,250,247,213,217,39,247,252,142,15,7,35,154,92,30,213,163,119,167,73,24,146,75,105,162,188,182,143,27,165,183,57,218,8,202,150,95,188,185,28,242,49,142,132,213,12,2,160,142,157,114,13,
125,118,31,19,71,19,77,84,163,37,40,190,168,248,220,78,18,190,22,163,165,94,14,50,93,24,157,105,118,250,98,151,25,233,64,25,53,208,115,9,252,169,216,233,199,227,75,142,212,224,40,1,184,29,135,52,236,115,
74,1,244,165,3,220,208,0,49,74,58,122,82,129,206,59,210,140,102,144,196,29,125,41,113,206,41,216,168,238,229,138,11,73,231,157,153,96,142,54,121,8,234,20,12,146,61,241,154,27,182,163,74,238,195,149,163,
243,17,60,232,131,59,109,80,242,42,238,62,131,38,183,47,188,61,171,90,233,201,127,37,166,235,70,36,25,35,96,251,8,234,24,14,87,241,24,247,175,141,60,115,226,157,107,197,90,165,156,250,134,161,53,244,86,
106,108,224,50,192,171,35,192,28,152,203,237,224,182,49,147,128,78,6,73,192,174,227,225,255,0,198,63,21,120,39,89,210,45,110,191,226,107,163,91,186,170,219,92,174,231,242,178,11,197,230,242,124,179,212,
171,7,193,25,92,98,191,55,197,113,190,38,21,210,163,74,60,189,83,110,255,0,39,255,0,0,251,138,28,41,70,84,159,61,71,205,209,219,79,235,230,123,222,49,219,35,235,214,148,123,214,134,175,173,105,126,35,
154,45,119,73,211,229,177,181,191,141,102,242,27,102,208,79,241,38,206,54,48,193,28,3,215,32,85,33,215,154,251,252,22,37,98,176,208,174,162,215,50,78,207,117,115,227,113,84,30,30,188,169,55,126,87,107,
161,184,165,3,214,156,58,230,151,29,206,107,168,192,104,30,212,224,42,190,167,119,111,167,233,247,23,183,111,178,222,5,220,237,140,255,0,158,120,171,41,113,165,203,107,163,75,103,172,89,93,75,169,233,
113,106,105,111,27,159,54,24,100,37,87,204,94,138,114,172,49,158,112,107,154,174,51,15,74,180,104,206,105,74,91,46,231,69,60,37,122,148,165,86,17,110,49,221,246,1,244,167,0,115,239,75,140,211,192,56,224,
115,93,39,48,209,211,233,78,227,31,90,92,26,80,40,0,29,58,28,211,128,252,40,197,72,171,64,13,199,181,40,30,212,253,191,157,40,95,90,87,2,50,13,63,7,165,63,20,98,152,88,64,14,77,46,57,233,75,183,62,244,
240,189,51,64,198,1,233,78,0,103,138,118,61,105,192,1,233,64,8,7,183,20,229,94,105,192,103,181,60,14,70,41,92,1,6,49,154,144,1,233,77,81,205,72,41,13,10,20,19,205,72,20,14,212,139,214,164,3,156,212,178,
145,249,66,218,165,181,213,140,16,77,161,192,52,252,202,182,226,208,55,156,31,112,36,101,155,3,230,228,142,119,113,145,199,53,173,180,117,189,11,29,140,141,117,112,48,4,241,220,44,176,130,217,11,189,186,
169,61,194,130,7,82,112,115,93,12,122,14,165,171,216,45,148,144,157,46,221,238,85,161,133,160,96,81,88,183,202,176,130,54,140,252,196,158,79,174,49,91,87,182,81,120,69,108,145,175,116,125,86,230,218,98,
47,36,68,201,181,135,0,21,76,125,238,8,37,112,73,32,246,192,175,203,104,242,78,167,191,177,250,44,211,140,52,220,232,180,31,135,186,22,155,225,230,187,185,241,68,87,122,202,194,178,51,44,130,72,45,100,
118,8,236,7,114,163,56,98,119,28,113,197,121,253,222,135,225,132,188,212,237,33,212,47,102,176,107,18,214,23,83,150,2,107,160,121,202,32,25,0,96,1,211,169,53,208,104,186,133,150,171,120,45,180,169,38,
137,146,114,209,91,90,74,66,207,30,49,190,109,220,134,0,224,47,64,61,199,28,167,136,237,47,35,22,22,18,52,81,136,137,68,64,192,22,109,219,73,227,35,245,233,93,120,204,85,27,167,70,154,81,90,117,255,0,
63,248,58,28,180,168,213,218,164,219,111,211,67,79,225,143,133,236,110,245,187,139,91,214,23,10,176,238,40,48,191,59,125,192,133,184,13,128,221,122,243,84,46,82,205,117,51,166,89,90,157,145,22,85,216,
132,179,96,159,188,61,114,59,86,29,253,212,172,101,181,75,159,50,18,66,78,216,5,29,144,240,113,223,24,250,241,89,214,247,111,109,35,78,191,51,51,100,101,143,35,184,221,212,3,250,87,149,82,175,61,40,193,
45,175,253,127,87,58,163,78,92,238,77,157,192,209,111,172,210,214,121,53,105,225,89,156,110,138,16,139,177,0,193,46,95,133,219,198,75,14,71,76,215,166,29,42,11,125,34,199,77,208,100,181,138,192,131,54,
163,112,142,162,73,131,161,3,115,103,45,158,9,201,0,14,43,204,172,181,36,188,157,99,130,37,154,27,246,84,187,146,232,59,60,74,236,171,242,55,114,49,187,62,139,143,167,67,20,179,13,26,239,65,47,35,216,
197,56,82,136,203,27,40,89,11,49,12,122,239,249,73,4,214,152,120,218,147,231,209,191,45,125,61,5,41,62,109,12,43,72,95,237,137,117,29,201,142,111,57,147,124,99,8,184,4,41,3,174,78,15,251,56,197,92,211,
239,110,23,79,130,222,24,173,229,146,226,66,152,60,225,199,202,31,97,200,13,128,6,71,39,233,85,245,123,72,98,157,82,213,154,59,103,3,231,36,146,164,174,72,220,58,247,233,192,170,87,16,68,150,239,21,194,
95,197,51,175,250,35,217,144,121,227,10,80,240,65,227,147,140,125,42,99,77,169,217,232,191,175,204,165,63,119,67,163,254,209,191,210,18,103,183,181,185,158,210,47,154,250,73,36,9,30,195,129,146,113,151,
99,198,208,6,14,78,5,71,225,109,58,246,194,117,184,213,99,219,60,210,111,209,227,105,76,150,193,25,140,153,8,24,6,43,145,183,32,0,201,158,192,213,31,15,90,220,94,220,205,45,233,69,211,225,1,162,143,206,
13,186,96,2,48,99,192,56,60,2,72,198,70,1,234,123,43,75,123,116,191,26,109,195,126,245,17,46,172,210,220,5,146,7,10,202,196,119,42,56,31,221,249,186,243,91,83,110,41,107,183,127,159,244,181,20,172,215,
175,98,148,151,176,201,173,233,193,35,113,32,185,150,16,235,38,12,74,203,151,140,224,109,99,189,115,212,227,62,162,187,73,52,88,100,209,22,31,54,214,86,19,150,134,230,66,67,49,206,10,6,3,228,83,202,241,
207,29,143,53,195,232,154,109,206,183,174,105,208,69,166,24,238,99,146,91,157,177,131,28,146,21,251,200,172,14,24,100,7,192,25,92,99,59,78,43,213,116,105,23,85,240,205,178,179,75,113,18,145,61,187,193,
10,135,129,221,179,150,141,136,218,67,228,17,233,156,138,215,11,135,142,37,74,47,127,43,254,159,215,114,43,85,116,82,151,67,39,80,211,100,181,138,246,207,254,37,242,233,119,19,132,184,73,161,198,224,170,
185,68,35,39,238,146,64,193,220,6,115,216,240,94,45,240,85,207,133,117,56,60,69,166,91,53,197,189,189,210,203,37,158,242,86,97,187,35,24,1,128,199,37,122,128,70,49,140,87,125,226,24,174,47,34,143,79,107,
61,70,125,84,90,135,185,180,141,7,151,26,140,148,37,137,27,240,70,65,80,72,239,90,190,29,6,227,68,177,188,147,81,186,184,47,60,83,161,55,0,60,202,71,152,145,148,115,128,66,230,60,147,201,25,239,196,79,
4,148,253,154,79,93,191,94,150,118,242,216,170,120,166,163,207,125,183,57,63,26,248,50,63,21,124,21,240,87,141,252,59,123,21,245,140,87,198,15,19,22,139,247,154,101,201,1,99,56,39,115,64,84,144,14,70,
91,111,77,217,30,99,241,20,105,179,106,79,29,166,216,96,211,172,33,72,147,141,211,59,49,95,208,101,136,236,43,186,248,113,226,155,127,135,255,0,28,53,173,34,226,105,223,192,215,215,147,90,234,22,138,72,
89,96,145,55,33,43,223,105,97,199,183,182,43,133,248,165,109,225,189,43,199,183,214,94,22,190,184,212,60,63,20,48,172,115,220,57,121,26,77,167,204,220,78,50,71,29,56,244,239,94,43,140,227,81,194,111,69,
183,245,249,30,187,112,112,85,32,183,221,17,252,61,240,241,213,154,234,230,100,31,98,181,93,206,72,232,59,156,119,227,245,198,125,106,75,237,63,83,211,181,27,91,104,243,37,196,172,25,20,41,243,1,14,201,
180,255,0,116,252,156,143,165,118,255,0,5,160,181,185,240,230,171,115,58,52,182,209,49,70,136,38,118,2,187,124,194,223,117,122,156,103,210,182,33,72,117,187,11,155,217,108,23,68,138,73,217,229,212,81,
75,196,208,44,204,84,22,108,227,156,144,167,39,13,129,158,181,235,209,194,123,72,67,145,234,238,222,187,91,185,228,215,196,114,202,78,75,69,107,124,206,82,215,195,26,189,197,173,253,237,214,167,21,181,
230,159,230,60,246,82,176,141,32,66,161,139,147,156,110,35,208,103,61,115,93,6,142,145,105,90,62,189,169,106,247,50,221,234,54,83,155,91,107,29,207,20,109,59,40,144,199,149,227,123,128,167,112,225,50,
65,231,57,142,206,198,230,210,194,238,61,50,91,139,187,21,125,254,108,106,74,204,202,65,220,70,56,85,199,27,178,78,115,142,245,157,115,125,113,168,70,101,121,128,183,4,180,91,70,15,39,115,55,60,128,115,
214,185,106,98,93,47,138,158,173,117,214,254,127,240,218,21,8,115,43,39,179,254,145,129,169,238,176,89,230,184,57,150,71,222,197,144,110,149,136,1,122,0,50,20,40,206,59,100,247,168,180,91,75,151,149,99,
19,165,168,189,153,96,40,179,29,164,179,168,195,246,0,159,92,226,179,245,79,46,244,197,50,25,4,54,251,138,66,220,9,71,63,48,39,173,5,46,33,215,244,232,20,152,25,165,138,68,117,29,9,96,84,143,112,122,126,
85,199,74,55,106,251,179,121,78,231,213,210,252,19,215,52,175,4,234,218,212,214,147,105,113,106,143,229,233,137,124,11,37,193,117,10,141,35,198,89,97,151,118,112,8,57,1,112,192,157,149,227,151,254,22,
241,141,140,51,90,71,165,92,132,210,88,72,182,78,208,169,143,207,98,223,187,114,126,98,206,9,43,146,20,129,194,131,207,216,122,55,197,253,111,254,16,235,75,125,71,77,107,105,52,203,155,88,98,103,82,93,
216,21,221,52,209,245,46,185,221,130,50,187,1,228,215,155,120,251,195,176,90,248,163,89,180,142,88,37,213,245,73,188,216,226,156,226,43,100,103,95,53,129,200,62,97,144,7,36,253,214,126,6,1,3,170,189,56,
165,104,108,101,6,250,159,45,194,186,158,151,226,38,187,212,52,153,30,123,57,135,219,52,235,149,193,99,202,178,228,12,6,244,35,32,16,8,62,185,90,94,143,127,168,205,168,95,218,218,77,113,110,155,228,87,
110,137,131,156,49,207,44,6,6,239,92,158,51,94,153,227,125,7,197,115,197,173,201,124,183,46,250,32,134,208,166,157,18,52,14,225,142,200,102,216,115,150,36,144,224,112,121,32,142,43,156,240,141,175,138,
181,251,87,208,188,53,163,94,204,46,87,121,142,1,151,144,39,204,88,15,226,224,96,128,114,122,0,107,154,74,95,101,26,115,93,28,85,213,164,223,102,144,11,184,13,185,25,5,78,76,164,28,49,7,140,129,254,20,
237,19,79,158,91,69,189,55,59,4,132,162,41,25,114,87,158,157,64,199,115,197,122,52,126,27,154,109,34,221,134,142,202,242,179,24,214,102,120,54,156,31,49,21,74,252,185,193,32,158,224,227,35,154,175,160,
199,224,225,32,211,245,43,247,179,180,158,25,228,181,121,32,18,126,253,64,218,164,130,55,99,5,64,94,91,57,232,42,83,187,229,191,224,76,239,21,123,25,26,124,246,17,219,136,218,96,101,158,108,198,118,237,
46,79,252,180,33,253,254,128,87,105,162,104,119,22,26,188,154,134,162,151,59,100,184,104,224,158,222,120,198,119,15,149,190,82,119,15,153,129,7,130,113,140,83,110,252,19,170,233,231,71,212,228,210,97,
123,77,90,218,41,173,238,228,222,145,76,36,82,222,82,158,127,120,184,218,66,241,158,135,131,137,111,236,226,210,173,173,103,154,57,90,120,162,2,56,164,98,2,242,120,25,25,227,29,71,229,82,176,238,109,194,
49,187,39,218,40,174,102,206,255,0,194,30,34,215,116,11,235,13,7,78,125,46,230,75,199,100,134,203,86,182,47,29,243,178,225,160,149,212,226,47,49,65,1,152,225,74,131,201,32,31,119,158,242,198,238,238,72,
44,238,30,244,195,51,239,123,118,91,182,182,145,72,89,23,60,66,143,22,230,71,234,93,57,28,131,95,14,120,113,46,245,239,18,93,219,234,147,137,108,37,203,152,130,0,168,73,224,143,167,21,245,7,195,61,127,
80,214,33,127,15,106,119,115,203,226,77,38,53,119,221,19,220,201,168,91,175,250,187,136,32,137,86,53,145,89,185,45,158,132,116,60,112,99,176,170,49,246,105,43,173,79,170,200,113,237,77,41,201,181,248,
127,72,239,33,184,105,109,205,236,55,17,72,178,136,208,207,21,209,218,129,182,148,85,186,111,153,255,0,214,51,68,98,0,117,66,69,62,218,230,218,107,3,28,214,112,139,27,175,156,36,144,20,142,81,33,95,49,
254,204,15,152,200,237,33,89,68,184,216,255,0,54,0,230,172,100,153,157,207,146,110,34,76,22,77,142,202,142,73,14,84,31,46,222,222,95,47,215,40,226,168,25,227,183,184,202,199,26,65,60,165,25,195,21,87,
149,70,27,204,115,243,77,48,8,65,85,249,100,78,122,215,130,225,39,161,247,81,169,15,139,250,254,191,166,78,151,191,233,132,188,215,49,205,108,255,0,188,198,199,186,181,218,168,78,249,63,213,67,34,43,156,
162,229,165,139,145,158,107,207,252,117,226,43,157,78,67,225,13,18,85,142,73,97,67,121,123,4,146,44,86,17,50,164,145,73,110,229,72,184,37,148,178,111,33,163,201,227,24,7,95,226,37,254,179,165,105,75,15,
134,116,57,110,245,57,10,69,101,31,150,166,40,83,230,42,201,17,32,42,12,126,238,73,14,114,74,54,56,175,142,172,37,241,127,134,237,228,178,182,182,184,177,182,153,165,63,61,179,42,7,86,34,83,191,166,240,
65,13,221,113,207,173,122,56,28,34,171,119,117,126,136,242,243,76,116,168,89,40,54,186,180,125,161,225,45,15,225,124,186,58,232,26,134,143,97,113,5,186,29,198,91,99,41,25,57,102,121,48,73,98,199,36,147,
201,60,215,107,23,195,111,14,95,68,205,225,159,19,234,26,124,162,221,96,133,109,238,214,226,59,112,191,116,164,19,6,85,101,236,113,154,227,127,98,125,22,47,20,106,173,167,67,116,22,102,211,230,150,73,
25,119,111,96,80,109,60,241,203,86,149,254,141,168,105,154,189,181,170,234,139,36,75,168,11,103,23,96,239,108,132,62,88,114,51,187,4,224,238,239,222,189,250,25,125,71,79,150,86,109,116,103,200,98,179,
104,194,175,53,54,210,125,139,90,183,128,188,113,165,223,75,169,105,247,90,62,177,123,21,178,65,104,242,180,186,125,214,1,37,149,238,19,204,249,57,44,17,85,66,182,113,193,53,206,222,106,126,38,210,98,
213,27,196,254,6,214,117,8,109,229,95,34,247,77,134,19,113,114,132,114,118,196,197,28,169,207,46,177,146,49,197,117,16,106,62,52,176,185,185,134,56,174,252,136,110,140,67,18,150,218,155,136,203,43,110,
0,140,0,71,29,250,87,73,168,120,138,247,72,182,130,61,69,33,186,186,185,63,185,48,68,80,147,206,20,129,158,73,224,30,50,72,3,147,87,79,219,224,31,181,164,220,45,187,79,250,77,121,3,175,79,50,181,26,209,
85,47,178,146,254,154,60,187,67,241,38,129,172,93,93,89,89,220,93,197,169,219,50,239,179,186,181,43,38,210,9,223,149,37,118,140,99,39,28,241,91,120,230,180,100,189,130,91,167,254,216,138,6,146,76,180,
183,113,55,151,45,162,100,172,123,183,124,174,165,134,10,28,52,108,112,79,32,212,48,193,29,216,185,125,50,229,111,133,171,109,185,68,141,146,123,99,128,113,44,44,3,198,112,65,228,99,4,28,224,215,222,112,
239,17,225,241,148,149,58,245,175,87,205,37,247,91,71,249,249,31,15,196,188,43,139,193,85,117,104,80,253,215,247,91,149,187,222,247,107,242,243,42,10,59,211,192,207,57,200,62,148,161,107,235,207,138,16,
10,80,41,192,115,253,105,112,123,208,3,64,233,78,3,222,140,28,211,128,199,106,0,0,207,21,143,241,149,181,63,11,120,99,70,189,178,183,139,80,178,215,225,116,130,242,9,135,250,36,203,157,241,201,25,7,39,
110,72,32,227,42,114,7,25,218,25,233,215,218,178,35,188,177,241,134,175,168,124,52,131,82,159,237,218,190,157,246,189,49,38,132,9,180,237,66,7,39,202,116,39,238,200,155,138,149,37,92,6,32,140,215,203,
113,102,59,21,131,193,198,166,26,124,174,246,245,190,155,181,101,109,250,105,126,199,208,112,238,23,15,137,196,74,53,227,117,107,250,124,175,175,227,248,159,53,220,89,95,248,114,211,237,139,0,62,96,228,
56,15,242,159,175,70,254,117,173,240,247,193,223,240,155,234,9,168,94,204,176,105,90,117,210,253,162,54,143,50,76,193,67,133,10,14,220,28,128,73,200,192,35,6,160,241,126,131,172,105,30,37,180,208,188,
85,115,115,167,75,116,231,204,158,123,39,113,12,63,243,218,52,249,90,78,64,24,4,122,113,95,110,124,6,240,103,134,53,223,128,80,105,254,27,219,22,177,109,60,247,48,249,132,179,204,75,29,232,206,220,228,
240,118,159,185,144,6,64,205,126,79,150,123,10,89,156,22,102,173,118,211,123,91,254,7,244,143,209,49,211,168,240,18,120,38,175,109,58,158,126,168,168,170,136,170,136,163,10,170,48,20,122,1,79,198,49,83,
220,65,36,19,180,82,198,241,202,135,107,35,174,210,167,208,138,104,90,253,254,45,114,174,93,143,199,100,159,51,230,220,140,14,121,167,1,205,63,111,110,244,160,126,117,100,156,79,197,109,5,181,175,11,78,
176,69,35,222,193,134,136,35,16,88,100,110,92,103,7,35,215,210,165,248,65,171,27,15,134,87,62,3,187,211,116,41,180,187,235,161,119,44,172,202,215,70,116,149,100,143,114,243,247,74,141,167,60,109,29,235,
177,152,74,32,145,161,68,105,130,146,138,199,0,158,192,214,183,236,169,225,45,39,196,158,23,241,153,187,210,98,147,90,176,184,243,82,100,5,100,85,117,102,217,145,219,122,183,31,74,249,156,239,47,194,212,
196,81,197,214,111,220,107,109,157,158,151,61,60,46,103,140,163,65,225,112,233,62,107,235,171,105,91,91,106,101,145,215,35,235,78,10,125,42,119,141,148,0,224,134,199,35,222,155,143,106,250,91,158,96,192,
41,118,146,73,230,158,7,108,82,224,99,20,0,197,94,71,165,74,23,29,171,95,194,58,68,218,239,136,44,244,171,125,254,101,203,21,5,87,37,112,9,206,61,56,31,157,80,189,181,184,178,187,154,210,238,23,134,234,
22,41,36,108,48,81,135,81,81,237,32,231,236,239,173,175,111,34,189,156,185,57,237,166,196,32,115,205,40,29,168,3,159,106,80,61,234,137,16,142,125,197,56,10,78,216,230,159,138,96,52,12,28,83,177,255,0,
234,165,199,175,52,160,119,197,0,38,56,167,40,160,140,142,148,229,25,160,5,2,156,58,210,170,254,84,237,167,60,10,67,17,71,7,166,106,69,3,191,106,21,105,216,199,90,67,66,175,3,29,170,69,198,121,53,26,224,
12,83,191,149,38,51,242,219,70,241,204,118,55,178,207,125,167,220,220,223,78,146,69,116,30,64,231,107,237,232,92,238,13,199,3,160,4,224,243,82,207,225,40,160,211,116,89,77,130,91,125,189,37,189,221,115,
34,172,177,90,42,12,57,199,203,141,204,9,201,12,23,0,142,77,107,232,94,16,151,76,215,238,237,239,210,206,96,146,54,101,158,82,80,237,147,203,105,58,30,74,12,246,206,118,131,212,211,126,40,74,52,139,136,
180,173,66,75,139,184,66,44,150,215,94,110,95,203,229,118,203,26,240,190,187,71,36,158,195,32,126,93,8,46,89,78,107,84,237,255,0,14,126,135,41,91,149,71,175,245,161,204,220,107,247,122,99,79,167,67,109,
96,209,71,107,228,43,68,129,84,163,62,241,48,97,203,59,113,144,120,198,6,56,231,22,227,82,123,229,15,36,159,188,140,124,165,216,227,129,235,221,171,42,254,65,60,229,108,153,166,141,149,114,248,192,95,
155,56,3,182,51,142,121,169,45,45,100,150,41,151,206,40,136,9,195,30,24,244,198,59,158,58,119,174,89,222,82,212,213,37,107,176,134,117,150,101,73,75,186,191,222,0,227,233,208,126,21,208,90,105,6,103,88,
167,89,109,238,67,198,85,10,141,146,70,125,54,228,130,58,146,120,199,227,93,230,141,164,105,150,255,0,217,150,118,58,190,159,111,13,216,243,36,251,66,121,166,80,35,13,230,9,0,92,156,112,184,254,51,199,
0,229,179,105,141,103,107,106,239,39,152,89,223,104,157,50,46,173,157,78,217,78,15,223,243,24,41,233,130,157,57,4,239,44,63,36,111,123,178,99,85,74,86,177,154,144,180,82,61,213,147,121,247,80,221,226,
86,48,133,134,73,114,63,120,152,57,59,84,158,78,113,129,233,90,51,104,58,189,236,208,11,212,72,97,158,225,224,70,13,187,123,12,184,94,188,240,62,246,122,241,82,94,153,60,63,120,46,30,230,234,59,66,209,
121,72,35,97,46,198,140,110,44,204,48,167,25,92,119,82,27,233,215,104,126,39,176,217,109,99,161,44,146,43,170,220,195,26,174,46,4,165,72,117,153,177,183,129,198,84,227,166,58,115,221,133,195,209,173,238,
87,157,157,246,220,228,196,212,169,15,122,156,110,115,126,28,210,110,149,6,162,246,87,23,246,65,94,56,98,85,253,224,10,74,16,70,50,71,7,7,212,16,125,230,209,237,244,201,188,161,115,108,44,101,105,252,
165,145,167,200,224,47,204,123,96,128,217,247,43,207,74,246,173,43,195,173,113,114,111,245,59,157,179,161,15,20,86,242,21,142,12,131,158,15,222,4,0,165,191,136,146,112,58,215,53,227,189,54,194,254,250,
79,14,233,226,209,99,186,6,234,238,34,202,131,202,27,84,199,133,25,37,250,3,195,12,49,7,138,239,173,151,42,107,149,201,53,209,59,59,254,171,207,93,14,74,120,175,104,239,109,122,239,167,249,155,30,17,209,
161,139,194,182,9,43,8,197,205,169,149,118,218,2,232,95,39,230,33,72,232,217,7,215,211,21,195,248,159,76,204,178,139,13,68,93,93,134,249,22,241,149,36,216,15,69,101,0,2,14,224,7,66,51,158,198,187,185,
53,123,203,159,15,195,105,53,173,198,153,97,21,170,31,180,164,163,8,145,156,50,59,174,88,156,96,121,128,112,51,187,145,147,14,159,100,151,30,31,183,187,210,162,145,68,82,41,99,109,55,154,210,236,224,130,
222,152,207,29,123,113,91,215,193,97,113,17,229,167,22,156,87,159,55,252,31,235,67,24,98,171,210,155,148,221,211,126,86,255,0,128,121,146,166,147,60,86,182,145,223,189,189,205,204,146,126,249,101,111,
180,194,84,99,114,187,31,149,193,200,101,192,10,87,4,98,189,31,194,218,170,233,210,93,77,45,148,166,34,201,53,207,155,38,243,110,10,5,207,28,20,59,78,14,120,24,207,173,110,55,135,45,181,187,120,111,165,
130,41,39,104,129,140,172,62,92,167,230,201,36,246,62,128,250,87,47,121,98,250,87,139,109,22,210,214,11,88,46,173,188,151,152,146,77,198,198,32,171,167,0,62,210,70,62,239,113,142,69,121,24,156,54,35,3,
82,21,126,20,182,127,240,63,79,203,67,186,142,34,142,42,50,165,123,190,199,103,175,92,233,250,142,149,21,204,16,93,53,213,148,202,208,75,16,196,209,200,72,57,82,123,30,67,103,130,9,4,96,215,51,224,225,
29,142,165,246,9,172,158,226,88,216,195,52,142,193,35,182,44,204,232,20,31,224,124,177,95,238,229,148,16,48,42,95,15,253,191,70,241,78,185,103,5,198,124,63,117,26,11,43,89,89,88,195,46,242,178,15,92,16,
84,140,253,59,115,171,226,212,210,255,0,225,63,251,13,220,22,151,103,95,211,69,187,92,28,128,175,9,99,229,171,30,73,34,82,1,199,240,156,215,124,171,203,23,78,158,33,84,189,72,180,158,150,87,243,245,211,
206,223,135,45,58,75,13,57,209,113,247,36,175,190,191,210,212,249,115,227,162,53,175,197,207,18,34,32,140,203,113,28,160,40,198,55,67,31,249,250,230,178,60,43,163,220,107,254,35,211,244,107,98,4,183,115,
5,46,231,229,141,64,37,216,158,216,80,223,142,43,115,227,197,173,197,183,196,185,160,156,157,233,107,2,135,61,93,0,96,27,219,161,24,60,140,117,60,84,223,8,109,222,127,16,220,148,218,38,146,209,224,128,
200,118,169,145,222,48,6,126,158,156,128,73,175,158,170,185,177,126,244,119,150,171,231,177,239,225,157,176,254,235,189,150,143,228,122,22,163,224,27,13,15,74,215,111,60,63,226,29,126,47,236,248,90,93,
201,116,129,36,217,243,97,194,124,140,56,232,195,156,154,102,148,250,222,147,162,218,221,95,89,221,234,218,124,238,103,125,211,198,140,170,227,118,225,16,194,129,158,224,103,181,122,15,138,173,174,52,
223,0,235,97,150,19,23,216,197,161,139,203,203,92,77,35,121,105,31,76,130,89,151,230,25,62,185,163,197,158,44,211,116,45,23,71,210,174,158,214,125,118,211,78,138,218,214,1,181,201,42,129,85,216,46,118,
227,147,131,215,158,107,215,199,96,112,144,175,105,123,139,150,250,122,252,255,0,224,158,70,31,19,136,169,74,255,0,19,230,183,225,253,122,30,103,171,252,66,186,214,109,167,211,98,89,109,226,151,50,207,
16,124,46,19,24,81,208,168,224,100,116,235,92,207,136,53,134,142,104,160,84,138,73,252,165,37,153,114,17,65,224,117,255,0,35,235,77,213,98,142,198,220,235,50,184,143,89,105,218,88,164,133,64,13,43,49,
98,118,54,70,208,73,33,78,64,24,172,203,91,180,89,218,109,91,195,186,94,169,37,202,156,178,249,150,147,43,14,124,192,241,159,189,253,238,48,220,12,0,57,249,169,243,215,151,180,157,229,109,15,93,83,133,
37,203,22,149,206,174,209,173,110,48,47,237,86,49,4,103,127,149,40,192,24,200,100,56,231,156,117,200,235,94,135,240,115,194,209,223,252,84,240,140,145,72,166,218,227,88,176,146,61,196,58,171,36,232,195,
112,254,239,31,47,185,175,29,153,236,46,224,140,105,210,92,239,96,222,109,165,212,99,205,133,87,31,54,229,249,93,72,36,241,200,198,13,122,247,236,193,40,127,140,222,6,22,215,137,19,127,108,65,16,135,36,
200,251,21,216,16,189,25,64,86,56,39,57,206,42,104,225,218,156,100,159,85,167,204,206,165,210,105,159,123,254,214,214,30,17,240,151,133,252,73,173,92,233,241,234,15,226,120,218,206,247,74,130,28,201,114,
90,50,173,32,101,229,72,10,185,39,130,6,50,43,205,60,49,226,187,75,143,134,18,248,146,234,231,67,99,12,103,74,58,172,240,153,119,205,253,249,38,207,238,212,20,142,50,112,114,202,251,189,253,227,226,223,
194,237,59,196,58,46,185,175,107,23,154,221,196,23,48,164,207,105,167,73,178,73,74,40,249,134,91,9,192,63,42,227,27,137,28,154,242,207,7,220,248,47,95,248,126,53,49,45,204,215,154,142,163,37,236,48,11,
93,238,240,168,80,109,220,96,109,37,87,45,145,158,121,231,138,245,26,247,153,148,94,135,47,175,232,86,58,239,142,155,87,95,15,235,86,23,178,89,207,170,58,218,192,128,78,124,173,177,200,101,83,181,217,
131,54,48,62,239,0,113,94,37,165,252,43,190,212,60,89,167,223,218,133,91,155,24,21,188,71,101,165,93,190,235,0,145,171,125,174,23,27,119,200,60,205,166,48,71,35,57,25,57,251,107,84,248,135,163,104,222,
37,127,177,248,122,250,107,203,29,59,201,147,206,182,242,163,134,35,49,216,172,167,4,224,116,219,212,117,237,158,47,227,199,136,225,178,182,211,53,191,135,115,120,78,25,237,214,59,251,148,180,27,111,154,
216,146,147,70,209,224,150,89,8,56,4,0,124,178,185,4,130,48,173,21,202,219,53,131,213,31,48,217,199,225,189,107,194,122,212,82,106,55,76,182,11,52,19,106,54,183,51,137,110,45,164,148,108,228,229,1,39,
105,195,225,176,91,7,57,174,103,195,95,13,53,29,119,66,129,180,168,180,172,151,54,114,199,16,11,14,216,212,54,86,48,73,142,70,4,48,139,239,48,7,29,235,179,95,17,207,224,175,5,71,173,90,222,70,218,223,
137,97,121,219,73,146,209,94,198,75,89,101,124,137,98,56,59,151,114,144,73,39,158,9,2,181,19,226,174,167,123,160,221,105,70,27,11,123,123,233,160,184,91,171,43,68,133,225,116,113,230,149,13,146,178,148,
102,5,187,118,231,53,231,55,29,165,216,210,86,103,113,224,143,135,31,16,60,43,227,41,180,91,171,93,15,82,138,195,70,77,89,109,35,131,237,47,117,229,203,186,18,169,39,203,21,195,57,192,60,96,12,159,111,
15,253,163,60,95,168,248,195,198,19,95,234,66,217,101,179,86,211,225,134,205,54,65,108,176,187,43,162,169,36,238,18,121,129,137,60,144,8,227,21,232,26,103,199,95,29,232,26,132,17,105,58,201,146,216,90,
61,178,65,36,8,203,229,224,172,114,59,227,115,58,19,158,15,65,205,124,217,113,52,215,86,115,93,150,222,237,41,219,243,238,105,49,203,187,123,18,115,158,228,154,237,193,215,162,170,37,123,127,93,206,90,
252,206,54,72,192,176,146,120,175,151,201,145,144,181,229,178,229,123,169,149,65,31,136,175,161,188,75,165,222,203,29,158,173,161,204,144,107,250,100,158,101,180,142,91,203,144,127,28,82,5,35,114,48,236,
78,51,131,94,27,225,43,5,191,215,68,14,76,97,103,138,83,199,247,27,118,63,49,95,72,217,18,214,206,79,28,126,117,197,152,180,177,26,127,90,35,183,8,218,166,154,42,120,103,226,70,18,75,91,239,12,201,109,
13,190,92,90,24,177,105,30,236,9,35,17,162,225,227,97,147,181,155,229,39,60,215,99,101,227,111,14,107,38,103,183,184,151,204,36,68,158,126,209,117,34,0,196,70,141,157,144,52,101,242,146,242,74,140,30,
121,174,7,195,254,43,240,253,158,161,119,13,255,0,218,34,147,118,192,226,34,192,1,215,167,189,118,22,255,0,240,131,107,152,67,123,166,205,33,232,146,237,12,15,227,131,94,87,179,195,213,214,233,72,250,
26,25,198,42,138,229,106,241,58,36,187,177,99,121,21,165,204,108,175,27,203,50,71,38,45,202,183,153,186,105,88,225,228,206,64,150,21,232,72,35,214,172,222,89,164,165,101,40,100,184,251,182,174,209,143,
50,54,196,129,69,188,103,128,251,73,9,51,113,34,252,143,92,197,199,195,189,62,84,50,233,247,119,86,236,78,67,91,221,48,0,142,135,184,207,53,70,127,14,120,174,202,25,163,211,252,64,248,120,204,109,231,
65,187,114,30,170,74,145,145,215,168,250,96,243,81,83,46,109,222,50,61,90,28,73,78,214,156,89,239,191,4,188,75,160,120,71,196,49,220,92,120,82,206,43,203,72,158,9,175,180,209,177,163,180,203,110,154,224,
240,173,183,203,1,163,25,120,143,170,156,215,113,241,50,206,231,94,184,182,189,240,158,161,225,205,105,32,213,45,164,104,47,221,18,77,190,89,207,148,78,211,184,182,210,167,113,207,60,241,138,249,36,223,
248,214,214,91,153,167,179,211,47,74,5,54,98,54,48,24,101,0,143,55,7,0,201,243,31,152,131,145,242,144,69,92,131,199,6,204,185,213,180,93,90,216,37,185,220,203,25,152,220,177,45,152,76,152,62,92,89,125,
202,19,5,8,202,156,124,181,232,80,197,99,104,45,99,207,99,205,196,97,178,172,100,220,161,83,145,179,236,109,83,195,73,47,138,124,107,113,117,225,219,155,88,77,216,158,43,216,100,45,246,172,237,44,192,
116,12,11,16,70,123,86,47,199,189,17,147,195,126,28,213,244,173,18,43,203,27,17,117,111,117,37,178,19,115,26,190,54,28,47,45,16,108,153,59,174,3,14,134,188,19,65,248,175,4,63,108,77,31,198,114,105,243,
79,0,185,152,13,200,96,198,236,31,44,255,0,173,156,101,65,44,118,200,58,225,133,122,61,175,197,207,23,71,4,177,253,166,219,81,25,23,30,76,172,163,114,29,223,52,147,71,133,142,38,218,48,112,89,79,13,144,
115,87,95,52,167,86,159,179,171,6,191,31,204,156,46,75,90,133,85,91,15,82,50,183,157,191,47,153,228,112,111,129,86,69,142,61,177,186,249,115,51,121,171,4,174,164,5,82,114,39,157,131,96,74,223,44,160,237,
56,106,135,81,180,18,73,114,240,220,181,165,204,97,92,207,21,195,70,240,141,219,68,143,63,222,222,48,85,71,240,255,0,171,144,21,32,214,182,173,40,23,38,246,41,163,134,43,137,156,108,134,211,106,69,185,
131,60,54,182,228,242,191,41,243,1,228,127,172,74,162,191,104,142,40,240,232,242,64,202,84,25,150,72,236,221,213,85,75,202,114,38,157,131,48,71,198,217,71,200,248,108,19,243,147,139,131,188,94,135,220,
82,168,170,47,125,89,190,131,83,89,185,181,178,180,143,91,179,186,213,153,87,247,215,246,214,233,5,246,193,180,52,242,193,194,16,187,190,117,249,93,113,145,184,30,55,62,202,36,138,107,139,25,227,189,181,
132,226,73,33,206,232,143,80,36,67,243,33,199,60,140,99,185,172,121,224,144,140,69,15,202,202,46,17,102,144,199,157,167,62,124,238,126,101,69,224,0,70,232,143,202,192,161,200,163,246,24,173,245,171,77,
110,221,10,235,9,186,222,210,227,38,11,155,166,234,234,74,114,34,83,150,242,198,70,6,248,247,41,219,95,83,147,241,134,59,3,104,77,243,195,179,253,31,79,197,121,31,37,157,240,78,93,152,222,165,53,236,234,
119,93,125,87,95,193,249,155,128,112,49,211,173,0,19,249,212,150,122,146,78,46,91,90,88,204,42,16,91,106,86,17,229,238,228,32,150,70,182,64,67,156,13,193,227,35,120,39,11,145,138,157,97,73,180,244,212,
108,110,173,175,180,199,98,171,119,105,40,146,45,192,144,84,145,202,176,32,130,172,1,4,17,95,168,229,60,71,128,204,146,84,165,105,255,0,43,223,229,223,228,126,69,156,112,190,99,149,182,234,195,154,31,
204,182,249,246,249,149,113,140,241,74,7,52,252,123,82,226,189,235,159,58,52,12,17,205,114,126,53,240,118,159,226,25,35,188,251,69,222,159,172,64,133,109,239,172,167,104,165,78,165,114,87,147,130,114,
58,17,206,8,205,118,0,116,20,229,0,182,222,128,247,172,113,24,122,88,138,110,149,104,169,69,244,102,180,43,213,161,81,84,165,43,53,213,30,65,171,120,192,69,21,159,132,62,44,233,113,120,210,13,31,228,182,
214,225,185,146,215,83,179,14,21,202,69,49,39,112,251,185,73,14,15,25,99,95,99,126,207,26,215,195,235,207,7,217,89,120,79,196,240,205,123,105,42,34,36,241,172,23,81,100,229,82,120,135,202,89,185,30,98,
224,57,32,215,231,134,189,120,218,158,187,226,11,201,162,150,55,147,86,184,132,91,201,247,134,198,242,199,30,234,129,135,177,21,232,191,4,62,10,234,126,57,23,119,250,127,138,46,60,61,170,193,55,151,102,
109,70,230,14,23,204,5,177,202,168,42,135,60,174,122,131,140,31,231,124,226,142,30,24,218,180,233,233,8,201,164,157,221,181,183,200,253,135,4,234,207,11,9,77,251,205,38,253,108,125,75,241,175,84,26,159,
141,174,17,173,22,25,172,135,217,218,64,121,156,14,67,55,184,201,21,192,1,93,15,141,182,191,137,47,37,73,101,148,177,85,152,204,155,36,89,130,40,145,92,96,97,131,2,122,96,130,8,224,138,193,199,62,245,
251,167,14,198,156,114,156,58,164,238,185,87,91,239,171,252,79,203,51,135,55,143,170,230,172,238,255,0,224,126,3,64,231,165,47,76,208,120,25,254,84,160,103,154,246,110,175,99,204,215,113,65,0,231,211,
154,220,253,143,60,72,218,71,198,109,75,70,251,71,153,23,136,222,79,244,88,185,16,50,6,117,145,207,64,113,145,180,100,242,51,94,17,227,125,91,89,190,214,111,116,219,121,26,13,62,9,54,109,67,180,190,7,
37,155,250,84,62,1,150,79,15,120,175,69,213,237,165,97,113,101,121,20,192,198,113,128,24,110,25,247,93,195,241,175,23,54,171,74,173,25,80,123,254,165,225,170,212,165,94,21,98,180,139,191,175,115,238,79,
136,223,9,53,123,173,118,243,81,208,101,183,158,222,230,70,148,193,43,249,111,27,19,146,1,198,8,207,78,149,226,154,246,145,168,232,186,148,150,26,165,156,150,183,104,50,82,65,212,118,32,244,35,220,87,
188,232,191,26,180,123,158,62,214,140,51,223,21,139,241,183,196,250,63,136,252,45,98,246,254,75,223,67,114,54,186,253,224,133,78,225,244,233,250,87,30,93,140,204,33,82,20,107,197,56,237,126,168,237,196,
98,50,188,66,148,232,205,198,111,163,217,158,33,129,211,189,31,90,92,113,64,239,95,78,121,198,150,133,123,127,165,92,201,170,233,179,249,55,118,177,179,161,82,55,55,28,168,207,168,205,75,226,187,201,117,
13,118,238,250,107,131,113,52,229,92,200,201,180,182,84,99,60,14,216,231,28,214,73,194,225,159,24,28,156,215,45,240,199,94,190,215,188,57,119,54,171,28,241,234,246,90,149,205,149,194,74,49,145,28,132,
70,192,237,80,65,77,189,51,198,57,53,231,205,42,120,168,206,79,227,186,90,45,52,190,251,244,103,116,111,44,44,146,95,13,155,213,250,108,117,67,210,157,222,148,15,126,104,245,175,64,225,16,125,41,202,48,
40,245,193,167,1,192,20,0,98,157,207,106,64,42,69,25,29,168,24,208,61,170,69,94,122,83,128,167,129,74,227,176,168,6,41,216,237,64,28,83,240,59,210,24,138,61,233,14,49,78,250,126,20,152,206,104,6,38,63,
149,40,31,157,29,56,167,0,120,233,64,143,206,31,19,120,143,86,179,180,123,89,212,219,219,223,65,186,1,45,172,98,88,148,54,66,38,6,73,40,191,47,29,71,76,117,242,191,19,95,92,234,58,197,197,235,219,178,
38,164,197,209,124,178,27,3,129,211,169,24,207,28,28,113,199,79,98,176,187,211,238,47,53,109,93,244,177,54,183,28,49,79,167,205,52,141,60,97,150,60,22,110,122,40,97,184,14,121,3,183,59,30,41,241,180,26,
132,215,90,94,141,161,135,144,236,150,223,82,142,84,96,173,180,110,87,36,124,160,41,218,74,246,207,61,107,243,101,135,82,77,212,155,138,221,105,123,244,242,62,250,85,154,183,36,121,165,247,88,249,237,
98,185,187,147,41,16,36,228,224,12,116,234,72,252,15,90,216,208,172,124,219,232,238,39,11,61,186,175,155,34,100,110,117,24,59,78,122,110,28,15,233,90,90,156,58,204,218,221,220,80,44,178,93,74,225,165,
91,120,124,178,92,198,187,148,122,0,164,15,79,204,214,151,133,188,59,226,43,215,183,123,81,110,144,92,200,35,128,158,36,44,118,140,128,71,0,43,231,175,64,107,207,141,57,186,138,48,87,58,28,215,45,222,
132,23,171,96,47,39,146,91,8,34,211,161,69,8,172,234,223,100,118,229,152,150,108,237,12,7,78,71,29,133,108,216,94,201,6,133,61,141,236,113,220,67,173,41,34,73,101,105,101,140,39,221,109,223,242,205,64,
233,216,147,158,181,106,15,10,234,154,38,155,171,233,177,24,173,245,43,36,141,238,77,190,215,253,222,238,122,142,64,37,72,233,206,120,172,205,111,78,189,178,214,110,52,167,210,108,237,38,22,97,163,142,
86,12,39,145,57,145,212,228,29,206,73,224,240,48,222,162,186,37,74,80,90,239,233,222,250,127,86,242,50,141,85,37,166,196,254,35,149,181,77,106,25,237,172,111,53,40,35,100,72,173,23,37,16,132,25,249,192,
224,247,36,243,134,60,128,42,246,131,226,61,51,195,58,160,243,244,150,123,169,97,229,100,156,238,132,183,57,0,12,147,199,67,219,0,83,188,28,183,246,247,105,5,219,71,110,99,65,31,216,98,194,205,56,0,239,
98,54,145,180,101,50,196,145,156,247,197,117,186,150,129,166,248,138,254,54,182,178,104,33,0,44,151,22,177,124,243,40,0,134,14,65,194,6,56,39,36,228,156,98,180,133,26,137,57,193,218,75,200,110,164,111,
202,246,103,85,164,124,64,179,212,210,56,172,109,62,221,118,241,45,185,142,208,111,86,224,178,143,155,28,2,8,111,67,199,56,53,115,195,122,124,176,223,234,183,119,114,23,185,189,153,132,142,20,43,22,194,
134,24,35,160,0,1,232,6,61,73,225,252,29,225,189,67,65,91,214,178,212,151,79,50,196,77,179,76,227,40,170,228,110,14,125,72,42,91,175,205,219,154,245,95,14,92,190,166,101,146,123,73,77,245,202,33,50,137,
65,45,2,103,166,56,27,155,119,61,112,57,237,94,230,14,172,177,49,135,59,247,175,166,158,86,191,169,230,98,105,71,15,41,56,175,118,218,253,247,183,161,214,197,166,197,229,69,9,141,126,201,20,97,17,67,224,
250,245,29,70,122,119,174,119,80,209,214,214,250,223,86,208,101,17,93,194,204,151,22,110,203,139,190,71,190,4,163,111,202,222,153,7,177,27,242,222,32,182,2,108,101,84,177,140,13,165,71,108,142,216,252,
123,214,85,253,237,169,188,123,201,38,128,69,177,85,206,194,114,114,66,174,125,178,78,125,197,125,13,104,81,140,20,86,143,190,205,127,95,119,67,193,167,42,211,155,147,219,183,70,90,211,239,236,190,202,
154,189,187,72,144,206,8,150,39,82,165,92,54,24,21,60,134,7,168,252,171,19,199,215,214,218,135,135,164,190,210,230,183,155,88,208,220,223,65,15,154,163,115,162,157,241,29,221,4,145,51,166,113,252,85,145,
123,164,219,106,218,193,49,200,87,101,212,113,239,134,49,21,193,249,73,27,178,112,203,128,6,58,48,7,61,106,228,250,164,150,130,53,215,60,49,246,152,36,86,131,237,86,209,68,84,231,130,90,38,125,234,8,201,
192,15,211,175,74,243,101,138,148,233,202,157,85,104,190,182,109,95,191,151,226,188,206,216,225,161,25,70,116,245,146,233,116,191,61,253,55,37,212,116,248,60,77,160,195,115,103,118,150,179,93,192,146,
217,92,43,143,220,49,27,215,140,130,71,35,35,142,51,92,167,133,166,155,92,215,124,45,117,169,181,184,49,139,216,210,19,106,162,91,91,200,60,181,105,209,199,203,34,150,31,40,218,9,222,115,195,16,54,126,
15,234,90,94,159,163,106,94,31,150,246,59,168,116,27,207,177,91,220,205,24,95,50,34,3,196,171,187,230,37,81,130,158,7,74,138,222,216,193,241,218,40,39,188,37,53,61,34,75,205,36,174,60,184,228,119,83,119,
180,127,121,132,113,28,251,31,122,224,250,174,26,110,158,34,130,87,147,74,93,175,231,211,117,111,59,223,177,216,170,213,132,170,81,169,116,146,110,47,175,203,229,249,30,47,251,83,72,179,124,64,211,163,
58,124,118,87,81,233,192,78,240,201,152,231,38,67,181,144,117,80,0,97,181,185,25,198,88,12,214,63,129,160,186,186,215,188,57,109,4,66,201,100,185,42,210,66,78,208,175,111,34,60,191,49,35,59,73,60,113,
154,185,251,69,35,77,241,174,238,222,105,20,194,150,182,202,172,131,113,41,135,111,196,228,181,47,129,53,9,238,188,117,225,171,37,142,71,211,108,174,15,157,12,7,204,103,87,67,184,1,140,156,176,28,118,
0,142,149,243,216,154,124,248,247,7,252,214,252,108,123,152,89,168,224,212,151,107,254,167,105,226,121,124,87,170,195,55,132,181,43,219,59,95,236,54,182,154,75,189,54,70,73,175,29,219,100,14,62,92,35,
18,75,112,196,238,80,64,193,166,252,64,240,62,157,160,223,234,190,42,182,191,68,179,88,86,107,136,166,136,167,204,0,70,242,155,140,238,33,112,164,103,36,243,205,110,248,90,27,121,190,45,235,239,61,202,
91,199,30,175,230,94,37,204,91,26,59,120,109,208,198,28,144,64,27,164,61,51,193,207,29,186,123,205,54,243,198,62,42,138,75,216,3,104,90,59,230,214,213,24,5,154,228,28,249,210,145,252,8,54,132,78,248,201,
246,232,141,37,143,197,74,53,27,114,217,125,253,109,217,47,191,212,231,117,62,171,66,50,130,73,110,254,239,213,158,37,227,127,13,235,150,23,250,109,230,177,4,112,219,92,196,90,209,18,93,196,1,180,157,
195,31,43,124,195,142,127,74,201,150,53,59,29,186,39,65,94,189,251,65,69,53,173,198,131,111,113,32,121,188,185,93,182,131,140,29,131,191,211,181,120,252,210,110,7,29,42,177,56,90,120,122,178,165,13,151,
114,40,226,37,94,154,156,186,246,40,222,6,243,35,154,50,22,104,207,200,72,227,220,31,80,107,212,127,102,189,95,195,186,47,237,9,240,207,95,213,117,171,93,51,71,179,212,18,231,81,150,250,81,18,216,183,
217,231,86,86,102,192,101,222,84,6,31,222,21,230,55,3,114,247,227,165,122,159,236,145,160,233,254,33,253,164,60,13,166,106,154,93,150,167,99,60,247,34,123,43,216,68,145,92,160,180,159,247,108,164,16,121,
193,231,186,250,226,184,253,155,115,78,44,235,140,215,45,164,126,175,120,163,226,215,132,228,248,111,14,181,225,93,91,79,241,10,106,83,53,174,159,45,129,55,48,52,195,174,242,159,116,47,83,222,188,35,246,
121,215,180,73,91,82,73,36,190,68,188,146,38,188,128,219,149,140,92,111,119,50,231,0,196,174,168,185,220,118,128,84,17,146,107,229,15,141,30,19,211,62,23,248,167,81,184,240,63,139,82,215,86,178,188,105,
117,127,12,216,234,19,121,58,120,9,182,35,230,41,92,191,205,196,111,187,106,144,6,20,87,39,225,15,137,94,34,209,109,53,77,90,109,50,255,0,85,187,146,4,88,210,73,204,40,133,88,238,149,158,31,153,216,135,
12,75,161,27,130,224,131,89,123,107,79,84,82,166,173,163,62,252,189,111,12,124,93,248,203,174,90,248,57,108,37,191,211,180,181,73,181,100,184,19,67,102,204,89,25,85,7,202,211,20,99,140,110,27,79,56,227,
62,63,251,65,120,126,31,7,120,10,11,239,7,140,125,142,37,17,219,105,113,46,18,225,68,138,229,93,240,234,138,153,50,134,12,73,33,148,113,94,121,240,31,226,162,120,95,206,189,210,254,29,89,232,58,140,210,
165,180,218,153,115,115,36,114,20,89,37,80,65,5,124,200,202,48,206,1,39,39,147,138,238,52,175,23,235,222,57,211,181,29,39,89,134,205,172,110,101,190,185,211,198,161,114,67,25,112,216,249,17,6,54,41,218,
155,190,102,56,220,78,50,114,169,40,114,234,52,154,60,215,225,255,0,132,35,212,96,62,28,213,124,69,13,171,206,134,93,54,194,73,10,48,147,134,243,12,140,2,240,153,0,3,130,14,122,244,167,241,27,225,158,
161,224,189,3,70,241,51,234,214,90,254,141,117,34,218,226,206,109,222,68,228,18,232,224,116,1,129,29,114,79,160,230,185,175,139,137,226,15,6,235,250,22,133,168,134,181,154,245,202,233,95,110,43,133,68,
98,153,70,35,230,25,85,77,160,100,22,224,17,205,119,159,7,188,43,105,226,143,12,107,240,248,147,92,159,78,209,244,184,162,121,110,149,20,66,190,100,152,144,183,97,180,96,1,206,72,174,37,8,84,92,188,186,
154,78,46,23,191,67,207,117,169,116,163,173,218,207,103,60,45,167,109,40,37,134,229,136,60,244,103,43,247,193,37,72,228,113,140,144,51,92,205,228,118,159,105,98,215,12,134,57,11,171,4,40,132,14,85,79,
25,228,142,189,51,78,183,158,212,44,208,219,149,88,67,98,206,70,31,235,198,112,171,143,231,233,90,151,182,208,219,134,149,102,13,48,118,23,46,100,80,165,66,240,2,129,215,191,39,181,115,46,88,207,146,164,
126,227,39,123,104,81,240,149,241,189,241,59,221,121,97,25,149,67,99,185,245,175,86,240,110,165,53,238,155,127,36,164,102,27,169,34,80,15,97,140,87,144,248,17,148,206,147,27,120,227,103,153,188,178,135,
157,128,1,243,2,115,146,114,107,215,244,24,98,181,177,41,2,133,66,89,219,253,166,60,228,212,227,28,61,167,186,180,181,142,172,61,253,158,166,188,190,21,176,212,33,89,222,0,36,113,146,195,138,197,188,240,
28,101,191,117,43,143,99,205,125,231,225,127,129,190,27,213,124,7,225,251,219,93,79,80,181,186,185,176,138,89,24,149,149,89,153,3,19,130,56,228,244,6,176,117,191,128,90,252,5,155,78,212,108,47,144,125,
208,251,161,127,234,63,90,197,224,233,85,87,113,220,218,53,170,193,217,51,226,8,124,57,226,29,53,139,88,95,203,17,94,158,92,140,191,203,138,208,135,196,158,60,211,14,36,157,238,99,28,98,88,195,255,0,245,
235,233,45,95,225,119,140,244,215,43,63,135,174,228,37,73,6,221,68,203,199,186,230,188,247,198,26,77,254,151,167,222,52,218,108,241,222,69,19,24,225,154,22,82,205,142,6,8,201,252,43,37,151,242,187,83,
155,143,204,211,235,82,127,28,110,113,54,31,19,174,35,62,94,165,164,68,220,124,219,50,167,242,57,173,171,47,29,248,74,226,76,205,12,214,142,78,51,229,228,126,107,94,13,169,232,30,38,188,146,194,225,175,
226,130,254,89,190,207,42,207,39,146,178,28,130,29,81,135,60,54,8,246,24,235,93,39,135,109,116,251,157,73,44,39,145,103,89,24,162,56,4,58,176,36,114,62,163,30,196,226,181,173,135,199,97,210,247,175,234,
137,133,124,53,71,107,88,246,192,60,19,175,169,83,123,167,220,51,116,89,182,150,31,247,208,6,170,221,252,59,211,89,95,251,62,73,173,209,211,105,54,215,44,163,30,152,59,151,28,14,43,207,239,252,35,105,
246,165,130,27,161,231,55,43,30,119,30,158,157,170,149,182,141,174,233,199,204,211,53,25,81,65,56,242,102,101,7,242,56,172,126,185,137,142,149,41,223,208,218,48,165,246,39,99,209,103,208,188,81,110,147,
11,127,17,220,59,189,168,181,45,121,111,230,157,128,146,184,116,33,149,148,146,67,12,17,154,170,110,124,99,100,172,95,76,211,111,150,40,89,96,91,105,66,170,200,79,204,198,39,0,29,223,196,14,70,64,35,7,
154,228,172,188,115,226,139,9,68,55,23,113,221,21,224,172,234,172,127,49,131,93,13,159,196,213,35,110,165,162,130,15,83,12,157,127,6,31,214,161,226,176,146,210,164,45,234,191,202,231,109,44,86,62,146,
253,221,75,252,239,249,147,255,0,194,89,107,100,242,29,83,74,214,116,232,213,18,89,18,120,183,125,170,127,149,67,75,48,201,118,85,80,20,244,42,118,184,97,130,53,109,181,109,23,83,130,107,56,181,8,165,
141,209,86,79,34,99,25,185,79,225,68,3,152,97,86,98,114,14,232,152,101,73,92,131,37,135,141,188,31,124,184,154,89,172,216,245,89,99,32,15,197,114,42,223,246,47,132,181,192,100,130,93,58,229,155,161,66,
187,191,241,220,26,184,208,194,212,119,167,45,125,127,166,118,199,62,198,65,37,82,38,147,72,22,25,12,178,110,145,115,12,179,67,7,152,137,146,91,200,142,53,63,60,132,5,221,140,43,253,248,202,176,32,50,
198,59,168,5,192,211,238,100,210,239,138,134,119,137,213,150,221,91,133,103,99,242,205,35,30,133,134,215,198,214,10,224,19,206,94,124,60,137,12,146,105,154,142,165,99,35,168,82,208,92,150,225,126,232,
1,243,140,118,193,166,205,164,248,198,193,29,225,213,45,181,24,86,79,49,34,188,181,216,21,200,195,18,70,67,110,207,32,245,60,245,230,165,224,37,23,120,72,238,135,16,81,154,229,171,31,212,234,110,117,168,
173,44,173,31,93,177,149,101,108,9,239,180,171,70,120,208,103,111,159,37,176,37,227,140,156,103,105,37,9,195,168,3,117,109,79,103,36,118,194,230,55,142,226,205,142,213,184,129,183,71,159,66,122,171,123,
48,7,218,188,250,45,91,89,181,51,54,165,225,185,230,73,103,89,165,54,183,75,35,78,193,66,135,118,60,150,80,0,198,48,203,242,176,32,12,58,203,197,122,125,142,185,109,172,11,189,67,77,191,142,54,142,232,
72,141,20,87,220,124,177,72,135,40,97,25,249,75,2,209,244,4,169,53,245,57,95,20,102,88,43,66,191,239,33,231,191,201,255,0,157,207,153,205,120,107,38,204,111,60,52,149,41,190,219,124,215,249,88,237,113,
218,148,116,193,239,85,108,124,77,225,205,98,225,68,18,69,111,230,2,82,234,214,65,53,164,140,62,244,123,65,50,35,143,69,14,152,228,30,195,83,236,210,24,62,209,3,71,115,107,140,137,237,164,18,199,143,92,
175,79,199,21,250,46,93,158,224,113,241,94,202,118,151,242,189,31,221,215,229,115,243,124,203,135,241,249,124,159,181,133,227,252,209,213,125,253,62,118,62,114,248,237,163,222,104,58,188,222,39,178,72,
164,211,117,39,69,185,143,163,67,114,23,104,147,61,213,149,64,62,132,15,90,206,248,105,241,7,197,186,30,165,101,171,193,127,44,118,139,34,179,71,10,133,4,103,159,175,25,228,215,179,252,86,240,251,248,
151,193,151,122,125,179,71,246,165,101,154,16,205,133,98,185,202,147,219,32,145,95,42,120,87,86,180,109,18,239,70,212,91,81,23,54,192,46,148,109,209,93,101,59,142,248,231,83,130,171,180,174,28,19,180,
130,8,193,175,207,184,199,44,132,49,158,211,147,221,154,187,245,235,250,63,153,245,60,59,141,149,76,34,143,55,189,29,62,93,15,208,111,9,120,189,126,46,107,154,96,93,10,226,211,75,48,53,163,106,209,149,
63,104,187,219,189,84,198,91,123,5,84,112,24,2,122,231,3,174,87,136,116,171,173,27,86,184,211,239,20,9,161,108,100,114,24,118,35,216,138,228,127,101,15,134,186,135,141,95,78,241,45,167,139,143,135,174,
244,41,150,125,46,43,123,101,156,163,242,172,206,172,113,181,176,234,64,193,60,242,43,219,191,104,75,155,244,187,211,164,213,244,207,43,84,140,44,79,121,110,163,236,151,192,176,95,145,137,220,174,185,
4,163,14,135,134,32,102,185,56,51,55,158,7,16,176,115,149,233,73,217,47,229,127,158,189,122,63,206,184,139,47,167,139,162,235,193,90,113,87,245,95,214,221,143,41,103,17,202,161,195,108,110,50,7,67,239,
237,82,52,37,33,30,99,14,73,232,220,129,234,125,42,157,196,214,172,74,188,187,178,11,121,100,101,78,58,240,122,254,53,28,83,236,182,220,210,181,197,179,0,170,84,143,48,46,0,25,245,224,117,235,95,170,78,
178,83,242,242,62,34,20,91,135,159,153,202,120,203,71,146,75,217,239,173,85,165,80,51,60,104,185,96,122,100,14,253,51,92,27,106,198,20,119,179,132,110,85,36,22,231,119,28,100,250,125,43,217,237,230,179,
158,234,229,134,35,129,118,33,89,9,80,58,240,127,48,57,175,8,212,165,137,117,43,152,215,30,82,206,233,245,1,136,175,58,166,26,19,126,209,107,115,151,28,229,13,22,151,62,197,178,253,156,225,214,124,45,
164,235,126,30,241,52,145,53,253,164,87,2,59,200,7,27,212,49,27,147,7,191,29,107,136,188,240,182,161,224,205,106,227,67,213,46,190,209,126,141,243,58,150,40,87,104,35,102,121,199,60,251,215,214,159,179,
38,164,218,183,192,95,5,93,72,219,165,77,57,32,144,255,0,181,30,80,255,0,232,53,196,126,213,126,20,186,187,181,177,241,38,156,4,107,7,238,117,9,71,38,56,191,133,192,245,201,219,158,217,207,106,241,240,
25,165,104,99,61,141,103,117,118,150,219,158,204,178,12,11,195,71,23,66,54,147,73,238,237,170,237,169,224,9,34,200,187,163,5,215,113,82,71,78,61,61,69,56,178,24,195,6,27,73,192,63,142,42,44,132,96,138,
219,87,25,80,0,252,135,225,80,67,57,55,14,165,71,148,138,89,139,30,51,156,97,79,67,235,95,99,237,182,125,207,35,216,238,135,220,203,178,41,55,130,129,80,238,30,156,112,115,245,174,103,192,74,33,241,215,
143,52,232,245,31,61,231,146,215,84,54,141,32,47,111,186,51,19,141,191,194,9,136,55,252,10,183,53,119,118,210,75,199,39,148,174,66,151,151,229,84,82,70,89,143,162,128,79,225,95,30,124,8,241,109,167,134,
252,103,226,63,17,93,95,139,145,20,83,180,137,44,175,155,232,132,140,198,113,32,207,0,40,60,131,159,48,99,167,62,6,119,140,150,22,48,148,23,51,77,53,253,122,93,30,214,85,132,88,133,52,221,147,86,103,219,
193,112,121,239,78,3,140,215,47,224,31,29,104,222,57,177,107,221,47,106,20,3,124,66,100,114,191,130,242,49,199,80,58,241,154,235,0,231,145,94,206,95,140,88,188,52,107,45,47,186,236,250,163,200,198,97,
158,26,188,169,118,252,136,241,214,157,143,81,214,164,219,205,59,24,226,187,110,115,88,136,41,169,0,199,122,118,49,78,81,214,139,133,134,129,79,81,212,83,177,197,40,20,134,3,233,78,199,20,1,210,157,130,
59,208,2,122,115,72,71,83,78,29,5,47,225,205,0,198,145,233,74,7,25,165,2,148,80,7,196,151,26,93,131,248,70,59,88,52,169,109,173,89,192,149,24,180,50,46,254,97,40,163,185,192,4,55,177,62,254,117,103,167,
219,248,62,206,121,45,110,33,146,254,230,216,51,67,120,163,120,81,130,164,133,60,177,13,213,125,79,161,175,70,215,124,115,166,95,120,90,59,59,41,38,130,252,91,165,133,164,9,42,201,36,69,200,134,59,146,
221,11,124,201,132,201,218,55,100,228,241,205,39,134,52,123,109,34,255,0,33,108,239,204,242,60,193,101,221,52,114,174,21,35,8,56,28,227,35,175,83,95,1,90,188,113,86,157,43,105,31,75,31,107,74,155,162,
159,61,245,127,127,245,255,0,0,230,124,21,174,220,69,173,73,226,29,116,193,2,193,41,103,41,17,68,86,224,148,192,60,18,66,47,83,146,78,115,138,244,177,117,166,218,248,127,193,16,60,240,157,78,194,104,238,
138,66,50,33,149,148,22,200,56,33,48,236,160,30,78,64,237,92,110,181,103,171,136,252,63,164,106,105,182,226,89,225,140,137,28,6,147,202,144,62,236,133,225,202,110,221,192,200,231,233,55,143,181,21,30,
44,213,14,158,178,69,28,201,111,7,219,45,200,253,230,19,231,139,57,236,78,71,92,103,60,215,28,49,126,194,155,157,238,219,94,187,223,244,71,84,168,243,201,71,162,191,166,223,240,79,96,212,252,75,162,218,
94,223,89,149,181,146,232,91,24,217,94,85,89,16,146,74,180,133,240,170,114,24,175,82,113,194,147,94,108,75,107,114,106,90,196,98,239,254,18,198,227,84,182,141,215,99,196,2,20,183,137,58,140,174,55,49,
33,139,18,50,48,20,113,186,6,178,186,117,190,177,106,183,19,203,11,110,48,206,219,157,153,179,140,182,236,183,247,190,241,231,24,24,226,189,11,71,215,214,15,7,180,26,62,157,101,111,126,193,98,190,189,
121,150,221,45,217,137,88,118,252,191,189,148,228,29,171,143,114,9,173,40,226,62,187,86,83,155,181,238,237,233,162,95,214,155,153,78,146,195,210,81,130,187,90,127,159,245,232,59,194,45,119,168,106,119,
210,91,104,82,193,104,98,85,116,184,84,224,130,119,56,143,32,186,174,48,55,30,160,156,158,251,250,133,157,242,232,241,62,149,4,211,223,72,12,75,108,176,20,83,129,157,225,176,84,14,164,177,108,112,48,57,
174,143,69,130,11,189,49,37,182,145,22,238,242,32,17,30,220,55,158,20,144,0,108,14,65,12,64,36,16,57,230,182,52,230,190,142,217,109,229,150,121,167,149,8,19,179,109,222,227,251,171,217,78,58,228,103,29,
235,208,163,132,147,139,131,87,79,127,39,219,111,243,94,167,53,74,235,153,59,217,173,188,237,253,122,156,16,240,244,150,147,71,121,121,107,18,42,64,160,55,151,230,0,95,32,40,220,189,114,7,160,228,231,
214,187,143,15,166,156,97,251,58,77,111,43,198,63,117,20,123,86,77,191,116,243,146,72,200,199,90,93,66,222,11,152,21,111,226,207,151,149,142,63,52,225,113,220,244,220,196,158,153,233,211,185,170,54,246,
118,154,107,167,216,237,254,109,170,100,184,14,255,0,42,142,78,227,248,231,31,95,90,236,195,202,150,22,170,140,44,213,173,173,239,248,232,115,214,133,76,77,54,229,117,175,67,122,101,211,202,76,137,35,
238,192,103,85,79,60,18,56,218,67,3,131,158,49,145,88,178,141,80,202,18,11,185,99,176,134,48,89,153,35,121,100,56,251,192,176,199,7,249,87,83,25,141,108,34,138,200,162,66,227,106,46,222,48,123,253,112,
73,230,160,123,13,240,198,131,114,67,156,133,83,144,71,4,103,187,116,207,39,140,215,208,202,154,169,105,45,61,52,253,79,5,79,217,55,25,43,250,234,114,211,216,94,196,151,79,28,119,182,215,23,87,40,31,230,
251,65,120,199,5,138,16,87,166,79,203,211,62,213,95,71,209,26,194,11,176,33,146,249,112,26,206,89,138,164,178,32,3,49,182,238,56,234,49,183,57,60,14,167,209,86,59,128,228,137,87,111,109,202,121,250,138,
175,123,101,36,202,234,146,24,201,61,80,0,15,224,115,207,189,101,83,46,132,181,87,79,229,255,0,13,248,23,12,124,163,163,181,190,127,240,255,0,137,225,176,120,114,215,87,248,161,226,59,1,162,91,53,196,
246,246,247,239,37,236,65,163,133,24,249,126,82,133,32,228,136,136,45,147,198,71,24,21,205,248,187,76,131,225,255,0,140,180,107,251,205,98,82,218,116,114,92,104,182,147,220,177,183,66,24,6,69,148,130,
202,2,146,25,95,35,18,101,79,202,69,122,215,142,52,59,219,95,16,104,126,42,209,225,50,234,90,34,58,92,90,91,201,181,239,236,177,151,140,103,130,224,157,202,15,25,200,227,57,16,252,76,209,45,254,36,124,
54,138,251,195,211,71,45,228,76,183,186,121,150,61,164,176,31,52,77,208,161,117,37,72,61,51,210,190,123,17,148,211,246,117,41,194,254,217,55,36,255,0,153,94,251,109,163,236,180,118,61,138,89,148,249,169,
212,118,246,123,63,39,183,174,171,245,62,119,253,162,47,229,95,140,119,218,149,163,178,199,113,101,107,37,179,28,28,196,81,182,145,248,238,255,0,38,185,175,134,154,131,217,248,170,203,83,109,241,155,91,
232,229,148,32,35,204,128,228,56,24,201,102,29,112,58,240,43,3,89,158,254,107,216,45,245,72,76,55,26,124,17,216,8,76,70,55,137,34,45,181,92,18,78,240,24,228,247,227,234,119,116,45,15,84,214,53,11,91,77,
58,54,102,149,194,163,153,2,40,32,128,14,73,227,25,31,211,53,242,243,175,63,172,123,101,241,94,255,0,59,220,250,74,116,161,236,149,63,179,107,124,143,81,241,205,229,134,185,168,107,48,104,23,190,117,198,
189,171,105,233,97,228,35,195,35,48,143,23,14,199,251,190,90,237,96,123,142,149,244,86,157,20,26,68,22,176,68,84,199,12,59,95,106,96,100,125,208,7,110,230,190,104,248,73,5,236,159,22,180,43,75,136,145,
158,5,188,75,155,150,98,27,203,141,218,49,229,246,229,128,198,122,141,198,189,239,197,254,49,208,244,107,181,211,173,211,237,250,180,199,17,217,218,3,45,196,167,190,224,62,234,231,0,177,56,29,50,43,233,
50,90,180,233,83,173,140,171,104,202,82,254,146,91,179,193,205,105,206,110,150,22,157,228,146,255,0,128,155,123,30,97,251,71,188,207,175,104,143,52,97,25,237,164,116,77,251,138,141,200,8,246,232,9,31,
225,94,57,35,12,170,142,231,154,234,190,34,29,120,222,233,183,30,34,159,58,149,197,152,149,173,85,84,71,98,11,176,16,174,9,206,49,201,39,36,250,116,174,18,197,157,162,44,199,146,236,71,211,60,87,6,50,
171,169,94,83,113,181,251,157,88,90,74,157,21,4,239,98,221,228,209,196,163,39,45,159,186,7,60,244,175,92,253,140,53,41,109,127,105,175,9,92,70,60,165,183,142,233,247,178,23,57,48,20,24,3,145,247,243,187,
160,199,53,227,192,125,220,242,64,198,123,159,122,245,223,217,38,254,13,55,246,134,240,165,220,242,203,26,32,184,44,209,199,188,149,242,91,32,142,184,35,184,232,64,237,154,243,227,204,154,114,103,102,
150,106,199,223,122,47,194,77,10,222,207,90,26,146,255,0,104,234,151,50,201,115,123,117,170,199,29,195,35,203,38,55,72,152,11,41,109,161,84,140,16,55,99,110,43,150,241,239,130,188,43,163,105,119,58,62,
143,167,104,122,117,188,31,190,185,187,91,49,51,201,34,96,108,159,156,147,34,49,218,164,15,152,46,73,175,102,241,45,222,130,218,23,216,180,125,118,53,155,204,181,187,55,55,23,153,81,24,99,181,228,39,60,
46,210,202,50,50,64,174,46,251,194,126,49,213,52,141,83,87,186,211,244,95,20,92,89,197,60,118,139,60,25,75,229,59,90,41,79,56,147,42,171,215,156,147,130,49,74,80,232,53,35,230,27,29,23,89,135,73,210,181,
127,19,248,118,230,93,26,226,233,210,214,233,45,149,46,119,63,221,142,68,251,165,48,216,32,156,0,48,54,247,235,227,208,160,208,117,221,49,180,201,149,45,108,31,237,151,22,186,124,158,116,87,105,41,1,222,
88,221,194,8,226,29,70,70,232,193,218,25,130,215,210,183,218,39,143,53,239,12,104,90,84,154,61,140,118,241,220,39,246,139,75,32,243,163,143,36,179,68,28,97,113,192,199,95,148,17,239,243,63,139,244,235,
191,134,48,91,105,222,27,183,189,155,69,187,189,38,71,212,109,204,79,122,188,249,176,184,60,152,202,101,9,198,27,119,4,26,226,175,21,77,57,116,54,140,219,86,62,145,184,215,135,140,165,240,49,155,86,75,
123,219,43,129,246,152,173,34,182,88,8,116,10,168,230,112,238,152,228,143,40,238,7,29,67,10,249,91,227,196,86,126,12,240,159,138,60,11,161,202,101,109,119,82,243,174,46,35,178,88,18,40,81,203,253,152,
132,98,160,6,85,98,195,174,79,169,3,173,214,124,103,99,96,45,45,96,208,45,37,210,237,175,21,204,218,118,164,143,4,142,232,55,68,68,139,189,225,42,8,202,231,230,231,60,87,129,124,85,215,32,213,245,59,137,
96,183,99,13,229,231,218,17,102,30,92,113,198,78,6,216,242,66,99,144,78,121,3,53,204,241,20,218,93,203,170,249,147,87,56,27,107,41,94,202,57,161,150,45,240,200,118,170,114,216,101,39,112,236,70,112,8,
237,91,250,101,213,165,214,151,97,99,152,82,242,118,81,37,180,131,108,91,186,13,167,183,39,60,138,205,83,29,250,72,230,221,173,86,9,84,162,204,237,25,152,14,126,85,28,19,211,25,192,25,205,84,184,179,51,
193,123,115,4,47,4,182,249,102,203,100,170,158,171,187,215,32,244,244,173,93,40,85,113,147,188,94,150,103,39,52,163,182,164,250,5,195,182,183,52,50,164,94,116,50,108,222,131,28,46,87,30,227,142,181,235,
86,243,98,208,128,79,17,146,127,42,241,143,14,221,75,119,173,203,113,59,110,148,237,5,187,159,122,245,219,6,221,26,175,251,56,175,63,49,135,179,169,101,228,119,225,93,224,153,250,211,240,196,121,127,14,
60,44,167,28,105,118,227,229,28,127,170,90,234,65,207,74,227,254,14,204,179,252,40,240,124,192,228,62,145,108,115,156,255,0,203,37,174,189,70,51,142,149,173,38,212,81,82,221,143,239,92,167,197,61,50,13,
99,225,207,137,180,249,222,56,214,125,54,116,18,200,155,132,68,198,223,62,7,60,117,227,159,74,234,122,213,13,121,85,244,61,70,55,143,205,141,173,164,13,31,63,56,218,114,56,245,173,156,186,50,86,231,228,
71,136,175,117,29,50,231,81,208,164,55,58,132,42,241,108,212,237,146,65,108,225,163,225,124,215,81,131,206,224,167,4,224,227,56,205,103,120,71,70,131,77,212,44,175,53,155,219,139,121,245,105,97,8,110,
34,192,98,204,238,205,24,31,121,118,124,216,224,231,57,197,104,124,72,241,174,159,172,248,126,93,31,64,180,212,44,52,152,174,99,125,110,214,59,147,113,25,149,34,62,87,239,29,68,145,178,2,126,82,57,224,
231,177,224,47,53,69,212,188,33,119,115,46,170,207,172,79,116,144,199,111,22,158,74,37,150,2,59,41,65,136,219,126,122,140,55,44,78,1,2,113,50,110,77,38,218,232,115,71,73,121,157,103,252,35,223,19,252,
83,226,251,63,14,120,110,198,219,80,213,218,198,9,237,101,176,187,249,90,217,243,229,207,114,228,15,44,227,59,148,228,169,24,0,156,87,170,232,62,3,241,117,183,194,77,86,103,211,158,59,93,53,231,179,125,
66,222,229,102,129,90,38,217,43,9,71,240,134,12,55,96,0,65,244,205,103,120,71,85,211,62,26,248,7,90,214,35,176,154,246,31,21,196,116,91,168,167,49,165,214,155,131,33,89,154,68,35,43,32,79,49,95,25,27,
242,112,70,218,246,111,128,255,0,20,101,215,190,17,93,248,22,79,15,219,105,30,5,210,252,29,44,17,106,39,122,44,142,145,176,218,172,70,212,140,40,80,172,223,52,153,44,6,57,174,42,85,146,154,230,216,234,
84,226,180,234,124,172,191,15,26,75,205,50,255,0,204,185,123,132,92,49,132,172,139,62,210,66,111,145,91,32,156,5,39,7,32,115,205,108,248,42,198,225,228,22,90,204,49,180,228,237,124,70,84,103,4,134,0,244,
206,8,35,219,222,164,240,101,205,150,191,166,89,235,214,151,15,101,225,119,146,49,99,121,123,24,62,124,129,199,157,134,141,136,1,92,133,238,126,110,112,65,175,84,248,113,225,133,151,226,238,129,163,107,
26,156,23,250,117,213,239,149,34,91,221,57,100,193,113,131,144,25,14,72,21,234,226,169,97,171,97,229,81,71,84,115,208,171,86,53,20,91,209,156,117,255,0,131,44,36,66,233,30,211,234,189,235,144,241,15,135,
45,116,155,54,190,150,243,200,64,202,138,92,129,150,99,133,80,79,114,107,244,107,88,248,23,225,187,152,199,246,118,161,125,100,221,131,145,42,254,184,63,173,124,231,251,74,252,32,151,194,222,9,254,217,
212,181,40,110,244,168,174,227,143,16,67,251,192,95,42,164,171,112,71,62,185,175,41,101,180,106,206,49,113,181,206,245,138,171,8,183,123,216,249,47,195,254,55,189,142,237,237,108,53,91,216,102,140,176,
49,202,24,125,211,207,7,210,189,27,79,241,183,139,109,226,71,154,8,111,33,35,33,140,121,220,62,163,21,67,83,240,150,149,117,166,52,232,18,210,227,83,137,154,41,222,38,141,174,18,22,27,201,117,39,147,133,
39,129,208,14,69,107,124,60,182,150,27,123,155,57,164,138,81,17,4,176,96,84,158,65,193,247,192,63,82,105,226,114,153,97,226,167,74,109,39,230,194,158,57,77,184,202,41,179,74,219,226,92,5,2,106,122,1,7,
191,148,255,0,209,133,109,90,120,227,193,87,152,89,252,219,102,244,154,2,84,126,35,34,155,117,163,90,206,187,188,132,63,173,98,220,120,62,194,96,199,200,218,222,163,138,228,80,198,193,233,37,47,85,254,
86,54,246,148,30,233,163,106,77,31,192,122,245,204,87,54,199,73,158,250,44,152,166,93,130,104,243,215,105,24,101,207,124,81,97,224,179,166,92,121,250,62,177,169,216,74,56,86,182,185,207,230,27,59,191,
28,215,41,165,120,66,11,77,114,194,233,228,38,24,103,87,145,89,119,13,160,243,250,87,170,173,249,212,36,146,11,120,158,222,198,44,97,149,112,210,103,220,125,223,167,94,107,210,203,112,248,204,117,95,100,
161,175,126,159,51,28,78,46,150,18,159,181,246,142,203,239,57,203,89,124,67,167,234,110,53,95,16,190,167,163,195,4,143,120,247,54,233,27,219,149,27,129,220,131,4,246,43,216,115,245,248,231,194,210,218,
127,194,89,125,44,197,154,205,167,12,216,92,145,17,144,23,192,245,217,208,127,42,250,151,227,70,163,112,159,15,117,157,59,71,146,40,46,164,132,169,102,27,149,96,235,54,54,228,238,49,238,218,125,107,228,
27,59,57,134,171,21,186,202,171,43,73,228,177,221,133,251,128,140,251,87,185,157,210,134,18,141,60,50,155,147,87,109,249,233,162,244,177,229,224,107,84,197,78,120,137,174,91,217,37,228,186,179,245,35,
246,35,185,240,213,166,134,6,141,116,32,155,86,73,166,182,177,187,184,86,153,146,57,166,85,101,81,131,134,31,54,222,113,234,122,215,89,251,66,220,255,0,109,252,57,212,47,86,221,126,217,166,237,156,111,
114,160,101,99,114,125,176,28,254,85,249,227,251,53,120,138,227,192,255,0,25,188,57,172,193,12,50,92,73,123,29,189,193,144,225,21,36,116,142,71,99,180,159,149,50,65,227,129,212,12,154,251,119,199,154,
248,31,17,60,75,224,197,148,73,37,197,149,239,150,170,220,40,48,110,143,241,249,115,239,138,248,63,109,245,76,93,54,165,246,147,252,145,244,48,165,237,233,206,235,236,179,195,116,121,94,224,169,133,92,
220,222,204,75,188,132,131,18,237,199,126,156,112,7,29,115,138,223,181,182,43,172,152,91,4,121,94,108,64,28,129,186,67,144,127,49,248,86,6,145,36,102,219,237,176,171,200,97,14,24,57,193,69,28,51,49,57,
249,249,237,234,77,23,186,132,135,69,91,137,231,101,64,36,14,32,144,135,155,103,203,181,62,172,123,245,62,194,191,101,163,90,42,131,156,250,124,182,222,251,31,3,86,140,189,178,132,127,225,251,30,95,224,
79,23,107,183,90,191,140,238,53,134,130,102,138,241,161,178,183,138,221,130,202,209,150,72,157,88,240,66,140,18,78,79,79,106,93,63,79,150,39,19,92,72,138,253,114,192,49,207,174,58,102,181,98,73,157,182,
199,17,137,49,203,30,42,229,189,186,196,119,100,187,255,0,120,245,175,156,197,241,53,12,36,28,21,78,102,219,118,90,239,210,251,37,248,249,27,67,135,241,121,141,85,57,83,246,113,90,107,167,206,219,183,
248,121,157,7,134,188,67,241,27,70,176,121,124,47,174,234,246,154,92,108,119,170,200,194,16,248,203,30,70,208,121,201,29,123,215,81,97,241,251,226,197,180,45,105,113,127,167,234,176,50,97,226,186,181,
142,93,234,120,32,237,218,72,53,237,223,177,109,217,109,19,197,58,91,149,34,59,152,110,66,48,200,1,208,161,253,82,189,191,94,240,207,133,117,38,81,170,104,154,93,209,148,149,95,58,213,11,49,193,36,3,140,
244,4,215,205,71,139,232,187,186,216,107,235,186,149,159,226,157,255,0,3,208,171,195,53,169,203,150,142,37,175,43,105,248,53,111,196,252,239,212,124,127,115,117,127,110,103,240,242,216,39,156,184,75,103,
96,189,126,234,135,206,50,122,12,159,74,245,53,10,250,143,207,30,194,144,43,152,201,206,210,88,245,237,242,227,21,239,62,50,248,59,224,223,248,70,117,201,180,29,41,116,221,69,244,249,163,134,75,121,27,
98,177,25,206,194,72,206,64,228,114,59,87,206,186,84,183,43,167,220,223,93,65,29,186,252,222,115,25,242,176,249,124,16,11,0,74,131,188,228,254,181,247,220,55,157,211,204,84,165,77,74,202,202,206,207,187,
233,232,207,7,31,149,215,194,165,237,100,157,239,182,157,191,204,204,241,149,148,30,36,211,231,208,101,13,37,158,179,19,91,204,165,177,229,198,23,247,142,48,114,56,96,63,222,43,218,190,95,248,161,224,
221,31,225,207,197,47,4,141,61,36,158,210,91,100,148,181,217,18,23,154,41,66,180,141,192,27,143,152,132,224,1,145,192,21,244,111,132,53,27,217,208,107,19,105,151,31,233,188,219,73,179,9,21,185,98,80,124,
204,15,207,247,207,25,203,1,209,69,121,47,237,136,183,19,233,158,26,212,77,169,86,177,186,154,19,54,0,35,122,6,217,140,147,213,1,244,224,87,163,152,211,165,136,194,206,115,94,245,180,242,43,3,58,148,177,
17,167,23,238,235,247,159,78,94,105,250,29,181,244,126,34,211,52,219,75,91,141,122,214,59,139,215,183,136,43,79,32,80,133,155,29,113,180,15,199,222,166,183,117,150,53,146,51,185,77,115,254,2,213,98,213,
190,16,120,107,80,96,25,44,147,97,56,200,218,201,156,254,27,79,95,90,188,111,227,183,182,177,100,44,241,152,129,98,156,236,4,12,51,122,10,215,135,235,69,224,35,229,163,249,104,115,102,244,159,214,219,
239,169,119,237,43,246,169,162,228,249,104,24,145,211,191,25,252,13,93,69,220,170,195,161,25,174,83,67,186,75,152,190,226,171,221,220,179,76,10,227,32,30,49,255,0,1,11,207,185,239,93,72,185,132,201,18,
134,12,37,4,171,14,65,246,21,237,194,162,145,230,78,159,43,177,41,64,57,163,29,251,211,251,226,151,24,39,165,104,102,55,160,247,245,160,118,167,96,26,80,41,133,132,31,165,56,115,78,219,211,138,80,166,
139,128,208,7,122,92,117,38,157,129,158,180,98,144,17,129,237,214,156,61,59,122,1,79,0,122,82,128,48,40,17,249,145,224,253,82,211,194,178,190,174,144,67,115,113,115,4,177,121,97,145,86,7,108,5,125,152,
249,184,220,72,207,126,43,207,239,53,91,167,158,89,146,230,101,50,182,78,101,46,236,125,75,30,73,247,60,156,213,137,173,60,253,14,75,150,96,179,61,192,216,64,40,128,145,243,30,152,56,94,2,231,211,181,
83,187,178,179,243,22,59,115,52,147,69,28,105,146,133,75,73,159,152,145,223,112,206,0,233,199,90,252,105,55,202,163,39,123,31,164,164,183,59,63,15,248,158,251,78,180,157,101,188,23,51,136,138,15,180,57,
103,195,114,64,102,36,228,224,30,184,226,178,245,61,105,214,13,57,110,82,229,124,177,152,202,21,222,174,122,150,44,57,61,8,233,214,179,110,69,189,132,208,199,230,219,205,43,51,9,19,118,76,64,224,157,192,
253,211,208,2,121,61,71,21,143,169,95,204,90,66,132,230,66,87,129,157,188,231,220,113,248,209,239,54,147,29,218,58,57,245,5,123,56,154,11,133,16,65,152,150,25,9,243,33,80,9,12,199,186,147,128,88,231,36,
227,21,78,198,215,87,212,237,230,145,39,118,120,119,54,248,156,145,211,45,131,198,56,254,85,142,39,185,55,6,57,227,43,20,135,5,20,133,28,14,221,133,117,191,15,227,154,107,227,116,243,36,182,186,114,253,
164,219,92,48,85,151,24,27,87,63,41,99,198,59,247,28,138,210,17,214,221,197,41,104,123,159,192,253,47,84,159,192,114,90,65,108,182,143,103,123,187,23,144,176,15,152,193,24,193,192,80,165,113,145,198,14,
65,206,107,209,150,61,114,11,104,205,148,208,185,89,25,174,174,228,70,40,208,133,63,186,31,54,208,249,109,193,148,109,32,16,72,200,34,231,130,47,173,117,123,91,72,225,177,100,157,179,60,150,115,135,100,
139,111,13,32,36,252,192,241,183,169,57,207,28,215,97,60,246,214,251,228,134,238,25,150,36,30,105,105,80,50,130,120,98,9,200,95,241,175,177,167,131,161,58,112,112,169,107,43,54,155,122,254,22,244,60,42,
152,154,208,156,148,161,123,189,19,75,254,13,206,10,229,13,206,183,167,136,53,11,99,106,100,12,133,75,199,146,170,121,112,153,0,101,187,158,71,21,181,165,232,18,52,123,111,35,134,227,113,117,4,176,35,
5,178,74,142,228,231,185,56,254,125,96,129,102,68,104,210,22,142,62,64,86,4,12,245,7,28,15,165,65,115,28,9,20,241,168,218,242,40,200,82,65,36,241,198,57,236,7,167,53,213,253,151,10,105,212,132,249,175,
175,244,206,127,237,25,201,242,74,28,182,42,165,171,71,229,121,144,182,200,147,17,162,73,242,171,127,181,233,145,142,231,161,173,96,27,115,100,46,208,112,184,238,61,106,156,109,110,35,49,70,198,102,234,
113,243,108,61,57,199,242,246,53,125,51,180,100,242,6,27,235,248,87,208,225,45,107,175,235,200,240,241,141,255,0,95,152,128,115,70,61,169,224,82,226,187,14,3,61,132,175,114,178,125,151,107,163,31,159,
119,36,99,252,245,175,35,241,125,142,175,224,109,95,85,241,78,131,165,77,119,225,219,179,246,141,87,77,181,196,111,4,128,127,199,196,0,54,14,114,75,175,66,121,245,175,109,11,207,52,99,130,120,56,246,174,
60,86,18,53,226,149,237,37,170,125,159,245,186,58,240,248,153,81,149,210,186,122,53,221,31,6,124,110,213,109,53,239,137,119,58,166,159,39,157,101,61,141,167,149,49,80,26,65,229,228,179,127,181,243,115,
87,60,59,116,254,30,130,215,88,145,166,85,91,133,115,26,64,55,92,108,0,249,99,56,80,51,131,239,206,107,47,226,229,141,166,147,241,107,197,182,150,49,121,22,240,234,76,209,198,58,38,244,73,27,30,131,115,
177,30,128,227,165,117,218,246,143,172,234,254,0,180,104,188,211,97,167,193,49,141,217,118,71,12,174,84,229,219,211,191,168,31,90,252,183,25,82,113,196,202,114,126,247,51,244,189,207,209,112,145,139,195,
197,69,105,101,247,88,216,248,55,160,221,120,190,239,90,212,142,175,113,97,111,36,34,196,77,108,234,239,42,41,220,219,3,174,20,22,102,4,158,157,128,175,161,60,21,225,77,35,195,208,150,176,178,91,107,137,
212,44,178,249,134,89,46,49,211,116,141,150,57,36,214,119,194,143,15,90,120,107,192,118,54,86,112,249,144,36,175,43,202,79,205,57,108,146,220,142,135,174,43,170,155,81,143,100,140,204,209,178,17,247,227,
57,60,142,72,29,177,147,154,251,12,174,142,19,11,133,133,70,151,52,149,239,215,83,230,113,243,196,215,196,74,156,95,186,157,173,250,159,50,254,209,210,22,248,153,36,24,33,45,244,248,20,43,28,224,146,228,
254,28,143,214,188,175,0,40,0,96,118,226,189,11,227,149,218,222,124,80,214,37,82,165,99,17,66,165,79,24,84,7,249,177,175,62,110,14,77,120,120,169,169,214,148,187,182,122,152,104,56,82,140,95,68,52,159,
122,237,62,8,219,53,215,196,219,72,62,201,117,116,147,218,207,111,36,86,169,190,103,141,194,249,129,6,71,59,84,243,219,223,53,195,202,216,24,239,94,209,251,28,216,190,165,251,67,248,114,210,56,96,146,
97,111,115,44,75,63,250,183,112,128,5,60,140,19,187,175,94,14,43,146,74,238,200,233,93,207,173,252,25,224,136,101,240,101,229,244,177,94,165,204,30,76,17,195,123,110,110,35,251,62,236,201,186,37,32,57,
43,144,14,70,8,199,65,138,246,109,55,80,185,240,221,148,215,86,26,254,159,166,233,242,198,12,240,199,101,229,36,43,192,79,47,239,1,180,19,247,179,199,166,43,213,124,13,225,187,15,9,36,154,62,147,166,165,
173,128,142,55,220,46,94,98,238,70,28,124,217,56,4,113,207,122,209,213,237,172,37,129,150,235,76,134,76,119,104,151,249,211,140,4,228,120,151,196,95,139,62,32,210,109,46,238,252,25,225,248,117,187,128,
241,249,239,52,160,69,136,242,36,201,227,57,77,184,32,100,18,120,56,175,34,248,133,119,226,191,136,186,14,167,173,89,248,118,198,216,248,116,25,164,128,94,2,82,25,35,59,74,62,8,42,113,146,65,207,183,106,
245,223,17,120,31,194,50,234,17,217,235,26,214,251,219,210,211,88,45,193,9,134,140,146,9,232,14,55,116,192,224,87,139,235,222,8,213,151,85,155,194,254,11,241,71,151,225,153,39,88,117,9,228,66,38,146,243,
135,67,181,7,250,163,243,12,142,152,39,24,25,174,74,212,230,239,125,190,70,180,228,186,30,101,226,239,7,248,107,195,154,60,16,201,21,229,222,169,103,162,198,111,117,217,85,99,72,174,101,101,16,196,164,
240,101,125,231,133,24,32,103,142,51,226,54,90,141,178,48,182,213,33,114,165,206,198,114,25,152,142,71,204,56,28,246,252,15,74,237,190,60,248,171,89,185,212,226,240,213,238,162,229,180,73,60,185,237,229,
144,183,153,117,146,14,14,72,59,16,1,158,152,98,59,87,11,111,169,200,215,150,112,141,57,30,91,150,45,43,100,31,52,21,32,51,5,229,91,167,61,240,59,243,94,125,74,16,122,62,159,34,220,147,90,23,230,187,50,
91,11,155,89,32,217,31,201,36,79,11,111,80,50,56,110,220,14,153,227,140,231,53,204,235,23,177,197,28,233,101,185,22,233,65,152,6,37,84,231,37,87,253,146,121,173,221,106,225,162,130,37,113,26,60,16,40,
137,18,2,170,119,18,25,152,127,123,3,57,57,228,215,19,125,50,156,140,242,122,10,238,192,97,105,219,218,53,182,199,37,89,187,242,162,255,0,132,36,219,170,201,131,216,116,252,107,215,116,201,73,134,54,233,
211,53,226,158,16,144,46,161,49,13,253,223,235,94,201,164,56,123,68,57,225,49,248,154,241,243,137,126,253,216,245,176,75,247,106,231,234,79,236,187,171,125,171,224,55,130,25,162,124,71,166,36,69,186,228,
166,80,254,163,165,122,188,119,144,20,118,87,220,84,19,180,15,155,142,216,175,14,253,139,164,50,254,207,126,30,14,50,18,123,180,28,118,23,18,87,169,120,247,95,211,60,47,225,199,214,181,69,31,217,241,77,
18,79,33,255,0,150,74,238,19,121,246,25,201,175,63,13,82,172,33,205,205,161,189,72,197,206,201,28,237,175,197,255,0,135,210,71,107,36,254,34,183,178,91,171,179,99,24,188,62,73,251,80,112,159,103,33,186,
74,89,134,19,169,207,25,175,46,253,174,188,117,226,45,27,69,188,95,7,106,51,88,235,250,44,246,206,145,71,251,198,191,19,238,66,130,60,28,170,142,73,245,244,0,154,203,248,247,160,107,179,107,23,122,231,
192,221,63,71,155,197,195,81,183,135,196,44,12,78,119,102,39,83,36,78,54,177,17,176,98,219,149,130,237,35,60,87,141,143,143,119,22,127,23,60,87,225,175,28,155,109,103,71,154,52,182,251,45,132,176,201,
229,76,153,243,217,31,57,98,161,9,9,206,214,221,131,200,0,157,122,245,34,226,181,107,183,245,253,108,62,74,113,179,125,79,154,60,119,227,31,21,234,243,218,203,174,188,55,122,165,140,162,32,154,173,170,
78,164,148,109,176,77,25,199,152,159,51,110,221,156,156,96,241,92,109,156,15,225,232,160,213,140,114,137,150,13,242,69,36,128,6,18,130,74,198,71,30,90,179,28,3,247,80,40,230,186,173,71,71,182,214,127,
181,245,219,29,67,82,182,211,146,8,239,89,181,88,146,119,155,108,172,21,85,145,134,2,252,160,179,2,65,99,193,4,87,7,169,174,181,13,253,156,83,75,8,223,11,92,69,36,176,180,113,52,65,152,12,2,48,118,178,
183,176,60,117,174,249,115,109,37,167,158,231,156,238,153,235,254,20,212,76,255,0,17,252,59,170,107,119,137,52,23,151,241,53,235,195,7,217,231,134,53,219,153,23,120,40,23,113,221,156,124,193,10,237,228,
154,238,126,46,120,243,64,79,143,94,52,77,61,46,245,29,22,91,141,58,223,77,58,53,202,162,220,188,73,150,104,210,53,42,204,196,174,226,20,133,72,137,192,221,154,241,107,111,24,94,91,233,90,140,247,182,
215,103,72,212,224,91,203,169,172,98,130,73,238,145,24,98,85,19,2,138,20,141,167,35,37,0,3,5,129,171,30,16,214,52,191,11,93,199,226,77,15,78,184,184,211,35,183,115,102,46,84,64,246,23,147,35,44,146,191,
150,121,149,163,227,49,225,0,118,228,224,26,229,228,183,188,209,178,158,150,238,116,30,24,215,164,125,122,234,243,196,48,233,126,38,209,116,139,145,113,18,220,105,205,229,90,72,129,92,11,120,227,219,251,
230,25,229,1,61,122,215,211,233,127,164,201,241,175,71,213,108,52,171,141,57,155,85,19,73,230,220,51,110,13,44,109,157,140,62,67,243,103,0,145,205,124,59,225,139,68,212,116,11,137,223,196,51,232,218,125,
156,138,162,218,91,87,146,107,128,227,18,78,151,33,176,131,0,160,220,164,225,112,62,246,107,209,44,252,125,172,106,178,120,110,198,211,86,130,203,195,122,62,183,12,133,238,47,11,204,66,8,222,73,2,145,
204,108,51,24,35,141,252,99,140,215,160,235,114,225,234,70,93,83,50,131,106,113,111,185,251,44,173,144,120,32,131,222,188,235,246,133,177,187,212,126,19,107,48,216,88,90,223,222,33,138,72,237,174,161,
142,88,228,196,139,144,86,66,23,166,123,143,106,239,237,110,97,158,202,43,164,127,220,73,24,145,88,241,242,145,156,159,194,184,223,140,176,105,186,191,195,15,21,105,55,151,36,71,113,165,77,35,44,33,94,
95,44,46,119,170,19,134,233,223,138,170,85,18,113,109,246,55,105,159,52,120,59,194,250,164,218,71,130,86,227,193,58,93,221,147,25,62,213,27,91,21,123,53,154,221,20,249,108,95,32,121,138,225,130,238,25,
237,192,173,15,217,14,202,194,15,136,62,40,211,31,66,184,211,163,184,211,161,149,172,53,2,101,120,221,91,230,25,117,82,70,95,140,140,245,228,215,9,225,237,63,70,254,210,248,106,186,111,142,12,90,117,204,
94,87,216,174,160,184,183,251,91,44,178,198,27,228,44,129,131,58,0,9,234,160,138,234,191,101,115,171,105,255,0,22,108,236,181,125,110,45,94,228,233,79,7,218,34,212,13,216,1,73,36,111,60,245,66,112,112,
121,174,252,124,92,112,250,244,107,186,251,76,198,140,185,167,167,111,39,209,31,83,235,30,5,240,134,168,115,121,225,219,6,124,99,114,66,35,111,205,112,107,136,214,254,9,120,86,239,38,198,226,250,197,136,
232,178,9,23,242,96,79,235,94,196,8,192,165,205,112,168,198,215,177,181,217,243,14,165,240,6,253,68,173,99,175,218,202,221,81,102,183,100,221,245,32,156,126,85,225,90,148,107,164,93,93,88,201,34,37,237,
180,243,45,198,8,95,49,213,202,16,153,233,211,167,167,61,107,244,69,136,198,107,243,239,227,177,139,74,248,213,175,197,35,205,231,199,169,25,109,227,72,247,101,102,141,29,156,246,80,50,192,19,142,135,
154,245,178,90,234,141,89,235,210,255,0,115,255,0,135,56,115,10,62,214,154,208,240,239,218,55,85,22,126,26,16,145,28,66,227,11,9,18,98,70,249,185,227,186,237,7,212,115,95,44,88,221,173,157,218,201,35,
25,60,205,206,161,122,134,60,119,250,126,181,239,95,180,197,149,157,213,181,149,246,89,46,150,227,236,241,0,219,188,205,192,179,253,0,218,49,142,185,247,175,10,241,54,157,28,127,101,88,129,8,145,47,126,
119,31,241,205,112,231,85,165,87,22,212,157,213,172,118,229,212,161,12,60,82,86,108,245,143,130,122,189,151,135,190,35,104,158,32,212,52,244,184,177,134,109,234,209,79,151,73,71,221,124,156,3,142,65,13,
145,134,60,26,251,51,196,118,182,26,55,197,97,227,196,183,188,188,211,60,79,4,115,217,203,110,158,97,177,101,140,71,36,68,40,57,85,13,191,158,204,221,150,190,20,240,199,134,181,107,127,13,174,179,28,15,
117,105,2,150,186,137,84,153,17,11,16,142,19,25,35,0,147,223,30,152,53,246,71,236,191,227,11,123,15,13,13,6,63,18,104,250,157,253,162,29,79,76,150,194,118,184,141,74,228,188,101,128,31,63,151,191,41,128,
123,12,215,192,103,152,42,233,42,146,77,39,107,62,234,250,91,207,250,103,209,224,49,20,226,218,141,158,233,163,144,177,111,236,215,45,179,48,73,62,200,229,136,25,17,66,225,30,224,147,194,227,174,227,149,
228,83,53,132,158,233,46,167,103,132,217,218,74,169,108,145,174,49,156,134,109,223,197,146,0,207,25,193,61,8,174,151,198,145,216,233,30,51,241,53,166,158,129,116,244,184,99,4,48,28,64,209,62,93,0,95,249,
104,31,205,39,174,48,192,103,229,174,59,77,123,185,52,233,244,185,247,219,73,101,106,132,90,180,36,163,70,153,193,137,193,218,49,145,185,123,96,112,1,201,253,87,21,53,136,202,101,77,93,243,194,255,0,59,
95,203,175,244,207,141,194,197,210,204,99,83,69,105,91,229,123,121,244,43,0,187,84,129,158,51,138,225,244,207,17,201,63,196,155,191,14,75,11,194,233,107,246,149,7,156,128,84,17,159,248,16,53,220,219,130,
80,143,76,230,188,187,91,181,158,15,143,222,31,190,130,6,145,46,116,249,162,157,148,128,17,112,14,227,158,188,170,142,61,107,242,220,44,99,83,153,62,206,222,171,83,238,107,73,197,38,187,163,236,191,217,
10,255,0,236,254,63,212,108,119,97,111,180,230,192,245,49,184,97,250,51,87,107,251,101,141,103,69,240,46,149,226,159,14,107,26,134,159,117,101,173,217,173,208,130,229,182,73,19,51,70,1,67,149,4,59,171,
103,28,227,7,35,138,241,143,217,238,248,216,124,91,240,203,30,22,121,218,221,143,179,163,15,231,138,250,131,246,149,210,46,245,159,129,30,52,180,211,224,243,245,21,211,218,123,104,246,146,90,88,136,145,
64,0,19,156,175,97,92,112,143,61,94,87,215,245,208,231,169,238,87,79,189,142,250,193,146,235,74,183,145,100,105,18,88,20,150,114,50,224,168,228,227,3,39,53,241,23,137,173,237,160,146,223,195,165,79,157,
168,235,23,11,115,8,193,111,42,57,201,148,184,29,20,129,26,159,250,232,163,189,125,115,240,142,249,245,15,133,254,26,187,149,89,37,54,49,171,137,20,171,6,81,140,16,121,237,95,51,120,175,195,219,62,56,
124,65,212,238,162,150,34,250,140,31,101,40,228,44,145,139,88,155,113,250,60,146,12,116,239,214,190,219,195,140,92,150,46,116,47,241,197,126,23,95,169,242,252,75,65,125,91,157,253,134,100,107,203,115,
185,37,138,217,154,56,164,87,117,92,29,202,14,72,0,127,133,121,15,237,59,12,87,127,11,245,79,32,174,45,68,23,113,201,140,146,81,195,58,126,49,245,200,232,107,222,174,26,56,153,93,136,94,66,142,122,228,
255,0,142,43,207,254,45,233,54,119,222,14,215,108,238,2,155,75,235,105,35,14,137,185,225,144,169,29,51,247,15,127,79,161,227,246,92,85,53,58,115,73,238,143,134,193,205,198,172,27,93,78,67,246,75,213,69,
255,0,193,173,99,75,150,224,167,246,118,119,109,39,118,212,231,245,6,181,175,181,137,180,251,102,129,131,73,35,176,138,64,138,8,144,236,202,229,79,32,21,57,62,152,56,53,226,95,177,39,136,30,203,197,151,
218,101,210,236,134,246,16,93,120,32,51,46,24,31,92,87,173,248,141,36,180,241,45,244,32,12,36,102,72,139,14,170,220,110,83,198,72,218,227,182,115,199,90,248,252,174,188,233,83,169,24,238,154,127,248,22,
159,161,244,120,218,49,156,226,229,217,254,31,240,230,198,155,169,71,164,105,147,64,209,180,246,208,159,220,163,2,178,74,67,156,6,237,128,121,10,62,167,57,174,159,72,213,124,171,61,60,57,145,34,243,2,
70,187,0,12,49,203,123,46,79,94,255,0,141,112,122,93,157,186,233,185,213,37,242,214,24,149,214,5,156,73,35,72,48,60,201,15,0,115,198,223,199,39,138,204,213,117,101,137,174,162,55,37,167,184,56,88,161,
206,203,117,223,128,172,79,68,200,45,147,207,28,14,43,221,142,46,116,226,164,254,227,201,120,88,212,109,126,39,182,255,0,108,218,186,185,142,69,108,28,40,232,122,31,240,207,210,179,110,117,159,181,65,
19,217,220,44,127,188,72,217,241,239,151,192,239,128,63,90,243,143,14,92,155,221,54,73,26,233,26,216,186,3,44,159,242,209,89,192,82,128,242,120,234,125,14,57,53,113,46,37,188,214,100,184,142,102,144,91,
111,141,24,96,125,161,155,7,36,116,11,200,25,199,24,35,158,149,215,28,116,231,20,237,185,204,240,112,132,154,190,199,169,217,234,94,125,211,198,35,93,170,219,119,110,246,206,62,184,235,90,248,228,215,
11,225,25,196,144,27,72,166,73,158,34,168,174,139,254,183,3,230,192,236,55,103,39,211,215,183,122,160,227,230,198,236,118,233,94,133,10,142,113,187,56,42,195,146,86,16,125,105,212,226,51,129,154,49,193,
173,204,218,27,140,209,143,122,112,3,129,74,57,198,77,33,13,193,31,253,106,49,158,244,236,98,140,81,113,88,252,168,135,83,177,212,126,195,160,150,181,179,211,237,36,51,205,40,4,139,150,143,12,2,167,247,
152,133,5,123,225,152,156,215,41,172,92,51,106,119,14,98,68,107,178,50,170,216,17,46,122,130,58,99,215,211,181,100,79,125,16,185,12,173,49,134,40,210,40,145,246,228,42,142,6,70,56,4,156,103,156,117,201,
167,73,61,198,165,119,107,28,8,60,214,92,97,152,0,15,45,201,232,0,0,159,83,205,126,65,36,229,43,159,164,164,208,249,165,131,125,194,72,171,185,101,93,219,122,19,128,48,63,159,227,82,233,215,136,35,184,
69,27,68,160,124,131,28,17,206,107,55,81,186,50,143,148,168,94,23,17,242,164,14,249,60,146,79,57,166,88,171,24,100,4,128,167,4,228,117,3,167,210,147,138,107,81,180,110,71,177,229,68,105,10,255,0,22,71,
57,233,199,182,112,107,167,208,226,136,106,81,91,173,196,177,64,234,222,96,114,8,28,228,227,29,127,173,113,86,204,25,64,216,197,137,24,231,131,142,184,175,77,240,166,155,123,125,96,179,65,113,110,169,
1,218,171,36,96,188,131,130,4,99,248,201,99,129,208,147,199,122,81,165,41,205,37,175,145,156,218,74,236,250,31,83,214,124,57,225,45,18,210,215,80,130,107,153,175,33,103,158,220,218,53,251,70,216,27,163,
36,176,40,164,182,66,17,192,39,24,3,2,181,183,196,239,15,190,139,36,80,120,111,83,134,253,194,79,159,236,64,232,146,168,0,0,84,101,240,56,29,200,3,154,241,159,138,250,239,142,252,9,241,55,83,158,45,114,
109,63,84,190,178,183,186,185,107,23,27,36,38,34,128,157,234,121,196,92,145,142,245,245,247,194,205,73,53,63,12,65,226,45,83,196,90,172,23,154,94,153,28,154,148,55,83,62,235,169,188,176,205,51,114,162,
33,187,230,80,164,18,122,12,0,43,219,175,155,84,163,41,37,77,43,105,107,55,111,198,223,129,201,71,1,78,172,20,156,219,190,183,219,244,253,79,48,212,126,32,200,179,217,92,233,30,28,190,183,251,89,111,49,
53,27,51,102,101,126,54,132,220,191,49,232,14,220,246,227,52,71,226,255,0,31,219,174,166,247,62,4,138,37,82,146,42,253,165,227,242,162,232,75,171,168,37,143,85,192,25,207,182,79,15,227,31,21,235,58,255,
0,196,45,51,95,191,189,212,46,228,180,191,183,184,182,134,242,246,73,150,1,20,171,34,42,239,39,28,168,39,185,232,115,129,94,181,227,47,137,158,58,190,248,39,241,0,223,233,208,73,166,235,150,198,59,221,
100,40,107,129,52,206,171,6,100,207,203,217,85,0,28,99,2,185,103,156,227,34,221,72,187,125,223,228,205,227,151,225,101,21,9,70,255,0,121,141,109,241,54,27,72,118,201,225,109,98,35,26,226,20,62,89,219,
254,249,13,201,39,60,140,245,230,157,39,197,221,38,223,10,158,31,214,100,154,69,44,224,196,170,35,57,225,114,79,205,199,113,145,235,94,17,23,136,239,92,157,243,57,25,254,46,106,236,122,227,156,110,36,
253,69,84,120,147,29,13,167,248,71,252,133,44,147,9,61,225,248,191,243,61,145,254,53,105,131,96,255,0,132,119,86,70,37,183,25,54,5,76,116,201,25,235,219,0,251,226,178,102,248,253,101,19,4,62,13,214,91,
0,124,194,72,192,207,126,167,53,231,15,173,237,133,154,40,99,146,65,200,86,24,207,227,90,176,234,182,18,168,243,173,211,36,115,237,87,254,181,227,147,187,159,224,191,200,143,245,123,6,213,148,63,23,254,
103,88,159,180,5,163,44,153,240,165,236,79,26,110,253,237,228,67,121,254,234,142,114,125,189,171,209,60,29,227,93,87,198,55,198,207,194,126,25,147,87,186,138,51,52,209,89,222,44,141,26,103,5,136,192,224,
18,51,239,84,255,0,101,239,4,248,75,199,126,38,241,36,90,229,172,243,89,89,90,91,188,49,69,33,66,204,242,72,27,230,4,99,229,64,63,30,213,245,111,192,15,1,120,51,194,190,47,241,107,233,30,20,146,239,75,
42,45,38,150,87,138,228,194,2,44,159,51,18,75,242,228,5,25,62,190,181,19,226,204,109,210,117,45,127,36,105,14,30,193,171,190,79,197,159,150,255,0,22,163,188,159,227,111,137,198,163,99,53,141,247,219,147,
206,182,152,97,225,97,12,67,107,100,122,0,127,26,250,103,192,233,101,127,240,119,196,94,30,129,214,254,57,246,202,217,141,118,77,32,100,15,16,231,36,225,135,108,29,160,26,249,219,226,125,190,159,119,241,
255,0,196,209,193,19,218,233,243,234,91,173,226,18,238,116,70,72,252,181,45,234,65,83,207,64,113,208,10,251,147,225,221,135,131,244,127,10,248,83,195,119,22,113,234,250,141,211,69,246,132,114,4,108,222,
106,166,50,6,8,42,252,130,8,56,239,131,94,78,34,172,171,83,246,146,222,90,253,250,158,158,30,154,167,238,71,68,180,57,56,52,239,137,214,74,176,143,135,87,126,83,159,221,7,188,128,23,92,0,9,37,251,253,
113,199,231,200,120,183,196,94,48,240,238,161,37,159,137,60,55,109,97,119,115,102,30,11,89,238,163,146,73,129,112,21,147,99,145,243,49,218,87,142,51,199,21,244,95,197,118,248,67,103,124,154,55,135,124,
37,225,13,71,198,31,104,219,52,22,118,42,224,56,226,52,96,137,151,249,176,74,174,64,60,181,115,223,11,126,16,199,226,27,219,77,71,198,26,118,147,163,13,57,101,151,95,91,66,166,59,99,51,130,150,203,180,
108,19,74,165,89,194,243,26,178,168,37,159,117,101,253,175,94,116,213,62,103,101,253,118,15,170,82,83,231,75,95,235,204,252,254,241,187,220,183,139,53,102,190,182,54,183,158,126,46,33,243,196,222,92,155,
70,225,188,112,220,231,167,3,160,233,92,244,207,181,73,35,233,93,255,0,199,251,125,62,199,227,151,196,59,61,34,214,43,77,34,219,93,184,130,206,218,20,8,144,70,155,80,34,168,225,64,42,120,29,57,175,55,
206,238,189,51,138,246,169,206,244,162,251,164,121,115,86,155,245,30,112,78,72,250,87,188,126,195,112,199,119,251,75,248,98,7,89,25,124,171,134,59,24,169,85,10,160,182,71,166,224,63,26,240,57,92,171,55,
176,226,190,134,255,0,130,123,163,191,237,69,160,21,193,198,153,124,199,63,246,198,181,165,110,116,76,190,22,126,186,91,24,96,69,128,128,60,165,218,153,233,140,116,21,157,226,11,165,138,202,89,199,17,
170,22,98,62,99,129,201,192,254,149,4,215,172,177,206,236,248,184,18,124,157,184,247,168,44,207,155,118,140,20,57,110,31,120,227,242,173,185,58,152,243,105,99,200,237,60,71,15,196,137,117,27,63,15,91,
71,37,165,168,17,201,53,229,164,150,179,169,39,18,24,140,139,192,3,131,129,212,227,34,176,165,240,72,240,23,129,245,139,63,1,105,218,157,223,136,90,214,89,135,217,35,15,53,220,152,33,93,147,143,152,100,
124,192,131,192,29,43,219,23,195,250,101,175,138,111,117,139,88,210,11,219,232,196,115,73,191,130,7,160,232,51,237,214,183,124,19,11,67,241,1,58,176,54,79,134,35,182,229,174,121,197,40,185,63,136,214,
58,181,21,177,249,107,171,124,60,209,52,251,123,164,241,15,246,204,90,181,190,167,29,141,245,212,82,172,219,110,165,30,103,150,196,125,230,124,145,156,114,251,148,124,195,21,194,252,71,240,174,173,224,
171,168,167,141,140,218,84,142,241,9,196,102,57,68,128,144,233,38,15,222,7,32,247,200,193,175,185,60,55,113,227,95,10,159,17,107,22,31,13,180,89,117,11,255,0,23,199,21,197,198,149,108,138,211,218,3,44,
162,89,76,46,251,136,145,216,156,133,249,152,146,1,57,175,16,248,241,166,233,254,79,196,77,39,76,210,174,173,109,45,238,167,120,99,154,237,167,18,77,230,177,105,35,44,1,25,61,178,126,180,123,56,84,210,
90,249,253,221,8,159,238,149,226,207,153,159,87,184,188,178,88,165,11,133,0,100,19,150,3,59,65,246,25,56,174,127,81,150,33,146,216,207,81,90,81,233,58,156,80,237,123,51,31,39,151,149,7,31,247,213,84,188,
209,239,25,101,121,36,176,69,141,75,59,181,208,42,128,40,98,73,80,112,2,144,73,236,8,61,13,111,24,211,167,14,88,36,145,143,53,229,118,204,223,10,201,157,109,198,113,185,112,7,227,94,225,163,48,88,2,3,
233,94,93,225,239,11,106,48,107,169,231,73,26,183,152,32,34,56,165,127,156,206,144,5,229,7,62,107,162,31,70,96,13,122,220,26,108,182,150,254,109,209,145,33,69,66,91,202,198,119,151,11,193,110,230,41,7,
252,4,215,203,102,212,229,42,183,138,61,172,29,88,168,89,179,244,47,246,63,215,180,189,11,246,102,131,85,213,111,86,218,202,207,80,189,243,164,147,160,38,118,192,3,185,57,24,3,169,53,193,252,117,253,163,
252,39,170,222,105,94,19,240,189,204,186,180,154,130,150,184,211,45,246,71,113,125,30,93,94,32,100,32,33,194,150,4,229,91,24,56,228,142,119,225,23,136,62,30,75,251,52,92,248,67,197,242,195,168,217,73,
170,59,189,173,164,133,238,220,73,38,244,146,53,78,87,111,222,222,14,64,82,221,171,231,31,26,248,122,29,38,61,10,27,143,16,197,15,134,237,110,46,39,151,73,186,138,56,47,238,101,102,98,190,91,50,238,144,
21,198,11,16,187,51,145,156,26,241,41,198,74,60,147,219,250,254,187,121,157,156,201,190,104,179,23,195,63,21,252,69,224,191,31,95,107,186,84,210,234,22,55,86,207,99,115,13,212,142,76,200,80,136,75,21,
206,28,43,1,184,113,184,16,6,8,197,77,103,83,248,121,169,124,9,208,45,95,77,251,111,196,53,89,69,214,175,123,107,28,107,106,147,62,249,81,25,70,29,226,40,133,93,147,169,96,8,201,174,89,244,13,114,75,255,
0,248,73,37,209,8,55,118,173,116,86,4,102,111,45,228,8,187,145,70,98,25,0,111,110,48,50,56,28,90,241,165,189,134,159,107,46,157,255,0,8,124,207,226,11,135,23,87,250,220,122,172,143,17,183,13,188,70,144,
109,40,84,171,170,52,131,39,60,129,206,71,108,97,11,167,29,253,78,100,234,107,175,244,206,123,70,130,127,63,88,146,202,91,249,244,120,173,191,211,212,35,60,23,5,72,40,140,71,8,255,0,196,15,108,30,189,
181,174,245,132,131,195,246,218,28,190,68,243,67,101,29,178,93,66,236,145,203,15,153,35,144,225,248,27,149,250,39,25,200,110,69,115,90,84,186,183,134,245,109,103,76,188,134,125,22,75,237,57,146,235,78,
184,57,83,111,62,0,12,185,56,56,93,195,56,97,131,211,161,181,105,226,219,123,91,151,151,82,133,167,132,64,17,75,162,4,118,207,202,204,8,57,27,120,24,228,147,207,74,237,154,247,108,181,57,239,101,99,111,
78,210,237,175,181,11,125,186,130,46,23,100,185,102,72,238,160,80,205,243,160,33,100,17,146,205,207,57,218,7,36,10,155,196,62,29,212,52,237,110,214,214,234,43,203,77,61,151,237,48,221,219,105,206,69,204,
68,42,177,216,248,43,9,12,51,237,208,215,63,226,29,75,88,213,161,210,231,107,217,173,225,149,12,112,172,165,72,143,107,101,2,162,12,229,152,142,122,18,65,230,155,29,182,175,164,220,218,106,119,154,213,
194,27,134,242,164,212,140,179,74,214,184,32,96,127,121,64,99,242,38,87,4,140,12,243,202,147,209,182,9,166,105,248,171,80,178,187,130,61,38,219,71,181,107,125,61,141,180,26,161,182,49,93,220,194,28,166,
66,231,130,87,231,43,206,91,25,230,186,109,7,76,210,116,152,116,91,235,22,148,235,22,232,147,36,50,206,146,239,135,202,116,38,85,218,161,9,102,223,133,63,46,0,193,230,178,124,13,113,29,174,135,226,173,
86,230,234,4,142,226,218,104,237,36,123,77,198,23,102,87,222,187,207,238,241,176,182,238,112,88,224,30,149,94,223,236,58,130,201,36,215,8,151,118,240,72,204,177,111,155,12,142,197,4,74,114,11,16,87,0,
3,130,123,156,214,242,82,148,45,117,216,27,214,232,253,43,179,248,191,171,190,175,169,232,208,222,104,239,101,119,161,90,234,58,36,183,142,60,171,200,194,36,119,65,49,201,137,89,134,92,146,114,196,1,129,
92,223,129,124,7,119,47,136,116,75,205,87,90,211,181,63,16,91,192,210,95,153,117,198,157,37,186,91,119,13,107,105,110,164,23,183,2,78,86,92,13,202,175,180,154,249,143,246,105,210,45,126,42,252,99,240,
214,135,170,106,109,111,172,232,218,12,111,108,215,0,92,67,40,128,21,217,229,124,185,3,206,243,3,103,228,116,83,150,56,175,208,207,128,26,63,128,163,248,99,99,99,225,55,77,82,31,14,125,171,70,93,98,230,
36,123,169,229,137,202,92,200,100,0,18,207,34,177,99,198,227,205,112,194,148,219,73,189,191,173,14,216,212,78,55,177,225,210,237,211,244,143,7,93,107,255,0,14,236,146,226,195,80,149,81,162,182,154,211,
236,255,0,60,82,110,93,135,24,44,73,57,224,149,172,127,2,106,218,54,145,251,81,233,154,110,145,99,117,104,162,89,60,225,53,216,152,49,107,135,136,237,224,21,31,56,56,62,188,87,89,103,226,57,174,252,45,
116,52,79,23,88,188,186,125,226,60,205,168,121,150,42,136,235,229,162,228,228,18,100,3,3,56,169,252,99,20,235,241,75,73,214,117,107,123,88,210,215,83,129,45,111,17,163,45,36,114,180,108,87,114,243,130,
192,117,244,21,244,88,222,103,135,154,242,191,221,169,203,75,151,218,45,63,173,143,174,192,0,241,74,126,181,88,70,219,131,44,140,57,57,29,141,60,172,189,152,17,94,108,42,202,223,9,171,75,185,33,63,133,
124,43,251,108,216,93,105,191,19,227,215,12,96,216,95,216,219,219,73,32,115,184,22,118,78,64,236,14,211,219,191,61,43,238,73,94,84,25,242,203,103,143,151,181,124,165,251,104,203,20,215,58,76,15,111,40,
153,173,10,249,193,67,8,139,76,170,191,33,224,243,187,158,112,43,187,44,169,25,98,185,30,142,207,240,87,48,196,166,169,95,205,126,103,195,159,25,167,182,142,214,234,43,237,242,61,172,170,35,146,56,176,
190,98,28,108,192,225,126,98,195,143,238,100,154,249,215,92,212,154,89,99,137,0,99,10,41,98,163,211,144,15,189,125,77,227,189,63,195,87,55,147,233,87,255,0,108,212,172,244,235,195,44,87,63,105,43,30,25,
66,172,87,7,33,145,178,72,82,115,184,0,73,203,16,124,35,90,240,162,233,158,51,212,52,255,0,46,69,140,72,100,141,60,178,140,3,124,202,165,91,4,96,28,115,233,75,56,167,39,81,87,150,219,117,252,77,242,233,
193,67,217,173,254,71,187,248,85,197,134,172,250,108,247,146,77,110,145,197,59,43,144,62,208,174,6,229,56,228,170,112,6,222,160,146,221,235,78,232,197,225,111,28,104,30,34,209,82,41,44,100,213,160,142,
24,162,36,164,119,18,182,209,24,99,192,73,3,96,143,225,44,14,49,90,218,47,135,224,213,44,5,214,147,52,9,174,232,107,228,195,58,168,2,89,48,9,14,58,237,144,96,224,253,209,130,43,150,241,126,160,53,77,107,
194,118,247,62,29,214,165,190,187,214,237,166,242,52,253,178,102,75,111,154,88,163,36,128,179,111,9,149,112,164,128,121,43,205,122,153,158,91,78,174,81,40,215,213,37,116,215,70,151,126,154,232,251,220,
243,48,88,199,28,122,246,125,236,214,215,79,203,241,191,75,30,201,241,111,195,247,190,4,215,155,77,142,206,121,109,96,81,53,164,174,185,65,9,4,70,185,200,200,64,74,156,112,49,158,245,200,73,168,61,247,
151,98,248,102,120,195,181,203,163,148,70,201,11,181,189,113,184,99,208,243,197,123,151,196,91,223,17,248,207,224,116,158,43,213,236,35,208,172,116,187,143,40,195,120,161,175,112,126,70,70,17,182,35,253,
238,213,218,114,65,92,251,87,135,120,103,79,185,191,212,69,236,114,196,151,108,12,151,62,108,204,219,100,198,0,216,184,192,81,133,27,186,245,35,154,142,31,114,196,229,148,163,29,108,185,108,223,109,63,
171,174,246,47,51,146,163,138,148,165,235,127,235,244,242,50,44,70,75,143,78,213,82,251,69,178,187,214,244,237,86,95,55,237,86,1,196,91,92,170,252,221,114,7,95,165,94,137,90,45,98,238,7,109,206,142,202,
79,169,7,173,94,218,64,36,14,43,242,218,156,248,122,210,134,206,45,175,208,251,180,227,82,41,244,122,154,94,21,187,58,118,187,165,106,0,224,218,93,197,55,29,130,184,39,244,6,191,69,75,143,49,100,83,242,
63,32,253,107,243,95,128,172,140,220,144,64,31,81,95,161,94,0,212,98,213,252,13,160,234,34,64,86,230,198,38,110,122,54,208,15,234,13,115,167,121,104,113,227,35,240,179,160,112,133,177,184,103,189,124,
235,241,218,201,172,252,108,151,175,50,165,189,213,154,148,0,145,243,169,218,197,187,30,4,96,125,77,125,19,50,7,3,111,39,29,115,94,47,251,69,88,230,199,67,212,93,95,253,30,89,34,7,156,46,229,4,19,255,
0,124,227,241,175,165,225,106,222,199,59,162,228,183,186,251,215,249,158,22,103,14,124,21,68,187,126,90,158,3,113,117,36,177,180,174,0,140,110,56,56,203,113,198,115,211,160,230,179,158,72,47,19,49,198,
143,52,128,129,206,85,3,16,55,224,240,78,123,31,65,81,248,210,230,216,104,119,15,52,174,162,33,230,70,32,102,222,36,94,169,235,142,57,252,107,35,69,150,210,219,76,67,111,105,61,210,8,15,239,96,141,137,
119,220,24,157,205,133,201,28,254,56,237,138,253,141,215,154,175,203,41,93,91,230,124,107,165,31,99,120,43,59,159,35,252,45,51,104,31,31,230,176,33,150,72,111,238,109,192,97,130,66,187,21,200,247,80,14,
61,235,232,207,30,93,233,118,127,16,101,181,212,22,105,116,235,97,21,205,202,91,71,185,158,25,188,201,49,33,99,140,51,129,24,3,215,176,205,124,221,241,62,75,173,3,246,133,155,83,186,218,38,158,230,11,
247,40,48,169,230,46,194,163,215,10,157,123,231,160,233,94,239,241,111,196,90,121,240,92,91,47,237,227,188,212,163,129,29,154,93,197,68,91,182,16,157,190,240,201,39,39,0,123,215,206,65,42,120,249,45,163,
175,225,170,61,201,222,165,5,221,219,241,209,153,95,4,52,123,95,23,124,105,241,191,137,47,14,167,225,237,14,198,202,231,88,118,180,9,59,70,235,229,36,72,193,184,144,159,153,176,57,27,118,131,140,19,221,
248,207,193,218,119,133,245,91,109,35,83,212,146,234,222,245,99,186,139,82,146,70,197,236,120,243,68,200,191,112,7,28,109,203,28,6,206,77,121,167,139,188,113,225,167,213,180,189,67,72,213,244,173,34,218,
239,65,180,131,82,183,182,153,113,105,48,140,173,212,50,1,247,156,17,184,147,201,220,190,149,213,234,223,17,108,181,239,3,90,248,121,188,75,166,106,145,233,218,148,22,222,23,158,40,153,230,154,0,127,121,
101,48,81,129,181,68,111,27,130,78,9,227,215,90,120,154,52,165,42,141,233,61,109,218,254,95,125,195,19,74,117,121,96,213,156,85,174,186,219,204,237,35,146,222,91,84,212,111,226,82,90,63,55,202,139,37,
54,168,220,162,73,49,180,149,11,243,5,194,140,16,73,205,81,248,127,99,113,174,88,65,169,201,28,169,167,188,107,59,36,60,45,212,197,137,102,7,169,68,220,2,238,35,36,112,48,6,121,235,168,97,71,211,180,237,
71,85,15,37,244,202,210,197,20,162,40,146,35,62,65,219,198,240,223,119,191,222,60,87,121,115,119,170,233,62,14,146,249,196,2,56,226,84,182,69,98,138,238,102,216,164,145,156,242,122,100,126,149,236,225,
241,148,171,73,69,61,149,207,26,182,22,165,56,222,219,187,23,188,27,14,157,164,120,167,196,55,235,53,196,49,222,148,2,209,136,104,35,116,80,173,36,121,229,55,241,149,251,167,104,61,73,207,165,91,177,146,
37,114,133,25,134,74,158,171,237,94,69,106,52,251,85,123,123,151,123,253,65,113,184,128,11,76,85,115,150,25,227,156,125,236,0,49,198,77,117,158,9,190,105,238,54,32,105,8,139,42,195,59,2,147,156,231,185,
207,31,79,211,213,195,98,99,123,45,153,230,98,40,59,115,117,71,87,170,76,214,214,111,50,161,125,131,113,85,25,36,14,188,85,57,117,20,104,109,230,141,100,219,38,72,24,199,56,60,31,167,165,103,106,55,249,
186,251,43,76,230,102,127,151,105,3,106,96,110,56,28,131,131,142,120,207,92,87,43,117,119,52,186,150,153,229,171,45,173,171,220,71,10,67,137,11,50,109,12,195,29,113,184,129,159,226,36,158,149,215,82,181,
182,48,133,27,173,78,249,110,154,52,134,21,253,236,173,143,154,70,219,212,142,221,123,254,149,126,229,228,72,243,28,123,254,111,152,122,15,95,122,224,45,166,100,188,55,55,175,178,59,4,243,124,144,224,
147,51,128,17,1,60,18,0,36,246,220,195,147,131,93,135,135,1,109,53,102,121,119,203,56,18,57,29,50,125,62,189,127,30,56,197,56,85,230,118,34,116,249,85,198,69,169,224,131,112,168,160,131,194,156,145,129,
150,250,227,35,211,173,107,66,234,235,185,24,48,245,30,245,196,220,92,70,60,77,168,219,201,28,102,95,221,172,36,46,124,214,217,150,223,142,0,35,130,79,167,210,174,233,26,227,166,225,119,8,69,144,230,48,
88,7,114,112,2,129,221,129,200,63,134,58,26,205,98,18,147,140,153,114,195,201,197,52,143,199,207,47,115,201,181,134,124,210,170,65,220,15,94,141,220,99,191,122,236,116,79,16,53,190,139,103,162,255,0,100,
216,189,189,173,219,220,220,73,115,150,73,93,145,145,60,197,60,48,93,220,12,243,181,65,199,38,185,43,70,158,40,156,199,34,16,88,100,21,220,15,78,191,225,245,174,231,193,126,25,178,215,173,117,11,235,189,
74,206,202,207,77,69,154,121,174,7,18,202,249,85,136,12,140,14,50,48,27,113,224,119,175,204,33,205,118,145,247,238,219,190,135,31,45,131,139,51,123,16,145,172,26,113,28,109,39,223,147,42,88,18,62,128,
158,188,125,57,170,207,42,152,200,200,1,70,112,127,138,180,188,71,52,67,81,16,65,36,237,109,16,108,187,174,198,144,228,225,182,231,140,140,12,16,8,25,6,169,105,168,240,204,147,150,41,60,101,94,50,7,66,
14,65,231,211,142,181,46,201,143,165,217,208,233,26,77,212,209,36,192,38,246,71,145,21,79,206,219,112,79,202,123,99,39,35,61,58,87,178,126,204,26,94,135,55,143,115,170,133,93,70,32,103,211,101,89,14,215,
60,169,140,174,48,120,96,203,223,37,171,140,180,210,237,111,180,88,225,105,4,122,174,163,116,140,141,24,5,118,156,35,33,76,131,230,19,200,99,129,243,96,114,115,94,219,240,127,225,158,177,160,124,72,186,
187,214,146,88,35,178,136,73,13,212,79,181,101,12,17,149,76,103,56,110,24,17,146,70,48,13,122,217,94,30,107,23,78,124,183,87,213,244,90,126,22,220,243,113,149,87,178,154,189,157,190,103,15,251,108,90,
24,126,38,216,78,178,153,150,243,69,17,2,120,0,163,200,128,143,111,222,103,240,175,78,248,199,227,91,111,21,106,54,73,165,233,214,246,58,61,141,172,112,91,172,113,108,105,182,162,130,239,239,199,3,183,
212,241,206,254,218,214,130,125,111,192,55,81,66,21,36,50,219,51,100,229,143,155,11,129,244,192,106,231,221,73,80,167,25,232,43,12,214,148,99,140,169,228,255,0,63,248,115,92,186,163,150,18,23,254,172,
107,120,7,195,119,62,39,215,239,109,225,158,27,104,52,237,58,125,66,230,234,229,246,67,18,71,180,0,206,120,82,204,193,70,122,243,215,6,151,87,190,157,62,27,248,147,73,71,101,180,187,150,202,87,78,70,36,
73,212,238,199,98,84,109,61,241,197,123,207,236,213,224,31,181,232,235,175,90,120,98,127,18,107,183,44,12,11,117,35,67,165,88,108,112,200,243,18,8,158,80,192,58,128,172,16,129,208,242,119,191,105,239,
132,215,182,223,15,101,212,244,93,30,227,90,241,109,238,178,173,170,205,100,85,34,8,32,144,236,138,38,112,2,121,155,62,98,75,22,60,156,116,241,42,87,247,157,62,231,171,26,77,37,62,199,231,148,147,94,36,
81,152,245,21,86,121,176,11,73,142,49,208,100,122,145,93,46,181,121,121,99,127,4,22,246,208,202,172,160,182,246,0,130,78,61,69,79,125,224,175,21,90,222,233,86,183,158,10,213,210,83,113,150,30,68,159,187,
1,212,110,59,50,49,195,30,123,41,206,48,113,7,138,244,185,111,124,121,21,172,122,70,169,113,60,66,38,45,109,110,242,42,128,249,57,1,14,49,198,78,123,246,173,189,154,118,185,42,111,161,62,141,127,119,119,
170,189,172,186,108,113,219,141,223,189,15,158,135,3,143,122,166,47,117,113,246,178,45,162,59,38,219,30,80,125,222,121,224,251,10,103,195,104,109,159,196,250,132,145,65,121,28,137,27,177,243,227,85,7,
116,156,227,0,30,163,242,172,205,36,88,75,106,207,27,92,145,45,241,7,62,89,32,237,207,108,113,243,125,107,57,82,141,246,41,84,125,207,190,255,0,96,105,160,179,211,60,113,44,105,3,235,119,34,214,222,216,
92,72,35,134,36,196,133,158,71,63,117,73,125,184,28,156,96,87,211,95,24,126,35,91,120,99,225,219,105,250,4,218,125,238,162,214,137,186,238,218,53,88,21,156,21,59,2,157,165,216,110,194,238,200,3,36,224,
12,252,77,240,155,225,46,145,226,47,236,185,252,83,226,15,236,255,0,14,222,198,146,155,116,143,124,183,140,132,101,19,57,81,135,32,100,130,71,81,130,65,175,116,211,188,15,30,135,227,45,59,194,48,88,44,
26,14,157,115,37,205,172,122,132,173,36,108,8,83,18,54,115,189,215,115,151,24,28,32,228,129,207,151,139,140,116,179,212,237,162,238,253,229,161,240,39,198,24,173,44,254,59,120,170,223,76,156,61,180,55,
177,164,82,172,194,92,1,111,6,78,225,212,231,63,76,99,181,122,159,194,203,251,253,79,226,39,133,60,49,115,126,240,105,250,150,176,150,198,113,30,231,141,221,112,90,53,207,204,118,168,235,198,50,123,98,
188,147,227,37,238,153,174,124,105,241,214,163,225,216,196,122,109,222,173,63,217,134,239,225,64,177,147,244,45,27,17,236,71,78,149,189,251,59,107,118,94,28,248,165,224,175,19,234,49,207,112,186,101,251,
93,44,72,227,204,37,98,145,84,29,221,178,227,159,106,246,98,151,176,73,174,135,21,255,0,121,229,115,244,127,226,178,232,159,9,44,31,65,248,119,225,134,186,241,222,181,102,196,234,247,76,38,185,144,114,
57,115,243,20,80,11,50,169,85,81,142,153,175,43,211,126,62,248,99,225,190,134,52,93,79,68,212,99,184,208,136,135,73,209,161,76,220,234,23,175,243,77,115,114,202,66,7,46,73,220,220,46,236,140,156,99,141,
181,241,103,139,245,77,90,251,226,70,171,61,213,142,147,167,233,243,139,205,76,218,111,138,218,0,219,194,66,167,57,114,66,140,157,197,155,7,7,0,87,203,54,113,220,107,62,35,191,241,78,164,39,23,55,178,
51,91,199,113,41,150,72,35,39,32,51,30,89,206,114,205,220,147,237,94,101,10,9,171,51,166,164,249,22,135,45,226,157,70,109,99,197,122,230,169,44,98,57,47,245,43,155,201,17,78,66,60,211,188,133,65,238,1,
124,103,190,51,88,211,56,70,3,63,151,106,154,238,85,55,151,100,30,60,231,255,0,208,141,103,220,16,91,35,144,195,53,244,241,74,49,72,241,36,239,38,201,93,184,45,216,154,250,107,254,9,214,172,223,180,245,
129,64,140,201,161,223,21,4,227,39,116,3,250,215,203,242,62,34,140,119,199,53,244,215,252,19,148,72,255,0,180,181,185,140,133,97,161,222,0,220,101,73,120,48,64,239,210,181,162,175,81,17,61,32,207,209,
159,14,106,183,254,35,210,45,117,123,123,155,59,117,156,186,73,24,132,187,67,42,57,142,72,201,221,130,86,64,71,224,107,165,142,198,244,200,178,141,106,85,221,142,35,183,140,1,156,247,199,251,39,243,246,
175,18,248,19,169,91,233,243,234,30,26,153,191,211,47,47,181,13,81,10,161,10,217,188,65,54,7,69,203,186,182,63,219,53,238,150,110,54,166,8,193,11,130,15,253,116,173,39,43,171,163,157,104,218,41,79,97,
49,143,204,150,254,254,95,148,145,251,192,160,224,3,252,32,127,123,255,0,29,30,245,185,240,227,77,128,120,185,158,89,46,166,41,111,42,161,154,225,219,248,213,122,19,232,1,250,147,89,173,51,11,61,199,44,
190,73,200,7,253,133,174,163,192,100,127,194,88,65,60,155,121,191,244,106,214,53,159,184,205,105,124,104,249,42,230,207,192,240,77,168,4,58,222,156,239,172,199,51,46,232,229,13,41,180,184,147,177,70,199,
202,91,30,160,118,170,95,23,116,205,46,13,123,197,107,27,71,119,26,221,78,72,120,25,68,56,187,181,62,88,13,144,64,18,56,200,224,239,62,245,216,234,23,190,34,147,84,150,218,15,24,104,122,132,47,168,34,
152,47,39,143,118,223,178,93,252,129,102,143,150,46,170,113,159,186,140,115,128,65,230,190,54,61,194,120,147,199,159,106,183,130,18,134,229,163,242,21,64,145,115,100,225,206,9,203,28,28,158,15,181,102,
186,122,127,144,84,90,61,58,255,0,153,243,111,134,173,237,127,225,54,111,42,8,82,4,158,218,20,218,128,1,178,251,90,128,142,157,178,163,240,21,198,59,1,240,142,250,73,63,214,106,58,49,113,234,73,240,172,
109,252,224,53,218,105,199,236,215,178,74,71,45,226,36,132,115,247,115,226,107,200,207,233,53,114,150,10,36,240,135,132,180,233,49,189,180,203,112,216,239,187,195,215,240,255,0,237,42,77,175,145,73,63,
235,200,233,245,200,213,188,119,12,78,217,75,123,155,185,85,179,220,106,154,61,209,207,253,245,91,250,146,77,53,150,160,150,214,176,92,202,13,180,9,28,242,249,105,145,169,106,49,28,182,14,6,27,249,87,
27,127,44,203,226,47,20,95,183,205,20,54,218,139,142,167,4,216,232,147,12,127,223,12,106,159,142,245,226,154,166,175,164,132,91,139,3,121,114,187,64,44,25,69,228,179,70,112,57,60,200,72,237,94,62,61,181,
52,118,82,247,98,122,7,236,237,225,29,67,198,255,0,3,182,232,250,26,219,107,254,30,212,224,185,51,53,212,191,108,98,230,32,35,91,140,109,42,224,62,80,228,160,32,14,49,158,67,246,129,139,196,30,62,248,
175,123,171,216,218,164,182,186,69,177,178,184,190,180,182,153,237,236,97,180,243,62,86,145,240,101,219,146,9,3,39,147,128,1,199,166,254,205,158,56,248,113,224,15,130,62,44,251,69,220,159,240,146,234,
55,182,183,118,218,92,193,229,140,76,201,28,106,128,31,148,180,108,140,199,28,129,134,35,24,173,143,20,232,63,179,220,158,46,213,34,131,82,155,78,209,160,184,243,146,241,188,83,50,219,95,203,48,119,121,
12,42,72,42,11,16,234,192,49,195,113,180,243,225,206,171,133,87,43,62,189,60,151,153,232,67,146,164,18,186,232,124,217,30,165,169,93,71,13,244,126,32,213,39,179,179,70,181,51,70,199,17,198,101,80,169,
135,56,10,92,112,184,192,10,115,90,254,22,211,244,91,253,27,193,211,235,30,34,111,42,9,229,180,185,85,101,101,211,163,40,210,41,134,56,118,200,192,182,0,102,39,230,36,112,48,181,173,227,4,240,230,131,
168,75,39,195,61,126,251,80,142,226,20,150,226,45,18,36,183,211,97,120,89,29,132,113,182,233,21,81,216,184,222,196,114,253,120,175,30,241,21,230,166,154,164,159,104,208,222,43,59,25,203,69,111,109,43,
22,98,209,169,14,152,1,217,24,177,36,129,141,231,0,87,77,52,234,124,58,126,102,21,34,224,244,212,147,94,180,147,78,183,147,82,209,99,186,209,101,242,221,46,5,237,240,154,123,173,237,135,12,78,226,160,
166,3,96,156,156,158,50,13,100,89,217,220,221,90,67,38,160,124,148,185,243,163,16,8,31,48,164,107,188,75,131,247,243,147,180,47,62,213,50,181,242,234,112,218,67,26,92,155,136,163,184,16,79,144,210,169,
94,8,45,207,36,252,170,51,184,131,138,234,252,123,124,214,82,105,246,23,246,150,150,247,90,77,168,180,87,72,76,50,51,108,201,119,3,25,56,56,46,114,91,210,186,212,218,92,172,231,94,104,206,248,113,224,
255,0,16,248,203,196,86,250,118,153,21,177,184,184,147,116,43,52,219,25,130,13,165,132,152,202,128,121,57,232,1,61,171,87,198,190,3,189,240,150,171,253,143,226,77,93,94,15,179,53,197,215,216,53,117,158,
37,102,108,36,17,128,9,37,155,0,132,77,199,112,192,207,204,57,88,181,141,70,217,163,212,52,201,238,5,223,148,118,255,0,164,48,24,32,243,142,55,19,200,166,75,163,217,218,73,53,221,246,161,109,5,243,78,
37,137,162,132,202,216,101,7,106,156,141,135,157,187,177,237,232,107,27,203,159,87,167,97,169,36,182,61,82,13,42,13,19,90,214,108,12,133,98,178,211,111,173,213,67,238,146,99,44,33,210,212,2,72,70,57,85,
50,100,99,110,114,55,113,200,233,126,36,179,211,217,163,254,206,142,226,59,152,76,82,71,28,155,91,113,56,47,230,47,43,140,49,31,222,39,154,229,101,154,214,107,4,158,225,163,58,124,82,144,210,179,136,209,
10,168,194,140,119,245,61,141,111,120,117,163,142,222,214,75,155,13,62,69,180,136,192,145,189,154,140,48,3,107,203,32,97,231,74,73,25,45,219,3,234,224,220,98,174,69,185,181,61,75,225,182,171,162,120,51,
225,54,191,115,97,172,234,233,241,50,254,229,180,237,62,206,216,22,136,89,72,20,73,190,98,0,230,44,229,129,202,176,1,50,217,207,209,63,177,111,137,252,117,168,248,100,248,63,76,183,93,34,222,250,111,48,
220,90,198,88,218,64,88,131,34,68,249,206,225,143,222,51,127,9,192,61,252,135,192,94,14,240,46,155,224,101,214,62,38,235,62,36,178,190,130,245,217,180,189,54,220,157,172,146,34,153,131,140,135,32,30,35,
66,72,12,65,27,176,7,218,30,14,248,219,240,139,194,95,13,116,107,77,39,91,188,188,146,75,117,254,207,183,186,111,54,242,250,86,124,121,101,198,65,96,205,130,79,0,2,122,3,94,117,89,182,218,74,218,234,206,
234,106,214,111,238,255,0,51,201,44,102,181,211,147,226,62,151,170,248,29,173,146,194,198,73,89,163,146,230,219,237,171,111,114,165,92,22,220,1,192,223,149,206,115,200,197,117,95,16,159,69,120,155,82,
178,23,241,223,62,147,167,223,42,179,198,241,21,141,241,247,176,27,32,47,92,99,167,74,197,240,134,179,168,106,255,0,25,190,33,73,225,239,25,91,207,97,39,246,132,54,54,243,106,114,4,18,180,59,196,145,43,
113,177,27,114,151,24,193,7,140,87,117,170,91,248,151,85,248,101,164,203,168,69,105,124,242,120,122,104,239,46,225,16,206,124,212,108,229,101,78,196,100,241,199,176,175,174,140,149,74,26,61,215,159,85,
255,0,0,226,218,126,143,203,185,245,133,140,194,107,72,103,82,10,202,129,199,208,140,213,156,241,92,207,195,235,177,121,224,143,15,220,131,159,55,79,133,179,235,242,12,215,70,72,10,73,56,21,226,80,168,
156,19,242,58,167,22,157,133,39,36,138,249,83,246,236,68,182,240,254,145,168,60,72,114,124,191,49,192,33,49,52,109,208,245,56,206,7,115,197,125,60,179,133,153,131,48,238,62,184,170,62,37,211,244,237,91,
76,146,211,82,179,134,234,209,199,205,28,168,24,31,194,157,60,193,97,228,235,69,93,171,233,234,172,41,80,83,247,37,179,63,53,62,213,5,151,135,12,54,90,100,147,191,218,90,86,182,88,195,45,214,15,207,153,
25,178,9,39,111,35,24,95,113,94,97,166,105,58,159,136,126,49,67,166,232,218,45,238,179,54,158,145,203,125,111,230,44,41,101,110,178,29,203,44,199,2,36,25,96,9,37,134,8,80,118,237,31,126,252,73,248,69,
96,218,61,197,199,135,47,62,192,109,162,44,176,78,217,137,21,65,99,180,227,32,123,28,142,7,74,252,240,240,239,197,9,252,57,225,136,52,189,32,172,242,205,42,95,235,23,146,227,207,212,249,45,28,110,65,225,
50,119,17,142,78,0,192,24,59,99,243,120,230,24,37,12,51,124,219,59,233,111,187,240,252,69,133,194,172,61,87,58,139,78,158,103,218,109,240,183,251,63,93,77,74,127,18,105,214,218,87,149,36,215,239,26,21,
150,59,37,37,160,118,39,229,44,184,97,146,62,109,204,64,29,43,182,211,126,29,248,34,210,15,10,235,13,20,79,171,222,107,209,234,54,55,50,64,137,59,178,192,200,55,250,159,39,229,39,174,49,232,43,243,71,
90,248,191,172,221,232,58,242,234,158,36,158,91,223,16,93,71,113,123,186,66,64,8,217,88,149,122,44,96,42,46,59,133,0,228,146,79,115,227,111,218,62,198,233,126,25,193,165,95,220,93,219,248,101,230,184,
152,189,156,136,219,155,203,10,159,56,93,248,2,78,65,192,200,231,215,207,197,213,205,241,52,125,141,90,142,81,215,68,172,175,110,189,95,93,205,232,211,193,211,169,237,33,27,62,255,0,215,200,253,75,241,
6,143,13,229,166,173,163,79,102,162,199,196,17,31,50,96,129,128,156,40,195,178,158,248,85,57,231,37,107,225,143,19,253,171,64,159,91,184,191,41,33,211,167,184,6,27,85,242,190,117,151,1,75,3,198,73,199,
7,44,79,67,94,131,225,191,219,99,225,175,137,173,35,182,213,109,181,95,14,106,145,108,120,231,212,45,212,218,22,86,95,249,107,27,54,208,114,71,204,6,6,73,192,167,126,208,103,71,241,14,169,115,173,232,
87,145,73,164,220,249,87,115,37,179,12,52,199,229,114,252,117,225,24,16,107,210,225,252,93,76,2,175,71,149,191,117,202,42,214,187,73,183,229,110,182,57,113,248,120,98,125,155,110,218,164,223,147,105,127,
145,243,206,150,215,194,101,184,185,45,53,211,101,229,201,37,153,143,92,103,158,253,79,97,93,88,142,98,20,228,42,30,160,117,171,54,112,196,145,129,18,5,30,213,99,0,175,78,107,243,218,213,231,82,164,167,
61,219,187,245,103,217,66,17,140,84,99,178,40,197,14,204,241,245,175,179,191,102,219,145,117,240,143,75,78,11,90,203,52,30,224,9,14,63,66,43,227,188,254,94,149,244,207,236,147,168,187,248,123,95,211,114,
8,182,188,89,148,31,71,64,63,154,26,194,82,87,77,152,98,163,122,122,116,61,232,141,220,14,8,174,99,226,128,131,254,21,207,136,222,238,72,227,130,27,41,37,105,101,108,44,123,70,237,196,246,198,43,141,253,
161,126,50,248,63,224,255,0,134,33,212,188,77,112,242,234,23,133,151,78,210,237,89,77,213,235,15,189,180,18,2,162,231,230,118,33,71,28,242,43,242,95,246,128,248,241,227,223,138,250,173,231,246,222,169,
45,175,134,76,142,182,218,45,163,152,237,210,34,199,111,154,185,253,243,237,198,89,242,185,31,42,175,83,238,229,56,10,245,170,198,180,61,213,22,157,253,59,35,199,173,56,198,54,151,83,213,124,127,241,83,
194,218,94,161,113,119,53,253,198,165,174,12,54,159,105,100,164,199,109,27,16,67,73,47,250,179,35,39,205,142,170,173,183,146,73,62,117,175,254,209,222,51,187,67,14,149,97,166,105,176,140,227,124,109,119,
41,99,198,224,196,168,92,100,224,109,32,87,129,58,237,64,195,32,177,39,129,220,156,147,80,229,203,130,27,229,199,110,181,250,91,205,113,14,238,15,150,251,219,119,243,62,127,234,20,116,230,87,183,115,111,
88,215,181,77,103,80,23,186,197,236,146,220,128,71,156,192,111,57,57,201,32,14,255,0,149,83,179,134,218,73,222,230,73,163,70,220,185,98,62,103,25,0,244,244,21,82,32,112,217,103,249,135,233,91,58,44,139,
103,121,28,174,143,60,56,195,169,81,146,58,124,188,30,107,134,85,39,38,219,119,103,90,73,104,142,143,86,184,222,182,54,90,92,22,247,90,114,13,222,89,136,147,38,236,112,205,156,169,255,0,107,229,231,154,
187,167,105,159,217,222,26,187,186,187,138,91,223,236,59,168,110,161,211,87,83,146,50,194,103,104,166,140,180,95,58,58,101,55,56,99,198,220,100,22,7,169,135,84,91,127,13,67,113,121,161,91,203,105,54,152,
145,92,195,114,60,176,88,242,101,76,117,207,24,7,167,60,214,223,192,111,14,106,254,40,241,110,191,61,181,139,203,99,117,2,173,194,193,33,155,251,61,62,226,3,187,110,89,203,16,172,64,192,13,150,199,78,
94,125,46,202,169,20,154,61,43,225,75,105,254,39,240,215,246,109,196,86,122,158,139,27,69,45,180,83,92,163,71,229,200,24,60,36,17,185,138,148,101,117,112,184,192,108,242,107,174,209,188,1,166,193,121,
37,174,153,62,167,163,205,26,237,18,233,247,134,37,134,86,12,209,188,145,171,73,11,171,151,63,49,70,27,129,61,107,198,126,14,94,71,225,143,18,76,154,69,152,134,230,215,196,210,89,25,143,157,43,71,11,90,
43,48,18,13,219,240,232,196,23,0,141,231,12,126,237,125,59,101,113,170,205,166,143,182,106,26,122,253,150,251,122,58,216,140,92,174,65,0,129,254,173,247,29,189,88,28,100,125,238,57,37,55,78,86,82,183,
99,106,77,53,201,37,115,136,180,183,157,244,201,109,236,46,32,251,67,150,107,169,237,237,252,146,168,27,230,220,155,142,246,221,242,229,14,15,28,46,114,58,120,175,35,210,180,135,151,237,82,51,189,174,
35,109,184,145,130,133,27,202,168,227,208,118,233,90,154,222,136,151,246,19,223,164,112,105,55,1,214,121,33,88,71,147,132,147,229,229,185,70,25,32,246,35,31,74,224,174,165,139,237,13,165,5,146,242,234,
220,173,176,10,197,188,199,32,149,45,192,0,8,247,182,0,57,102,83,207,21,246,25,14,50,117,41,75,157,221,199,245,60,12,227,11,8,85,139,142,204,234,52,221,106,56,175,229,212,46,11,71,113,58,36,123,151,110,
119,6,229,114,122,130,216,25,238,6,106,190,131,122,182,222,15,143,85,184,114,178,110,50,92,92,33,249,35,73,25,131,44,109,221,75,62,115,128,49,234,107,154,49,23,241,23,218,109,241,28,81,90,73,105,167,197,
9,46,4,128,129,60,236,87,128,168,165,148,124,184,86,83,130,119,0,55,53,69,176,189,185,210,60,47,167,136,255,0,179,34,151,237,183,49,77,8,41,10,219,148,242,144,238,35,230,45,229,144,15,100,207,122,250,
42,85,103,246,191,167,208,241,42,83,133,244,216,210,177,181,191,150,35,168,203,102,144,139,137,29,68,102,61,229,163,85,11,24,199,95,186,1,0,144,7,36,224,156,14,154,199,203,182,91,75,104,228,148,177,67,
24,115,131,230,12,28,169,111,239,103,244,250,84,26,164,222,101,188,82,90,25,89,166,69,146,91,185,93,184,80,122,42,175,62,248,24,224,28,154,213,158,198,199,84,129,4,178,56,153,20,226,117,249,28,48,57,13,
199,96,122,103,183,227,93,81,166,238,236,245,57,165,82,233,115,45,15,58,91,59,235,157,99,86,84,14,33,91,136,162,11,230,48,114,230,53,59,65,234,0,12,164,183,35,143,194,182,30,25,108,181,4,179,142,56,174,
175,161,6,88,89,144,2,132,12,48,86,192,4,143,82,59,159,92,85,23,150,93,51,198,151,246,83,106,40,102,188,145,64,148,196,89,162,27,23,104,93,160,13,206,168,220,227,229,217,223,34,159,62,152,250,145,186,
184,134,57,99,75,95,49,145,183,182,94,48,160,109,92,227,110,70,252,158,57,198,123,215,11,140,111,45,61,235,157,156,206,234,239,75,35,242,226,212,184,189,242,96,99,135,36,146,61,49,147,131,94,177,240,167,
68,240,251,89,107,250,206,173,168,64,151,26,92,43,36,30,99,46,89,217,73,85,1,186,110,56,0,130,50,120,199,2,188,215,195,55,209,88,234,182,247,247,22,118,119,118,246,104,241,201,2,79,229,53,208,96,121,222,
6,78,210,84,231,142,128,123,211,117,157,102,125,74,194,206,208,217,90,91,199,106,100,43,228,69,130,204,252,57,250,28,242,181,240,176,229,138,187,213,159,90,211,216,177,227,109,66,61,115,198,151,154,170,
218,203,14,159,125,48,154,222,18,87,34,30,0,0,12,227,238,158,188,250,228,228,214,84,210,134,184,103,5,137,47,144,73,232,185,224,126,92,84,87,179,249,247,42,83,11,182,36,137,1,60,237,81,128,79,191,90,114,
49,11,18,249,76,81,206,84,236,63,54,14,14,15,124,30,56,172,217,76,246,95,129,126,28,180,241,31,138,108,141,253,204,182,48,174,219,164,48,112,210,42,144,66,131,252,32,158,7,57,25,226,190,241,210,85,204,
82,92,72,113,18,227,126,71,204,196,231,31,78,58,159,111,122,248,187,246,106,240,244,122,247,141,37,184,146,107,147,99,165,162,92,178,68,252,44,185,249,99,108,131,145,156,146,6,8,192,245,21,247,103,195,
143,14,222,248,150,250,230,11,232,167,211,52,53,143,116,115,167,50,62,73,221,212,97,112,163,175,108,231,57,224,122,249,118,105,71,3,70,82,158,237,236,183,249,246,60,252,78,6,120,169,37,30,157,246,60,79,
227,215,131,53,143,25,197,224,235,95,14,218,37,236,182,26,155,77,117,151,8,33,67,27,40,103,99,192,5,246,143,92,158,156,26,236,62,25,252,19,210,191,225,36,73,181,205,84,106,62,30,176,124,106,23,176,97,
32,150,97,255,0,44,33,201,37,213,79,14,252,3,208,119,174,183,90,176,240,147,104,87,43,113,168,220,67,224,75,105,73,183,186,185,155,18,234,101,79,17,91,40,234,155,185,105,155,36,244,94,164,214,253,214,
167,174,248,183,193,241,105,127,10,60,31,62,159,162,182,35,147,89,190,2,56,151,182,33,79,188,255,0,144,21,225,230,153,149,92,101,89,87,74,215,254,181,127,161,232,96,176,112,195,82,84,214,182,61,31,198,
62,50,240,239,130,124,20,98,210,231,180,210,44,84,8,145,238,91,10,160,15,249,103,16,249,220,246,0,1,147,95,49,252,97,241,230,167,117,225,173,30,107,109,51,196,86,134,240,72,109,238,117,136,132,73,120,
155,224,37,224,132,115,26,134,0,101,254,99,244,32,215,175,248,23,193,62,26,240,227,106,183,254,36,149,245,111,19,90,237,73,175,239,2,200,33,99,252,43,187,229,78,220,12,177,197,84,248,157,225,223,20,248,
135,79,121,174,208,44,183,44,173,101,123,115,3,238,181,138,55,87,31,40,96,0,102,80,113,130,72,28,227,183,151,78,84,233,75,154,79,230,118,212,230,146,62,51,150,31,21,235,26,230,132,215,23,95,102,242,204,
79,58,199,46,30,112,234,224,200,113,206,24,93,182,71,76,215,177,248,123,75,147,194,250,229,213,238,157,40,69,102,204,206,110,14,233,82,31,237,102,76,129,193,225,34,207,175,30,130,188,199,197,211,222,248,
3,86,75,123,41,190,221,119,44,30,83,200,150,172,190,90,239,141,151,150,200,57,49,41,3,208,31,90,210,208,62,48,120,106,45,53,173,117,95,7,107,243,93,139,89,22,75,139,59,235,127,223,74,208,201,25,98,146,
1,183,38,87,99,140,224,227,0,243,94,205,58,212,234,195,153,59,163,137,232,207,62,210,124,49,175,232,122,252,154,54,171,115,28,239,2,45,168,44,229,139,75,229,162,143,82,50,223,94,181,39,252,43,159,26,233,
71,72,177,189,137,103,187,184,136,204,76,106,252,162,236,82,223,52,106,113,149,147,144,48,114,57,231,141,171,239,136,26,77,223,196,56,124,65,38,141,116,182,7,87,75,233,44,230,112,100,40,37,89,60,166,43,
149,232,10,228,118,53,237,30,29,248,247,224,221,79,80,212,53,127,16,220,94,218,248,142,242,224,48,145,52,187,153,146,202,221,75,236,130,50,177,149,42,161,201,24,234,221,121,226,163,17,57,171,114,107,220,
186,74,18,191,51,177,244,151,192,31,4,232,154,127,194,111,13,205,115,98,235,175,68,141,126,210,50,25,37,182,145,157,128,79,41,254,82,78,2,108,192,200,231,130,1,173,143,27,105,119,186,46,143,121,168,120,
146,101,179,142,214,70,212,224,212,108,166,88,198,156,66,49,117,216,252,227,147,128,75,238,201,235,210,180,116,191,138,158,3,240,255,0,128,46,190,33,234,250,216,107,77,70,82,108,131,15,54,226,227,130,
86,24,151,0,179,158,73,192,227,7,36,1,94,13,225,120,188,91,251,81,235,230,247,197,214,55,58,15,194,13,45,139,197,99,9,97,253,167,48,57,92,200,191,51,178,224,49,145,70,212,36,42,18,114,195,194,173,9,78,
77,203,69,215,254,7,153,222,166,150,136,252,218,240,233,103,140,78,237,229,77,41,46,229,151,0,23,201,111,167,36,241,248,87,184,252,7,240,109,175,136,190,44,120,38,198,238,57,23,195,215,250,161,130,230,
250,92,109,68,75,89,101,109,199,35,3,247,99,175,28,215,142,170,218,105,254,37,213,162,17,139,136,109,175,110,161,128,30,85,130,204,232,159,160,28,214,166,156,183,55,17,73,102,183,119,48,199,114,64,113,
12,205,25,0,14,79,4,30,70,87,232,121,226,190,133,251,212,244,60,244,237,43,158,227,241,223,226,141,207,142,124,93,38,151,225,91,155,139,15,134,218,10,155,27,21,181,184,120,191,182,7,1,166,152,41,1,227,
56,194,169,200,43,146,115,184,99,132,13,181,6,70,49,92,151,137,117,72,116,45,54,11,43,40,163,89,138,143,45,0,249,99,81,223,31,200,82,120,55,94,155,83,142,230,11,178,191,104,137,119,171,40,198,229,250,
122,143,235,88,83,130,130,229,69,78,82,147,187,56,123,166,9,113,112,3,6,6,71,228,116,60,154,134,67,194,12,243,142,104,184,216,68,111,30,70,248,193,108,182,126,124,157,223,65,211,2,171,51,115,94,162,126,
233,195,109,71,179,102,190,164,255,0,130,117,234,186,110,149,241,210,246,125,82,246,43,72,95,73,109,146,72,192,1,181,203,62,73,232,49,138,249,81,159,13,182,189,63,224,68,183,208,107,122,237,205,139,170,
201,22,155,229,157,192,18,254,108,129,118,40,35,169,10,220,246,3,60,212,78,183,178,139,146,232,84,105,243,59,31,109,234,62,58,210,188,57,121,109,170,219,94,196,247,22,183,243,60,138,3,55,153,3,221,194,
242,5,219,146,196,196,174,64,3,150,11,235,94,158,255,0,31,126,26,91,180,109,55,137,172,45,33,66,0,23,215,49,196,100,198,252,149,203,127,180,43,243,113,45,24,168,211,239,244,219,217,254,199,137,32,154,
222,98,26,32,121,140,124,139,146,6,115,159,238,144,59,84,150,90,77,223,153,32,150,24,196,115,184,71,138,73,134,21,129,224,227,250,154,230,88,185,62,133,125,78,55,189,207,208,11,239,218,59,225,164,86,242,
249,90,210,93,17,17,255,0,143,68,154,224,28,42,169,199,151,25,207,74,181,225,127,218,159,193,122,119,139,32,109,70,59,155,81,52,76,96,38,213,220,202,178,56,101,32,100,96,124,164,115,95,1,203,225,171,253,
73,100,134,192,194,177,219,49,19,72,164,101,1,36,47,61,201,198,50,63,17,94,229,240,235,225,255,0,194,79,8,120,118,13,103,226,30,179,3,235,23,33,100,211,190,221,112,35,17,40,7,49,196,136,75,48,206,115,
156,231,61,184,21,157,76,92,218,54,134,18,55,208,244,239,21,124,112,248,79,169,106,114,201,163,124,52,241,209,150,214,235,206,51,67,113,105,12,82,176,73,98,4,137,102,98,170,68,238,121,0,242,15,106,241,
79,136,191,26,174,53,221,83,196,122,150,151,165,217,216,89,235,55,111,21,212,119,119,209,73,113,12,173,20,72,199,100,96,134,82,177,71,140,99,241,205,101,234,154,175,195,117,241,5,222,163,107,113,28,112,
90,126,239,75,211,180,239,15,52,54,136,25,243,36,146,238,199,154,216,10,203,128,2,224,142,166,187,239,14,92,124,36,248,131,175,207,107,119,160,31,237,201,99,38,202,88,174,90,211,201,192,224,32,1,88,145,
129,193,220,24,18,8,53,146,198,205,187,39,247,51,73,97,34,222,177,60,31,81,241,103,137,90,206,75,179,169,219,199,0,186,105,213,237,224,143,230,151,207,55,33,241,183,35,247,196,203,158,205,235,210,185,
253,107,198,154,169,240,158,149,127,101,171,204,215,145,203,36,18,67,34,149,242,196,121,88,140,101,118,130,187,36,147,130,15,14,71,114,15,163,124,116,240,43,120,42,213,53,200,108,45,238,180,123,198,17,
197,231,78,76,231,157,223,48,3,59,71,221,45,216,227,32,117,175,154,111,110,4,179,92,176,88,226,138,73,75,253,156,19,178,51,236,59,86,145,156,166,175,125,136,149,40,71,161,187,119,227,111,17,234,246,112,
216,94,235,23,183,55,68,199,20,99,33,118,34,231,42,54,129,198,9,224,231,32,12,146,5,80,176,215,175,237,174,196,209,234,55,8,192,124,175,184,103,232,48,56,205,97,110,84,42,209,237,243,129,56,218,167,138,
125,206,230,84,50,100,177,231,3,31,174,41,242,197,232,209,9,45,143,70,211,181,73,238,227,142,125,66,121,102,153,192,242,69,188,160,50,41,225,129,65,140,19,129,130,57,227,168,173,28,201,110,18,56,175,45,
138,194,164,202,230,82,70,198,235,242,145,247,179,198,125,235,204,180,91,231,91,216,195,185,16,169,231,3,159,211,175,165,92,213,175,103,55,183,16,193,35,52,110,248,220,9,37,143,215,175,181,115,79,14,159,
160,88,236,6,180,239,115,117,246,59,251,248,154,86,24,48,220,249,97,248,249,131,128,70,83,177,245,192,227,138,235,60,63,174,195,103,107,168,233,55,94,50,213,108,236,47,4,107,127,37,165,161,150,75,248,
193,24,80,101,206,192,167,56,3,25,3,34,188,126,200,168,93,185,143,120,31,48,112,122,125,127,207,74,177,151,101,221,44,170,177,28,169,98,50,114,65,231,142,115,244,172,229,135,91,13,54,182,59,214,212,163,
73,148,218,222,75,120,242,93,69,29,172,142,48,97,57,194,100,146,88,99,140,109,194,130,56,192,20,154,237,165,214,167,28,115,49,186,191,213,110,46,177,120,242,18,173,188,47,10,238,217,98,199,28,147,206,
222,122,226,184,205,9,90,45,86,214,119,134,57,97,183,117,153,209,238,90,19,34,116,192,144,13,202,115,130,25,70,70,56,174,235,196,218,180,243,233,113,95,189,228,210,171,202,10,162,153,10,92,36,71,15,36,
108,64,203,135,98,155,134,91,37,143,98,41,58,124,173,40,149,20,154,58,255,0,27,95,233,135,226,23,139,14,157,168,70,218,115,94,162,233,183,66,35,10,34,136,213,165,219,18,112,67,146,241,17,208,0,74,224,
156,215,39,226,13,83,75,183,97,127,20,190,117,211,132,138,91,24,237,71,239,212,228,49,50,22,60,0,114,62,233,39,142,213,233,26,191,195,61,27,81,240,11,107,190,18,241,12,218,205,249,176,145,164,209,117,
40,154,210,242,210,227,42,66,196,7,250,224,168,31,147,242,185,96,65,3,145,206,120,3,71,240,76,210,233,23,254,37,210,165,184,209,161,180,138,247,207,177,146,89,77,234,100,137,139,13,193,81,160,59,29,163,
63,51,140,237,83,208,243,174,88,199,153,173,191,225,138,112,151,54,166,159,194,79,13,203,125,225,177,171,221,60,112,139,116,142,251,74,181,251,98,196,247,173,28,219,137,27,142,210,6,221,191,48,198,79,
184,53,163,241,63,81,208,181,51,165,207,164,92,93,203,112,214,70,227,80,179,154,212,20,183,33,153,130,111,67,134,144,231,36,100,241,179,213,107,183,186,208,44,172,111,124,79,99,29,246,149,119,39,135,175,
83,82,183,176,154,220,75,13,254,157,46,11,73,28,235,129,185,73,120,218,54,228,148,83,208,243,79,70,208,181,43,237,111,93,240,191,132,111,110,225,213,181,93,62,29,66,214,11,200,213,90,241,20,59,92,219,
6,25,2,88,207,151,40,42,78,240,140,6,54,228,113,202,163,155,114,249,163,86,173,30,91,25,90,39,138,173,53,143,13,232,169,119,163,189,238,163,167,220,131,125,125,168,222,180,226,226,50,223,36,17,219,112,
128,124,137,184,147,187,110,227,147,147,94,181,240,250,31,134,90,247,198,157,19,83,241,118,131,119,6,175,37,205,181,233,142,205,82,210,214,34,49,36,96,169,249,229,140,72,2,160,249,65,83,156,122,248,102,
189,164,120,128,248,63,82,241,28,175,5,157,165,229,157,213,212,75,112,168,39,91,184,161,243,210,7,132,109,100,103,14,216,56,193,193,7,7,0,253,47,103,63,195,253,59,194,62,36,58,61,172,79,125,115,127,225,
20,134,226,87,19,74,32,186,146,217,138,198,88,146,20,62,243,129,208,102,185,171,41,183,117,215,204,170,60,207,73,109,230,125,62,126,27,65,165,120,206,255,0,196,246,62,12,72,154,87,157,217,52,205,97,18,
57,68,138,202,204,97,150,53,85,98,9,99,134,198,226,125,107,199,97,241,223,195,175,10,232,250,30,129,21,222,170,82,197,174,97,79,181,65,17,146,79,52,125,204,198,216,4,18,78,78,1,0,226,182,63,108,95,140,
186,20,222,12,212,60,47,224,175,16,125,179,196,54,243,111,190,142,210,101,88,86,15,152,58,73,41,251,172,87,37,85,78,73,92,100,115,95,5,104,94,41,183,185,211,181,141,107,90,81,115,113,28,255,0,104,141,
68,100,70,81,84,15,159,4,49,199,203,223,0,247,175,99,7,140,171,10,41,57,105,178,211,250,238,58,145,195,253,181,171,63,88,62,12,120,243,194,47,224,221,3,71,139,196,22,166,242,56,204,9,185,182,134,219,202,
156,144,49,185,122,122,242,43,215,225,184,73,227,70,141,131,12,144,216,32,128,71,99,95,140,222,26,241,55,139,150,73,52,155,81,14,233,97,158,123,72,78,49,107,44,114,70,206,25,143,12,198,41,149,148,30,62,
90,250,51,246,91,248,179,63,134,180,125,56,234,179,75,115,103,113,28,230,120,154,66,92,186,76,202,205,245,1,56,252,184,172,253,146,167,21,200,238,106,212,42,63,119,115,239,91,148,30,114,110,46,20,18,155,
129,239,131,140,254,75,87,228,220,150,110,100,38,73,54,238,96,163,175,174,43,229,219,15,138,171,168,252,23,248,107,174,233,151,146,155,191,17,235,90,125,141,211,201,47,148,246,237,33,243,36,221,188,29,
199,98,176,35,240,207,6,186,95,18,252,80,210,165,214,188,109,225,219,219,145,117,107,162,89,177,154,213,72,89,93,162,96,101,109,224,130,48,55,54,7,32,40,245,231,196,119,230,154,182,255,0,240,198,202,55,
113,87,56,207,219,131,196,158,49,209,126,8,193,127,225,95,17,199,164,69,115,172,91,218,106,23,43,18,188,166,210,121,60,159,221,111,4,103,115,161,35,169,93,192,16,121,31,0,248,247,225,63,197,109,58,118,
159,70,241,54,159,169,232,50,68,36,75,200,110,163,178,49,198,87,254,90,197,180,178,247,3,107,48,59,78,49,210,189,47,246,187,187,181,189,248,14,154,141,174,169,61,221,188,158,39,138,59,72,159,81,121,163,
136,71,28,65,138,41,39,144,230,81,158,192,30,227,53,243,159,143,62,32,75,171,252,30,240,191,135,46,39,121,245,8,228,184,251,86,127,130,52,148,249,67,61,126,117,35,114,231,105,218,78,1,174,252,171,11,82,
141,4,174,155,109,244,39,48,81,141,75,121,126,165,109,35,192,158,24,158,11,133,215,62,40,216,91,106,194,38,116,130,11,38,146,39,144,2,74,52,165,129,249,177,128,216,28,144,72,61,43,163,209,60,45,240,74,
43,227,22,181,226,63,20,121,115,218,193,117,4,137,117,105,110,241,44,169,157,142,188,171,50,176,96,74,146,8,199,189,120,154,199,116,240,239,87,1,91,32,123,250,212,218,38,153,113,113,116,109,226,148,35,
48,220,118,40,228,250,158,107,215,154,118,119,155,254,189,15,61,94,250,35,219,126,36,232,31,8,124,53,162,165,239,130,252,87,171,120,131,92,155,104,129,47,26,3,29,167,171,184,137,64,98,6,113,187,35,32,
113,95,65,120,123,226,127,252,36,63,10,210,253,172,218,246,253,199,217,111,22,57,82,63,38,81,26,5,125,167,168,96,160,123,16,61,107,224,155,136,238,82,249,173,179,230,74,146,108,194,142,73,206,56,245,230,
190,133,248,27,167,106,250,127,134,53,123,219,200,130,219,201,115,228,71,11,2,38,141,192,66,238,202,71,8,223,234,249,231,114,55,21,133,90,146,195,195,218,65,251,202,251,249,171,51,74,73,84,154,132,150,
154,126,7,208,218,93,244,109,109,230,40,218,48,88,228,213,184,103,46,98,114,72,220,132,144,126,163,252,107,131,209,239,21,154,40,132,128,41,194,20,98,51,216,145,93,78,173,48,251,125,176,193,72,46,119,
50,149,224,237,7,230,233,208,228,53,124,93,74,22,185,245,20,234,93,26,251,198,73,28,128,113,154,246,223,217,71,80,72,124,109,171,105,237,210,242,196,58,227,185,141,255,0,193,205,124,221,172,107,186,125,
147,39,159,119,5,157,178,198,94,70,154,64,18,21,24,201,39,252,107,31,194,63,180,103,131,124,11,227,219,45,86,8,239,245,152,109,188,196,147,236,80,236,89,85,148,140,163,202,85,72,206,14,73,228,126,21,48,
203,241,85,149,233,65,191,200,140,69,122,74,14,50,149,153,243,159,237,25,227,77,99,199,31,25,60,89,226,45,109,228,107,166,191,158,206,8,88,144,45,173,224,154,72,226,137,84,159,148,0,185,62,172,204,123,
140,121,99,185,50,178,168,4,227,230,111,79,106,233,254,42,248,146,211,197,127,16,252,79,226,93,51,76,58,117,134,175,169,205,121,21,163,200,174,214,226,70,220,202,74,240,114,197,219,143,239,87,59,107,109,
53,211,42,67,25,32,242,73,56,3,220,215,232,180,35,203,78,49,181,172,182,62,94,172,147,147,101,104,6,9,56,103,124,112,42,213,181,187,187,130,81,143,126,14,7,231,86,82,43,59,116,151,207,118,150,100,255,
0,150,74,112,15,227,212,213,173,19,86,183,179,134,79,244,84,47,140,130,235,156,15,199,165,116,40,182,98,229,97,246,186,116,179,249,197,55,21,78,92,244,9,239,156,215,85,225,63,15,107,154,179,109,210,172,
119,196,23,105,121,100,194,47,183,205,198,79,97,92,150,175,119,121,116,145,35,91,60,37,73,109,184,43,145,156,131,143,167,122,244,207,0,93,221,207,162,238,132,34,73,18,151,184,12,11,16,188,15,212,15,195,
174,42,159,34,91,146,156,174,157,142,183,72,240,247,136,124,61,162,105,247,47,45,181,173,190,161,51,91,24,124,182,142,227,143,187,33,43,247,134,115,142,71,35,7,61,107,159,91,91,223,10,252,83,159,74,186,
213,134,149,6,177,167,53,136,157,109,119,169,4,159,32,200,165,178,35,99,140,176,98,84,237,97,128,26,187,104,239,175,86,38,181,183,212,205,214,159,117,3,45,136,103,111,244,77,217,82,172,10,224,40,32,242,
7,168,227,3,52,188,81,21,226,93,232,119,26,240,183,22,240,90,157,63,78,154,11,148,202,49,70,83,150,33,138,168,201,43,188,125,236,115,200,21,199,41,83,133,221,205,39,55,201,169,233,145,233,150,113,107,
90,61,236,122,156,90,157,141,219,219,207,117,50,19,229,88,79,44,98,37,118,149,24,9,152,161,46,81,151,230,98,188,169,2,189,215,68,248,105,119,101,115,121,99,163,95,173,214,154,151,137,10,92,74,206,162,
82,192,6,119,81,150,49,22,93,185,7,0,115,200,205,124,165,224,141,78,125,79,77,210,117,17,119,118,154,125,138,162,121,102,77,158,124,177,134,95,48,252,171,147,180,146,83,4,29,160,227,56,53,244,31,128,62,
43,220,69,173,89,220,153,39,58,85,158,159,29,189,220,66,52,80,36,44,229,92,48,249,66,112,227,111,94,122,87,157,57,198,77,115,45,191,175,212,84,166,211,189,143,65,241,119,134,181,47,14,92,120,146,87,137,
210,214,226,5,139,205,83,230,64,197,118,158,36,44,112,71,203,212,100,110,238,6,7,135,248,196,73,166,248,90,250,125,42,238,233,53,233,158,35,121,113,110,200,172,236,206,185,104,137,82,3,38,78,220,156,112,
65,227,53,244,45,231,198,207,11,219,11,227,112,97,189,208,5,204,43,115,121,110,193,214,55,147,17,240,58,17,147,247,148,224,142,58,241,94,17,227,31,16,248,23,73,241,214,173,225,141,87,86,211,237,35,102,
79,177,201,60,207,36,119,86,82,71,152,101,70,111,148,19,243,112,9,251,173,200,233,95,69,144,215,165,74,115,139,149,175,242,179,48,204,169,202,180,99,40,171,217,234,132,104,173,52,77,54,7,184,183,130,226,
242,226,221,44,172,97,5,193,114,238,89,45,163,110,178,157,168,90,71,239,201,32,0,0,209,240,221,149,183,135,164,212,148,188,115,93,105,150,193,102,145,45,152,153,239,37,111,54,98,157,118,143,245,106,1,
206,7,3,165,115,63,15,18,77,35,77,77,70,105,31,196,26,149,184,91,109,57,109,229,141,212,18,74,9,20,142,16,50,8,213,152,243,247,250,86,229,212,87,250,30,137,246,91,171,248,110,117,59,132,50,106,55,96,237,
65,35,56,127,144,28,242,73,101,68,61,129,36,156,87,217,97,171,66,80,78,58,219,250,255,0,134,62,106,181,25,70,78,50,234,122,47,133,225,184,154,209,226,149,13,188,62,65,80,188,2,165,128,251,195,57,200,28,
99,61,8,20,203,134,137,38,198,101,107,216,223,100,214,128,231,118,121,7,118,51,140,12,140,246,56,53,203,218,235,115,196,144,195,101,5,194,197,28,121,145,204,37,227,144,134,225,99,115,141,229,143,241,244,
235,201,171,55,119,183,177,222,190,177,128,151,50,196,68,96,18,18,68,3,132,110,50,87,115,15,152,119,28,100,26,244,35,94,14,54,234,112,58,51,191,169,37,253,165,173,230,161,168,29,61,25,238,70,150,179,90,
174,227,24,103,142,105,24,114,57,28,128,164,245,27,189,234,183,137,47,116,221,22,206,234,245,98,73,226,26,121,190,57,125,251,6,7,147,230,147,235,151,192,31,123,110,49,199,57,122,86,181,13,199,140,52,201,
44,102,242,26,234,194,232,207,5,193,6,77,136,98,218,202,1,229,79,39,61,9,60,145,210,184,173,38,254,199,82,212,53,47,10,94,234,139,124,210,106,209,90,218,64,138,91,203,130,29,178,59,176,231,32,196,60,176,
113,128,92,47,83,92,149,43,193,251,209,93,119,249,92,236,133,41,47,137,232,173,167,224,126,123,221,195,103,46,157,12,246,198,69,96,49,58,56,0,7,237,179,29,70,50,73,61,248,21,81,28,161,3,102,229,28,34,
183,111,168,21,171,166,233,183,58,132,86,246,58,117,172,215,90,132,140,25,162,135,44,113,216,158,203,245,56,250,215,215,127,179,167,236,159,226,159,20,95,89,235,254,42,99,166,104,249,36,42,160,146,75,
134,219,140,40,35,24,231,169,28,99,60,215,231,117,43,66,10,242,208,251,24,66,83,118,71,201,250,45,141,222,183,18,105,218,118,151,60,250,172,243,229,18,214,32,193,211,96,27,66,117,220,92,100,158,156,245,
21,247,151,236,249,251,36,195,61,130,106,127,18,175,76,240,90,0,241,232,246,174,226,17,43,224,249,111,49,193,114,120,202,32,11,146,114,91,173,125,57,160,120,35,225,127,193,253,59,79,22,186,78,157,102,
144,18,173,60,159,61,197,220,170,185,217,191,150,145,128,25,252,0,21,141,227,127,138,151,77,225,132,215,18,221,60,37,165,234,183,70,11,107,195,3,92,94,206,160,255,0,203,24,177,181,73,207,7,230,39,167,
21,197,83,21,42,139,247,122,35,170,52,163,15,139,86,116,247,126,22,240,135,132,52,203,24,19,79,178,180,213,48,171,109,160,105,150,163,114,162,159,184,21,121,35,213,137,199,169,3,138,226,254,46,235,183,
169,167,205,14,163,34,155,153,163,243,99,240,182,151,32,111,41,0,249,26,246,101,224,40,234,83,33,73,227,230,235,92,119,133,245,63,20,252,77,240,222,176,218,59,75,225,191,3,218,66,102,191,213,166,114,250,
142,180,249,96,170,242,145,145,141,167,40,56,92,129,138,183,241,39,195,190,19,240,223,192,217,238,180,223,59,79,134,250,109,158,99,187,181,198,162,248,202,51,7,0,128,216,39,60,128,43,21,21,29,222,191,
215,245,223,208,114,147,229,109,108,124,237,174,235,26,247,138,117,251,101,188,184,55,151,188,69,111,18,15,221,68,51,194,162,244,10,43,235,43,31,25,89,124,59,240,30,152,158,39,241,69,165,156,190,94,60,
217,216,201,184,170,228,199,4,17,252,210,183,24,1,71,227,95,13,61,240,142,116,154,218,225,20,35,148,148,70,228,178,252,185,232,58,142,41,250,157,186,94,200,215,183,215,166,234,242,125,169,28,166,66,239,
44,106,79,66,120,84,0,227,111,24,57,233,201,174,234,177,132,218,132,180,71,20,42,184,93,245,61,215,196,159,180,134,157,103,126,38,240,135,130,222,242,248,76,242,54,171,226,107,128,127,120,122,60,86,144,
146,160,99,143,153,149,128,252,107,202,124,111,241,251,226,255,0,137,103,243,47,124,115,119,104,164,96,195,166,90,195,111,16,30,128,21,118,3,254,5,94,103,125,4,151,55,203,111,20,176,219,68,34,110,36,249,
158,35,184,140,113,212,241,238,14,122,215,3,226,104,230,211,196,18,201,117,44,177,74,160,175,222,76,159,78,122,254,21,209,74,150,30,218,47,212,29,105,203,67,181,215,60,67,171,234,119,109,115,170,107,55,
215,151,12,119,52,151,23,44,196,156,99,61,127,149,99,11,164,46,91,205,4,158,251,235,10,24,44,229,134,83,45,194,197,113,22,119,195,35,159,51,3,28,224,142,156,213,34,218,113,206,217,220,227,175,238,250,
86,233,83,91,63,192,134,229,216,237,78,161,115,37,180,118,230,234,86,183,141,139,36,123,178,170,216,193,35,223,20,200,166,100,147,112,111,155,190,123,215,38,34,136,90,189,204,55,46,99,83,130,84,99,154,
72,110,230,18,42,37,204,132,145,144,25,50,63,207,20,114,193,189,24,175,46,168,250,107,246,108,212,254,20,175,143,35,184,248,195,117,122,52,104,21,86,206,1,110,210,217,179,179,124,198,229,148,18,145,140,
3,142,16,228,151,232,43,245,131,73,212,244,189,83,195,246,122,167,133,165,211,238,244,102,183,255,0,67,159,79,149,26,217,147,111,10,140,188,17,140,122,98,191,5,45,117,119,70,30,114,116,254,37,235,249,
87,168,124,30,248,181,226,191,134,58,141,198,169,224,253,93,109,161,185,201,188,177,184,67,45,149,230,70,51,44,64,129,191,166,29,74,190,64,201,32,109,174,28,110,89,237,151,53,236,215,221,253,127,86,58,
40,98,84,116,177,230,5,55,107,58,134,220,46,47,174,78,230,227,0,78,253,107,183,211,165,140,89,218,186,91,226,49,51,131,116,71,92,47,41,159,161,206,62,149,193,232,209,180,247,96,74,75,179,179,60,140,196,
2,228,146,204,199,182,73,39,243,175,75,125,36,159,130,126,36,215,162,98,203,103,127,109,28,91,198,2,199,44,158,75,16,127,189,150,25,174,159,134,153,54,109,232,121,22,183,120,247,250,156,247,108,120,145,
190,64,123,40,232,63,42,238,254,20,91,248,97,124,57,227,187,221,102,125,66,47,17,38,145,179,195,43,12,101,173,228,188,222,89,214,98,57,0,198,2,169,111,147,37,178,65,219,94,118,227,218,187,93,56,37,158,
128,3,157,184,137,153,143,161,34,185,219,182,134,145,71,24,229,7,220,36,167,240,147,233,218,162,102,193,60,140,230,163,12,85,16,119,192,166,179,170,96,177,193,110,5,119,185,246,56,212,73,51,151,2,187,
143,135,151,58,157,188,90,172,122,125,178,139,107,180,72,239,46,164,200,85,141,73,38,48,112,70,227,191,129,212,156,87,4,141,43,51,21,140,184,24,10,170,57,98,78,0,250,146,64,30,230,190,203,240,47,193,139,
155,143,14,89,62,167,120,186,77,134,139,102,139,119,122,214,207,52,19,222,72,119,148,117,71,7,40,160,29,231,229,80,70,115,208,114,215,170,148,84,119,185,189,58,109,234,121,253,163,220,216,232,80,222,253,
130,201,166,88,247,109,186,137,230,155,127,77,220,48,25,224,99,32,227,21,115,192,246,122,199,140,188,105,164,104,242,45,148,41,121,54,233,36,91,101,68,143,24,44,204,114,57,231,161,35,36,245,175,102,210,
62,8,13,104,220,203,164,248,186,223,89,104,238,188,153,100,210,202,180,80,229,60,197,73,36,232,172,87,105,193,236,125,198,125,91,246,123,248,85,107,225,205,103,86,212,46,217,110,245,41,217,96,67,61,184,
6,16,196,174,87,60,243,235,143,233,92,209,223,84,116,90,218,35,206,255,0,104,29,78,215,225,79,128,180,15,8,233,41,107,62,161,60,33,150,73,97,207,151,26,238,6,118,110,178,150,200,80,31,13,156,177,39,6,
190,13,214,252,69,113,231,185,178,18,77,56,1,94,234,102,44,91,3,28,147,203,126,53,238,159,183,22,185,38,175,241,251,197,118,104,204,150,218,109,208,211,33,8,196,230,43,113,130,0,237,151,105,15,255,0,168,
87,207,186,144,88,161,201,136,132,110,222,149,149,36,170,165,82,91,61,144,234,77,197,242,196,199,184,213,245,121,28,51,93,200,167,176,92,1,82,217,248,135,86,183,145,11,204,39,8,193,128,149,7,4,114,8,35,
4,16,112,115,218,170,236,46,196,237,198,58,3,81,176,7,57,174,190,74,109,91,149,24,169,205,107,115,219,108,126,33,221,120,167,76,180,176,215,110,38,186,72,200,138,69,152,238,33,8,198,79,247,135,251,93,
113,215,145,94,123,226,171,36,211,117,251,184,34,141,37,128,177,54,251,208,184,42,73,193,57,238,48,125,199,7,189,115,58,76,205,111,171,89,178,18,9,149,80,227,184,98,7,245,175,169,60,25,224,249,117,107,
127,50,27,162,146,111,47,33,141,0,243,126,95,226,108,16,160,28,29,222,216,239,90,197,90,38,115,149,221,207,155,173,180,189,93,220,121,90,124,236,36,97,176,42,128,31,28,224,103,245,236,71,6,162,146,18,
135,202,150,54,138,69,36,56,36,100,31,64,7,165,122,143,134,46,174,27,95,212,127,180,117,33,57,176,146,72,163,127,47,100,119,3,36,111,25,228,3,142,59,144,125,235,205,60,75,16,79,17,94,20,127,221,75,33,
145,31,97,65,134,231,129,232,58,81,173,174,37,107,216,203,85,27,142,194,78,120,193,29,106,213,245,204,162,66,22,231,205,98,67,49,219,142,112,14,65,31,151,30,149,32,184,221,49,158,69,143,108,125,84,16,
187,254,148,194,98,107,133,112,141,228,133,206,1,228,122,115,67,42,195,244,155,233,4,143,178,21,124,131,187,60,224,113,200,255,0,26,208,134,68,158,245,34,46,82,50,14,78,210,114,216,227,35,235,84,18,102,
186,216,177,128,183,8,24,39,28,57,108,19,146,49,140,159,174,59,87,73,109,167,91,77,36,118,118,247,209,188,141,106,37,59,72,255,0,90,20,150,141,137,199,66,15,35,218,178,147,72,45,216,135,69,154,25,99,104,
229,118,55,2,85,120,192,81,203,109,35,4,245,3,167,31,90,208,241,11,207,105,162,104,242,198,247,174,20,177,135,204,98,209,196,202,196,159,44,103,0,150,25,42,113,212,158,230,183,60,49,225,109,30,252,105,
55,122,141,212,186,110,143,38,149,21,237,237,228,119,74,242,60,132,176,145,81,92,16,164,16,62,76,119,60,214,223,140,254,13,235,54,26,164,246,190,24,187,183,213,237,39,142,57,173,203,78,33,158,227,119,
202,112,156,43,56,56,224,48,39,35,165,98,229,14,107,50,213,55,185,181,162,143,18,223,105,243,106,126,8,212,27,84,75,120,11,106,22,31,104,19,75,34,56,81,52,209,169,1,166,181,82,68,114,5,37,148,128,54,132,
195,15,70,184,109,69,46,97,241,192,180,142,203,69,212,97,88,181,4,176,27,227,73,27,1,146,228,141,197,157,213,91,202,184,24,220,62,78,28,40,110,103,195,26,103,135,87,196,182,218,69,251,75,225,31,26,65,
44,114,233,183,193,205,189,150,166,1,3,100,184,219,246,59,167,5,144,72,155,98,118,44,174,163,112,83,218,124,46,26,205,135,138,215,76,240,252,186,77,254,163,116,179,173,182,141,120,234,44,188,77,110,236,
223,104,176,144,2,86,11,181,42,73,65,255,0,45,23,204,143,39,122,63,5,104,57,191,115,110,251,255,0,157,255,0,51,95,135,212,227,188,116,53,45,62,201,80,75,2,234,58,0,121,44,173,140,98,84,212,108,164,84,
146,39,89,73,253,226,249,106,171,147,195,20,7,130,72,175,65,210,117,115,99,123,225,191,21,104,15,111,30,173,160,203,178,71,249,118,59,97,100,140,74,122,130,200,66,112,62,235,154,243,111,137,182,118,26,
111,246,78,173,164,92,94,220,248,51,237,179,216,219,197,119,129,169,120,126,228,110,121,180,171,181,63,197,27,7,116,110,142,187,176,115,247,177,180,235,155,139,57,228,2,232,92,219,107,240,70,177,133,192,
17,93,66,199,1,135,108,1,27,14,236,172,221,112,43,206,173,9,197,242,55,174,233,153,55,105,106,106,120,190,242,11,248,97,190,210,224,154,43,59,141,74,238,222,48,203,137,18,11,150,121,33,70,61,127,118,225,
225,25,244,199,122,243,123,141,91,81,181,208,166,178,178,185,100,177,149,109,91,27,206,68,144,177,146,28,31,188,187,11,54,54,145,142,157,134,61,27,196,49,90,92,252,60,215,46,109,33,113,29,244,48,223,90,
170,177,253,219,201,34,205,146,122,157,178,2,71,166,226,43,200,181,9,155,236,250,67,93,202,18,61,66,214,214,72,229,39,42,168,197,212,22,233,141,152,249,177,156,1,158,107,162,151,189,39,109,239,253,126,
166,18,139,122,163,168,241,142,162,243,234,222,113,11,37,188,178,9,68,119,18,135,105,101,200,86,50,227,145,150,13,207,247,78,122,243,93,21,140,17,233,154,166,151,226,139,25,237,143,133,98,108,203,13,195,
252,209,128,54,221,89,200,57,203,162,57,145,88,128,174,155,8,60,19,94,121,23,147,110,206,110,99,50,220,66,89,227,218,119,36,236,140,84,12,142,170,195,36,48,237,131,222,172,233,87,174,211,165,188,128,71,
165,207,242,221,8,128,37,99,7,119,152,79,170,183,35,241,29,234,229,27,43,47,235,254,28,133,45,110,122,223,131,36,134,203,196,126,58,240,197,219,173,166,161,97,50,92,89,152,166,46,60,182,137,97,105,85,
178,120,242,218,217,200,206,8,115,142,84,212,223,8,245,40,237,52,203,11,123,153,111,14,175,13,253,228,50,46,208,241,52,196,172,234,160,246,30,89,117,207,3,36,14,226,184,11,104,36,178,181,181,185,183,154,
5,191,210,85,244,169,212,198,84,61,187,130,87,63,222,4,18,131,156,101,64,234,5,81,179,214,252,139,232,238,236,174,81,79,154,146,160,149,9,9,112,19,10,205,130,9,82,6,198,250,138,110,165,219,229,191,252,
31,248,38,202,172,147,186,61,99,198,186,203,120,87,196,26,103,133,109,245,43,231,211,124,55,112,111,116,232,73,196,75,34,198,141,17,246,2,55,97,156,28,18,43,14,61,126,246,218,27,189,50,245,222,107,205,
86,218,250,13,78,240,146,215,17,193,123,60,115,92,29,196,225,139,36,113,166,227,209,26,165,253,163,116,231,154,31,4,120,222,213,91,236,122,245,143,216,103,80,192,148,158,44,20,86,193,251,198,22,101,39,
191,144,61,70,112,174,108,166,186,134,198,245,166,83,45,255,0,130,76,76,23,59,133,196,72,209,200,223,238,229,35,249,189,205,101,200,212,99,39,46,223,128,229,86,119,247,78,43,198,36,94,248,102,226,123,
130,194,88,167,6,30,126,80,220,179,176,30,224,133,252,79,74,226,34,134,41,211,205,67,185,73,251,222,135,210,189,11,95,64,214,54,182,13,251,200,197,162,72,98,3,13,189,212,151,231,175,0,15,233,92,62,135,
101,53,189,204,222,98,19,11,72,2,177,24,14,71,82,7,249,244,174,188,35,180,26,190,194,167,39,45,196,138,214,88,156,219,202,196,40,60,47,165,95,91,87,141,150,75,119,100,124,99,32,213,203,194,175,168,72,
234,114,185,234,106,254,159,19,221,220,69,111,4,47,44,210,184,72,227,65,150,118,39,1,64,238,73,175,78,16,86,187,41,187,108,123,127,236,91,240,14,195,226,237,255,0,139,167,215,47,174,45,180,173,50,31,179,
36,150,210,21,157,111,36,85,146,57,7,85,100,85,39,42,217,12,88,113,197,83,240,167,136,110,219,193,215,58,14,171,108,78,167,165,106,119,90,100,151,98,7,184,55,42,174,17,12,171,247,183,7,85,80,114,112,160,
146,122,154,251,255,0,246,39,240,66,120,3,225,160,176,152,35,107,87,243,27,205,75,105,4,121,204,170,2,3,142,66,34,170,3,223,25,239,95,0,252,102,211,181,31,10,252,123,241,221,158,145,13,193,134,207,197,
51,94,75,28,146,249,98,113,59,9,212,71,234,14,230,3,61,193,199,34,188,216,90,173,89,95,84,182,244,58,92,57,105,198,75,114,189,238,181,173,216,137,236,146,202,43,109,82,213,82,229,149,213,93,14,83,0,140,
49,98,54,145,130,56,249,143,161,170,222,44,212,117,27,169,47,108,238,239,175,10,7,15,106,35,70,132,199,186,48,233,30,7,37,78,10,159,94,167,165,82,241,61,246,157,169,219,105,58,130,186,199,119,167,179,
198,119,21,14,209,113,34,30,15,43,243,145,244,24,174,43,198,94,51,153,181,136,230,134,226,229,45,164,137,55,201,40,11,35,200,56,110,65,193,69,10,160,99,29,235,127,103,11,222,49,87,244,253,73,149,121,218,
215,100,90,141,149,215,246,115,90,51,67,99,110,228,73,115,177,50,210,12,6,220,92,156,187,101,122,129,209,64,207,99,204,106,90,85,162,105,82,207,109,231,9,192,223,243,224,103,143,83,216,240,71,122,218,
182,214,217,237,16,220,92,218,67,115,50,6,83,179,231,249,219,10,71,231,130,189,241,235,81,120,138,232,89,233,182,247,141,108,211,51,47,217,103,6,92,43,183,98,125,8,231,7,190,0,53,180,101,85,53,115,158,
238,71,13,162,199,28,226,89,29,241,177,193,198,50,8,250,214,141,214,162,173,102,96,137,12,96,96,18,188,126,126,181,62,153,30,129,117,117,136,77,246,156,38,144,121,113,9,190,212,95,176,92,176,27,14,114,
65,57,200,224,115,131,91,86,122,70,141,58,203,37,213,133,204,113,188,173,7,159,37,211,40,70,216,8,98,163,238,243,206,236,113,156,115,219,119,86,48,187,98,232,114,154,116,79,123,118,87,204,80,228,114,29,
136,200,174,174,210,206,209,29,149,116,152,100,101,134,48,178,176,220,162,65,201,101,13,129,184,183,222,80,122,30,115,144,43,19,84,104,98,187,143,236,86,145,219,5,41,185,35,56,83,242,227,161,254,34,195,
39,181,118,111,35,199,224,177,21,222,157,231,155,153,22,72,4,151,63,187,141,227,198,247,242,241,131,192,29,72,24,199,82,43,42,149,36,237,216,106,13,173,141,93,2,234,214,220,219,106,186,131,218,133,182,
132,27,73,45,100,10,145,150,24,242,241,206,215,198,78,59,116,198,43,103,72,241,7,132,236,116,91,118,75,183,91,148,145,17,96,179,44,25,148,1,147,130,112,50,125,185,219,94,23,125,121,119,117,114,235,45,
200,42,141,140,33,245,244,94,199,21,119,76,139,201,136,204,209,152,241,193,119,61,63,62,106,37,132,115,213,201,175,65,54,125,1,105,174,104,186,230,163,107,111,105,61,221,141,253,194,152,175,167,146,72,
255,0,210,3,16,14,192,65,85,29,49,145,223,159,90,236,52,239,134,77,171,233,183,235,115,104,144,222,92,42,125,158,226,243,82,38,113,32,25,203,36,3,102,65,11,145,200,244,38,190,112,240,238,191,101,103,114,
174,19,51,41,70,19,201,27,50,91,237,108,156,42,114,192,140,130,62,149,220,73,173,107,250,126,183,169,69,22,191,171,111,142,37,19,93,11,131,29,172,91,129,104,136,88,142,118,157,204,57,231,158,120,2,170,
56,88,83,248,95,234,16,75,177,232,254,7,213,69,229,158,171,240,255,0,196,54,107,30,161,167,234,57,96,209,238,71,42,126,233,13,159,147,105,12,185,193,43,140,247,175,76,210,252,61,167,199,170,107,80,107,
246,130,227,70,212,17,81,163,88,148,69,56,251,129,25,78,62,96,205,198,50,48,114,43,226,143,13,106,167,251,99,237,119,247,186,132,115,220,35,48,187,140,25,238,146,108,13,140,73,59,152,241,183,32,146,56,
224,140,215,173,105,183,190,37,179,146,217,45,174,167,108,134,158,73,190,215,49,243,213,65,117,117,37,140,123,212,128,54,20,25,57,35,24,197,57,80,110,233,63,67,69,45,44,118,26,190,169,111,225,189,99,70,
210,245,152,146,223,66,77,86,72,46,115,17,17,249,63,102,98,138,85,72,4,44,190,91,1,131,142,188,226,175,124,108,151,193,190,42,240,230,149,168,233,22,177,153,211,108,14,101,183,15,43,161,82,89,84,71,243,
121,123,178,79,0,6,30,248,174,197,252,87,167,120,218,1,173,120,251,236,215,247,175,60,114,67,23,217,145,102,128,160,25,49,146,112,155,213,24,3,223,127,4,98,188,106,238,249,117,29,41,244,159,15,11,152,
111,108,229,107,173,48,189,187,195,121,229,43,200,118,200,51,184,190,194,165,159,57,114,50,65,198,12,168,174,100,222,150,45,61,116,103,32,254,3,214,44,108,110,53,173,10,95,179,36,146,69,228,165,149,211,
64,209,68,227,42,236,170,67,50,28,28,55,66,70,58,213,239,248,78,124,127,225,219,150,134,61,106,246,242,206,209,67,129,168,192,178,130,253,62,82,70,115,206,58,251,87,81,224,175,26,203,246,168,95,82,58,
124,143,117,27,90,5,185,141,21,38,220,188,101,65,201,27,142,113,192,56,245,230,189,192,255,0,194,15,171,220,233,214,246,250,53,175,159,115,111,18,24,38,150,88,29,103,32,231,37,126,234,238,220,164,227,
229,218,15,74,184,85,169,6,238,254,224,115,140,180,154,71,146,232,223,180,7,136,237,117,24,159,196,30,30,179,189,185,142,44,187,194,228,58,71,254,208,233,235,199,255,0,174,183,37,248,209,225,109,110,206,
47,62,242,255,0,71,152,183,250,201,163,37,10,228,5,85,116,201,80,7,215,39,154,139,197,30,10,210,166,188,190,128,218,71,16,145,217,90,49,38,227,128,72,198,238,11,1,211,61,235,149,212,252,2,173,125,110,
227,59,161,93,177,68,224,108,82,123,226,189,120,227,241,177,141,185,174,142,89,96,240,114,149,212,108,207,93,184,190,211,53,212,179,111,9,95,105,151,151,240,179,206,46,205,192,86,66,20,124,133,73,220,
67,2,209,128,114,0,249,142,78,51,107,224,206,147,110,254,59,241,46,181,253,159,117,166,79,125,166,219,52,241,74,10,50,57,121,64,140,202,48,91,98,136,215,42,216,62,135,138,249,155,80,240,18,218,94,196,
238,101,105,153,75,179,130,19,118,84,144,1,94,64,246,170,26,109,247,142,124,62,209,71,101,226,61,98,216,42,151,152,155,147,58,15,69,85,125,220,87,77,28,206,163,146,114,134,138,222,71,5,108,21,27,56,198,
109,95,185,245,95,129,188,17,103,225,109,26,219,84,107,59,29,55,75,211,100,243,35,183,88,129,51,201,128,60,217,50,114,231,221,141,123,190,131,241,105,181,47,5,92,90,219,222,188,118,113,39,144,13,140,99,
237,119,110,88,171,162,22,32,32,92,229,156,246,228,118,175,1,241,55,138,116,251,77,126,210,45,83,70,188,189,77,98,73,85,22,238,223,124,13,134,218,37,100,200,94,115,145,17,198,123,156,2,104,253,157,53,
61,67,197,30,48,214,116,197,180,177,48,203,118,110,32,114,20,173,174,54,226,64,120,66,85,0,225,134,213,36,129,187,25,63,43,36,170,43,179,220,140,157,249,81,133,226,77,94,242,211,91,186,240,199,132,166,
107,235,155,44,220,65,170,220,234,140,69,162,49,141,35,249,166,5,216,101,178,73,220,196,176,56,197,104,104,158,38,150,207,80,23,154,252,205,172,78,171,28,86,115,92,55,200,144,150,242,154,40,212,12,34,
172,201,243,21,93,196,74,185,36,1,93,119,237,117,167,120,39,67,135,79,211,14,149,115,22,175,111,122,137,60,177,96,33,181,145,29,214,235,126,237,192,153,23,5,178,78,67,41,0,48,53,243,124,183,55,166,247,
201,214,229,75,120,139,60,87,142,141,147,243,128,26,225,115,208,100,69,35,14,156,19,92,85,113,13,89,37,255,0,4,198,115,112,103,210,150,191,27,237,52,111,11,235,90,86,157,111,60,222,34,190,142,226,209,
136,80,82,21,50,110,137,192,25,5,252,151,10,199,214,47,122,249,239,226,183,142,53,93,124,190,187,174,93,220,95,204,171,251,152,207,44,66,131,181,85,123,253,125,78,13,114,250,195,106,94,24,188,240,223,
136,163,141,36,212,236,230,87,188,88,55,51,45,197,187,151,219,183,28,249,145,111,29,129,92,125,107,168,248,165,167,195,97,241,50,255,0,195,161,124,141,49,238,32,189,178,146,64,25,13,173,218,130,36,79,
238,133,249,215,29,51,22,113,205,108,170,42,106,246,255,0,134,186,255,0,50,92,220,150,167,29,226,251,200,52,237,67,87,146,212,110,185,181,211,224,183,51,224,52,18,184,10,205,42,96,240,192,51,35,19,143,
184,5,73,113,168,201,101,109,12,41,37,205,236,239,27,8,166,151,96,84,141,192,109,217,24,25,207,126,57,192,172,157,98,202,230,207,86,215,96,104,228,141,237,117,22,179,146,45,217,242,206,211,184,243,215,
118,9,7,209,134,42,125,47,80,146,247,72,156,127,103,197,119,6,157,7,152,97,60,249,170,24,47,78,6,113,159,151,24,228,251,215,68,43,42,139,221,70,47,83,75,71,186,138,248,91,180,50,66,140,93,151,115,176,
144,29,128,109,63,55,204,0,27,184,198,15,61,77,114,223,20,110,103,22,250,108,250,118,170,179,24,230,146,218,85,129,196,142,172,65,118,220,2,241,199,7,4,240,7,53,180,183,214,83,234,247,123,52,243,109,102,
173,18,220,178,0,152,42,120,201,83,133,7,3,169,254,28,115,205,80,149,99,211,237,53,27,77,54,221,162,150,123,144,183,17,68,72,51,200,196,185,80,95,5,6,211,195,31,151,158,15,81,91,195,112,177,230,246,151,
45,111,116,237,116,237,26,73,2,202,173,242,187,50,183,221,101,235,215,29,249,172,155,105,156,9,246,170,151,101,0,3,140,117,173,7,22,247,119,87,204,150,166,221,19,59,85,91,113,24,235,248,224,99,189,54,
198,108,196,201,107,25,17,111,1,216,144,114,123,244,255,0,235,87,98,212,174,166,206,159,20,210,120,85,155,201,95,50,75,140,4,218,0,192,63,95,106,124,54,193,80,200,194,52,100,79,152,99,160,219,212,31,169,
169,163,191,182,26,68,118,201,3,132,13,206,58,30,255,0,254,179,94,161,224,77,26,223,72,240,183,136,60,107,168,216,218,199,115,5,148,246,218,82,234,114,170,68,101,120,31,108,139,17,5,158,80,84,121,74,64,
14,115,216,134,173,27,140,23,53,175,97,43,202,232,241,167,94,213,11,40,32,130,78,15,94,106,123,136,226,142,99,18,200,193,16,42,175,205,158,221,201,254,117,77,28,153,66,146,167,62,220,143,202,187,94,38,
148,162,211,71,58,165,52,238,153,210,120,105,9,186,50,36,138,155,83,119,32,157,220,242,56,254,85,236,154,189,254,144,223,179,95,136,45,154,236,71,175,27,205,62,41,44,88,145,178,35,120,28,50,241,134,4,
42,146,71,76,224,247,174,111,224,167,132,223,90,23,79,178,92,200,203,0,194,142,21,134,239,48,100,117,24,224,116,99,197,122,55,237,71,99,225,191,15,248,31,251,51,77,153,164,213,145,161,73,78,224,113,26,
200,178,236,63,240,29,152,250,87,151,82,74,220,182,61,8,69,164,229,208,249,118,20,223,112,138,71,12,195,53,175,173,234,86,173,166,75,20,115,7,103,24,80,188,215,50,215,106,195,3,35,158,79,167,248,209,10,
199,43,184,101,111,51,178,227,0,125,106,57,53,187,23,180,210,200,170,243,178,182,54,28,158,226,172,172,47,229,44,140,185,36,103,39,28,15,106,188,97,68,85,25,12,203,206,79,76,213,27,128,75,150,45,219,36,
230,180,114,108,205,36,111,248,23,82,209,116,159,22,232,218,134,181,109,113,113,97,103,120,147,205,4,11,151,152,166,89,23,146,0,27,194,19,236,13,126,129,120,127,226,29,175,136,190,12,248,127,80,147,68,
185,251,127,137,53,95,47,75,211,100,98,144,237,50,249,76,199,110,23,103,151,150,227,118,78,43,243,82,20,146,231,81,130,24,35,146,89,100,96,18,52,92,147,223,129,248,19,244,21,250,49,251,34,248,3,89,212,
180,191,134,154,244,55,146,77,225,125,55,73,47,184,163,71,25,184,195,21,242,187,151,4,131,184,141,184,36,30,115,88,213,130,106,253,77,20,157,185,81,237,223,179,175,193,139,63,135,30,31,212,237,116,13,
78,234,56,175,221,38,150,43,156,205,28,108,171,130,203,25,42,6,99,1,114,70,239,151,57,233,94,207,167,105,81,55,136,180,233,90,89,4,178,92,66,12,204,224,239,10,223,40,61,121,235,205,75,167,141,78,89,103,
142,100,185,138,69,68,34,70,68,218,249,234,70,27,36,100,103,244,246,169,124,47,8,151,196,58,57,107,137,55,199,57,221,32,127,149,219,112,249,89,64,249,91,142,63,222,60,86,77,123,134,169,234,126,49,254,
209,87,55,47,241,255,0,226,12,146,252,178,143,16,106,10,64,25,255,0,151,169,113,255,0,142,226,188,190,254,237,165,98,21,137,92,96,18,49,154,244,175,218,46,234,57,126,58,252,65,146,34,192,31,16,95,133,
249,123,11,185,135,243,6,188,186,68,105,100,218,171,150,60,112,50,106,240,235,247,48,244,68,85,254,36,189,72,100,149,208,2,217,56,245,21,69,165,45,33,29,137,227,21,98,246,41,160,216,101,15,30,255,0,185,
230,41,93,255,0,76,245,252,51,81,219,66,172,91,121,7,6,182,86,51,36,176,67,246,251,102,251,219,103,70,57,244,12,13,125,147,225,253,118,242,47,4,38,139,104,126,207,103,48,15,112,70,3,74,184,251,173,254,
200,60,129,235,95,32,91,21,142,234,18,160,97,100,83,140,240,112,192,215,209,126,30,119,146,197,29,22,86,140,178,151,193,224,147,192,199,215,61,43,90,102,85,47,208,173,44,26,84,26,197,228,165,81,39,157,
114,229,184,24,201,228,102,188,59,88,134,123,175,21,221,89,218,163,221,74,211,178,194,128,228,178,128,88,99,39,24,219,207,92,87,162,248,223,196,150,86,250,156,177,68,139,113,121,109,186,18,188,148,66,
27,230,25,61,62,96,71,168,239,94,108,151,81,193,125,121,112,39,204,242,143,149,153,59,158,123,246,28,15,122,42,121,5,52,197,116,242,172,141,188,104,6,240,31,114,175,36,128,123,210,53,196,94,80,30,76,101,
9,24,14,55,30,57,252,58,227,142,220,83,38,184,157,247,22,97,50,176,203,22,30,253,125,51,147,250,213,65,56,92,254,235,115,30,112,122,14,253,43,43,23,212,159,78,182,121,175,8,130,53,69,83,151,103,60,40,
200,207,39,142,1,206,58,156,113,93,87,246,85,147,94,181,174,155,168,199,168,73,45,218,89,195,113,110,173,9,186,50,99,102,216,228,249,128,57,218,67,96,130,1,60,86,69,129,105,111,32,18,75,246,100,108,72,
247,72,141,36,80,70,65,203,108,94,184,25,221,158,131,158,213,222,124,62,211,124,71,162,235,151,58,105,176,211,181,11,11,200,158,73,173,46,221,30,59,152,212,16,36,76,253,236,134,199,4,28,30,118,241,88,
84,168,163,167,94,198,144,133,245,123,26,62,12,211,174,188,39,175,221,71,171,175,217,245,107,73,215,203,180,154,220,180,23,59,211,180,132,96,48,32,3,158,159,76,26,245,95,22,248,178,199,88,240,28,218,37,
252,231,79,189,134,194,98,151,40,100,102,180,68,10,88,186,3,191,229,100,70,12,185,39,104,56,174,83,196,41,23,139,52,251,43,118,191,142,56,194,167,246,107,220,200,11,172,241,171,150,183,184,147,130,33,
113,242,198,231,44,25,138,156,134,218,93,47,138,116,155,207,13,248,135,88,213,44,143,219,117,45,58,104,162,130,113,228,203,229,34,0,224,58,147,176,135,109,167,42,113,128,120,6,184,92,229,43,73,173,78,
148,225,29,19,34,191,215,238,245,61,39,77,180,23,118,158,40,123,100,32,178,177,149,173,164,89,9,73,173,230,32,57,221,141,205,17,12,135,110,14,50,73,230,244,73,197,214,187,36,145,198,186,150,163,113,114,
26,107,15,48,218,189,236,187,137,42,146,46,60,153,227,33,74,113,184,108,3,156,100,104,88,205,165,107,254,29,209,110,180,15,12,73,167,220,90,137,7,246,141,165,216,96,89,20,43,179,8,207,152,188,161,113,
129,147,147,201,169,245,95,14,234,190,33,54,203,61,221,174,179,123,48,196,237,42,197,21,209,76,28,110,124,2,234,57,192,116,206,79,95,76,53,132,181,57,106,182,229,116,89,248,217,226,171,63,20,44,26,149,
245,157,241,241,107,121,118,247,250,147,64,45,230,212,163,143,238,46,163,0,198,205,66,14,210,0,82,100,25,224,237,219,228,47,169,220,194,22,33,40,204,69,26,54,65,128,219,78,228,127,168,245,244,227,181,
106,120,235,80,241,5,205,252,113,107,87,55,87,23,86,200,109,210,91,216,136,184,49,169,249,81,156,141,210,42,144,118,150,44,70,78,24,131,92,229,157,189,198,167,28,144,71,36,30,108,76,102,80,227,107,183,
24,96,173,253,222,132,175,175,205,93,45,70,166,172,198,78,236,244,143,8,120,142,229,180,45,103,71,149,154,88,26,205,197,174,112,62,206,173,201,76,247,93,192,31,99,211,131,129,199,92,109,184,184,209,76,
113,203,242,19,108,113,150,192,105,84,18,7,98,1,63,74,169,165,20,135,195,186,189,220,204,4,204,233,167,195,17,39,118,231,6,71,127,248,2,168,198,114,9,56,173,61,2,246,53,154,52,43,26,182,119,100,194,93,
176,28,22,218,249,2,50,71,114,8,61,56,235,88,78,60,158,242,233,254,68,221,173,205,6,191,143,74,241,101,205,196,16,177,180,182,184,157,44,210,93,199,100,108,89,114,50,126,108,161,32,251,147,82,248,75,73,
182,151,94,91,44,45,205,157,236,91,26,194,9,202,189,228,32,18,30,54,254,61,152,109,241,240,216,201,231,128,116,237,116,223,8,248,170,211,91,45,171,55,135,117,13,45,99,54,82,222,92,147,103,116,143,35,15,
45,195,110,242,8,108,101,145,138,101,195,21,198,64,230,215,82,151,75,191,77,50,200,174,159,122,70,219,193,52,74,46,164,249,9,5,219,7,43,131,198,220,122,140,138,105,74,201,43,237,253,127,95,144,249,82,
119,102,213,216,185,183,209,245,59,9,45,158,125,93,219,251,54,238,51,54,215,11,35,44,150,51,145,129,185,145,215,96,7,7,113,234,55,102,151,192,54,58,86,177,170,234,26,94,177,123,253,153,113,115,103,35,
53,215,217,30,227,108,145,156,161,17,41,12,84,144,202,251,62,117,59,78,8,206,41,180,209,93,148,142,246,226,84,188,253,220,75,37,196,251,36,69,224,152,139,28,249,128,16,178,36,132,238,83,144,112,5,94,176,
178,215,116,223,17,64,171,166,155,253,98,61,72,220,219,89,172,102,43,187,137,91,50,41,88,206,11,6,10,196,50,240,65,232,67,12,213,73,46,91,45,88,210,190,166,255,0,131,245,155,221,127,192,90,207,128,111,
163,77,250,137,130,239,64,153,230,5,87,80,181,141,9,181,86,63,42,137,225,93,160,228,13,254,187,234,222,155,111,115,115,225,43,88,138,59,93,67,167,235,122,41,82,14,245,150,82,179,70,164,117,28,19,197,92,
241,45,134,145,169,233,30,35,214,52,176,139,108,139,105,226,93,41,217,8,86,88,216,27,146,157,179,229,49,202,99,42,209,231,128,203,81,105,186,153,138,91,189,67,164,143,107,109,122,10,127,12,178,57,137,
219,234,74,243,92,211,173,62,84,225,31,95,192,174,88,165,239,61,76,45,59,85,188,26,165,220,44,126,206,182,141,34,222,76,168,30,100,64,177,198,193,59,6,30,94,220,244,25,106,231,181,193,117,48,146,232,217,
125,154,201,28,71,2,34,147,26,40,200,24,126,135,144,123,228,146,79,210,27,120,252,223,16,94,73,113,231,11,115,113,32,150,72,152,42,131,146,88,176,207,32,145,140,12,242,69,110,120,147,88,147,80,211,238,
44,153,101,141,246,125,172,164,67,16,176,12,170,17,151,162,149,77,164,99,169,39,142,181,48,118,196,71,77,94,254,95,215,245,184,226,213,173,115,136,138,66,102,60,240,125,107,232,191,217,119,72,177,254,
219,139,90,157,22,107,200,201,16,238,255,0,150,67,145,185,127,218,60,243,233,248,215,205,49,147,189,136,24,57,175,163,127,102,75,208,140,240,153,48,251,200,199,124,122,138,246,235,223,217,232,92,45,204,
174,126,147,124,44,190,72,224,86,46,6,64,86,246,62,245,241,167,237,189,4,58,119,237,11,122,246,241,73,246,237,87,69,142,227,4,110,92,224,168,145,84,12,177,71,137,73,255,0,123,222,190,154,240,29,202,75,
104,129,78,229,117,228,126,21,225,31,240,81,189,55,76,191,178,240,31,140,37,189,149,47,37,130,230,194,56,2,236,89,112,86,98,124,204,252,174,161,27,111,174,77,121,84,223,239,149,186,157,210,214,147,62,
53,241,85,229,136,177,22,236,17,214,91,125,247,177,163,121,46,178,35,239,82,156,112,163,149,42,57,110,49,142,65,241,189,65,195,204,74,185,49,146,66,14,112,6,123,122,87,162,235,62,15,241,142,171,99,167,
235,13,161,78,186,125,237,187,92,193,119,112,194,48,214,169,184,155,153,9,194,164,61,71,154,118,171,55,220,13,223,148,185,240,191,137,34,132,79,46,135,125,26,121,62,111,207,9,76,39,64,216,108,48,206,62,
80,64,102,234,160,138,245,233,242,199,75,156,13,59,149,180,128,37,150,38,134,206,22,47,136,155,116,123,130,158,50,224,3,215,143,175,90,232,60,103,28,237,161,91,94,253,142,84,136,57,196,146,184,36,230,
78,8,29,199,65,236,49,88,186,125,142,167,167,153,110,39,181,185,180,65,25,27,221,54,121,109,180,48,108,159,65,215,235,207,35,21,163,113,226,37,184,208,143,219,52,212,149,167,125,209,102,77,202,113,232,
15,65,144,58,119,166,211,230,78,34,181,140,189,62,91,20,179,154,224,195,46,55,43,5,200,96,135,61,84,250,19,216,242,49,87,175,47,133,190,251,152,194,165,204,168,85,163,87,200,198,115,207,185,239,89,211,
234,23,151,152,137,214,60,42,237,25,64,184,169,44,180,216,229,83,36,178,130,1,231,61,63,42,57,57,157,228,23,102,52,87,0,220,18,225,216,96,128,87,214,186,123,41,245,40,173,18,104,53,41,163,151,63,51,0,
48,131,29,6,114,49,235,193,205,46,157,13,172,151,11,16,124,13,141,243,34,100,46,7,28,113,193,56,31,141,103,93,75,117,145,12,184,202,29,165,85,190,80,71,92,122,214,174,55,87,182,129,205,208,187,37,234,
168,92,55,156,209,150,49,6,0,44,123,142,91,110,6,78,79,60,254,149,38,169,13,165,222,141,107,117,29,193,147,80,114,134,72,204,167,8,15,222,27,71,3,21,136,99,119,155,203,193,97,220,14,255,0,90,190,14,0,
82,62,81,216,213,198,200,151,190,132,179,91,79,166,72,139,44,108,133,227,18,35,14,152,97,199,62,189,114,58,142,245,110,79,18,106,208,233,113,105,112,94,77,21,147,22,43,109,22,213,142,32,199,231,33,64,
28,183,124,147,88,58,134,170,238,30,11,124,178,51,100,133,229,115,140,103,220,251,210,233,49,54,240,243,2,50,114,121,228,210,43,99,119,78,211,210,232,41,154,249,237,31,115,5,147,201,44,20,0,54,183,200,
119,100,243,156,30,14,42,237,187,220,105,218,140,23,122,84,230,246,230,206,34,92,79,27,220,36,141,206,95,203,99,146,121,233,216,128,195,146,106,140,183,48,149,69,138,50,179,140,239,33,242,27,211,232,107,
180,179,184,240,253,155,89,220,233,247,102,45,73,102,222,24,6,60,227,31,49,63,116,114,121,30,253,115,138,87,108,75,65,116,253,83,84,150,205,27,71,240,197,212,58,189,194,53,188,151,50,69,39,217,165,222,
6,21,20,225,99,63,46,67,158,1,206,51,146,15,183,89,233,107,22,147,107,174,217,234,250,114,107,178,73,28,13,168,203,194,69,254,179,204,141,99,45,149,27,144,142,73,225,179,156,99,28,54,183,226,141,39,81,
208,236,219,69,210,183,248,150,41,146,197,36,87,149,13,214,225,149,194,140,163,21,60,135,227,131,201,224,214,229,230,183,226,239,11,232,246,122,142,143,168,105,178,107,151,51,93,233,218,172,82,108,146,
38,87,114,0,88,155,10,92,48,39,126,126,95,78,162,177,169,204,210,177,172,18,190,167,51,224,107,189,94,211,91,191,138,222,29,61,244,151,212,62,200,210,220,34,172,66,71,144,162,21,118,249,151,44,7,0,156,
100,125,79,168,233,22,151,134,77,74,44,70,243,66,210,4,158,203,2,120,100,40,140,11,13,199,123,35,228,2,184,31,48,200,32,228,121,135,194,91,79,19,105,158,45,210,32,180,210,210,234,207,83,107,105,102,23,
123,29,100,130,57,182,180,164,177,24,10,100,109,204,62,110,125,178,61,199,81,211,98,93,74,237,180,201,224,134,43,107,141,202,240,128,190,68,113,51,33,220,228,228,170,130,48,223,196,50,114,107,25,180,165,
110,228,207,85,115,238,27,15,30,124,58,79,15,232,222,23,241,198,154,147,234,49,105,246,209,201,36,154,103,218,34,156,180,107,134,86,80,204,1,231,168,4,96,253,106,140,159,11,126,16,120,198,73,31,64,185,
251,60,114,174,97,147,76,187,15,22,87,134,27,27,112,227,35,142,8,244,175,149,108,252,69,167,95,120,127,77,158,107,192,151,201,12,106,247,214,155,229,84,108,110,76,140,170,202,141,207,206,6,72,207,29,235,
132,241,183,138,132,90,196,66,206,104,35,189,46,75,220,219,57,183,146,35,183,5,183,166,8,192,59,185,39,131,220,138,170,117,28,82,138,232,116,197,199,150,253,14,219,246,147,240,78,153,240,243,85,142,91,
61,78,231,80,183,187,139,100,111,37,160,84,129,193,192,12,202,126,108,130,79,0,96,14,122,228,120,220,223,14,181,207,18,79,107,125,225,182,107,150,187,143,47,109,27,33,146,54,201,69,44,153,220,3,177,82,
50,58,84,30,54,213,245,141,116,233,146,106,186,237,222,175,43,89,180,80,69,60,198,88,202,157,202,9,8,66,6,3,42,78,3,16,6,115,94,199,251,39,252,71,176,248,109,172,248,139,83,241,29,150,169,112,183,177,
164,49,136,30,57,45,144,140,177,50,33,59,128,7,0,20,201,235,145,128,13,109,12,92,225,53,239,104,250,63,235,67,147,17,74,156,165,205,24,144,221,233,82,248,155,78,179,211,116,59,155,27,253,38,226,17,30,
163,169,234,151,49,193,23,148,89,81,193,4,96,35,7,108,46,223,155,102,55,1,150,172,159,135,158,54,208,252,1,225,191,18,105,218,54,137,45,238,155,172,120,178,13,46,207,81,47,28,143,20,34,120,163,96,216,
109,199,41,189,144,128,65,0,2,84,245,243,123,91,25,147,76,147,77,75,183,77,127,79,50,180,122,124,225,154,51,23,15,112,155,7,222,110,67,96,229,182,29,201,144,8,175,46,212,174,237,109,155,65,182,75,182,
182,137,117,51,51,186,203,184,67,16,126,28,58,231,118,7,43,234,49,156,100,215,138,177,10,163,229,72,210,83,228,179,93,79,172,127,105,8,226,179,212,44,180,251,123,123,13,70,198,234,224,54,147,123,169,31,
44,198,242,146,243,233,205,39,241,43,162,44,136,224,21,5,20,144,25,70,255,0,57,241,60,54,58,53,155,199,103,9,214,167,134,218,73,162,188,146,16,202,176,35,16,234,16,145,189,226,206,50,64,59,88,16,164,10,
195,240,119,141,45,245,253,47,92,211,252,75,226,51,175,105,75,58,70,22,234,220,52,50,194,99,43,31,154,209,128,241,21,49,111,89,99,251,185,228,96,112,205,95,80,138,38,150,195,86,103,184,210,96,152,77,12,
141,118,191,218,118,196,70,66,77,4,136,90,57,250,245,202,229,14,29,79,34,185,42,198,242,180,186,127,95,215,252,19,57,190,103,116,97,199,169,73,169,120,82,215,194,218,164,137,38,157,59,155,141,11,86,182,
114,5,219,130,89,161,151,25,97,40,92,144,58,54,10,247,21,196,252,69,26,245,134,182,116,141,105,100,145,90,218,51,109,43,220,180,235,37,187,35,121,79,12,153,199,146,235,202,133,227,42,227,130,24,14,162,
195,71,188,185,134,245,96,142,227,251,50,247,99,73,106,170,195,205,101,36,249,150,251,176,109,174,148,149,127,40,227,119,240,156,240,53,52,214,131,196,246,240,248,31,196,23,234,154,129,149,164,240,230,
168,202,16,52,142,217,146,212,151,251,169,51,97,144,31,245,119,0,169,249,93,86,182,86,77,219,250,254,187,47,151,98,90,185,145,174,43,235,127,216,250,220,183,86,214,167,196,90,101,164,183,51,72,216,91,
123,136,39,91,91,183,99,219,102,244,102,239,180,18,51,88,237,2,36,15,105,106,176,180,112,205,186,102,8,7,153,28,121,32,228,255,0,180,161,129,234,1,6,189,79,68,209,44,97,183,181,209,222,211,102,153,61,
212,190,109,172,138,91,200,121,34,251,45,237,177,206,124,180,222,109,46,84,19,158,91,143,148,215,153,124,68,181,182,130,239,197,26,170,73,58,76,218,226,88,67,27,145,254,169,236,150,86,86,245,97,181,87,
233,66,124,233,42,110,221,127,175,75,153,201,61,204,120,172,181,9,209,146,198,198,86,4,121,151,41,111,25,77,135,35,25,237,215,215,185,39,214,188,253,162,145,117,182,184,153,221,39,221,189,37,108,141,145,
19,142,95,184,30,220,117,175,114,240,190,176,99,240,166,178,186,152,149,236,47,140,70,226,113,35,71,45,196,168,24,172,42,192,100,231,113,102,35,166,65,61,129,189,31,132,231,241,252,19,75,101,111,12,58,
117,157,181,190,153,106,75,132,242,14,75,72,209,33,3,113,84,218,72,99,192,57,5,137,219,91,97,177,238,147,247,246,254,191,175,65,193,223,67,205,252,25,164,175,147,113,53,229,130,221,36,160,169,40,192,135,
193,32,149,99,239,207,108,215,113,107,224,231,155,86,208,142,167,167,197,22,155,61,180,97,239,9,86,242,145,179,201,81,209,240,8,206,15,122,179,168,248,98,63,10,201,37,141,147,129,102,75,155,111,58,77,
206,202,15,4,158,164,15,95,175,165,122,166,165,227,125,23,82,187,209,252,59,105,167,165,253,189,156,81,9,102,49,24,188,217,22,32,140,228,147,194,245,200,3,4,87,178,170,198,172,21,74,122,166,107,20,175,
102,84,241,79,193,223,0,67,224,219,93,96,222,92,165,156,8,1,251,45,235,147,54,88,252,197,1,10,15,56,198,59,119,175,1,241,54,139,225,251,123,155,227,164,106,55,55,241,171,19,9,184,133,203,227,238,141,204,
196,144,113,199,35,32,40,237,128,62,173,190,240,247,136,53,79,9,233,177,234,17,43,105,247,178,237,177,210,116,216,132,69,194,18,76,142,120,194,224,103,159,67,88,186,246,147,225,235,29,54,215,79,139,75,
130,89,100,149,30,117,143,45,37,196,129,178,169,198,114,7,183,39,167,76,215,37,76,92,112,234,245,37,114,229,21,109,52,62,71,155,195,183,173,18,222,44,42,35,50,96,146,8,88,248,199,7,163,14,114,106,190,
183,225,141,99,72,179,178,212,47,34,72,236,174,29,162,137,193,32,51,1,156,1,244,4,245,56,175,165,181,189,87,236,250,189,157,142,161,2,193,123,20,199,203,176,142,48,209,193,130,25,139,149,24,44,66,142,
9,192,28,123,215,154,124,117,154,238,71,210,99,185,189,105,214,123,203,155,152,99,49,180,96,103,0,144,167,160,249,240,59,115,85,135,198,58,210,74,214,33,40,155,191,8,252,103,169,104,26,29,166,149,166,
65,246,141,70,234,225,90,37,45,188,48,25,11,149,39,167,44,49,211,24,53,129,241,203,82,159,90,146,123,189,91,81,137,174,228,141,100,159,203,132,124,196,58,228,112,70,0,37,78,123,253,43,185,248,111,103,
105,225,191,135,151,90,163,219,91,207,170,93,66,98,133,72,12,210,38,244,220,128,117,25,234,113,206,87,176,205,121,246,163,167,157,95,196,54,210,93,75,34,137,109,229,179,146,229,35,0,68,238,114,160,110,
7,12,10,128,8,233,93,147,213,232,104,246,177,226,54,241,23,119,104,192,112,163,56,97,201,30,184,167,40,155,206,194,112,196,143,157,134,49,244,175,171,60,59,240,103,65,81,163,234,30,117,196,183,111,42,
230,24,37,114,175,145,208,238,198,9,206,49,247,121,247,174,175,78,248,57,225,183,89,99,189,210,227,145,78,38,133,201,146,57,99,144,30,136,235,147,130,160,101,122,30,226,177,117,172,246,18,129,241,253,
181,165,221,249,153,44,96,154,238,237,9,13,20,16,51,24,198,58,150,28,15,161,193,168,162,208,181,89,175,227,176,104,86,9,204,102,77,151,78,35,220,65,3,96,207,86,57,225,56,61,122,87,219,114,120,61,108,44,
30,218,213,101,142,205,99,242,150,56,151,119,70,24,89,24,14,132,143,226,206,64,52,203,255,0,133,150,16,217,193,169,11,56,167,126,89,217,211,126,7,109,153,92,110,227,134,235,129,214,161,87,149,236,37,6,
124,197,224,111,6,89,76,186,140,119,218,167,216,181,117,154,222,59,107,99,42,169,156,137,7,159,27,0,172,72,40,202,1,86,28,228,54,71,21,250,233,240,166,235,195,218,63,134,180,61,39,66,212,214,29,46,214,
202,24,109,33,88,149,163,129,66,125,223,48,252,199,169,31,133,124,137,113,224,43,59,29,21,218,195,73,125,238,25,162,121,231,49,50,76,199,62,110,70,6,11,17,206,122,251,212,119,186,47,136,230,240,205,182,
174,186,188,172,100,72,2,109,212,101,206,227,181,74,168,223,140,158,132,123,208,167,123,220,165,22,143,189,45,110,208,217,95,52,151,118,173,246,117,62,98,172,107,184,129,158,171,159,66,48,71,90,196,240,
142,169,165,218,106,122,100,173,171,217,8,226,184,68,204,174,176,200,84,3,157,192,156,28,117,207,7,218,191,58,252,103,113,119,161,233,114,90,60,82,79,59,95,55,152,46,218,73,29,85,66,187,109,37,142,14,
9,192,7,129,200,244,175,105,240,86,145,107,253,157,107,169,219,216,207,52,225,139,4,146,202,48,252,140,134,39,110,48,65,206,62,238,15,210,149,211,92,165,41,43,159,22,124,122,240,167,137,174,126,53,248,
225,236,116,107,155,232,38,215,47,229,138,123,84,13,28,138,247,82,184,42,196,141,195,107,14,71,20,207,133,30,7,215,244,239,20,197,170,107,250,84,150,58,95,217,167,87,150,114,184,7,228,234,1,56,175,183,
117,109,59,71,185,51,106,58,157,187,1,28,88,34,73,86,51,16,199,79,148,129,201,252,133,120,191,196,223,17,233,98,19,166,105,12,205,45,193,1,97,102,45,188,99,161,39,32,40,228,245,201,171,167,25,168,168,
116,66,148,149,220,142,79,226,31,141,87,84,240,205,199,131,116,157,22,206,123,27,85,154,72,238,38,129,84,70,178,16,211,58,62,50,6,224,15,60,100,87,206,82,232,119,9,118,208,139,93,205,146,129,17,184,200,
28,144,123,129,130,107,212,252,111,123,163,217,233,198,213,175,229,185,188,97,180,8,81,90,212,171,0,197,179,140,183,67,242,131,219,62,181,229,183,154,156,243,106,51,95,92,75,36,151,151,12,75,176,0,57,
227,110,238,59,224,1,90,168,168,233,20,102,228,222,229,36,210,153,160,130,116,185,137,188,201,24,36,74,114,237,180,100,126,7,181,104,223,120,131,84,142,212,90,90,93,205,26,43,16,60,150,33,201,198,8,7,
183,167,175,189,102,181,201,144,164,22,214,194,53,89,55,168,0,228,156,109,193,39,147,244,165,130,57,100,185,68,71,142,25,226,59,255,0,123,38,196,200,232,75,118,254,184,197,82,147,91,18,245,100,247,214,
54,214,150,177,102,99,61,196,128,22,102,82,2,115,156,46,121,62,228,231,38,161,85,179,141,34,147,113,150,231,118,76,106,6,7,28,3,159,122,218,179,182,134,227,85,51,95,56,185,220,165,227,98,167,107,48,25,
218,123,14,220,123,26,187,166,233,86,63,111,242,46,84,73,189,246,167,144,192,153,14,113,128,125,73,56,31,92,246,169,189,150,163,190,166,64,185,243,85,237,46,172,246,194,197,89,17,119,55,204,59,158,1,62,
184,199,6,178,208,173,149,210,125,166,57,80,78,25,118,75,17,83,183,32,14,27,156,116,227,233,93,70,167,165,94,233,30,32,150,48,246,239,53,186,239,124,202,10,199,254,207,251,68,2,63,58,235,14,167,163,222,
199,165,248,118,111,180,69,20,147,249,81,222,75,34,110,134,73,65,203,5,217,134,201,42,189,120,201,60,227,6,28,159,69,113,164,175,169,139,224,88,116,109,63,196,17,93,75,171,203,108,209,178,152,103,128,
239,184,183,113,200,41,22,65,113,201,12,16,147,142,54,176,36,87,208,122,70,145,161,106,118,18,193,173,197,167,199,107,173,75,178,13,66,210,227,202,211,117,57,119,96,20,149,64,91,59,206,112,203,132,89,
27,119,202,50,113,227,158,27,208,53,95,12,75,43,106,122,231,135,173,180,201,218,69,158,195,84,116,54,215,101,1,94,88,163,134,99,142,159,187,62,245,163,225,223,23,89,105,183,77,31,135,181,8,188,44,247,
234,209,94,233,90,225,55,154,30,168,8,63,35,103,119,150,164,117,222,56,0,5,96,43,202,196,39,93,251,173,180,187,127,87,191,223,230,111,43,193,36,209,220,120,147,225,165,245,175,159,165,69,102,211,105,203,
230,125,189,239,102,48,188,72,0,219,19,197,142,36,39,113,12,167,99,175,42,220,12,248,222,171,37,206,137,124,66,49,133,124,207,53,94,81,230,153,118,183,202,205,184,28,186,253,204,159,188,1,12,9,21,239,
250,87,141,188,77,161,107,122,118,139,174,248,71,77,178,240,124,145,121,86,57,212,204,182,176,169,60,173,157,235,6,93,141,156,45,180,167,103,10,1,76,3,94,117,241,226,211,74,139,81,69,211,117,79,178,89,
106,211,187,77,107,119,27,127,161,202,170,27,32,147,148,87,10,62,76,28,245,4,143,154,162,147,159,63,44,245,79,250,233,212,206,124,173,105,185,87,192,222,42,190,62,44,187,214,214,246,206,216,180,24,187,
137,44,162,134,57,161,64,118,72,168,84,141,234,205,181,200,192,193,82,123,10,238,163,241,46,191,226,125,33,140,182,62,20,212,108,228,141,82,226,223,88,208,227,109,172,164,144,158,104,146,32,70,78,65,231,
25,39,57,226,190,111,83,119,166,11,27,201,98,6,123,92,183,150,37,14,146,194,114,178,42,224,224,228,146,8,236,216,39,140,85,217,230,176,125,102,214,222,255,0,75,138,243,77,10,143,4,38,125,134,120,220,0,
167,205,0,178,18,58,158,64,59,189,43,162,88,116,229,204,191,35,37,54,142,247,198,23,90,78,161,226,91,120,117,125,51,64,178,119,71,89,100,211,252,76,205,20,88,192,7,47,189,34,96,57,17,244,114,220,244,174,
3,196,182,16,105,154,232,147,67,191,55,182,209,226,104,167,141,150,83,17,244,145,144,108,4,126,0,131,239,90,186,236,90,13,246,169,169,69,225,134,152,88,66,45,96,211,210,228,121,83,220,180,141,137,119,
133,60,186,147,130,203,242,178,160,108,12,154,195,183,212,155,77,186,213,180,153,76,109,3,9,45,156,178,23,12,202,229,114,50,120,25,82,112,122,241,233,84,169,56,236,71,83,31,84,191,107,187,243,112,208,
71,12,140,138,28,32,192,44,58,144,59,103,210,181,180,233,35,104,24,146,200,10,133,37,122,251,145,89,237,167,205,127,48,130,198,9,167,184,242,154,95,45,20,22,216,184,201,198,65,61,71,3,39,158,1,168,45,
124,248,163,102,66,73,136,231,110,220,231,7,191,167,106,39,79,158,10,218,18,226,237,161,219,92,120,98,238,213,237,97,100,142,102,150,205,46,225,130,73,21,89,214,76,237,201,60,51,224,28,132,206,63,26,191,
225,189,66,11,45,127,79,131,91,213,46,45,44,35,136,70,116,201,44,140,204,201,134,249,80,140,109,3,182,122,28,12,1,94,141,113,175,89,167,192,175,133,254,48,211,108,81,60,79,224,77,86,235,66,213,33,63,39,
218,82,64,102,180,154,76,130,9,42,2,19,207,36,240,49,198,221,182,177,14,179,98,158,51,210,97,26,142,189,110,200,205,4,246,145,178,223,194,132,176,136,130,54,181,196,36,176,27,74,249,169,242,177,220,3,
86,51,131,166,237,83,91,253,215,253,63,173,75,81,73,158,119,241,59,193,26,110,149,160,104,158,50,210,34,184,186,210,175,181,3,20,172,110,195,193,17,93,178,197,180,128,24,137,98,89,1,15,194,62,85,114,0,
39,38,219,198,41,105,173,234,243,197,164,195,125,101,60,49,193,102,154,169,119,184,210,132,108,90,221,237,103,86,204,77,19,51,108,43,149,216,118,17,198,71,65,47,140,19,92,191,138,96,182,54,154,35,197,
28,70,221,49,29,184,104,157,164,137,204,121,40,142,174,73,207,3,248,14,71,79,48,149,110,109,110,230,154,234,8,110,73,103,12,161,25,176,88,147,247,65,200,61,192,25,3,182,64,171,140,221,146,237,253,124,
255,0,48,149,239,204,143,96,248,97,125,127,170,109,210,44,9,150,59,123,214,188,154,218,105,144,121,246,151,42,209,220,12,177,3,114,72,230,78,160,237,125,188,147,138,226,126,215,113,167,90,221,105,119,
13,186,226,222,49,109,35,48,218,93,162,152,156,227,220,228,253,13,98,248,42,246,61,47,199,26,109,190,167,18,255,0,103,221,171,88,223,199,33,3,116,19,46,214,13,216,16,74,54,14,14,80,14,180,207,25,109,177,
215,239,236,159,83,143,85,150,202,225,160,55,176,169,69,185,40,2,153,54,158,87,144,65,7,184,61,176,107,57,209,147,37,222,195,77,238,88,131,200,221,150,199,124,127,139,31,210,174,94,52,235,30,200,93,84,
78,165,228,155,7,247,184,193,242,240,79,29,242,123,96,112,114,43,150,182,114,24,18,223,55,94,153,193,255,0,63,169,173,203,71,154,234,5,183,0,177,88,152,66,139,22,236,176,237,199,66,121,203,159,215,138,
203,217,40,77,59,19,179,185,69,6,215,113,232,107,213,254,3,222,180,30,36,8,174,138,14,15,204,125,235,201,158,80,206,113,211,3,241,226,187,207,131,247,73,109,226,120,217,191,187,206,87,57,175,110,74,244,
236,110,190,35,244,111,192,115,121,144,90,161,243,20,22,198,3,240,121,4,12,255,0,156,214,207,198,255,0,14,248,99,196,126,1,210,46,188,77,166,69,171,90,120,111,92,23,177,216,78,65,183,153,158,55,139,108,
195,32,148,2,77,251,65,27,153,84,30,9,174,35,225,156,210,61,157,139,46,194,170,185,96,79,34,186,223,138,150,247,215,255,0,9,62,32,90,105,214,201,115,170,37,138,221,90,194,252,36,147,35,171,40,39,211,129,
207,181,120,211,78,50,186,220,239,142,177,104,240,255,0,136,31,18,244,249,32,185,184,214,45,173,39,117,154,57,119,188,217,141,164,137,112,146,152,217,74,229,91,105,140,99,100,120,249,23,63,53,124,205,
227,63,138,145,92,94,236,179,183,242,152,177,102,189,188,204,128,103,169,68,31,49,98,51,243,54,15,79,188,56,175,54,241,85,214,161,127,171,92,253,162,240,170,199,43,129,130,95,144,199,37,71,110,115,207,
57,29,235,158,134,20,75,149,42,143,44,165,128,5,186,177,39,0,115,146,73,61,0,28,154,222,20,163,107,205,221,158,115,147,185,216,91,235,26,116,193,94,107,137,175,110,37,110,38,189,69,46,237,159,151,106,
156,71,24,4,12,12,19,254,200,53,204,235,214,82,68,241,94,220,65,113,20,55,14,206,139,48,42,197,137,207,70,1,191,19,235,219,165,118,211,216,234,30,31,183,158,218,234,8,96,213,36,11,35,219,218,156,186,161,
31,118,89,64,36,55,251,42,195,28,115,88,250,195,106,211,232,210,46,169,165,220,90,64,228,60,50,74,54,41,25,25,193,115,189,142,72,228,100,28,245,230,171,15,89,74,87,134,207,205,23,102,157,153,199,200,97,
13,146,18,60,12,146,126,102,38,149,87,204,183,198,74,47,39,36,228,159,92,10,179,105,13,147,94,67,21,218,178,218,177,196,142,14,221,190,135,62,153,164,215,30,196,222,183,216,101,243,109,182,174,21,73,42,
173,220,110,238,43,214,73,218,228,245,177,95,205,17,133,48,200,234,228,109,1,88,240,61,232,186,253,220,224,6,5,198,51,176,228,231,235,235,77,96,222,87,150,211,128,88,228,69,26,228,254,38,170,188,171,8,
101,95,188,122,156,115,154,87,97,161,168,142,176,198,102,102,140,40,224,198,15,39,235,84,102,184,154,246,82,57,17,145,130,122,100,122,85,84,6,119,253,239,79,66,113,86,132,177,44,101,1,109,216,194,162,
138,72,123,18,198,144,194,161,78,1,166,221,94,161,140,193,108,55,57,24,223,142,149,86,107,41,102,249,188,178,132,117,44,216,207,225,84,20,148,60,16,49,232,51,78,192,105,193,132,82,30,109,171,223,212,213,
155,125,64,91,222,69,45,162,254,241,24,50,52,139,144,24,119,218,122,143,173,87,75,27,171,139,97,63,146,198,5,97,190,82,64,11,147,129,212,243,207,166,107,164,79,15,105,54,109,12,154,158,167,43,199,48,127,
38,61,46,223,204,146,82,56,251,242,237,69,231,142,121,199,34,139,5,199,248,91,206,55,49,234,66,246,72,18,206,232,20,186,73,188,182,73,66,22,9,26,227,150,232,73,233,180,144,114,9,175,68,240,207,138,23,
83,211,52,29,34,254,102,159,88,93,91,237,41,120,35,70,88,45,164,131,203,112,231,24,32,22,121,3,3,147,187,7,0,100,109,234,250,22,155,225,255,0,3,218,88,55,134,180,185,224,179,184,115,115,123,60,109,115,
246,199,97,145,230,178,133,198,223,149,66,224,40,3,35,210,172,248,24,218,120,151,70,213,47,124,69,225,219,77,62,226,197,151,81,254,208,211,96,54,230,231,147,12,158,91,70,118,229,55,166,255,0,159,119,57,
228,101,107,9,55,203,118,86,198,159,135,47,238,236,60,67,123,172,165,252,203,31,218,29,101,117,243,124,183,143,128,185,234,145,146,170,143,177,64,59,91,208,243,190,222,56,182,62,125,203,233,17,197,169,
220,88,1,121,105,23,239,60,246,102,59,4,77,128,163,42,0,7,168,61,135,65,159,224,221,39,83,178,176,181,179,209,53,72,239,124,61,246,121,150,246,222,88,192,201,195,42,74,146,166,12,110,1,56,117,224,128,
220,96,113,177,226,111,12,248,142,89,166,188,134,11,11,219,73,153,68,50,249,99,50,130,6,65,139,28,28,146,112,51,202,147,218,188,170,216,138,62,211,86,132,167,29,174,113,94,40,241,236,54,122,45,198,155,
167,180,4,204,239,231,61,188,126,99,73,24,195,40,18,227,229,108,231,140,159,80,112,43,200,181,29,110,246,77,87,124,58,172,178,189,223,239,103,178,149,87,17,144,1,249,155,156,16,160,30,24,116,30,181,221,
234,94,9,213,97,243,22,213,45,166,188,130,29,210,71,181,131,186,147,145,130,216,4,227,25,254,246,79,221,233,94,103,226,127,10,107,86,250,147,155,139,72,84,205,35,97,45,206,224,25,118,240,220,97,114,9,
42,122,54,15,74,244,112,245,104,205,123,172,122,180,111,105,210,221,234,183,136,52,251,214,115,149,242,164,75,38,32,49,29,25,183,1,156,140,2,64,27,184,7,154,246,59,221,110,218,225,166,153,35,184,148,171,
111,86,150,82,198,95,148,9,3,238,25,29,1,32,227,56,234,64,175,32,240,175,133,181,171,121,167,15,167,222,207,13,178,174,101,137,132,145,192,140,72,201,32,240,6,14,113,192,234,216,2,187,253,44,88,79,127,
119,253,142,147,61,170,2,179,193,52,146,31,33,155,10,33,119,255,0,150,164,237,206,229,39,130,189,136,37,85,213,94,47,68,17,151,47,169,204,104,62,47,145,135,216,60,65,170,197,3,217,2,250,46,183,28,59,230,
211,101,24,193,94,67,24,143,163,125,220,182,126,82,86,181,103,185,240,206,171,163,106,158,49,177,209,13,176,138,242,79,180,76,247,70,56,210,240,198,131,109,188,49,252,172,178,103,204,42,95,0,191,57,226,
188,155,86,155,69,188,89,39,131,77,125,42,248,131,155,120,152,205,100,231,29,99,44,119,194,115,209,73,100,0,96,26,165,121,174,234,115,232,58,94,130,210,40,211,236,37,184,146,24,227,77,165,222,102,12,229,
206,126,99,242,128,188,12,12,142,115,154,229,169,132,230,248,116,127,167,93,191,225,186,247,188,243,43,52,110,95,234,242,90,175,217,33,150,85,73,237,224,185,185,137,144,161,75,130,197,156,103,0,176,1,
134,8,59,112,112,184,25,171,22,55,246,183,26,138,207,4,55,102,201,33,135,237,17,220,72,129,166,152,111,203,101,78,21,121,83,215,32,175,126,135,136,119,121,62,124,179,4,27,119,99,167,160,174,239,194,222,
27,26,183,135,237,102,135,196,122,5,181,253,230,160,109,211,77,187,190,88,166,40,62,65,54,15,1,119,100,115,216,22,206,43,73,82,86,189,140,237,115,210,180,159,27,37,187,191,246,94,166,255,0,101,242,119,
223,201,169,198,162,230,80,191,242,197,118,0,179,57,232,24,0,192,116,4,215,73,119,225,251,127,31,104,247,31,218,182,159,217,146,220,41,123,91,21,249,154,62,49,191,121,63,51,158,224,17,216,54,14,9,249,
247,196,19,120,125,226,130,210,195,70,188,180,191,134,225,146,226,235,251,72,204,172,21,138,254,238,54,202,228,227,57,56,3,32,40,35,154,93,55,197,158,37,180,176,150,214,27,249,154,219,254,89,59,168,47,
1,254,242,49,206,9,28,29,219,189,177,92,213,112,210,146,92,174,207,191,85,232,82,107,169,235,218,159,141,238,116,187,155,91,93,126,240,93,107,150,210,165,181,213,196,63,242,246,163,228,73,100,206,54,206,
161,153,88,117,97,131,244,242,189,111,86,212,230,210,191,177,159,82,186,158,56,181,73,103,154,57,122,25,82,49,15,156,125,92,162,237,39,190,123,246,228,111,228,189,150,230,73,46,154,115,55,67,231,110,222,
59,224,238,231,63,94,125,106,214,151,157,190,100,175,183,36,227,113,173,249,35,78,26,19,39,165,207,81,240,228,223,218,158,18,138,91,251,175,178,233,90,94,167,246,98,34,27,216,121,168,36,152,196,157,89,
219,98,28,246,193,0,99,53,235,95,14,117,203,137,244,120,180,219,61,62,235,78,208,44,226,101,154,104,158,51,52,174,242,111,33,228,112,68,106,120,95,148,22,10,56,198,5,124,243,21,213,221,252,182,226,206,
40,213,116,232,252,193,229,90,180,145,7,7,106,153,148,28,115,184,166,230,199,101,228,154,247,255,0,5,79,168,95,233,208,201,170,45,244,23,176,168,83,109,133,129,2,244,44,58,101,114,50,14,73,28,14,57,21,
228,99,215,36,84,187,254,31,215,224,68,21,221,209,161,170,120,102,246,107,43,221,70,4,88,38,154,9,54,61,225,116,130,4,201,193,105,27,36,169,36,19,201,36,40,80,57,99,95,72,104,254,21,248,103,167,235,62,
24,241,62,153,165,69,107,117,112,136,22,210,59,181,118,96,223,43,76,176,142,172,193,152,242,54,128,51,206,43,200,175,117,59,8,188,42,151,218,145,251,78,155,1,6,11,59,139,53,188,153,166,220,21,100,42,217,
93,202,196,28,183,200,152,7,156,113,200,120,227,226,101,206,135,109,44,154,52,214,115,106,211,177,181,75,201,137,118,66,23,113,96,133,179,33,32,156,55,8,0,246,197,24,44,93,119,22,173,254,95,215,245,233,
210,165,24,171,179,212,254,54,248,206,219,72,147,84,152,221,9,136,243,12,69,35,88,188,168,1,227,118,6,0,192,25,245,53,243,29,151,139,188,85,38,175,127,115,101,5,204,87,170,191,55,250,36,134,123,116,198,
237,163,43,242,22,82,14,0,220,202,120,200,230,185,139,251,175,17,235,151,232,247,182,51,107,6,64,34,123,133,97,60,50,140,240,205,229,240,236,73,33,135,203,179,3,0,224,103,186,240,223,142,117,235,127,11,
70,250,149,202,221,69,225,129,12,43,54,169,11,221,197,105,15,153,34,162,75,25,116,221,140,237,81,156,157,131,113,24,32,239,26,9,123,211,149,231,235,183,249,24,202,92,206,237,156,77,213,151,136,173,53,
209,20,209,179,106,155,94,238,230,221,230,196,194,48,6,231,102,254,15,188,14,49,198,71,210,177,53,73,239,117,93,78,198,61,65,228,41,110,10,91,192,199,152,227,102,5,137,99,208,29,160,254,28,112,43,210,
52,75,63,17,120,211,95,125,74,249,181,9,82,241,21,39,189,54,233,139,141,131,42,58,42,1,243,18,2,141,131,166,13,121,167,136,33,127,248,79,239,180,248,229,226,214,117,183,73,36,127,148,149,0,51,147,216,
100,158,156,96,113,93,216,22,221,78,88,246,52,140,82,92,200,247,15,4,92,233,233,117,101,52,101,197,190,157,102,99,121,31,18,43,92,28,167,158,78,58,97,142,59,147,154,147,195,241,219,105,208,65,44,80,162,
67,13,186,219,201,44,174,74,111,47,193,35,251,192,1,158,255,0,62,115,87,252,15,62,145,23,131,174,116,235,123,57,47,162,79,46,230,231,10,126,116,73,148,144,95,30,172,23,182,50,106,118,123,125,31,89,213,
174,237,238,166,185,141,110,21,226,134,55,86,51,103,32,41,70,251,167,169,203,114,113,138,244,42,201,68,217,106,143,73,84,176,119,180,85,158,66,165,188,232,163,182,119,216,241,144,27,151,220,15,7,166,122,
115,129,90,218,122,221,207,169,125,174,29,61,90,216,172,76,38,185,148,239,136,14,161,121,198,126,108,123,241,92,213,182,177,105,21,161,131,80,214,109,224,189,184,130,61,214,147,196,171,247,163,7,129,233,
142,8,92,242,69,115,58,191,142,226,210,109,116,141,12,201,108,196,72,150,247,26,128,132,43,166,19,113,61,241,144,70,9,227,167,60,226,185,212,238,149,193,187,30,173,172,234,102,13,3,82,142,43,17,4,104,
60,217,93,66,150,35,118,114,202,64,227,142,48,122,254,53,157,224,193,171,248,147,73,135,83,188,141,99,134,224,49,181,139,205,249,25,1,192,124,15,239,116,24,233,94,99,63,142,116,125,90,99,44,150,151,82,
218,2,34,138,9,163,141,67,21,221,201,249,128,118,231,167,78,57,233,93,117,143,196,57,52,237,4,189,187,90,170,91,198,168,45,103,93,242,46,91,144,73,33,99,39,140,12,96,114,73,172,21,69,207,203,33,57,171,
158,143,226,56,238,19,69,41,111,35,220,207,11,203,25,137,110,28,51,72,160,16,184,45,194,231,249,26,243,143,15,106,250,213,196,90,22,137,60,109,107,13,181,233,121,9,151,205,68,242,242,24,238,200,218,67,
129,199,251,64,142,149,196,235,191,21,53,155,141,114,222,216,60,119,45,60,140,210,189,169,124,40,7,104,0,47,223,57,254,45,216,3,159,74,225,46,252,79,172,47,136,245,173,59,82,212,174,174,173,94,105,32,
157,110,36,72,226,71,127,152,168,35,25,2,50,188,157,205,151,60,158,181,191,63,189,160,181,155,209,30,163,174,90,13,124,73,253,173,171,196,239,30,174,215,43,177,131,188,68,2,185,101,57,33,132,75,147,131,
158,83,3,181,119,218,159,142,252,57,164,88,70,144,94,79,59,68,64,85,84,81,184,133,0,18,113,199,244,175,143,245,141,107,92,211,35,241,27,218,95,37,148,150,145,91,71,11,35,6,242,88,200,119,194,165,193,39,
40,252,3,221,115,233,94,81,175,107,119,154,220,162,75,211,31,154,55,110,242,151,203,86,7,110,70,208,113,219,165,111,8,117,34,246,216,247,159,141,159,23,238,117,235,179,111,166,204,226,208,16,92,121,135,
15,254,254,15,233,94,65,62,181,119,122,209,199,117,115,42,219,2,85,165,136,101,219,35,146,160,156,1,232,15,94,50,107,10,213,21,224,37,198,20,159,189,191,4,250,143,122,146,50,134,55,130,3,27,203,191,31,
41,201,4,118,255,0,235,122,214,201,217,89,11,125,206,203,196,103,75,191,215,46,23,195,240,145,166,220,172,109,111,20,170,193,162,27,70,224,197,201,35,230,12,115,239,142,130,164,211,52,88,227,72,229,120,
158,64,224,108,88,19,118,246,61,20,99,191,127,165,103,89,220,105,246,58,43,169,130,41,181,57,176,11,57,57,136,12,130,191,137,198,65,244,163,194,151,250,149,132,201,127,30,162,208,97,207,146,209,202,99,
216,71,202,206,2,247,219,199,174,62,149,156,46,194,104,189,63,132,111,180,221,72,92,120,147,79,189,178,182,12,187,80,32,243,25,155,230,81,128,79,37,67,55,62,156,224,241,88,55,49,25,229,186,190,142,7,134,
41,254,88,183,119,219,158,199,146,7,95,67,214,172,107,58,189,245,237,197,204,173,121,121,40,152,156,205,115,41,105,28,1,130,119,103,36,159,92,231,29,107,62,243,88,213,14,156,44,110,103,87,133,229,18,100,
40,222,10,174,208,55,1,144,49,219,165,9,72,90,34,117,186,119,75,133,143,2,24,206,246,131,121,244,219,198,122,255,0,62,107,161,240,70,188,254,24,190,93,98,16,134,252,13,144,198,223,242,205,88,114,196,114,
58,113,142,189,178,6,77,112,73,12,239,40,150,41,23,57,224,15,95,106,117,223,157,21,204,223,106,97,246,144,231,113,13,144,8,3,156,247,244,171,182,150,11,157,159,139,231,181,156,127,104,192,20,189,207,88,
208,96,69,238,121,229,207,82,71,29,189,235,156,178,212,174,96,134,120,80,168,251,68,102,55,37,119,22,67,140,131,158,220,14,152,62,245,150,183,187,191,214,43,54,78,238,14,57,173,141,41,36,88,86,9,139,188,
51,74,165,163,137,190,119,227,129,211,35,191,231,74,201,32,221,157,111,135,181,104,180,195,28,208,89,233,195,80,150,81,35,155,155,183,143,123,1,202,132,229,31,131,143,152,96,100,99,218,13,3,86,111,14,
199,113,121,166,77,173,104,247,242,143,46,24,173,5,188,246,193,64,232,198,76,185,250,237,173,2,235,165,165,181,196,26,26,93,253,138,15,52,73,44,133,124,173,237,141,216,24,39,7,118,84,245,39,60,10,220,
240,227,232,58,245,162,190,169,107,255,0,19,64,151,22,205,105,111,11,47,219,29,151,204,138,80,202,78,214,69,86,92,183,202,74,243,92,53,95,91,93,117,53,229,109,45,72,124,49,168,73,119,164,75,15,136,180,
73,47,116,187,252,145,120,154,186,91,128,88,29,172,96,144,249,78,58,117,193,199,78,128,87,7,226,37,131,205,185,181,188,183,145,239,45,167,88,164,149,110,73,85,69,31,42,34,182,237,128,100,149,0,224,3,143,
122,200,150,208,91,235,45,113,61,148,34,0,225,153,109,165,70,44,188,3,134,39,63,54,51,207,35,118,59,10,149,231,17,218,24,225,177,77,60,204,27,112,142,87,32,199,159,149,93,72,198,112,70,27,248,128,207,
20,213,53,205,205,7,191,111,248,114,27,111,67,58,229,227,180,157,86,198,105,90,40,27,124,114,74,171,212,143,152,28,1,193,161,47,29,236,163,178,75,117,66,146,23,128,198,62,108,177,207,151,254,208,201,56,
239,206,41,233,13,201,178,123,208,159,232,209,176,67,41,193,27,152,100,41,25,207,32,19,210,146,223,119,218,174,85,252,184,220,141,129,153,118,172,108,72,0,227,168,252,63,74,222,218,106,75,61,19,225,36,
246,112,205,171,232,250,189,170,193,115,169,45,171,90,94,73,106,75,67,45,173,193,158,72,214,92,126,237,154,61,192,227,39,229,42,113,198,124,193,100,185,184,134,57,153,90,105,46,31,124,143,176,157,206,
223,57,27,186,103,230,39,29,113,205,108,13,74,56,116,63,236,235,75,137,75,75,120,12,240,204,197,163,115,131,135,10,78,0,201,25,61,115,201,56,171,186,86,162,250,29,151,216,154,215,77,212,149,110,76,146,
91,106,80,27,136,99,125,161,115,195,128,114,20,13,195,156,15,76,82,189,175,166,162,178,91,151,116,47,12,106,83,120,70,95,22,199,47,151,166,88,234,177,89,74,248,249,162,119,76,164,128,231,35,230,101,67,
142,70,245,108,128,115,89,144,188,179,121,101,95,108,179,0,133,64,4,18,190,221,15,31,227,94,221,168,120,21,245,15,133,222,32,213,188,35,51,195,98,154,124,58,143,136,180,187,155,51,255,0,18,251,118,51,
52,55,86,195,59,252,181,41,32,100,37,153,48,27,144,0,175,15,209,86,119,182,199,153,246,105,109,138,205,34,150,3,231,15,242,5,31,199,248,112,58,242,13,96,170,70,172,121,227,208,114,131,137,211,248,87,196,
19,218,252,63,241,47,135,110,117,86,139,70,214,60,185,118,198,89,146,75,251,118,18,91,51,46,48,0,5,209,219,176,101,201,224,87,65,240,126,241,245,239,22,71,224,189,83,85,159,76,93,94,225,99,211,53,24,72,
6,195,82,225,45,228,97,156,52,110,113,27,142,224,131,234,107,204,188,65,116,183,6,210,104,174,227,97,112,179,60,208,198,54,249,50,153,112,126,94,128,56,10,195,212,15,106,207,177,145,98,104,204,68,70,234,
66,163,110,43,177,143,10,192,142,70,211,181,178,8,228,10,214,84,212,227,239,9,179,232,95,31,232,151,154,85,200,212,47,244,59,137,60,64,247,115,104,186,214,148,136,82,31,181,195,184,184,103,69,27,101,206,
103,78,190,98,54,252,103,53,227,95,107,130,206,226,101,182,191,14,208,144,139,115,17,242,140,217,63,242,205,91,238,46,122,142,9,25,244,25,250,39,227,23,143,180,45,103,76,210,238,173,116,105,109,188,65,
127,225,225,101,169,217,173,204,132,205,172,218,19,28,119,145,178,99,205,114,190,100,115,51,245,82,1,39,173,124,193,170,253,134,120,207,217,225,49,46,24,206,249,96,147,150,111,147,202,7,156,5,206,125,
249,28,26,152,211,73,249,20,237,125,2,102,150,101,154,254,224,187,93,220,62,233,27,28,31,83,159,90,135,207,243,121,221,159,92,243,90,209,92,179,233,173,45,188,15,61,196,104,90,232,148,2,40,99,4,42,62,
23,182,49,184,158,132,231,56,57,172,168,161,189,212,245,40,237,44,97,121,239,101,108,8,173,225,105,28,156,18,0,68,5,143,0,158,157,1,61,141,46,89,54,238,69,141,93,47,79,190,189,25,180,176,187,157,1,249,
158,56,88,168,250,190,48,63,58,244,63,6,248,105,108,245,219,8,252,84,178,166,141,119,196,159,217,114,197,117,58,28,229,75,193,156,202,163,105,220,139,243,21,206,210,24,0,112,190,27,79,167,13,66,6,213,
210,91,221,24,105,210,202,246,242,200,68,114,76,62,85,12,1,31,41,220,91,229,33,142,192,65,224,169,215,248,96,116,235,125,101,52,139,249,45,45,244,219,185,77,172,151,50,202,34,88,157,153,85,110,29,128,
249,87,33,73,124,124,187,129,56,10,107,154,172,100,181,18,75,169,196,235,2,40,245,173,65,33,230,17,59,136,206,198,79,144,59,109,249,88,6,3,110,56,97,184,116,110,65,174,147,225,228,190,87,136,109,31,187,
100,117,235,84,190,36,233,87,26,23,142,117,221,26,233,38,142,107,27,249,160,117,153,113,32,42,71,94,79,80,65,206,72,96,67,2,67,3,81,120,78,115,111,172,218,74,0,59,92,123,215,169,27,184,26,117,63,66,254,
17,93,184,211,109,247,146,54,224,177,61,27,129,205,123,91,89,189,237,142,169,3,55,238,245,13,50,107,114,80,225,179,176,168,199,191,34,190,111,248,85,120,195,70,27,72,227,238,243,142,220,10,250,127,195,
15,187,79,180,27,152,152,246,182,79,82,9,193,175,34,170,179,108,239,164,126,54,78,183,246,182,22,49,92,217,204,178,136,148,72,8,17,146,225,64,96,73,233,130,57,171,186,77,228,218,69,213,181,221,189,181,
188,26,179,171,50,75,35,18,176,41,227,112,221,193,124,116,199,76,215,109,241,193,46,188,61,241,31,196,86,80,216,218,69,44,26,181,220,33,227,76,50,160,157,153,48,14,75,49,66,50,72,0,19,197,121,117,253,
222,171,112,229,239,222,241,11,128,177,153,114,224,175,92,100,253,223,194,186,253,157,57,174,201,156,28,182,118,58,205,47,196,81,104,154,214,221,43,73,154,239,89,145,140,146,222,94,230,82,88,156,147,26,
54,84,30,126,241,246,235,89,30,43,212,37,213,245,25,111,53,8,245,89,117,153,14,90,107,219,166,148,169,61,118,130,48,7,24,194,224,12,1,216,10,167,225,237,51,196,51,221,48,210,81,249,27,68,145,202,166,48,
120,200,207,33,79,177,193,173,107,175,14,248,154,206,39,150,250,104,45,226,12,89,89,175,65,199,118,62,217,198,107,74,116,104,66,92,209,126,247,175,252,31,192,119,108,228,174,34,103,32,28,228,28,124,199,
147,78,145,17,45,208,51,108,25,193,59,120,31,227,90,122,150,149,169,25,21,231,138,121,87,98,172,115,121,138,200,195,182,221,153,39,146,122,243,147,244,172,169,238,174,37,83,111,52,224,34,157,174,136,156,
240,122,31,124,215,101,208,154,101,91,137,2,67,178,38,59,89,185,112,184,207,226,106,59,56,68,140,118,46,246,236,77,90,88,163,23,65,38,108,71,179,43,230,54,113,70,241,24,42,172,10,246,57,192,162,193,114,
95,178,254,233,164,121,70,84,31,148,14,149,84,220,125,156,43,68,129,134,70,70,57,111,199,181,50,89,55,140,51,28,30,131,24,231,233,81,200,37,222,168,1,62,128,10,118,11,151,165,191,184,184,181,120,164,41,
20,44,56,69,25,39,241,170,246,107,8,152,110,216,236,189,85,215,32,159,76,119,168,212,206,141,177,85,131,103,129,183,154,189,101,108,237,32,251,65,108,30,89,71,25,25,231,38,154,212,151,161,42,188,146,63,
200,190,99,142,20,142,72,250,87,75,160,94,219,199,169,67,113,171,219,201,53,156,32,143,179,91,75,229,183,76,96,49,228,12,128,79,173,86,185,158,206,206,219,135,88,194,116,43,206,125,171,157,155,81,121,
137,22,203,180,116,50,56,231,240,20,218,18,212,250,187,198,94,32,248,119,173,252,32,208,101,208,210,235,78,147,195,147,164,183,166,248,167,153,119,54,192,76,50,72,73,107,128,202,217,59,115,129,130,15,
21,197,222,252,65,189,212,62,19,60,17,104,90,101,189,223,246,194,139,237,78,205,37,251,92,8,165,39,141,128,3,203,40,126,88,88,146,165,136,76,142,107,207,190,18,105,154,230,185,123,168,233,250,18,218,92,
235,70,219,206,183,130,242,17,32,159,103,44,145,150,249,22,76,96,141,195,14,56,200,198,71,121,241,127,254,19,31,2,248,127,74,209,197,213,173,182,153,121,10,139,155,37,134,53,16,93,144,210,74,209,162,224,
109,110,112,126,97,148,199,94,107,14,77,84,82,54,186,146,244,61,63,192,222,37,182,213,190,18,47,159,122,182,207,19,199,99,228,95,41,182,121,21,83,106,180,82,183,44,84,16,9,24,201,36,140,96,131,79,197,
58,173,254,131,170,216,106,72,6,167,167,94,160,120,225,121,76,146,195,58,125,246,222,49,185,56,233,156,130,79,56,53,242,124,18,180,183,13,119,63,239,36,65,141,236,119,54,61,50,122,15,97,94,151,225,217,
159,93,240,213,247,157,168,219,218,127,103,42,75,5,162,164,129,36,141,129,12,76,139,156,21,218,1,92,116,96,114,43,150,120,26,92,206,79,91,144,161,22,236,209,234,222,31,241,28,222,45,68,151,78,139,79,139,
85,88,152,199,166,56,120,4,219,85,129,182,59,191,2,14,14,48,59,87,17,115,241,10,217,172,36,251,101,166,10,128,183,75,25,83,36,50,3,180,70,202,9,56,206,87,112,24,224,244,172,207,9,105,246,222,41,177,54,
90,84,207,22,169,105,109,35,197,116,207,181,189,36,222,216,202,169,206,70,55,100,47,227,94,75,127,111,123,167,71,46,149,36,176,203,21,204,139,50,60,82,9,21,202,156,7,89,59,41,207,36,245,28,246,38,174,
157,24,109,21,107,22,164,226,180,62,151,240,199,140,108,90,45,46,235,74,184,130,91,45,203,246,139,9,173,229,136,195,46,211,186,61,224,18,224,131,195,41,35,230,228,117,168,45,252,73,160,29,74,249,172,180,
187,109,39,206,113,31,252,75,217,2,9,135,241,205,24,228,187,18,128,149,81,144,1,233,211,201,207,132,108,173,116,75,56,47,60,71,1,213,239,240,214,150,54,131,116,146,72,78,213,82,15,240,182,8,18,96,97,240,
184,231,155,186,71,128,60,88,254,18,185,187,181,178,215,86,107,11,129,29,224,150,211,203,72,16,168,113,34,115,230,182,27,130,84,30,112,114,1,171,112,131,86,108,137,78,235,204,212,190,248,77,51,239,58,
39,139,108,175,48,1,221,47,135,245,43,36,108,231,3,116,145,245,227,145,142,43,207,110,244,171,143,15,107,80,95,77,23,218,45,109,188,201,22,100,31,38,244,86,86,82,8,7,134,201,206,0,224,119,172,168,254,
215,125,170,73,36,215,83,203,52,1,238,12,211,57,156,128,152,203,13,228,228,252,195,215,168,224,212,182,73,109,230,248,133,174,17,82,83,163,93,16,192,224,187,238,140,15,235,89,91,206,247,37,111,161,133,
178,226,220,24,11,182,240,170,24,6,225,142,1,4,213,155,153,30,215,207,177,146,218,54,148,101,100,89,83,44,140,71,127,127,175,28,213,107,209,59,220,188,108,140,85,242,8,233,184,122,123,113,93,39,140,117,
8,181,91,29,39,88,111,38,125,66,107,95,178,223,22,83,230,44,208,144,162,95,99,36,66,60,147,159,186,71,113,90,217,221,92,102,113,138,85,139,237,210,171,11,33,58,198,110,217,25,34,50,109,13,179,113,249,
119,99,157,164,231,28,227,28,215,89,162,248,122,238,226,210,211,196,55,182,17,201,225,127,183,253,158,226,228,221,27,123,120,156,134,218,147,72,185,146,220,49,43,181,202,237,60,12,224,154,167,224,47,20,
120,151,195,241,200,60,59,174,93,218,59,200,230,234,201,202,201,4,205,180,8,217,237,221,89,88,109,36,22,101,200,228,3,129,138,234,53,207,24,233,183,30,32,130,238,223,193,226,214,59,91,164,134,27,157,54,
230,226,194,59,200,213,114,33,120,128,104,185,109,196,108,25,192,81,176,128,115,148,162,249,149,191,175,235,250,66,178,44,234,254,19,130,41,237,86,41,102,185,181,187,70,154,202,254,113,178,91,248,64,27,
228,145,119,16,166,60,240,192,145,63,202,202,193,51,88,190,6,182,70,146,57,154,32,98,185,115,243,48,229,151,39,110,71,110,152,63,81,93,134,159,44,126,36,210,117,189,67,75,211,53,88,45,245,64,150,209,67,
113,165,74,208,64,136,196,171,91,94,65,152,226,96,236,236,233,176,70,253,8,4,147,83,248,7,195,215,151,154,206,139,103,28,5,141,226,74,246,219,215,96,154,48,29,178,9,192,35,104,7,32,159,173,121,216,233,
242,81,147,151,245,253,127,93,131,146,239,67,182,208,52,117,189,159,76,150,234,212,61,172,246,204,173,22,242,160,92,5,98,128,133,24,104,240,3,121,109,158,64,61,69,119,55,218,183,134,244,141,51,86,183,
210,166,91,132,209,180,227,171,139,88,8,34,38,141,148,202,177,147,208,73,9,113,183,37,85,227,24,3,156,241,218,239,141,180,38,208,108,124,27,225,203,149,191,190,157,196,247,122,164,3,116,16,108,251,207,
191,251,139,194,150,28,14,6,115,197,113,30,7,186,240,198,143,20,23,26,76,122,134,165,175,216,198,237,125,119,41,13,4,209,51,180,97,34,141,72,37,112,204,50,195,119,78,216,199,12,105,202,73,74,170,118,93,
59,167,220,87,177,236,191,13,188,121,225,40,44,181,166,241,238,145,103,226,3,168,238,191,180,150,222,15,34,242,8,6,23,116,50,161,92,225,72,102,143,253,225,158,153,229,252,75,161,105,215,218,119,141,163,
209,108,69,245,230,173,13,180,48,79,28,69,222,40,146,83,36,108,7,84,71,36,33,144,0,202,1,234,121,62,97,161,13,25,236,172,99,123,171,217,237,116,98,194,41,45,209,150,85,39,115,41,10,70,3,50,140,148,3,147,
199,124,87,211,30,32,212,237,254,28,252,33,178,243,34,135,71,241,22,168,175,229,88,89,206,146,155,40,1,9,25,115,202,203,48,192,114,115,181,78,197,31,42,146,93,72,123,38,185,85,154,118,190,157,244,90,111,
223,82,212,46,174,217,243,6,163,172,183,133,117,171,189,3,64,146,197,117,57,165,205,212,210,93,27,145,12,170,172,160,110,27,82,57,134,60,178,170,31,60,6,36,17,141,29,55,70,212,188,59,168,232,250,222,171,
127,107,115,230,180,87,118,215,54,199,207,71,87,115,251,192,73,228,43,128,73,101,234,71,3,138,203,210,124,55,167,60,3,84,183,184,146,43,123,56,129,186,213,39,105,39,89,89,153,149,174,89,7,38,50,27,230,
57,192,229,136,228,154,221,54,48,120,126,211,254,17,253,118,206,222,251,74,218,108,90,104,75,179,90,45,201,202,76,174,188,152,101,42,8,39,229,14,157,65,24,174,138,179,162,223,37,221,250,233,175,201,255,
0,95,144,185,86,231,179,120,129,172,111,108,117,45,86,251,79,182,181,186,91,113,43,106,251,140,182,144,182,27,229,150,28,254,239,105,1,139,198,49,221,148,96,215,203,214,158,20,241,4,62,35,181,182,185,
181,102,109,64,3,109,115,19,137,160,187,71,59,68,177,74,167,107,175,35,166,8,238,23,138,245,187,157,46,233,188,25,170,11,27,43,201,164,212,244,247,142,116,80,2,204,156,174,239,47,112,206,237,167,5,7,60,
224,247,174,75,224,224,215,108,228,187,177,180,26,156,26,116,228,190,102,112,209,52,138,9,220,138,185,8,219,128,203,41,4,247,205,116,229,149,227,38,212,37,126,150,125,63,174,198,170,45,218,231,208,255,
0,9,126,28,234,86,122,62,185,170,77,121,109,13,238,167,109,228,194,25,67,45,179,7,141,142,223,87,3,130,189,129,207,215,132,107,248,53,41,47,100,70,182,158,218,84,145,110,149,194,112,98,198,229,46,57,56,
98,64,62,174,221,135,31,69,107,118,247,178,252,62,188,131,73,73,163,184,104,86,88,231,45,146,185,4,30,49,193,101,84,203,123,255,0,181,154,248,98,222,59,173,61,181,121,44,208,162,25,140,156,57,101,155,
36,179,147,187,239,118,232,58,116,226,187,49,43,159,70,205,90,74,200,236,53,117,185,191,241,45,255,0,217,18,219,105,146,39,95,51,0,228,34,2,160,231,230,249,87,60,114,113,92,46,191,127,171,89,120,131,94,
134,216,72,111,30,236,201,104,2,160,6,0,160,141,170,115,130,24,228,147,198,24,115,233,220,93,107,58,132,150,90,77,234,110,104,102,154,222,3,28,74,162,86,144,176,80,228,140,227,3,156,253,77,115,58,149,
179,88,234,246,7,139,173,65,103,251,61,209,183,147,229,87,66,85,203,147,243,109,59,122,99,7,142,198,185,41,190,93,100,140,229,23,99,54,49,168,221,95,197,2,189,172,86,209,160,121,228,119,193,222,236,118,
109,32,156,243,142,128,142,70,78,43,69,60,65,107,109,1,211,218,222,247,84,120,166,102,147,116,33,80,50,134,25,202,242,8,201,231,176,56,173,13,27,74,107,157,74,242,118,211,17,46,99,100,93,204,70,224,188,
129,133,36,128,122,116,4,224,143,76,86,173,168,211,116,203,168,69,244,75,228,198,75,9,190,214,144,160,98,70,208,228,242,73,36,14,152,57,29,233,185,38,149,149,194,54,138,212,243,196,214,181,145,122,110,
236,236,34,209,150,108,127,164,67,9,150,72,212,100,170,43,48,194,0,73,224,174,14,79,28,215,59,30,155,171,92,107,82,90,77,61,211,75,130,37,45,41,5,134,14,58,251,117,39,167,173,122,174,189,38,153,121,127,
111,25,134,9,33,89,89,239,29,216,199,28,72,172,8,108,147,146,112,192,140,30,198,184,141,86,252,199,117,121,54,151,117,36,145,177,34,220,200,85,132,202,88,151,14,49,145,132,251,184,255,0,129,98,186,105,
212,147,217,106,58,158,238,137,156,108,141,119,246,29,106,234,244,230,25,163,49,226,70,0,203,54,224,16,128,58,237,32,18,120,227,145,156,214,27,20,158,231,34,34,93,155,0,3,128,7,166,63,173,106,207,45,221,
218,249,166,213,150,221,73,42,168,167,97,110,228,14,213,70,15,53,110,67,121,101,223,120,44,185,193,63,143,97,93,145,191,83,2,221,128,141,188,179,58,74,144,110,33,164,93,172,72,199,240,143,95,173,109,54,
168,210,193,167,194,82,20,142,207,97,140,20,25,59,78,87,36,114,72,60,243,85,26,222,88,210,43,63,177,121,114,135,39,115,54,89,183,115,252,184,168,37,255,0,71,70,117,218,101,231,24,24,199,165,49,164,106,
73,21,214,187,172,222,206,33,142,54,145,218,91,151,141,49,24,102,57,36,1,211,147,207,227,81,45,165,188,119,178,20,221,36,45,133,80,19,151,56,244,94,217,172,107,43,184,54,188,100,50,153,6,27,223,61,65,
62,158,213,161,19,204,101,108,201,35,91,198,163,124,99,41,244,193,56,60,83,74,218,9,145,222,128,34,104,140,123,24,2,57,199,24,245,172,25,101,254,22,111,149,56,24,174,175,92,58,124,186,118,200,174,17,175,
149,129,68,143,144,6,57,82,63,188,43,153,22,140,170,178,59,1,41,57,85,253,57,244,161,177,34,75,91,153,138,145,111,108,124,213,82,204,254,221,248,252,106,157,220,178,201,54,38,140,228,127,8,4,246,236,42,
216,146,107,81,143,157,92,14,54,245,246,175,76,240,47,133,245,29,84,219,106,186,79,246,22,181,171,42,9,110,180,121,245,0,151,142,114,55,58,194,251,1,237,134,71,249,79,56,200,193,137,73,69,57,50,162,174,
204,175,3,248,67,237,26,91,234,122,133,181,193,183,57,10,68,108,0,32,3,128,216,198,226,8,56,61,133,122,29,159,134,32,210,37,138,120,238,173,175,108,230,84,54,242,36,1,36,136,144,27,107,131,146,24,109,
192,32,144,123,122,86,135,138,175,87,195,118,211,93,217,90,235,186,29,212,178,44,119,186,61,244,34,246,198,65,142,90,57,240,178,96,28,103,206,25,57,33,90,184,29,123,80,210,181,61,18,93,96,195,45,147,218,
58,36,158,65,96,129,206,2,178,202,126,233,28,29,132,134,238,164,215,149,42,245,170,78,235,225,127,63,235,241,53,180,98,122,173,169,240,50,218,92,166,163,36,145,69,170,91,188,23,114,79,10,155,205,60,200,
54,172,182,205,141,147,67,145,251,200,79,36,124,202,65,81,159,20,182,212,47,252,41,226,216,47,172,231,182,140,216,220,75,3,189,186,239,85,44,118,51,97,179,144,200,67,128,115,133,124,117,166,248,183,86,
186,214,180,232,239,239,245,248,175,239,131,152,220,52,10,151,1,87,59,89,182,224,21,57,198,64,224,140,30,121,56,41,173,93,159,15,95,104,178,5,154,198,121,86,229,203,229,153,29,23,102,229,254,239,238,201,
70,245,82,58,99,39,108,61,41,36,249,158,172,85,42,169,61,52,69,77,239,167,234,205,37,180,34,51,28,196,7,140,5,0,131,132,192,35,158,15,67,156,85,141,78,103,184,142,9,22,121,36,183,202,196,162,87,59,144,
5,200,7,251,202,184,198,238,153,226,179,181,29,72,221,93,218,78,240,70,147,198,144,171,237,201,243,30,62,60,198,207,241,48,11,144,56,249,105,103,185,251,66,136,94,66,241,164,210,202,188,12,97,206,88,12,
116,203,12,254,28,10,236,73,164,174,103,113,52,219,104,174,181,91,56,148,69,243,220,195,25,60,2,67,74,170,70,125,57,53,119,196,54,233,22,177,226,40,161,40,169,21,244,169,28,97,179,133,23,44,184,207,160,
92,99,233,91,126,21,177,211,91,195,58,230,161,116,233,26,41,182,183,12,209,25,36,142,41,37,196,179,12,3,183,0,0,189,24,176,32,103,165,98,105,182,208,223,234,223,103,182,121,28,77,56,142,41,26,50,187,153,
152,133,249,70,79,63,46,7,82,78,61,169,57,89,93,160,182,134,19,43,36,188,255,0,22,113,253,42,214,155,187,203,145,217,64,181,95,146,83,142,62,97,247,115,238,5,105,120,195,76,188,209,124,77,171,105,23,209,
164,122,165,157,204,145,78,138,85,145,25,113,156,109,36,119,7,25,239,142,185,170,82,0,241,253,158,41,18,40,166,144,110,243,91,10,167,24,5,143,245,199,122,28,185,151,169,54,62,187,248,13,175,165,141,223,
133,181,40,245,25,236,31,93,240,148,154,45,213,222,253,194,202,120,136,9,116,80,130,172,209,205,176,149,110,177,188,164,228,41,7,142,248,173,240,107,92,177,248,127,111,241,22,61,0,71,167,132,83,171,219,
233,208,150,180,176,98,6,37,136,243,136,78,113,130,112,156,244,0,215,23,240,26,226,43,237,158,26,187,107,136,238,211,80,55,154,100,209,159,245,87,95,101,156,170,176,254,227,52,24,96,62,240,144,142,153,
53,246,13,239,136,245,93,7,246,123,241,78,189,103,117,37,223,133,245,173,42,206,121,84,91,150,128,233,183,141,246,105,143,150,224,134,40,251,153,129,233,144,15,92,215,156,161,82,21,148,109,101,119,170,
251,237,233,111,197,26,43,218,199,231,197,220,27,220,165,171,101,142,64,86,93,173,145,201,199,174,49,250,86,43,187,253,161,163,242,74,169,236,1,192,226,186,221,122,206,218,203,197,23,86,113,221,221,127,
101,199,33,125,51,80,186,181,120,158,234,208,147,228,76,200,64,108,58,96,147,235,187,168,6,178,196,240,77,56,142,254,21,22,241,146,209,189,184,13,36,64,12,42,171,227,230,136,30,118,99,39,156,16,112,43,
210,78,218,50,25,167,13,207,145,22,157,50,219,207,21,195,75,29,200,129,230,44,110,227,35,4,150,110,88,54,28,142,112,49,223,145,92,188,14,100,139,110,231,120,20,150,137,51,144,188,96,1,248,113,197,117,
62,38,54,162,211,64,178,154,100,191,254,204,182,146,214,226,91,103,196,83,194,100,119,183,49,201,140,240,178,48,57,25,24,193,245,172,125,58,206,226,234,250,207,78,128,160,154,240,168,82,56,245,36,254,
65,142,61,128,169,186,72,77,244,103,77,96,78,151,225,125,39,196,22,154,148,19,220,45,236,214,242,217,34,21,82,155,72,17,187,17,130,36,93,207,208,228,99,142,245,234,191,7,124,95,167,199,224,61,99,194,182,
154,34,220,234,250,76,235,169,248,115,93,182,70,143,81,134,89,8,13,108,96,198,233,226,221,187,112,70,222,168,55,70,25,176,71,146,91,129,111,253,129,107,169,200,27,79,146,213,102,137,109,167,223,181,36,
119,120,145,179,254,173,129,24,97,201,80,195,156,18,42,127,12,93,203,225,143,136,62,31,213,44,124,194,209,136,174,33,121,99,0,196,72,59,153,119,101,88,160,201,222,191,47,4,143,186,69,40,164,155,86,26,
103,162,104,76,222,34,182,77,55,195,218,69,189,175,136,117,73,224,181,213,237,231,204,113,207,34,153,102,142,103,217,135,138,83,133,141,143,10,225,200,96,9,227,158,242,44,212,235,58,111,147,117,97,170,
54,160,139,30,139,168,219,73,37,197,161,88,215,247,166,231,5,100,71,12,200,200,64,59,89,28,103,27,141,255,0,19,233,154,238,139,175,120,115,196,86,179,90,92,106,240,94,61,164,147,91,76,174,46,53,6,121,
39,108,40,43,190,22,44,93,24,127,22,229,200,192,174,151,88,142,111,136,26,86,143,226,41,60,73,169,77,241,62,221,210,194,250,218,230,37,137,213,142,252,35,4,28,64,138,206,86,94,112,27,15,146,170,181,156,
226,172,239,215,250,184,114,223,70,121,7,138,30,75,139,211,113,41,155,207,145,134,255,0,53,203,190,66,133,193,36,146,112,20,40,244,0,14,130,161,210,216,69,36,109,232,227,57,250,213,191,20,90,222,89,106,
50,105,250,149,171,91,106,150,146,180,23,145,185,201,50,175,5,241,216,48,195,15,80,192,142,181,153,25,33,122,140,123,87,69,4,213,52,152,218,177,246,215,193,217,227,155,76,178,103,109,202,6,252,3,215,143,
214,190,188,248,123,182,109,14,50,184,44,201,193,245,231,138,248,119,246,124,186,103,176,181,86,255,0,86,0,0,3,211,182,63,58,251,71,225,173,202,164,101,93,206,25,130,224,177,227,208,98,188,250,235,86,
142,202,47,68,124,19,251,71,104,218,69,151,237,204,44,188,73,246,164,240,222,187,169,233,243,94,181,172,222,84,145,36,233,228,51,163,142,70,215,141,91,35,208,250,215,149,248,62,214,77,67,226,73,240,213,
213,246,153,121,104,117,57,172,86,227,87,184,242,97,145,146,98,145,151,151,12,177,150,218,57,43,180,150,3,43,154,250,7,254,10,45,225,203,19,241,135,194,26,197,213,243,90,91,94,232,242,137,230,64,28,230,
222,112,87,98,159,188,195,237,7,129,201,236,9,2,188,167,225,134,133,225,29,46,251,68,214,116,207,21,195,127,115,112,200,186,181,181,239,135,164,123,108,73,159,54,219,230,101,45,27,3,177,220,134,83,184,
58,21,216,64,213,168,78,146,111,177,133,85,106,140,246,109,115,195,218,61,198,141,55,134,53,109,14,202,221,124,198,178,178,212,236,22,56,230,178,157,57,146,2,87,128,202,192,101,78,85,212,100,110,92,215,
201,39,69,210,97,213,174,76,255,0,16,244,205,59,85,142,86,71,99,97,117,16,119,67,181,134,244,24,60,131,199,243,175,162,60,77,119,225,31,2,248,219,196,17,104,254,49,127,17,232,90,204,68,93,233,82,171,199,
113,106,171,242,198,147,220,239,96,215,17,50,0,140,6,253,187,28,51,2,115,201,104,255,0,13,109,181,95,12,60,58,78,166,208,166,181,110,110,173,154,72,80,92,106,100,16,204,76,143,144,99,12,65,219,24,64,114,
79,59,171,131,235,11,14,218,148,154,219,84,150,222,110,204,222,157,63,107,189,175,249,158,100,186,37,132,9,12,199,196,158,16,186,185,145,246,153,180,173,76,165,195,122,19,28,145,42,147,245,231,174,43,
207,103,146,70,158,71,149,151,206,36,137,60,161,128,79,127,207,175,227,93,183,140,124,51,38,131,226,1,104,151,55,246,242,161,12,109,238,173,39,183,150,14,192,230,78,92,19,156,48,227,183,53,204,45,152,
123,136,225,121,183,34,0,138,219,70,17,71,110,43,220,195,222,80,82,78,233,156,213,90,82,181,172,204,163,28,132,171,36,71,111,76,227,165,77,37,172,135,110,93,65,207,126,181,126,246,94,150,182,203,188,116,
227,171,125,125,41,171,100,227,62,99,115,198,21,121,252,235,103,100,102,45,172,54,205,40,218,129,164,35,106,144,216,0,251,214,166,158,218,125,148,138,230,212,94,221,168,27,209,219,204,139,223,32,224,126,
3,243,172,243,148,222,138,64,84,60,56,24,207,191,181,57,110,237,225,132,170,162,150,242,194,231,57,193,238,127,26,37,176,145,221,73,173,248,38,233,165,136,124,59,104,30,92,159,54,207,95,158,54,135,183,
201,25,12,133,70,1,218,120,206,70,113,201,225,245,187,139,21,212,24,105,41,121,29,178,146,140,183,147,36,165,136,238,25,64,249,79,161,230,178,238,239,165,85,217,8,216,174,57,114,48,72,246,170,81,238,108,
99,38,165,43,109,249,150,217,171,50,149,118,136,203,20,163,63,44,136,164,43,231,28,243,200,31,94,152,173,189,59,195,43,62,226,124,69,163,224,7,24,181,47,116,21,130,229,119,50,237,85,4,241,158,221,122,
87,53,20,13,41,82,91,167,169,171,143,168,69,9,88,237,140,178,72,173,157,194,77,168,62,152,228,251,231,252,105,187,247,17,209,93,197,39,135,181,59,121,244,153,230,212,81,37,4,126,225,246,146,6,78,243,17,
193,7,63,116,55,65,156,224,241,217,120,135,89,212,124,85,225,219,139,55,180,242,37,218,170,35,104,227,151,205,56,44,174,31,106,152,155,104,3,140,231,146,73,174,107,194,183,177,157,61,45,228,215,47,108,
164,148,249,75,109,103,22,29,139,18,114,28,252,155,65,227,7,146,15,7,35,5,218,189,215,136,199,135,229,186,188,135,83,188,179,177,184,142,54,186,185,156,21,130,119,36,133,218,14,114,217,250,122,99,165,
46,163,70,5,230,137,119,109,164,127,106,139,171,57,109,162,149,33,153,45,239,3,79,108,238,14,210,241,145,242,228,140,3,200,206,61,107,173,212,53,75,11,189,11,237,240,120,121,33,105,92,166,161,115,111,
108,60,150,156,141,199,12,173,185,78,14,64,60,97,177,147,192,174,123,64,241,85,197,183,159,27,233,186,116,144,92,219,139,107,161,52,37,222,120,243,157,185,200,217,158,71,0,227,57,170,90,155,196,145,234,
195,76,107,216,244,155,166,77,246,141,33,57,92,241,19,231,151,42,199,134,28,145,130,125,104,181,247,13,142,142,235,237,58,47,137,116,187,125,63,81,178,2,242,222,27,187,89,237,110,252,229,183,105,149,145,
91,118,1,70,7,32,175,56,7,156,231,2,121,188,37,174,223,233,215,218,157,238,159,61,131,45,251,217,221,220,169,197,178,128,187,248,32,237,115,184,140,162,146,253,15,0,226,184,68,123,205,70,250,230,251,201,
145,90,121,177,51,198,187,17,72,80,10,238,227,107,96,15,126,125,197,123,229,133,221,158,187,240,255,0,196,43,170,92,90,11,187,155,165,188,142,61,62,224,40,148,71,110,152,143,247,152,34,118,84,61,20,16,
8,195,14,163,41,115,45,137,125,17,103,225,103,131,215,70,209,228,188,188,214,116,60,160,143,237,22,122,165,188,176,27,112,65,217,39,154,119,161,80,207,180,149,81,128,119,100,98,173,107,159,19,36,210,53,
5,130,73,110,6,169,166,72,241,249,214,177,70,208,96,48,89,48,178,176,114,91,107,114,62,240,110,123,17,153,240,199,93,178,188,137,31,93,187,243,146,246,22,88,227,129,25,31,202,140,22,206,252,18,248,109,
159,48,61,79,34,188,239,198,86,254,26,212,117,217,238,180,251,141,79,77,123,136,254,209,139,164,23,11,56,40,8,111,221,224,198,73,4,127,18,140,86,84,211,115,124,250,151,202,138,218,91,193,175,232,105,166,
69,107,111,164,220,198,165,228,188,138,57,30,57,99,137,122,8,211,46,10,135,39,57,192,220,121,249,184,172,154,75,195,173,90,233,22,214,113,94,223,106,69,108,236,149,72,81,55,153,44,72,138,241,177,253,217,
222,71,222,36,225,186,117,175,171,102,248,73,175,120,99,246,109,180,240,54,135,5,141,183,197,15,136,10,151,190,32,23,215,162,25,172,180,222,77,181,140,109,131,135,115,134,145,113,201,103,29,49,143,43,
240,127,134,109,52,79,218,7,74,213,126,32,173,215,135,244,253,54,254,199,80,186,105,237,25,162,127,40,197,185,11,169,194,141,241,253,240,72,4,97,176,14,107,145,98,41,70,163,135,55,227,175,249,135,178,
151,50,211,115,199,181,221,58,219,75,147,88,210,110,224,186,147,86,182,150,104,17,203,70,209,164,177,204,209,236,114,0,7,144,65,35,130,121,224,98,177,230,240,252,242,120,82,125,114,41,237,165,72,111,26,
214,234,214,45,198,91,126,50,146,48,233,229,185,220,128,231,239,130,184,38,189,127,196,30,27,212,103,214,124,74,215,186,114,217,234,150,122,212,178,203,35,202,153,130,222,121,158,120,164,216,51,230,2,
74,237,110,83,5,135,80,69,69,224,133,183,210,46,238,166,251,76,87,30,28,241,45,148,186,110,175,99,229,28,200,204,251,140,138,9,1,76,114,132,116,35,149,57,10,9,106,209,98,169,69,234,239,215,191,167,165,
196,160,238,113,159,13,238,236,236,116,237,66,235,84,213,109,37,137,213,129,211,36,146,71,158,86,225,75,252,184,49,55,150,24,44,165,142,221,253,15,74,235,108,126,34,195,165,105,215,30,29,240,213,190,156,
150,55,183,128,218,220,127,96,172,151,144,150,112,34,86,184,185,98,78,192,85,119,136,242,122,158,245,230,250,236,49,90,234,50,61,230,157,119,102,226,86,223,109,44,172,66,191,57,5,88,43,0,14,56,97,154,
77,60,66,46,173,101,183,97,37,186,204,141,181,209,86,69,112,84,177,247,0,228,143,81,138,167,8,213,92,210,213,124,137,107,91,92,235,252,81,241,63,198,119,122,204,215,35,89,188,243,97,6,54,105,239,119,49,
33,142,70,216,214,56,227,231,140,4,63,165,122,98,104,235,164,234,186,117,214,185,122,70,149,105,224,123,72,100,220,242,74,139,231,201,36,115,75,141,199,99,52,118,210,103,24,228,130,113,138,243,111,138,
154,124,167,226,141,158,145,18,1,13,204,86,14,232,167,135,80,204,64,97,234,0,111,204,215,67,127,171,235,62,47,179,139,195,90,36,240,164,58,136,41,113,121,60,130,40,35,132,6,243,18,73,10,146,136,11,57,
206,50,75,13,185,82,69,99,36,172,148,85,147,223,250,251,194,43,91,24,190,43,214,52,205,47,90,178,139,68,51,29,47,84,138,29,78,254,68,35,207,189,89,68,140,177,147,159,148,35,28,4,232,8,228,147,91,95,10,
116,11,185,252,55,172,106,122,190,173,99,163,233,179,233,194,222,11,153,226,5,190,82,165,166,117,70,45,30,54,144,119,1,184,158,219,121,233,180,15,134,30,4,240,251,219,75,226,31,23,203,173,248,158,38,142,
81,166,232,239,28,86,225,247,6,141,26,226,69,44,121,92,228,4,206,58,96,26,229,252,93,241,26,11,13,78,233,60,47,224,79,8,233,205,59,200,102,189,9,253,162,247,76,236,29,191,122,251,58,54,9,192,43,145,193,
194,214,60,222,218,46,157,20,223,155,255,0,131,254,77,20,224,151,196,98,235,87,54,222,26,241,21,189,229,182,191,161,120,128,75,114,46,67,105,115,52,155,83,141,168,204,64,216,114,8,217,130,121,7,185,21,
151,226,141,87,196,215,254,38,212,180,205,87,74,55,142,97,123,100,180,37,201,182,0,6,83,4,131,36,50,22,25,192,59,137,32,142,69,77,161,105,118,23,43,54,163,226,123,251,217,53,91,134,99,4,48,224,200,89,
160,221,4,164,145,183,230,152,128,85,136,92,99,35,154,209,241,229,197,142,159,241,2,43,190,46,172,222,238,75,219,115,246,135,138,91,67,182,16,238,121,229,78,198,98,51,207,35,216,235,26,80,231,73,235,166,
254,106,222,158,125,1,71,75,244,57,255,0,134,83,67,105,226,97,118,218,149,212,82,90,69,51,183,144,222,82,144,170,67,6,86,254,29,216,4,30,15,25,175,162,254,18,120,147,76,189,240,126,167,161,93,221,203,
45,209,210,205,132,22,42,87,207,183,180,72,138,249,27,212,229,134,83,124,114,228,50,145,134,239,159,152,109,205,244,154,149,219,233,237,37,205,205,232,150,221,162,242,185,117,121,60,194,161,72,232,66,
142,58,128,49,237,94,187,240,71,89,157,109,62,192,250,112,93,83,77,241,28,58,189,188,207,106,22,73,84,218,77,111,115,15,204,58,132,140,73,179,169,32,240,49,89,99,112,148,235,65,185,61,181,94,191,215,235,
230,107,66,163,140,172,141,241,227,203,3,225,127,12,71,171,2,222,25,75,103,181,187,123,121,2,92,217,219,255,0,4,209,68,59,229,145,192,36,143,221,184,0,146,40,248,95,253,161,255,0,9,237,213,150,187,122,
36,212,162,125,159,218,16,160,79,58,16,9,73,149,212,124,234,64,141,134,119,17,191,4,146,13,101,248,71,72,211,60,115,225,37,183,240,195,143,237,166,158,59,75,187,89,65,219,113,20,178,98,18,128,242,187,
242,188,142,140,24,103,189,110,120,99,73,135,225,254,177,125,164,60,241,234,237,166,94,77,107,122,182,69,231,75,67,189,87,99,200,132,40,140,50,149,39,128,161,14,112,115,131,7,44,61,42,146,82,92,178,244,
239,215,202,255,0,143,200,190,121,181,171,62,186,212,181,27,157,59,195,183,242,189,180,86,80,91,69,42,201,1,24,73,31,201,35,104,32,146,219,89,64,45,211,32,126,31,159,58,94,179,121,112,140,247,81,173,180,
226,17,108,110,95,12,230,225,118,162,227,110,20,5,33,195,96,1,207,21,245,215,198,141,106,125,47,225,158,189,224,219,136,13,167,136,175,180,173,137,16,187,141,166,137,100,140,157,206,64,219,134,243,9,222,
164,231,140,19,95,35,234,105,98,103,150,40,101,43,45,216,127,61,34,182,222,164,158,119,2,1,4,135,249,206,70,9,200,198,77,116,206,181,41,187,167,114,102,209,211,92,220,75,107,173,105,243,27,219,159,38,
234,17,35,189,176,219,22,85,114,27,4,16,152,42,1,29,195,19,158,49,87,53,187,134,154,246,202,214,25,39,55,174,55,186,44,99,247,129,134,115,242,140,130,14,114,112,64,201,198,107,146,125,54,235,202,23,218,
84,87,81,198,78,223,33,8,31,49,80,29,112,3,101,14,55,114,51,146,120,29,43,67,66,186,213,127,180,210,61,66,43,95,181,206,141,111,20,194,112,183,17,100,229,90,84,218,55,70,0,8,79,92,16,64,192,174,103,56,
77,90,50,87,5,9,218,246,52,38,179,251,68,214,207,106,247,81,88,125,164,37,141,220,10,129,220,3,135,96,196,100,49,36,21,199,83,207,122,219,109,47,84,142,222,123,189,111,73,89,214,193,156,172,132,171,207,
29,180,169,243,252,165,137,57,198,114,57,234,49,129,154,143,74,26,166,171,226,37,222,150,119,47,108,222,117,252,51,90,145,14,67,21,11,12,202,191,50,227,105,220,59,146,56,198,42,183,197,95,21,105,208,139,
237,58,7,188,138,232,52,126,80,184,77,145,42,136,182,252,133,127,132,22,57,62,224,123,85,82,162,234,89,63,192,202,218,94,74,204,243,159,29,190,163,52,182,235,35,33,138,232,110,180,128,130,174,177,15,148,
111,32,252,196,227,25,227,56,246,174,94,246,246,125,62,202,43,43,151,50,42,57,116,137,161,10,241,146,57,4,129,147,159,175,21,209,92,120,143,83,93,62,119,180,243,33,51,63,151,60,209,34,159,53,49,196,101,
216,112,56,7,3,29,50,58,154,227,181,153,228,153,161,18,200,73,219,201,200,56,25,232,72,31,206,189,40,83,81,208,87,45,71,119,44,241,9,119,152,237,226,195,71,30,66,177,250,123,126,102,167,210,45,46,101,
188,89,110,98,149,16,48,49,159,44,237,39,223,215,210,183,124,39,47,246,102,133,168,220,136,173,36,146,230,220,125,157,110,98,86,218,192,252,175,147,202,114,123,117,197,103,106,154,141,205,219,196,103,
121,164,134,84,194,130,10,170,142,228,28,0,122,118,53,74,253,22,131,178,91,179,164,131,193,122,166,169,108,117,8,190,203,2,1,177,227,50,170,130,234,1,125,184,206,122,140,251,154,226,245,219,53,183,189,
142,47,183,44,243,237,30,96,64,72,67,232,27,161,250,214,187,107,87,177,79,53,253,184,153,37,145,60,144,97,66,17,120,192,45,145,140,224,103,142,77,114,55,31,235,164,145,228,144,78,217,220,234,217,45,158,
212,146,151,54,172,61,219,17,160,150,25,1,182,226,114,126,98,84,49,206,120,199,165,58,236,205,52,47,37,196,206,95,142,17,112,42,172,183,243,68,159,185,84,69,236,27,156,125,106,148,87,206,138,224,237,114,
221,120,227,240,173,73,0,110,192,36,72,234,16,240,67,96,169,246,199,122,187,98,225,98,80,238,237,38,115,150,231,242,61,115,89,173,115,44,142,72,27,119,0,48,162,183,52,171,41,197,204,126,99,237,76,141,
172,8,200,61,186,254,180,1,210,120,111,66,181,190,146,218,123,237,98,59,70,99,28,225,27,77,146,247,204,5,129,10,209,198,114,65,29,70,15,7,7,21,237,90,191,135,244,98,109,86,218,231,225,170,221,220,199,
184,197,101,60,250,76,247,8,73,193,16,186,156,244,56,193,234,13,113,222,24,95,3,91,198,78,173,23,137,147,82,139,204,221,121,165,205,166,70,238,27,24,3,206,96,204,163,145,140,14,190,188,141,72,219,224,
197,248,91,59,157,79,226,69,187,5,8,147,92,193,164,75,28,100,114,51,180,238,252,121,174,12,67,231,124,186,233,233,253,124,141,86,136,200,186,188,154,206,242,231,77,155,95,22,86,238,171,229,216,107,23,
13,113,106,164,28,21,243,91,230,95,108,250,251,86,114,91,248,211,192,193,117,205,50,241,98,211,111,83,99,203,30,39,178,158,63,73,9,6,38,94,192,177,86,244,35,53,31,138,252,61,225,231,85,151,195,126,59,
176,212,35,146,79,150,13,70,120,109,46,162,4,224,6,10,204,25,179,216,5,10,59,156,86,62,157,226,15,16,120,34,237,14,149,226,139,187,73,126,242,29,11,82,89,160,126,49,251,192,112,140,57,254,233,172,233,
81,93,247,222,232,201,232,115,243,221,249,146,222,220,74,150,137,29,192,35,203,142,53,218,196,242,25,23,36,16,189,114,173,145,192,230,179,174,102,180,49,45,179,38,247,140,237,91,168,37,194,178,238,201,
24,192,200,42,74,243,200,61,51,80,234,250,181,238,161,127,121,127,121,115,246,139,219,230,47,112,205,16,82,167,61,126,80,23,45,236,58,117,193,170,48,186,187,162,150,117,119,36,59,156,96,46,59,119,255,
0,34,187,99,77,71,97,11,47,148,183,13,36,65,196,69,206,208,205,243,109,207,0,159,92,113,154,221,134,206,217,252,27,123,168,173,238,203,171,93,66,20,75,41,66,130,240,76,140,68,170,122,182,29,64,56,227,
28,247,21,204,229,222,62,48,27,28,103,177,173,11,241,12,183,141,228,35,37,170,29,176,249,172,25,213,49,192,102,29,79,95,210,169,171,245,30,219,157,159,135,181,187,195,240,247,94,208,44,237,172,163,176,
184,146,41,239,231,158,61,243,146,73,72,204,96,17,130,167,113,206,62,80,1,245,174,70,222,75,136,228,185,54,146,21,242,51,62,248,142,211,182,55,200,112,65,200,198,67,12,28,140,251,82,233,222,120,186,217,
108,229,16,148,36,238,11,184,175,161,110,3,28,145,131,193,206,15,21,211,219,37,172,246,62,36,104,222,217,110,37,72,110,98,133,1,115,41,37,247,160,254,238,73,37,186,1,145,212,0,40,148,212,85,196,217,201,
73,59,23,45,35,187,59,200,119,100,229,142,79,39,158,164,158,114,122,213,173,129,229,185,216,170,70,6,220,30,132,98,183,60,65,107,103,183,74,177,176,179,242,34,72,68,247,19,8,153,229,5,177,146,195,171,
124,191,55,4,103,166,7,90,160,99,180,149,174,255,0,179,109,165,130,208,89,59,66,247,18,6,146,92,144,190,107,21,249,84,131,216,112,50,7,83,89,243,39,170,69,45,85,203,122,58,234,58,118,183,225,189,75,79,
183,103,212,227,154,43,139,79,45,65,121,63,121,183,96,29,201,35,110,59,230,190,170,248,31,227,235,121,62,19,104,186,23,136,175,13,151,135,172,245,93,75,195,18,220,91,206,161,163,182,212,32,91,148,142,
100,147,42,34,75,136,134,24,13,203,183,106,245,32,252,161,107,119,34,65,163,161,102,181,184,178,18,203,3,2,202,255,0,52,171,34,48,193,234,24,18,184,231,165,118,154,164,18,235,154,165,219,106,90,209,210,
238,245,11,129,121,168,63,144,204,141,113,180,238,151,100,120,85,124,146,249,3,134,99,142,245,53,42,66,11,223,216,35,163,185,175,241,42,88,53,127,132,222,19,190,191,188,137,60,83,224,235,134,240,141,244,
10,54,153,173,162,243,37,179,184,0,49,206,63,125,11,18,50,9,0,158,0,175,49,178,178,121,116,166,185,75,152,94,97,114,45,163,182,221,251,214,38,38,144,48,7,130,167,105,95,247,190,181,221,252,80,208,53,219,
215,127,24,155,139,27,248,117,57,163,138,105,52,232,21,85,31,203,95,41,10,41,98,190,96,76,129,212,186,176,63,49,0,241,158,28,242,102,179,215,44,102,51,150,158,196,188,94,82,47,19,64,225,211,126,225,184,
39,223,12,23,13,146,59,102,174,51,140,226,164,130,73,166,87,183,253,237,134,161,111,19,180,139,29,188,151,49,185,24,222,177,128,75,96,242,49,146,49,223,159,90,177,103,61,197,140,227,84,176,139,9,4,190,
90,249,159,49,4,166,121,199,28,31,79,106,119,195,152,6,163,227,235,45,38,70,43,29,220,207,108,115,223,116,82,129,250,133,173,155,61,26,253,252,23,160,201,107,42,155,157,74,47,180,203,24,24,56,105,60,132,
85,254,241,108,134,199,108,245,226,162,173,160,151,155,33,174,197,95,10,104,83,106,26,46,163,42,89,199,116,214,58,116,146,68,134,71,89,36,151,124,64,50,129,254,181,129,112,187,58,54,238,121,2,157,227,
53,241,37,151,246,38,139,117,4,182,77,164,172,239,165,19,16,71,242,229,148,188,184,110,73,195,239,30,217,35,21,233,30,18,75,31,6,120,102,211,89,214,160,184,181,181,213,228,158,59,25,88,179,4,104,228,76,
59,38,48,163,28,134,198,14,220,231,166,106,120,200,207,169,217,189,220,26,88,188,186,86,30,93,254,233,102,154,227,140,151,108,228,71,26,229,136,85,27,119,49,36,228,154,231,142,33,170,150,123,119,232,52,
143,30,177,19,90,70,240,67,40,89,54,52,108,64,232,164,228,170,255,0,116,100,103,140,115,207,90,216,254,212,189,139,80,55,182,250,182,161,12,198,17,24,186,89,140,115,96,129,149,220,132,124,187,135,232,
9,230,176,117,105,91,237,59,99,85,243,24,183,152,7,0,31,243,159,165,69,4,18,202,76,66,100,14,23,37,119,129,129,235,147,93,246,86,187,43,83,181,241,119,137,181,31,20,106,177,234,90,164,239,53,218,199,20,
76,207,38,242,118,166,221,217,0,99,56,206,59,18,107,50,220,13,196,99,190,49,84,45,161,176,130,194,17,14,162,183,87,239,38,233,146,32,76,81,39,240,128,196,13,204,73,201,199,0,113,238,111,219,125,242,64,
199,169,162,22,9,94,231,208,191,0,47,220,57,133,129,218,132,96,241,95,113,252,47,149,90,117,110,27,112,29,78,121,220,58,126,181,249,235,240,110,225,227,189,132,35,16,68,192,87,219,223,9,175,36,91,171,
104,209,67,177,159,46,115,192,83,210,184,241,11,86,205,232,189,14,83,254,10,7,225,233,175,124,25,160,234,246,214,13,125,46,151,168,200,204,171,8,151,106,60,4,156,129,243,0,76,106,164,168,39,158,152,201,
31,156,18,92,44,215,11,38,157,28,118,86,179,111,242,88,202,101,125,153,229,16,242,205,140,244,29,113,212,26,253,78,253,188,237,103,155,246,115,241,13,221,169,145,110,52,233,172,239,68,144,177,71,64,151,
10,29,131,41,4,97,25,143,28,215,230,189,190,163,170,74,233,98,186,144,144,106,146,125,176,221,67,107,246,171,150,34,61,133,10,198,139,151,81,30,89,85,136,29,79,57,53,20,85,233,222,215,22,33,123,199,117,
240,239,225,55,136,238,147,195,90,203,88,219,219,105,119,119,74,144,222,222,76,12,164,175,44,177,64,185,216,54,254,241,73,249,138,247,235,143,127,241,70,131,30,155,166,248,143,73,241,28,87,49,248,124,
3,29,142,161,28,11,28,208,78,202,202,124,165,98,36,220,141,182,77,248,33,150,70,28,215,156,254,199,242,91,221,120,229,203,88,120,195,83,158,123,116,75,89,227,88,99,136,6,207,152,36,227,6,60,148,195,41,
200,234,14,9,175,182,236,188,11,226,255,0,16,235,144,106,83,105,113,67,173,219,198,150,23,66,237,69,204,210,70,134,77,175,39,5,124,192,54,33,98,252,227,35,140,26,241,241,21,42,75,20,213,68,154,73,90,215,
223,91,235,250,107,127,35,122,18,140,33,191,93,123,31,156,30,37,241,30,175,170,120,113,60,27,227,45,110,93,73,52,249,101,151,71,212,110,226,150,107,136,36,32,43,164,119,14,73,146,217,246,130,209,238,59,
78,210,8,32,10,243,51,104,86,53,73,25,55,99,230,216,251,176,123,243,95,167,30,59,248,15,121,11,203,168,248,162,215,67,210,116,217,100,219,53,148,186,146,172,82,163,48,249,198,17,164,50,168,7,1,112,50,
216,36,245,31,53,248,211,194,159,8,124,51,120,4,126,28,186,188,219,35,108,77,67,198,51,88,71,112,9,194,128,6,247,28,124,216,234,123,226,189,154,88,198,180,148,95,220,151,231,103,248,24,206,154,123,51,
230,85,130,43,123,83,35,58,199,26,143,153,201,198,62,166,178,174,46,212,160,48,57,97,212,48,25,7,241,175,111,212,181,159,10,65,44,173,225,221,19,194,158,26,155,105,68,184,154,210,235,90,154,22,7,34,68,
154,232,34,171,231,130,66,28,140,122,87,155,107,246,86,151,122,179,234,26,151,142,173,181,57,231,124,203,59,218,180,123,176,6,58,18,15,3,24,0,116,174,152,98,33,45,215,224,255,0,68,103,236,250,221,51,19,
76,208,245,221,106,195,84,212,108,180,249,165,180,210,173,77,205,243,163,40,242,98,7,153,10,146,9,3,190,50,125,1,231,22,254,29,105,90,70,165,227,61,26,207,88,189,138,203,77,184,187,72,238,39,149,194,132,
79,226,249,143,202,164,129,180,19,198,72,235,208,245,223,14,252,122,62,30,120,133,181,93,18,242,61,86,11,168,205,190,165,96,246,108,144,205,109,221,87,112,31,188,25,98,172,188,115,131,215,53,206,124,71,
240,118,163,225,75,155,105,158,222,89,124,47,169,47,155,163,222,174,26,59,187,114,187,212,103,168,101,66,20,171,115,242,146,50,51,141,212,239,167,125,187,254,36,56,59,27,95,180,159,128,180,255,0,3,120,
244,90,104,215,255,0,106,208,238,173,196,150,142,242,171,200,165,78,36,142,76,127,16,37,78,112,1,14,49,208,154,242,180,96,139,201,0,158,131,189,111,234,190,40,213,117,15,10,105,186,5,196,145,62,151,99,
60,151,16,48,4,200,197,243,195,62,78,229,25,249,71,24,192,235,138,230,226,33,220,247,247,61,170,162,154,90,132,154,189,208,52,204,241,176,99,182,37,25,32,122,15,90,211,93,58,230,222,234,91,91,155,118,
130,226,22,2,72,223,27,148,144,8,206,56,232,65,227,214,169,172,96,125,226,54,158,185,21,167,30,171,4,9,243,187,200,199,150,108,150,118,62,164,159,243,197,2,220,233,45,108,45,18,221,38,89,164,89,227,193,
0,158,228,15,207,191,74,196,213,110,140,176,157,58,220,143,36,79,230,202,225,206,9,198,0,35,56,32,30,71,124,214,61,246,185,121,50,178,198,60,168,219,142,14,75,125,79,244,21,66,1,113,43,5,25,1,142,50,6,
72,252,5,26,143,67,125,228,178,182,129,67,49,121,55,114,23,171,127,159,122,214,240,62,165,167,218,107,176,94,235,186,100,23,118,14,161,99,19,48,217,108,119,13,210,16,120,102,219,144,14,70,27,29,137,21,
171,224,31,134,90,198,189,14,155,168,253,130,105,236,46,37,96,241,193,112,4,243,71,229,238,18,33,63,38,51,142,174,15,7,34,174,124,69,240,155,120,118,226,215,77,71,23,54,74,99,18,24,1,105,160,144,169,253,
217,99,242,242,196,224,158,27,43,131,212,86,46,164,37,46,75,234,83,166,220,91,182,135,179,232,126,38,211,181,141,70,250,246,222,206,20,211,46,183,52,17,101,99,243,157,19,45,12,65,151,239,140,22,219,158,
123,28,12,214,100,94,30,179,213,35,177,189,240,94,177,115,102,137,51,67,43,78,22,57,161,135,27,91,10,188,59,179,168,201,192,57,10,121,198,15,157,120,58,219,80,240,243,37,242,193,113,169,105,218,114,53,
252,169,105,44,102,8,200,202,131,41,96,88,96,156,110,94,135,175,28,214,134,173,227,104,181,63,180,73,165,218,164,87,174,217,43,123,1,117,126,65,111,156,17,130,58,128,122,241,140,117,28,244,112,252,149,
27,139,208,152,83,81,94,241,218,218,105,26,36,94,26,137,53,45,66,109,70,245,174,102,185,154,41,174,17,102,130,119,39,115,166,221,160,242,217,61,9,12,122,244,175,59,241,143,135,244,187,59,93,79,81,176,
71,182,158,8,163,38,206,85,142,72,190,118,195,28,159,155,36,158,10,156,2,42,47,26,75,113,168,105,218,86,168,117,102,184,71,200,146,56,226,36,219,76,70,92,99,128,20,241,199,39,174,114,8,174,98,121,82,59,
43,165,186,188,137,21,246,35,199,56,11,184,41,4,3,220,97,191,10,236,81,73,92,106,201,159,95,252,115,208,116,15,28,252,82,186,241,28,222,57,240,67,235,55,87,75,44,215,147,248,137,76,192,1,136,32,128,32,
99,24,102,198,74,156,245,10,167,57,24,26,110,167,227,205,42,221,237,252,115,22,157,119,164,91,33,255,0,143,235,134,142,236,170,51,41,104,75,40,121,16,50,224,43,168,124,96,146,14,51,23,130,190,54,124,63,
240,160,157,254,29,120,15,197,62,40,241,27,70,170,154,174,187,170,65,26,219,1,156,178,36,33,150,2,224,156,182,21,186,12,174,56,143,227,133,214,173,241,26,231,224,222,145,12,6,11,171,169,110,44,245,123,
237,62,70,251,36,183,31,218,17,200,226,39,118,103,151,201,7,6,66,72,50,55,7,32,129,243,50,195,84,186,85,237,202,187,218,253,244,182,223,121,179,141,227,238,189,250,122,158,63,226,207,30,223,55,128,117,
79,9,195,97,37,166,137,166,92,67,246,23,185,157,140,176,65,11,239,88,65,63,121,139,12,228,253,209,242,0,57,174,223,89,248,85,115,4,222,43,186,241,13,228,186,132,214,62,22,213,47,181,63,32,178,71,14,161,
5,188,50,199,11,108,35,106,178,205,27,149,60,182,27,156,87,145,95,233,218,116,191,17,47,70,147,165,137,180,237,63,81,242,39,179,185,187,107,151,17,173,199,151,37,193,221,206,198,111,152,156,157,172,70,
114,50,107,239,61,34,234,79,16,106,255,0,182,55,131,173,215,201,251,61,140,247,64,20,249,64,155,78,88,120,29,120,104,83,240,53,232,210,163,10,114,82,130,181,238,223,158,203,252,159,201,25,187,189,217,
241,47,197,169,228,214,27,195,190,39,190,183,118,212,47,45,190,199,173,72,164,3,117,117,110,163,108,248,60,43,201,1,66,203,140,102,50,62,188,95,138,239,180,230,208,167,142,43,8,188,231,183,42,151,65,64,
108,228,49,32,245,35,3,110,24,124,185,108,117,174,222,250,234,77,95,225,38,163,127,246,117,145,224,123,105,29,137,230,25,102,1,21,254,160,77,143,124,118,174,115,192,94,22,26,228,183,23,58,131,73,105,161,
198,129,174,167,206,193,14,72,10,202,88,21,108,124,196,169,232,160,147,192,1,186,233,201,89,183,209,137,69,183,107,106,117,58,235,91,207,241,122,194,226,212,44,173,113,164,199,45,211,110,45,228,184,134,
103,99,146,126,92,109,65,236,15,189,114,90,191,137,38,157,180,104,180,137,230,130,230,11,104,149,136,92,68,178,224,239,159,203,28,18,65,4,41,31,46,59,30,106,230,179,123,109,168,79,4,154,123,121,13,99,
164,71,97,52,139,129,246,129,24,40,92,231,5,137,12,114,113,243,13,167,3,156,226,17,44,23,201,48,138,107,59,73,101,64,254,90,29,242,142,62,84,227,44,204,49,143,247,135,90,206,17,78,215,90,216,77,52,111,
95,218,221,106,246,22,122,118,147,165,11,196,213,227,121,18,79,52,253,169,174,88,109,45,35,18,7,200,164,130,187,182,0,114,220,129,158,223,225,174,148,158,40,248,253,225,141,63,80,186,211,245,79,58,230,
43,75,150,141,80,64,95,97,136,121,106,255,0,43,198,141,229,229,152,97,176,196,47,221,207,35,54,167,38,161,103,113,163,218,88,202,32,134,218,104,228,181,144,135,104,155,206,82,145,169,25,220,85,88,134,
7,134,118,114,121,21,103,193,62,22,151,87,240,244,90,191,135,227,154,203,89,210,244,229,186,136,16,88,221,92,171,22,73,226,61,10,134,142,49,129,254,175,184,199,21,157,89,114,71,222,124,177,239,235,214,
227,138,110,73,37,115,213,47,62,30,235,126,24,215,162,209,110,109,214,245,46,194,219,202,86,221,227,243,158,220,73,27,59,7,249,134,229,98,67,231,24,140,14,195,62,57,227,93,39,88,240,239,140,244,173,54,
107,224,22,55,18,219,92,70,20,103,247,108,84,149,35,30,102,208,115,212,100,215,232,111,141,60,65,7,197,63,134,90,124,158,29,146,91,111,136,250,109,140,90,166,132,93,6,111,98,150,34,210,217,48,108,12,185,
142,104,192,39,137,34,67,223,159,206,175,22,248,142,247,197,30,42,209,158,218,209,46,110,173,192,146,23,182,4,44,200,112,75,170,49,59,0,80,64,220,126,83,144,77,103,133,246,206,87,109,91,250,254,190,243,
73,193,69,89,238,87,213,245,59,8,175,116,237,90,223,251,77,173,162,127,61,35,158,225,88,13,185,201,10,15,0,114,14,59,244,226,189,22,235,197,218,230,133,169,233,82,223,75,105,171,235,69,190,219,155,157,
134,230,205,149,74,21,152,196,0,102,104,155,42,8,201,82,122,100,215,12,208,233,233,163,203,53,195,173,221,156,18,206,45,158,43,129,186,51,229,131,184,186,97,92,160,249,137,81,180,18,87,60,84,183,186,117,
181,172,30,22,240,235,8,237,36,54,203,117,168,203,19,243,9,157,195,149,200,31,121,99,33,120,251,188,117,205,107,82,17,157,147,70,54,229,216,250,7,246,119,183,131,194,126,32,213,254,38,45,141,172,58,102,
130,241,233,154,12,114,91,170,155,173,82,232,136,206,209,157,204,177,66,91,24,28,153,48,9,57,3,39,246,106,215,174,180,79,17,252,77,214,110,202,89,90,221,105,146,220,95,218,180,95,103,66,99,188,23,30,76,
113,183,204,25,94,64,54,12,144,132,240,112,107,31,196,55,242,143,131,90,123,188,66,75,36,214,181,11,146,16,228,40,183,84,86,124,169,5,92,17,144,71,113,145,92,246,145,28,87,126,1,214,35,115,44,250,182,
181,168,201,54,153,36,179,147,229,69,10,199,6,230,108,18,29,228,85,0,145,150,217,154,202,156,185,32,220,183,178,95,135,252,22,196,221,213,151,67,210,188,67,172,120,251,226,118,145,123,113,127,109,53,199,
137,252,46,215,23,55,23,246,182,65,77,237,171,58,73,178,16,168,88,181,187,204,74,162,231,16,202,70,28,138,224,95,78,241,234,216,195,171,220,219,203,113,163,153,90,222,57,204,8,17,202,169,37,114,163,229,
97,134,5,88,41,37,79,29,43,220,188,35,226,24,188,53,164,124,33,241,221,156,122,165,166,165,168,120,148,188,144,219,1,40,185,136,90,73,230,68,99,45,159,49,92,236,112,0,202,146,50,88,40,28,231,197,95,16,
163,120,223,80,147,194,122,108,122,69,189,138,36,182,214,115,91,157,215,242,44,78,210,121,160,144,31,124,108,6,31,4,24,200,7,61,57,177,116,228,251,107,123,95,190,159,153,172,98,175,239,63,83,128,240,126,
164,53,45,74,227,71,120,132,23,82,91,153,34,184,120,118,109,56,31,197,130,50,58,225,134,61,233,117,91,223,18,233,115,48,180,75,5,184,49,130,35,154,217,37,134,238,48,70,29,36,218,124,183,201,111,221,238,
59,177,149,32,3,89,150,254,58,179,214,47,172,108,236,163,113,116,73,88,98,119,27,227,87,35,97,137,148,18,227,60,0,220,252,227,61,49,91,255,0,20,98,183,147,195,55,250,109,188,42,218,229,182,55,11,85,222,
179,30,88,148,218,74,224,100,240,190,167,4,96,138,229,142,6,113,169,106,176,92,175,231,111,248,15,231,174,204,233,139,140,32,229,77,251,203,228,100,207,174,62,179,120,14,178,240,216,91,219,129,6,109,46,
25,77,202,12,144,84,100,237,93,249,24,32,224,231,174,115,94,120,247,191,218,178,199,14,167,119,43,188,27,161,55,87,179,155,143,179,12,18,54,140,224,118,202,175,28,12,118,174,34,11,228,51,58,193,32,150,
22,35,63,50,228,14,195,219,189,91,151,89,242,236,102,143,200,146,52,49,34,41,72,134,214,35,160,207,184,228,158,9,34,190,138,133,24,209,138,132,52,72,228,171,86,85,101,121,110,77,173,222,40,189,49,91,73,
40,182,118,62,66,201,32,82,220,99,46,1,199,99,199,80,8,21,30,143,246,121,100,51,92,70,100,144,177,33,16,99,57,254,240,231,128,61,57,252,171,50,222,118,107,149,154,88,247,171,33,80,161,186,103,249,116,
206,106,252,8,176,196,110,133,226,125,176,124,171,24,67,150,29,73,246,29,189,107,100,186,153,238,94,188,212,229,212,53,40,204,146,163,199,109,0,10,210,160,80,193,112,2,31,94,48,20,123,19,235,80,106,19,
153,111,110,30,121,4,145,40,202,144,122,142,139,192,224,86,72,98,219,134,25,246,140,144,168,72,207,114,127,198,181,244,168,96,91,153,29,154,77,240,176,10,139,143,155,61,79,235,86,54,95,210,12,150,90,124,
154,131,137,225,147,39,236,252,183,24,224,183,230,113,158,156,142,121,174,118,114,100,184,56,33,154,66,48,65,193,7,241,233,93,62,183,115,169,152,109,140,174,62,192,132,44,42,126,80,118,168,194,129,201,
192,200,224,245,53,198,223,202,82,240,3,134,125,195,59,78,70,122,159,240,169,91,220,58,17,92,218,201,202,200,155,54,140,237,98,51,142,223,227,84,222,50,62,108,1,140,115,235,90,240,186,203,20,242,76,25,
217,152,129,207,3,130,73,63,149,99,92,72,210,111,4,144,184,199,208,83,67,27,111,184,204,112,112,112,78,115,138,237,244,147,107,28,44,27,115,220,109,220,174,72,24,29,253,248,174,37,21,94,85,3,229,83,140,
224,116,21,215,88,233,242,93,89,191,216,116,171,185,158,71,72,196,182,246,210,76,200,24,128,78,20,115,198,78,56,206,41,73,164,174,216,172,219,178,61,31,71,241,150,154,186,21,160,143,193,186,85,197,229,
159,204,146,79,117,112,84,54,194,166,92,41,7,45,150,202,125,208,49,134,53,129,226,111,23,37,235,89,88,218,233,122,101,173,184,8,168,144,105,162,208,111,0,2,170,90,86,202,244,28,140,227,20,251,136,181,
63,15,67,110,187,175,108,111,97,222,45,46,154,196,193,60,128,2,164,236,113,157,172,9,25,96,113,158,9,173,127,13,107,26,54,133,165,221,11,219,187,40,111,188,237,193,63,178,222,89,174,80,28,136,164,220,
155,140,71,24,33,138,124,167,131,206,107,142,74,13,243,45,75,213,232,207,47,186,107,187,171,217,45,211,55,23,67,33,146,220,153,72,198,73,1,87,39,128,15,229,85,46,173,100,142,91,84,184,186,137,124,248,
214,76,249,133,150,37,108,240,248,28,48,199,42,51,140,142,121,175,64,241,175,137,102,213,77,170,90,195,107,4,82,18,150,162,5,88,150,52,118,37,182,48,219,229,169,249,120,25,192,94,166,184,173,127,68,188,
210,100,177,23,182,173,28,55,33,254,207,113,30,12,83,4,108,19,25,24,220,57,95,189,131,243,3,140,26,214,18,109,93,171,16,227,99,30,123,103,19,21,135,204,157,187,109,136,130,125,62,94,163,61,106,59,88,166,
113,59,170,18,145,0,172,221,148,177,192,254,71,242,169,109,210,226,75,135,120,167,216,203,27,187,179,204,83,229,28,145,187,175,60,12,14,189,42,203,221,205,119,101,105,98,182,170,215,41,32,85,145,95,46,
200,1,9,24,94,128,13,222,189,70,123,214,183,96,102,237,221,38,197,229,125,106,253,140,111,37,207,217,96,136,203,44,224,162,32,140,179,22,198,70,0,231,119,7,24,205,110,233,126,18,241,29,195,143,35,64,188,
86,242,215,115,51,160,4,244,221,185,152,0,9,231,29,177,93,21,143,134,236,52,149,187,142,251,196,48,199,226,56,174,76,80,166,151,121,28,162,36,81,251,194,204,48,199,253,146,132,46,65,201,200,172,93,122,
122,164,239,233,175,228,95,179,159,84,59,69,248,59,241,75,89,179,150,234,199,225,207,138,103,180,65,204,159,217,110,168,115,255,0,93,54,159,210,186,173,7,224,135,197,123,44,222,201,225,89,44,228,116,199,
151,127,50,64,84,6,228,72,29,128,94,253,216,227,156,86,133,151,197,123,205,15,84,142,230,45,123,196,151,250,146,33,63,103,139,81,17,91,166,214,225,118,41,32,38,115,200,12,252,114,77,114,222,52,248,173,
226,237,122,59,196,151,83,213,35,209,102,159,207,49,207,112,36,96,199,37,142,78,78,227,207,67,129,208,98,184,36,241,245,37,104,56,168,249,166,223,254,148,175,248,22,213,20,186,178,215,139,252,3,171,232,
126,13,147,196,186,151,137,52,83,99,109,114,182,54,227,78,212,126,215,229,74,206,3,68,92,42,133,216,0,39,37,184,24,62,135,209,36,240,70,139,255,0,8,189,236,67,93,179,190,151,87,208,111,47,172,236,173,
97,63,52,246,192,51,134,126,177,183,76,14,73,4,158,6,0,225,188,93,126,27,224,228,94,26,58,229,180,54,122,46,167,109,20,58,122,38,233,175,238,102,221,61,205,196,173,217,19,124,106,189,50,223,47,64,1,231,
223,196,218,255,0,135,46,252,45,169,79,109,113,62,159,161,105,237,110,145,176,42,155,102,142,65,44,110,227,59,89,195,131,207,162,224,19,154,231,246,88,186,208,74,85,44,219,118,182,154,116,233,223,123,
62,187,154,69,209,140,182,211,75,254,167,99,125,7,195,221,27,82,143,75,241,23,134,188,65,167,236,181,137,87,86,210,239,22,73,3,237,12,93,225,124,197,32,218,192,252,191,49,231,3,34,189,43,86,240,95,135,
199,130,180,221,99,75,77,26,243,68,189,158,9,180,205,122,75,102,91,137,88,113,36,114,163,127,171,12,173,230,41,231,231,140,22,28,13,220,183,195,88,46,47,254,63,104,218,71,139,39,138,238,27,205,27,237,
150,150,246,240,236,138,230,104,236,230,16,6,66,196,33,10,100,98,79,4,198,190,188,65,240,79,197,151,26,23,193,251,13,98,238,73,100,209,60,45,170,71,14,163,20,202,205,20,214,151,240,152,164,200,0,150,216,
242,51,142,51,242,96,115,73,82,147,140,37,119,204,173,213,217,222,235,243,86,215,189,236,78,138,90,173,30,135,71,240,214,29,22,231,196,23,209,95,104,109,2,37,201,209,252,83,165,66,54,28,9,12,150,215,49,
168,227,120,101,47,25,24,234,84,19,132,175,57,248,175,225,198,248,127,227,159,237,77,19,80,121,46,44,175,204,98,234,111,222,11,168,228,136,180,23,128,16,3,36,209,72,85,128,192,89,85,148,103,57,175,185,
101,248,87,163,104,218,134,216,150,109,78,93,111,72,151,67,214,82,51,255,0,33,25,82,3,125,165,95,249,163,230,142,76,91,203,8,151,130,100,8,70,14,43,231,63,139,150,11,226,47,5,89,107,113,219,25,116,121,
52,232,237,165,213,80,2,178,218,187,23,138,64,188,20,198,232,230,80,83,247,82,45,196,99,134,92,245,81,115,166,211,123,75,250,249,254,123,118,98,112,79,75,159,46,248,51,78,141,124,107,96,159,218,107,166,
66,178,160,91,198,181,146,111,36,240,170,74,39,206,71,29,178,70,115,207,53,236,126,21,208,117,13,26,31,12,234,145,92,65,123,164,34,234,49,37,196,36,152,164,54,76,200,155,51,200,221,41,141,192,35,59,107,
205,124,50,44,30,55,131,87,182,15,122,179,237,147,19,180,32,199,180,13,202,113,144,193,198,225,145,181,212,128,122,230,189,83,95,241,44,41,225,77,25,224,185,158,207,87,177,149,35,102,123,114,214,242,191,
156,38,105,22,60,231,56,139,158,79,4,122,115,181,122,151,156,99,230,105,28,59,116,220,206,87,199,247,112,95,106,122,127,129,116,187,99,52,86,183,144,218,218,78,170,24,136,182,172,108,11,96,22,62,98,153,
75,224,2,167,215,154,236,254,19,54,155,226,125,110,111,9,105,205,113,8,189,211,174,244,107,19,12,153,123,27,184,18,89,109,174,225,42,87,116,114,44,50,35,161,60,146,7,32,131,94,103,240,170,117,95,137,94,
31,109,85,150,105,238,175,4,215,145,203,38,30,68,144,56,109,189,126,109,205,187,142,202,107,7,224,230,173,113,225,237,106,199,196,209,95,197,111,168,104,119,182,146,233,234,88,183,218,29,75,134,69,94,
225,211,10,123,0,249,237,90,56,46,77,122,91,254,28,230,179,78,197,77,67,194,34,43,191,181,201,172,91,73,160,24,82,113,169,32,110,81,192,40,130,38,249,188,210,49,136,206,120,193,39,29,121,251,251,168,92,
249,22,86,166,43,8,143,238,209,200,46,231,251,242,48,251,204,125,190,81,208,12,12,158,151,197,150,240,221,105,154,124,246,215,83,61,231,155,41,154,219,4,193,100,165,142,216,226,236,20,12,15,80,62,94,152,
199,53,111,96,94,101,23,78,68,109,253,209,140,254,53,189,38,220,110,221,196,218,185,14,158,196,92,12,145,147,232,43,165,140,168,221,235,235,89,215,118,214,182,207,178,220,125,208,14,236,100,147,158,153,
254,181,118,35,192,98,121,235,90,39,113,35,209,126,23,93,24,117,37,64,79,44,15,234,51,95,103,124,39,187,43,168,88,78,101,98,175,34,166,27,27,123,12,115,245,175,132,124,29,114,246,250,164,110,164,117,254,
153,233,95,97,252,45,212,93,244,203,57,99,80,37,76,62,72,239,199,95,126,43,159,16,141,105,62,135,211,191,180,6,153,62,173,240,51,198,86,150,172,82,234,109,34,117,137,147,134,15,229,146,165,79,168,42,15,
212,87,230,207,138,255,0,225,54,240,102,157,161,234,250,111,140,90,239,195,154,185,77,75,67,191,142,193,21,8,146,77,207,34,190,10,193,56,148,186,73,25,39,36,183,27,91,3,245,42,225,164,185,240,204,202,
248,33,160,27,129,254,16,125,125,120,60,215,195,22,158,1,208,181,175,138,90,255,0,195,205,74,233,244,237,115,87,156,190,141,116,187,163,182,209,245,88,92,60,86,205,18,226,54,183,184,66,54,57,25,86,11,
130,89,134,56,105,74,205,65,245,102,245,82,106,231,35,161,195,227,127,8,248,131,72,184,183,215,44,159,77,241,39,153,117,165,221,193,108,109,224,158,68,69,38,38,136,175,238,100,27,136,104,79,77,187,186,
114,61,75,199,223,23,190,40,182,140,62,205,226,205,67,74,181,72,255,0,126,154,124,129,10,133,80,55,21,218,89,186,117,86,24,29,125,107,199,229,155,198,30,27,215,245,63,13,95,218,94,107,250,62,163,122,176,
235,182,218,157,145,34,206,238,41,152,137,20,48,67,12,235,26,146,146,2,21,148,237,36,166,205,189,111,196,205,32,92,89,232,218,134,149,226,40,53,157,50,245,159,200,186,72,124,134,70,144,166,194,85,143,
238,101,80,14,248,156,118,5,73,25,174,106,152,103,26,234,172,36,226,188,180,253,122,246,255,0,134,42,157,149,39,27,107,253,127,90,30,85,226,43,191,23,248,138,233,231,184,215,167,212,101,42,24,77,121,169,
171,72,75,48,85,101,18,156,30,88,113,158,153,39,129,90,90,126,175,170,36,122,190,153,31,134,45,38,183,213,93,226,150,198,226,70,17,218,229,113,136,155,25,79,186,72,231,174,123,98,183,124,37,22,167,225,
233,22,95,51,70,131,204,145,89,34,158,120,37,91,57,17,179,36,100,18,70,205,193,199,161,12,64,56,218,105,222,59,240,230,139,29,134,137,226,127,8,107,26,117,157,209,184,217,165,180,179,237,137,110,82,109,
198,197,192,192,42,132,109,142,65,247,163,101,7,32,146,109,194,124,205,191,190,223,159,252,57,151,35,90,166,113,218,183,131,175,245,9,100,125,70,220,202,116,237,53,60,205,68,220,5,223,110,170,204,175,
43,29,223,116,7,30,96,231,140,54,113,94,123,174,232,254,21,84,243,173,60,81,114,138,202,10,169,177,146,232,51,119,10,232,17,118,142,199,38,190,210,241,15,137,188,19,103,161,105,215,250,157,236,26,38,175,
38,152,36,178,183,159,49,204,136,196,137,109,220,14,62,73,83,7,147,130,131,251,220,252,85,171,104,218,100,90,166,163,21,159,139,244,95,236,201,231,145,173,192,19,75,181,73,37,62,85,92,43,40,59,51,147,
211,240,29,24,56,86,122,206,163,249,90,207,239,76,218,171,166,163,238,197,59,247,190,159,145,207,223,88,11,48,38,105,210,123,39,56,138,226,55,82,178,122,128,1,59,91,142,135,145,93,95,132,254,32,107,150,
115,233,54,87,114,11,239,14,88,93,195,113,30,149,116,161,162,202,58,182,71,67,187,1,176,11,109,220,122,86,116,94,24,208,119,9,199,142,116,214,243,20,51,4,210,110,221,207,213,87,158,59,231,145,237,82,13,
27,195,48,234,107,111,55,138,46,238,99,8,119,173,150,139,36,50,43,113,199,250,67,1,211,53,233,243,69,173,127,35,142,206,250,27,63,31,99,240,205,215,143,46,245,175,13,234,182,87,122,30,180,5,228,105,3,
141,246,238,64,18,35,160,230,35,156,29,167,156,150,53,227,238,254,83,20,25,39,183,210,189,58,75,95,2,69,48,118,30,45,184,126,20,69,44,54,42,27,60,14,85,243,220,98,166,147,194,94,26,51,78,102,241,70,153,
13,176,4,198,215,183,173,20,241,48,28,198,241,162,128,199,57,4,169,249,113,223,145,77,78,41,36,18,77,187,158,74,100,150,78,6,107,87,195,186,29,246,179,170,90,216,90,199,153,231,144,34,146,112,23,61,216,
156,0,63,31,166,78,1,234,151,66,210,225,149,199,252,36,62,17,35,160,31,218,55,51,115,234,0,65,159,166,123,213,214,176,240,252,123,64,241,38,144,144,176,5,130,233,183,115,252,192,123,99,140,244,244,245,
170,114,95,213,201,179,58,157,7,224,245,236,23,88,215,86,84,138,53,115,57,141,182,249,56,232,216,193,200,207,66,196,43,3,145,207,21,30,177,105,167,248,99,199,107,101,164,232,182,247,80,219,199,24,150,
107,201,88,132,102,36,190,24,144,84,236,232,195,145,145,216,26,189,168,248,226,229,35,151,201,248,139,43,21,69,88,60,159,14,206,12,103,106,140,254,240,156,142,58,30,51,131,218,184,137,181,77,30,238,246,
89,174,111,53,107,203,134,11,185,237,236,96,140,30,57,200,156,179,103,57,231,167,63,90,201,69,191,141,223,239,52,209,108,123,230,143,226,43,93,79,251,94,215,69,180,138,242,230,213,26,242,27,19,113,37,
186,205,26,109,15,18,133,33,76,155,91,114,145,212,229,79,169,243,231,241,95,135,53,175,12,153,147,71,191,107,212,46,110,70,211,32,104,134,74,59,182,72,42,57,56,63,48,35,166,43,157,211,181,251,59,75,171,
59,227,107,226,9,44,227,184,37,146,120,172,196,114,144,57,83,229,170,176,237,200,53,45,167,138,180,43,75,200,238,80,120,174,125,204,90,123,73,174,45,18,25,71,35,105,219,134,35,158,231,173,74,163,5,170,
69,123,73,218,198,32,241,148,130,231,80,182,251,4,211,233,215,214,175,101,52,126,99,52,139,25,4,2,172,170,50,202,114,64,108,171,6,33,129,224,140,184,244,91,227,226,7,211,116,195,113,169,196,236,166,41,
99,181,150,53,117,32,29,204,15,220,35,44,10,147,156,131,140,228,87,73,174,120,163,69,191,191,105,163,209,245,232,17,128,221,4,90,217,142,50,222,160,0,74,246,249,71,30,149,81,60,81,111,5,149,245,184,209,
238,101,134,112,174,207,55,136,39,105,35,218,114,10,184,92,168,249,176,123,17,140,246,173,213,150,203,242,255,0,51,39,119,212,87,177,214,32,100,211,158,11,167,88,174,119,52,145,69,33,10,122,2,175,142,
70,48,114,62,158,213,106,222,13,119,72,93,42,125,7,72,188,77,78,222,89,158,226,234,68,18,197,112,119,98,45,168,231,27,66,22,83,192,201,57,61,141,97,182,188,215,122,140,17,91,104,150,147,60,131,203,88,
175,238,230,190,86,35,145,129,35,46,210,0,53,169,174,38,181,46,152,215,154,151,135,252,62,32,180,85,24,93,56,111,88,198,112,50,28,225,20,17,198,122,10,82,179,209,160,90,49,215,154,135,137,124,90,97,23,
175,56,210,66,200,87,49,173,189,186,164,127,120,149,69,8,17,56,3,131,207,28,156,214,255,0,135,190,37,141,19,224,238,143,160,190,159,36,250,134,153,174,220,221,89,222,13,232,150,246,183,16,0,241,252,189,
51,56,220,170,221,198,122,128,67,252,85,113,15,133,254,10,218,216,221,15,63,95,215,95,236,233,230,147,186,218,194,35,151,80,123,111,115,183,61,112,88,246,53,228,182,23,23,55,83,198,150,145,72,110,103,
114,133,80,101,29,176,118,163,103,229,60,103,27,186,114,70,58,215,13,24,198,164,95,187,238,167,167,203,168,227,41,65,182,137,60,33,168,13,63,95,211,174,140,50,76,182,82,165,197,200,141,194,188,177,35,
198,210,12,177,199,40,164,16,120,57,231,53,246,127,195,79,138,90,119,136,62,52,252,95,241,77,163,75,97,97,226,29,15,81,220,215,64,47,217,225,95,43,203,50,116,223,26,131,206,121,82,236,58,10,248,223,80,
210,36,211,238,196,183,22,235,107,35,29,241,71,26,151,136,247,253,220,132,96,227,174,50,79,60,28,10,187,167,107,162,195,77,72,237,214,230,59,134,141,161,184,144,202,162,39,142,67,137,163,96,1,202,50,128,
8,62,157,176,13,116,78,156,102,174,183,42,19,182,231,83,160,197,121,111,240,111,87,45,44,138,247,165,109,30,37,85,35,203,16,35,40,113,213,127,123,30,9,31,48,56,233,84,117,93,94,227,77,210,239,180,77,56,
201,111,165,106,166,29,70,53,12,119,44,100,103,102,59,17,34,184,61,240,49,208,210,107,23,50,218,120,56,105,201,123,114,191,52,146,170,188,35,238,43,110,72,165,126,95,120,118,98,184,56,42,79,90,111,133,
44,236,117,239,18,105,26,74,207,141,37,93,146,39,158,80,178,198,178,159,245,101,128,219,145,38,14,113,141,164,250,26,201,165,172,150,223,215,249,22,175,116,163,232,26,87,216,109,116,123,253,86,75,41,218,
241,163,217,101,9,109,203,201,77,210,238,60,238,11,188,12,14,135,185,226,189,23,195,113,219,120,107,225,182,173,226,205,79,85,243,60,74,183,39,76,208,45,100,99,39,217,46,37,143,116,151,11,144,84,121,81,
247,199,82,20,99,0,87,61,174,233,58,126,149,163,88,69,111,169,69,44,144,249,166,121,8,203,64,233,42,174,54,130,65,223,134,100,231,128,50,112,115,156,75,235,235,134,208,180,91,182,118,142,199,75,190,120,
227,137,151,121,154,82,190,115,185,29,24,18,83,60,114,199,29,7,60,242,253,237,172,244,191,222,187,14,178,246,122,51,211,117,127,12,232,254,14,248,103,240,166,236,66,243,248,151,196,246,233,175,75,35,114,
182,240,79,112,214,139,30,238,15,250,185,81,143,247,156,110,61,78,125,27,193,154,81,255,0,133,49,169,222,233,23,48,233,82,120,75,92,158,207,74,155,82,147,107,61,181,228,8,46,86,41,79,5,214,226,25,38,27,
247,97,119,118,32,215,51,241,147,70,151,196,50,252,2,240,229,185,186,104,127,225,92,104,205,36,118,46,178,92,72,89,46,101,117,134,60,114,251,99,99,147,145,144,63,187,207,57,111,226,221,59,90,184,181,142,
254,242,210,211,65,16,44,26,46,141,5,225,134,207,74,92,169,136,133,206,201,103,100,12,36,45,184,150,108,134,80,20,84,87,75,150,82,155,218,250,121,93,219,240,211,230,194,28,171,75,30,171,174,219,106,186,
109,134,159,38,147,174,217,193,4,187,53,77,45,167,135,103,152,29,214,99,36,100,15,220,186,92,70,14,215,4,153,85,246,160,13,154,249,95,198,222,49,147,196,62,58,213,53,25,172,209,44,47,47,158,246,123,27,
120,133,188,50,202,238,90,70,39,5,151,36,147,243,110,198,72,218,43,232,223,138,87,243,222,232,54,122,37,166,141,105,123,46,150,209,79,166,93,91,68,237,50,171,241,113,20,171,130,191,49,72,216,190,64,44,
129,142,49,184,252,219,227,93,59,80,151,93,188,254,208,48,38,171,120,30,102,134,192,137,185,236,54,169,33,16,255,0,1,220,126,85,231,7,138,120,26,148,39,27,198,223,215,226,86,39,158,54,230,100,26,215,137,
45,53,67,13,173,190,143,97,166,218,4,91,88,230,251,76,151,69,16,2,191,40,56,14,126,102,39,130,88,240,122,228,117,62,41,11,175,120,206,213,116,187,75,141,39,195,175,115,28,58,84,87,138,235,36,177,239,85,
201,44,3,59,51,238,99,219,45,183,160,174,63,66,241,54,163,28,214,203,167,73,167,218,57,183,138,208,92,180,74,162,53,78,18,66,231,113,93,163,146,122,113,247,79,11,94,141,119,226,235,72,252,23,39,132,252,
41,115,19,106,90,189,171,217,223,234,250,182,208,243,68,3,60,178,23,35,247,66,66,166,52,65,150,84,234,219,164,227,166,164,37,27,40,174,189,250,119,48,73,53,185,211,124,75,240,150,161,160,248,63,194,246,
50,106,118,137,103,226,13,27,251,106,39,89,60,150,113,114,238,86,7,70,7,56,85,193,198,14,118,128,0,36,84,58,77,134,151,167,235,154,24,178,159,86,154,39,129,47,173,1,242,35,181,184,181,14,168,11,32,203,
124,210,169,193,86,80,193,92,246,174,119,226,12,119,26,253,221,138,235,87,151,9,115,167,104,208,89,192,96,147,205,69,133,182,253,150,64,253,208,9,39,102,92,238,14,165,71,28,158,130,120,181,45,35,226,143,
133,245,109,50,222,91,148,109,42,222,210,202,213,227,13,28,161,109,110,35,104,132,76,219,190,80,241,156,0,112,91,113,199,90,230,158,29,59,199,155,93,123,126,32,229,20,244,67,215,199,122,230,165,107,105,
126,60,43,164,105,208,233,147,92,223,91,11,43,54,242,52,153,94,93,147,132,136,239,69,66,234,195,231,97,146,248,4,241,155,233,101,226,157,71,76,129,110,111,68,147,223,35,53,153,104,85,228,152,0,102,11,
24,45,243,228,22,96,164,228,128,197,126,233,21,226,134,239,86,158,41,53,155,104,245,59,61,63,85,182,134,214,107,133,156,162,94,72,168,11,134,35,229,45,149,222,16,244,198,225,130,78,61,151,224,239,139,
244,157,71,193,211,248,67,198,118,178,204,109,109,18,91,45,102,218,73,77,197,156,134,97,44,77,35,2,78,228,126,83,35,10,3,35,3,213,185,241,184,90,147,135,60,30,189,116,219,211,211,77,52,52,163,43,187,51,
164,178,240,126,189,102,240,94,104,62,38,179,186,178,91,120,167,129,127,177,35,251,90,41,25,101,145,56,49,20,96,224,140,18,8,232,13,69,225,11,189,3,92,147,82,251,99,218,95,165,162,7,7,120,70,142,48,196,
252,192,0,83,37,159,7,30,188,246,175,105,241,70,149,168,235,105,103,171,104,208,219,193,228,35,91,187,94,75,229,49,117,96,204,240,21,24,127,151,230,13,144,50,64,234,49,95,56,252,103,147,194,154,6,155,
117,105,164,234,247,218,150,161,117,44,190,89,32,41,138,60,13,198,86,80,23,151,60,0,11,116,207,25,174,124,37,25,98,18,117,19,77,246,211,239,218,231,77,71,26,46,209,212,242,255,0,139,30,69,215,142,46,244,
237,62,88,70,133,106,162,75,116,130,40,199,148,54,0,234,172,20,23,111,151,171,103,147,140,112,115,231,81,31,49,118,151,115,18,177,242,227,201,32,2,115,156,116,207,115,239,154,232,60,65,113,107,56,73,44,
97,158,39,100,62,98,186,253,0,195,3,147,192,39,53,7,132,173,45,238,181,107,88,174,228,138,222,207,57,150,121,152,170,12,2,66,146,50,114,196,109,3,185,227,138,250,42,80,228,143,47,99,206,110,237,179,178,
240,111,135,37,109,22,231,89,212,45,82,45,38,40,36,120,164,150,64,159,104,144,2,2,174,50,199,7,28,227,29,185,233,92,117,238,210,46,158,102,145,231,200,201,11,129,158,50,79,96,51,145,138,233,188,73,30,
144,116,205,42,254,194,221,96,138,79,50,37,84,220,29,87,59,182,252,249,32,131,199,95,214,185,51,19,179,229,1,17,168,249,212,57,59,185,227,53,178,141,133,123,150,21,205,181,169,222,74,22,57,85,50,109,86,
4,243,219,167,2,159,107,36,241,203,36,143,52,86,197,31,136,228,25,44,72,201,24,244,199,173,67,103,167,75,121,15,152,132,34,70,9,153,156,237,80,51,198,15,122,171,114,222,84,120,75,135,121,11,100,151,228,
171,123,119,199,29,232,2,245,237,205,221,211,71,102,111,37,150,206,22,44,136,95,228,143,32,2,192,118,206,7,228,43,44,90,52,186,162,68,210,0,160,242,205,216,127,90,141,102,38,223,102,213,57,124,238,81,
201,166,70,251,101,15,188,150,233,193,193,169,11,150,53,7,139,207,144,91,196,87,112,218,249,108,231,212,251,103,142,42,188,182,231,236,169,38,51,230,13,196,168,206,220,28,96,154,114,178,176,33,130,147,
239,212,243,254,52,75,246,134,138,88,84,178,67,188,72,209,145,140,62,54,228,131,206,113,145,67,216,46,95,240,253,171,220,186,68,33,12,219,198,221,163,36,159,74,244,45,107,195,190,40,240,213,181,158,161,
59,223,232,240,206,231,100,201,42,237,144,247,95,145,136,207,94,24,125,61,107,206,188,61,42,91,92,69,52,146,68,172,142,72,89,147,124,103,229,234,203,145,145,254,25,174,242,250,47,15,220,88,161,182,214,
18,61,102,112,0,176,178,209,36,137,101,124,100,179,72,223,41,32,3,208,102,185,113,83,146,138,81,249,233,127,248,111,153,173,27,93,183,254,70,124,58,222,156,183,19,94,107,144,234,26,228,230,47,46,47,180,
93,101,65,198,6,226,196,146,131,174,7,61,107,147,213,53,91,171,184,74,0,202,185,249,191,127,36,164,140,96,41,46,196,144,42,198,168,209,67,61,229,171,134,19,66,64,12,73,44,24,30,84,1,144,73,207,39,182,
56,172,169,91,114,111,100,56,206,7,24,230,157,53,110,132,73,183,169,4,204,191,99,16,225,114,93,165,147,41,200,224,40,4,244,32,140,156,123,12,211,45,245,11,152,136,221,44,146,162,163,170,36,146,51,42,150,
0,100,2,79,160,233,215,2,172,20,45,117,113,18,171,51,44,68,128,167,147,128,56,172,240,155,134,112,8,60,138,217,89,161,10,46,72,39,229,4,159,110,159,79,74,156,44,146,201,110,168,153,105,14,54,227,36,99,
174,71,211,244,230,163,68,69,228,231,62,245,54,66,42,128,72,57,207,191,52,219,236,35,69,216,29,48,65,117,113,59,36,100,172,80,177,253,218,1,140,124,185,199,175,111,198,180,180,176,110,145,101,125,65,99,
17,48,5,102,83,242,174,54,171,110,232,123,40,92,113,215,160,172,43,3,53,212,175,20,86,230,226,112,164,4,40,100,99,219,112,3,156,142,49,233,197,58,226,222,226,40,227,105,224,154,221,119,0,173,44,14,160,
146,51,193,32,12,224,231,233,81,37,125,1,27,247,90,157,204,186,103,216,214,86,145,26,228,8,220,58,110,4,38,198,225,71,49,148,24,92,113,208,242,73,167,232,186,141,144,185,130,222,250,221,60,166,127,44,
73,147,128,160,150,5,151,56,36,54,78,126,153,28,82,120,57,108,228,241,158,148,218,154,68,250,60,83,110,184,73,221,85,26,53,70,118,86,36,227,7,96,7,63,222,199,122,194,210,25,110,117,52,119,130,51,25,18,
78,208,242,23,24,39,96,239,193,96,0,246,233,81,202,154,104,118,212,246,109,10,214,221,254,1,234,119,105,103,3,220,248,131,198,150,58,116,215,14,185,104,224,138,217,174,130,174,56,5,158,67,185,142,120,
31,151,151,248,191,83,186,212,181,217,210,73,143,217,226,115,106,85,79,203,44,113,205,33,66,235,209,143,205,223,240,197,81,177,212,175,32,210,197,159,246,141,210,219,197,112,38,242,17,152,70,102,8,83,
204,219,211,120,82,87,29,112,113,80,207,101,229,106,50,69,52,195,200,138,101,73,38,137,179,193,1,137,0,251,30,51,198,120,170,141,56,169,167,219,254,7,252,2,156,238,172,122,231,130,188,67,29,143,196,223,
133,122,250,234,208,134,134,207,78,179,184,145,226,27,98,82,210,218,206,175,206,114,169,38,121,235,237,93,215,129,111,244,139,56,252,117,240,168,73,105,171,90,223,127,106,24,67,41,16,73,113,103,121,231,
91,115,200,218,235,28,189,9,192,53,243,4,146,60,113,75,246,117,253,216,13,178,64,187,92,245,218,125,143,67,237,248,87,172,107,158,53,189,210,126,42,233,254,47,179,177,178,146,226,234,210,194,245,146,123,
85,117,108,219,172,82,237,207,0,58,163,2,72,39,44,122,86,51,195,65,105,29,218,118,254,189,89,106,165,223,188,125,53,240,131,226,224,130,195,225,118,179,113,116,70,138,140,158,13,213,30,70,63,186,120,220,
93,232,183,44,59,148,79,145,206,71,27,235,198,167,241,189,255,0,134,238,124,91,164,67,162,219,127,100,221,234,119,226,199,79,188,62,111,217,191,210,91,237,118,132,3,135,183,96,118,31,66,21,192,39,167,
147,89,120,167,80,211,147,85,151,73,141,33,178,212,46,252,217,237,2,132,91,114,178,51,66,80,15,148,17,27,180,120,233,183,129,198,42,142,173,226,91,205,73,236,238,245,9,214,95,38,88,196,179,172,120,153,
149,99,88,247,56,254,38,100,69,5,187,237,30,156,211,164,158,141,127,195,148,164,173,163,58,77,15,78,191,241,15,137,236,225,88,174,164,180,88,254,206,166,226,3,39,149,19,110,218,147,63,2,77,187,206,14,
115,140,26,235,238,228,107,221,103,200,212,47,22,230,125,58,59,150,186,185,212,85,70,100,37,45,161,31,40,27,73,253,230,15,61,7,190,120,219,31,21,92,24,77,141,182,161,111,113,19,194,251,81,225,120,100,
136,114,23,44,1,249,128,199,206,190,223,90,213,240,101,141,207,138,37,212,133,236,176,136,175,33,142,73,164,180,109,224,152,201,98,204,15,35,144,91,143,226,36,154,231,159,50,188,167,177,213,7,27,90,15,
94,166,110,131,28,112,248,142,203,196,90,163,188,26,116,87,142,151,23,95,119,202,194,48,5,24,3,137,56,1,70,59,254,88,87,235,111,117,169,92,235,86,197,98,182,51,121,233,110,136,64,135,161,5,156,97,67,103,
12,64,238,220,112,1,61,142,129,161,220,53,200,123,77,81,229,178,70,147,49,44,36,7,76,242,49,243,41,60,142,72,200,237,222,151,86,211,180,244,150,202,251,66,187,11,45,181,198,253,66,56,130,206,144,15,44,
238,113,23,27,138,247,78,184,4,12,156,96,88,133,26,137,61,153,203,56,95,97,182,218,71,159,107,167,106,168,201,122,47,86,73,37,181,75,82,99,12,185,253,223,57,243,166,117,59,193,198,17,129,39,37,114,121,
171,253,39,236,41,59,94,221,90,68,241,124,182,182,18,92,25,46,216,238,0,9,17,71,202,7,32,179,16,9,4,129,200,173,125,78,226,109,3,89,186,210,180,221,92,195,165,218,159,221,164,59,228,32,58,134,100,207,
13,144,73,234,70,1,28,86,21,189,166,144,168,79,246,158,167,97,113,48,243,45,82,254,218,37,138,235,182,65,78,185,60,2,1,228,123,85,198,114,214,207,71,218,255,0,215,175,232,67,138,78,221,74,23,211,89,249,
154,148,127,97,6,66,236,45,200,46,161,65,199,239,91,31,121,147,4,40,57,4,30,122,10,116,88,40,160,116,244,168,181,123,73,109,30,72,175,166,31,110,109,160,110,38,66,195,131,242,129,201,234,7,183,233,69,
161,200,62,222,181,215,15,133,106,65,183,160,75,229,106,214,238,122,7,3,167,90,250,251,224,179,121,176,8,148,17,153,56,3,160,199,61,107,227,189,36,1,169,196,91,56,83,184,125,71,74,250,219,225,69,194,24,
22,56,212,238,9,150,3,175,35,142,61,235,60,66,208,186,123,159,113,216,92,8,124,61,36,243,99,16,219,238,144,160,4,240,51,192,175,159,127,105,67,226,31,15,193,170,120,171,195,49,219,95,232,186,230,143,108,
186,165,140,214,235,52,182,162,2,98,91,187,118,225,163,42,251,119,48,63,43,8,207,29,107,218,252,36,226,239,194,147,166,9,73,109,136,44,27,161,219,140,12,215,129,124,68,180,215,117,159,14,234,143,225,207,
46,235,196,30,8,179,155,86,135,77,117,148,127,107,105,215,17,152,175,44,193,81,137,1,120,67,249,124,144,194,51,220,87,159,8,223,67,122,154,197,28,39,198,63,23,94,124,84,248,55,97,227,61,26,210,203,254,
19,123,107,99,31,138,173,188,144,227,82,91,69,43,44,177,130,27,107,121,82,172,173,31,223,104,164,44,164,152,136,174,39,225,87,138,188,45,164,233,55,169,170,88,234,126,68,172,215,58,171,218,36,18,24,68,
128,21,87,133,190,86,8,162,48,31,118,64,3,4,156,154,226,116,207,20,105,30,29,215,238,117,141,34,254,230,45,54,245,237,53,59,50,172,25,116,203,194,164,58,77,129,208,70,118,180,131,42,241,203,140,149,167,
65,226,109,59,68,127,25,89,205,225,255,0,34,255,0,88,50,58,92,42,179,11,66,255,0,50,60,59,91,28,14,20,242,187,112,58,12,82,173,207,56,164,227,175,245,216,203,153,71,84,246,54,181,63,135,186,47,137,53,
75,61,83,195,243,193,13,132,22,176,170,253,133,34,15,44,76,14,221,169,191,3,56,206,121,4,2,48,112,43,151,176,180,255,0,132,103,196,58,126,137,170,219,61,255,0,133,117,59,216,180,251,193,60,10,109,158,
34,80,65,50,174,112,37,140,176,207,3,42,8,233,128,42,120,155,198,113,107,119,201,117,35,201,105,113,133,13,54,157,167,71,108,210,96,117,103,201,98,115,248,103,156,86,116,26,69,177,180,18,197,168,189,244,
131,105,205,229,204,174,228,227,190,200,199,60,227,173,58,117,39,27,169,106,55,82,18,217,106,123,182,159,225,8,53,93,7,69,240,207,141,111,211,89,159,65,212,30,223,79,213,45,230,27,175,44,229,82,9,98,192,
244,100,71,78,173,149,0,147,201,62,49,226,143,3,207,224,159,137,26,6,153,173,90,188,246,119,151,82,172,55,36,161,138,230,6,87,72,250,1,137,1,104,139,14,155,143,203,197,122,159,130,237,165,107,35,12,58,
149,173,149,188,74,171,31,238,110,139,71,134,7,12,210,15,152,13,163,160,236,72,198,107,157,248,179,100,250,165,212,58,141,215,137,244,248,39,180,212,226,188,59,96,158,68,50,175,151,28,107,24,36,109,86,
40,50,58,101,137,163,5,59,78,84,219,235,249,157,53,163,25,83,83,75,250,71,53,241,38,210,198,194,209,188,139,35,26,221,194,217,191,68,255,0,143,127,45,247,136,198,213,249,119,43,21,37,136,27,9,30,181,229,
94,11,186,177,182,241,230,135,125,123,111,246,187,65,125,18,220,164,168,113,42,183,238,200,33,186,227,112,231,167,202,43,233,63,20,105,186,45,245,149,180,137,123,113,4,202,142,95,247,12,240,222,68,200,
17,224,101,44,1,28,46,9,233,207,173,121,79,142,60,17,160,216,232,154,110,175,167,107,211,218,91,220,60,33,173,110,44,254,104,195,178,129,229,190,226,119,14,73,45,187,159,64,43,190,50,95,3,234,114,74,47,
116,31,180,39,129,174,60,21,226,59,246,181,67,54,141,25,89,237,46,248,8,3,16,201,27,48,224,48,108,46,61,135,173,111,124,67,240,237,166,173,253,143,127,101,123,109,166,201,125,1,55,18,92,126,230,218,78,
85,85,145,176,85,92,51,168,216,112,24,54,65,249,77,125,1,37,214,135,53,172,214,87,107,170,201,136,130,195,19,76,165,37,233,144,227,111,0,140,242,51,244,172,79,21,248,87,66,212,180,136,116,107,134,187,
176,182,75,164,185,72,224,155,120,96,160,174,214,37,72,8,50,14,23,184,28,214,49,155,180,83,221,26,184,45,90,62,112,189,240,133,184,215,160,69,152,199,178,86,138,77,58,231,247,87,23,178,194,126,117,182,
7,135,73,6,48,115,149,45,193,96,86,168,127,194,59,97,29,253,212,194,43,139,219,118,133,38,143,70,86,17,234,48,65,33,80,90,64,64,195,71,158,24,19,145,141,216,201,35,232,157,111,193,62,19,213,52,253,42,
197,245,29,66,75,125,42,9,34,182,218,236,30,53,121,4,135,231,192,35,5,16,12,31,148,40,3,142,41,254,33,240,158,135,168,248,186,15,21,6,187,125,110,212,198,98,104,131,33,59,23,106,179,0,64,60,100,28,245,
238,77,104,170,59,106,67,133,186,31,50,234,30,30,69,142,230,207,66,221,174,220,205,112,64,251,34,178,201,101,180,159,149,208,245,221,134,82,88,133,36,101,78,71,53,190,14,180,81,252,69,178,183,189,156,
67,111,114,146,66,229,193,228,227,32,31,79,186,126,149,244,102,143,224,239,13,104,215,250,173,213,165,174,168,183,82,219,179,75,34,73,35,137,35,148,134,102,39,127,238,242,232,57,227,191,173,121,95,136,
60,61,225,157,35,226,103,135,46,108,46,175,188,203,171,247,145,237,28,38,200,198,198,57,200,27,134,88,247,206,114,107,78,110,120,202,36,56,219,83,208,245,191,12,233,119,186,86,161,104,35,23,203,35,171,
41,13,229,188,0,114,211,70,196,31,157,87,36,2,48,220,169,224,215,11,240,243,73,210,245,63,15,235,41,167,105,246,250,158,144,182,109,190,91,175,42,43,251,59,153,50,168,200,36,249,20,184,200,94,118,228,
28,28,131,94,195,4,214,54,147,237,123,1,45,188,241,50,237,145,203,161,12,48,193,129,224,142,121,29,42,236,54,154,62,151,99,37,237,166,151,105,110,39,101,91,200,225,136,34,204,84,151,80,112,59,103,35,211,
57,21,204,155,229,177,180,146,118,208,249,223,197,250,95,134,52,201,116,141,67,81,125,186,101,195,61,174,151,113,3,121,108,177,194,20,127,165,68,196,54,248,100,98,167,105,221,199,57,192,173,13,99,195,
113,91,203,13,181,156,250,116,62,43,212,30,40,46,103,183,159,116,0,76,161,200,48,227,8,178,39,241,14,25,248,33,73,24,246,253,103,196,90,8,119,134,211,65,179,118,125,197,38,105,24,52,36,255,0,172,93,173,
149,109,249,201,99,206,107,9,239,146,230,16,68,17,162,5,216,172,51,145,211,0,31,160,173,162,229,109,73,105,38,124,160,67,233,90,204,50,149,112,45,46,148,147,34,144,89,21,240,91,158,112,84,19,254,53,244,
107,194,151,186,116,177,219,132,130,105,163,42,147,34,143,221,146,56,108,116,247,231,175,113,92,255,0,198,69,177,187,209,149,255,0,225,27,91,205,76,143,46,25,82,206,71,145,120,207,85,201,219,215,131,199,
53,221,104,75,173,75,165,217,203,111,107,60,41,36,73,42,71,246,124,18,8,251,164,17,242,145,142,65,197,92,221,210,100,71,70,121,110,169,109,167,107,190,46,211,252,57,226,155,139,131,171,64,177,2,150,87,
40,165,174,92,162,155,98,174,48,91,145,243,110,83,151,61,50,107,107,196,223,9,252,79,224,13,74,226,251,76,140,222,216,72,51,28,3,2,229,91,60,35,197,32,27,182,229,190,100,45,159,76,245,240,157,82,251,251,
71,90,158,245,188,227,117,38,14,194,124,207,58,65,203,49,39,24,207,92,115,207,25,239,95,67,120,95,198,191,16,164,130,27,61,63,196,144,47,135,167,81,29,180,183,172,174,110,100,0,121,136,145,29,219,194,
184,96,73,69,192,82,114,107,205,173,12,85,24,194,84,90,243,77,233,242,58,232,123,26,144,146,146,106,93,44,175,247,158,47,175,94,235,119,186,167,217,245,152,239,97,131,75,137,94,91,23,243,17,109,20,0,141,
34,219,200,71,150,48,202,48,163,159,196,227,6,27,59,251,31,244,232,115,54,159,231,121,70,88,185,14,205,187,110,87,175,205,140,142,188,140,117,224,253,63,120,117,125,118,193,237,188,99,225,111,11,235,48,
18,90,57,180,219,223,179,92,133,57,1,210,55,24,82,113,144,85,144,123,210,104,254,1,188,189,91,152,52,33,107,62,143,122,190,69,222,159,119,108,176,79,134,3,56,96,118,146,48,167,114,145,200,206,73,234,163,
156,208,74,211,105,53,230,154,255,0,129,243,177,149,74,21,33,43,73,51,230,27,189,70,238,75,49,103,36,141,243,0,196,176,193,13,209,212,254,88,232,62,131,21,155,103,49,142,70,102,218,96,202,137,35,223,180,
200,9,229,71,175,78,125,1,205,125,41,241,127,225,212,22,99,85,184,212,81,237,53,120,34,55,147,92,52,69,68,177,34,0,242,16,6,60,207,184,72,3,12,65,199,83,143,159,52,123,27,153,174,196,137,100,39,97,137,
30,54,56,69,35,174,91,60,12,142,162,187,232,226,105,85,135,60,118,48,73,243,104,118,179,221,64,124,26,32,116,76,73,56,184,18,18,251,173,185,57,133,56,218,67,238,44,119,100,252,189,121,226,15,18,72,205,
240,191,67,72,167,137,227,51,93,27,156,143,222,36,211,56,49,168,29,246,198,153,61,128,34,177,228,211,174,188,69,226,91,109,62,24,227,138,43,134,16,43,65,109,228,70,238,20,179,5,86,61,112,91,231,61,72,
201,237,93,71,142,238,173,53,73,180,221,55,71,95,47,73,83,37,218,72,99,203,73,189,65,137,136,3,118,118,196,14,222,219,151,223,18,227,239,71,239,29,73,41,92,247,127,14,184,241,149,132,31,17,252,71,225,
201,108,180,221,47,195,66,211,72,135,76,189,104,109,177,166,171,172,147,74,70,28,144,166,83,183,1,29,178,189,179,82,248,118,219,72,182,213,60,99,167,105,154,28,218,71,139,150,25,174,6,157,59,179,165,236,
107,23,156,6,118,185,70,123,121,150,111,45,57,220,174,7,122,225,254,18,232,250,206,187,61,199,193,233,175,158,249,252,73,165,160,209,174,236,92,178,105,243,180,110,235,17,206,63,112,248,104,221,120,1,
200,101,228,146,61,11,227,183,140,108,15,196,61,71,196,222,24,183,130,218,250,202,211,75,212,103,178,184,98,151,22,154,133,189,185,183,187,183,126,185,2,56,34,86,85,28,172,188,103,117,121,143,13,86,117,
42,74,109,167,127,118,205,165,111,77,157,186,171,95,228,209,215,22,149,172,244,107,94,233,245,127,214,135,151,235,247,190,34,191,210,108,174,172,245,99,164,91,90,216,71,117,108,52,217,93,160,184,193,101,
158,21,63,116,224,108,117,140,134,220,11,125,238,220,22,159,172,65,46,165,61,196,48,216,88,235,169,109,33,72,237,109,246,197,43,33,35,50,70,135,239,177,35,152,199,37,115,142,213,208,15,27,77,225,109,66,
103,240,237,174,149,63,133,110,150,73,151,73,212,44,252,245,150,57,79,54,183,17,179,5,113,27,150,85,219,130,0,4,63,44,42,41,188,75,107,171,248,142,208,120,97,46,252,27,164,223,92,253,156,67,111,33,187,
134,206,76,22,127,37,246,199,35,33,62,88,11,35,97,11,31,155,104,174,245,67,154,242,183,233,250,51,154,110,75,220,190,159,214,198,134,189,224,171,203,175,136,58,221,230,151,105,191,72,140,91,170,94,106,
16,136,45,132,166,218,39,201,101,225,152,0,205,180,114,79,92,117,172,141,7,194,203,62,183,106,177,24,110,66,222,69,117,116,210,79,27,51,198,179,171,184,88,215,160,219,147,180,243,206,51,94,173,119,168,
120,31,78,208,102,117,188,251,69,174,154,137,11,188,119,146,220,53,195,177,8,249,59,153,85,137,36,144,56,60,168,36,214,199,194,13,83,86,145,117,45,78,254,195,77,210,252,53,44,121,177,130,59,80,46,112,
62,227,22,39,133,97,234,50,73,24,198,57,243,49,25,142,38,43,154,49,180,86,215,210,254,139,119,248,33,74,42,46,221,78,86,233,97,211,116,189,88,207,106,239,166,55,153,44,23,215,176,186,71,12,104,237,35,
44,35,156,5,46,248,92,12,237,199,35,21,231,126,63,214,110,173,117,136,180,232,101,55,150,150,178,47,150,199,33,179,24,92,109,153,121,3,105,10,84,126,57,175,165,60,126,250,63,138,180,173,47,195,246,119,
16,219,53,138,202,211,51,219,74,26,238,105,31,179,5,0,4,70,227,39,7,230,239,95,60,15,5,52,90,181,230,159,121,115,191,77,216,242,195,53,187,249,109,20,133,66,2,115,198,208,81,131,175,39,59,72,224,240,240,
181,27,253,237,87,103,253,126,62,134,111,178,40,248,107,67,213,188,75,225,77,126,246,218,20,181,210,52,39,185,212,167,243,110,130,68,209,8,81,229,130,53,63,126,115,184,72,170,64,5,119,0,195,145,93,206,
141,166,141,51,194,218,165,197,158,144,243,106,30,77,188,145,220,6,103,251,109,179,134,84,63,40,98,128,109,12,88,169,10,68,156,142,181,231,122,143,132,44,116,95,22,233,208,62,187,107,168,104,55,115,149,
107,192,26,25,12,67,31,188,32,253,197,87,224,146,112,189,79,183,170,89,218,120,223,76,241,26,105,43,253,169,7,217,218,221,12,23,236,45,238,109,92,187,77,28,50,75,31,47,8,14,36,86,27,212,150,96,221,72,
174,170,205,74,43,145,255,0,193,242,52,165,36,157,217,177,241,107,87,241,20,222,6,240,77,164,19,95,120,115,75,242,229,127,236,209,52,95,104,97,40,86,141,212,35,30,62,254,16,144,112,192,129,144,5,124,247,
227,9,111,181,173,101,27,85,146,59,38,141,85,89,2,55,158,203,180,17,185,78,54,156,99,59,179,201,230,190,158,241,252,81,120,167,75,211,117,59,95,10,233,86,82,71,118,176,46,162,214,161,163,134,80,50,36,
183,157,126,95,222,56,7,99,1,207,33,184,219,95,56,124,79,211,26,47,137,186,204,43,114,183,145,153,131,9,237,217,218,41,113,26,150,100,102,254,29,196,174,57,195,43,115,233,209,131,149,55,4,151,75,233,213,
121,26,86,77,202,253,254,227,156,154,9,164,179,121,162,121,133,133,152,88,208,72,65,56,39,106,147,142,50,115,138,167,164,234,176,218,221,60,127,102,87,182,146,3,12,128,177,221,130,121,111,118,252,187,
85,125,107,89,123,171,104,237,173,213,97,181,1,119,44,99,2,86,92,237,114,122,147,205,98,166,124,222,73,207,181,119,211,186,87,103,60,210,110,200,238,111,174,97,150,213,22,86,117,123,127,146,56,216,146,
196,16,8,118,29,55,31,85,192,245,172,0,15,152,100,218,158,80,251,204,199,60,122,99,189,103,73,44,205,38,226,217,144,242,121,201,171,246,141,31,152,158,123,146,202,6,220,227,0,227,57,246,173,22,172,134,
105,197,169,220,157,22,123,75,120,188,177,35,43,249,185,218,35,193,232,163,25,57,7,28,158,56,197,99,219,68,145,153,25,228,221,38,72,201,57,201,250,214,133,180,202,80,137,9,145,200,59,198,122,231,166,43,
46,105,15,32,97,121,207,28,211,107,65,18,39,49,130,1,7,111,92,245,174,215,195,190,5,212,117,68,150,68,154,218,76,68,39,142,52,98,76,209,20,4,72,8,251,163,36,12,17,156,250,142,107,133,179,157,62,218,136,
214,98,239,114,182,97,220,203,187,229,60,130,188,140,117,207,183,60,115,93,70,151,226,73,116,59,147,38,141,124,242,68,233,11,186,200,152,204,162,61,172,8,238,160,18,7,111,231,88,212,231,183,185,184,153,
214,89,105,26,78,145,166,220,93,207,109,229,217,91,78,142,183,183,163,100,147,46,210,195,203,86,225,100,14,10,228,12,16,120,228,241,207,234,190,23,190,127,13,91,248,141,228,255,0,143,213,154,238,84,148,
146,255,0,51,131,18,140,12,238,96,199,131,198,70,51,93,166,141,117,39,136,252,27,45,192,181,134,35,167,94,37,209,242,161,15,27,50,245,83,184,146,191,43,144,125,65,174,246,226,206,15,236,33,165,219,234,
81,197,253,159,182,222,36,109,196,70,67,175,44,115,156,48,8,220,158,61,120,175,54,120,153,83,118,123,223,82,57,172,124,249,7,133,117,241,111,103,51,105,166,59,107,183,85,130,73,165,84,39,114,150,86,35,
146,170,66,158,72,24,198,49,154,151,65,186,134,218,238,91,143,179,196,232,170,82,36,149,119,252,220,124,217,63,194,8,36,14,153,57,205,123,204,54,47,121,225,235,75,45,66,209,238,229,119,50,44,141,38,211,
12,146,140,20,83,145,183,106,18,187,250,130,249,245,207,159,120,167,225,157,229,189,218,95,198,214,43,167,148,95,38,8,111,195,50,140,12,150,60,237,195,99,32,103,183,78,131,74,88,175,104,218,158,134,148,
212,166,239,99,206,239,128,242,90,72,182,69,28,140,112,177,99,146,78,114,61,185,172,155,132,242,118,59,62,246,192,99,27,169,35,7,177,206,63,74,235,82,205,34,150,226,17,120,223,109,7,203,132,91,70,38,243,
9,224,252,217,1,65,25,193,239,237,88,58,133,133,204,122,146,218,75,12,205,122,196,16,166,65,51,224,142,16,237,36,103,190,7,64,69,109,77,247,53,156,76,150,220,202,215,12,236,29,223,0,12,141,195,191,62,
220,12,84,25,115,128,51,131,238,106,237,202,72,97,137,218,72,219,141,161,19,36,198,57,225,184,192,61,241,212,245,170,193,246,177,30,168,80,123,102,186,19,100,17,110,206,121,39,20,43,102,93,220,28,115,
78,104,208,65,27,35,59,72,197,140,139,229,224,38,15,203,206,126,108,142,125,186,84,107,144,196,17,138,97,177,191,225,107,216,109,117,34,110,38,146,221,241,152,110,226,98,178,90,191,36,58,17,206,236,224,
122,227,167,187,155,81,187,22,49,219,106,2,107,168,4,194,100,89,46,220,21,112,2,156,100,149,97,142,48,71,126,184,226,176,198,124,183,98,50,1,4,158,227,60,127,58,35,44,235,179,118,216,137,231,35,59,126,
162,167,149,5,217,37,237,226,73,44,146,37,172,16,198,88,144,144,194,19,3,36,140,129,193,63,74,210,54,210,65,1,149,147,126,27,10,87,12,164,142,115,244,224,143,175,29,235,42,231,202,89,7,204,54,46,2,42,
156,238,3,161,39,176,63,157,91,210,228,184,113,53,180,110,235,20,193,145,143,150,206,35,4,131,184,1,208,130,23,230,237,154,44,173,160,51,94,25,32,188,251,60,87,145,71,111,18,204,166,91,136,96,243,89,99,
28,141,232,14,101,92,224,117,28,19,207,90,213,213,44,98,187,181,190,120,204,194,230,20,89,87,201,31,104,137,151,158,21,148,22,83,199,71,232,114,6,120,170,26,79,153,5,149,169,19,71,19,198,10,78,146,31,
45,145,89,179,181,143,81,143,195,142,49,93,54,159,107,120,214,214,247,105,228,73,107,242,164,88,154,69,116,228,174,67,1,150,193,28,131,192,200,61,184,194,114,229,119,42,16,93,142,50,235,72,186,183,182,
190,147,49,21,131,229,101,12,119,197,156,98,76,99,238,130,64,252,106,239,137,111,173,46,151,72,75,69,186,142,206,206,201,98,138,27,167,243,60,172,28,159,155,186,146,88,129,158,1,3,140,86,199,141,52,139,
184,180,251,11,137,132,77,40,221,246,235,135,31,189,138,82,196,58,187,112,76,92,135,80,121,198,123,98,153,227,43,77,54,219,81,125,46,214,226,234,59,187,117,104,202,92,38,4,140,20,20,126,71,0,140,96,15,
239,12,214,138,170,118,123,148,227,101,98,253,134,135,109,127,171,120,122,20,150,192,171,121,78,246,111,171,24,192,142,70,44,75,73,183,204,249,84,49,147,0,180,106,163,0,230,184,207,24,141,62,199,196,186,
133,134,155,119,109,123,97,109,59,71,21,204,65,204,87,40,14,55,46,255,0,152,161,237,187,146,48,127,138,189,183,194,122,39,138,60,55,240,75,197,254,45,179,215,52,40,108,181,253,9,126,209,167,220,70,36,
184,123,51,115,229,44,224,146,60,137,139,150,48,242,75,13,220,100,0,62,119,184,147,204,184,144,49,85,102,115,206,120,247,252,61,234,160,249,155,96,227,203,20,187,158,237,251,31,248,82,207,197,31,21,231,
142,230,194,207,80,177,211,52,151,191,150,215,80,211,197,220,18,226,68,83,27,198,24,16,8,220,162,65,157,133,148,144,121,169,239,124,53,226,187,253,74,45,27,194,58,78,161,99,229,195,119,21,253,130,76,130,
91,105,162,184,117,150,25,38,200,12,10,152,240,89,176,195,118,73,21,233,95,179,71,132,62,32,120,7,225,175,142,124,93,22,131,44,55,190,43,240,156,150,122,28,230,227,108,145,23,109,202,230,61,164,130,199,
27,73,249,64,82,88,129,215,149,248,149,227,51,164,234,90,222,137,167,121,58,231,138,174,229,107,157,69,227,182,205,164,119,5,84,42,198,163,230,157,201,49,141,196,99,149,199,77,181,197,90,163,114,106,158,
175,167,149,186,182,116,211,92,176,215,67,219,191,102,239,8,193,224,13,82,51,226,155,15,11,235,55,247,86,169,37,182,153,34,139,201,173,217,80,25,11,55,221,10,50,56,36,142,71,173,122,15,197,109,71,79,241,
68,55,122,38,157,162,217,88,199,113,183,100,118,86,145,161,4,145,183,1,23,25,36,143,192,143,90,240,27,105,52,255,0,15,235,55,94,24,155,80,158,230,93,43,75,254,219,241,238,174,242,133,184,186,88,176,203,
97,22,220,133,67,35,8,202,175,24,47,201,98,77,105,219,106,183,250,167,195,255,0,15,120,144,107,77,22,169,172,95,221,223,52,81,204,35,30,98,200,225,45,178,57,253,209,130,48,161,126,243,5,207,4,131,243,
243,194,85,196,98,19,122,38,251,111,163,179,244,211,69,218,205,238,41,46,88,190,231,16,250,36,55,210,178,235,19,219,27,242,62,127,50,32,85,241,140,158,221,55,15,113,156,244,171,151,26,69,245,142,133,46,
155,162,93,165,187,93,64,208,95,71,18,40,55,81,44,134,72,163,218,224,240,165,152,229,89,78,78,115,218,188,199,77,187,212,175,172,151,73,17,72,158,33,179,180,58,154,9,3,70,100,242,137,200,82,196,110,33,
11,38,228,27,93,67,14,113,199,65,225,79,16,93,94,88,218,184,158,41,244,123,156,172,15,33,12,109,37,234,96,151,186,140,28,163,116,35,24,35,129,87,83,13,136,195,221,210,147,178,233,111,201,125,228,40,222,
222,103,45,173,248,109,46,47,96,183,184,187,150,194,229,110,23,125,221,219,49,17,200,228,40,46,196,239,8,73,4,190,78,210,50,115,131,92,101,213,180,182,26,157,254,159,63,250,251,59,153,32,147,36,55,204,
174,84,242,9,7,56,206,115,207,7,189,123,150,165,117,120,145,205,14,167,109,21,246,154,161,139,249,224,121,136,153,1,179,187,150,193,32,16,121,28,28,145,205,120,118,169,20,86,190,39,213,109,32,136,71,109,
29,193,17,38,49,181,72,12,6,61,57,252,171,212,202,113,149,43,94,19,233,253,122,145,82,13,107,107,22,45,92,172,170,228,28,227,210,190,157,248,37,34,92,42,3,184,131,6,70,15,161,200,25,252,171,229,237,197,
91,53,236,255,0,6,181,31,178,182,21,221,142,220,42,21,232,59,159,192,115,94,189,85,120,153,197,218,71,232,191,195,169,140,254,24,146,20,1,79,221,206,125,186,230,190,127,253,160,245,255,0,18,120,14,107,
47,16,248,74,123,123,125,110,4,186,211,158,73,45,124,233,12,19,5,115,177,114,57,87,138,39,255,0,128,215,171,124,16,188,55,90,63,148,172,65,138,64,237,192,249,215,182,63,74,224,255,0,105,47,14,107,30,36,
107,63,10,232,48,197,113,46,181,173,219,219,79,20,197,145,100,135,153,152,121,202,119,67,143,39,59,198,120,4,96,230,188,232,190,89,223,177,212,245,137,240,159,198,9,52,171,159,28,207,170,69,255,0,18,73,
181,6,18,155,72,213,154,43,82,202,94,67,20,153,2,91,127,48,177,10,57,69,112,56,0,1,206,92,193,60,34,59,77,101,22,22,183,143,117,173,196,44,36,64,131,178,246,104,193,110,83,32,174,114,49,145,94,235,166,
104,13,227,61,82,231,75,240,90,219,255,0,100,235,134,234,246,55,188,71,142,11,255,0,177,149,146,43,101,118,4,164,145,164,132,249,195,253,102,24,176,32,128,56,223,134,250,20,250,70,177,168,248,122,125,
72,199,170,125,168,90,221,233,119,177,121,49,135,96,162,11,149,96,9,143,150,17,57,195,109,18,163,50,152,179,142,136,212,230,143,153,205,40,180,245,60,251,67,153,103,149,162,55,54,73,34,169,33,100,185,
107,127,49,191,184,161,213,134,239,69,56,207,173,117,16,92,233,177,216,59,222,11,139,14,118,25,47,45,26,72,179,145,130,210,194,204,170,1,61,212,83,181,127,12,232,246,122,216,183,187,188,184,218,102,17,
195,110,144,162,94,200,67,152,230,143,0,152,227,154,7,192,120,142,84,140,20,56,224,116,31,10,141,205,173,229,197,132,137,21,181,254,154,178,51,92,74,26,52,110,118,50,191,120,136,45,131,157,235,147,200,
21,201,94,164,33,119,37,230,56,83,114,181,141,79,135,151,254,31,77,66,210,59,153,35,50,221,202,176,67,113,165,74,179,196,210,51,0,170,74,121,114,41,39,143,186,70,78,51,94,153,227,152,60,25,44,211,216,
234,47,123,37,239,154,13,197,133,228,30,92,164,166,211,247,38,193,36,100,31,94,65,199,57,174,6,95,5,220,77,170,68,108,22,211,75,215,17,136,41,169,198,137,111,127,242,229,148,92,70,12,78,251,122,36,137,
27,144,51,200,230,186,15,27,234,122,206,165,164,233,58,78,169,118,182,23,226,29,208,197,171,133,146,211,81,143,59,23,200,187,109,194,55,28,0,140,85,78,118,240,121,30,84,234,70,88,136,184,55,103,219,95,
248,63,52,253,108,174,122,120,104,53,78,81,156,127,53,242,237,247,146,216,248,215,225,178,219,142,53,215,218,255,0,187,141,98,143,9,235,201,82,122,226,185,79,26,248,139,225,135,136,30,22,191,209,245,219,
137,109,137,216,169,124,96,83,158,164,133,32,103,222,184,21,123,139,89,26,17,108,169,47,204,88,179,40,29,112,115,253,61,106,156,95,107,221,48,242,209,88,124,236,195,147,215,166,49,94,204,20,183,75,241,
127,230,113,74,74,250,216,246,125,63,226,199,135,236,164,73,97,240,245,243,186,13,160,253,183,230,233,142,189,248,171,151,63,28,52,77,238,23,193,50,181,211,228,60,146,95,49,243,20,156,149,32,122,144,15,
225,94,13,114,151,168,202,17,99,144,245,59,115,211,214,137,82,229,10,133,100,153,76,123,240,56,39,160,3,147,199,90,165,9,61,108,30,213,45,143,96,151,226,205,147,77,35,195,224,235,75,96,237,147,155,169,
191,30,49,87,255,0,225,103,120,74,24,49,14,141,170,249,238,187,100,43,117,177,24,119,7,45,156,127,74,241,37,243,30,233,163,104,21,157,0,220,205,41,56,31,149,92,183,148,62,82,40,227,222,184,202,176,110,
245,41,77,61,135,207,125,207,87,143,226,135,135,227,73,224,139,194,247,78,179,168,18,33,212,219,231,255,0,123,29,107,18,239,196,222,12,184,213,162,212,229,248,101,103,45,244,69,90,57,165,189,149,138,149,
228,99,131,208,243,92,60,209,92,71,20,243,134,141,136,32,144,16,239,199,108,115,200,250,84,86,87,55,151,0,121,110,161,65,219,243,70,115,156,227,214,143,222,45,131,154,54,61,106,227,226,164,178,198,168,
158,7,210,118,41,59,114,91,228,39,184,1,125,133,99,220,124,74,215,110,157,194,232,218,90,237,192,63,232,252,2,7,185,244,174,7,88,55,22,182,126,123,206,28,35,21,117,141,21,74,241,223,215,183,53,93,99,149,
108,201,138,225,190,127,153,147,142,7,122,73,77,171,255,0,95,144,249,210,59,150,248,149,226,141,132,165,190,147,30,123,125,145,51,252,234,57,62,35,248,207,98,40,188,181,139,204,28,108,133,70,107,205,102,
149,254,213,4,105,111,251,169,176,55,111,233,193,207,111,165,109,46,152,8,73,119,185,0,112,55,158,7,168,170,112,123,147,237,44,236,116,146,120,231,198,227,115,46,178,202,79,116,11,255,0,196,214,101,215,
139,252,95,63,47,174,221,51,177,198,4,132,103,242,21,139,111,4,173,168,27,121,212,52,120,200,96,91,145,245,205,91,123,8,214,1,128,161,177,144,196,103,159,67,71,43,67,246,141,236,207,37,210,52,251,235,
189,86,218,214,218,222,70,158,226,64,145,238,28,103,4,245,244,0,19,244,21,235,62,27,15,105,240,230,218,254,109,81,236,236,223,80,85,176,212,45,221,89,161,104,151,247,129,19,239,22,33,85,66,244,218,75,
115,156,87,33,167,232,207,111,168,88,149,215,132,54,79,57,142,107,203,120,228,113,26,168,59,93,118,141,220,130,84,46,122,183,57,2,189,7,225,60,154,70,151,226,41,211,198,250,67,75,162,222,233,215,54,144,
91,68,23,206,136,200,17,97,221,130,50,168,3,18,160,131,185,179,207,109,43,212,141,72,174,87,127,235,204,138,79,144,139,73,248,157,170,248,115,68,186,212,188,59,121,122,242,139,129,9,185,185,69,242,237,
134,198,109,209,70,65,42,57,3,147,130,79,11,218,190,128,253,159,110,117,143,20,235,175,166,248,159,196,215,218,199,136,229,129,167,22,17,89,167,149,105,243,1,153,167,141,64,119,27,151,228,82,2,238,193,
220,121,31,33,120,180,59,149,211,44,46,47,36,72,231,118,22,10,164,171,28,110,15,181,127,136,40,25,7,176,220,49,130,43,163,248,9,46,165,31,138,161,179,182,186,107,11,43,235,89,62,215,123,230,60,127,98,
211,211,50,221,58,145,128,161,214,48,155,184,193,110,57,53,229,230,57,62,27,19,135,147,183,44,159,218,178,114,251,218,211,229,99,187,9,140,110,162,141,84,229,30,215,118,95,43,159,76,124,81,154,63,19,167,
138,252,3,99,168,219,95,79,164,88,109,186,186,154,80,126,206,242,185,141,18,32,50,88,131,203,115,128,188,228,215,137,106,190,6,212,108,252,61,111,61,134,144,179,68,118,11,197,158,225,130,176,97,130,237,
193,0,6,61,58,96,140,240,42,231,192,91,235,237,115,198,222,63,241,46,155,108,83,89,187,210,238,126,199,112,202,158,78,150,146,96,137,72,60,19,28,81,170,5,29,72,25,192,59,171,222,229,101,211,124,49,227,
131,168,24,6,153,225,232,173,33,146,86,113,48,184,89,45,162,103,4,15,189,131,32,39,25,24,61,115,197,120,181,22,43,42,229,165,65,115,69,114,221,55,119,119,100,246,181,181,106,222,143,67,63,102,171,55,56,
104,245,252,47,250,35,226,189,126,91,182,213,53,75,75,251,57,45,167,186,101,183,120,229,80,210,89,194,178,98,64,157,112,78,15,78,161,120,224,138,245,93,4,218,107,90,190,143,61,181,173,173,173,133,157,
205,243,189,193,27,158,241,102,18,36,59,148,168,33,227,128,196,161,78,56,231,175,21,203,205,224,237,71,251,50,195,86,154,242,234,125,127,85,138,75,137,227,22,108,118,238,70,219,23,154,196,43,72,74,182,
121,28,1,142,58,85,248,117,125,103,30,185,225,67,119,29,186,91,45,229,188,51,92,71,56,83,31,153,40,66,100,7,157,192,57,108,244,194,251,138,250,202,114,133,84,185,30,221,187,236,115,198,60,173,243,45,79,
76,248,95,170,106,151,30,40,127,22,120,122,49,161,31,10,107,26,23,246,158,157,103,51,196,183,22,31,104,143,100,133,24,6,218,172,24,178,240,0,151,230,200,197,109,126,209,158,44,240,230,183,227,223,23,233,
182,177,71,111,168,189,252,150,81,125,134,113,50,221,197,28,140,194,107,133,56,10,25,74,129,181,183,48,110,64,242,193,174,71,192,240,220,120,99,199,254,55,210,111,231,130,230,207,197,154,78,177,225,199,
154,32,74,159,45,194,9,152,227,104,8,208,196,3,115,243,49,199,21,228,118,11,166,223,94,92,206,163,81,134,78,29,97,68,142,4,143,24,249,16,124,204,0,193,206,49,198,123,86,181,41,83,169,43,189,215,245,250,
13,78,80,143,169,99,83,211,244,252,153,238,163,214,94,37,108,188,73,45,186,97,121,249,99,57,125,163,39,56,39,61,123,154,163,3,104,5,5,191,246,62,171,52,68,229,34,150,253,70,205,221,65,1,112,115,198,78,
51,197,116,190,31,211,238,117,221,70,21,210,52,11,125,74,60,128,178,181,252,147,68,31,27,202,200,191,34,185,43,193,82,122,119,167,120,134,11,139,139,219,239,179,232,186,40,107,120,26,41,23,74,177,75,120,
214,76,224,40,249,219,44,113,203,14,131,245,143,108,163,47,102,222,190,182,253,76,92,90,143,49,210,120,63,195,186,99,248,26,247,196,176,105,250,106,77,97,172,173,186,92,95,221,187,4,68,180,243,155,12,
23,145,131,146,135,0,224,253,43,23,68,248,132,147,223,235,112,248,128,9,52,155,230,89,98,138,56,152,60,115,46,66,75,19,15,245,64,97,24,161,202,158,152,4,19,94,141,226,187,13,75,195,255,0,4,53,93,10,222,
233,26,9,239,132,179,199,229,36,106,236,182,241,172,159,42,242,57,218,54,231,253,88,81,212,154,249,146,75,185,22,224,173,196,63,48,234,7,21,52,212,107,38,215,245,233,115,54,218,118,61,123,197,191,19,181,
141,67,80,190,254,197,18,216,233,146,203,230,71,25,41,230,110,40,170,75,224,149,39,229,199,127,148,40,245,172,159,14,93,235,122,213,195,164,146,200,113,243,207,132,103,46,189,247,237,35,43,254,200,192,
174,30,218,109,57,200,15,117,131,220,55,95,124,103,31,165,122,111,134,244,88,32,210,110,175,227,103,249,20,23,242,37,216,249,199,1,119,0,165,187,114,222,192,231,138,229,196,80,167,70,157,163,27,51,74,
16,230,168,153,221,233,151,154,4,147,71,105,127,122,150,211,216,157,241,220,195,104,177,180,37,129,92,225,129,192,195,21,59,129,7,53,237,209,91,106,58,180,126,14,189,210,167,183,214,53,13,34,91,105,110,
26,246,67,7,155,45,187,143,34,234,39,143,59,101,104,63,117,42,176,41,32,84,192,92,182,126,103,210,18,242,89,27,74,148,222,233,235,243,92,6,158,37,196,204,160,43,137,145,199,202,193,0,201,137,240,122,237,
60,103,217,188,15,172,233,186,119,134,230,214,172,46,167,147,95,182,212,124,132,210,111,227,17,226,220,195,135,42,196,225,217,89,149,148,169,198,214,0,170,176,231,197,197,82,196,208,73,225,231,102,187,
217,175,248,31,122,251,236,118,65,170,179,148,231,177,212,254,209,30,15,181,159,90,241,62,165,224,251,91,159,248,69,245,76,155,253,21,82,105,98,130,98,124,205,209,198,57,139,247,135,112,116,82,21,183,
3,144,87,31,9,234,247,87,81,69,117,5,202,220,41,121,63,116,179,22,202,109,225,134,15,67,158,8,224,142,227,154,251,171,194,63,18,97,187,211,52,235,29,82,232,203,28,143,49,181,146,112,18,70,96,124,183,242,
200,193,44,173,193,101,231,160,97,92,191,199,175,131,82,248,163,195,146,120,183,64,213,99,130,127,180,145,169,219,92,204,77,188,151,1,118,164,140,248,205,172,133,62,92,56,242,223,42,84,169,57,59,101,57,
141,74,53,93,28,102,141,236,245,244,93,62,87,126,72,223,19,70,21,96,157,13,82,254,191,164,124,75,28,14,202,8,92,133,198,246,7,133,205,94,88,21,25,38,85,89,89,131,110,238,7,110,149,165,171,104,122,198,
151,119,62,157,127,167,92,90,75,20,132,50,202,191,41,35,143,149,134,85,191,224,36,138,210,188,183,181,146,194,218,56,201,91,184,128,23,50,179,149,218,54,244,32,12,140,242,9,245,199,94,223,93,41,36,143,
33,167,123,51,19,79,183,50,58,249,120,98,195,230,103,25,8,122,83,53,75,101,138,220,36,120,121,91,150,199,33,127,26,208,84,54,154,112,157,217,74,223,21,144,160,83,230,67,201,224,147,193,224,117,232,125,
170,11,56,196,247,114,125,199,67,19,40,86,7,228,206,66,179,38,65,207,3,190,57,172,227,39,171,232,77,146,50,153,167,129,84,157,165,118,231,175,74,143,111,153,25,124,140,55,80,79,53,216,203,167,216,93,221,
249,121,187,211,244,168,148,180,215,23,144,7,113,209,72,140,32,1,142,65,194,19,147,215,142,133,186,188,126,13,178,144,199,97,246,205,64,99,62,117,222,20,183,35,10,162,62,19,35,112,36,228,131,249,85,170,
183,217,92,113,77,171,179,143,135,206,93,201,106,231,204,151,228,101,140,224,176,235,140,250,86,134,149,166,76,101,75,117,182,105,174,24,134,43,9,222,35,7,140,49,28,12,119,25,227,35,154,222,240,239,131,
181,205,127,65,213,181,189,42,206,4,210,180,249,210,41,30,107,128,48,207,202,162,231,151,33,126,102,61,151,158,79,21,223,124,62,240,243,233,240,153,157,31,81,243,45,254,100,180,147,203,104,112,121,33,
129,13,140,19,156,100,140,125,115,21,42,198,41,234,105,26,110,93,12,111,10,217,120,175,79,176,70,130,210,238,219,72,220,228,78,235,228,252,205,206,88,15,152,28,2,6,239,94,56,233,173,224,237,69,37,213,
181,11,57,145,175,165,187,100,50,74,255,0,235,4,187,135,206,59,0,49,147,244,61,115,93,253,206,165,166,174,149,6,146,110,44,117,213,186,177,243,237,226,93,99,115,198,177,56,253,223,158,139,149,45,146,64,
193,57,233,192,192,181,63,128,99,181,190,210,181,29,7,83,91,109,32,202,241,202,151,91,188,216,176,80,132,145,27,230,70,109,192,47,102,193,60,3,131,230,87,170,181,231,86,191,245,235,250,21,42,16,190,135,
63,172,107,215,94,30,176,153,245,24,126,220,172,143,108,232,209,141,178,237,5,143,39,143,186,114,120,232,49,131,144,43,142,184,62,32,241,46,177,254,139,7,219,85,3,91,91,219,220,24,196,118,224,149,144,
172,96,144,160,116,35,105,228,175,57,197,125,3,169,232,233,39,133,231,138,113,111,43,199,9,146,5,104,151,104,249,112,78,7,94,57,39,59,134,57,174,110,247,194,246,74,179,46,138,209,73,119,36,9,117,28,82,
12,73,108,229,92,56,70,32,243,243,31,155,28,103,24,239,89,225,235,194,43,109,77,99,133,148,81,193,120,107,193,246,151,113,69,111,226,53,215,101,187,117,112,154,62,144,176,52,219,198,75,121,161,249,242,
207,7,122,183,247,130,244,4,249,126,189,165,127,99,78,233,139,109,129,213,218,210,210,229,156,68,164,19,178,73,128,4,48,24,7,140,156,134,245,175,160,252,17,29,236,58,44,48,170,155,27,253,54,40,196,105,
20,173,26,170,18,9,43,184,134,137,112,79,25,228,0,59,241,231,158,43,210,140,151,243,216,16,226,246,27,188,45,183,148,171,109,28,15,17,101,127,48,140,249,158,105,192,4,149,199,63,123,21,209,78,171,231,
105,153,184,43,30,49,60,179,62,219,96,89,35,46,90,56,55,16,50,122,224,30,167,24,230,159,99,101,45,197,210,218,198,34,243,220,245,119,11,180,231,24,201,227,63,165,122,159,133,52,41,60,65,96,52,141,26,53,
254,219,190,97,36,151,215,17,178,70,136,153,59,21,31,56,200,32,158,140,220,144,48,50,57,205,63,71,211,44,111,91,73,214,154,218,194,234,219,205,183,184,154,73,93,227,36,166,98,185,82,49,186,61,202,20,40,
60,249,128,158,226,186,213,69,240,245,51,113,107,83,149,150,214,16,150,163,204,70,125,210,43,70,177,20,39,7,230,204,141,242,190,48,71,29,58,119,21,70,250,15,179,73,110,237,134,243,237,214,93,173,252,5,
139,124,191,128,3,233,93,7,138,53,11,43,157,88,182,159,121,118,186,117,165,164,80,218,139,194,36,145,19,24,145,118,129,181,48,51,242,245,192,28,147,140,104,248,251,77,212,116,89,225,210,117,75,63,179,
222,197,166,172,45,230,72,142,210,163,237,150,9,148,169,32,238,141,179,144,123,28,224,130,5,197,187,43,145,37,103,161,196,195,25,148,144,188,129,157,216,254,17,235,237,210,181,13,186,217,89,234,182,119,
154,76,178,234,15,12,78,31,13,186,213,21,183,200,229,71,56,104,217,121,199,0,131,198,115,84,231,184,159,99,91,11,131,246,83,32,148,198,15,202,205,180,12,228,12,158,56,254,153,173,11,253,69,238,227,128,
217,69,246,51,29,157,189,173,198,37,203,92,60,106,87,121,61,126,98,195,142,156,96,230,171,80,51,181,235,59,251,29,90,230,195,83,183,242,117,11,105,60,169,85,182,228,54,7,202,118,241,156,17,211,214,175,
248,113,236,173,243,61,224,148,75,189,151,203,73,89,76,145,148,201,221,179,231,80,78,0,97,145,247,178,56,167,248,117,116,100,212,44,254,223,231,236,103,104,174,118,1,26,70,133,130,176,93,160,177,35,37,
143,76,227,0,228,213,137,110,45,224,251,82,219,233,150,19,72,160,219,172,183,17,27,133,97,208,58,18,120,114,0,57,228,224,251,81,41,116,176,110,80,188,212,190,208,45,173,225,89,166,145,9,104,252,226,36,
155,145,243,47,202,62,97,149,221,154,234,116,59,237,70,239,73,123,104,85,48,214,155,164,138,225,24,121,74,88,4,117,99,128,160,55,241,12,130,9,224,226,185,189,39,85,212,116,232,110,109,173,117,27,203,24,
229,32,220,173,172,158,91,56,94,57,56,7,63,136,173,45,5,53,93,81,101,123,109,20,234,16,91,161,95,51,30,75,38,1,96,60,204,133,37,64,36,71,220,18,112,69,103,81,165,23,38,180,69,193,54,236,142,134,243,86,
48,221,219,206,88,194,129,156,20,19,121,234,172,167,28,142,3,110,32,16,121,192,207,210,185,29,71,89,136,69,60,78,210,94,29,146,237,185,121,78,81,241,251,166,69,63,112,70,9,29,65,35,140,97,69,89,213,53,
103,184,208,82,209,173,35,11,1,98,178,191,50,4,101,0,12,140,100,113,156,99,189,103,120,66,25,110,124,97,162,193,21,164,87,15,37,228,74,45,164,92,172,161,156,43,35,122,130,165,179,237,159,74,116,162,173,
118,131,153,179,161,155,90,184,178,248,121,39,135,124,168,212,92,221,249,215,82,60,205,231,110,71,222,139,180,244,76,50,113,199,32,159,226,53,199,232,86,239,125,175,90,68,150,203,63,206,29,161,96,72,40,
167,115,2,23,157,165,65,4,251,215,173,120,159,194,17,106,254,34,212,222,221,225,185,190,179,134,70,150,198,222,228,139,187,216,162,220,171,113,10,30,31,17,170,150,29,204,109,133,37,171,206,180,127,237,
155,29,17,175,173,222,104,44,34,127,150,224,19,24,144,187,108,62,91,99,46,9,81,144,9,0,117,239,86,154,107,66,156,90,122,159,108,248,167,89,190,212,62,29,233,63,6,252,7,105,52,158,32,23,182,119,94,36,212,
141,214,82,206,57,165,154,100,178,14,196,149,219,182,40,194,224,130,184,83,158,107,231,255,0,12,248,111,89,211,53,173,71,251,34,247,76,139,84,210,111,223,77,138,86,185,243,110,103,189,44,36,41,111,133,
1,217,10,166,210,229,118,50,229,134,5,94,253,144,252,81,127,107,227,107,141,22,41,237,213,188,69,113,102,69,213,228,152,5,237,46,77,219,243,253,230,65,42,130,127,140,140,245,174,103,227,133,172,90,63,
196,255,0,136,122,61,220,243,201,105,15,138,174,230,142,215,204,204,71,123,18,24,140,159,156,134,10,88,140,237,192,174,38,164,170,251,62,150,79,206,247,215,244,58,20,211,247,140,109,67,197,174,158,21,
215,244,189,62,22,23,122,237,252,115,106,83,202,133,240,176,72,210,69,26,185,36,224,202,207,36,140,121,39,104,201,228,142,147,78,213,52,55,22,139,43,221,203,163,104,26,77,245,228,177,188,1,2,187,149,104,
33,93,188,100,206,99,114,195,171,14,191,41,175,43,181,105,165,23,47,30,248,225,104,36,66,177,157,168,136,74,238,207,170,142,50,189,79,30,149,211,106,55,172,218,21,143,135,180,225,246,187,173,94,202,223,
237,74,150,173,191,205,23,13,35,70,58,3,180,70,128,158,119,28,30,48,107,166,81,90,121,28,210,119,109,156,77,238,175,170,92,8,214,123,233,164,242,137,219,243,240,132,130,14,206,234,57,56,3,0,103,128,43,
58,34,202,1,1,130,16,83,32,28,17,220,122,17,237,87,46,210,221,111,10,192,101,16,237,80,124,213,195,6,199,205,199,166,122,119,164,251,44,198,193,165,83,33,142,55,201,64,24,133,82,57,147,208,12,140,31,194,
186,21,172,103,118,119,126,6,241,70,169,113,169,79,162,234,122,171,27,77,94,21,128,203,118,190,105,130,101,66,45,230,83,212,21,96,171,156,242,173,134,36,1,88,86,146,73,53,220,175,50,148,152,177,243,17,
134,10,176,225,151,240,32,143,160,174,122,5,108,99,0,62,120,45,145,130,58,96,143,186,125,235,173,188,181,181,182,188,181,91,103,212,15,157,106,146,202,183,240,170,72,174,217,232,84,144,232,70,8,126,167,
156,244,172,148,97,25,123,170,205,148,229,41,45,89,49,39,3,61,43,210,254,15,75,31,246,132,190,113,5,17,65,219,159,189,147,199,227,94,96,72,252,235,170,240,13,194,195,123,40,220,70,224,7,61,143,53,114,
87,137,61,79,209,111,217,226,237,39,183,212,12,121,198,23,10,171,157,164,100,99,63,133,55,246,135,186,189,177,240,79,136,245,77,59,81,185,179,213,45,173,162,185,138,91,114,3,166,62,71,92,158,129,149,217,
73,235,134,56,231,6,184,191,217,163,84,151,251,70,56,81,213,4,170,138,241,17,203,0,59,127,141,122,143,199,59,8,181,63,11,234,113,0,118,94,233,147,198,196,113,146,23,32,254,98,188,223,134,165,206,184,187,
196,248,175,192,222,29,213,181,127,13,129,100,211,218,90,104,247,210,226,69,112,176,197,103,44,43,20,128,47,67,184,141,152,246,28,103,138,203,248,219,225,47,236,248,52,237,86,93,94,13,70,123,114,44,45,
174,229,155,116,154,149,164,113,9,35,121,84,242,11,91,72,99,36,112,219,9,30,131,214,126,8,107,54,86,190,26,127,14,76,193,245,61,106,206,84,89,90,47,49,99,243,27,206,137,92,14,25,144,32,200,200,234,121,
231,3,67,226,238,186,60,85,115,167,248,35,69,240,172,51,49,130,44,125,158,47,52,192,209,67,228,143,45,84,3,128,132,144,189,179,92,209,197,254,247,149,62,239,112,133,53,40,55,35,196,117,29,38,254,235,66,
158,43,171,120,164,154,253,197,221,149,250,179,188,144,205,27,40,146,194,105,155,12,101,64,80,199,35,31,157,36,28,149,98,69,109,6,250,61,58,250,235,80,254,203,55,136,128,45,252,51,198,196,202,142,187,
91,107,245,4,228,140,55,94,1,236,71,168,233,94,6,241,172,31,106,179,127,10,234,211,195,44,66,41,45,231,129,188,167,34,47,44,75,180,144,4,129,48,55,117,194,175,160,171,16,252,45,241,156,22,229,228,240,
134,176,203,52,38,9,36,148,140,176,98,27,31,123,159,153,65,205,105,40,169,180,230,215,157,157,190,123,247,249,162,83,140,91,229,252,143,40,190,184,190,209,163,186,211,99,154,121,52,221,70,202,55,66,231,
228,188,132,128,209,179,14,196,99,42,221,84,131,130,58,28,153,245,91,201,173,133,157,227,139,155,125,167,230,43,243,183,179,115,134,60,156,28,103,174,73,175,109,147,225,23,142,245,13,62,222,33,225,41,
213,33,12,176,51,205,26,144,140,114,84,229,249,92,228,143,76,159,90,138,31,128,222,59,147,228,255,0,132,126,220,237,3,57,186,143,140,244,239,78,244,35,202,157,155,93,116,31,61,70,222,246,125,15,4,131,
247,103,107,70,26,54,80,172,56,37,126,190,221,170,197,148,105,12,209,184,100,120,153,14,238,121,7,183,212,115,94,213,162,252,8,241,205,205,236,109,246,93,42,40,212,144,124,251,188,115,200,231,10,107,170,
255,0,133,3,226,229,132,179,92,104,145,48,57,56,184,115,252,146,182,142,34,157,237,205,249,153,184,75,249,79,153,162,11,27,171,169,10,229,118,228,131,180,242,122,31,108,254,53,233,63,3,109,124,51,22,177,
38,171,175,93,201,113,97,50,92,105,178,88,91,233,51,221,204,205,50,47,239,191,119,242,194,83,156,22,201,63,54,0,235,94,177,162,252,6,214,110,181,107,72,245,29,83,77,91,79,57,60,255,0,36,185,115,30,126,
96,188,12,18,50,1,237,156,215,218,158,31,210,116,125,34,194,222,203,77,211,237,237,109,32,27,86,56,80,5,199,78,221,120,239,89,226,49,81,86,140,117,191,200,170,84,37,39,119,208,252,150,107,72,23,89,184,
141,100,19,194,234,0,145,1,1,200,232,195,211,61,113,86,45,52,181,129,228,144,40,44,196,18,65,224,241,143,195,138,229,62,43,92,221,233,127,17,188,71,164,249,175,3,233,218,132,246,155,20,109,1,82,86,9,249,
166,195,248,215,34,250,181,216,224,223,220,6,199,64,199,7,241,21,217,26,77,164,238,21,44,164,209,237,230,43,79,177,126,244,98,85,92,14,0,63,159,244,53,130,82,194,222,38,142,54,140,38,62,232,97,199,173,
121,51,234,83,54,51,119,51,30,224,185,168,141,247,32,179,74,72,245,106,211,217,221,221,191,192,206,241,74,199,176,155,120,175,80,173,166,194,93,78,236,144,21,128,235,146,120,52,183,250,115,105,218,66,
223,92,148,22,142,230,48,233,32,124,190,62,233,199,78,149,197,248,46,241,174,96,150,54,119,43,17,39,110,227,131,158,167,21,219,124,121,143,86,147,194,63,14,245,173,71,80,123,211,117,167,220,219,188,139,
20,105,143,42,112,35,66,80,0,197,81,176,9,249,177,223,173,115,197,165,83,145,162,237,117,116,206,110,238,251,74,193,104,229,92,142,195,189,44,94,37,178,141,112,85,206,58,96,12,87,155,60,192,127,11,18,
121,228,212,34,70,223,247,240,51,222,186,35,74,43,169,155,149,250,30,151,55,137,44,27,37,32,144,18,115,212,14,106,77,63,87,138,238,70,141,70,211,215,44,107,204,188,217,3,13,174,62,130,182,60,45,118,241,
234,136,174,216,86,24,231,181,41,211,74,55,142,227,140,186,51,255,217,0,0};

const char* RenderingTestComponent::demoJpeg_jpg = (const char*) resource_RenderingTestComponent_demoJpeg_jpg;
const int RenderingTestComponent::demoJpeg_jpgSize = 111719;

// JUCER_RESOURCE: demoPng_png, 15290, "../../../../../../../../Users/jules/Desktop/DemoPNG.png"
static const unsigned char resource_RenderingTestComponent_demoPng_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,140,0,0,0,86,8,6,0,0,0,29,101,126,119,0,0,0,9,112,72,89,115,0,0,11,19,0,
0,11,19,1,0,154,156,24,0,0,0,4,103,65,77,65,0,0,177,142,124,251,81,147,0,0,0,32,99,72,82,77,0,0,122,37,0,0,128,131,0,0,249,255,0,0,128,233,0,0,117,48,0,0,234,96,0,0,58,152,0,0,23,111,146,95,197,70,0,0,
59,48,73,68,65,84,120,218,98,100,160,30,96,68,195,76,72,24,89,140,17,77,223,63,32,254,143,70,255,67,226,195,212,12,58,160,163,163,192,112,252,248,20,6,38,38,102,134,55,111,62,50,240,240,112,50,176,178,
178,48,252,249,243,7,136,255,49,48,2,125,202,206,206,6,246,198,143,31,191,24,254,3,125,35,38,38,204,240,244,233,75,6,102,102,38,134,159,63,127,51,60,124,248,146,129,139,139,157,97,235,214,19,12,143,30,
189,98,48,53,85,103,200,204,156,128,213,190,89,179,138,24,230,205,219,193,16,23,231,6,182,251,247,239,191,12,2,2,60,12,191,126,253,102,48,48,80,97,248,242,229,59,208,158,159,12,188,188,220,96,243,65,118,
130,0,19,19,35,3,27,27,43,208,77,127,25,62,124,248,204,32,44,204,15,118,203,185,115,183,24,56,57,217,193,248,196,137,171,12,147,39,111,96,40,43,139,96,72,75,235,133,235,69,7,0,1,196,66,165,132,2,75,8,
204,72,152,9,137,141,45,65,49,64,19,4,58,254,11,197,127,160,9,229,47,84,253,255,193,154,112,70,18,0,8,32,22,10,18,9,3,82,9,194,12,53,11,134,33,137,132,145,145,153,135,139,139,131,135,137,129,79,130,157,
89,76,137,147,133,239,195,31,166,223,172,204,76,204,223,127,253,248,7,148,254,247,135,145,241,247,163,207,95,223,255,98,102,251,244,241,231,207,47,63,126,254,250,5,53,31,148,80,126,67,19,14,114,226,249,
135,148,184,70,1,157,1,64,0,177,80,80,162,48,33,37,16,54,36,54,147,58,31,187,132,190,176,128,150,40,195,31,25,35,17,110,77,101,182,223,42,66,172,255,20,152,153,126,113,178,75,112,252,102,98,97,96,249,
245,150,241,223,159,95,255,254,254,253,199,244,251,219,23,158,119,31,24,217,95,221,254,202,117,235,236,135,159,87,175,252,102,60,127,237,237,167,103,223,127,255,253,137,84,218,252,134,98,228,210,103,52,
209,208,25,0,4,16,11,137,9,133,17,169,186,97,5,98,118,104,98,97,226,224,226,226,209,226,100,81,12,22,102,117,52,227,254,229,166,166,240,95,147,75,150,139,157,149,7,168,64,138,143,129,153,135,141,129,137,
157,153,129,153,139,133,129,145,153,145,225,239,247,191,12,255,254,254,99,248,247,243,47,195,159,15,191,36,255,190,255,161,109,242,138,193,209,255,41,243,159,167,175,254,61,188,41,37,113,250,228,95,174,
131,203,110,63,62,249,229,231,175,55,64,181,32,123,126,65,49,35,82,105,51,154,112,232,8,0,2,136,133,196,82,133,25,169,68,1,37,22,14,102,96,43,207,94,90,66,39,77,150,63,90,155,227,131,163,160,224,55,17,
97,123,25,6,118,5,30,6,70,78,86,104,139,4,24,159,255,128,248,47,144,243,23,88,56,252,254,207,192,12,52,129,25,212,42,4,38,36,118,9,110,80,203,12,28,239,255,127,252,101,17,123,246,69,89,245,234,71,101,
187,187,31,195,35,4,101,206,46,121,243,103,225,154,59,207,246,125,248,241,235,27,52,193,124,135,210,191,209,26,206,163,128,198,0,32,128,136,73,48,200,13,90,88,66,1,151,44,138,226,226,146,9,50,66,193,190,
252,31,35,100,245,222,74,243,26,9,51,176,137,202,48,48,130,34,31,216,90,103,120,253,17,90,123,252,7,165,14,160,14,94,6,6,110,49,96,146,99,135,152,252,27,24,255,191,190,48,48,124,254,4,140,114,160,58,22,
62,6,70,22,38,6,14,121,126,6,14,21,1,6,190,247,223,24,249,206,189,51,41,59,249,85,219,138,85,116,235,212,231,223,231,157,125,241,238,38,212,45,63,160,9,231,55,82,21,197,48,154,112,104,11,0,2,136,133,200,
146,133,5,90,5,113,2,49,23,63,63,63,119,172,170,180,71,24,251,151,4,5,241,215,58,226,126,226,12,108,192,72,102,248,241,15,92,122,48,124,127,3,140,82,160,114,41,29,160,106,41,96,41,34,15,212,41,7,76,48,
146,192,4,194,198,240,255,205,107,160,90,96,98,145,6,38,30,30,110,6,70,182,175,12,12,183,22,51,48,60,61,4,180,13,152,168,190,255,1,39,7,54,96,119,79,196,89,154,65,64,239,59,39,255,158,231,33,58,87,24,
172,151,243,139,77,157,245,240,253,134,175,63,126,195,170,198,239,80,119,254,129,86,83,116,3,90,90,10,12,255,128,37,39,184,112,28,33,0,32,128,88,8,148,44,76,72,85,16,40,177,112,178,178,177,113,149,104,
43,36,68,178,63,43,16,117,227,97,225,53,82,97,96,4,69,211,7,104,124,253,1,70,190,146,7,3,131,124,48,195,255,23,192,4,116,15,88,32,252,249,195,240,255,246,78,6,134,199,64,246,219,151,12,255,63,188,4,138,
1,227,153,79,132,129,129,87,152,129,81,223,137,129,41,174,10,82,56,60,61,2,76,154,60,208,18,8,168,255,215,63,96,193,195,193,32,22,169,194,192,113,246,149,100,230,182,23,77,50,236,34,178,181,55,223,206,
248,246,243,23,44,65,35,131,191,244,42,101,242,243,131,193,99,27,44,44,44,35,38,193,0,4,16,11,129,6,46,72,158,3,86,178,136,10,9,10,149,42,11,39,4,138,60,73,148,13,145,100,97,147,1,150,42,159,127,67,218,
39,160,56,250,11,76,4,134,165,64,182,37,195,191,169,93,12,255,15,111,0,202,1,171,37,198,255,144,10,3,220,10,2,22,12,160,0,102,7,26,255,245,57,3,195,167,39,12,255,175,93,96,248,247,250,57,3,83,94,50,3,
195,51,96,130,249,15,29,114,249,243,13,82,85,129,74,46,160,62,62,19,80,149,199,201,20,188,252,65,58,219,127,33,158,170,59,239,39,124,254,254,243,3,90,87,255,63,189,218,52,223,190,253,28,113,85,18,64,0,
177,224,72,44,176,198,45,59,44,177,136,9,241,11,53,107,201,20,120,138,60,137,146,142,87,96,96,226,2,166,163,143,191,160,73,11,72,252,250,12,172,98,172,24,254,179,0,19,75,158,31,3,195,155,59,12,12,130,
64,53,140,220,144,4,192,200,136,105,19,27,11,68,156,253,55,195,255,203,59,128,49,144,199,192,32,160,12,108,251,92,1,138,241,49,48,200,58,48,48,136,153,1,93,2,44,224,94,28,7,150,80,251,25,56,36,88,25,164,
146,85,24,252,22,221,143,254,253,95,248,87,221,189,247,83,191,124,251,142,62,248,55,218,237,166,17,0,8,32,38,60,189,33,88,53,196,205,203,203,45,80,161,44,150,234,204,254,40,82,60,88,134,129,137,19,152,
16,190,253,65,77,4,255,129,241,195,175,192,192,112,227,2,48,177,220,5,70,52,176,45,194,8,77,143,216,18,11,92,223,127,136,181,255,25,33,205,215,255,64,66,64,150,129,193,102,6,208,230,88,96,41,245,138,225,
255,238,251,12,255,185,162,24,24,236,129,98,127,249,129,53,214,47,6,241,112,89,6,95,137,111,177,69,178,124,49,192,210,7,152,42,25,120,160,238,101,67,234,250,51,210,42,224,128,205,56,6,54,96,130,255,63,
194,146,36,64,0,177,224,40,93,88,161,85,17,15,19,11,11,119,153,186,116,156,47,223,171,100,153,24,25,70,54,49,96,188,124,3,54,19,254,1,91,165,127,128,152,137,25,146,153,255,1,171,143,47,207,128,9,197,6,
168,147,3,210,149,38,182,199,254,15,168,159,157,139,225,63,211,79,6,70,94,25,6,6,245,2,134,255,139,150,50,252,91,213,3,52,255,55,164,156,88,197,205,192,20,211,194,192,232,63,143,129,225,88,38,3,187,200,
11,6,133,20,5,150,132,149,79,50,31,255,19,123,62,255,246,243,237,12,152,243,80,52,27,17,238,237,205,97,208,212,148,103,248,253,251,15,120,46,102,164,0,128,0,98,193,147,88,64,185,150,35,68,73,194,201,159,
227,117,146,84,144,4,19,187,172,0,48,81,0,235,237,63,192,174,176,152,46,176,202,240,4,170,146,135,36,150,251,235,192,189,28,70,147,16,6,70,73,29,134,255,207,207,1,147,27,23,180,125,67,160,31,246,239,47,
3,35,23,23,48,177,0,19,163,92,37,195,255,249,115,24,254,173,104,103,96,16,226,2,13,9,66,27,192,191,25,254,77,47,100,96,124,255,26,216,64,238,101,96,56,152,192,192,2,116,142,180,175,4,123,214,179,39,37,
143,165,4,222,237,121,246,225,40,3,98,46,138,198,137,102,100,78,107,1,4,16,19,150,4,3,42,210,185,64,197,187,190,140,152,122,150,48,115,150,140,13,23,15,151,134,16,48,177,0,115,251,79,96,91,69,209,157,
129,65,165,137,225,223,241,79,12,255,102,0,75,130,13,192,106,72,22,216,203,49,169,6,182,107,142,48,48,166,86,2,195,147,25,220,59,34,10,0,115,41,131,28,176,237,34,105,204,240,127,239,65,134,127,203,58,
161,237,31,102,72,149,5,194,172,192,116,44,192,201,240,127,69,27,195,255,237,251,25,24,76,27,129,13,230,79,12,108,178,188,12,138,86,92,162,133,82,156,105,124,156,236,18,48,183,35,85,77,84,175,150,216,
217,89,25,20,20,196,193,165,203,72,3,0,1,196,132,165,116,97,7,7,58,19,19,79,12,247,255,64,21,217,207,42,2,118,192,106,226,43,48,211,254,124,207,192,32,101,196,240,95,36,150,225,95,77,18,195,255,137,192,
170,99,239,60,134,255,11,107,25,254,101,185,48,252,191,5,212,42,99,207,192,104,164,206,192,232,16,6,140,208,239,68,68,215,127,112,19,149,81,223,158,129,225,253,51,134,127,139,90,129,46,248,15,25,199,65,
46,24,64,137,6,212,195,18,224,96,248,55,179,24,216,77,7,230,112,195,92,160,29,239,24,248,109,36,25,84,133,191,153,186,138,240,152,193,186,255,12,136,249,45,170,183,101,108,109,117,25,156,157,45,192,203,
9,70,26,0,8,32,38,36,26,185,161,203,105,38,204,167,225,40,198,226,33,236,34,6,108,187,2,165,126,3,219,43,124,192,12,172,10,76,36,125,13,12,255,175,3,123,45,226,192,42,132,31,216,184,21,229,97,248,255,
229,33,195,191,150,56,134,127,135,111,49,48,240,170,50,48,70,151,48,48,8,11,3,245,253,196,223,232,5,149,66,108,192,210,68,207,142,225,223,166,89,12,12,207,129,13,102,78,104,207,10,75,218,2,39,36,150,191,
12,255,250,243,129,250,156,24,24,68,12,25,88,184,255,50,136,90,8,49,68,11,50,70,137,241,241,136,34,37,26,86,90,148,50,30,30,230,12,191,126,125,3,175,51,25,105,0,32,128,152,144,122,70,136,210,5,72,251,
10,176,122,41,104,51,8,115,40,10,66,70,94,65,3,114,250,69,12,255,246,159,102,248,127,122,11,48,162,184,161,173,131,255,16,154,7,216,13,230,252,203,240,191,63,135,225,255,242,201,12,140,42,250,192,68,3,
172,162,190,253,6,183,81,112,130,31,63,24,24,21,13,129,145,207,198,240,127,7,176,65,203,197,70,184,87,5,74,80,175,238,51,252,219,186,26,152,128,129,189,167,47,63,24,4,204,69,25,76,181,152,116,2,196,184,
29,145,134,3,216,105,81,202,128,26,185,255,255,143,204,30,59,64,0,161,207,19,129,90,152,172,186,194,124,170,206,18,140,118,124,230,66,144,17,141,31,159,128,173,75,75,96,196,235,48,48,44,239,2,150,32,160,
72,69,235,145,131,186,213,172,64,113,54,70,134,127,211,202,24,254,111,158,201,192,20,146,207,192,104,237,205,192,240,249,27,246,68,0,18,2,117,165,173,128,141,231,179,187,24,24,94,60,134,152,65,40,50,64,
242,92,64,251,79,108,6,38,5,57,160,171,129,133,10,11,35,131,152,147,8,147,63,239,55,15,17,14,86,113,104,98,225,128,102,4,170,37,24,97,97,190,17,89,178,192,0,64,0,33,15,255,195,74,24,78,123,94,54,11,69,
69,22,73,86,41,96,149,3,90,170,199,2,84,162,153,1,108,220,46,102,248,255,234,17,48,26,216,113,84,25,255,33,114,192,200,252,55,173,156,225,255,165,195,12,76,249,83,24,24,37,20,128,37,205,87,204,68,3,170,
142,4,128,85,158,186,62,195,255,125,171,33,19,148,196,2,96,194,250,255,226,26,195,255,71,192,68,198,13,44,5,191,253,98,96,1,186,87,85,146,89,83,157,141,73,141,1,177,244,130,21,234,63,138,99,89,80,144,
135,97,193,130,10,6,62,62,174,17,91,194,0,4,16,19,218,168,46,7,31,7,7,176,217,194,102,206,165,12,12,103,102,160,240,175,143,224,17,215,255,31,248,24,254,111,155,14,108,199,176,224,47,1,64,221,104,208,
56,204,143,143,12,255,38,229,0,35,83,0,216,107,106,3,38,142,255,144,82,8,150,176,64,209,247,21,88,29,153,186,2,35,251,35,195,255,59,231,129,250,56,137,119,57,168,132,251,7,44,158,126,124,129,118,205,255,
49,48,177,50,51,240,136,179,178,121,72,241,25,1,5,57,145,74,24,170,12,228,153,152,168,51,168,171,203,128,215,226,142,84,0,16,64,76,72,237,23,240,210,5,101,110,118,25,57,222,255,42,236,114,188,144,238,
46,104,88,94,41,138,225,255,198,249,192,46,243,107,160,10,14,194,163,26,160,68,195,3,44,157,110,93,1,246,164,114,129,61,166,96,6,70,39,96,91,227,227,87,200,186,23,208,92,18,168,199,195,196,204,192,104,
235,203,240,255,216,14,200,200,9,177,211,190,224,169,8,96,15,69,72,5,216,107,3,54,196,191,189,134,183,109,5,244,5,24,116,4,88,13,57,217,217,249,145,74,25,88,9,67,81,130,81,86,150,2,207,31,49,50,142,220,
42,9,32,128,152,208,186,211,108,214,50,162,26,130,252,127,248,89,69,56,33,75,16,68,13,128,61,32,96,233,114,112,5,48,17,176,146,22,230,252,92,12,255,118,47,97,248,183,115,49,3,83,124,21,3,179,148,28,3,
179,0,176,113,44,46,193,240,143,23,72,91,184,49,48,74,43,50,48,221,60,2,44,137,136,72,136,32,187,65,137,234,55,176,154,252,242,159,129,209,47,3,216,131,123,0,100,191,1,138,131,74,195,63,12,156,138,252,
12,138,66,127,228,204,101,196,229,25,80,151,143,82,212,91,146,145,17,101,72,76,244,24,145,93,105,100,0,16,64,24,171,232,228,153,254,40,242,74,178,48,48,129,86,203,253,249,13,78,48,12,55,46,51,48,188,121,
6,84,193,206,64,210,228,9,176,4,249,207,193,198,240,111,58,176,11,252,249,37,195,67,159,28,134,162,83,239,24,12,215,61,98,16,93,246,152,65,115,213,117,134,234,186,122,134,39,160,201,104,113,113,134,255,
184,74,24,80,35,19,132,65,83,17,239,128,13,240,111,76,12,76,105,61,12,76,46,192,154,231,194,36,160,102,46,72,159,27,52,29,193,193,204,32,40,194,204,41,201,193,42,5,205,8,172,212,232,94,255,253,251,15,
88,186,252,26,209,165,11,8,0,4,16,11,82,131,151,149,133,141,157,75,136,233,143,40,155,40,43,98,132,21,216,3,249,127,249,20,98,86,154,148,4,3,82,11,76,100,44,108,204,12,87,167,214,50,68,157,254,201,112,
233,198,55,184,244,199,123,15,24,218,128,120,181,0,59,195,2,55,25,6,43,126,126,134,63,239,63,0,173,129,218,13,234,142,131,150,117,254,6,141,48,3,53,136,74,48,48,26,88,50,48,121,69,3,171,73,96,252,31,205,
132,148,54,156,66,208,210,9,226,54,14,118,70,22,177,255,191,164,144,252,198,204,128,186,71,138,164,22,171,142,142,34,3,43,176,125,244,23,228,22,6,150,17,157,96,0,2,136,5,169,132,97,21,226,229,225,230,
103,102,228,7,151,46,255,161,93,101,240,114,107,38,72,247,26,148,203,97,99,47,132,27,26,160,21,186,12,255,129,237,149,247,252,98,12,89,155,47,48,92,122,250,21,171,202,219,31,126,50,228,28,124,206,176,
219,67,156,129,239,215,15,240,216,13,35,43,208,78,80,181,197,39,204,192,32,167,206,192,104,236,196,192,232,24,201,192,40,10,76,7,223,143,3,75,61,96,21,41,1,44,253,64,203,97,222,223,3,186,9,180,206,134,
7,156,168,57,68,216,24,164,25,255,74,48,113,112,112,254,251,241,227,51,165,85,18,104,115,217,203,151,239,193,139,165,70,58,0,8,32,22,164,110,53,51,31,7,59,23,31,203,31,94,54,208,36,32,168,87,3,154,203,
121,115,137,129,209,45,144,225,255,110,96,163,247,45,48,114,184,217,32,189,39,144,28,56,248,255,65,26,185,176,68,4,79,76,255,193,187,2,152,57,217,25,118,63,254,204,112,8,71,98,129,129,243,47,191,49,172,
122,202,202,144,105,23,196,240,91,92,133,129,73,89,139,129,145,31,152,88,152,217,24,24,5,128,165,220,47,96,17,115,249,24,144,254,4,196,192,4,245,93,153,225,191,128,41,3,147,148,40,3,131,42,80,238,218,
20,6,134,15,192,46,54,151,16,48,229,51,51,40,113,51,72,137,11,240,241,62,127,241,227,45,3,98,67,29,89,9,6,212,43,2,85,73,163,128,129,1,32,128,88,144,138,106,230,255,255,255,51,179,241,48,49,177,10,177,
65,218,3,108,192,132,243,112,47,3,163,149,43,3,83,235,6,134,255,75,187,25,254,63,185,2,236,14,191,7,134,226,23,200,178,4,80,187,131,157,5,210,155,98,99,101,96,100,231,102,248,15,234,93,49,178,51,252,231,
6,150,16,242,10,12,219,182,93,33,202,49,135,88,101,25,50,67,114,129,118,0,27,178,15,174,49,252,59,119,128,129,225,245,19,6,198,223,63,25,254,127,7,54,54,191,1,219,47,44,144,46,249,127,112,175,138,133,
225,47,176,219,206,96,31,201,192,20,94,199,192,120,165,14,232,238,207,12,140,192,42,144,129,249,31,200,15,172,12,168,59,49,241,86,73,220,192,134,247,215,175,63,70,83,5,30,0,16,64,200,221,77,38,54,38,70,
86,102,22,96,75,149,25,22,166,76,144,241,142,147,192,200,208,78,102,96,108,234,100,248,255,238,51,164,1,252,230,45,164,20,226,3,182,31,68,36,64,43,182,193,51,202,140,192,72,252,15,140,216,255,64,189,140,
28,60,12,255,217,217,25,62,95,200,99,96,56,123,159,160,99,222,92,63,203,240,191,55,158,225,207,157,251,12,140,224,113,21,144,11,129,13,103,112,239,8,24,231,124,156,136,248,6,181,167,64,37,27,104,156,104,
213,100,134,255,34,114,12,140,150,1,192,174,252,44,96,161,196,194,240,129,225,247,215,47,191,126,255,37,118,12,70,65,65,130,97,225,194,10,6,103,231,226,209,170,7,15,0,8,32,22,164,65,250,255,34,172,76,
188,220,191,127,112,253,7,70,6,35,100,52,12,148,140,32,244,153,118,96,162,152,196,192,168,234,207,192,96,148,10,140,6,121,134,255,79,239,50,48,220,187,8,44,13,174,67,22,119,63,186,206,240,255,213,83,134,
255,63,190,3,75,128,191,12,127,64,123,144,120,217,24,100,158,61,33,202,49,226,156,140,12,191,129,250,254,113,115,65,218,75,56,219,166,140,136,222,19,104,92,136,27,216,40,126,114,13,200,118,3,175,31,254,
7,76,200,140,191,126,48,51,254,253,131,126,24,0,214,177,24,49,49,1,134,141,27,155,25,68,69,5,71,19,11,1,0,16,64,40,11,168,190,254,103,252,245,243,15,243,223,127,191,129,109,15,14,88,33,3,12,223,31,239,
33,221,107,45,96,175,228,45,7,195,255,233,211,25,254,159,223,193,240,255,205,99,200,158,34,216,234,58,164,188,12,42,21,64,109,230,63,124,252,12,142,226,124,12,147,136,232,97,57,203,242,50,252,248,14,108,
159,0,75,142,255,196,110,221,128,141,30,203,169,50,64,214,73,252,101,248,245,238,47,168,243,244,255,47,68,14,57,145,96,36,22,121,121,113,134,29,59,186,24,20,21,37,192,167,39,140,2,252,0,32,128,144,19,
12,211,227,247,31,63,189,20,227,126,255,231,253,47,65,102,65,96,241,15,202,160,160,53,48,226,38,12,12,6,221,12,255,22,207,102,248,191,121,26,176,13,243,22,50,249,7,154,40,4,173,170,99,68,138,15,120,227,
23,50,226,251,27,152,248,76,133,89,25,34,53,133,24,150,95,123,139,211,33,118,146,156,12,14,18,236,12,63,222,190,102,224,98,36,161,117,10,181,143,81,76,22,152,176,95,128,221,242,231,235,31,96,149,196,242,
229,199,239,63,255,208,218,44,40,41,86,86,86,140,97,215,174,110,6,53,53,25,134,215,175,63,142,232,73,69,98,1,64,0,177,32,5,228,255,215,31,191,124,127,243,139,227,227,95,208,110,0,112,23,26,88,212,179,
113,51,48,232,86,51,252,155,221,195,240,127,229,36,6,6,33,96,195,66,152,23,181,123,13,143,134,255,104,227,109,192,138,13,88,197,176,124,249,194,80,162,193,195,240,230,63,15,195,238,123,47,129,137,16,181,
97,105,98,106,198,208,98,38,196,192,113,247,16,176,128,98,34,45,226,254,2,75,36,94,81,6,70,69,13,96,47,233,8,3,168,225,243,231,235,23,134,215,63,255,124,248,251,235,215,31,44,245,26,152,47,41,41,196,176,
115,103,55,131,146,146,52,195,167,79,95,25,24,71,211,10,81,0,32,128,152,24,16,107,94,65,253,227,191,47,152,121,94,254,249,249,31,210,251,1,173,129,145,182,6,246,90,62,48,252,223,58,25,88,217,115,65,218,
12,200,221,104,60,173,12,80,188,179,2,99,226,223,251,55,12,194,192,58,102,202,148,73,12,253,254,38,12,94,242,220,12,150,210,60,12,110,10,124,12,157,230,194,12,11,188,213,25,228,99,75,129,213,144,0,3,235,
159,159,144,132,70,172,15,126,0,19,181,146,62,48,17,179,51,48,188,187,10,238,157,125,123,247,155,225,29,3,203,103,6,212,61,74,240,181,189,114,114,226,12,123,247,246,49,104,106,202,50,124,251,246,125,196,
143,222,146,2,0,2,176,114,245,42,17,196,64,120,50,73,60,214,221,205,33,200,117,250,26,130,149,173,141,118,190,193,21,247,22,182,231,139,248,8,87,249,28,214,119,219,184,90,136,39,194,178,183,201,248,101,
179,224,54,130,11,22,97,32,132,252,205,144,249,230,99,38,241,133,241,163,22,170,214,214,95,187,15,114,61,219,138,174,226,28,192,118,155,74,96,99,154,67,248,27,73,26,117,192,210,7,57,52,107,60,201,213,
29,249,247,87,186,221,63,211,245,197,41,125,122,166,66,11,205,96,32,135,167,71,226,124,78,217,242,158,204,195,10,6,211,66,239,246,119,208,43,244,195,60,99,139,234,242,6,110,18,161,123,243,70,190,203,105,
255,210,180,213,65,215,35,131,241,24,234,181,214,178,88,156,200,102,179,134,27,58,27,94,22,53,120,54,153,92,50,18,134,187,96,214,170,40,50,118,46,83,198,88,213,117,29,130,188,132,173,172,61,146,200,92,
91,107,4,238,57,164,165,228,95,178,35,18,44,4,200,199,180,81,26,163,85,89,230,218,185,99,28,139,99,25,140,74,227,20,214,183,18,153,106,31,63,88,97,14,144,146,184,165,105,27,249,22,64,44,12,136,131,122,
192,248,232,227,103,15,159,242,51,127,21,121,253,157,155,149,157,9,178,157,4,212,86,97,98,32,121,161,60,35,176,136,97,254,254,157,129,77,70,133,129,211,59,138,225,223,220,42,134,239,160,29,40,172,140,
12,2,108,144,54,14,51,11,43,3,187,172,36,3,211,190,5,12,12,170,189,12,172,197,64,188,8,216,141,23,230,135,110,97,193,2,128,61,153,191,159,190,48,48,124,255,6,158,46,96,176,112,98,96,120,212,5,172,62,153,
25,126,60,252,194,240,244,11,199,235,43,239,62,191,128,249,13,88,162,48,197,197,185,114,185,185,153,50,105,107,43,114,8,9,9,1,195,23,114,180,23,35,120,30,130,233,63,43,235,119,80,59,6,182,185,31,47,0,
245,164,128,181,29,19,63,63,55,208,44,62,150,23,47,222,242,156,63,127,71,233,235,215,31,18,63,127,254,18,252,249,243,183,0,208,2,112,243,159,153,153,233,15,59,59,235,91,14,14,246,247,124,124,220,207,140,
141,85,239,139,136,240,255,4,170,253,249,237,219,143,159,160,200,38,167,128,3,37,2,144,59,128,102,51,11,8,112,115,112,114,114,49,61,123,246,88,232,226,197,59,234,223,190,253,20,253,241,227,151,40,80,30,
214,117,1,29,89,246,137,147,147,253,13,55,55,251,43,11,11,237,155,192,140,243,21,216,59,252,243,253,251,207,31,255,192,41,159,56,71,0,4,16,114,130,1,159,130,240,224,231,239,215,247,190,242,61,82,191,245,
94,147,213,68,144,129,225,205,101,96,163,215,154,129,129,147,7,50,183,131,43,18,113,213,121,95,254,48,176,199,37,51,236,60,113,146,225,192,166,195,12,44,156,28,64,11,191,65,124,1,244,52,51,180,41,164,
199,203,197,16,180,162,139,129,181,110,41,195,57,183,92,134,117,179,39,49,176,114,112,161,164,127,240,142,123,160,223,100,121,89,25,162,164,248,24,184,223,1,27,228,118,1,12,140,124,192,68,253,234,12,176,
11,207,195,240,237,238,99,134,91,63,152,30,188,248,244,245,19,200,95,192,92,247,215,203,203,156,239,217,179,183,145,19,38,172,149,102,103,103,249,13,140,76,70,164,128,255,15,12,52,246,152,24,151,35,42,
42,50,43,112,37,24,72,9,244,31,116,30,29,179,181,181,54,251,207,159,63,133,118,236,56,173,177,111,223,57,247,107,215,30,152,0,19,140,44,48,199,10,1,149,242,162,117,38,96,59,49,223,3,19,205,59,51,51,141,
251,250,250,74,7,34,34,156,118,169,168,72,61,83,82,146,122,255,242,229,251,63,164,148,57,80,149,76,202,202,82,108,194,194,252,194,91,182,28,215,171,173,157,239,125,229,202,3,147,203,151,239,129,230,208,
132,25,32,107,129,144,251,154,160,56,254,4,244,239,123,43,43,173,39,42,42,210,71,226,227,61,182,233,233,41,221,253,253,251,207,59,160,245,191,129,238,255,15,74,59,248,18,48,64,0,193,18,12,252,132,167,
47,255,24,62,159,252,203,115,199,249,213,87,77,96,140,1,219,5,55,24,24,149,128,246,170,152,49,48,220,216,199,192,192,205,67,92,41,6,178,21,88,186,48,73,43,51,48,57,6,50,236,204,201,100,152,114,21,20,135,
159,176,42,143,209,18,98,8,22,5,182,153,166,150,49,156,85,242,98,104,187,8,106,130,124,198,170,214,66,146,155,33,72,82,152,129,135,95,136,225,191,127,10,3,195,227,141,96,231,255,251,246,143,225,253,195,
191,127,119,188,248,118,9,216,221,6,181,172,127,113,115,115,252,5,118,157,57,167,77,219,232,245,248,241,43,19,92,206,85,86,150,6,150,68,18,171,176,141,4,131,138,114,96,143,138,81,94,94,130,19,88,245,240,
239,218,117,198,177,167,103,101,200,237,219,79,220,128,129,204,77,112,82,13,50,1,42,246,227,199,79,177,67,135,46,106,0,177,231,188,121,219,243,156,156,12,87,149,150,70,172,85,87,151,185,46,37,37,242,25,
88,50,252,198,55,14,4,58,4,17,88,162,129,26,236,108,170,170,210,252,23,46,220,53,8,9,105,204,184,114,229,190,59,48,210,9,185,3,148,55,5,129,9,67,240,232,209,171,74,64,108,183,114,229,129,116,59,59,189,
77,85,85,49,11,61,61,205,175,8,11,243,125,126,247,238,211,175,127,120,154,29,0,1,196,194,128,122,28,216,47,96,15,230,231,209,87,111,111,62,187,205,224,36,252,237,55,55,195,127,208,2,240,135,12,76,206,
17,12,255,46,2,19,12,215,127,226,166,100,64,99,32,192,210,229,95,114,22,100,228,247,206,5,252,161,10,234,130,243,240,48,48,190,188,201,240,247,238,107,252,165,22,208,236,63,159,191,50,252,11,44,100,96,
18,3,58,251,208,6,96,190,22,98,248,117,245,3,195,229,167,127,31,28,255,252,235,46,212,95,191,128,57,234,47,40,194,89,88,152,240,142,200,1,171,8,166,23,47,222,193,166,18,224,39,64,128,78,198,116,118,54,
102,1,150,64,60,192,8,82,203,204,236,207,62,120,240,98,24,52,7,147,13,128,246,73,111,222,124,188,16,152,248,130,83,82,188,167,53,55,39,46,251,240,225,203,251,53,107,14,126,193,217,33,252,251,143,177,169,
41,129,83,92,92,80,180,172,108,86,210,210,165,123,178,128,98,34,228,186,1,88,109,9,3,237,79,220,189,251,172,87,118,118,192,132,210,210,176,149,95,190,252,120,109,98,162,254,245,232,209,43,88,83,13,64,
0,49,33,85,71,160,5,4,160,92,249,247,228,203,183,247,118,62,101,187,244,241,216,43,96,2,1,134,203,221,181,12,140,118,142,12,140,114,234,64,85,223,25,8,86,186,160,30,214,7,96,105,97,228,192,240,223,39,
137,225,231,190,149,12,127,191,227,159,124,4,21,115,191,254,252,99,248,201,193,206,240,251,35,254,4,243,15,216,45,255,107,21,206,240,207,59,140,225,223,217,22,96,218,4,205,27,49,50,188,61,245,158,97,221,
139,95,167,63,252,248,245,158,1,114,66,213,15,96,23,29,124,10,231,255,255,12,120,135,2,129,213,18,211,219,183,159,80,86,230,129,246,78,183,183,167,176,2,171,15,209,222,222,213,1,126,126,213,75,129,137,
37,142,210,196,130,12,128,237,29,185,169,83,55,180,5,7,55,244,190,122,245,65,53,40,200,142,223,211,211,12,35,128,99,99,221,152,252,253,173,248,126,252,248,173,25,30,222,212,182,104,209,174,90,74,18,11,
90,117,43,62,101,202,250,182,240,240,230,150,47,95,190,171,206,157,91,202,99,110,174,137,53,146,1,2,136,9,169,74,250,5,77,48,63,128,38,252,88,251,238,207,241,123,71,62,253,252,5,58,76,227,243,45,200,146,
2,183,84,32,251,31,254,2,6,212,151,254,10,76,28,98,138,12,140,37,253,12,255,128,9,229,207,165,195,12,255,64,59,23,241,37,130,255,144,142,24,232,72,152,63,172,248,247,42,255,151,80,5,38,150,40,134,255,
215,187,24,254,191,189,201,192,200,207,199,240,237,202,91,134,51,55,254,62,217,243,254,215,101,168,127,64,13,216,159,192,18,230,15,49,221,102,96,196,49,127,253,250,29,190,12,2,52,17,217,221,157,193,22,
22,230,32,90,87,55,47,164,182,118,222,212,55,111,62,170,208,106,120,99,255,254,243,161,65,65,117,83,174,94,125,96,208,209,145,202,239,232,104,0,119,180,156,156,24,67,122,186,15,231,147,39,111,212,146,
146,58,187,143,29,187,26,197,64,253,29,157,140,39,78,92,139,138,137,105,237,2,150,116,74,157,157,233,60,160,1,77,116,0,16,64,176,18,230,15,82,41,3,62,63,238,196,139,183,15,246,190,103,63,247,229,40,168,
148,1,182,91,46,207,96,96,114,7,150,50,198,46,192,102,200,103,236,165,12,51,19,100,119,192,95,30,6,166,220,54,6,70,41,96,151,239,202,49,134,127,111,159,50,252,99,97,37,88,194,128,11,138,255,12,132,87,
228,115,179,51,252,187,61,153,225,255,211,195,12,140,188,194,12,127,191,253,97,120,190,231,37,195,214,55,127,78,190,250,254,243,45,3,226,56,179,159,192,46,228,111,96,79,5,100,36,35,254,46,242,63,70,96,
162,129,171,41,44,12,97,13,15,119,20,236,233,89,21,0,108,44,55,49,64,246,107,209,20,0,27,230,86,101,101,51,171,128,93,98,185,204,76,63,46,72,162,96,100,0,86,23,236,192,106,72,172,180,116,102,222,173,91,
79,28,105,233,134,219,183,159,186,148,151,207,202,149,146,18,18,139,143,119,103,67,151,7,8,32,88,9,243,27,41,144,65,75,226,128,13,132,191,95,150,190,248,122,242,238,177,143,95,127,222,3,10,255,1,150,242,
143,231,48,48,149,79,102,96,20,146,7,170,248,12,169,122,96,9,7,52,171,253,13,168,245,47,59,80,205,108,6,70,96,94,252,255,244,28,195,255,27,151,24,254,253,254,78,176,119,5,63,181,249,63,225,54,245,255,
207,15,129,248,9,195,127,118,96,167,4,216,245,127,187,227,17,195,233,231,92,119,86,191,253,121,22,154,232,191,66,241,119,96,149,244,27,216,83,250,71,168,144,1,245,156,128,13,78,112,117,4,236,246,50,6,
4,88,115,3,123,65,86,253,253,107,90,128,98,2,36,134,251,95,164,118,33,73,3,29,55,111,62,118,171,171,155,159,47,35,35,198,165,169,41,199,0,116,7,11,176,65,42,92,85,53,39,227,216,177,43,49,68,118,185,63,
243,241,113,221,7,246,132,206,233,232,40,156,86,83,147,61,11,236,210,223,7,138,127,34,70,63,176,218,77,156,52,105,125,88,96,160,173,128,139,139,49,74,196,1,4,16,11,3,226,244,109,88,162,129,5,54,247,133,
15,223,238,207,121,43,180,167,98,215,75,127,197,116,96,10,120,178,155,129,65,200,128,129,177,114,33,195,255,154,64,96,71,17,152,136,56,160,171,240,64,163,195,194,82,12,76,133,115,24,24,117,128,61,203,
131,233,12,12,198,157,12,255,30,109,99,160,194,130,125,180,100,206,2,94,199,203,196,251,157,225,243,185,103,12,119,47,252,251,60,237,19,235,174,15,223,64,179,164,224,4,255,5,169,132,1,214,134,44,127,9,
57,0,188,34,244,31,164,187,29,17,225,200,6,108,252,169,2,171,161,74,96,34,18,36,198,73,28,28,108,239,36,37,133,47,1,139,241,187,192,200,122,7,140,156,31,80,59,57,128,109,35,145,107,215,30,106,189,126,
253,65,7,104,30,222,146,10,216,19,59,174,171,171,116,6,88,53,113,128,70,223,92,92,140,216,129,93,101,227,213,171,15,198,17,114,3,176,26,125,14,236,38,239,5,118,183,175,1,253,253,18,88,178,62,7,250,11,
212,240,103,249,245,235,55,232,160,2,41,96,207,206,224,194,133,59,174,191,126,253,193,231,47,166,25,51,54,229,0,123,80,103,188,189,45,142,31,56,112,225,43,208,221,224,132,15,16,64,44,72,153,27,86,37,193,
2,156,11,24,130,156,179,30,190,63,36,197,194,43,156,185,230,129,141,168,159,12,3,227,149,73,12,140,166,77,12,76,253,123,25,254,45,239,96,96,184,127,9,60,127,195,168,227,192,192,20,2,108,227,176,1,219,
59,7,74,32,37,15,171,8,3,227,199,15,144,118,13,85,107,92,96,11,150,139,153,225,235,189,79,12,143,182,188,253,219,251,154,123,235,225,7,207,111,64,19,201,23,104,130,7,31,209,10,26,253,100,103,103,35,234,
200,15,208,212,19,63,63,55,147,141,141,46,47,176,7,226,3,236,134,155,18,28,200,98,97,254,102,100,164,186,222,192,64,121,255,207,159,127,94,191,126,253,241,221,139,23,239,129,221,227,63,255,65,109,39,144,
253,252,252,60,28,126,126,150,66,192,110,177,204,225,195,151,131,129,145,102,141,54,86,3,90,98,113,3,24,65,43,140,140,212,246,191,123,247,249,30,176,219,254,17,88,42,176,25,27,171,75,166,164,116,103,0,
149,72,224,11,16,96,73,178,19,232,238,213,31,62,124,125,112,243,230,147,79,111,222,124,248,3,108,68,255,1,118,213,255,179,179,179,50,1,187,204,47,164,164,132,175,1,75,157,51,134,134,170,123,129,189,163,
248,251,247,159,91,225,25,156,148,154,54,109,67,100,79,79,230,13,13,13,185,159,192,174,59,120,124,10,32,128,144,19,204,31,164,54,204,23,6,216,217,118,255,255,178,78,125,241,115,191,210,49,86,9,79,230,
167,42,34,33,114,12,12,39,170,129,85,78,56,3,115,126,5,80,7,27,195,127,208,80,254,47,96,124,61,233,101,96,120,113,26,178,124,147,133,3,58,194,68,214,186,107,252,69,46,43,51,195,175,23,192,196,178,254,
229,223,233,175,185,247,172,189,247,252,12,208,252,159,208,129,155,79,72,37,204,47,96,149,196,12,236,82,19,156,252,2,201,130,46,149,0,70,62,7,48,146,229,129,57,58,156,144,59,184,184,56,158,250,250,90,
206,228,229,229,58,117,228,200,149,119,192,4,246,253,243,103,208,86,209,255,200,7,51,130,70,123,89,128,165,206,91,77,77,249,151,94,94,230,247,110,221,146,186,176,103,207,185,248,223,191,255,8,0,205,120,
98,105,169,181,93,67,67,118,23,176,36,122,56,111,222,246,55,247,238,61,255,14,108,83,253,171,173,141,229,62,117,234,186,62,176,116,114,193,231,14,99,99,181,117,192,196,54,23,216,24,126,7,44,61,62,2,219,
98,63,25,144,142,109,3,118,159,25,159,62,125,195,4,196,172,151,46,221,251,108,97,161,245,209,199,199,242,221,182,109,199,255,220,189,251,220,14,151,185,199,143,95,243,62,123,246,246,202,132,4,143,119,
37,37,211,193,9,6,32,128,88,144,219,156,72,213,210,55,104,224,131,119,14,190,254,242,253,85,195,115,150,245,252,71,24,163,237,120,158,73,241,187,73,51,48,222,91,14,44,93,54,130,215,208,50,254,1,186,239,
251,59,72,87,7,116,2,38,168,189,242,247,43,164,221,3,58,151,8,52,105,204,200,76,76,229,11,77,91,248,231,32,254,127,251,197,240,114,223,199,255,171,95,242,28,154,115,239,209,97,96,108,127,135,38,148,143,
80,119,127,131,38,254,223,192,4,3,140,48,230,127,132,183,107,255,3,117,163,153,92,93,77,184,54,109,58,234,9,236,41,168,19,40,254,95,132,134,58,244,3,75,145,83,27,54,28,126,247,230,205,167,175,12,136,51,
131,255,34,53,205,152,128,221,95,230,247,239,191,48,3,35,244,43,48,81,125,118,116,52,90,229,231,103,245,30,200,86,54,51,211,216,11,76,40,119,119,239,62,251,17,152,80,62,3,115,54,200,237,191,129,9,236,
191,137,137,58,251,228,201,235,125,128,237,43,54,92,238,0,150,26,231,29,29,13,23,238,218,117,250,41,48,49,128,252,255,19,152,224,65,13,125,80,239,240,31,82,27,13,228,14,22,96,98,98,61,116,232,210,143,
239,223,127,254,246,247,183,158,184,100,201,94,145,87,175,222,107,225,232,57,138,29,63,126,213,43,40,200,230,12,208,158,31,192,70,249,95,128,0,98,65,107,168,193,186,215,95,25,144,182,159,128,216,247,222,
127,126,82,207,196,179,182,113,199,199,16,139,79,191,37,69,131,129,221,102,208,84,193,151,183,144,6,47,11,15,106,19,22,52,177,245,230,16,3,163,71,40,3,195,66,96,169,243,253,51,161,153,60,160,11,191,129,
71,135,25,254,225,223,50,251,237,227,255,255,235,190,243,157,92,240,232,209,209,191,191,127,127,129,38,150,15,208,4,3,47,93,32,254,97,100,33,102,242,27,52,184,7,44,41,88,129,129,204,117,250,244,77,15,
2,202,127,3,75,138,121,156,156,236,199,86,175,222,255,6,152,88,190,48,32,78,38,255,139,101,214,13,190,127,253,241,227,215,63,215,175,63,252,13,88,170,108,84,84,148,100,219,191,255,194,183,59,119,158,126,
7,70,206,15,104,34,7,31,143,15,172,94,88,129,137,76,242,234,213,251,166,120,242,215,79,79,79,179,37,188,188,156,239,128,213,204,111,103,103,35,70,9,9,33,38,65,65,94,102,160,24,176,93,197,14,95,127,242,
237,219,15,166,79,159,190,3,19,238,103,230,23,47,222,49,126,252,248,245,151,136,136,192,43,15,15,211,133,139,22,237,106,129,198,51,6,216,189,251,140,129,143,143,133,44,176,84,250,178,110,221,225,127,0,
1,196,130,165,117,255,139,1,117,131,62,108,161,56,243,133,183,95,238,22,255,231,91,94,127,228,71,160,235,183,219,138,34,129,10,12,204,66,60,144,147,169,64,9,4,214,21,1,31,254,3,140,244,123,91,25,24,77,
108,24,152,51,39,51,48,148,23,227,28,234,135,116,201,217,129,213,156,53,3,163,154,41,3,227,205,103,12,12,199,38,226,84,250,244,55,195,251,99,239,63,95,255,247,231,207,103,104,34,121,15,77,48,159,160,137,
253,7,172,135,2,46,180,192,19,140,132,18,204,127,6,33,33,94,96,145,125,95,27,216,117,149,195,167,22,212,14,80,80,144,216,183,106,213,129,183,192,196,242,9,106,31,52,129,98,189,166,135,9,169,215,244,251,
243,231,111,191,128,37,202,87,80,163,246,31,100,90,251,47,122,130,3,54,124,57,158,63,127,171,241,252,249,59,113,92,238,16,16,224,189,5,108,167,112,1,221,96,199,205,205,249,11,148,8,128,213,207,223,39,
79,94,255,3,173,41,2,54,124,255,35,15,27,128,252,8,44,193,152,128,13,96,102,96,2,101,3,102,12,214,175,95,127,112,1,19,215,195,207,159,191,99,29,99,2,86,99,90,192,182,157,44,48,108,110,131,220,7,16,64,
44,88,122,182,176,82,230,27,82,3,4,182,54,150,225,206,187,79,143,170,255,114,175,126,203,204,225,238,249,242,150,142,180,135,36,51,151,46,176,90,2,237,11,250,254,23,177,209,158,17,186,88,255,76,35,3,131,
86,30,3,163,62,176,157,119,238,1,238,102,185,186,33,3,99,72,8,80,27,176,196,186,126,19,127,246,254,243,247,55,176,246,248,14,44,118,65,137,229,29,20,127,64,43,93,254,146,210,120,2,86,73,160,198,49,243,
151,47,223,228,129,69,183,40,62,165,192,134,233,150,135,15,95,190,1,226,143,176,222,24,3,254,147,200,97,43,255,224,99,94,160,29,26,72,227,77,200,55,180,128,79,52,6,70,16,168,20,0,37,92,156,163,185,192,
106,83,123,243,230,227,205,148,246,32,8,100,40,145,39,79,94,137,75,75,139,176,3,171,246,31,0,1,196,130,99,56,228,55,3,234,197,89,76,200,51,159,143,62,126,125,94,246,245,251,250,67,111,121,111,199,188,
122,110,101,97,252,73,74,212,74,152,129,69,158,23,162,236,215,95,72,137,3,58,45,10,116,210,193,213,105,12,12,111,223,224,119,246,247,59,192,146,101,34,208,178,31,12,140,95,191,19,154,121,248,207,193,206,
246,25,152,131,222,65,75,23,228,182,11,114,78,39,126,224,228,47,104,150,150,137,25,152,227,196,24,240,108,111,4,182,93,222,105,106,202,94,1,150,46,159,144,18,203,111,34,22,127,160,223,58,199,136,69,238,
31,100,24,5,88,47,114,113,176,2,75,24,65,2,99,71,212,56,180,158,17,121,246,30,27,56,118,236,154,60,176,123,207,201,207,207,245,25,32,128,88,240,120,236,23,218,140,43,3,210,82,136,63,191,254,252,251,179,
238,249,199,83,59,95,49,223,246,127,205,160,19,124,233,135,161,153,54,135,148,168,1,31,19,139,44,55,3,51,31,59,176,124,1,54,30,64,91,79,64,51,254,255,158,19,152,124,252,201,192,196,199,203,240,239,39,
104,46,233,43,161,233,253,63,28,28,108,31,129,141,59,88,233,242,17,169,161,11,75,44,68,206,146,34,192,199,143,95,184,46,95,190,47,133,79,13,176,157,243,136,149,149,245,203,151,47,160,147,10,224,165,10,
177,137,243,63,82,137,130,75,14,84,229,49,177,178,50,11,30,60,120,81,158,97,16,128,55,111,62,138,242,240,112,113,0,195,156,9,32,128,112,37,24,88,32,252,198,145,11,224,235,103,190,2,203,239,101,207,63,
31,219,254,150,229,170,253,147,223,234,62,103,190,106,25,203,176,203,73,200,48,115,114,200,114,50,252,19,226,96,248,47,202,202,192,128,127,238,143,225,215,251,95,12,111,15,61,103,248,250,234,223,159,71,
215,254,252,194,55,20,15,90,59,4,204,233,159,129,189,26,88,35,247,27,52,183,255,197,209,232,36,170,100,6,213,237,192,18,134,147,64,239,232,13,27,27,243,119,96,21,134,220,192,37,165,52,35,164,150,145,131,
131,149,9,88,221,178,2,219,14,204,131,33,193,0,195,4,52,210,205,6,106,115,1,4,16,11,1,143,253,65,227,255,71,155,221,6,69,18,168,30,226,126,255,235,207,175,13,47,255,124,216,246,154,233,138,206,211,191,
146,14,119,57,149,52,184,190,202,9,179,126,224,19,145,251,205,247,225,254,103,54,124,169,230,233,171,191,95,150,156,103,187,121,235,195,247,103,215,191,253,3,229,114,99,124,53,8,176,161,246,25,180,138,
12,154,96,126,160,181,1,200,152,177,5,23,205,76,4,83,54,3,104,30,149,229,15,176,58,36,39,177,16,5,126,255,254,11,74,188,12,12,52,60,201,156,164,58,11,216,198,1,246,34,65,203,78,25,1,2,136,80,130,249,143,
214,30,248,143,212,40,254,137,52,255,196,3,197,156,191,254,253,251,117,238,227,183,175,64,252,128,137,145,145,93,74,144,87,192,140,147,67,250,198,151,47,160,197,75,56,123,31,247,127,253,127,245,237,237,
167,115,192,134,220,103,1,62,110,22,2,195,53,127,248,249,121,190,1,115,251,55,180,54,4,217,27,160,129,213,219,71,80,219,129,157,157,21,239,141,19,192,94,8,63,55,55,23,51,180,203,138,115,4,57,46,206,21,
124,227,43,176,43,203,240,246,237,39,240,13,174,160,109,184,192,98,157,193,220,92,147,65,67,67,14,60,246,3,90,48,181,98,197,126,208,196,35,162,57,247,253,23,8,255,7,117,162,6,67,130,249,253,27,86,210,
49,50,0,4,16,11,145,69,40,172,219,247,15,109,118,27,121,194,242,11,100,148,142,1,118,84,24,251,191,255,255,57,158,126,248,252,243,206,183,95,127,62,252,253,167,78,160,33,251,135,131,133,249,11,211,191,
255,239,249,121,57,241,46,136,249,254,253,55,31,104,209,53,176,39,241,139,80,98,97,132,2,80,175,4,159,153,204,204,140,223,128,213,192,15,21,21,169,87,87,174,224,222,214,251,225,195,87,101,14,14,22,54,
89,89,81,150,91,183,30,99,77,44,229,229,17,12,29,29,105,224,0,126,246,236,53,3,176,171,206,48,101,202,122,240,145,103,155,55,31,99,200,200,8,96,240,240,240,0,7,229,159,63,95,192,247,22,212,215,47,132,
59,249,197,139,183,255,191,125,251,241,89,79,79,241,197,149,43,247,112,198,139,163,163,193,34,25,25,209,251,160,1,57,38,42,95,218,4,218,236,1,44,193,65,85,255,31,9,9,161,251,192,4,255,229,211,167,175,
255,0,2,136,216,4,195,128,212,174,65,94,210,9,171,150,190,66,19,10,236,20,110,216,145,167,156,44,204,76,156,60,156,108,172,31,153,153,126,19,40,246,126,1,115,247,59,96,99,239,53,48,247,138,227,159,243,
249,205,205,197,197,14,26,160,34,120,213,13,176,171,204,4,154,75,1,230,88,124,99,43,255,152,153,89,126,126,253,250,19,88,213,113,189,132,250,149,25,123,125,254,93,4,216,107,208,243,244,52,187,125,242,
228,117,148,19,169,164,164,68,24,146,146,60,24,170,171,99,128,165,202,71,112,130,121,255,254,11,176,148,129,156,137,3,76,100,192,18,234,55,195,55,208,172,62,184,157,254,23,200,254,204,32,46,46,196,160,
165,37,207,112,237,218,67,216,8,235,255,87,175,62,254,1,102,136,215,248,123,55,12,191,4,5,121,142,29,59,118,253,51,40,130,169,185,17,15,100,30,104,196,24,24,126,63,206,158,189,245,21,232,182,207,192,18,
242,31,64,0,17,123,58,14,250,245,50,255,24,48,23,94,193,18,14,7,3,226,108,57,78,160,39,184,129,197,48,43,144,254,129,191,132,97,250,9,236,74,190,5,210,175,120,120,56,94,1,213,127,251,247,239,63,174,134,
47,31,176,52,224,16,21,229,255,143,150,232,192,155,224,144,142,230,96,212,210,146,99,126,246,236,13,223,247,239,63,241,149,48,63,64,118,191,121,243,225,47,27,27,219,83,96,64,189,5,86,21,98,184,194,242,
228,201,107,94,190,190,22,219,129,57,156,117,243,230,227,240,30,143,159,159,37,248,162,240,247,239,63,131,170,56,80,169,133,220,5,102,128,236,171,131,76,67,32,142,172,249,15,84,199,194,16,24,104,3,75,
48,224,48,6,86,205,255,128,57,251,5,116,216,0,107,247,26,88,18,90,25,25,169,174,123,252,248,229,167,151,47,223,147,115,150,218,127,44,61,97,6,180,30,241,111,164,17,232,127,0,1,196,68,134,5,200,35,147,
63,144,170,35,216,136,43,168,50,6,13,186,128,54,42,191,96,102,102,126,201,199,199,245,26,152,16,190,19,74,48,124,124,220,239,128,17,254,6,88,68,191,1,54,44,241,174,221,216,189,251,140,158,141,141,46,143,
148,148,48,220,163,81,81,206,12,89,89,254,112,53,160,91,234,237,236,244,153,175,94,125,104,0,44,9,120,241,148,110,127,88,88,152,190,220,188,249,248,167,128,0,247,117,69,69,201,71,248,236,62,127,254,142,
237,222,189,231,189,242,242,130,248,92,93,141,153,65,119,15,76,158,156,11,190,157,254,213,171,247,88,55,198,65,182,133,252,1,150,38,130,12,34,34,2,172,175,95,63,87,184,126,253,166,42,43,43,27,55,48,225,
51,163,95,214,117,227,198,163,191,192,112,187,1,77,52,184,186,187,218,192,132,165,21,30,238,244,7,216,99,252,130,52,159,70,8,131,87,227,43,43,75,125,141,140,116,254,174,175,175,252,13,24,254,232,106,62,
67,11,0,228,129,201,127,0,1,196,68,102,170,68,222,203,132,60,97,249,21,105,94,7,132,223,3,75,130,119,192,6,234,7,32,253,11,127,130,97,252,3,140,172,79,192,156,248,1,24,1,207,129,109,148,23,248,212,159,
62,125,211,13,152,32,196,99,99,221,56,69,68,32,37,77,69,69,4,3,176,170,96,0,205,163,128,34,49,61,221,155,93,86,86,76,4,88,34,184,224,27,140,3,173,150,151,147,19,187,123,233,210,221,31,192,72,248,12,12,
200,67,4,194,128,103,210,164,117,165,55,110,60,182,236,234,74,231,207,204,244,3,39,26,80,59,2,219,36,39,104,244,31,52,87,5,204,60,140,189,189,89,28,234,234,50,210,161,161,13,53,86,86,57,75,22,45,218,225,
105,103,167,43,230,230,102,10,114,43,44,103,255,7,150,30,191,128,37,229,107,96,227,248,12,62,135,108,219,118,34,94,71,71,94,42,55,55,232,31,48,195,125,131,198,3,94,12,244,239,215,130,130,224,63,211,167,
23,114,3,75,40,243,222,222,76,161,41,83,242,152,130,131,237,126,138,137,9,194,226,17,166,254,7,82,59,241,63,64,0,81,218,207,255,143,111,140,6,132,129,17,255,207,194,66,139,235,254,253,231,214,175,95,127,
84,195,101,16,176,190,190,107,106,170,190,247,212,169,27,111,180,181,21,191,1,91,230,134,143,31,191,214,192,221,240,5,111,214,250,9,244,248,89,91,91,221,223,47,94,188,255,107,108,172,6,26,182,103,184,
124,249,62,99,90,154,15,71,104,168,163,240,162,69,59,163,118,236,56,29,135,47,193,200,201,137,95,3,186,113,205,166,77,199,62,1,219,24,255,93,93,77,190,157,57,115,211,1,232,6,94,220,115,165,255,249,247,
238,61,103,1,76,172,31,50,50,252,158,3,19,217,239,87,175,62,48,72,75,139,252,7,149,108,160,196,3,42,85,64,37,199,187,119,159,89,162,163,157,89,129,145,207,247,249,243,55,157,196,196,174,186,163,71,175,
196,0,221,47,189,125,251,41,175,179,103,111,243,2,115,250,157,212,84,239,111,64,45,255,79,156,184,254,31,52,231,195,195,195,197,174,161,33,251,15,232,22,47,96,66,196,26,87,95,190,252,16,191,118,237,145,
28,208,191,23,98,99,93,223,0,219,96,127,94,190,124,247,11,88,242,254,1,38,12,48,6,186,17,216,120,229,4,225,191,160,197,221,211,167,23,176,3,195,73,14,152,232,51,38,78,92,219,126,236,216,85,99,96,41,243,
35,38,198,245,123,124,188,219,23,160,59,129,102,188,255,13,218,113,1,204,200,255,64,109,42,88,85,9,16,64,52,233,182,35,77,37,176,1,3,80,160,185,57,201,72,91,91,97,3,150,182,16,28,3,3,124,87,123,123,170,
37,208,131,82,192,46,169,50,208,241,173,12,152,251,162,209,241,15,95,95,203,217,219,182,181,155,127,254,188,85,252,245,235,245,98,175,94,173,19,251,250,117,135,232,233,211,211,245,195,195,29,219,161,185,
4,159,25,192,4,98,60,161,168,40,84,27,88,10,138,2,75,10,137,236,236,0,3,111,111,243,73,132,244,65,241,87,96,36,172,1,230,210,200,59,119,150,40,125,253,186,77,236,203,151,173,98,223,190,109,23,249,250,
117,187,40,144,22,187,119,111,153,252,130,5,21,238,137,137,158,221,192,158,199,61,108,230,104,104,200,239,93,186,180,58,224,229,203,181,226,192,94,22,39,48,114,185,197,196,4,196,106,106,98,205,117,117,
149,54,18,114,7,176,116,58,14,44,49,194,128,254,151,121,255,126,19,255,143,31,59,57,128,246,179,0,195,133,5,68,63,124,184,130,227,195,135,205,252,175,94,173,151,92,176,160,220,91,65,65,226,0,186,25,64,
251,78,2,237,46,59,121,114,170,206,223,191,123,4,110,221,90,4,90,4,143,18,185,0,1,68,139,35,33,145,83,226,95,78,78,118,80,215,140,152,225,243,255,192,122,248,15,48,210,126,3,219,8,76,65,65,118,71,128,
189,160,135,192,6,164,2,30,61,236,192,70,103,10,48,135,56,232,233,41,93,4,86,125,119,65,250,63,126,252,162,124,235,214,83,253,71,143,94,170,18,242,35,176,52,120,109,106,170,121,252,232,209,203,159,129,
85,0,104,247,223,159,35,71,46,189,11,8,176,217,113,225,194,61,203,167,79,95,155,16,112,55,23,176,183,20,12,196,46,51,102,108,126,10,236,9,93,1,54,218,159,128,38,234,64,126,2,246,144,148,158,63,127,167,
114,243,230,35,57,160,217,56,123,127,55,110,60,116,138,137,105,51,2,230,240,105,21,21,209,203,129,37,211,147,201,147,215,255,60,123,246,230,107,39,39,195,77,183,110,61,49,255,249,243,23,78,253,143,31,
191,178,200,201,153,56,103,238,220,173,7,76,77,53,118,133,134,58,92,1,150,50,159,64,235,148,129,97,2,108,91,125,224,89,185,114,191,46,176,180,114,189,112,225,174,53,176,167,41,140,110,6,80,141,89,103,
231,10,163,133,11,119,197,3,237,92,223,218,154,188,88,84,84,224,46,242,0,46,64,0,209,26,49,171,168,72,243,244,247,103,235,1,75,152,117,4,74,152,157,147,38,229,154,74,74,10,131,102,103,69,128,85,141,174,
143,143,101,47,145,185,156,108,12,172,206,22,148,150,134,27,10,11,243,138,192,6,32,129,165,128,72,112,176,173,78,122,186,111,38,144,253,138,214,110,64,47,53,59,58,82,179,102,207,46,230,3,207,68,112,115,
136,231,228,4,26,132,134,218,183,163,205,147,17,194,255,128,153,245,62,16,223,5,246,82,31,129,14,35,32,197,29,192,166,196,141,29,59,186,236,59,59,211,80,246,252,0,4,208,160,186,162,27,84,95,2,123,43,224,
217,242,253,251,47,124,52,49,81,219,15,204,37,23,104,101,159,128,0,207,29,43,43,157,45,39,78,92,123,255,246,237,103,216,44,247,31,96,238,251,181,115,231,153,247,192,54,198,73,96,181,54,9,88,242,189,167,
87,24,88,90,106,175,215,212,148,223,191,113,227,49,112,164,131,54,237,239,219,119,238,19,80,108,39,80,110,13,41,77,3,96,59,79,1,136,149,128,254,144,37,180,145,15,173,71,247,33,63,63,168,251,227,199,207,
183,230,205,219,134,50,137,11,16,64,131,44,193,48,1,123,18,224,4,243,23,216,173,252,122,229,202,131,59,129,129,54,51,64,167,14,80,189,232,99,102,250,232,231,103,53,11,216,13,190,13,44,166,191,194,18,11,
108,108,9,216,13,255,182,99,199,169,183,192,42,107,59,176,120,239,101,101,101,166,117,162,249,7,108,180,175,4,182,25,38,207,154,181,229,205,214,173,39,96,61,147,223,192,176,248,4,116,203,115,7,7,131,249,
102,102,26,43,105,28,7,31,226,227,221,219,216,217,217,142,84,86,206,249,118,243,230,19,148,4,3,16,64,131,42,193,64,22,242,128,215,161,254,5,182,204,127,239,217,115,246,51,11,11,203,89,96,119,111,42,48,
130,63,81,203,30,208,188,81,80,144,45,176,228,96,221,15,236,25,189,253,254,29,116,8,48,124,230,25,54,245,241,19,216,83,248,186,101,203,241,247,64,245,123,128,37,77,15,176,122,120,66,163,72,250,14,236,
153,205,240,244,52,155,181,124,249,190,231,192,196,242,11,122,154,3,56,193,0,123,100,63,79,159,190,249,97,215,174,211,143,237,237,13,22,185,184,24,207,103,64,93,126,66,21,192,199,199,245,56,38,198,165,
147,155,155,125,15,176,253,244,233,222,189,231,127,209,7,248,0,2,136,214,211,231,76,66,66,124,172,192,92,33,120,238,220,109,135,215,175,63,104,226,235,86,219,216,232,238,61,118,236,202,219,247,239,191,
128,151,87,2,139,210,255,160,33,118,93,93,229,231,192,54,208,125,96,215,92,25,40,38,68,97,53,244,24,216,54,154,10,12,156,157,235,214,29,122,9,236,242,126,101,64,44,186,194,152,102,0,13,135,63,121,242,
250,15,176,49,251,28,232,190,171,64,251,89,129,141,67,57,42,117,24,254,1,123,145,23,129,137,119,178,140,140,200,206,109,219,78,190,184,112,225,206,39,6,212,197,228,240,161,139,231,207,223,254,249,246,
237,199,47,43,43,237,59,138,138,146,15,95,188,120,43,5,108,28,83,99,127,245,31,96,87,250,0,48,99,78,2,154,127,106,221,186,195,111,222,190,5,207,103,252,70,115,3,3,64,0,81,45,193,4,4,88,51,168,171,203,
129,118,238,145,157,96,172,173,117,246,30,61,10,79,48,224,241,253,119,239,190,252,125,240,224,197,95,96,119,251,53,48,194,46,1,187,189,127,64,235,92,255,253,35,120,204,6,106,87,134,139,253,185,153,153,
230,86,47,47,243,41,192,72,63,179,101,203,177,87,64,123,190,32,13,123,163,247,228,224,13,64,96,59,224,31,48,183,253,2,22,128,31,128,126,185,4,108,160,223,5,29,137,241,241,227,87,94,160,59,200,217,66,251,
93,78,78,236,146,189,189,254,42,160,159,231,61,124,248,242,218,246,237,39,95,62,122,244,234,51,116,100,245,55,142,6,46,195,203,151,239,126,223,185,243,244,151,152,152,224,51,11,11,205,179,130,130,60,31,
128,238,224,6,186,81,136,129,196,229,16,192,146,237,179,178,178,228,41,71,71,131,197,90,90,10,43,47,93,186,119,103,223,190,11,111,63,127,6,237,119,134,175,2,64,89,236,5,16,64,84,235,86,7,6,58,48,196,197,
249,50,120,120,228,48,0,27,140,212,234,154,255,1,237,55,2,182,51,62,109,221,122,252,175,174,174,210,79,61,61,229,199,90,90,242,123,129,185,94,19,152,56,13,128,221,86,5,232,32,30,11,108,231,34,104,62,9,
148,176,128,85,200,75,80,195,22,152,27,111,42,42,74,92,2,150,22,55,65,141,233,91,183,30,129,6,232,96,139,197,113,37,22,6,228,238,36,208,252,127,199,142,93,251,123,253,250,227,239,70,70,170,239,77,77,213,
79,219,219,235,41,62,125,250,70,21,216,229,213,2,70,26,200,29,160,19,168,216,127,253,250,203,140,212,144,255,15,26,90,0,246,84,94,241,242,114,62,150,145,17,189,5,236,57,222,4,118,187,239,2,75,136,103,
192,220,252,13,216,117,255,12,116,251,55,164,196,251,27,173,180,131,205,219,253,0,157,107,4,44,225,254,1,195,227,151,130,130,196,87,125,125,229,167,33,33,118,251,129,213,170,18,176,205,167,3,12,43,245,
175,95,191,203,0,221,203,243,243,39,98,1,22,51,51,227,127,160,27,126,129,118,104,242,242,114,63,80,82,146,184,6,76,248,55,126,255,254,123,23,24,150,175,14,29,186,252,5,152,161,97,107,139,144,195,5,101,
21,0,64,0,86,174,93,5,97,32,8,166,240,93,136,17,67,64,11,187,20,246,130,31,225,239,248,27,254,141,32,136,118,22,54,134,8,98,19,11,69,66,26,73,206,83,99,188,92,116,150,168,4,81,43,171,107,151,153,217,205,
110,118,238,254,38,24,206,147,27,1,195,97,95,233,118,123,202,96,48,251,135,104,94,132,69,81,28,155,166,45,86,171,237,169,217,212,61,8,96,137,12,29,161,114,228,0,98,17,253,134,46,165,204,211,223,85,218,
13,233,186,234,226,60,163,100,135,32,134,77,38,230,1,25,28,0,200,75,10,20,161,124,183,88,166,69,243,90,135,120,30,23,227,241,60,32,3,23,200,119,241,73,89,116,58,173,17,68,81,192,52,149,129,128,85,223,
231,85,114,236,147,41,11,83,159,168,215,107,59,33,34,65,211,23,200,62,90,150,205,17,203,229,241,68,218,147,156,235,27,73,183,207,9,148,196,34,101,44,215,107,39,220,108,220,51,170,13,3,38,182,97,52,102,
237,182,81,66,195,154,69,211,94,70,172,26,225,65,158,93,52,239,129,166,85,104,19,127,133,168,133,227,236,217,116,186,228,116,237,133,177,99,58,142,240,23,46,119,1,88,185,130,20,132,97,32,136,180,13,169,
6,91,74,63,231,19,250,209,222,122,20,138,120,243,172,32,181,138,155,180,56,19,68,66,91,15,130,47,200,50,179,187,217,100,50,249,243,197,157,243,98,117,85,237,62,9,227,81,91,205,148,209,57,59,243,223,21,
166,85,238,165,7,16,110,41,18,162,170,123,144,116,214,58,81,198,172,147,162,48,199,52,213,49,1,18,17,94,167,15,32,143,224,8,186,137,96,235,8,85,87,9,102,132,241,71,149,222,75,31,93,247,120,34,121,239,
109,123,186,33,134,139,194,4,205,132,41,203,109,156,101,38,122,63,195,161,31,104,192,54,59,32,110,135,138,167,13,196,34,177,37,80,250,237,194,204,178,132,213,244,207,97,63,156,163,144,72,62,77,102,125,
211,28,174,232,32,138,9,147,231,155,136,254,164,152,182,79,128,139,174,227,234,122,63,34,30,203,3,5,141,108,232,106,225,250,18,196,241,245,125,209,75,0,81,49,193,192,54,220,131,102,100,217,65,219,72,65,
129,245,15,116,161,38,144,205,249,233,211,87,65,2,221,220,223,192,92,248,239,207,159,127,184,18,13,242,44,249,79,96,32,176,2,205,103,6,97,96,195,149,233,209,163,151,200,247,9,32,71,46,250,162,175,191,
104,237,3,98,75,187,191,12,168,107,129,192,215,6,1,221,193,2,172,14,152,129,24,60,29,2,44,214,209,151,121,34,207,179,33,187,5,157,102,96,32,110,231,193,127,180,196,14,43,21,88,126,253,250,3,194,160,99,
52,152,161,238,96,196,18,38,200,75,83,144,39,144,255,50,16,177,106,17,32,128,168,150,96,64,235,61,129,110,7,93,88,196,102,98,162,255,117,201,146,202,223,155,54,29,103,6,237,85,6,230,114,49,96,149,161,
141,79,63,176,136,127,15,12,116,240,38,118,92,141,79,44,145,15,187,81,142,17,75,224,160,7,208,63,180,226,254,63,153,237,42,6,6,196,102,191,63,12,136,173,56,204,72,9,133,9,139,190,127,120,48,169,110,249,
135,68,195,220,193,140,230,14,38,164,156,76,200,29,127,137,76,176,12,0,1,68,181,4,3,172,67,153,183,110,61,97,222,216,184,192,55,45,205,231,180,177,177,250,189,148,20,175,127,87,175,62,144,234,235,91,147,
8,76,249,98,248,244,3,219,28,239,65,117,233,151,47,63,254,19,81,61,252,69,242,36,242,254,41,108,106,145,3,130,26,51,174,255,208,74,29,38,180,34,150,17,135,91,24,112,12,195,83,218,206,67,46,69,25,25,240,
95,196,129,173,138,69,47,205,241,2,128,0,162,90,130,1,21,135,63,127,126,227,57,125,250,102,4,16,231,1,235,210,107,192,58,253,43,176,103,162,2,108,160,17,60,99,69,79,79,233,209,139,23,239,190,3,171,46,
66,251,137,176,237,237,97,36,160,150,22,128,20,119,208,195,45,255,177,180,17,168,238,22,128,0,162,86,130,97,2,182,214,89,120,121,185,224,199,63,3,27,119,90,63,126,16,55,24,9,76,88,79,128,221,224,199,192,
22,251,95,10,170,139,193,0,6,211,237,231,52,113,11,64,0,81,35,193,128,139,62,96,195,19,212,51,32,241,158,98,8,208,208,144,3,141,75,220,132,30,90,51,50,175,156,31,34,0,32,128,168,53,151,4,218,56,206,12,
44,81,72,190,87,17,52,71,100,103,167,183,238,206,157,39,159,159,62,125,243,135,129,70,155,195,70,1,117,0,64,0,81,109,242,17,116,2,37,104,171,41,137,9,230,151,171,171,241,66,96,123,231,212,190,125,23,190,
48,144,182,79,121,20,12,0,0,8,32,170,37,152,183,111,63,253,3,157,222,200,195,195,249,150,24,245,236,236,108,111,65,147,128,192,234,104,205,218,181,135,94,3,75,151,31,12,228,237,85,30,5,116,4,0,1,68,141,
201,71,112,183,246,237,219,143,204,10,10,18,191,221,220,76,46,8,10,242,222,228,224,96,125,207,201,201,254,19,116,100,216,191,127,12,160,35,180,126,2,19,211,39,208,46,58,29,29,197,77,62,62,22,115,89,89,
153,15,109,217,114,226,249,163,71,175,96,147,128,196,28,155,49,10,6,16,0,4,16,35,149,204,0,37,60,54,62,62,110,78,57,57,113,62,57,57,81,65,96,194,16,0,109,96,3,118,183,65,43,229,133,65,231,154,241,240,
112,188,227,228,100,123,247,234,213,135,239,87,175,62,252,112,255,254,243,207,223,190,253,128,237,125,65,94,98,48,10,6,41,0,8,32,106,237,173,132,29,113,6,218,237,8,218,249,200,193,194,194,194,14,44,65,
216,5,5,121,88,4,4,120,152,128,189,32,208,249,43,255,222,188,249,8,154,219,1,221,19,4,219,204,143,60,183,67,179,173,12,163,128,58,0,32,128,24,169,104,14,108,152,30,148,104,96,183,130,192,206,199,67,62,
144,8,54,143,129,124,139,202,223,209,196,50,52,0,64,0,49,82,217,44,88,245,132,126,19,61,174,9,48,228,33,254,209,196,50,4,0,64,128,1,0,102,20,176,16,7,130,12,56,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* RenderingTestComponent::demoPng_png = (const char*) resource_RenderingTestComponent_demoPng_png;
const int RenderingTestComponent::demoPng_pngSize = 15290;
