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


//==============================================================================
// String::hashCode64 actually hit some dupes, so this is a more powerful version.
const int64 hashCode64 (const String& s);
const String randomHexString (Random& random, int numChars);
const String hexString8Digits (int value);

const String createAlphaNumericUID();
const String createGUID (const String& seed); // Turns a seed into a windows GUID

const StringPairArray parsePreprocessorDefs (const String& defs);
const StringPairArray mergePreprocessorDefs (StringPairArray inheritedDefs, const StringPairArray& overridingDefs);
const String replacePreprocessorDefs (const StringPairArray& definitions, String sourceString);

//==============================================================================
int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex);

void autoScrollForMouseEvent (const MouseEvent& e, bool scrollX = true, bool scrollY = true);

void drawComponentPlaceholder (Graphics& g, int w, int h, const String& text);
void drawRecessedShadows (Graphics& g, int w, int h, int shadowSize);


//==============================================================================
class PropertyPanelWithTooltips  : public Component,
                                   public Timer
{
public:
    PropertyPanelWithTooltips();
    ~PropertyPanelWithTooltips();

    PropertyPanel& getPanel() throw()        { return panel; }

    void paint (Graphics& g);
    void resized();
    void timerCallback();

private:
    PropertyPanel panel;
    TextLayout layout;
    Component* lastComp;
    String lastTip;

    const String findTip (Component* c);
};

//==============================================================================
class FloatingLabelComponent    : public Component
{
public:
    FloatingLabelComponent();

    void remove();
    void update (Component* parent, const String& text, const Colour& textColour, int x, int y, bool toRight, bool below);
    void paint (Graphics& g);

private:
    Font font;
    Colour colour;
    GlyphArrangement glyphs;
};

//==============================================================================
class JucerToolbarButton   : public ToolbarItemComponent
{
public:
    //==============================================================================
    JucerToolbarButton (int itemId_, const String& labelText)
        : ToolbarItemComponent (itemId_, labelText, true)
    {
        setClickingTogglesState (false);
    }

    ~JucerToolbarButton()
    {
    }

    //==============================================================================
    bool getToolbarItemSizes (int toolbarDepth, bool isToolbarVertical, int& preferredSize, int& minSize, int& maxSize)
    {
        preferredSize = minSize = maxSize = 50;
        return true;
    }

    void paintButton (Graphics& g, bool over, bool down)
    {
        Path p;
        p.addRoundedRectangle (1.5f, 2.5f, getWidth() - 3.0f, getHeight() - 5.0f, 3.0f);

        if (getToggleState())
        {
            g.setColour (Colours::grey.withAlpha (0.5f));
            g.fillPath (p);
        }

        g.setColour (Colours::darkgrey.withAlpha (0.3f));
        g.strokePath (p, PathStrokeType (1.0f));

        g.setFont (11.0f);
        g.setColour (Colours::black.withAlpha ((over || down) ? 1.0f : 0.7f));
        g.drawFittedText (getButtonText(), 2, 2, getWidth() - 4, getHeight() - 4, Justification::centred, 2);
    }

    void paintButtonArea (Graphics& g, int width, int height, bool isMouseOver, bool isMouseDown)
    {
    }

    void contentAreaChanged (const Rectangle<int>& newBounds)
    {
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucerToolbarButton);
};


//==============================================================================
class DrawableComponent    : public Component,
                             public ValueTree::Listener
{
public:
    DrawableComponent (const ValueTree& drawable_)
    {
        setDrawable (drawable_);
    }

    ~DrawableComponent()
    {
    }

    void setDrawable (const ValueTree& newDrawable)
    {
        drawable.removeListener (this);
        drawable = newDrawable;
        drawable.addListener (this);
        drawableObject = Drawable::createFromValueTree (drawable, 0); // xxx image provider missing
        addAndMakeVisible (drawableObject);
        resized();
        repaint();
    }

    void resized()
    {
/*        DrawableComposite* dc = dynamic_cast <DrawableComposite*> (static_cast <Drawable*> (drawableObject));

        if (dc != 0)
        {
            const RelativeCoordinate origin, right (getWidth()), bottom (getHeight());

            dc->setContentArea (RelativeRectangle (origin, right, origin, bottom));
            //dc->resetBoundingBoxToContentArea();
        }*/
    }

    void valueTreePropertyChanged (ValueTree&, const Identifier&)       { updateGraphics(); }
    void valueTreeChildrenChanged (ValueTree&)                          { updateGraphics(); }
    void valueTreeParentChanged (ValueTree&)                            { updateGraphics(); }

private:
    ValueTree drawable;
    ScopedPointer<Drawable> drawableObject;

    void updateGraphics()
    {
        if (drawableObject != 0)
            drawableObject->refreshFromValueTree (drawable, 0);
    }
};


//==============================================================================
/**
*/
class RelativeRectangleLayoutManager    : public ComponentListener,
                                          public Expression::EvaluationContext,
                                          public AsyncUpdater
{
public:
    //==============================================================================
    /**
    */
    RelativeRectangleLayoutManager (Component* parentComponent);

    /** Destructor. */
    ~RelativeRectangleLayoutManager();

    //==============================================================================
    /**
    */
    void setMarker (const String& name, const RelativeCoordinate& coord);

    /**
    */
    void setComponentBounds (Component* component, const String& componentName, const RelativeRectangle& bounds);

    /**
    */
    void applyLayout();

    //==============================================================================
    /** @internal */
    const Expression getSymbolValue (const String& symbol, const String& member) const;
    /** @internal */
    void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized);
    /** @internal */
    void componentBeingDeleted (Component& component);
    /** @internal */
    void handleAsyncUpdate();

private:
    //==============================================================================
    struct ComponentPosition
    {
        ComponentPosition (Component* component, const String& name, const RelativeRectangle& coords);

        Component* component;
        String name;
        RelativeRectangle coords;
    };

    struct MarkerPosition
    {
        MarkerPosition (const String& name, const RelativeCoordinate& coord);

        String markerName;
        RelativeCoordinate position;
    };

    Component* parent;
    OwnedArray <ComponentPosition> components;
    OwnedArray <MarkerPosition> markers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativeRectangleLayoutManager);
};
