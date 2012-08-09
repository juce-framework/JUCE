/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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
String hexString8Digits (int value);

String createAlphaNumericUID();
String createGUID (const String& seed); // Turns a seed into a windows GUID

String escapeSpaces (const String& text); // replaces spaces with blackslash-space
String addQuotesIfContainsSpaces (const String& text);

StringPairArray parsePreprocessorDefs (const String& defs);
StringPairArray mergePreprocessorDefs (StringPairArray inheritedDefs, const StringPairArray& overridingDefs);
String createGCCPreprocessorFlags (const StringPairArray& defs);
String replacePreprocessorDefs (const StringPairArray& definitions, String sourceString);

StringArray getSearchPathsFromString (const String& searchPath);

void setValueIfVoid (Value value, const var& defaultValue);

//==============================================================================
int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex);

void autoScrollForMouseEvent (const MouseEvent& e, bool scrollX = true, bool scrollY = true);

void showUTF8ToolWindow (ScopedPointer<Component>& ownerPointer);

bool cancelAnyModalComponents();
bool reinvokeCommandAfterCancellingModalComps (const ApplicationCommandTarget::InvocationInfo&);

//==============================================================================
struct Icon
{
    Icon() : path (nullptr) {}
    Icon (const Path& p, const Colour& c)  : path (&p), colour (c) {}
    Icon (const Path* p, const Colour& c)  : path (p),  colour (c) {}

    void draw (Graphics& g, const Rectangle<float>& area) const
    {
        if (path != nullptr)
        {
            g.setColour (colour);

            const RectanglePlacement placement (RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize);
            g.fillPath (*path, placement.getTransformToFit (path->getBounds(), area));
        }
    }

    Icon withContrastingColourTo (const Colour& background) const
    {
        return Icon (path, background.contrasting (colour, 0.6f));
    }

    const Path* path;
    Colour colour;
};


//==============================================================================
class RolloverHelpComp   : public Component,
                           private Timer
{
public:
    RolloverHelpComp();

    void paint (Graphics& g);
    void timerCallback();

private:
    Component* lastComp;
    String lastTip;

    static String findTip (Component*);
};

//==============================================================================
class PropertyListBuilder
{
public:
    PropertyListBuilder() {}

    void add (PropertyComponent* propertyComp)
    {
        components.add (propertyComp);
    }

    void add (PropertyComponent* propertyComp, const String& tooltip)
    {
        propertyComp->setTooltip (tooltip);
        add (propertyComp);
    }

    void addSearchPathProperty (const Value& value, const String& name, const String& mainHelpText)
    {
        add (new TextPropertyComponent (value, name, 16384, true),
             mainHelpText + " Use semi-colons or new-lines to separate multiple paths.");
    }

    void setPreferredHeight (int height)
    {
        for (int j = components.size(); --j >= 0;)
            components.getUnchecked(j)->setPreferredHeight (height);
    }

    Array <PropertyComponent*> components;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyListBuilder);
};

//==============================================================================
class FloatingLabelComponent    : public Component
{
public:
    FloatingLabelComponent();

    void remove();
    void update (Component* parent, const String& text, const Colour& textColour,
                 int x, int y, bool toRight, bool below);

    void paint (Graphics& g);

private:
    Font font;
    Colour colour;
    GlyphArrangement glyphs;
};

//==============================================================================
// A ValueSource which takes an input source, and forwards any changes in it.
// This class is a handy way to create sources which re-map a value.
class ValueSourceFilter   : public Value::ValueSource,
                            public Value::Listener
{
public:
    ValueSourceFilter (const Value& source)  : sourceValue (source)
    {
        sourceValue.addListener (this);
    }

    void valueChanged (Value&)      { sendChangeMessage (true); }

protected:
    Value sourceValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueSourceFilter);
};

//==============================================================================
class FloatingToolWindow  : public DialogWindow
{
public:
    FloatingToolWindow (const String& title,
                        const String& windowPosPropertyName,
                        Component* content,
                        ScopedPointer<Component>& ownerPointer,
                        int defaultW, int defaultH,
                        int minW, int minH,
                        int maxW, int maxH)
        : DialogWindow (title, Colours::darkgrey, true, true),
          windowPosProperty (windowPosPropertyName),
          owner (ownerPointer)
    {
        setUsingNativeTitleBar (true);
        setResizable (true, true);
        setResizeLimits (minW, minH, maxW, maxH);
        setContentOwned (content, false);

        const String windowState (getGlobalProperties().getValue (windowPosProperty));

        if (windowState.isNotEmpty())
            restoreWindowStateFromString (windowState);
        else
            centreAroundComponent (Component::getCurrentlyFocusedComponent(), defaultW, defaultH);

        setVisible (true);
        owner = this;
    }

    ~FloatingToolWindow()
    {
        getGlobalProperties().setValue (windowPosProperty, getWindowStateAsString());
    }

    void closeButtonPressed()
    {
        owner = nullptr;
    }

private:
    String windowPosProperty;
    ScopedPointer<Component>& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FloatingToolWindow);
};

//==============================================================================
class PopupColourSelector   : public Component,
                              public ChangeListener,
                              public Value::Listener,
                              public ButtonListener
{
public:
    PopupColourSelector (const Value& colourValue_,
                         const Colour& defaultColour_,
                         const bool canResetToDefault)
        : defaultButton ("Reset to Default"),
          colourValue (colourValue_),
          defaultColour (defaultColour_)
    {
        addAndMakeVisible (&selector);
        selector.setName ("Colour");
        selector.setCurrentColour (getColour());
        selector.addChangeListener (this);

        if (canResetToDefault)
        {
            addAndMakeVisible (&defaultButton);
            defaultButton.addListener (this);
        }

        colourValue.addListener (this);
        setSize (300, 400);
    }

    void resized()
    {
        if (defaultButton.isVisible())
        {
            selector.setBounds (0, 0, getWidth(), getHeight() - 30);
            defaultButton.changeWidthToFitText (22);
            defaultButton.setTopLeftPosition (10, getHeight() - 26);
        }
        else
        {
            selector.setBounds (0, 0, getWidth(), getHeight());
        }
    }

    Colour getColour() const
    {
        if (colourValue.toString().isEmpty())
            return defaultColour;

        return Colour::fromString (colourValue.toString());
    }

    void setColour (const Colour& newColour)
    {
        if (getColour() != newColour)
        {
            if (newColour == defaultColour && defaultButton.isVisible())
                colourValue = var::null;
            else
                colourValue = newColour.toDisplayString (true);
        }
    }

    void buttonClicked (Button*)
    {
        setColour (defaultColour);
        selector.setCurrentColour (defaultColour);
    }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        if (selector.getCurrentColour() != getColour())
            setColour (selector.getCurrentColour());
    }

    void valueChanged (Value&)
    {
        selector.setCurrentColour (getColour());
    }

private:
    StoredSettings::ColourSelectorWithSwatches selector;
    TextButton defaultButton;
    Value colourValue;
    Colour defaultColour;
};

//==============================================================================
/**
    A component that shows a colour swatch with hex ARGB value, and which pops up
    a colour selector when you click it.
*/
class ColourEditorComponent    : public Component,
                                 public Value::Listener
{
public:
    ColourEditorComponent (UndoManager* undoManager_, const Value& colourValue_,
                           const Colour& defaultColour_, const bool canResetToDefault_)
        : undoManager (undoManager_), colourValue (colourValue_), defaultColour (defaultColour_),
          canResetToDefault (canResetToDefault_)
    {
        colourValue.addListener (this);
    }

    void paint (Graphics& g)
    {
        const Colour colour (getColour());

        g.fillAll (Colours::grey);
        g.fillCheckerBoard (getLocalBounds().reduced (2),
                            10, 10,
                            Colour (0xffdddddd).overlaidWith (colour),
                            Colour (0xffffffff).overlaidWith (colour));

        g.setColour (Colours::white.overlaidWith (colour).contrasting());
        g.setFont (Font (getHeight() * 0.6f, Font::bold));
        g.drawFittedText (colour.toDisplayString (true), getLocalBounds().reduced (2, 1),
                          Justification::centred, 1);
    }

    Colour getColour() const
    {
        if (colourValue.toString().isEmpty())
            return defaultColour;

        return Colour::fromString (colourValue.toString());
    }

    void setColour (const Colour& newColour)
    {
        if (getColour() != newColour)
        {
            if (newColour == defaultColour && canResetToDefault)
                colourValue = var::null;
            else
                colourValue = newColour.toDisplayString (true);
        }
    }

    void resetToDefault()
    {
        setColour (defaultColour);
    }

    void refresh()
    {
        const Colour col (getColour());

        if (col != lastColour)
        {
            lastColour = col;
            repaint();
        }
    }

    void mouseDown (const MouseEvent&)
    {
        if (undoManager != nullptr)
            undoManager->beginNewTransaction();

        CallOutBox::launchAsynchronously (*this, new PopupColourSelector (colourValue,
                                                                          defaultColour,
                                                                          canResetToDefault), nullptr);
    }

    void valueChanged (Value&)
    {
        refresh();
    }

private:
    UndoManager* undoManager;
    Value colourValue;
    Colour lastColour;
    const Colour defaultColour;
    const bool canResetToDefault;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourEditorComponent);
};

//==============================================================================
class ColourPropertyComponent  : public PropertyComponent
{
public:
    ColourPropertyComponent (UndoManager* undoManager, const String& name, const Value& colour,
                             const Colour& defaultColour, bool canResetToDefault)
        : PropertyComponent (name),
          colourEditor (undoManager, colour, defaultColour, canResetToDefault)
    {
        addAndMakeVisible (&colourEditor);
    }

    void resized()
    {
        colourEditor.setBounds (getLookAndFeel().getPropertyComponentContentPosition (*this));
    }

    void refresh() {}

protected:
    ColourEditorComponent colourEditor;
};
