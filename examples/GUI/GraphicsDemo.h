/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             GraphicsDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Showcases various graphics features.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        GraphicsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/** Holds the various toggle buttons for the animation modes. */
class ControllersComponent  : public Component
{
public:
    ControllersComponent()
    {
        setOpaque (true);

        initialiseToggle (animatePosition, "Animate Position",  true);
        initialiseToggle (animateRotation, "Animate Rotation",  true);
        initialiseToggle (animateSize,     "Animate Size",      false);
        initialiseToggle (animateShear,    "Animate Shearing",  false);
        initialiseToggle (animateAlpha,    "Animate Alpha",     false);
        initialiseToggle (clipToRectangle, "Clip to Rectangle", false);
        initialiseToggle (clipToPath,      "Clip to Path",      false);
        initialiseToggle (clipToImage,     "Clip to Image",     false);
        initialiseToggle (quality,         "Higher quality image interpolation", false);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (4);

        int buttonHeight = 22;

        auto columns = r.removeFromTop (buttonHeight * 4);
        auto col = columns.removeFromLeft (200);

        animatePosition.setBounds (col.removeFromTop (buttonHeight));
        animateRotation.setBounds (col.removeFromTop (buttonHeight));
        animateSize    .setBounds (col.removeFromTop (buttonHeight));
        animateShear   .setBounds (col.removeFromTop (buttonHeight));

        columns.removeFromLeft (20);
        col = columns.removeFromLeft (200);

        animateAlpha   .setBounds (col.removeFromTop (buttonHeight));
        clipToRectangle.setBounds (col.removeFromTop (buttonHeight));
        clipToPath     .setBounds (col.removeFromTop (buttonHeight));
        clipToImage    .setBounds (col.removeFromTop (buttonHeight));

        r.removeFromBottom (6);
        quality.setBounds (r.removeFromTop (buttonHeight));
    }

    void initialiseToggle (ToggleButton& b, const char* name, bool on)
    {
        addAndMakeVisible (b);
        b.setButtonText (name);
        b.setToggleState (on, dontSendNotification);
    }

    ToggleButton animateRotation, animatePosition, animateAlpha, animateSize, animateShear;
    ToggleButton clipToRectangle, clipToPath, clipToImage, quality;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControllersComponent)
};

//==============================================================================
class GraphicsDemoBase  : public Component
{
public:
    GraphicsDemoBase (ControllersComponent& cc, const String& name)
        : Component (name),
          controls (cc)
    {
        displayFont = Font (Font::getDefaultMonospacedFontName(), 12.0f, Font::bold);
    }

    AffineTransform getTransform()
    {
        auto hw = 0.5f * (float) getWidth();
        auto hh = 0.5f * (float) getHeight();

        AffineTransform t;

        if (controls.animateRotation.getToggleState())
            t = t.rotated (rotation.getValue() * MathConstants<float>::twoPi);

        if (controls.animateSize.getToggleState())
            t = t.scaled (0.3f + size.getValue() * 2.0f);

        if (controls.animatePosition.getToggleState())
            t = t.translated (hw + hw * (offsetX.getValue() - 0.5f),
                              hh + hh * (offsetY.getValue() - 0.5f));
        else
            t = t.translated (hw, hh);

        if (controls.animateShear.getToggleState())
            t = t.sheared (shear.getValue() * 2.0f - 1.0f, 0.0f);

        return t;
    }

    float getAlpha() const
    {
        if (controls.animateAlpha.getToggleState())
            return alpha.getValue();

        return 1.0f;
    }

    void paint (Graphics& g) override
    {
        auto startTime = 0.0;

        {
            // A ScopedSaveState will return the Graphics context to the state it was at the time of
            // construction when it goes out of scope. We use it here to avoid clipping the fps text
            const Graphics::ScopedSaveState state (g);

            if (controls.clipToRectangle.getToggleState())  clipToRectangle (g);
            if (controls.clipToPath     .getToggleState())  clipToPath (g);
            if (controls.clipToImage    .getToggleState())  clipToImage (g);

            g.setImageResamplingQuality (controls.quality.getToggleState() ? Graphics::highResamplingQuality
                                                                           : Graphics::mediumResamplingQuality);

            // take a note of the time before the render
            startTime = Time::getMillisecondCounterHiRes();

            // then let the demo draw itself..
            drawDemo (g);
        }

        auto now = Time::getMillisecondCounterHiRes();
        auto filtering = 0.08;

        auto elapsedMs = now - startTime;
        averageTimeMs += (elapsedMs - averageTimeMs) * filtering;

        auto sinceLastRender = now - lastRenderStartTime;
        lastRenderStartTime = now;

        auto effectiveFPS = 1000.0 / averageTimeMs;
        auto actualFPS = sinceLastRender > 0 ? (1000.0 / sinceLastRender) : 0;
        averageActualFPS += (actualFPS - averageActualFPS) * filtering;

        GlyphArrangement ga;
        ga.addFittedText (displayFont,
                          "Time: " + String (averageTimeMs, 2)
                            + " ms\nEffective FPS: " + String (effectiveFPS, 1)
                            + "\nActual FPS: " + String (averageActualFPS, 1),
                          0, 10.0f, (float) getWidth() - 10.0f, (float) getHeight(), Justification::topRight, 3);

        g.setColour (Colours::white.withAlpha (0.5f));
        g.fillRect (ga.getBoundingBox (0, ga.getNumGlyphs(), true).getSmallestIntegerContainer().expanded (4));

        g.setColour (Colours::black);
        ga.draw (g);
    }

    virtual void drawDemo (Graphics&) = 0;

    void clipToRectangle (Graphics& g)
    {
        auto w = getWidth()  / 2;
        auto h = getHeight() / 2;

        auto x = (int) ((float) w * clipRectX.getValue());
        auto y = (int) ((float) h * clipRectY.getValue());

        g.reduceClipRegion (x, y, w, h);
    }

    void clipToPath (Graphics& g)
    {
        auto pathSize = (float) jmin (getWidth(), getHeight());

        Path p;
        p.addStar (Point<float> (clipPathX.getValue(),
                                 clipPathY.getValue()) * pathSize,
                   7,
                   pathSize * (0.5f + clipPathDepth.getValue()),
                   pathSize * 0.5f,
                   clipPathAngle.getValue());

        g.reduceClipRegion (p, AffineTransform());
    }

    void clipToImage (Graphics& g)
    {
        if (! clipImage.isValid())
            createClipImage();

        AffineTransform transform (AffineTransform::translation ((float) clipImage.getWidth()  / -2.0f,
                                                                 (float) clipImage.getHeight() / -2.0f)
                                   .rotated (clipImageAngle.getValue() * MathConstants<float>::twoPi)
                                   .scaled (2.0f + clipImageSize.getValue() * 3.0f)
                                   .translated ((float) getWidth()  * 0.5f,
                                                (float) getHeight() * 0.5f));

        g.reduceClipRegion (clipImage, transform);
    }

    void createClipImage()
    {
        clipImage = Image (Image::ARGB, 300, 300, true);

        Graphics g (clipImage);

        g.setGradientFill (ColourGradient (Colours::transparentBlack, 0, 0,
                                           Colours::black, 0, 300, false));

        for (int i = 0; i < 20; ++i)
            g.fillRect (Random::getSystemRandom().nextInt (200),
                        Random::getSystemRandom().nextInt (200),
                        Random::getSystemRandom().nextInt (100),
                        Random::getSystemRandom().nextInt (100));
    }

    //==============================================================================
    ControllersComponent& controls;

    SlowerBouncingNumber offsetX, offsetY, rotation, size, shear, alpha, clipRectX,
                         clipRectY, clipPathX, clipPathY, clipPathDepth, clipPathAngle,
                         clipImageX, clipImageY, clipImageAngle, clipImageSize;

    double lastRenderStartTime = 0.0, averageTimeMs = 0.0, averageActualFPS = 0.0;
    Image clipImage;
    Font displayFont;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphicsDemoBase)
};

//==============================================================================
class RectangleFillTypesDemo  : public GraphicsDemoBase
{
public:
    RectangleFillTypesDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "Fill Types: Rectangles")
    {}

    void drawDemo (Graphics& g) override
    {
        g.addTransform (getTransform());

        const int rectSize = jmin (getWidth(), getHeight()) / 2 - 20;

        g.setColour (colour1.withAlpha (getAlpha()));
        g.fillRect (-rectSize, -rectSize, rectSize, rectSize);

        g.setGradientFill (ColourGradient (colour1, 10.0f, (float) -rectSize,
                                           colour2, 10.0f + (float) rectSize, 0.0f, false));
        g.setOpacity (getAlpha());
        g.fillRect (10, -rectSize, rectSize, rectSize);

        g.setGradientFill (ColourGradient (colour1, (float) rectSize * -0.5f, 10.0f + (float) rectSize * 0.5f,
                                           colour2, 0, 10.0f + (float) rectSize, true));
        g.setOpacity (getAlpha());
        g.fillRect (-rectSize, 10, rectSize, rectSize);

        g.setGradientFill (ColourGradient (colour1, 10.0f, 10.0f,
                                           colour2, 10.0f + (float) rectSize, 10.0f + (float) rectSize, false));
        g.setOpacity (getAlpha());
        g.drawRect (10, 10, rectSize, rectSize, 5);
    }

    Colour colour1 { Colours::red }, colour2 { Colours::green };
};

//==============================================================================
class PathsDemo  : public GraphicsDemoBase
{
public:
    PathsDemo (ControllersComponent& cc, bool linear, bool radial)
        : GraphicsDemoBase (cc, String ("Paths") + (radial ? ": Radial Gradients"
                                                           : (linear ? ": Linear Gradients"
                                                                     : ": Solid"))),
          useLinearGradient (linear), useRadialGradient (radial)
    {
        logoPath = getJUCELogoPath();

        // rescale the logo path so that it's centred about the origin and has the right size.
        logoPath.applyTransform (RectanglePlacement (RectanglePlacement::centred)
                                 .getTransformToFit (logoPath.getBounds(),
                                                     Rectangle<float> (-120.0f, -120.0f, 240.0f, 240.0f)));

        // Surround it with some other shapes..
        logoPath.addStar ({ -300.0f, -50.0f }, 7, 30.0f, 70.0f, 0.1f);
        logoPath.addStar ({ 300.0f, 50.0f }, 6, 40.0f, 70.0f, 0.1f);
        logoPath.addEllipse (-100.0f, 150.0f, 200.0f, 140.0f);
        logoPath.addRectangle (-100.0f, -280.0f, 200.0f, 140.0f);
    }

    void drawDemo (Graphics& g) override
    {
        auto& p = logoPath;

        if (useLinearGradient || useRadialGradient)
        {
            Colour c1 (gradientColours[0].getValue(), gradientColours[1].getValue(), gradientColours[2].getValue(), 1.0f);
            Colour c2 (gradientColours[3].getValue(), gradientColours[4].getValue(), gradientColours[5].getValue(), 1.0f);
            Colour c3 (gradientColours[6].getValue(), gradientColours[7].getValue(), gradientColours[8].getValue(), 1.0f);

            auto x1 = gradientPositions[0].getValue() * (float) getWidth()  * 0.25f;
            auto y1 = gradientPositions[1].getValue() * (float) getHeight() * 0.25f;
            auto x2 = gradientPositions[2].getValue() * (float) getWidth()  * 0.75f;
            auto y2 = gradientPositions[3].getValue() * (float) getHeight() * 0.75f;

            ColourGradient gradient (c1, x1, y1,
                                     c2, x2, y2,
                                     useRadialGradient);

            gradient.addColour (gradientIntermediate.getValue(), c3);

            g.setGradientFill (gradient);
        }
        else
        {
            g.setColour (Colours::blue);
        }

        g.setOpacity (getAlpha());
        g.fillPath (p, getTransform());
    }

    Path logoPath;
    bool useLinearGradient, useRadialGradient;
    SlowerBouncingNumber gradientColours[9], gradientPositions[4], gradientIntermediate;
};

//==============================================================================
class StrokesDemo  : public GraphicsDemoBase
{
public:
    StrokesDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "Paths: Stroked")
    {}

    void drawDemo (Graphics& g) override
    {
        auto w = (float) getWidth();
        auto h = (float) getHeight();

        Path p;
        p.startNewSubPath (points[0].getValue() * w,
                           points[1].getValue() * h);

        for (int i = 2; i < numElementsInArray (points); i += 4)
            p.quadraticTo (points[i]    .getValue() * w,
                           points[i + 1].getValue() * h,
                           points[i + 2].getValue() * w,
                           points[i + 3].getValue() * h);

        p.closeSubPath();

        PathStrokeType stroke (0.5f + 10.0f * thickness.getValue());
        g.setColour (Colours::purple.withAlpha (getAlpha()));
        g.strokePath (p, stroke, AffineTransform());
    }

    SlowerBouncingNumber points[2 + 4 * 8], thickness;
};

//==============================================================================
class ImagesRenderingDemo  : public GraphicsDemoBase
{
public:
    ImagesRenderingDemo (ControllersComponent& cc, bool argb, bool tiled)
        : GraphicsDemoBase (cc, String ("Images") + (argb ? ": ARGB" : ": RGB") + (tiled ? " Tiled" : String() )),
          isArgb (argb), isTiled (tiled)
    {
        argbImage = getImageFromAssets ("juce_icon.png");
        rgbImage  = getImageFromAssets ("portmeirion.jpg");
    }

    void drawDemo (Graphics& g) override
    {
        auto image = isArgb ? argbImage : rgbImage;

        AffineTransform transform (AffineTransform::translation ((float) (image.getWidth()  / -2),
                                                                 (float) (image.getHeight() / -2))
                                   .followedBy (getTransform()));

        if (isTiled)
        {
            FillType fill (image, transform);
            fill.setOpacity (getAlpha());
            g.setFillType (fill);
            g.fillAll();
        }
        else
        {
            g.setOpacity (getAlpha());
            g.drawImageTransformed (image, transform, false);
        }
    }

    bool isArgb, isTiled;
    Image rgbImage, argbImage;
};

//==============================================================================
class GlyphsDemo  : public GraphicsDemoBase
{
public:
    GlyphsDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "Glyphs")
    {
        glyphs.addFittedText ({ 20.0f }, "The Quick Brown Fox Jumped Over The Lazy Dog",
                              -120, -50, 240, 100, Justification::centred, 2, 1.0f);
    }

    void drawDemo (Graphics& g) override
    {
        g.setColour (Colours::black.withAlpha (getAlpha()));
        glyphs.draw (g, getTransform());
    }

    GlyphArrangement glyphs;
};

//==============================================================================
class SVGDemo  : public GraphicsDemoBase
{
public:
    SVGDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "SVG")
    {
        createSVGDrawable();
    }

    void drawDemo (Graphics& g) override
    {
        if (Time::getCurrentTime().toMilliseconds() > lastSVGLoadTime.toMilliseconds() + 2000)
            createSVGDrawable();

        svgDrawable->draw (g, getAlpha(), getTransform());
    }

    void createSVGDrawable()
    {
        lastSVGLoadTime = Time::getCurrentTime();

        ZipFile icons (createAssetInputStream ("icons.zip").release(), true);

        // Load a random SVG file from our embedded icons.zip file.
        const std::unique_ptr<InputStream> svgFileStream (icons.createStreamForEntry (Random::getSystemRandom().nextInt (icons.getNumEntries())));

        if (svgFileStream.get() != nullptr)
        {
            svgDrawable = Drawable::createFromImageDataStream (*svgFileStream);

            if (svgDrawable != nullptr)
            {
                // to make our icon the right size, we'll set its bounding box to the size and position that we want.

                if (auto comp = dynamic_cast<DrawableComposite*> (svgDrawable.get()))
                    comp->setBoundingBox ({ -100.0f, -100.0f, 200.0f, 200.0f });
            }
        }
    }

    Time lastSVGLoadTime;
    std::unique_ptr<Drawable> svgDrawable;
};

//==============================================================================
class LinesDemo  : public GraphicsDemoBase
{
public:
    LinesDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "Lines")
    {}

    void drawDemo (Graphics& g) override
    {
        {
            RectangleList<float> verticalLines;
            verticalLines.ensureStorageAllocated (getWidth());

            auto pos = offset.getValue();

            for (int x = 0; x < getWidth(); ++x)
            {
                auto y = (float) getHeight() * 0.3f;
                auto length = y * std::abs (std::sin ((float) x / 100.0f + 2.0f * pos));
                verticalLines.addWithoutMerging (Rectangle<float> ((float) x, y - length * 0.5f, 1.0f, length));
            }

            g.setColour (Colours::blue.withAlpha (getAlpha()));
            g.fillRectList (verticalLines);
        }

        {
            RectangleList<float> horizontalLines;
            horizontalLines.ensureStorageAllocated (getHeight());

            auto pos = offset.getValue();

            for (int y = 0; y < getHeight(); ++y)
            {
                auto x = (float) getWidth() * 0.3f;
                auto length = x * std::abs (std::sin ((float) y / 100.0f + 2.0f * pos));
                horizontalLines.addWithoutMerging (Rectangle<float> (x - length * 0.5f, (float) y, length, 1.0f));
            }

            g.setColour (Colours::green.withAlpha (getAlpha()));
            g.fillRectList (horizontalLines);
        }

        g.setColour (Colours::red.withAlpha (getAlpha()));

        auto w = (float) getWidth();
        auto h = (float) getHeight();

        g.drawLine (positions[0].getValue() * w,
                    positions[1].getValue() * h,
                    positions[2].getValue() * w,
                    positions[3].getValue() * h);

        g.drawLine (positions[4].getValue() * w,
                    positions[5].getValue() * h,
                    positions[6].getValue() * w,
                    positions[7].getValue() * h);
    }

    SlowerBouncingNumber offset, positions[8];
};

//==============================================================================
class DemoHolderComponent  : public Component,
                             private Timer
{
public:
    DemoHolderComponent()
    {
        setOpaque (true);
    }

    void paint (Graphics& g) override
    {
        g.fillCheckerBoard (getLocalBounds().toFloat(), 48.0f, 48.0f,
                            Colours::lightgrey, Colours::white);
    }

    void timerCallback() override
    {
        if (currentDemo != nullptr)
            currentDemo->repaint();
    }

    void setDemo (GraphicsDemoBase* newDemo)
    {
        if (currentDemo != nullptr)
            removeChildComponent (currentDemo);

        currentDemo = newDemo;

        if (currentDemo != nullptr)
        {
            addAndMakeVisible (currentDemo);
            startTimerHz (60);
            resized();
        }
    }

    void resized() override
    {
        if (currentDemo != nullptr)
            currentDemo->setBounds (getLocalBounds());
    }

private:
    GraphicsDemoBase* currentDemo = nullptr;
};

//==============================================================================
class TestListComponent   : public Component,
                            private ListBoxModel
{
public:
    TestListComponent (DemoHolderComponent& holder, ControllersComponent& controls)
        : demoHolder (holder)
    {
        demos.add (new PathsDemo (controls, false, true));
        demos.add (new PathsDemo (controls, true,  false));
        demos.add (new PathsDemo (controls, false, false));
        demos.add (new RectangleFillTypesDemo (controls));
        demos.add (new StrokesDemo (controls));
        demos.add (new ImagesRenderingDemo (controls, false, false));
        demos.add (new ImagesRenderingDemo (controls, false, true));
        demos.add (new ImagesRenderingDemo (controls, true,  false));
        demos.add (new ImagesRenderingDemo (controls, true,  true));
        demos.add (new GlyphsDemo (controls));
        demos.add (new SVGDemo    (controls));
        demos.add (new LinesDemo  (controls));

        addAndMakeVisible (listBox);
        listBox.setTitle ("Test List");
        listBox.setModel (this);
        listBox.selectRow (0);
    }

    void resized() override
    {
        listBox.setBounds (getLocalBounds());
    }

    int getNumRows() override
    {
        return demos.size();
    }

    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (demos[rowNumber] == nullptr)
            return;

        if (rowIsSelected)
            g.fillAll (Colour::contrasting (findColour (ListBox::textColourId),
                                            findColour (ListBox::backgroundColourId)));

        g.setColour (findColour (ListBox::textColourId));
        g.setFont (14.0f);
        g.drawFittedText (getNameForRow (rowNumber), 8, 0, width - 10, height, Justification::centredLeft, 2);
    }

    String getNameForRow (int rowNumber) override
    {
        if (auto* demo = demos[rowNumber])
            return demo->getName();

        return {};
    }

    void selectedRowsChanged (int lastRowSelected) override
    {
        demoHolder.setDemo (demos [lastRowSelected]);
    }

private:
    DemoHolderComponent& demoHolder;
    ListBox listBox;
    OwnedArray<GraphicsDemoBase> demos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestListComponent)
};

//==============================================================================
class GraphicsDemo  : public Component
{
public:
    GraphicsDemo()
        : testList (demoHolder, controllersComponent)
    {
        setOpaque (true);

        addAndMakeVisible (demoHolder);
        addAndMakeVisible (controllersComponent);
        addAndMakeVisible (performanceDisplay);
        addAndMakeVisible (testList);

        setSize (750, 750);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::grey);
    }

    void resized() override
    {
        auto area = getLocalBounds();

        controllersComponent.setBounds (area.removeFromBottom (150));
        testList            .setBounds (area.removeFromRight (150));
        demoHolder          .setBounds (area);
        performanceDisplay  .setBounds (area.removeFromTop (20).removeFromRight (100));
    }

private:
    ControllersComponent controllersComponent;
    DemoHolderComponent demoHolder;
    Label performanceDisplay;
    TestListComponent testList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphicsDemo)
};
