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
StringArray getCommaOrWhitespaceSeparatedItems (const String&);

void setValueIfVoid (Value value, const var& defaultValue);

void addPlistDictionaryKey (XmlElement* xml, const String& key, const String& value);
void addPlistDictionaryKeyBool (XmlElement* xml, const String& key, bool value);
void addPlistDictionaryKeyInt (XmlElement* xml, const String& key, int value);

//==============================================================================
int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex);

void autoScrollForMouseEvent (const MouseEvent& e, bool scrollX = true, bool scrollY = true);

void showUTF8ToolWindow (ScopedPointer<Component>& ownerPointer);
void showSVGPathDataToolWindow (ScopedPointer<Component>& ownerPointer);

bool cancelAnyModalComponents();
bool reinvokeCommandAfterCancellingModalComps (const ApplicationCommandTarget::InvocationInfo&);

StringArray getCleanedStringArray (StringArray);

//==============================================================================
class RolloverHelpComp   : public Component,
                           private Timer
{
public:
    RolloverHelpComp();

    void paint (Graphics&) override;
    void timerCallback() override;

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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyListBuilder)
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

    void valueChanged (Value&) override      { sendChangeMessage (true); }

protected:
    Value sourceValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueSourceFilter)
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

    void closeButtonPressed() override
    {
        owner = nullptr;
    }

private:
    String windowPosProperty;
    ScopedPointer<Component>& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FloatingToolWindow)
};

//==============================================================================
class PopupColourSelector   : public Component,
                              public ChangeListener,
                              public Value::Listener,
                              public ButtonListener
{
public:
    PopupColourSelector (const Value& colour,
                         Colour defaultCol,
                         const bool canResetToDefault)
        : defaultButton ("Reset to Default"),
          colourValue (colour),
          defaultColour (defaultCol)
    {
        addAndMakeVisible (selector);
        selector.setName ("Colour");
        selector.setCurrentColour (getColour());
        selector.addChangeListener (this);

        if (canResetToDefault)
        {
            addAndMakeVisible (defaultButton);
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
            selector.setBounds (getLocalBounds());
        }
    }

    Colour getColour() const
    {
        if (colourValue.toString().isEmpty())
            return defaultColour;

        return Colour::fromString (colourValue.toString());
    }

    void setColour (Colour newColour)
    {
        if (getColour() != newColour)
        {
            if (newColour == defaultColour && defaultButton.isVisible())
                colourValue = var();
            else
                colourValue = newColour.toDisplayString (true);
        }
    }

    void buttonClicked (Button*) override
    {
        setColour (defaultColour);
        selector.setCurrentColour (defaultColour);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        if (selector.getCurrentColour() != getColour())
            setColour (selector.getCurrentColour());
    }

    void valueChanged (Value&) override
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
    ColourEditorComponent (UndoManager* um, const Value& colour,
                           Colour defaultCol, const bool canReset)
        : undoManager (um), colourValue (colour), defaultColour (defaultCol),
          canResetToDefault (canReset)
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

    void setColour (Colour newColour)
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

    void mouseDown (const MouseEvent&) override
    {
        if (undoManager != nullptr)
            undoManager->beginNewTransaction();

        CallOutBox::launchAsynchronously (new PopupColourSelector (colourValue,
                                                                   defaultColour,
                                                                   canResetToDefault),
                                          getScreenBounds(), nullptr);
    }

    void valueChanged (Value&) override
    {
        refresh();
    }

private:
    UndoManager* undoManager;
    Value colourValue;
    Colour lastColour;
    const Colour defaultColour;
    const bool canResetToDefault;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourEditorComponent)
};

//==============================================================================
class ColourPropertyComponent  : public PropertyComponent
{
public:
    ColourPropertyComponent (UndoManager* undoManager, const String& name, const Value& colour,
                             Colour defaultColour, bool canResetToDefault)
        : PropertyComponent (name),
          colourEditor (undoManager, colour, defaultColour, canResetToDefault)
    {
        addAndMakeVisible (colourEditor);
    }

    void resized() override
    {
        colourEditor.setBounds (getLookAndFeel().getPropertyComponentContentPosition (*this));
    }

    void refresh() override {}

protected:
    ColourEditorComponent colourEditor;
};

//==============================================================================
class FilePathPropertyComponent :    public PropertyComponent
{
public:
    /** A Property Component for selecting files or folders.

        The user may drag files over the property box, enter the path
        manually and/or click the '...' button to open a file selection
        dialog box
    */
    FilePathPropertyComponent (Value valueToControl,
                               const String& propertyDescription,
                               bool isDirectory,
                               const String& wildcards = "*",
                               const File& rootToUseForRelativePaths = File::nonexistent)
        : PropertyComponent (propertyDescription),
          innerComp (valueToControl, isDirectory, wildcards, rootToUseForRelativePaths)
    {
        addAndMakeVisible (innerComp);
    }

    void refresh() override {} // N/A

private:
    struct InnerComponent   : public Component,
                              public FileDragAndDropTarget,
                              private Button::Listener
    {
        InnerComponent (Value v, bool isDir, const String& wc, const File& rt)
            : value (v),
              isDirectory (isDir),
              highlightForDragAndDrop (false),
              wildcards (wc),
              root (rt),
              button ("...")
        {
            addAndMakeVisible (textbox);
            textbox.getTextValue().referTo (value);

            addAndMakeVisible (button);
            button.addListener (this);
        }

        void paintOverChildren (Graphics& g) override
        {
            if (highlightForDragAndDrop)
            {
                g.setColour (Colours::green.withAlpha (0.1f));
                g.fillRect (getLocalBounds());
            }
        }

        void resized() override
        {
            Rectangle<int> r (getLocalBounds());

            button.setBounds (r.removeFromRight (24));
            textbox.setBounds (r);
        }

        bool isInterestedInFileDrag (const StringArray&) override   { return true; }
        void fileDragEnter (const StringArray&, int, int) override  { highlightForDragAndDrop = true;  repaint(); }
        void fileDragExit (const StringArray&) override             { highlightForDragAndDrop = false; repaint(); }

        void filesDropped (const StringArray& files, int, int) override
        {
            const File firstFile (files[0]);

            if (isDirectory)
                setTo (firstFile.isDirectory() ? firstFile
                                               : firstFile.getParentDirectory());
            else
                setTo (firstFile);
        }

        void buttonClicked (Button*) override
        {
            const File currentFile (root.getChildFile (value.toString()));

            if (isDirectory)
            {
                FileChooser chooser ("Select directory", currentFile);

                if (chooser.browseForDirectory())
                    setTo (chooser.getResult());
            }
            else
            {
                FileChooser chooser ("Select file", currentFile, wildcards);

                if (chooser.browseForFileToOpen())
                    setTo (chooser.getResult());
            }
        }

        void setTo (const File& f)
        {
            value = (root == File::nonexistent) ? f.getFullPathName()
                                                : f.getRelativePathFrom (root);
        }

        Value value;
        bool isDirectory, highlightForDragAndDrop;
        String wildcards;
        File root;
        TextEditor textbox;
        TextButton button;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InnerComponent)
    };

    InnerComponent innerComp;  // Used so that the PropertyComponent auto first-child positioning works

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilePathPropertyComponent)
};
