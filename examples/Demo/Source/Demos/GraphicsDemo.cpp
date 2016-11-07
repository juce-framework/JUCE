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

#include "../JuceDemoHeader.h"


/** Holds the various toggle buttons for the animation modes. */
class ControllersComponent  : public Component
{
public:
    ControllersComponent()
    {
        setOpaque (true);
        initialiseToggle (animatePosition, "Animate Position", true);
        initialiseToggle (animateRotation, "Animate Rotation", true);
        initialiseToggle (animateSize, "Animate Size", false);
        initialiseToggle (animateShear, "Animate Shearing", false);
        initialiseToggle (animateAlpha, "Animate Alpha", false);
        initialiseToggle (clipToRectangle, "Clip to Rectangle", false);
        initialiseToggle (clipToPath, "Clip to Path", false);
        initialiseToggle (clipToImage, "Clip to Image", false);
        initialiseToggle (quality, "Higher quality image interpolation", false);
    }

    void paint (Graphics& g) override
    {
        fillStandardDemoBackground (g);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (4));

        int buttonHeight = 22;

        Rectangle<int> columns (r.removeFromTop (buttonHeight * 4));
        Rectangle<int> col (columns.removeFromLeft (200));

        animatePosition.setBounds (col.removeFromTop (buttonHeight));
        animateRotation.setBounds (col.removeFromTop (buttonHeight));
        animateSize.setBounds (col.removeFromTop (buttonHeight));
        animateShear.setBounds (col.removeFromTop (buttonHeight));

        columns.removeFromLeft (20);
        col = columns.removeFromLeft (200);

        animateAlpha.setBounds (col.removeFromTop (buttonHeight));
        clipToRectangle.setBounds (col.removeFromTop (buttonHeight));
        clipToPath.setBounds (col.removeFromTop (buttonHeight));
        clipToImage.setBounds (col.removeFromTop (buttonHeight));

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
        : Component (name), controls (cc),
          lastRenderStartTime (0),
          averageTimeMs (0),
          averageActualFPS (0)
    {
        displayFont = Font (Font::getDefaultMonospacedFontName(), 12.0f, Font::bold);
    }

    AffineTransform getTransform()
    {
        const float hw = 0.5f * getWidth();
        const float hh = 0.5f * getHeight();

        AffineTransform t;

        if (controls.animateRotation.getToggleState())
            t = t.rotated (rotation.getValue() * float_Pi * 2.0f);

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
        double startTime = 0.0;

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

        double now = Time::getMillisecondCounterHiRes();
        double filtering = 0.08;

        const double elapsedMs = now - startTime;
        averageTimeMs += (elapsedMs - averageTimeMs) * filtering;

        const double sinceLastRender = now - lastRenderStartTime;
        lastRenderStartTime = now;

        const double effectiveFPS = 1000.0 / averageTimeMs;
        const double actualFPS = sinceLastRender > 0 ? (1000.0 / sinceLastRender) : 0;
        averageActualFPS += (actualFPS - averageActualFPS) * filtering;

        GlyphArrangement ga;
        ga.addFittedText (displayFont,
                          "Time: " + String (averageTimeMs, 2)
                            + " ms\nEffective FPS: " + String (effectiveFPS, 1)
                            + "\nActual FPS: " + String (averageActualFPS, 1),
                          0, 10.0f, getWidth() - 10.0f, (float) getHeight(), Justification::topRight, 3);

        g.setColour (Colours::white.withAlpha (0.5f));
        g.fillRect (ga.getBoundingBox (0, ga.getNumGlyphs(), true).getSmallestIntegerContainer().expanded (4));

        g.setColour (Colours::black);
        ga.draw (g);
    }

    virtual void drawDemo (Graphics&) = 0;

    void clipToRectangle (Graphics& g)
    {
        int w = getWidth() / 2;
        int h = getHeight() / 2;

        int x = (int) (w * clipRectX.getValue());
        int y = (int) (h * clipRectY.getValue());

        g.reduceClipRegion (x, y, w, h);
    }

    void clipToPath (Graphics& g)
    {
        float pathSize = (float) jmin (getWidth(), getHeight());

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

        AffineTransform transform (AffineTransform::translation (clipImage.getWidth() / -2.0f,
                                                                 clipImage.getHeight() / -2.0f)
                                   .rotated (clipImageAngle.getValue() * float_Pi * 2.0f)
                                   .scaled (2.0f + clipImageSize.getValue() * 3.0f)
                                   .translated (getWidth() * 0.5f,
                                                getHeight() * 0.5f));

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

    double lastRenderStartTime, averageTimeMs, averageActualFPS;
    Image clipImage;
    Font displayFont;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphicsDemoBase)
};

//==============================================================================
class RectangleFillTypesDemo  : public GraphicsDemoBase
{
public:
    RectangleFillTypesDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "Fill Types: Rectangles"),
          colour1 (Colours::red),
          colour2 (Colours::green)
    {
    }

    void drawDemo (Graphics& g) override
    {
        g.addTransform (getTransform());

        const int rectSize = jmin (getWidth(), getHeight()) / 2 - 20;

        g.setColour (colour1.withAlpha (getAlpha()));
        g.fillRect (-rectSize, -rectSize, rectSize, rectSize);

        g.setGradientFill (ColourGradient (colour1, 10.0f, (float) -rectSize,
                                           colour2, 10.0f + rectSize, 0.0f, false));
        g.setOpacity (getAlpha());
        g.fillRect (10, -rectSize, rectSize, rectSize);

        g.setGradientFill (ColourGradient (colour1, rectSize * -0.5f, 10.0f + rectSize * 0.5f,
                                           colour2, 0, 10.0f + rectSize, true));
        g.setOpacity (getAlpha());
        g.fillRect (-rectSize, 10, rectSize, rectSize);

        g.setGradientFill (ColourGradient (colour1, 10.0f, 10.0f,
                                           colour2, 10.0f + rectSize, 10.0f + rectSize, false));
        g.setOpacity (getAlpha());
        g.drawRect (10, 10, rectSize, rectSize, 5);
    }

    Colour colour1, colour2;
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
        logoPath = MainAppWindow::getJUCELogoPath();

        // rescale the logo path so that it's centred about the origin and has the right size.
        logoPath.applyTransform (RectanglePlacement (RectanglePlacement::centred)
                                 .getTransformToFit (logoPath.getBounds(),
                                                     Rectangle<float> (-120.0f, -120.0f, 240.0f, 240.0f)));

        // Surround it with some other shapes..
        logoPath.addStar (Point<float> (-300.0f, -50.0f), 7, 30.0f, 70.0f, 0.1f);
        logoPath.addStar (Point<float> (300.0f, 50.0f), 6, 40.0f, 70.0f, 0.1f);
        logoPath.addEllipse (-100.0f, 150.0f, 200.0f, 140.0f);
        logoPath.addRectangle (-100.0f, -280.0f, 200.0f, 140.0f);
    }

    void drawDemo (Graphics& g) override
    {
        const Path& p = logoPath;

        if (useLinearGradient || useRadialGradient)
        {
            Colour c1 (gradientColours[0].getValue(), gradientColours[1].getValue(), gradientColours[2].getValue(), 1.0f);
            Colour c2 (gradientColours[3].getValue(), gradientColours[4].getValue(), gradientColours[5].getValue(), 1.0f);
            Colour c3 (gradientColours[6].getValue(), gradientColours[7].getValue(), gradientColours[8].getValue(), 1.0f);

            float x1 = gradientPositions[0].getValue() * getWidth()  * 0.25f;
            float y1 = gradientPositions[1].getValue() * getHeight() * 0.25f;
            float x2 = gradientPositions[2].getValue() * getWidth()  * 0.75f;
            float y2 = gradientPositions[3].getValue() * getHeight() * 0.75f;

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
    {
    }

    void drawDemo (Graphics& g) override
    {
        float w = (float) getWidth();
        float h = (float) getHeight();

        Path p;
        p.startNewSubPath (points[0].getValue() * w,
                           points[1].getValue() * h);

        for (int i = 2; i < numElementsInArray (points); i += 4)
            p.quadraticTo (points[i].getValue() * w,
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
    ImagesRenderingDemo (ControllersComponent& cc, bool argb_, bool tiled_)
        : GraphicsDemoBase (cc, String ("Images") + (argb_ ? ": ARGB" : ": RGB") + (tiled_ ? " Tiled" : String() )),
          isArgb (argb_), isTiled (tiled_)
    {
        argbImage = ImageFileFormat::loadFrom (BinaryData::juce_icon_png, (size_t) BinaryData::juce_icon_pngSize);
        rgbImage = ImageFileFormat::loadFrom (BinaryData::portmeirion_jpg, (size_t) BinaryData::portmeirion_jpgSize);
    }

    void drawDemo (Graphics& g) override
    {
        Image image = isArgb ? argbImage : rgbImage;

        AffineTransform transform (AffineTransform::translation ((float) (image.getWidth() / -2),
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
        glyphs.addFittedText (Font (20.0f), "The Quick Brown Fox Jumped Over The Lazy Dog",
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

        MemoryInputStream iconsFileStream (BinaryData::icons_zip, BinaryData::icons_zipSize, false);
        ZipFile icons (&iconsFileStream, false);

        // Load a random SVG file from our embedded icons.zip file.
        const ScopedPointer<InputStream> svgFileStream (icons.createStreamForEntry (Random::getSystemRandom().nextInt (icons.getNumEntries())));

        if (svgFileStream != nullptr)
        {
            svgDrawable = dynamic_cast<DrawableComposite*> (Drawable::createFromImageDataStream (*svgFileStream));

            if (svgDrawable != nullptr)
            {
                // to make our icon the right size, we'll set its bounding box to the size and position that we want.
                svgDrawable->setBoundingBox (RelativeParallelogram (Point<float> (-100, -100),
                                                                    Point<float> (100, -100),
                                                                    Point<float> (-100, 100)));
            }
        }
    }

    Time lastSVGLoadTime;
    ScopedPointer<DrawableComposite> svgDrawable;
};

//==============================================================================
class LinesDemo  : public GraphicsDemoBase
{
public:
    LinesDemo (ControllersComponent& cc)
        : GraphicsDemoBase (cc, "Lines")
    {
    }

    void drawDemo (Graphics& g) override
    {
        {
            RectangleList<float> verticalLines;
            verticalLines.ensureStorageAllocated (getWidth());

            float pos = offset.getValue();

            for (int x = 0; x < getWidth(); ++x)
            {
                float y = getHeight() * 0.3f;
                float length = y * std::abs (std::sin (x / 100.0f + 2.0f * pos));
                verticalLines.addWithoutMerging (Rectangle<float> ((float) x, y - length * 0.5f, 1.0f, length));
            }

            g.setColour (Colours::blue.withAlpha (getAlpha()));
            g.fillRectList (verticalLines);
        }

        {
            RectangleList<float> horizontalLines;
            horizontalLines.ensureStorageAllocated (getHeight());

            float pos = offset.getValue();

            for (int y = 0; y < getHeight(); ++y)
            {
                float x = getWidth() * 0.3f;
                float length = x * std::abs (std::sin (y / 100.0f + 2.0f * pos));
                horizontalLines.addWithoutMerging (Rectangle<float> (x - length * 0.5f, (float) y, length, 1.0f));
            }

            g.setColour (Colours::green.withAlpha (getAlpha()));
            g.fillRectList (horizontalLines);
        }

        g.setColour (Colours::red.withAlpha (getAlpha()));

        const float w = (float) getWidth();
        const float h = (float) getHeight();

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
        : currentDemo (nullptr)
    {
        setOpaque (true);
    }

    void paint (Graphics& g) override
    {
        g.fillCheckerBoard (getLocalBounds(), 48, 48,
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
    GraphicsDemoBase* currentDemo;
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
        demos.add (new PathsDemo (controls, true, false));
        demos.add (new PathsDemo (controls, false, false));
        demos.add (new RectangleFillTypesDemo (controls));
        demos.add (new StrokesDemo (controls));
        demos.add (new ImagesRenderingDemo (controls, false, false));
        demos.add (new ImagesRenderingDemo (controls, false, true));
        demos.add (new ImagesRenderingDemo (controls, true, false));
        demos.add (new ImagesRenderingDemo (controls, true, true));
        demos.add (new GlyphsDemo (controls));
        demos.add (new SVGDemo (controls));
        demos.add (new LinesDemo (controls));

        addAndMakeVisible (listBox);
        listBox.setModel (this);
        listBox.selectRow (0);
        listBox.setColour (ListBox::backgroundColourId, Colour::greyLevel (0.9f));
    }

    void resized()
    {
        listBox.setBounds (getLocalBounds());
    }

    int getNumRows()
    {
        return demos.size();
    }

    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
    {
        Component* demo = demos [rowNumber];

        if (demo != nullptr)
        {
            if (rowIsSelected)
                g.fillAll (findColour (TextEditor::highlightColourId));

            g.setColour (Colours::black);
            g.setFont (14.0f);
            g.drawFittedText (demo->getName(), 8, 0, width - 10, height, Justification::centredLeft, 2);
        }
    }

    void selectedRowsChanged (int lastRowSelected)
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
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::grey);
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds());
        controllersComponent.setBounds (area.removeFromBottom (150));
        testList.setBounds (area.removeFromRight (150));
        demoHolder.setBounds (area);
        performanceDisplay.setBounds (area.removeFromTop (20).removeFromRight (100));
    }

private:
    ControllersComponent controllersComponent;
    DemoHolderComponent demoHolder;
    Label performanceDisplay;
    TestListComponent testList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphicsDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<GraphicsDemo> demo ("20 Graphics: 2D Rendering");
