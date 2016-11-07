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


static void showBubbleMessage (Component* targetComponent, const String& textToShow,
                               ScopedPointer<BubbleMessageComponent>& bmc)
{
    bmc = new BubbleMessageComponent();

    if (Desktop::canUseSemiTransparentWindows())
    {
        bmc->setAlwaysOnTop (true);
        bmc->addToDesktop (0);
    }
    else
    {
        targetComponent->getTopLevelComponent()->addChildComponent (bmc);
    }

    AttributedString text (textToShow);
    text.setJustification (Justification::centred);

    bmc->showAt (targetComponent, text, 2000, true, false);
}

//==============================================================================
/** To demonstrate how sliders can have custom snapping applied to their values,
    this simple class snaps the value to 50 if it comes near.
*/
struct SnappingSlider  : public Slider
{
    double snapValue (double attemptedValue, DragMode dragMode) override
    {
        if (dragMode == notDragging)
            return attemptedValue;  // if they're entering the value in the text-box, don't mess with it.

        if (attemptedValue > 40 && attemptedValue < 60)
            return 50.0;

        return attemptedValue;
    }
};

/** A TextButton that pops up a colour chooser to change its colours. */
class ColourChangeButton  : public TextButton,
                            public ChangeListener
{
public:
    ColourChangeButton()
        : TextButton ("Click to change colour...")
    {
        setSize (10, 24);
        changeWidthToFitText();
    }

    void clicked() override
    {
        ColourSelector* colourSelector = new ColourSelector();
        colourSelector->setName ("background");
        colourSelector->setCurrentColour (findColour (TextButton::buttonColourId));
        colourSelector->addChangeListener (this);
        colourSelector->setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
        colourSelector->setSize (300, 400);

        CallOutBox::launchAsynchronously (colourSelector, getScreenBounds(), nullptr);
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (ColourSelector* cs = dynamic_cast<ColourSelector*> (source))
            setColour (TextButton::buttonColourId, cs->getCurrentColour());
    }
};

//==============================================================================
struct SlidersPage  : public Component
{
    SlidersPage()
        : hintLabel ("hint", "Try right-clicking on a slider for an options menu. \n\n"
                             "Also, holding down CTRL while dragging will turn on a slider's velocity-sensitive mode")
    {
        Slider* s = createSlider (false);
        s->setSliderStyle (Slider::LinearVertical);
        s->setTextBoxStyle (Slider::TextBoxBelow, false, 100, 20);
        s->setBounds (10, 25, 70, 200);
        s->setDoubleClickReturnValue (true, 50.0); // double-clicking this slider will set it to 50.0
        s->setTextValueSuffix (" units");

        s = createSlider (false);
        s->setSliderStyle (Slider::LinearVertical);
        s->setVelocityBasedMode (true);
        s->setSkewFactor (0.5);
        s->setTextBoxStyle (Slider::TextBoxAbove, true, 100, 20);
        s->setBounds (85, 25, 70, 200);
        s->setTextValueSuffix (" rels");

        s = createSlider (true);
        s->setSliderStyle (Slider::LinearHorizontal);
        s->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
        s->setBounds (180, 35, 150, 20);

        s = createSlider (false);
        s->setSliderStyle (Slider::LinearHorizontal);
        s->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        s->setBounds (180, 65, 150, 20);
        s->setPopupDisplayEnabled (true, this);
        s->setTextValueSuffix (" nuns required to change a lightbulb");

        s = createSlider (false);
        s->setSliderStyle (Slider::IncDecButtons);
        s->setTextBoxStyle (Slider::TextBoxLeft, false, 50, 20);
        s->setBounds (180, 105, 100, 20);
        s->setIncDecButtonsMode (Slider::incDecButtonsDraggable_Vertical);

        s = createSlider (false);
        s->setSliderStyle (Slider::Rotary);
        s->setRotaryParameters (float_Pi * 1.2f, float_Pi * 2.8f, false);
        s->setTextBoxStyle (Slider::TextBoxRight, false, 70, 20);
        s->setBounds (190, 145, 120, 40);
        s->setTextValueSuffix (" mm");

        s = createSlider (false);
        s->setSliderStyle (Slider::LinearBar);
        s->setBounds (180, 195, 100, 30);
        s->setTextValueSuffix (" gallons");

        s = createSlider (false);
        s->setSliderStyle (Slider::TwoValueHorizontal);
        s->setBounds (360, 20, 160, 40);

        s = createSlider (false);
        s->setSliderStyle (Slider::TwoValueVertical);
        s->setBounds (360, 110, 40, 160);

        s = createSlider (false);
        s->setSliderStyle (Slider::ThreeValueHorizontal);
        s->setBounds (360, 70, 160, 40);

        s = createSlider (false);
        s->setSliderStyle (Slider::ThreeValueVertical);
        s->setBounds (440, 110, 40, 160);

        s = createSlider (false);
        s->setSliderStyle (Slider::LinearBarVertical);
        s->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        s->setBounds (540, 35, 20, 230);
        s->setPopupDisplayEnabled (true, this);
        s->setTextValueSuffix (" mickles in a muckle");

        for (int i = 7; i <= 10; ++i)
        {
            sliders.getUnchecked(i)->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
            sliders.getUnchecked(i)->setPopupDisplayEnabled (true, this);
        }

        /* Here, we'll create a Value object, and tell a bunch of our sliders to use it as their
           value source. By telling them all to share the same Value, they'll stay in sync with
           each other.

           We could also optionally keep a copy of this Value elsewhere, and by changing it,
           cause all the sliders to automatically update.
        */
        Value sharedValue;
        sharedValue = Random::getSystemRandom().nextDouble() * 100;
        for (int i = 0; i < 7; ++i)
            sliders.getUnchecked(i)->getValueObject().referTo (sharedValue);

        // ..and now we'll do the same for all our min/max slider values..
        Value sharedValueMin, sharedValueMax;
        sharedValueMin = Random::getSystemRandom().nextDouble() * 40.0;
        sharedValueMax = Random::getSystemRandom().nextDouble() * 40.0 + 60.0;

        for (int i = 7; i <= 10; ++i)
        {
            sliders.getUnchecked(i)->getMaxValueObject().referTo (sharedValueMax);
            sliders.getUnchecked(i)->getMinValueObject().referTo (sharedValueMin);
        }

        hintLabel.setBounds (20, 245, 350, 150);
        addAndMakeVisible (hintLabel);
    }

private:
    OwnedArray<Slider> sliders;
    Label hintLabel;

    Slider* createSlider (bool isSnapping)
    {
        Slider* s = isSnapping ? new SnappingSlider() : new Slider();
        sliders.add (s);
        addAndMakeVisible (s);
        s->setRange (0.0, 100.0, 0.1);
        s->setPopupMenuEnabled (true);
        s->setValue (Random::getSystemRandom().nextDouble() * 100.0, dontSendNotification);
        return s;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlidersPage)
};

//==============================================================================
struct ButtonsPage   : public Component,
                       public ButtonListener
{
    ButtonsPage()
    {
        {
            GroupComponent* group = addToList (new GroupComponent ("group", "Radio buttons"));
            group->setBounds (20, 20, 220, 140);
        }

        for (int i = 0; i < 4; ++i)
        {
            ToggleButton* tb = addToList (new ToggleButton ("Radio Button #" + String (i + 1)));

            tb->setRadioGroupId (1234);
            tb->setBounds (45, 46 + i * 22, 180, 22);
            tb->setTooltip ("A set of mutually-exclusive radio buttons");

            if (i == 0)
                tb->setToggleState (true, dontSendNotification);
        }

        for (int i = 0; i < 4; ++i)
        {
            DrawablePath normal, over;

            Path p;
            p.addStar (Point<float>(), i + 5, 20.0f, 50.0f, -0.2f);
            normal.setPath (p);
            normal.setFill (Colours::lightblue);
            normal.setStrokeFill (Colours::black);
            normal.setStrokeThickness (4.0f);

            over.setPath (p);
            over.setFill (Colours::blue);
            over.setStrokeFill (Colours::black);
            over.setStrokeThickness (4.0f);

            DrawableButton* db = addToList (new DrawableButton (String (i + 5) + " points", DrawableButton::ImageAboveTextLabel));
            db->setImages (&normal, &over, 0);
            db->setClickingTogglesState (true);
            db->setRadioGroupId (23456);

            const int buttonSize = 50;
            db->setBounds (25 + i * buttonSize, 180, buttonSize, buttonSize);

            if (i == 0)
                db->setToggleState (true, dontSendNotification);
        }

        for (int i = 0; i < 4; ++i)
        {
            TextButton* tb = addToList (new TextButton ("Button " + String (i + 1)));

            tb->setClickingTogglesState (true);
            tb->setRadioGroupId (34567);
            tb->setColour (TextButton::buttonColourId, Colours::white);
            tb->setColour (TextButton::buttonOnColourId, Colours::blueviolet.brighter());

            tb->setBounds (20 + i * 55, 260, 55, 24);
            tb->setConnectedEdges (((i != 0) ? Button::ConnectedOnLeft : 0)
                                    | ((i != 3) ? Button::ConnectedOnRight : 0));

            if (i == 0)
                tb->setToggleState (true, dontSendNotification);
        }

        {
            ColourChangeButton* colourChangeButton = new ColourChangeButton();
            components.add (colourChangeButton);
            addAndMakeVisible (colourChangeButton);
            colourChangeButton->setTopLeftPosition (20, 320);
        }

        {
            HyperlinkButton* hyperlink = addToList (new HyperlinkButton ("This is a HyperlinkButton",
                                                                         URL ("http://www.juce.com")));
            hyperlink->setBounds (260, 20, 200, 24);
        }

        // create some drawables to use for our drawable buttons...
        DrawablePath normal, over;

        {
            Path p;
            p.addStar (Point<float>(), 5, 20.0f, 50.0f, 0.2f);
            normal.setPath (p);
            normal.setFill (getRandomDarkColour());
        }

        {
            Path p;
            p.addStar (Point<float>(), 9, 25.0f, 50.0f, 0.0f);
            over.setPath (p);
            over.setFill (getRandomBrightColour());
            over.setStrokeFill (getRandomDarkColour());
            over.setStrokeThickness (5.0f);
        }

        DrawableImage down;
        down.setImage (ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize));
        down.setOverlayColour (Colours::black.withAlpha (0.3f));

        {
            // create an image-above-text button from these drawables..
            DrawableButton* db = addToList (new DrawableButton ("Button 1", DrawableButton::ImageAboveTextLabel));
            db->setImages (&normal, &over, &down);
            db->setBounds (260, 60, 80, 80);
            db->setTooltip ("This is a DrawableButton with a label");
            db->addListener (this);
        }

        {
            // create an image-only button from these drawables..
            DrawableButton* db = addToList (new DrawableButton ("Button 2", DrawableButton::ImageFitted));
            db->setImages (&normal, &over, &down);
            db->setClickingTogglesState (true);
            db->setBounds (370, 60, 80, 80);
            db->setTooltip ("This is an image-only DrawableButton");
            db->addListener (this);
        }

        {
            // create an image-on-button-shape button from the same drawables..
            DrawableButton* db = addToList (new DrawableButton ("Button 3", DrawableButton::ImageOnButtonBackground));
            db->setImages (&normal, 0, 0);
            db->setBounds (260, 160, 110, 25);
            db->setTooltip ("This is a DrawableButton on a standard button background");
            db->addListener (this);
        }

        {
            DrawableButton* db = addToList (new DrawableButton ("Button 4", DrawableButton::ImageOnButtonBackground));
            db->setImages (&normal, &over, &down);
            db->setClickingTogglesState (true);
            db->setColour (DrawableButton::backgroundColourId, Colours::white);
            db->setColour (DrawableButton::backgroundOnColourId, Colours::yellow);
            db->setBounds (400, 150, 50, 50);
            db->setTooltip ("This is a DrawableButton on a standard button background");
            db->addListener (this);
        }

        {
            ShapeButton* sb = addToList (new ShapeButton ("ShapeButton",
                                                          getRandomDarkColour(),
                                                          getRandomDarkColour(),
                                                          getRandomDarkColour()));
            sb->setShape (MainAppWindow::getJUCELogoPath(), false, true, false);
            sb->setBounds (260, 220, 200, 120);
        }

        {
            ImageButton* ib = addToList (new ImageButton ("ImageButton"));

            Image juceImage = ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize);

            ib->setImages (true, true, true,
                           juceImage, 0.7f, Colours::transparentBlack,
                           juceImage, 1.0f, Colours::transparentBlack,
                           juceImage, 1.0f, getRandomBrightColour().withAlpha (0.8f),
                           0.5f);

            ib->setBounds (260, 350, 100, 100);
            ib->setTooltip ("ImageButton - showing alpha-channel hit-testing and colour overlay when clicked");
        }
    }

private:
    OwnedArray<Component> components;
    ScopedPointer<BubbleMessageComponent> bubbleMessage;

    // This little function avoids a bit of code-duplication by adding a component to
    // our list as well as calling addAndMakeVisible on it..
    template <typename ComponentType>
    ComponentType* addToList (ComponentType* newComp)
    {
        components.add (newComp);
        addAndMakeVisible (newComp);
        return newComp;
    }

    void buttonClicked (Button* button) override
    {
        showBubbleMessage (button,
                           "This is a demo of the BubbleMessageComponent, which lets you pop up a message pointing "
                           "at a component or somewhere on the screen.\n\n"
                           "The message bubbles will disappear after a timeout period, or when the mouse is clicked.",
                           bubbleMessage);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonsPage)
};


//==============================================================================
struct MiscPage   : public Component
{
    MiscPage()
        : textEditor2 ("Password", (juce_wchar) 0x2022),
          comboBox ("Combo")
    {
        addAndMakeVisible (textEditor1);
        textEditor1.setBounds (10, 25, 200, 24);
        textEditor1.setText ("Single-line text box");

        addAndMakeVisible (textEditor2);
        textEditor2.setBounds (10, 55, 200, 24);
        textEditor2.setText ("Password");

        addAndMakeVisible (comboBox);
        comboBox.setBounds (10, 85, 200, 24);
        comboBox.setEditableText (true);
        comboBox.setJustificationType (Justification::centred);

        for (int i = 1; i < 100; ++i)
            comboBox.addItem ("combo box item " + String (i), i);

        comboBox.setSelectedId (1);
    }

    TextEditor textEditor1, textEditor2;
    ComboBox comboBox;
};

//==============================================================================
class ToolbarDemoComp   : public Component,
                          public SliderListener,
                          public ButtonListener
{
public:
    ToolbarDemoComp()
        : depthLabel (String(), "Toolbar depth:"),
          infoLabel (String(), "As well as showing off toolbars, this demo illustrates how to store "
                               "a set of SVG files in a Zip file, embed that in your application, and read "
                               "them back in at runtime.\n\nThe icon images here are taken from the open-source "
                                "Tango icon project."),
          orientationButton ("Vertical/Horizontal"),
          customiseButton ("Customise...")
    {
        // Create and add the toolbar...
        addAndMakeVisible (toolbar);

        // And use our item factory to add a set of default icons to it...
        toolbar.addDefaultItems (factory);

        // Now we'll just create the other sliders and buttons on the demo page, which adjust
        // the toolbar's properties...
        addAndMakeVisible (infoLabel);
        infoLabel.setJustificationType (Justification::topLeft);
        infoLabel.setBounds (80, 80, 450, 100);
        infoLabel.setInterceptsMouseClicks (false, false);

        addAndMakeVisible (depthSlider);
        depthSlider.setRange (10.0, 200.0, 1.0);
        depthSlider.setValue (50, dontSendNotification);
        depthSlider.setSliderStyle (Slider::LinearHorizontal);
        depthSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
        depthSlider.addListener (this);
        depthSlider.setBounds (80, 210, 300, 22);
        depthLabel.attachToComponent (&depthSlider, false);

        addAndMakeVisible (orientationButton);
        orientationButton.addListener (this);
        orientationButton.changeWidthToFitText (22);
        orientationButton.setTopLeftPosition (depthSlider.getX(), depthSlider.getBottom() + 20);

        addAndMakeVisible (customiseButton);
        customiseButton.addListener (this);
        customiseButton.changeWidthToFitText (22);
        customiseButton.setTopLeftPosition (orientationButton.getRight() + 20, orientationButton.getY());
    }

    void resized() override
    {
        int toolbarThickness = (int) depthSlider.getValue();

        if (toolbar.isVertical())
            toolbar.setBounds (getLocalBounds().removeFromLeft (toolbarThickness));
        else
            toolbar.setBounds (getLocalBounds().removeFromTop  (toolbarThickness));
    }

    void sliderValueChanged (Slider*) override
    {
        resized();
    }

    void buttonClicked (Button* button) override
    {
        if (button == &orientationButton)
        {
            toolbar.setVertical (! toolbar.isVertical());
            resized();
        }
        else if (button == &customiseButton)
        {
            toolbar.showCustomisationDialog (factory);
        }
    }

private:
    Toolbar toolbar;
    Slider depthSlider;
    Label depthLabel, infoLabel;
    TextButton orientationButton, customiseButton;

    //==============================================================================
    class DemoToolbarItemFactory   : public ToolbarItemFactory
    {
    public:
        DemoToolbarItemFactory() {}

        //==============================================================================
        // Each type of item a toolbar can contain must be given a unique ID. These
        // are the ones we'll use in this demo.
        enum DemoToolbarItemIds
        {
            doc_new         = 1,
            doc_open        = 2,
            doc_save        = 3,
            doc_saveAs      = 4,
            edit_copy       = 5,
            edit_cut        = 6,
            edit_paste      = 7,
            juceLogoButton  = 8,
            customComboBox  = 9
        };

        void getAllToolbarItemIds (Array<int>& ids) override
        {
            // This returns the complete list of all item IDs that are allowed to
            // go in our toolbar. Any items you might want to add must be listed here. The
            // order in which they are listed will be used by the toolbar customisation panel.

            ids.add (doc_new);
            ids.add (doc_open);
            ids.add (doc_save);
            ids.add (doc_saveAs);
            ids.add (edit_copy);
            ids.add (edit_cut);
            ids.add (edit_paste);
            ids.add (juceLogoButton);
            ids.add (customComboBox);

            // If you're going to use separators, then they must also be added explicitly
            // to the list.
            ids.add (separatorBarId);
            ids.add (spacerId);
            ids.add (flexibleSpacerId);
        }

        void getDefaultItemSet (Array<int>& ids) override
        {
            // This returns an ordered list of the set of items that make up a
            // toolbar's default set. Not all items need to be on this list, and
            // items can appear multiple times (e.g. the separators used here).
            ids.add (doc_new);
            ids.add (doc_open);
            ids.add (doc_save);
            ids.add (doc_saveAs);
            ids.add (spacerId);
            ids.add (separatorBarId);
            ids.add (edit_copy);
            ids.add (edit_cut);
            ids.add (edit_paste);
            ids.add (separatorBarId);
            ids.add (flexibleSpacerId);
            ids.add (customComboBox);
            ids.add (flexibleSpacerId);
            ids.add (separatorBarId);
            ids.add (juceLogoButton);
        }

        ToolbarItemComponent* createItem (int itemId) override
        {
            switch (itemId)
            {
                case doc_new:           return createButtonFromZipFileSVG (itemId, "new", "document-new.svg");
                case doc_open:          return createButtonFromZipFileSVG (itemId, "open", "document-open.svg");
                case doc_save:          return createButtonFromZipFileSVG (itemId, "save", "document-save.svg");
                case doc_saveAs:        return createButtonFromZipFileSVG (itemId, "save as", "document-save-as.svg");
                case edit_copy:         return createButtonFromZipFileSVG (itemId, "copy", "edit-copy.svg");
                case edit_cut:          return createButtonFromZipFileSVG (itemId, "cut", "edit-cut.svg");
                case edit_paste:        return createButtonFromZipFileSVG (itemId, "paste", "edit-paste.svg");
                case juceLogoButton:    return new ToolbarButton (itemId, "juce!", Drawable::createFromImageData (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize), 0);
                case customComboBox:    return new CustomToolbarComboBox (itemId);
                default:                break;
            }

            return nullptr;
        }

    private:
        StringArray iconNames;
        OwnedArray<Drawable> iconsFromZipFile;

        // This is a little utility to create a button with one of the SVG images in
        // our embedded ZIP file "icons.zip"
        ToolbarButton* createButtonFromZipFileSVG (const int itemId, const String& text, const String& filename)
        {
            if (iconsFromZipFile.size() == 0)
            {
                // If we've not already done so, load all the images from the zip file..
                MemoryInputStream iconsFileStream (BinaryData::icons_zip, BinaryData::icons_zipSize, false);
                ZipFile icons (&iconsFileStream, false);

                for (int i = 0; i < icons.getNumEntries(); ++i)
                {
                    ScopedPointer<InputStream> svgFileStream (icons.createStreamForEntry (i));

                    if (svgFileStream != nullptr)
                    {
                        iconNames.add (icons.getEntry(i)->filename);
                        iconsFromZipFile.add (Drawable::createFromImageDataStream (*svgFileStream));
                    }
                }
            }

            Drawable* image = iconsFromZipFile [iconNames.indexOf (filename)]->createCopy();
            return new ToolbarButton (itemId, text, image, 0);
        }

        // Demonstrates how to put a custom component into a toolbar - this one contains
        // a ComboBox.
        class CustomToolbarComboBox : public ToolbarItemComponent
        {
        public:
            CustomToolbarComboBox (const int toolbarItemId)
                : ToolbarItemComponent (toolbarItemId, "Custom Toolbar Item", false),
                  comboBox ("demo toolbar combo box")
            {
                addAndMakeVisible (comboBox);

                for (int i = 1; i < 20; ++i)
                    comboBox.addItem ("Toolbar ComboBox item " + String (i), i);

                comboBox.setSelectedId (1);
                comboBox.setEditableText (true);
            }

            bool getToolbarItemSizes (int /*toolbarDepth*/, bool isVertical,
                                      int& preferredSize, int& minSize, int& maxSize) override
            {
                if (isVertical)
                    return false;

                preferredSize = 250;
                minSize = 80;
                maxSize = 300;
                return true;
            }

            void paintButtonArea (Graphics&, int, int, bool, bool) override
            {
            }

            void contentAreaChanged (const Rectangle<int>& newArea) override
            {
                comboBox.setSize (newArea.getWidth() - 2,
                                  jmin (newArea.getHeight() - 2, 22));

                comboBox.setCentrePosition (newArea.getCentreX(), newArea.getCentreY());
            }

        private:
            ComboBox comboBox;
        };
    };

    DemoToolbarItemFactory factory;
};


//==============================================================================
/**
    This class shows how to implement a TableListBoxModel to show in a TableListBox.
*/
class TableDemoComponent    : public Component,
                              public TableListBoxModel
{
public:
    TableDemoComponent()   : font (14.0f)
    {
        // Load some data from an embedded XML file..
        loadData();

        // Create our table component and add it to this component..
        addAndMakeVisible (table);
        table.setModel (this);

        // give it a border
        table.setColour (ListBox::outlineColourId, Colours::grey);
        table.setOutlineThickness (1);

        // Add some columns to the table header, based on the column list in our database..
        forEachXmlChildElement (*columnList, columnXml)
        {
            table.getHeader().addColumn (columnXml->getStringAttribute ("name"),
                                         columnXml->getIntAttribute ("columnId"),
                                         columnXml->getIntAttribute ("width"),
                                         50, 400,
                                         TableHeaderComponent::defaultFlags);
        }

        // we could now change some initial settings..
        table.getHeader().setSortColumnId (1, true); // sort forwards by the ID column
        table.getHeader().setColumnVisible (7, false); // hide the "length" column until the user shows it

        // un-comment this line to have a go of stretch-to-fit mode
        // table.getHeader().setStretchToFitActive (true);

        table.setMultipleSelectionEnabled (true);
    }

    // This is overloaded from TableListBoxModel, and must return the total number of rows in our table
    int getNumRows() override
    {
        return numRows;
    }

    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll (Colours::lightblue);
        else if (rowNumber % 2)
            g.fillAll (Colour (0xffeeeeee));
    }

    // This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom
    // components.
    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour (Colours::black);
        g.setFont (font);

        if (const XmlElement* rowElement = dataList->getChildElement (rowNumber))
        {
            const String text (rowElement->getStringAttribute (getAttributeNameForColumnId (columnId)));

            g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
        }

        g.setColour (Colours::black.withAlpha (0.2f));
        g.fillRect (width - 1, 0, 1, height);
    }

    // This is overloaded from TableListBoxModel, and tells us that the user has clicked a table header
    // to change the sort order.
    void sortOrderChanged (int newSortColumnId, bool isForwards) override
    {
        if (newSortColumnId != 0)
        {
            DemoDataSorter sorter (getAttributeNameForColumnId (newSortColumnId), isForwards);
            dataList->sortChildElements (sorter);

            table.updateContent();
        }
    }

    // This is overloaded from TableListBoxModel, and must update any custom components that we're using
    Component* refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                        Component* existingComponentToUpdate) override
    {
        if (columnId == 1 || columnId == 7) // The ID and Length columns do not have a custom component
        {
            jassert (existingComponentToUpdate == nullptr);
            return nullptr;
        }

        if (columnId == 5) // For the ratings column, we return the custom combobox component
        {
            RatingColumnCustomComponent* ratingsBox = static_cast<RatingColumnCustomComponent*> (existingComponentToUpdate);

            // If an existing component is being passed-in for updating, we'll re-use it, but
            // if not, we'll have to create one.
            if (ratingsBox == nullptr)
                ratingsBox = new RatingColumnCustomComponent (*this);

            ratingsBox->setRowAndColumn (rowNumber, columnId);
            return ratingsBox;
        }

        // The other columns are editable text columns, for which we use the custom Label component
        EditableTextCustomComponent* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

        // same as above...
        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent (*this);

        textLabel->setRowAndColumn (rowNumber, columnId);
        return textLabel;
    }

    // This is overloaded from TableListBoxModel, and should choose the best width for the specified
    // column.
    int getColumnAutoSizeWidth (int columnId) override
    {
        if (columnId == 5)
            return 100; // (this is the ratings column, containing a custom combobox component)

        int widest = 32;

        // find the widest bit of text in this column..
        for (int i = getNumRows(); --i >= 0;)
        {
            if (const XmlElement* rowElement = dataList->getChildElement (i))
            {
                const String text (rowElement->getStringAttribute (getAttributeNameForColumnId (columnId)));

                widest = jmax (widest, font.getStringWidth (text));
            }
        }

        return widest + 8;
    }

    // A couple of quick methods to set and get cell values when the user changes them
    int getRating (const int rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getIntAttribute ("Rating");
    }

    void setRating (const int rowNumber, const int newRating)
    {
        dataList->getChildElement (rowNumber)->setAttribute ("Rating", newRating);
    }

    String getText (const int columnNumber, const int rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getStringAttribute ( getAttributeNameForColumnId(columnNumber));
    }

    void setText (const int columnNumber, const int rowNumber, const String& newText)
    {
        const String& columnName = table.getHeader().getColumnName (columnNumber);
        dataList->getChildElement (rowNumber)->setAttribute (columnName, newText);
    }

    //==============================================================================
    void resized() override
    {
        // position our table with a gap around its edge
        table.setBoundsInset (BorderSize<int> (8));
    }


private:
    TableListBox table;     // the table component itself
    Font font;

    ScopedPointer<XmlElement> demoData;   // This is the XML document loaded from the embedded file "demo table data.xml"
    XmlElement* columnList; // A pointer to the sub-node of demoData that contains the list of columns
    XmlElement* dataList;   // A pointer to the sub-node of demoData that contains the list of data rows
    int numRows;            // The number of rows of data we've got

    //==============================================================================
    // This is a custom Label component, which we use for the table's editable text columns.
    class EditableTextCustomComponent  : public Label
    {
    public:
        EditableTextCustomComponent (TableDemoComponent& td)  : owner (td)
        {
            // double click to edit the label text; single click handled below
            setEditable (false, true, false);
            setColour (textColourId, Colours::black);
        }

        void mouseDown (const MouseEvent& event) override
        {
            // single click on the label should simply select the row
            owner.table.selectRowsBasedOnModifierKeys (row, event.mods, false);

            Label::mouseDown (event);
        }

        void textWasEdited() override
        {
            owner.setText (columnId, row, getText());
        }

        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText (owner.getText(columnId, row), dontSendNotification);
        }

    private:
        TableDemoComponent& owner;
        int row, columnId;
    };


    //==============================================================================
    // This is a custom component containing a combo box, which we're going to put inside
    // our table's "rating" column.
    class RatingColumnCustomComponent    : public Component,
                                           private ComboBoxListener
    {
    public:
        RatingColumnCustomComponent (TableDemoComponent& td)  : owner (td)
        {
            // just put a combo box inside this component
            addAndMakeVisible (comboBox);
            comboBox.addItem ("fab", 1);
            comboBox.addItem ("groovy", 2);
            comboBox.addItem ("hep", 3);
            comboBox.addItem ("mad for it", 4);
            comboBox.addItem ("neat", 5);
            comboBox.addItem ("swingin", 6);
            comboBox.addItem ("wild", 7);

            // when the combo is changed, we'll get a callback.
            comboBox.addListener (this);
            comboBox.setWantsKeyboardFocus (false);
        }

        void resized() override
        {
            comboBox.setBoundsInset (BorderSize<int> (2));
        }

        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn (int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            comboBox.setSelectedId (owner.getRating (row), dontSendNotification);
        }

        void comboBoxChanged (ComboBox*) override
        {
            owner.setRating (row, comboBox.getSelectedId());
        }

    private:
        TableDemoComponent& owner;
        ComboBox comboBox;
        int row, columnId;
    };

    //==============================================================================
    // A comparator used to sort our data when the user clicks a column header
    class DemoDataSorter
    {
    public:
        DemoDataSorter (const String& attributeToSortBy, bool forwards)
            : attributeToSort (attributeToSortBy),
              direction (forwards ? 1 : -1)
        {
        }

        int compareElements (XmlElement* first, XmlElement* second) const
        {
            int result = first->getStringAttribute (attributeToSort)
                           .compareNatural (second->getStringAttribute (attributeToSort));

            if (result == 0)
                result = first->getStringAttribute ("ID")
                           .compareNatural (second->getStringAttribute ("ID"));

            return direction * result;
        }

    private:
        String attributeToSort;
        int direction;
    };

    //==============================================================================
    // this loads the embedded database XML file into memory
    void loadData()
    {
        demoData = XmlDocument::parse (BinaryData::demo_table_data_xml);

        dataList   = demoData->getChildByName ("DATA");
        columnList = demoData->getChildByName ("COLUMNS");

        numRows = dataList->getNumChildElements();
    }

    // (a utility method to search our XML for the attribute that matches a column ID)
    String getAttributeNameForColumnId (const int columnId) const
    {
        forEachXmlChildElement (*columnList, columnXml)
        {
            if (columnXml->getIntAttribute ("columnId") == columnId)
                return columnXml->getStringAttribute ("name");
        }

        return String();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableDemoComponent)
};

//==============================================================================
class DragAndDropDemo  : public Component,
                         public DragAndDropContainer
{
public:
    DragAndDropDemo()
        : sourceListBox ("D+D source", nullptr)
    {
        setName ("Drag-and-Drop");

        sourceListBox.setModel (&sourceModel);
        sourceListBox.setMultipleSelectionEnabled (true);

        addAndMakeVisible (sourceListBox);
        addAndMakeVisible (target);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (8));

        sourceListBox.setBounds (r.withSize (250, 180));
        target.setBounds (r.removeFromBottom (150).removeFromRight (250));
    }

private:
    //==============================================================================
    struct SourceItemListboxContents  : public ListBoxModel
    {
        // The following methods implement the necessary virtual functions from ListBoxModel,
        // telling the listbox how many rows there are, painting them, etc.
        int getNumRows() override
        {
            return 30;
        }

        void paintListBoxItem (int rowNumber, Graphics& g,
                               int width, int height, bool rowIsSelected) override
        {
            if (rowIsSelected)
                g.fillAll (Colours::lightblue);

            g.setColour (Colours::black);
            g.setFont (height * 0.7f);

            g.drawText ("Draggable Thing #" + String (rowNumber + 1),
                        5, 0, width, height,
                        Justification::centredLeft, true);
        }

        var getDragSourceDescription (const SparseSet<int>& selectedRows) override
        {
            // for our drag description, we'll just make a comma-separated list of the selected row
            // numbers - this will be picked up by the drag target and displayed in its box.
            StringArray rows;

            for (int i = 0; i < selectedRows.size(); ++i)
                rows.add (String (selectedRows[i] + 1));

            return rows.joinIntoString (", ");
        }
    };

    //==============================================================================
    // and this is a component that can have things dropped onto it..
    class DragAndDropDemoTarget : public Component,
                                  public DragAndDropTarget,
                                  public FileDragAndDropTarget,
                                  public TextDragAndDropTarget
    {
    public:
        DragAndDropDemoTarget()
            : message ("Drag-and-drop some rows from the top-left box onto this component!\n\n"
                       "You can also drag-and-drop files and text from other apps"),
              somethingIsBeingDraggedOver (false)
        {
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Colours::green.withAlpha (0.2f));

            // draw a red line around the comp if the user's currently dragging something over it..
            if (somethingIsBeingDraggedOver)
            {
                g.setColour (Colours::red);
                g.drawRect (getLocalBounds(), 3);
            }

            g.setColour (Colours::black);
            g.setFont (14.0f);
            g.drawFittedText (message, getLocalBounds().reduced (10, 0), Justification::centred, 4);
        }

        //==============================================================================
        // These methods implement the DragAndDropTarget interface, and allow our component
        // to accept drag-and-drop of objects from other Juce components..

        bool isInterestedInDragSource (const SourceDetails& /*dragSourceDetails*/) override
        {
            // normally you'd check the sourceDescription value to see if it's the
            // sort of object that you're interested in before returning true, but for
            // the demo, we'll say yes to anything..
            return true;
        }

        void itemDragEnter (const SourceDetails& /*dragSourceDetails*/) override
        {
            somethingIsBeingDraggedOver = true;
            repaint();
        }

        void itemDragMove (const SourceDetails& /*dragSourceDetails*/) override
        {
        }

        void itemDragExit (const SourceDetails& /*dragSourceDetails*/) override
        {
            somethingIsBeingDraggedOver = false;
            repaint();
        }

        void itemDropped (const SourceDetails& dragSourceDetails) override
        {
            message = "Items dropped: " + dragSourceDetails.description.toString();

            somethingIsBeingDraggedOver = false;
            repaint();
        }

        //==============================================================================
        // These methods implement the FileDragAndDropTarget interface, and allow our component
        // to accept drag-and-drop of files..

        bool isInterestedInFileDrag (const StringArray& /*files*/) override
        {
            // normally you'd check these files to see if they're something that you're
            // interested in before returning true, but for the demo, we'll say yes to anything..
            return true;
        }

        void fileDragEnter (const StringArray& /*files*/, int /*x*/, int /*y*/) override
        {
            somethingIsBeingDraggedOver = true;
            repaint();
        }

        void fileDragMove (const StringArray& /*files*/, int /*x*/, int /*y*/) override
        {
        }

        void fileDragExit (const StringArray& /*files*/) override
        {
            somethingIsBeingDraggedOver = false;
            repaint();
        }

        void filesDropped (const StringArray& files, int /*x*/, int /*y*/) override
        {
            message = "Files dropped: " + files.joinIntoString ("\n");

            somethingIsBeingDraggedOver = false;
            repaint();
        }

        //==============================================================================
        // These methods implement the TextDragAndDropTarget interface, and allow our component
        // to accept drag-and-drop of text..

        bool isInterestedInTextDrag (const String& /*text*/) override
        {
            return true;
        }

        void textDragEnter (const String& /*text*/, int /*x*/, int /*y*/) override
        {
            somethingIsBeingDraggedOver = true;
            repaint();
        }

        void textDragMove (const String& /*text*/, int /*x*/, int /*y*/) override
        {
        }

        void textDragExit (const String& /*text*/) override
        {
            somethingIsBeingDraggedOver = false;
            repaint();
        }

        void textDropped (const String& text, int /*x*/, int /*y*/) override
        {
            message = "Text dropped:\n" + text;

            somethingIsBeingDraggedOver = false;
            repaint();
        }

    private:
        String message;
        bool somethingIsBeingDraggedOver;
    };

    //==============================================================================
    ListBox sourceListBox;
    SourceItemListboxContents sourceModel;
    DragAndDropDemoTarget target;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DragAndDropDemo)
};

//==============================================================================
class MenusDemo : public Component,
                  public MenuBarModel,
                  private Button::Listener
{
public:
    MenusDemo()
    {
        addAndMakeVisible (menuBar = new MenuBarComponent (this));

        popupButton.setButtonText ("Show Popup Menu");
        popupButton.setTriggeredOnMouseDown (true);
        popupButton.addListener (this);
        addAndMakeVisible (popupButton);

        setApplicationCommandManagerToWatch (&MainAppWindow::getApplicationCommandManager());
    }

    ~MenusDemo()
    {
       #if JUCE_MAC
        MenuBarModel::setMacMainMenu (nullptr);
       #endif
        PopupMenu::dismissAllActiveMenus();

        popupButton.removeListener (this);
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds());
        menuBar->setBounds (area.removeFromTop (LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight()));

        area.removeFromTop (20);
        area = area.removeFromTop (33);
        popupButton.setBounds (area.removeFromLeft (200).reduced (5));
    }

    //==============================================================================
    StringArray getMenuBarNames() override
    {
        const char* const names[] = { "Demo", "Look-and-feel", "Tabs", "Misc", nullptr };

        return StringArray (names);
    }

    PopupMenu getMenuForIndex (int menuIndex, const String& /*menuName*/) override
    {
        ApplicationCommandManager* commandManager = &MainAppWindow::getApplicationCommandManager();

        PopupMenu menu;

        if (menuIndex == 0)
        {
            menu.addCommandItem (commandManager, MainAppWindow::showPreviousDemo);
            menu.addCommandItem (commandManager, MainAppWindow::showNextDemo);
            menu.addSeparator();
            menu.addCommandItem (commandManager, StandardApplicationCommandIDs::quit);
        }
        else if (menuIndex == 1)
        {
            menu.addCommandItem (commandManager, MainAppWindow::useLookAndFeelV1);
            menu.addCommandItem (commandManager, MainAppWindow::useLookAndFeelV2);
            menu.addCommandItem (commandManager, MainAppWindow::useLookAndFeelV3);
            menu.addSeparator();
            menu.addCommandItem (commandManager, MainAppWindow::useNativeTitleBar);

           #if JUCE_MAC
            menu.addItem (6000, "Use Native Menu Bar");
           #endif

           #if ! JUCE_LINUX
            menu.addCommandItem (commandManager, MainAppWindow::goToKioskMode);
           #endif

            if (MainAppWindow* mainWindow = MainAppWindow::getMainAppWindow())
            {
                StringArray engines (mainWindow->getRenderingEngines());

                if (engines.size() > 1)
                {
                    menu.addSeparator();

                    for (int i = 0; i < engines.size(); ++i)
                        menu.addCommandItem (commandManager, MainAppWindow::renderingEngineOne + i);
                }
            }
        }
        else if (menuIndex == 2)
        {
            if (TabbedComponent* tabs = findParentComponentOfClass<TabbedComponent>())
            {
                menu.addItem (3000, "Tabs at Top",    true, tabs->getOrientation() == TabbedButtonBar::TabsAtTop);
                menu.addItem (3001, "Tabs at Bottom", true, tabs->getOrientation() == TabbedButtonBar::TabsAtBottom);
                menu.addItem (3002, "Tabs on Left",   true, tabs->getOrientation() == TabbedButtonBar::TabsAtLeft);
                menu.addItem (3003, "Tabs on Right",  true, tabs->getOrientation() == TabbedButtonBar::TabsAtRight);
            }
        }
        else if (menuIndex == 3)
        {
            return getDummyPopupMenu();
        }

        return menu;
    }

    void menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/) override
    {
        // most of our menu items are invoked automatically as commands, but we can handle the
        // other special cases here..

        if (menuItemID == 6000)
        {
           #if JUCE_MAC
            if (MenuBarModel::getMacMainMenu() != nullptr)
            {
                MenuBarModel::setMacMainMenu (nullptr);
                menuBar->setModel (this);
            }
            else
            {
                menuBar->setModel (nullptr);
                MenuBarModel::setMacMainMenu (this);
            }
           #endif
        }
        else if (menuItemID >= 3000 && menuItemID <= 3003)
        {
            if (TabbedComponent* tabs = findParentComponentOfClass<TabbedComponent>())
            {
                TabbedButtonBar::Orientation o = TabbedButtonBar::TabsAtTop;

                if (menuItemID == 3001) o = TabbedButtonBar::TabsAtBottom;
                if (menuItemID == 3002) o = TabbedButtonBar::TabsAtLeft;
                if (menuItemID == 3003) o = TabbedButtonBar::TabsAtRight;

                tabs->setOrientation (o);
            }
        }
    }

private:
    TextButton popupButton;
    ScopedPointer<MenuBarComponent> menuBar;

    PopupMenu getDummyPopupMenu()
    {
        PopupMenu m;
        m.addItem (1, "Normal item");
        m.addItem (2, "Disabled item", false);
        m.addItem (3, "Ticked item", true, true);
        m.addColouredItem (4, "Coloured item", Colours::green);
        m.addSeparator();
        m.addCustomItem (5, new CustomMenuComponent());
        m.addSeparator();

        for (int i = 0; i < 8; ++i)
        {
            PopupMenu subMenu;

            for (int s = 0; s < 8; ++s)
            {
                PopupMenu subSubMenu;

                for (int item = 0; item < 8; ++item)
                    subSubMenu.addItem (1000 + (i * s * item), "Item " + String (item + 1));

                subMenu.addSubMenu ("Sub-sub menu " + String (s + 1), subSubMenu);
            }

            m.addSubMenu ("Sub menu " + String (i + 1), subMenu);
        }

        return m;
    }

    //==============================================================================
    void buttonClicked (Button* button) override
    {
        if (button == &popupButton)
            getDummyPopupMenu().showMenuAsync (PopupMenu::Options().withTargetComponent (&popupButton), nullptr);
    }

    //==============================================================================
    class CustomMenuComponent   : public PopupMenu::CustomComponent,
                                  private Timer
    {
    public:
        CustomMenuComponent()
        {
            // set off a timer to move a blob around on this component every
            // 300 milliseconds - see the timerCallback() method.
            startTimer (300);
        }

        void getIdealSize (int& idealWidth, int& idealHeight) override
        {
            // tells the menu how big we'd like to be..
            idealWidth = 200;
            idealHeight = 60;
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Colours::yellow.withAlpha (0.3f));

            g.setColour (Colours::pink);
            g.fillEllipse (blobPosition);

            g.setFont (Font (14.0f, Font::italic));
            g.setColour (Colours::black);

            g.drawFittedText ("This is a customised menu item (also demonstrating the Timer class)...",
                              getLocalBounds().reduced (4, 0),
                              Justification::centred, 3);
        }

    private:
        void timerCallback() override
        {
            Random random;
            blobPosition.setBounds ((float) random.nextInt (getWidth()),
                                    (float) random.nextInt (getHeight()),
                                    40.0f, 30.0f);
            repaint();
        }

        Rectangle<float> blobPosition;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenusDemo)
};

//==============================================================================
class DemoTabbedComponent  : public TabbedComponent
{
public:
    DemoTabbedComponent()
        : TabbedComponent (TabbedButtonBar::TabsAtTop)
    {
        addTab ("Menus",            getRandomTabBackgroundColour(), new MenusDemo(),           true);
        addTab ("Buttons",          getRandomTabBackgroundColour(), new ButtonsPage(),         true);
        addTab ("Sliders",          getRandomTabBackgroundColour(), new SlidersPage(),         true);
        addTab ("Toolbars",         getRandomTabBackgroundColour(), new ToolbarDemoComp(),     true);
        addTab ("Misc",             getRandomTabBackgroundColour(), new MiscPage(),            true);
        addTab ("Tables",           getRandomTabBackgroundColour(), new TableDemoComponent(),  true);
        addTab ("Drag & Drop",      getRandomTabBackgroundColour(), new DragAndDropDemo(),     true);

        getTabbedButtonBar().getTabButton (5)->setExtraComponent (new CustomTabButton(), TabBarButton::afterText);
    }

    static Colour getRandomTabBackgroundColour()
    {
        return Colour (Random::getSystemRandom().nextFloat(), 0.1f, 0.97f, 1.0f);
    }

    // This is a small star button that is put inside one of the tabs. You can
    // use this technique to create things like "close tab" buttons, etc.
    class CustomTabButton  : public Component
    {
    public:
        CustomTabButton()
        {
            setSize (20, 20);
        }

        void paint (Graphics& g) override
        {
            Path star;
            star.addStar (Point<float>(), 7, 1.0f, 2.0f);

            g.setColour (Colours::green);
            g.fillPath (star, star.getTransformToScaleToFit (getLocalBounds().reduced (2).toFloat(), true));
        }

        void mouseDown (const MouseEvent&) override
        {
            showBubbleMessage (this,
                               "This is a custom tab component\n"
                               "\n"
                               "You can use these to implement things like close-buttons "
                               "or status displays for your tabs.",
                               bubbleMessage);
        }
    private:
        ScopedPointer<BubbleMessageComponent> bubbleMessage;
    };
};

//==============================================================================
class WidgetsDemo   : public Component
{
public:
    WidgetsDemo()
    {
        setOpaque (true);
        addAndMakeVisible (tabs);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white);
    }

    void resized() override
    {
        tabs.setBounds (getLocalBounds().reduced (4));
    }

private:
    DemoTabbedComponent tabs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WidgetsDemo)
};

// This static object will register this demo type in a global list of demos..
static JuceDemoType<WidgetsDemo> demo ("09 Components: Tabs & Widgets");
