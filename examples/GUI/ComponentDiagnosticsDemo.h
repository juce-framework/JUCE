/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             ComponentDiagnosticsDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays various types of windows.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, vs2026, linux_make, androidstudio,
                   xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        ComponentDiagnosticsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

struct UserDiagnostics
{
    TimedDiagnostic user0;
    TimedDiagnostic user1;

    using Callback = std::function<void (const UserDiagnostics&)>;
};

static float getAnimatedValue()
{
    return (float) std::sin (std::fmod (Time::getMillisecondCounterHiRes() / 1000.0,
                                        MathConstants<double>::pi));
}

//==============================================================================
struct CheapComponent : public Component,
                        public SettableTooltipClient
{
    CheapComponent()
    {
        setName ("Cheap");
        setTooltip ("A simple component that fills its bounds red.");
    }

    void paint (Graphics& g) override
    {
        g.setColour (Colours::red);
        g.fillRect (getLocalBounds());
    }
};

struct CheapUnclippedComponent : public CheapComponent
{
    CheapUnclippedComponent()
    {
        setName ("Cheap (unclipped)");
        setTooltip ("A simple component that fills its bounds red, and calls setPaintingIsUnclipped (true).");
        setPaintingIsUnclipped (true);
    }
};

struct CheapOpaqueComponent : public CheapComponent
{
    CheapOpaqueComponent()
    {
        setName ("Cheap (opaque)");
        setTooltip ("A simple component that fills its bounds red, and calls setOpaque (true).");
        setOpaque (true);
    }
};

struct CheapUnclippedOpaqueComponent : public CheapComponent
{
    CheapUnclippedOpaqueComponent()
    {
        setName ("Cheap (unclipped & opaque)");
        setTooltip ("A simple component that fills its bounds red, and calls setPaintingIsUnclipped (true) and setOpaque (true).");
        setPaintingIsUnclipped (true);
        setOpaque (true);
    }
};

struct CheapBufferedToImageComponent : public CheapComponent
{
    CheapBufferedToImageComponent()
    {
        setName ("Cheap (buffered to image)");
        setTooltip ("A simple component that fills its bounds red and buffers the component to an image using setBufferedToImage (true).\n\n"
                    "Click the component to invalidate the cached image.");
        setBufferedToImage (true);
    }

    void mouseDown (const MouseEvent&) final
    {
        repaint();
    }
};

struct CheapUserDiagnosticComponent : public CheapComponent
{
    explicit CheapUserDiagnosticComponent (UserDiagnostics::Callback cb)
        : callback (std::move (cb))
    {
        setName ("Cheap (user diagnostic example)");
        setTooltip ("A simple component that fills its bounds red and demonstrates adding custom user diagnostics.");
    }

    void paint (Graphics& g) final
    {
        UserDiagnostics diagnostics;

        {
            const auto timer = diagnostics.user0.createTimer();
            CheapComponent::paint (g);
        }

        callback (diagnostics);
    }

    UserDiagnostics::Callback callback;
};

//==============================================================================
struct AnimatedComponent : public Component,
                           public SettableTooltipClient
{
    AnimatedComponent()
    {
        setName ("Animated");
        setTooltip ("A simple component that triggers a repaint at the screen refresh rate.\n"
                    "Useful as a proxy for any component that regularly redraws.");
    }

    void paint (Graphics& g) final
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto t = getAnimatedValue();
        const auto size = jmin (bounds.getWidth(), bounds.getHeight()) * t;
        g.setColour (Colours::green);
        g.fillEllipse (bounds.withSizeKeepingCentre (size, size));
    }

    VBlankAttachment vblank { this, [&] { repaint(); } };
};

//==============================================================================
struct PaintOverChildrenComponent : public Component,
                                    public SettableTooltipClient
{
    PaintOverChildrenComponent()
    {
        setName ("Paint over children");
        child.setTooltip ("A simple component that a yellow square in its paint() method"
                          "and a second rotating semi-transparent orange square in its paintOverChildren() method."
                          "The component also contains an animated child.\n\n"
                          "Notice how the graph shows the time spent in the paintOverChildren() function.");
        addAndMakeVisible (child);
    }

    void paint (Graphics& g)
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto size = jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        g.setColour (Colours::yellow);
        g.fillRect (bounds.withSizeKeepingCentre (size, size));
    }

    void paintOverChildren (Graphics& g)
    {
        const auto bounds = getLocalBounds().toFloat();
        g.addTransform (AffineTransform::rotation (getAnimatedValue() * MathConstants<float>::twoPi,
                                                   bounds.getCentreX(),
                                                   bounds.getCentreY()));

        const auto size = jmin (bounds.getWidth(), bounds.getHeight()) * (1.0f - getAnimatedValue()) * 0.5f;
        g.setColour (Colours::darkorange.withAlpha (0.8f));
        g.fillRect (bounds.withSizeKeepingCentre (size, size));
    }

    void resized() final
    {
        child.setBounds (getLocalBounds());
    }

    AnimatedComponent child;
};

//==============================================================================
struct PixelateEffectComponent : public Component,
                                 private SettableTooltipClient
{
    PixelateEffectComponent()
    {
        setName ("Pixelate");
        setTooltip ("An animated component which applies a pixelation effect.\n\n"
                    "Notice how the graph shows the time spent in the applyEffect() function.");
        setComponentEffect (&effect);
    }

    void paint (Graphics& g) final
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto size = jmin (bounds.getWidth(), bounds.getHeight());
        g.setColour (Colours::blue);
        g.fillEllipse (bounds.withSizeKeepingCentre (size, size));
    }

    struct PixelateEffect : public ImageEffectFilter
    {
        virtual void applyEffect (Image& sourceImage,
                                  Graphics& destContext,
                                  float scaleFactor,
                                  float alpha)
        {
            const auto w = (float) sourceImage.getWidth();
            const auto h = (float) sourceImage.getHeight();
            const auto size = jmap (getAnimatedValue(), 1.0f, w / (2.0f * scaleFactor));

            Image smallImage { Image::ARGB,
                               roundToInt (w / size),
                               roundToInt (h / size),
                               true };

            {
                Graphics g { smallImage };
                g.setOpacity (alpha);
                g.drawImage (sourceImage, smallImage.getBounds().toFloat());
            }

            destContext.setImageResamplingQuality (juce::Graphics::lowResamplingQuality);
            destContext.drawImage (smallImage, sourceImage.getBounds().toFloat());
        }
    };

    PixelateEffect effect;
};

//==============================================================================
struct ExpensiveComponent : public Component,
                            public SettableTooltipClient
{
    explicit ExpensiveComponent (UserDiagnostics::Callback cb)
        : callback (std::move (cb))
    {
        setName ("Expensive");
        setTooltip ("A component that is computationally expensive to draw.");
    }

    void paint (Graphics& g)
    {
        UserDiagnostics diagnostics;

        const auto scale = g.getInternalContext().getPhysicalPixelScaleFactor();
        const auto bounds = (getLocalBounds().toFloat() * scale).toNearestInt();

        Image image { Image::ARGB, bounds.getWidth(), bounds.getHeight(), true };

        {
            const auto timer = diagnostics.user0.createTimer();
            Graphics ig { image };

            ig.setGradientFill (ColourGradient (Colours::blueviolet,
                                                (float) bounds.getCentreX(),
                                                (float) bounds.getCentreY(),
                                                Colours::transparentBlack,
                                                (float) bounds.getWidth(),
                                                (float) bounds.getHeight(),
                                                true));

            ig.fillRect (bounds);
            ig.setFont (FontOptions { (float) image.getWidth() / 5.0f });
            ig.setColour (Colours::white.withAlpha (0.7f));
            ig.drawText ("JUCE", image.getBounds(), Justification::centred);
        }

        {
            const auto timer = diagnostics.user1.createTimer();

            if (auto ptr = image.getPixelData())
                ptr->applyGaussianBlurEffect (15.0f * scale);
        }

        {
            Graphics ig { image };
            ig.setFont (FontOptions { (float) image.getWidth() / 5.0f });
            ig.setColour (Colours::white.withAlpha (0.8f));
            ig.drawText ("JUCE", image.getBounds(), Justification::centred);
        }

        g.drawImage (image, getLocalBounds().toFloat());

        callback (diagnostics);
    }

    UserDiagnostics::Callback callback;
};

struct ExpensiveBufferedToImageComponent : public ExpensiveComponent
{
    explicit ExpensiveBufferedToImageComponent (UserDiagnostics::Callback cb)
        : ExpensiveComponent (std::move (cb))
    {
        setName ("Expensive (buffered to image)");
        setTooltip ("A component that is computationally expensive to draw and buffers the component to an image using setBufferedToImage (true).\n\n"
                    "Click the component to invalidate the cached image.");
        setBufferedToImage (true);
    }

    void mouseDown (const MouseEvent&) final
    {
        repaint();
    }
};

struct ExpensiveBufferedToImageWithChildComponent : public ExpensiveComponent
{
    explicit ExpensiveBufferedToImageWithChildComponent (UserDiagnostics::Callback cb)
        : ExpensiveComponent (std::move (cb))
    {
        setName ("Expensive (buffered to image) with child");

        // The child will always be at the front so use it's tooltip
        child.setTooltip ("A component that is computationally expensive to draw and buffers the component to an image. "
                          "The component also has a child component that triggers a repaint at the screen refresh rate.\n\n"
                          "Notice how the advantages of buffering a component to an image are lost due to the child continuously invalidating its parent.");

        setBufferedToImage (true);
        addAndMakeVisible (child);
    }

    void resized() final
    {
        child.setBounds (getLocalBounds());
    }

    void mouseDown (const MouseEvent&) final
    {
        repaint();
    }

    AnimatedComponent child;
};

struct ExpensiveBufferedToImageWithSiblingComponent : public Component,
                                                      public SettableTooltipClient
{
    explicit ExpensiveBufferedToImageWithSiblingComponent (UserDiagnostics::Callback cb)
        : background (std::move (cb))
    {
        setName ("Expensive (buffer to image) with sibling");

        // The child will always be at the front so use it's tooltip
        child.setTooltip ("A component containing two child components:\n"
                          "1. A component that is computationally expensive to draw and buffers the component to an image.\n"
                          "2. A simple component that triggers a repaint at the screen refresh rate.\n\n"
                          "Notice how the advantages of buffering a component to an image remain as sibling components do not invalidate other sibling components.");

        background.setBufferedToImage (true);

        addAndMakeVisible (background);
        addAndMakeVisible (child);
    }

    void resized() final
    {
        background.setBounds (getLocalBounds());
        child.setBounds (getLocalBounds());
    }

    void mouseDown (const MouseEvent&) final
    {
        background.repaint();
    }

    ExpensiveComponent background;
    AnimatedComponent child;
};

//==============================================================================
struct FontAnimatedHeightComponent : public Component,
                                     public SettableTooltipClient
{
    FontAnimatedHeightComponent()
    {
        setName ("Font (animated height)");
        setTooltip ("A component that dynamically adjusts font height.");
    }

    void paint (Graphics& g) final
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto scale = getAnimatedValue();
        const auto size = jmin (bounds.getWidth(), bounds.getHeight()) / 5.0 * scale;
        g.setFont (FontOptions{}.withHeight ((float) size));
        g.setColour (Colours::black);
        g.drawText (SystemStats::getJUCEVersion(), getLocalBounds(), Justification::topLeft, false);
    }
};

struct FontAnimatedTransformComponent : public Component,
                                        public SettableTooltipClient
{
    FontAnimatedTransformComponent()
    {
        setName ("Font (animated transform)");
        setTooltip ("A component that dynamically adjusts a transform to simulate a change in font height.");
    }

    void paint (Graphics& g) final
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto scale = getAnimatedValue();
        const auto size = jmin (bounds.getWidth(), bounds.getHeight()) / 5.0;
        g.addTransform (AffineTransform::scale ((float) scale));
        g.setFont (FontOptions{}.withHeight ((float) size));
        g.setColour (Colours::black);
        g.drawText (SystemStats::getJUCEVersion(), getLocalBounds(), Justification::topLeft, false);
    }
};

struct FontNoAnimationComponent : public Component,
                                  public SettableTooltipClient
{
    FontNoAnimationComponent()
    {
        setName ("Font (no animation)");
        setTooltip ("Useful for comparing with the animated font components.");
    }

    void paint (Graphics& g) final
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto size = jmin (bounds.getWidth(), bounds.getHeight()) / 5.0;
        g.setFont (FontOptions{}.withHeight ((float) size));
        g.setColour (Colours::black);
        g.drawText (SystemStats::getJUCEVersion(), getLocalBounds(), Justification::topLeft, false);
    }
};

//==============================================================================
template <bool EnableFontFallback>
struct FontFallbackComponent : public Component,
                               public SettableTooltipClient
{
    FontFallbackComponent()
    {
        if constexpr (EnableFontFallback)
        {
            setName ("Font fallback enabled");
            setTooltip ("A component that draws text with a GlyphArrangement using a font with fontFallback enabled.");
        }
        else
        {
            setName ("Font fallback disabled");
            setTooltip ("A component that draws text with a GlyphArrangement using a font with fontFallback disabled.");
        }
    }

    void paint (Graphics& g) final
    {
        g.setColour (Colours::black);

        const auto bounds = getLocalBounds().toFloat();

        GlyphArrangement arrangement;
        arrangement.addFittedText (font, text,
                                   bounds.getX(), bounds.getY(),
                                   bounds.getWidth(), bounds.getHeight(),
                                   Justification::centred,
                                   (int) (bounds.getHeight() / font.getHeight()),
                                   0.0f,
                                   {});
        arrangement.draw (g);
    }

    FontOptions font = FontOptions{}.withHeight (14.0f)
                                    .withFallbackEnabled (EnableFontFallback);

    String text { "Lorem ipsum dolor sit amet, consectetur "
                  "adipiscing elit, sed do eiusmod tempor incididunt ut labore et "
                  "dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
                  "exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                  "consequat. Duis aute irure dolor in reprehenderit in voluptate "
                  "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
                  "occaecat cupidatat non proident, sunt in culpa qui officia "
                  "deserunt mollit anim id est laborum" };
};

struct DrawFittedTextComponent : public Component,
                                 public SettableTooltipClient
{
    DrawFittedTextComponent()
    {
        setName ("Draw fitted text");
        setTooltip ("A simple component that draws text using drawFittedText() on the graphics context");
    }

    void paint (Graphics& g) final
    {
        g.setColour (Colours::black);

        auto bounds = getLocalBounds();

        g.drawFittedText (text,
                          bounds,
                          Justification::centred,
                          (int) ((float) bounds.getHeight() / font.getHeight()));
    }

    FontOptions font = FontOptions{}.withHeight (14.0f);
    String text { "Lorem ipsum dolor sit amet, consectetur "
                  "adipiscing elit, sed do eiusmod tempor incididunt ut labore et "
                  "dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
                  "exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                  "consequat. Duis aute irure dolor in reprehenderit in voluptate "
                  "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
                  "occaecat cupidatat non proident, sunt in culpa qui officia "
                  "deserunt mollit anim id est laborum" };
};

template <bool UseCache>
struct AddFittedTextComponent : public Component,
                                public SettableTooltipClient
{
    AddFittedTextComponent()
    {
        if constexpr (UseCache)
        {
            setName ("Add fitted text (cached)");
            setTooltip ("A simple component that draws text using addFittedText() with a GlyphArrangement instance stored as a member variable.");
        }
        else
        {
            setName ("Add fitted text (not cached)");
            setTooltip ("A simple component that draws text using addFittedText() with a local GlyphArrangement instance.");
        }
    }

    void paint (Graphics& g) final
    {
        g.setColour (Colours::black);

        const auto bounds = getLocalBounds().toFloat();

        if (UseCache)
        {
            cachedArrangement.draw (g);
        }
        else
        {
            GlyphArrangement arrangement;
            arrangement.addFittedText (font, text,
                                       bounds.getX(), bounds.getY(),
                                       bounds.getWidth(), bounds.getHeight(),
                                       Justification::centred,
                                       (int) (bounds.getHeight() / font.getHeight()),
                                       0.0f,
                                       {});
            arrangement.draw (g);
        }
    }

    void resized() final
    {
        const auto bounds = getLocalBounds().toFloat();
        cachedArrangement.clear();
        cachedArrangement.addFittedText (font, text,
                                         bounds.getX(), bounds.getY(),
                                         bounds.getWidth(), bounds.getHeight(),
                                         Justification::centred,
                                         (int) (bounds.getHeight() / font.getHeight()),
                                         0.0f,
                                         {});
    }

    GlyphArrangement cachedArrangement;
    FontOptions font = FontOptions{}.withHeight (14.0f);
    String text { "Lorem ipsum dolor sit amet, consectetur "
                  "adipiscing elit, sed do eiusmod tempor incididunt ut labore et "
                  "dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
                  "exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                  "consequat. Duis aute irure dolor in reprehenderit in voluptate "
                  "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
                  "occaecat cupidatat non proident, sunt in culpa qui officia "
                  "deserunt mollit anim id est laborum" };
};

//==============================================================================
struct GridComponent : public Component,
                       public SettableTooltipClient
{
    GridComponent()
    {
        setName ("Grid");
        setTooltip ("A component containing a 25 x 25 grid of child components.");

        for (auto& component : components)
            addAndMakeVisible (component);
    }

    void resized() final
    {
        Grid grid;

        using namespace juce;

        const auto track = Grid::TrackInfo (1_fr);

        for (int i = 0; i < gridSize; ++i)
        {
            grid.templateRows.add (track);
            grid.templateColumns.add (track);
        }

        for (auto& component : components)
            grid.items.add (GridItem (component));

        grid.performLayout (getLocalBounds());
    }

    struct GridItemComponent : public Component,
                               public TooltipClient
    {
        void paint (Graphics& g) final
        {
            g.setColour (colour);

            // Especially with non-integer scaling factors, small gaps may
            // appear between components. This small amount of overdraw ensures
            // these gaps are completely covered even when clipping is applied.
            g.fillRect (getLocalBounds().toFloat().expanded (0.5f));
        }

        void resized() final
        {
            const auto x = (float) getBoundsInParent().getCentreX() / (float) getParentWidth();
            const auto y = (float) getBoundsInParent().getCentreY() / (float) getParentHeight();

            colour = Colour::fromHSL (x, y, x * y * 0.75f + 0.25f, 1.0f);
            jassert (colour.isOpaque());
        }

        String getTooltip() final
        {
            if (auto* parent = dynamic_cast<TooltipClient*> (getParentComponent()))
                return parent->getTooltip();

            return {};
        }

        Colour colour;
    };

    static constexpr int gridSize = 25;
    std::array<GridItemComponent, gridSize * gridSize> components;
};


struct GridUnclippedComponent : public GridComponent
{
    GridUnclippedComponent()
    {
        setName ("Grid (unclipped)");
        setTooltip ("A component containing a 25 x 25 grid of child components, each of which has painting set to unclipped.");

        for (auto& component : components)
            component.setPaintingIsUnclipped (true);
    }
};

struct GridOpaqueComponent : public GridComponent
{
    GridOpaqueComponent()
    {
        setName ("Grid (opaque)");
        setTooltip ("A component containing a 25 x 25 grid of child components, each of which is set to opaque.\n\n"
                    "Note this component may behave differently based on which side it is on, and the other component selected.");

        for (auto& component : components)
            component.setOpaque (true);
    }
};

//==============================================================================
template <bool Opaque>
struct GridOverlayComponent : public Component,
                              public SettableTooltipClient
{
    GridOverlayComponent()
    {
        if constexpr (Opaque)
        {
            setName ("Overlay (opaque)");
            setTooltip ("A component containing a 25 x 25 grid of child components, partially covered by a simple component marked opaque.\n");
        }
        else
        {
            setName ("Overlay (non-opaque)");
            setTooltip ("A component containing a grid of 625 child components, partially covered by a simple component.\n");
        }

        overlay.setOpaque (Opaque);
        addAndMakeVisible (background);
        addAndMakeVisible (overlay);
    }

    void resized() final
    {
        background.setBounds (getLocalBounds());
        overlay.setBounds (getLocalBounds().reduced ((int) (getWidth() * 0.1),
                                                     (int) (getHeight() * 0.1)));
    }

    GridComponent background;
    CheapComponent overlay;
};

//==============================================================================
template <bool Cached>
struct ComplexPathComponent : public Component,
                              public SettableTooltipClient
{
    ComplexPathComponent()
    {
        if constexpr (Cached)
        {
            setName ("Complex path (cached)");
            setTooltip ("A component that draws a complex path created once during construction.");
            path = createComplexPath();
        }
        else
        {
            setName ("Complex path (not-cached)");
            setTooltip ("A component that draws a complex path created on each call to paint().");
        }

    }

    juce::Path createComplexPath() const
    {
        juce::Path p;

        juce::Random rng { 0 };

        const auto bounds = getBounds().toFloat();
        const auto h = bounds.getHeight();

        p.startNewSubPath (0.0f, h * 0.5f);

        for (float x = 0.0f; x < bounds.getWidth(); x += 4.0f)
        {
            const auto y = juce::jmap (rng.nextFloat(), h * 0.35f, 0.0f);
            p.lineTo (x, y);
        }

        p.lineTo (bounds.getWidth(), h * 0.5f);

        for (float x = bounds.getWidth() - 1.0f; x >= 0.0f; x -= 4.0f)
        {
            const auto y = juce::jmap (rng.nextFloat(), h * 0.65f, h);
            p.lineTo (x, y);
        }

        p.closeSubPath();

        return p;
    }

    static void drawPath (Graphics& g, const juce::Path p)
    {
        g.setColour (juce::Colours::green);
        g.fillPath (p);

        g.setColour (juce::Colours::black);
        g.strokePath (p, juce::PathStrokeType (2.0f));
    }

    void paint (Graphics& g) override
    {
        drawPath (g, Cached ? path : createComplexPath());
    }

    void resized() final
    {
        if constexpr (Cached)
            path = createComplexPath();
    }

    Path path;
};

struct ComplexPathWithRoundedCornersComponent : public ComplexPathComponent<false>
{
    ComplexPathWithRoundedCornersComponent()
    {
        setName ("Complex path (with rounded corners)");
        setTooltip ("A component that draws a complex path with rounded corners. In this example the path is recreated on each call to paint().");
    }

    void paint (Graphics& g) final
    {
        drawPath (g, createComplexPath().createPathWithRoundedCorners (std::numeric_limits<float>::max()));
    }
};

//==============================================================================
template <typename Demo>
class GraphicsDemoWrapper : public Component
{
public:
    template <typename... Args>
    GraphicsDemoWrapper (Args&&... args)
        : demo (nullptr, std::forward<Args> (args)...)
    {
        setName (String ("Graphics demo (") + demo.getName() + ")");
        addAndMakeVisible (demo);
    }

    void resized()
    {
        demo.setBounds (getLocalBounds());
    }

private:
    Demo demo;
};

//==============================================================================
struct Timings
{
    double paintMs{};
    double paintOverChildrenMs{};
    double applyEffectMs{};
    double totalMs{};
    double totalMedianMs{};
    double user0Ms{};
    double user1Ms{};
};

//==============================================================================
class ComponentPerformanceGraph : public Component
{
public:
    ~ComponentPerformanceGraph() override
    {
        if (auto it = maxValues.find (this); it != maxValues.end())
            maxValues.erase (it);
    }

    void paint (Graphics& g) override
    {
        g.setColour (Colours::black);
        g.fillRect (getLocalBounds());

        if (paintTimes.empty())
            return;

        const auto maxValue = std::invoke ([&]
        {
            double result{};

            for (auto& [graph, value] : maxValues)
            {
                ignoreUnused (graph);
                result = jmax (result, value);
            }

            return result;
        });

        const auto createGraphLine = [&] (const auto& data, bool closePath)
        {
            Path path;

            bool startPath = true;
            const auto numDataPointsToDraw = jmin ((int) data.size(), getWidth());
            const auto height = (double) getHeight();

            for (auto i = 0; i < numDataPointsToDraw; ++i)
            {
                const auto index = ((int) data.size()) - i - 1;
                const auto x = getWidth() - i;
                const auto y = jmap (data[(size_t) index], 0.0, maxValue, height, 0.0);
                const Point<int> pos { x, (int) y };

                if (startPath)
                    path.startNewSubPath (pos.toFloat());
                else
                    path.lineTo (pos.toFloat());

                startPath = false;

                if (closePath && i == (numDataPointsToDraw - 1))
                {
                    const auto h = (float) getHeight();
                    path.lineTo ({ (float) x, h });
                    path.lineTo ({ (float) getWidth(), h });
                    path.closeSubPath();
                }
            }

            return path;
        };

        g.setColour (Colours::red);
        g.fillPath (createGraphLine (paintComponentTimes, true));

        g.setColour (Colours::blue);
        g.fillPath (createGraphLine (applyEffectTimes, true));

        g.setColour (Colours::darkorange);
        g.fillPath (createGraphLine (paintOverChildrenTimes, true));

        g.setColour (Colours::yellow);
        g.fillPath (createGraphLine (paintTimes, true));

        g.setColour (Colours::magenta);
        g.fillPath (createGraphLine (user1Times, true));

        g.setColour (Colours::cyan);
        g.fillPath (createGraphLine (user0Times, true));

        g.setColour (Colours::green);
        g.strokePath (createGraphLine (paintComponentMedianTimes, false),
                                       PathStrokeType (4.0f, PathStrokeType::JointStyle::curved));
    }

    void addTimings (const Timings& t)
    {
        while ((int) paintComponentMedianTimes.size() >= getWidth())
        {
            paintComponentMedianSum -= paintComponentMedianTimes.front();
            paintComponentMedianTimes.pop_front();
        }

        std::deque<double>* containers[] { &paintTimes,
                                           &paintOverChildrenTimes,
                                           &applyEffectTimes,
                                           &user0Times,
                                           &user1Times,
                                           &paintComponentTimes };

        for (auto* container : containers)
            while ((int) container->size() >= getWidth())
                container->pop_front();

                    paintTimes.push_back (t.paintMs);
        paintOverChildrenTimes.push_back (t.paintMs + t.paintOverChildrenMs);
              applyEffectTimes.push_back (t.paintMs + t.paintOverChildrenMs + t.applyEffectMs);

        user0Times.push_back (t.user0Ms);
        user1Times.push_back (t.user0Ms + t.user1Ms);

        paintComponentTimes.push_back (t.totalMs);
        paintComponentMedianTimes.push_back (t.totalMedianMs);
        paintComponentMedianSum += t.totalMedianMs;

        const auto average = paintComponentMedianSum / (double) paintComponentMedianTimes.size();
        maxValues[this] = jmax (average * 2.0, 0.01);
    }

    void clearValues()
    {
        paintTimes.clear();
        paintOverChildrenTimes.clear();
        applyEffectTimes.clear();
        paintComponentTimes.clear();
        paintComponentMedianTimes.clear();
        user0Times.clear();
        user1Times.clear();
        paintComponentMedianSum = 0.0;
    }

private:
    inline static std::map<ComponentPerformanceGraph*, double> maxValues{};
    std::deque<double> paintTimes;
    std::deque<double> paintOverChildrenTimes;
    std::deque<double> applyEffectTimes;
    std::deque<double> user0Times;
    std::deque<double> user1Times;
    std::deque<double> paintComponentTimes;
    std::deque<double> paintComponentMedianTimes;
    double paintComponentMedianSum{};
    VBlankAttachment vblank { this, [&] { repaint(); } };
};

class ComponentPerformanceLegend : public Component
{
public:
    void drawSection (Graphics& g, Rectangle<int> bounds, const Colour& colour, const String& name, double ms)
    {
        const auto swatchBounds = bounds.removeFromLeft (30)
                                        .withSizeKeepingCentre (10, 10);

        g.setColour (colour);
        g.fillRect (swatchBounds);

        g.setColour (Colours::black);
        g.drawRect (swatchBounds);

        bounds.removeFromRight (5);

        g.setFont (FontOptions { 14.0f });
        g.drawFittedText (name, bounds, Justification::centredLeft, 1);

        g.setFont (FontOptions { 14.0f });
        g.drawFittedText (String { ms, 3 } + " ms", bounds, Justification::centredRight, 1);
    }

    void paint (Graphics& g)
    {
        auto bounds = getLocalBounds();
        g.setColour (Colours::white.withAlpha (0.93f));
        g.fillRoundedRectangle (bounds.toFloat(), 5.0f);

        g.setColour (Colours::black);
        g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f), 5.0f, 1.0f);

        const auto numSections = 7;
        auto sectionHeight = bounds.getHeight() / numSections;
        drawSection (g, bounds.removeFromTop (sectionHeight), Colours::red, "total", timings.totalMs);
        drawSection (g, bounds.removeFromTop (sectionHeight), Colours::green, "total (median)", timings.totalMedianMs);
        drawSection (g, bounds.removeFromTop (sectionHeight), Colours::blue, "applyEffect", timings.applyEffectMs);
        drawSection (g, bounds.removeFromTop (sectionHeight), Colours::darkorange, "paintOverChildren", timings.paintOverChildrenMs);
        drawSection (g, bounds.removeFromTop (sectionHeight), Colours::yellow, "paint", timings.paintMs);
        drawSection (g, bounds.removeFromTop (sectionHeight), Colours::magenta, "user1", timings.user1Ms);
        drawSection (g, bounds.removeFromTop (sectionHeight), Colours::cyan, "user0", timings.user0Ms);
    }

    void setTimings (Timings newTimings)
    {
        timings = std::move (newTimings);
    }

private:
    Timings timings{};
};

//==============================================================================
class ComponentProfiler : public Component,
                          private ComponentListener,
                          private ComboBox::Listener
{
public:
    ComponentProfiler()
    {
        for (auto [itemId, component] : enumerate (components, int { 1 }))
        {
            jassert (component->getName().isNotEmpty());
            comboBox.addItem (String { itemId } + ": " + component->getName(), itemId);
        }

        addAndMakeVisible (comboBox);
        addAndMakeVisible (graph);
        addAndMakeVisible (legend);

        comboBox.addListener (this);
        comboBox.setSelectedItemIndex (0);
    }

    ~ComponentProfiler() override
    {
        stopListeningToAllComponents();
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        comboBox.setBounds (bounds.removeFromTop (50));
        graph.setBounds (bounds.removeFromBottom (150));

        if (currentComponent != nullptr)
            currentComponent->setBounds (bounds);

        legend.setBounds (bounds.removeFromRight (250)
                                .removeFromBottom (150).reduced (5));
    }

private:
    void stopListeningToAllComponents()
    {
        for (auto* c : componentsBeingListenedTo)
            c->removeComponentListener (this);

        componentsBeingListenedTo.clear();
    }

    void startListeningToComponentAndDescendents (Component* c)
    {
        c->addComponentListener (this);
        componentsBeingListenedTo.add (c);

        for (auto* child : c->getChildren())
            startListeningToComponentAndDescendents (child);
    }

    void comboBoxChanged (ComboBox*) override
    {
        if (currentComponent != nullptr)
        {
            stopListeningToAllComponents();
            removeChildComponent (currentComponent);
            currentComponent = nullptr;
        }

        const auto index = comboBox.getSelectedItemIndex();

        if (index < 0)
            return;

        currentComponent = components[(size_t) index].get();
        addAndMakeVisible (currentComponent);
        legend.toFront (false);

        startListeningToComponentAndDescendents (currentComponent);

        resized();
        graph.clearValues();
        totalMsTimes.fill (0.0);
    }

    void componentPainted (Component& c, const ComponentPaintDiagnostics& d) override
    {
        timings.paintMs += d.paintDuration.get<Milliseconds>();
        timings.paintOverChildrenMs += d.paintOverChildrenDuration.get<Milliseconds>();
        timings.applyEffectMs += d.applyEffectDuration.get<Milliseconds>();

        if (&c != currentComponent)
            return;

        timings.totalMs = d.totalPaintDuration.get<Milliseconds>();

        ++totalMsPos;
        totalMsPos %= (int) totalMsTimes.size();
        totalMsTimes[(size_t) totalMsPos] = timings.totalMs;

        timings.totalMedianMs = std::invoke ([&]
                                             {
                                                 auto tmp = totalMsTimes;
                                                 const auto mid = (std::ptrdiff_t) (tmp.size() / 2);
                                                 std::nth_element (tmp.begin(), tmp.begin() + mid, tmp.end());
                                                 return tmp[mid];
                                             });

        graph.addTimings (timings);
        legend.setTimings (timings);
        timings = Timings{};
    }

    void componentBeingDeleted (Component& c) override
    {
        componentsBeingListenedTo.removeFirstMatchingValue (&c);
    }

    void componentChildrenChanged (Component&) override
    {
        stopListeningToAllComponents();
        startListeningToComponentAndDescendents (currentComponent);
    }

    UserDiagnostics::Callback getUserDiagnosticsCallback()
    {
        return [this] (const UserDiagnostics& d)
        {
            timings.user0Ms += d.user0.get<Milliseconds>();
            timings.user1Ms += d.user1.get<Milliseconds>();
        };
    }

    std::vector<std::shared_ptr<Component>> components
    {
        std::make_shared<CheapComponent>(),
        std::make_shared<CheapUnclippedComponent>(),
        std::make_shared<CheapOpaqueComponent>(),
        std::make_shared<CheapUnclippedOpaqueComponent>(),
        std::make_shared<CheapBufferedToImageComponent>(),
        std::make_shared<CheapUserDiagnosticComponent> (getUserDiagnosticsCallback()),
        std::make_shared<AnimatedComponent>(),
        std::make_shared<PaintOverChildrenComponent>(),
        std::make_shared<PixelateEffectComponent>(),
        std::make_shared<ExpensiveComponent> (getUserDiagnosticsCallback()),
        std::make_shared<ExpensiveBufferedToImageComponent> (getUserDiagnosticsCallback()),
        std::make_shared<ExpensiveBufferedToImageWithChildComponent> (getUserDiagnosticsCallback()),
        std::make_shared<ExpensiveBufferedToImageWithSiblingComponent> (getUserDiagnosticsCallback()),
        std::make_shared<GridComponent>(),
        std::make_shared<GridUnclippedComponent>(),
        std::make_shared<GridOpaqueComponent>(),
        std::make_shared<GridOverlayComponent<false>>(),
        std::make_shared<GridOverlayComponent<true>>(),
        std::make_shared<FontAnimatedHeightComponent>(),
        std::make_shared<FontAnimatedTransformComponent>(),
        std::make_shared<FontNoAnimationComponent>(),
        std::make_shared<DrawFittedTextComponent>(),
        std::make_shared<AddFittedTextComponent<false>>(),
        std::make_shared<AddFittedTextComponent<true>>(),
        std::make_shared<FontFallbackComponent<false>>(),
        std::make_shared<FontFallbackComponent<true>>(),
        std::make_shared<ComplexPathComponent<false>>(),
        std::make_shared<ComplexPathComponent<true>>(),
        std::make_shared<ComplexPathWithRoundedCornersComponent>(),
       #if JUCE_DEMO_RUNNER
        std::make_shared<AnimationAppDemo>(),
        std::make_shared<AnimationEasingDemo>(),
       #if ! JUCE_ANDROID
        std::make_shared<CodeEditorDemo>(),
       #endif
        std::make_shared<ComponentDemo>(),
        std::make_shared<ComponentTransformsDemo>(),
        std::make_shared<FontsDemo>(),
        std::make_shared<FontFeaturesDemo>(),
        std::make_shared<GraphicsDemo>(),
        std::make_shared<GraphicsDemoWrapper<PathsDemo>> (false, true),
        std::make_shared<GraphicsDemoWrapper<PathsDemo>> (true,  false),
        std::make_shared<GraphicsDemoWrapper<PathsDemo>> (false, false),
        std::make_shared<GraphicsDemoWrapper<RectangleFillTypesDemo>>(),
        std::make_shared<GraphicsDemoWrapper<StrokesDemo>>(),
        std::make_shared<GraphicsDemoWrapper<ImagesRenderingDemo>> (false, false),
        std::make_shared<GraphicsDemoWrapper<ImagesRenderingDemo>> (false, true),
        std::make_shared<GraphicsDemoWrapper<ImagesRenderingDemo>> (true,  false),
        std::make_shared<GraphicsDemoWrapper<ImagesRenderingDemo>> (true,  true),
        std::make_shared<GraphicsDemoWrapper<BlurDemo>>(),
        std::make_shared<GraphicsDemoWrapper<GlyphsDemo>>(),
        std::make_shared<GraphicsDemoWrapper<SVGDemo>>(),
        std::make_shared<GraphicsDemoWrapper<LinesDemo>>(),
        std::make_shared<GraphicsDemoWrapper<ShapesDemo>>(),
        std::make_shared<LookAndFeelDemo>(),
        std::make_shared<WebBrowserDemo>(),
        std::make_shared<WidgetsDemo>()
       #endif
    };

    ComboBox comboBox;
    Component* currentComponent = nullptr;
    Array<Component*> componentsBeingListenedTo;
    ComponentPerformanceGraph graph;
    ComponentPerformanceLegend legend;
    Timings timings;
    std::array<double, 60> totalMsTimes;
    int totalMsPos{};
};

//==============================================================================
class ComponentDiagnosticsDemo : public Component
{
public:
    ComponentDiagnosticsDemo()
    {
        addAndMakeVisible (profilerA);
        addAndMakeVisible (profilerB);

        setSize (800, 600);
    }

    void resized() final
    {
        auto bounds = getLocalBounds();

        if (getWidth() >= getHeight())
        {
            const auto width = bounds.getWidth() / 2;
            profilerB.setVisible (true);

            profilerA.setBounds (bounds.removeFromLeft (width));
            profilerB.setBounds (bounds);
        }
        else
        {
            profilerB.setVisible (false);
            profilerA.setBounds (bounds);
        }
    }

    void paint (Graphics& g) final
    {
        g.fillCheckerBoard (getLocalBounds().toFloat(), 48.0f, 48.0f,
                            Colours::lightgrey, Colours::white);
    }

private:
    SharedResourcePointer<TooltipWindow> tooltipWindow;
    ComponentProfiler profilerA;
    ComponentProfiler profilerB;

    // We repaint everything rather than just the components being profiled,
    // this helps because the first component to be painted in the tree of all
    // components being painted may take measurably longer than other components
    // which leads to inconsistent comparisons even when comparing the same
    // component in both profilers
    VBlankAttachment vblank { this, [&] { repaint(); } };
};
