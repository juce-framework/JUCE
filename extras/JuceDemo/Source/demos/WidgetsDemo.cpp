/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#include "../jucedemo_headers.h"


//==============================================================================
class BouncingBallComponent : public Component,
                              public Timer
{
public:
    BouncingBallComponent()
    {
        Random random;

        const float size = 10.0f + random.nextInt (30);

        ballBounds.setBounds (random.nextFloat() * 100.0f,
                              random.nextFloat() * 100.0f,
                              size, size);

        direction.x = random.nextFloat() * 8.0f - 4.0f;
        direction.y = random.nextFloat() * 8.0f - 4.0f;

        colour = Colour ((uint32) random.nextInt())
                    .withAlpha (0.5f)
                    .withBrightness (0.7f);

        startTimer (60);
    }

    void paint (Graphics& g)
    {
        g.setColour (colour);
        g.fillEllipse (ballBounds - getPosition().toFloat());
    }

    void timerCallback()
    {
        ballBounds += direction;

        if (ballBounds.getX() < 0)                      direction.x =  fabsf (direction.x);
        if (ballBounds.getY() < 0)                      direction.y =  fabsf (direction.y);
        if (ballBounds.getRight() > getParentWidth())   direction.x = -fabsf (direction.x);
        if (ballBounds.getBottom() > getParentHeight()) direction.y = -fabsf (direction.y);

        setBounds (ballBounds.getSmallestIntegerContainer());
    }

    bool hitTest (int /* x */, int /* y */)
    {
        return false;
    }

private:
    Colour colour;
    Rectangle<float> ballBounds;
    Point<float> direction;
};

//==============================================================================
class DragOntoDesktopDemoComp : public Component
{
public:
    DragOntoDesktopDemoComp (Component* p)
        : parent (p)
    {
        // show off semi-transparency if it's supported by the current OS.
        setOpaque (! Desktop::canUseSemiTransparentWindows());

        for (int i = 0; i < numElementsInArray (balls); ++i)
            addAndMakeVisible (&(balls[i]));
    }

    void mouseDown (const MouseEvent& e)
    {
        dragger.startDraggingComponent (this, e);
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (parent == nullptr)
        {
            delete this;  // If our parent has been deleted, we'll just get rid of this component
        }
        else
        {
            // if the mouse is inside the parent component, we'll make that the
            // parent - otherwise, we'll put this comp on the desktop.
            if (parent->getLocalBounds().contains (e.getEventRelativeTo (parent).getPosition()))
            {
                // re-add this component to a parent component, which will
                // remove it from the desktop..
                parent->addChildComponent (this);
            }
            else
            {
                // add the component to the desktop, which will remove it
                // from its current parent component..
                addToDesktop (ComponentPeer::windowIsTemporary);
            }

            dragger.dragComponent (this, e, 0);
        }
    }

    void paint (Graphics& g)
    {
        if (isOpaque())
            g.fillAll (Colours::white);
        else
            g.fillAll (Colours::blue.withAlpha (0.2f));

        g.setFont (15.0f);
        g.setColour (Colours::black);
        g.drawFittedText ("drag this box onto the desktop to show how the same component can move from being lightweight to being a separate window",
                          getLocalBounds().reduced (4, 0),
                          Justification::horizontallyJustified, 5);

        g.drawRect (getLocalBounds());
    }

private:
    Component::SafePointer<Component> parent; // A safe-pointer will become null if the component that it refers to is deleted..
    ComponentDragger dragger;

    BouncingBallComponent balls[3];
};

//==============================================================================
class CustomMenuComponent  : public PopupMenu::CustomComponent,
                             public Timer
{
public:
    CustomMenuComponent()
    {
        // set off a timer to move a blob around on this component every
        // 300 milliseconds - see the timerCallback() method.
        startTimer (300);
    }

    void getIdealSize (int& idealWidth,
                       int& idealHeight)
    {
        // tells the menu how big we'd like to be..
        idealWidth = 200;
        idealHeight = 60;
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::yellow.withAlpha (0.3f));

        g.setColour (Colours::pink);
        g.fillEllipse (blobPosition);

        g.setFont (Font (14.0f, Font::italic));
        g.setColour (Colours::black);

        g.drawFittedText ("this is a customised menu item (also demonstrating the Timer class)...",
                          getLocalBounds().reduced (4, 0),
                          Justification::centred, 3);
    }

    void timerCallback()
    {
        Random random;
        blobPosition.setBounds ((float) random.nextInt (getWidth()),
                                (float) random.nextInt (getHeight()),
                                40.0f, 30.0f);
        repaint();
    }

private:
    Rectangle<float> blobPosition;
};

//==============================================================================
/** To demonstrate how sliders can have custom snapping applied to their values,
    this simple class snaps the value to 50 if it comes near.
*/
class SnappingSlider  : public Slider
{
public:
    SnappingSlider (const String& name)
        : Slider (name)
    {
    }

    double snapValue (double attemptedValue, bool userIsDragging)
    {
        if (! userIsDragging)
            return attemptedValue;  // if they're entering the value in the text-box, don't mess with it.

        if (attemptedValue > 40 && attemptedValue < 60)
            return 50.0;
        else
            return attemptedValue;
    }
};

/** A TextButton that pops up a colour chooser to change its colours. */
class ColourChangeButton  : public TextButton,
                            public ChangeListener
{
public:
    ColourChangeButton()
        : TextButton ("click to change colour...")
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

    void changeListenerCallback (ChangeBroadcaster* source)
    {
        if (ColourSelector* cs = dynamic_cast <ColourSelector*> (source))
            setColour (TextButton::buttonColourId, cs->getCurrentColour());
    }
};

//==============================================================================
/* A component to act as a simple container for our demos, which deletes all the child
   components that we stuff into it.
*/
class DemoPageComp  : public Component
{
public:
    DemoPageComp()
    {
    }

    ~DemoPageComp()
    {
        /* Deleting your child components indiscriminately using deleteAllChildren() is not recommended! It's much
           safer to make them embedded members or use ScopedPointers to automatically manage their lifetimes!

           In this demo, where we're throwing together a whole bunch of random components, it's simpler to do it
           like this, but don't treat this as an example of good practice!
        */
        deleteAllChildren();
    }
};

//==============================================================================
static Component* createSlidersPage()
{
    DemoPageComp* page = new DemoPageComp();

    const int numSliders = 11;
    Slider* sliders [numSliders];

    for (int i = 0; i < numSliders; ++i)
    {
        if (i == 2)
            page->addAndMakeVisible (sliders[i] = new SnappingSlider ("slider"));
        else
            page->addAndMakeVisible (sliders[i] = new Slider ("slider"));

        sliders[i]->setRange (0.0, 100.0, 0.1);
        sliders[i]->setPopupMenuEnabled (true);
        sliders[i]->setValue (Random::getSystemRandom().nextDouble() * 100.0, dontSendNotification);
    }

    sliders[0]->setSliderStyle (Slider::LinearVertical);
    sliders[0]->setTextBoxStyle (Slider::TextBoxBelow, false, 100, 20);
    sliders[0]->setBounds (10, 25, 70, 200);
    sliders[0]->setDoubleClickReturnValue (true, 50.0); // double-clicking this slider will set it to 50.0
    sliders[0]->setTextValueSuffix (" units");

    sliders[1]->setSliderStyle (Slider::LinearVertical);
    sliders[1]->setVelocityBasedMode (true);
    sliders[1]->setSkewFactor (0.5);
    sliders[1]->setTextBoxStyle (Slider::TextBoxAbove, true, 100, 20);
    sliders[1]->setBounds (85, 25, 70, 200);
    sliders[1]->setTextValueSuffix (" rels");

    sliders[2]->setSliderStyle (Slider::LinearHorizontal);
    sliders[2]->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
    sliders[2]->setBounds (180, 35, 150, 20);

    sliders[3]->setSliderStyle (Slider::LinearHorizontal);
    sliders[3]->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    sliders[3]->setBounds (180, 65, 150, 20);
    sliders[3]->setPopupDisplayEnabled (true, page);
    sliders[3]->setTextValueSuffix (" nuns required to change a lightbulb");

    sliders[4]->setSliderStyle (Slider::IncDecButtons);
    sliders[4]->setTextBoxStyle (Slider::TextBoxLeft, false, 50, 20);
    sliders[4]->setBounds (180, 105, 100, 20);
    sliders[4]->setIncDecButtonsMode (Slider::incDecButtonsDraggable_Vertical);

    sliders[5]->setSliderStyle (Slider::Rotary);
    sliders[5]->setRotaryParameters (float_Pi * 1.2f, float_Pi * 2.8f, false);
    sliders[5]->setTextBoxStyle (Slider::TextBoxRight, false, 70, 20);
    sliders[5]->setBounds (190, 145, 120, 40);
    sliders[5]->setTextValueSuffix (" mm");

    sliders[6]->setSliderStyle (Slider::LinearBar);
    sliders[6]->setBounds (180, 195, 100, 30);
    sliders[6]->setTextValueSuffix (" gallons");

    sliders[7]->setSliderStyle (Slider::TwoValueHorizontal);
    sliders[7]->setBounds (360, 20, 160, 40);

    sliders[8]->setSliderStyle (Slider::TwoValueVertical);
    sliders[8]->setBounds (360, 110, 40, 160);

    sliders[9]->setSliderStyle (Slider::ThreeValueHorizontal);
    sliders[9]->setBounds (360, 70, 160, 40);

    sliders[10]->setSliderStyle (Slider::ThreeValueVertical);
    sliders[10]->setBounds (440, 110, 40, 160);

    for (int i = 7; i <= 10; ++i)
    {
        sliders[i]->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        sliders[i]->setPopupDisplayEnabled (true, page);
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
        sliders[i]->getValueObject().referTo (sharedValue);

    // ..and now we'll do the same for all our min/max slider values..
    Value sharedValueMin, sharedValueMax;
    sharedValueMin = Random::getSystemRandom().nextDouble() * 40.0;
    sharedValueMax = Random::getSystemRandom().nextDouble() * 40.0 + 60.0;

    for (int i = 7; i <= 10; ++i)
    {
        sliders[i]->getMaxValueObject().referTo (sharedValueMax);
        sliders[i]->getMinValueObject().referTo (sharedValueMin);
    }

    // Create a description label...
    Label* label = new Label ("hint", "Try right-clicking on a slider for an options menu. \n\nAlso, holding down CTRL while dragging will turn on a slider's velocity-sensitive mode");
    label->setBounds (20, 245, 350, 150);
    page->addAndMakeVisible (label);

    return page;
}

//==============================================================================
static Component* createRadioButtonPage()
{
    DemoPageComp* page = new DemoPageComp();

    GroupComponent* group = new GroupComponent ("group", "radio buttons");
    group->setBounds (20, 20, 220, 140);
    page->addAndMakeVisible (group);

    for (int i = 0; i < 4; ++i)
    {
        ToggleButton* tb = new ToggleButton ("radio button #" + String (i + 1));
        page->addAndMakeVisible (tb);
        tb->setRadioGroupId (1234);
        tb->setBounds (45, 46 + i * 22, 180, 22);
        tb->setTooltip ("a set of mutually-exclusive radio buttons");

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

        DrawableButton* db = new DrawableButton (String (i + 5) + " points", DrawableButton::ImageAboveTextLabel);
        db->setImages (&normal, &over, 0);

        page->addAndMakeVisible (db);
        db->setClickingTogglesState (true);
        db->setRadioGroupId (23456);

        const int buttonSize = 50;
        db->setBounds (25 + i * buttonSize, 180, buttonSize, buttonSize);

        if (i == 0)
            db->setToggleState (true, dontSendNotification);
    }

    for (int i = 0; i < 4; ++i)
    {
        TextButton* tb = new TextButton ("button " + String (i + 1));

        page->addAndMakeVisible (tb);
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

    return page;
}

//==============================================================================
class ButtonsPage   : public Component,
                      public ButtonListener
{
public:
    ButtonsPage (ButtonListener* buttonListener)
    {
        //==============================================================================
        // create some drawables to use for our drawable buttons...
        DrawablePath normal, over;

        Path p;
        p.addStar (Point<float>(), 5, 20.0f, 50.0f, 0.2f);
        normal.setPath (p);
        normal.setFill (Colours::red);

        p.clear();
        p.addStar (Point<float>(), 7, 30.0f, 50.0f, 0.0f);
        over.setPath (p);
        over.setFill (Colours::pink);
        over.setStrokeFill (Colours::black);
        over.setStrokeThickness (5.0f);

        DrawableImage down;
        down.setImage (ImageCache::getFromMemory (BinaryData::juce_png, BinaryData::juce_pngSize));
        down.setOverlayColour (Colours::black.withAlpha (0.3f));

        //==============================================================================
        // create an image-above-text button from these drawables..
        DrawableButton* db = new DrawableButton ("Button 1", DrawableButton::ImageAboveTextLabel);
        db->setImages (&normal, &over, &down);
        db->setBounds (10, 30, 80, 80);
        db->setTooltip ("this is a DrawableButton with a label");
        addAndMakeVisible (db);

        //==============================================================================
        // create an image-only button from these drawables..
        db = new DrawableButton ("Button 2", DrawableButton::ImageFitted);
        db->setImages (&normal, &over, &down);
        db->setClickingTogglesState (true);
        db->setBounds (90, 30, 80, 80);
        db->setTooltip ("this is an image-only DrawableButton");
        db->addListener (buttonListener);
        addAndMakeVisible (db);

        //==============================================================================
        // create an image-on-button-shape button from the same drawables..
        db = new DrawableButton ("Button 3", DrawableButton::ImageOnButtonBackground);
        db->setImages (&normal, 0, 0);
        db->setBounds (200, 30, 110, 25);
        db->setTooltip ("this is a DrawableButton on a standard button background");
        addAndMakeVisible (db);

        //==============================================================================
        db = new DrawableButton ("Button 4", DrawableButton::ImageOnButtonBackground);
        db->setImages (&normal, &over, &down);
        db->setClickingTogglesState (true);
        db->setColour (DrawableButton::backgroundColourId, Colours::white);
        db->setColour (DrawableButton::backgroundOnColourId, Colours::yellow);
        db->setBounds (200, 70, 50, 50);
        db->setTooltip ("this is a DrawableButton on a standard button background");
        db->addListener (buttonListener);
        addAndMakeVisible (db);

        //==============================================================================
        HyperlinkButton* hyperlink
            = new HyperlinkButton ("this is a HyperlinkButton",
                                    URL ("http://www.juce.com"));

        hyperlink->setBounds (10, 130, 200, 24);
        addAndMakeVisible (hyperlink);

        //==============================================================================
        ImageButton* imageButton = new ImageButton ("imagebutton");
        addAndMakeVisible (imageButton);

        Image juceImage = ImageCache::getFromMemory (BinaryData::juce_png, BinaryData::juce_pngSize);
        imageButton->setImages (true, true, true,
                                juceImage, 0.7f, Colours::transparentBlack,
                                juceImage, 1.0f, Colours::transparentBlack,
                                juceImage, 1.0f, Colours::pink.withAlpha (0.8f),
                                0.5f);

        imageButton->setTopLeftPosition (10, 160);
        imageButton->setTooltip ("image button - showing alpha-channel hit-testing and colour overlay when clicked");

        //==============================================================================
        ColourChangeButton* colourChangeButton = new ColourChangeButton();
        addAndMakeVisible (colourChangeButton);
        colourChangeButton->setTopLeftPosition (350, 30);

        //==============================================================================
        animateButton = new TextButton ("click to animate...");
        animateButton->changeWidthToFitText (24);
        animateButton->setTopLeftPosition (350, 70);
        animateButton->addListener (this);
        addAndMakeVisible (animateButton);
    }

    ~ButtonsPage()
    {
        /* Deleting your child components indiscriminately using deleteAllChildren() is not recommended! It's much
           safer to make them embedded members or use ScopedPointers to automatically manage their lifetimes!

           In this demo, where we're throwing together a whole bunch of random components, it's simpler to do it
           like this, but don't treat this as an example of good practice!
        */
        deleteAllChildren();
    }

    void buttonClicked (Button*)
    {
        for (int i = getNumChildComponents(); --i >= 0;)
        {
            if (getChildComponent (i) != animateButton)
            {
                animator.animateComponent (getChildComponent (i),
                                           Rectangle<int> (Random::getSystemRandom().nextInt (getWidth() / 2),
                                                           Random::getSystemRandom().nextInt (getHeight() / 2),
                                                           60 + Random::getSystemRandom().nextInt (getWidth() / 3),
                                                           16 + Random::getSystemRandom().nextInt (getHeight() / 6)),
                                           Random::getSystemRandom().nextFloat(),
                                           500 + Random::getSystemRandom().nextInt (2000),
                                           false,
                                           Random::getSystemRandom().nextDouble(),
                                           Random::getSystemRandom().nextDouble());
            }
        }
    }

private:
    TextButton* animateButton;
    ComponentAnimator animator;
};


//==============================================================================
static Component* createMiscPage()
{
    DemoPageComp* page = new DemoPageComp();

    TextEditor* textEditor1 = new TextEditor();
    page->addAndMakeVisible (textEditor1);
    textEditor1->setBounds (10, 25, 200, 24);
    textEditor1->setText ("single-line text box");

    TextEditor* textEditor2 = new TextEditor ("password", (juce_wchar) 0x2022);
    page->addAndMakeVisible (textEditor2);
    textEditor2->setBounds (10, 55, 200, 24);
    textEditor2->setText ("password");

    //==============================================================================
    ComboBox* comboBox = new ComboBox ("combo");
    page->addAndMakeVisible (comboBox);
    comboBox->setBounds (300, 25, 200, 24);
    comboBox->setEditableText (true);
    comboBox->setJustificationType (Justification::centred);

    for (int i = 1; i < 100; ++i)
        comboBox->addItem ("combo box item " + String (i), i);

    comboBox->setSelectedId (1);

    DragOntoDesktopDemoComp* d = new DragOntoDesktopDemoComp (page);
    page->addAndMakeVisible (d);
    d->setBounds (20, 100, 200, 80);

    return page;
}

//==============================================================================
class ToolbarDemoComp   : public Component,
                          public SliderListener,
                          public ButtonListener
{
public:
    ToolbarDemoComp()
        : depthLabel (String::empty, "Toolbar depth:"),
          infoLabel (String::empty, "As well as showing off toolbars, this demo illustrates how to store "
                                    "a set of SVG files in a Zip file, embed that in your application, and read "
                                    "them back in at runtime.\n\nThe icon images here are taken from the open-source "
                                    "Tango icon project."),
          orientationButton ("Vertical/Horizontal"),
          customiseButton ("Customise...")
    {
        // Create and add the toolbar...
        addAndMakeVisible (&toolbar);

        // And use our item factory to add a set of default icons to it...
        toolbar.addDefaultItems (factory);

        // Now we'll just create the other sliders and buttons on the demo page, which adjust
        // the toolbar's properties...
        addAndMakeVisible (&infoLabel);
        infoLabel.setJustificationType (Justification::topLeft);
        infoLabel.setBounds (80, 80, 450, 100);
        infoLabel.setInterceptsMouseClicks (false, false);

        addAndMakeVisible (&depthSlider);
        depthSlider.setRange (10.0, 200.0, 1.0);
        depthSlider.setValue (50, dontSendNotification);
        depthSlider.setSliderStyle (Slider::LinearHorizontal);
        depthSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
        depthSlider.addListener (this);
        depthSlider.setBounds (80, 210, 300, 22);
        depthLabel.attachToComponent (&depthSlider, false);

        addAndMakeVisible (&orientationButton);
        orientationButton.addListener (this);
        orientationButton.changeWidthToFitText (22);
        orientationButton.setTopLeftPosition (depthSlider.getX(), depthSlider.getBottom() + 20);

        addAndMakeVisible (&customiseButton);
        customiseButton.addListener (this);
        customiseButton.changeWidthToFitText (22);
        customiseButton.setTopLeftPosition (orientationButton.getRight() + 20, orientationButton.getY());
    }

    void resized()
    {
        int toolbarThickness = (int) depthSlider.getValue();

        if (toolbar.isVertical())
            toolbar.setBounds (getLocalBounds().removeFromLeft (toolbarThickness));
        else
            toolbar.setBounds (getLocalBounds().removeFromTop  (toolbarThickness));
    }

    void sliderValueChanged (Slider*)
    {
        resized();
    }

    void buttonClicked (Button* button)
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

        void getAllToolbarItemIds (Array <int>& ids)
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

        void getDefaultItemSet (Array <int>& ids)
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

        ToolbarItemComponent* createItem (int itemId)
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
                case juceLogoButton:    return new ToolbarButton (itemId, "juce!", Drawable::createFromImageData (BinaryData::juce_png, BinaryData::juce_pngSize), 0);
                case customComboBox:    return new CustomToolbarComboBox (itemId);
                default:                break;
            }

            return 0;
        }

    private:
        StringArray iconNames;
        OwnedArray <Drawable> iconsFromZipFile;

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

                    if (svgFileStream != 0)
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
                addAndMakeVisible (&comboBox);

                for (int i = 1; i < 20; ++i)
                    comboBox.addItem ("Toolbar ComboBox item " + String (i), i);

                comboBox.setSelectedId (1);
                comboBox.setEditableText (true);
            }

            bool getToolbarItemSizes (int /*toolbarDepth*/, bool isVertical,
                                      int& preferredSize, int& minSize, int& maxSize)
            {
                if (isVertical)
                    return false;

                preferredSize = 250;
                minSize = 80;
                maxSize = 300;
                return true;
            }

            void paintButtonArea (Graphics&, int, int, bool, bool)
            {
            }

            void contentAreaChanged (const Rectangle<int>& newArea)
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
class DemoTabbedComponent  : public TabbedComponent,
                             public ButtonListener
{
public:
    DemoTabbedComponent()
        : TabbedComponent (TabbedButtonBar::TabsAtTop)
    {
        addTab ("sliders",       getRandomBrightColour(), createSlidersPage(),      true);
        addTab ("toolbars",      getRandomBrightColour(), new ToolbarDemoComp(),    true);
        addTab ("buttons",       getRandomBrightColour(), new ButtonsPage (this),   true);
        addTab ("radio buttons", getRandomBrightColour(), createRadioButtonPage(),  true);
        addTab ("misc widgets",  getRandomBrightColour(), createMiscPage(),         true);

        getTabbedButtonBar().getTabButton (2)->setExtraComponent (new CustomTabButton(), TabBarButton::afterText);
    }

    void buttonClicked (Button* button)
    {
        showBubbleMessage (button,
                           "This is a demo of the BubbleMessageComponent, which lets you pop up a message pointing "
                           "at a component or somewhere on the screen.\n\n"
                           "The message bubbles will disappear after a timeout period, or when the mouse is clicked.");
    }

    void showBubbleMessage (Component* targetComponent, const String& textToShow)
    {
        BubbleMessageComponent* bmc = new BubbleMessageComponent();

        if (Desktop::canUseSemiTransparentWindows())
        {
            bmc->setAlwaysOnTop (true);
            bmc->addToDesktop (0);
        }
        else
        {
            addChildComponent (bmc);
        }

        AttributedString text (textToShow);
        text.setJustification (Justification::centred);

        bmc->showAt (targetComponent, text, 2000, true, true);
    }

    static const Colour getRandomBrightColour()
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

        void paint (Graphics& g)
        {
            Path p;
            p.addStar (Point<float>(), 7, 1.0f, 2.0f);

            g.setColour (Colours::green);
            g.fillPath (p, RectanglePlacement (RectanglePlacement::centred)
                                .getTransformToFit (p.getBounds(), getLocalBounds().reduced (2).toFloat()));
        }

        void mouseDown (const MouseEvent&)
        {
            DemoTabbedComponent* dtc = findParentComponentOfClass<DemoTabbedComponent>();

            dtc->showBubbleMessage (this, "This is a custom tab component");
        }
    };
};


//==============================================================================
class DemoBackgroundThread  : public ThreadWithProgressWindow
{
public:
    DemoBackgroundThread()
        : ThreadWithProgressWindow ("busy doing some important things...",
                                    true,
                                    true)
    {
        setStatusMessage ("Getting ready...");
    }

    void run() override
    {
        setProgress (-1.0); // setting a value beyond the range 0 -> 1 will show a spinning bar..
        setStatusMessage ("Preparing to do some stuff...");
        wait (2000);

        const int thingsToDo = 10;

        for (int i = 0; i < thingsToDo; ++i)
        {
            // must check this as often as possible, because this is
            // how we know if the user's pressed 'cancel'
            if (threadShouldExit())
                return;

            // this will update the progress bar on the dialog box
            setProgress (i / (double) thingsToDo);

            setStatusMessage (String (thingsToDo - i) + " things left to do...");

            wait (500);
        }

        setProgress (-1.0); // setting a value beyond the range 0 -> 1 will show a spinning bar..
        setStatusMessage ("Finishing off the last few bits and pieces!");
        wait (2000);
    }
};

#if JUCE_MAC

//==============================================================================
/** This pops open a dialog box and waits for you to press keys on your Apple Remote,
    which it describes in the box.
*/
class AppleRemoteTestWindow  : public AlertWindow,
                               public AppleRemoteDevice
{
public:
    AppleRemoteTestWindow()
        : AlertWindow ("Apple Remote Control Test!",
                       "If you've got an Apple Remote, press some buttons now...",
                       AlertWindow::NoIcon)
    {
        addButton ("done", 0);

        // (To open the device in non-exclusive mode, pass 'false' in here)..
        if (! start (true))
            setMessage ("Couldn't open the remote control device!");
    }

    ~AppleRemoteTestWindow()
    {
        stop();
    }

    void buttonPressed (const ButtonType buttonId, const bool isDown)
    {
        setMessage (getDescriptionOfButtonType (buttonId) + (isDown ? " -- [down]"
                                                                    : " -- [up]"));
    }

    static String getDescriptionOfButtonType (const ButtonType type)
    {
        switch (type)
        {
            case menuButton:            return "menu button (short)";
            case playButton:            return "play button";
            case plusButton:            return "plus button";
            case minusButton:           return "minus button";
            case rightButton:           return "right button (short)";
            case leftButton:            return "left button (short)";
            case rightButton_Long:      return "right button (long)";
            case leftButton_Long:       return "left button (long)";
            case menuButton_Long:       return "menu button (long)";
            case playButtonSleepMode:   return "play (sleep mode)";
            case switched:              return "remote switched";
            default:                    return "unknown";
        }
    }
};

#endif

//==============================================================================
class WidgetsDemo  : public Component,
                     public ButtonListener,
                     public SliderListener
{
public:
    //==============================================================================
    WidgetsDemo()
        : menuButton ("click for a popup menu..",
                      "click for a demo of the different types of item you can put into a popup menu..."),
          enableButton ("enable/disable components")
    {
        setName ("Widgets");

        addAndMakeVisible (&tabs);

        //==============================================================================
        addAndMakeVisible (&menuButton);
        menuButton.setBounds (10, 10, 200, 24);
        menuButton.addListener (this);
        menuButton.setTriggeredOnMouseDown (true); // because this button pops up a menu, this lets us
                                                   // hold down the button and drag straight onto the menu

        //==============================================================================
        addAndMakeVisible (&enableButton);
        enableButton.setBounds (230, 10, 180, 24);
        enableButton.setTooltip ("Enables/disables all the components");
        enableButton.setToggleState (true, dontSendNotification);
        enableButton.addListener (this);

        addAndMakeVisible (&transformSlider);
        transformSlider.setSliderStyle (Slider::LinearBar);
        transformSlider.setTextValueSuffix (" degrees rotation");
        transformSlider.setRange (-180.0, 180.0, 0.1);
        transformSlider.setBounds (440, 10, 180, 24);
        transformSlider.setTooltip ("Applies a transform to the components");
        transformSlider.addListener (this);
    }

    ~WidgetsDemo()
    {
        PopupMenu::dismissAllActiveMenus();
    }

    void resized()
    {
        tabs.setBounds (10, 40, getWidth() - 20, getHeight() - 50);
    }

    //==============================================================================
    void buttonClicked (Button* button)
    {
        if (button == &enableButton)
        {
            const bool enabled = enableButton.getToggleState();

            menuButton.setEnabled (enabled);
            tabs.setEnabled (enabled);
        }
        else if (button == &menuButton)
        {
            PopupMenu m;
            m.addItem (1, "Normal item");
            m.addItem (2, "Disabled item", false);
            m.addItem (3, "Ticked item", true, true);
            m.addColouredItem (4, "Coloured item", Colours::green);
            m.addSeparator();
            m.addCustomItem (5, new CustomMenuComponent());
            m.addSeparator();

            PopupMenu tabsMenu;
            tabsMenu.addItem (1001, "Show tabs at the top", true, tabs.getOrientation() == TabbedButtonBar::TabsAtTop);
            tabsMenu.addItem (1002, "Show tabs at the bottom", true, tabs.getOrientation() == TabbedButtonBar::TabsAtBottom);
            tabsMenu.addItem (1003, "Show tabs at the left", true, tabs.getOrientation() == TabbedButtonBar::TabsAtLeft);
            tabsMenu.addItem (1004, "Show tabs at the right", true, tabs.getOrientation() == TabbedButtonBar::TabsAtRight);

            m.addSubMenu ("Tab position", tabsMenu);
            m.addSeparator();

            PopupMenu dialogMenu;
            dialogMenu.addItem (100, "Show a plain alert-window...");
            dialogMenu.addItem (101, "Show an alert-window with a 'warning' icon...");
            dialogMenu.addItem (102, "Show an alert-window with an 'info' icon...");
            dialogMenu.addItem (103, "Show an alert-window with a 'question' icon...");

            dialogMenu.addSeparator();
            dialogMenu.addItem (110, "Show an ok/cancel alert-window...");
            dialogMenu.addSeparator();
            dialogMenu.addItem (111, "Show an alert-window with some extra components...");
            dialogMenu.addSeparator();
            dialogMenu.addItem (112, "Show a ThreadWithProgressWindow demo...");

            m.addSubMenu ("AlertWindow demonstrations", dialogMenu);
            m.addSeparator();

            m.addItem (120, "Show a colour selector demo...");
            m.addSeparator();

           #if JUCE_MAC
            m.addItem (140, "Run the Apple Remote Control test...");
            m.addSeparator();
           #endif

            PopupMenu nativeFileChoosers;
            nativeFileChoosers.addItem (121, "'Load' file browser...");
            nativeFileChoosers.addItem (124, "'Load' file browser with an image file preview...");
            nativeFileChoosers.addItem (122, "'Save' file browser...");
            nativeFileChoosers.addItem (123, "'Choose directory' file browser...");

            PopupMenu juceFileChoosers;
            juceFileChoosers.addItem (131, "'Load' file browser...");
            juceFileChoosers.addItem (134, "'Load' file browser with an image file preview...");
            juceFileChoosers.addItem (132, "'Save' file browser...");
            juceFileChoosers.addItem (133, "'Choose directory' file browser...");

            PopupMenu fileChoosers;
            fileChoosers.addSubMenu ("Operating system dialogs", nativeFileChoosers);
            fileChoosers.addSubMenu ("Juce dialogs", juceFileChoosers);

            m.addSubMenu ("File chooser dialogs", fileChoosers);

            m.showMenuAsync (PopupMenu::Options().withTargetComponent (&menuButton),
                             ModalCallbackFunction::forComponent (menuItemChosenCallback, this));
        }
    }

    //==============================================================================
    // This gets called when our popup menu has an item selected or is dismissed.
    static void menuItemChosenCallback (int result, WidgetsDemo* demoComponent)
    {
        if (result != 0 && demoComponent != 0)
            demoComponent->performDemoMenuItem (result);
    }

    static void alertBoxResultChosen (int result, WidgetsDemo*)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                          "Alert Box",
                                          "Result code: " + String (result));
    }

    void performDemoMenuItem (int result)
    {
        if (result >= 100 && result < 105)
        {
            AlertWindow::AlertIconType icon = AlertWindow::NoIcon;

            switch (result)
            {
                case 101:  icon = AlertWindow::WarningIcon;  break;
                case 102:  icon = AlertWindow::InfoIcon;     break;
                case 103:  icon = AlertWindow::QuestionIcon; break;
            }

            AlertWindow::showMessageBoxAsync (icon,
                                              "This is an AlertWindow",
                                              "And this is the AlertWindow's message. Blah blah blah blah blah blah blah blah blah blah blah blah blah.",
                                              "ok");
        }
        else if (result == 110)
        {
            AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon,
                                          "This is an ok/cancel AlertWindow",
                                          "And this is the AlertWindow's message. Blah blah blah blah blah blah blah blah blah blah blah blah blah.",
                                          String::empty,
                                          String::empty,
                                          0,
                                          ModalCallbackFunction::forComponent (alertBoxResultChosen, this));
        }
        else if (result == 111)
        {
           #if JUCE_MODAL_LOOPS_PERMITTED
            AlertWindow w ("AlertWindow demo..",
                           "This AlertWindow has a couple of extra components added to show how to add drop-down lists and text entry boxes.",
                           AlertWindow::QuestionIcon);

            w.addTextEditor ("text", "enter some text here", "text field:");

            const char* options[] = { "option 1", "option 2", "option 3", "option 4", nullptr };
            w.addComboBox ("option", StringArray (options), "some options");

            w.addButton ("ok",     1, KeyPress (KeyPress::returnKey, 0, 0));
            w.addButton ("cancel", 0, KeyPress (KeyPress::escapeKey, 0, 0));

            if (w.runModalLoop() != 0) // is they picked 'ok'
            {
                // this is the item they chose in the drop-down list..
                const int optionIndexChosen = w.getComboBoxComponent ("option")->getSelectedItemIndex();
                (void) optionIndexChosen; // (just avoids a compiler warning about unused variables)


                // this is the text they entered..
                String text = w.getTextEditorContents ("text");

            }
           #endif
        }
        else if (result == 112)
        {
            DemoBackgroundThread demoThread;

           #if JUCE_MODAL_LOOPS_PERMITTED
            if (demoThread.runThread())
            {
                // thread finished normally..
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                  "Progress window",
                                                  "Thread finished ok!");
            }
            else
            {
                // user pressed the cancel button..
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                  "Progress window",
                                                  "You pressed cancel!");
            }
           #endif
        }
        else if (result == 120)
        {
            DialogWindow::LaunchOptions o;

            o.content.setOwned (new ColourSelector());
            o.content->setSize (400, 400);

            o.dialogTitle                   = "Colour Selector Demo";
            o.dialogBackgroundColour        = Colours::grey;
            o.escapeKeyTriggersCloseButton  = true;
            o.useNativeTitleBar             = false;
            o.resizable                     = true;

            o.launchAsync();
        }
        else if (result == 140)
        {
           #if JUCE_MAC
            AppleRemoteTestWindow test;
            test.runModalLoop();
           #endif
        }
        else if (result >= 121 && result < 139)
        {
           #if JUCE_MODAL_LOOPS_PERMITTED
            const bool useNativeVersion = result < 130;
            if (result > 130)
                result -= 10;

            if (result == 121)
            {
                FileChooser fc ("Choose a file to open...",
                                File::getCurrentWorkingDirectory(),
                                "*",
                                useNativeVersion);

                if (fc.browseForMultipleFilesToOpen())
                {
                    String chosen;
                    for (int i = 0; i < fc.getResults().size(); ++i)
                        chosen << fc.getResults().getReference(i).getFullPathName() << "\n";

                    AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                      "File Chooser...",
                                                      "You picked: " + chosen);
                }
            }
            else if (result == 124)
            {
                ImagePreviewComponent imagePreview;
                imagePreview.setSize (200, 200);

                FileChooser fc ("Choose an image to open...",
                                File::getCurrentWorkingDirectory(),
                                "*.jpg;*.jpeg;*.png;*.gif",
                                useNativeVersion);

                if (fc.browseForMultipleFilesToOpen (&imagePreview))
                {
                    String chosen;
                    for (int i = 0; i < fc.getResults().size(); ++i)
                        chosen << fc.getResults().getReference(i).getFullPathName() << "\n";

                    AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                      "File Chooser...",
                                                      "You picked: " + chosen);
                }
            }
            else if (result == 122)
            {
                FileChooser fc ("Choose a file to save...",
                                File::getCurrentWorkingDirectory(),
                                "*",
                                useNativeVersion);

                if (fc.browseForFileToSave (true))
                {
                    File chosenFile = fc.getResult();

                    AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                      "File Chooser...",
                                                      "You picked: " + chosenFile.getFullPathName());
                }
            }
            else if (result == 123)
            {
                FileChooser fc ("Choose a directory...",
                                File::getCurrentWorkingDirectory(),
                                "*",
                                useNativeVersion);

                if (fc.browseForDirectory())
                {
                    File chosenDirectory = fc.getResult();

                    AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                      "File Chooser...",
                                                      "You picked: " + chosenDirectory.getFullPathName());
                }
            }
           #endif
        }
        else if (result == 1001)
        {
            tabs.setOrientation (TabbedButtonBar::TabsAtTop);
        }
        else if (result == 1002)
        {
            tabs.setOrientation (TabbedButtonBar::TabsAtBottom);
        }
        else if (result == 1003)
        {
            tabs.setOrientation (TabbedButtonBar::TabsAtLeft);
        }
        else if (result == 1004)
        {
            tabs.setOrientation (TabbedButtonBar::TabsAtRight);
        }
    }

    void sliderValueChanged (Slider*)
    {
        // When you move the rotation slider, we'll apply a rotaion transform to the whole tabs component..
        tabs.setTransform (AffineTransform::rotation ((float) (transformSlider.getValue() / (180.0 / double_Pi)),
                                                      getWidth() * 0.5f, getHeight() * 0.5f));
    }

private:
    TextButton menuButton;
    ToggleButton enableButton;
    Slider transformSlider;
    DemoTabbedComponent tabs;
};


//==============================================================================
Component* createWidgetsDemo()
{
    return new WidgetsDemo();
}
