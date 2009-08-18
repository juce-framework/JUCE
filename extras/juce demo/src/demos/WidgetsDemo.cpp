/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../jucedemo_headers.h"


//==============================================================================
class BouncingBallComponent : public Component,
                              public Timer
{
    Colour colour;
    float x, y, dx, dy;

public:
    BouncingBallComponent()
    {
        x = Random::getSystemRandom().nextFloat() * 100.0f;
        y = Random::getSystemRandom().nextFloat() * 100.0f;

        dx = Random::getSystemRandom().nextFloat() * 8.0f - 4.0f;
        dy = Random::getSystemRandom().nextFloat() * 8.0f - 4.0f;

        colour = Colour (Random::getSystemRandom().nextInt())
                    .withAlpha (0.5f)
                    .withBrightness (0.7f);

        int size = 10 + Random::getSystemRandom().nextInt (30);
        setSize (size, size);

        startTimer (60);
    }

    ~BouncingBallComponent()
    {
    }

    void paint (Graphics& g)
    {
        g.setColour (colour);
        g.fillEllipse (x - getX(), y - getY(), getWidth() - 2.0f, getHeight() - 2.0f);
    }

    void timerCallback()
    {
        x += dx;
        y += dy;

        if (x < 0)
            dx = fabsf (dx);

        if (x > getParentWidth())
            dx = -fabsf (dx);

        if (y < 0)
            dy = fabsf (dy);

        if (y > getParentHeight())
            dy = -fabsf (dy);

        setTopLeftPosition ((int) x, (int) y);
    }

    bool hitTest (int x, int y)
    {
        return false;
    }
};

//==============================================================================
class DragOntoDesktopDemoComp : public Component
{
    Component* parent;
    ComponentDragger dragger;

public:
    DragOntoDesktopDemoComp (Component* p)
        : parent (p)
    {
        // show off semi-transparency if it's supported by the current OS.
        setOpaque (! Desktop::canUseSemiTransparentWindows());

        for (int i = 3; --i >= 0;)
            addAndMakeVisible (new BouncingBallComponent());
    }

    ~DragOntoDesktopDemoComp()
    {
        deleteAllChildren();
    }

    void mouseDown (const MouseEvent& e)
    {
        dragger.startDraggingComponent (this, 0);
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (! parent->isValidComponent())
        {
            delete this;
        }
        else
        {
            MouseEvent e2 (e.getEventRelativeTo (parent));

            // if the mouse is inside the parent component, we'll make that the
            // parent - otherwise, we'll put this comp on the desktop.
            if (e2.x >= 0 && e2.y >= 0 && e2.x < parent->getWidth() && e2.y < parent->getHeight())
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

            dragger.dragComponent (this, e);
        }
    }

    void paint (Graphics& g)
    {
        if (isOpaque())
            g.fillAll (Colours::white);
        else
            g.fillAll (Colours::blue.withAlpha (0.2f));

        String desc (T("drag this box onto the desktop to show how the same component can move from being lightweight to being a separate window"));

        g.setFont (15.0f);
        g.setColour (Colours::black);
        g.drawFittedText (desc, 4, 0, getWidth() - 8, getHeight(), Justification::horizontallyJustified, 5);

        g.drawRect (0, 0, getWidth(), getHeight());
    }
};

//==============================================================================
class CustomMenuComponent  : public PopupMenuCustomComponent,
                             public Timer
{
    int blobX, blobY;

public:
    CustomMenuComponent()
        : blobX (0),
          blobY (0)
    {
        // set off a timer to move a blob around on this component every
        // 300 milliseconds - see the timerCallback() method.
        startTimer (300);
    }

    ~CustomMenuComponent()
    {
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
        g.fillEllipse ((float) blobX, (float) blobY, 30.0f, 40.0f);

        g.setFont (14.0f, Font::italic);
        g.setColour (Colours::black);

        g.drawFittedText (T("this is a customised menu item (also demonstrating the Timer class)..."),
                          4, 0, getWidth() - 8, getHeight(),
                          Justification::centred, 3);
    }

    void timerCallback()
    {
        blobX = Random::getSystemRandom().nextInt (getWidth());
        blobY = Random::getSystemRandom().nextInt (getHeight());
        repaint();
    }
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

    double snapValue (double attemptedValue, const bool userIsDragging)
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
        : TextButton (T("click to change colour..."))
    {
        setSize (10, 24);
        changeWidthToFitText();
    }

    ~ColourChangeButton()
    {
    }

    void clicked()
    {
        // create two colour selector components for our background and
        // text colour..
        ColourSelector colourSelector1;
        colourSelector1.setName (T("background"));
        colourSelector1.setCurrentColour (findColour (TextButton::buttonColourId));
        colourSelector1.addChangeListener (this);

        ColourSelector colourSelector2;
        colourSelector2.setName (T("text"));
        colourSelector2.setCurrentColour (findColour (TextButton::textColourId));
        colourSelector2.addChangeListener (this);

        // and add the selectors as custom menu items to a PopupMenu, putting
        // them in two different sub-menus..
        PopupMenu m, sub1, sub2;

        sub1.addCustomItem (1234, &colourSelector1, 300, 300, false);
        m.addSubMenu (T("background colour"), sub1);

        sub2.addCustomItem (1234, &colourSelector2, 300, 300, false);
        m.addSubMenu (T("text colour"), sub2);

        // and show the menu (modally)..
        m.showAt (this);
    }

    void changeListenerCallback (void* source)
    {
        ColourSelector* cs = (ColourSelector*) source;

        if (cs->getName() == T("text"))
            setColour (TextButton::textColourId, cs->getCurrentColour());
        else
            setColour (TextButton::buttonColourId, cs->getCurrentColour());
    }
};

//==============================================================================
// just a component that deletes all its children, to use for the tabbed pages to avoid
// memory leaks when they're deleted
class DemoPageComp  : public Component
{
public:
    DemoPageComp()
    {
    }

    ~DemoPageComp()
    {
        deleteAllChildren();
    }
};

//==============================================================================
static Component* createSlidersPage()
{
    DemoPageComp* page = new DemoPageComp();

    const int numSliders = 11;
    Slider* sliders [numSliders];

    int i;
    for (i = 0; i < numSliders; ++i)
    {
        if (i == 2)
            page->addAndMakeVisible (sliders[i] = new SnappingSlider (T("slider")));
        else
            page->addAndMakeVisible (sliders[i] = new Slider (T("slider")));

        sliders[i]->setRange (0.0, 100.0, 0.1);
        sliders[i]->setPopupMenuEnabled (true);
        sliders[i]->setValue (Random::getSystemRandom().nextDouble() * 100.0, false, false);
    }

    sliders[0]->setSliderStyle (Slider::LinearVertical);
    sliders[0]->setTextBoxStyle (Slider::TextBoxBelow, false, 100, 20);
    sliders[0]->setBounds (10, 25, 70, 200);
    sliders[0]->setDoubleClickReturnValue (true, 50.0); // double-clicking this slider will set it to 50.0
    sliders[0]->setTextValueSuffix (T(" units"));

    sliders[1]->setSliderStyle (Slider::LinearVertical);
    sliders[1]->setVelocityBasedMode (true);
    sliders[1]->setSkewFactor (0.5);
    sliders[1]->setTextBoxStyle (Slider::TextBoxAbove, true, 100, 20);
    sliders[1]->setBounds (85, 25, 70, 200);
    sliders[1]->setTextValueSuffix (T(" rels"));

    sliders[2]->setSliderStyle (Slider::LinearHorizontal);
    sliders[2]->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
    sliders[2]->setBounds (180, 35, 150, 20);

    sliders[3]->setSliderStyle (Slider::LinearHorizontal);
    sliders[3]->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    sliders[3]->setBounds (180, 65, 150, 20);
    sliders[3]->setPopupDisplayEnabled (true, page);
    sliders[3]->setTextValueSuffix (T(" nuns required to change a lightbulb"));

    sliders[4]->setSliderStyle (Slider::IncDecButtons);
    sliders[4]->setTextBoxStyle (Slider::TextBoxLeft, false, 50, 20);
    sliders[4]->setBounds (180, 105, 100, 20);
    sliders[4]->setIncDecButtonsMode (Slider::incDecButtonsDraggable_Vertical);

    sliders[5]->setSliderStyle (Slider::Rotary);
    sliders[5]->setRotaryParameters (float_Pi * 1.2f, float_Pi * 2.8f, false);
    sliders[5]->setTextBoxStyle (Slider::TextBoxRight, false, 70, 20);
    sliders[5]->setBounds (190, 145, 120, 40);
    sliders[5]->setTextValueSuffix (T(" mm"));

    sliders[6]->setSliderStyle (Slider::LinearBar);
    sliders[6]->setBounds (180, 195, 100, 30);
    sliders[6]->setTextValueSuffix (T(" gallons"));

    sliders[7]->setSliderStyle (Slider::TwoValueHorizontal);
    sliders[7]->setBounds (360, 20, 160, 40);

    sliders[8]->setSliderStyle (Slider::TwoValueVertical);
    sliders[8]->setBounds (360, 110, 40, 160);

    sliders[9]->setSliderStyle (Slider::ThreeValueHorizontal);
    sliders[9]->setBounds (360, 70, 160, 40);

    sliders[10]->setSliderStyle (Slider::ThreeValueVertical);
    sliders[10]->setBounds (440, 110, 40, 160);

    for (i = 7; i <= 10; ++i)
    {
        sliders[i]->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        sliders[i]->setMinValue (Random::getSystemRandom().nextDouble() * 100.0, false, false);
        sliders[i]->setMaxValue (Random::getSystemRandom().nextDouble() * 100.0, false, false);
        sliders[i]->setPopupDisplayEnabled (true, page);
    }

    Label* label = new Label (T("hint"), T("Try right-clicking on a slider for an options menu. \n\nAlso, holding down CTRL while dragging will turn on a slider's velocity-sensitive mode"));
    label->setBounds (20, 245, 350, 150);
    page->addAndMakeVisible (label);

    return page;
}

//==============================================================================
static Component* createRadioButtonPage()
{
    DemoPageComp* page = new DemoPageComp();

    GroupComponent* group = new GroupComponent (T("group"), T("radio buttons"));
    group->setBounds (20, 20, 220, 140);
    page->addAndMakeVisible (group);

    int i;
    for (i = 0; i < 4; ++i)
    {
        ToggleButton* tb = new ToggleButton (T("radio button #") + String (i + 1));
        page->addAndMakeVisible (tb);
        tb->setRadioGroupId (1234);
        tb->setBounds (45, 46 + i * 22, 180, 22);
        tb->setTooltip (T("a set of mutually-exclusive radio buttons"));

        if (i == 0)
            tb->setToggleState (true, false);
    }

    for (i = 0; i < 4; ++i)
    {
        DrawablePath normal, over;

        Path p;
        p.addStar (0.0f, 0.0f, i + 5, 20.0f, 50.0f, -0.2f);
        normal.setPath (p);
        normal.setSolidFill (Colours::lightblue);
        normal.setOutline (4.0f, Colours::black);

        over.setPath (p);
        over.setSolidFill (Colours::blue);
        over.setOutline (4.0f, Colours::black);

        DrawableButton* db = new DrawableButton (String (i + 5) + T(" points"), DrawableButton::ImageAboveTextLabel);
        db->setImages (&normal, &over, 0);

        page->addAndMakeVisible (db);
        db->setClickingTogglesState (true);
        db->setRadioGroupId (23456);

        const int buttonSize = 50;
        db->setBounds (25 + i * buttonSize, 180, buttonSize, buttonSize);

        if (i == 0)
            db->setToggleState (true, false);
    }

    for (i = 0; i < 4; ++i)
    {
        TextButton* tb = new TextButton (T("button ") + String (i + 1));

        page->addAndMakeVisible (tb);
        tb->setClickingTogglesState (true);
        tb->setRadioGroupId (34567);
        tb->setColour (TextButton::buttonColourId, Colours::white);
        tb->setColour (TextButton::buttonOnColourId, Colours::blueviolet.brighter());

        tb->setBounds (20 + i * 55, 260, 55, 24);
        tb->setConnectedEdges (((i != 0) ? Button::ConnectedOnLeft : 0)
                                | ((i != 3) ? Button::ConnectedOnRight : 0));

        if (i == 0)
            tb->setToggleState (true, false);
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
        p.addStar (0.0f, 0.0f, 5, 20.0f, 50.0f, 0.2f);
        normal.setPath (p);
        normal.setSolidFill (Colours::red);

        p.clear();
        p.addStar (0.0f, 0.0f, 7, 30.0f, 50.0f, 0.0f);
        over.setPath (p);
        over.setSolidFill (Colours::pink);
        over.setOutline (5.0f, Colours::black);

        DrawableImage down;
        down.setImage (ImageCache::getFromMemory (BinaryData::juce_png, BinaryData::juce_pngSize), true);
        down.setOverlayColour (Colours::black.withAlpha (0.3f));

        //==============================================================================
        // create an image-above-text button from these drawables..
        DrawableButton* db = new DrawableButton (T("Button 1"), DrawableButton::ImageAboveTextLabel);
        db->setImages (&normal, &over, &down);

        addAndMakeVisible (db);
        db->setBounds (10, 30, 80, 80);
        db->setTooltip (T("this is a DrawableButton with a label"));

        //==============================================================================
        // create an image-only button from these drawables..
        db = new DrawableButton (T("Button 2"), DrawableButton::ImageFitted);
        db->setImages (&normal, &over, &down);
        db->setClickingTogglesState (true);

        addAndMakeVisible (db);
        db->setBounds (90, 30, 80, 80);
        db->setTooltip (T("this is an image-only DrawableButton"));
        db->addButtonListener (buttonListener);

        //==============================================================================
        // create an image-on-button-shape button from the same drawables..
        db = new DrawableButton (T("Button 3"), DrawableButton::ImageOnButtonBackground);
        db->setImages (&normal, 0, 0);

        addAndMakeVisible (db);
        db->setBounds (200, 30, 110, 25);
        db->setTooltip (T("this is a DrawableButton on a standard button background"));

        //==============================================================================
        db = new DrawableButton (T("Button 4"), DrawableButton::ImageOnButtonBackground);
        db->setImages (&normal, &over, &down);
        db->setClickingTogglesState (true);
        db->setBackgroundColours (Colours::white, Colours::yellow);

        addAndMakeVisible (db);
        db->setBounds (200, 70, 50, 50);
        db->setTooltip (T("this is a DrawableButton on a standard button background"));
        db->addButtonListener (buttonListener);

        //==============================================================================
        HyperlinkButton* hyperlink
            = new HyperlinkButton (T("this is a HyperlinkButton"),
                                    URL (T("http://www.rawmaterialsoftware.com/juce")));

        hyperlink->setBounds (10, 130, 200, 24);
        addAndMakeVisible (hyperlink);

        //==============================================================================
        ImageButton* imageButton = new ImageButton (T("imagebutton"));
        addAndMakeVisible (imageButton);

        Image* juceImage = ImageCache::getFromMemory (BinaryData::juce_png, BinaryData::juce_pngSize);
        ImageCache::incReferenceCount (juceImage);
        ImageCache::incReferenceCount (juceImage);

        imageButton->setImages (true, true, true,
                                juceImage, 0.7f, Colours::transparentBlack,
                                juceImage, 1.0f, Colours::transparentBlack,
                                juceImage, 1.0f, Colours::pink.withAlpha (0.8f),
                                0.5f);

        imageButton->setTopLeftPosition (10, 160);
        imageButton->setTooltip (T("image button - showing alpha-channel hit-testing and colour overlay when clicked"));

        //==============================================================================
        ColourChangeButton* colourChangeButton = new ColourChangeButton();
        addAndMakeVisible (colourChangeButton);
        colourChangeButton->setTopLeftPosition (350, 30);

        //==============================================================================
        animateButton = new TextButton (T("click to animate..."));
        addAndMakeVisible (animateButton);
        animateButton->changeWidthToFitText (24);
        animateButton->setTopLeftPosition (350, 70);
        animateButton->addButtonListener (this);
    }

    ~ButtonsPage()
    {
        deleteAllChildren();
    }

    void buttonClicked (Button*)
    {
        for (int i = getNumChildComponents(); --i >= 0;)
        {
            if (getChildComponent (i) != animateButton)
            {
                animator.animateComponent (getChildComponent (i),
                                           Rectangle (Random::getSystemRandom().nextInt (getWidth() / 2),
                                                      Random::getSystemRandom().nextInt (getHeight() / 2),
                                                      60 + Random::getSystemRandom().nextInt (getWidth() / 3),
                                                      16 + Random::getSystemRandom().nextInt (getHeight() / 6)),
                                           500 + Random::getSystemRandom().nextInt (2000),
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

    TextEditor* textEditor = new TextEditor();
    page->addAndMakeVisible (textEditor);
    textEditor->setBounds (10, 25, 200, 24);
    textEditor->setText (T("single-line text box"));

    textEditor = new TextEditor (T("password"), (tchar) 0x2022);
    page->addAndMakeVisible (textEditor);
    textEditor->setBounds (10, 55, 200, 24);
    textEditor->setText (T("password"));

    //==============================================================================
    ComboBox* comboBox = new ComboBox (T("combo"));
    page->addAndMakeVisible (comboBox);
    comboBox->setBounds (300, 25, 200, 24);
    comboBox->setEditableText (true);
    comboBox->setJustificationType (Justification::centred);

    int i;
    for (i = 1; i < 100; ++i)
        comboBox->addItem (T("combo box item ") + String (i), i);

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
    ToolbarDemoComp (ApplicationCommandManager* commandManager)
    {
        // Create and add the toolbar...
        addAndMakeVisible (toolbar = new Toolbar());

        // And use our item factory to add a set of default icons to it...
        toolbar->addDefaultItems (factory);

        // Now we'll just create the other sliders and buttons on the demo page, which adjust
        // the toolbar's properties...
        Label* info = new Label (String::empty,
            "As well as showing off toolbars, this demo illustrates how to store "
            "a set of SVG files in a Zip file, embed that in your application, and read "
            "them back in at runtime.\n\nThe icon images here are taken from the open-source "
            "Tango icon project.");

        addAndMakeVisible (info);
        info->setJustificationType (Justification::topLeft);
        info->setBounds (80, 80, 450, 100);
        info->setInterceptsMouseClicks (false, false);

        addAndMakeVisible (depthSlider = new Slider (T("toolbar depth:")));
        depthSlider->setRange (10.0, 200.0, 1.0);
        depthSlider->setValue (50, false);
        depthSlider->setSliderStyle (Slider::LinearHorizontal);
        depthSlider->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
        depthSlider->addListener (this);
        depthSlider->setBounds (80, 210, 300, 22);
        (new Label (depthSlider->getName(), depthSlider->getName()))->attachToComponent (depthSlider, false);

        addAndMakeVisible (orientationButton = new TextButton (T("vertical/horizontal")));
        orientationButton->addButtonListener (this);
        orientationButton->changeWidthToFitText (22);
        orientationButton->setTopLeftPosition (depthSlider->getX(), depthSlider->getBottom() + 20);

        addAndMakeVisible (customiseButton = new TextButton (T("customise...")));
        customiseButton->addButtonListener (this);
        customiseButton->changeWidthToFitText (22);
        customiseButton->setTopLeftPosition (orientationButton->getRight() + 20, orientationButton->getY());
    }

    ~ToolbarDemoComp()
    {
        deleteAllChildren();
    }

    void resized()
    {
        if (toolbar->isVertical())
            toolbar->setBounds (0, 0, (int) depthSlider->getValue(), getHeight());
        else
            toolbar->setBounds (0, 0, getWidth(), (int) depthSlider->getValue());
    }

    void sliderValueChanged (Slider* slider)
    {
        resized();
    }

    void buttonClicked (Button* button)
    {
        if (button == orientationButton)
        {
            toolbar->setVertical (! toolbar->isVertical());
            resized();
        }
        else if (button == customiseButton)
        {
            toolbar->showCustomisationDialog (factory);
        }
    }

private:
    Toolbar* toolbar;
    Slider* depthSlider;
    TextButton* orientationButton;
    TextButton* customiseButton;

    //==============================================================================
    class DemoToolbarItemFactory   : public ToolbarItemFactory
    {
    public:
        DemoToolbarItemFactory() {}
        ~DemoToolbarItemFactory() {}

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

        ToolbarItemComponent* createItem (const int itemId)
        {
            switch (itemId)
            {
            case doc_new:
                return createButtonFromZipFileSVG (itemId, T("new"), T("document-new.svg"));

            case doc_open:
                return createButtonFromZipFileSVG (itemId, T("open"), T("document-open.svg"));

            case doc_save:
                return createButtonFromZipFileSVG (itemId, T("save"), T("document-save.svg"));

            case doc_saveAs:
                return createButtonFromZipFileSVG (itemId, T("save as"), T("document-save-as.svg"));

            case edit_copy:
                return createButtonFromZipFileSVG (itemId, T("copy"), T("edit-copy.svg"));

            case edit_cut:
                return createButtonFromZipFileSVG (itemId, T("cut"), T("edit-cut.svg"));

            case edit_paste:
                return createButtonFromZipFileSVG (itemId, T("paste"), T("edit-paste.svg"));

            case juceLogoButton:
                return new ToolbarButton (itemId, T("juce!"), Drawable::createFromImageData (BinaryData::juce_png, BinaryData::juce_pngSize), 0);

            case customComboBox:
                return new CustomToolbarComboBox (itemId);

            default:
                break;
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
                    InputStream* svgFileStream = icons.createStreamForEntry (i);

                    if (svgFileStream != 0)
                    {
                        iconNames.add (icons.getEntry(i)->filename);
                        iconsFromZipFile.add (Drawable::createFromImageDataStream (*svgFileStream));

                        delete svgFileStream;
                    }
                }
            }

            Drawable* image = iconsFromZipFile [iconNames.indexOf (filename)]->createCopy();
            return new ToolbarButton (itemId, text, image, 0);

            return 0;
        }

        // Demonstrates how to put a custom component into a toolbar - this one contains
        // a ComboBox.
        class CustomToolbarComboBox : public ToolbarItemComponent
        {
        public:
            CustomToolbarComboBox (const int toolbarItemId)
                : ToolbarItemComponent (toolbarItemId, T("Custom Toolbar Item"), false)
            {
                addAndMakeVisible (comboBox = new ComboBox (T("demo toolbar combo box")));

                for (int i = 1; i < 20; ++i)
                    comboBox->addItem (T("Toolbar ComboBox item ") + String (i), i);

                comboBox->setSelectedId (1);
                comboBox->setEditableText (true);
            }

            ~CustomToolbarComboBox()
            {
                delete comboBox;
            }

            bool getToolbarItemSizes (int toolbarDepth,
                                      bool isToolbarVertical,
                                      int& preferredSize, int& minSize, int& maxSize)
            {
                if (isToolbarVertical)
                    return false;

                preferredSize = 250;
                minSize = 80;
                maxSize = 300;
                return true;
            }

            void paintButtonArea (Graphics&, int, int, bool, bool)
            {
            }

            void contentAreaChanged (const Rectangle& contentArea)
            {
                comboBox->setSize (contentArea.getWidth() - 2,
                                   jmin (contentArea.getHeight() - 2, 22));

                comboBox->setCentrePosition (contentArea.getCentreX(), contentArea.getCentreY());
            }

        private:
            ComboBox* comboBox;
        };
    };

    DemoToolbarItemFactory factory;
};

//==============================================================================
class DemoTabbedComponent  : public TabbedComponent,
                             public ButtonListener
{
public:
    DemoTabbedComponent (ApplicationCommandManager* commandManager)
        : TabbedComponent (TabbedButtonBar::TabsAtTop)
    {
        addTab (T("sliders"),       getRandomBrightColour(), createSlidersPage(),      true);
        addTab (T("toolbars"),      getRandomBrightColour(), new ToolbarDemoComp (commandManager),    true);
        addTab (T("buttons"),       getRandomBrightColour(), new ButtonsPage (this),   true);
        addTab (T("radio buttons"), getRandomBrightColour(), createRadioButtonPage(),  true);
        addTab (T("misc widgets"),  getRandomBrightColour(), createMiscPage(),         true);
    }

    ~DemoTabbedComponent()
    {
    }

    void buttonClicked (Button* button)
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

        bmc->showAt (button, T("This is a demo of the BubbleMessageComponent, which lets you pop up a message pointing at a component or somewhere on the screen.\n\nThe message bubbles will disappear after a timeout period, or when the mouse is clicked."),
                     2000, true, true);
    }

    static const Colour getRandomBrightColour()
    {
        return Colour (Random::getSystemRandom().nextFloat(), 0.1f, 0.97f, 1.0f);
    }
};


//==============================================================================
class DemoBackgroundThread  : public ThreadWithProgressWindow
{
public:
    DemoBackgroundThread()
        : ThreadWithProgressWindow (T("busy doing some important things..."),
                                    true,
                                    true)
    {
        setStatusMessage (T("Getting ready..."));
    }

    ~DemoBackgroundThread()
    {
    }

    void run()
    {
        setProgress (-1.0); // setting a value beyond the range 0 -> 1 will show a spinning bar..
        setStatusMessage (T("Preparing to do some stuff..."));
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

            setStatusMessage (String (thingsToDo - i) + T(" things left to do..."));

            wait (500);
        }

        setProgress (-1.0); // setting a value beyond the range 0 -> 1 will show a spinning bar..
        setStatusMessage (T("Finishing off the last few bits and pieces!"));
        wait (2000);
    }
};

//==============================================================================
/** A DialogWindow containing a ColourSelector component */
class ColourSelectorDialogWindow  : public DialogWindow
{
public:
    ColourSelectorDialogWindow()
        : DialogWindow (T("Colour selector demo"),
                        Colours::lightgrey,
                        true)
    {
        setContentComponent (new ColourSelector());
        centreWithSize (400, 400);
        setResizable (true, true);
    }

    ~ColourSelectorDialogWindow()
    {
    }

    void closeButtonPressed()
    {
        // we expect this component to be run within a modal loop, so when the close
        // button is clicked, we can make it invisible to cause the loop to exit and the
        // calling code will delete this object.
        setVisible (false);
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
        addButton (T("done"), 0);

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
        String desc;

        switch (buttonId)
        {
        case menuButton:
            desc = "menu button (short)";
            break;
        case playButton:
            desc = "play button";
            break;
        case plusButton:
            desc = "plus button";
            break;
        case minusButton:
            desc = "minus button";
            break;
        case rightButton:
            desc = "right button (short)";
            break;
        case leftButton:
            desc = "left button (short)";
            break;
        case rightButton_Long:
            desc = "right button (long)";
            break;
        case leftButton_Long:
            desc = "left button (long)";
            break;
        case menuButton_Long:
            desc = "menu button (long)";
            break;
        case playButtonSleepMode:
            desc = "play (sleep mode)";
            break;
        case switched:
            desc = "remote switched";
            break;
        }

        if (isDown)
            desc << " -- [down]";
        else
            desc << " -- [up]";

        setMessage (desc);
    }
};

#endif

//==============================================================================
const int numGroups = 4;

class WidgetsDemo  : public Component,
                     public ButtonListener
{
    TextButton* menuButton;
    ToggleButton* enableButton;

    DemoTabbedComponent* tabs;

public:
    //==============================================================================
    WidgetsDemo (ApplicationCommandManager* commandManager)
    {
        setName (T("Widgets"));

        addAndMakeVisible (tabs = new DemoTabbedComponent (commandManager));

        //==============================================================================
        menuButton = new TextButton (T("click for a popup menu.."),
                                     T("click for a demo of the different types of item you can put into a popup menu..."));

        addAndMakeVisible (menuButton);
        menuButton->setBounds (10, 10, 200, 24);
        menuButton->addButtonListener (this);
        menuButton->setTriggeredOnMouseDown (true); // because this button pops up a menu, this lets us
                                                    // hold down the button and drag straight onto the menu

        //==============================================================================
        enableButton = new ToggleButton (T("enable/disable components"));
        addAndMakeVisible (enableButton);
        enableButton->setBounds (230, 10, 180, 24);
        enableButton->setTooltip (T("toggle button"));
        enableButton->setToggleState (true, false);
        enableButton->addButtonListener (this);
    }

    ~WidgetsDemo()
    {
        deleteAllChildren();
    }

    void resized()
    {
        tabs->setBounds (10, 40, getWidth() - 20, getHeight() - 50);
    }

    //==============================================================================
    void buttonClicked (Button* button)
    {
        if (button == enableButton)
        {
            const bool enabled = enableButton->getToggleState();

            menuButton->setEnabled (enabled);
            tabs->setEnabled (enabled);
        }
        else if (button == menuButton)
        {
            PopupMenu m;
            m.addItem (1, T("Normal item"));
            m.addItem (2, T("Disabled item"), false);
            m.addItem (3, T("Ticked item"), true, true);
            m.addColouredItem (4, T("Coloured item"), Colours::green);
            m.addSeparator();
            m.addCustomItem (5, new CustomMenuComponent());

            m.addSeparator();

            PopupMenu tabsMenu;
            tabsMenu.addItem (1001, T("Show tabs at the top"), true, tabs->getOrientation() == TabbedButtonBar::TabsAtTop);
            tabsMenu.addItem (1002, T("Show tabs at the bottom"), true, tabs->getOrientation() == TabbedButtonBar::TabsAtBottom);
            tabsMenu.addItem (1003, T("Show tabs at the left"), true, tabs->getOrientation() == TabbedButtonBar::TabsAtLeft);
            tabsMenu.addItem (1004, T("Show tabs at the right"), true, tabs->getOrientation() == TabbedButtonBar::TabsAtRight);
            m.addSubMenu (T("Tab position"), tabsMenu);

            m.addSeparator();

            PopupMenu dialogMenu;
            dialogMenu.addItem (100, T("Show a plain alert-window..."));
            dialogMenu.addItem (101, T("Show an alert-window with a 'warning' icon..."));
            dialogMenu.addItem (102, T("Show an alert-window with an 'info' icon..."));
            dialogMenu.addItem (103, T("Show an alert-window with a 'question' icon..."));

            dialogMenu.addSeparator();

            dialogMenu.addItem (110, T("Show an ok/cancel alert-window..."));

            dialogMenu.addSeparator();

            dialogMenu.addItem (111, T("Show an alert-window with some extra components..."));

            dialogMenu.addSeparator();

            dialogMenu.addItem (112, T("Show a ThreadWithProgressWindow demo..."));

            m.addSubMenu (T("AlertWindow demonstrations"), dialogMenu);

            m.addSeparator();

            m.addItem (120, T("Show a colour selector demo..."));
            m.addSeparator();

#if JUCE_MAC
            m.addItem (140, T("Run the Apple Remote Control test..."));
            m.addSeparator();
#endif

            PopupMenu nativeFileChoosers;
            nativeFileChoosers.addItem (121, T("'Load' file browser..."));
            nativeFileChoosers.addItem (124, T("'Load' file browser with an image file preview..."));
            nativeFileChoosers.addItem (122, T("'Save' file browser..."));
            nativeFileChoosers.addItem (123, T("'Choose directory' file browser..."));

            PopupMenu juceFileChoosers;
            juceFileChoosers.addItem (131, T("'Load' file browser..."));
            juceFileChoosers.addItem (134, T("'Load' file browser with an image file preview..."));
            juceFileChoosers.addItem (132, T("'Save' file browser..."));
            juceFileChoosers.addItem (133, T("'Choose directory' file browser..."));

            PopupMenu fileChoosers;
            fileChoosers.addSubMenu (T("Operating system dialogs"), nativeFileChoosers);
            fileChoosers.addSubMenu (T("Juce dialogs"), juceFileChoosers);

            m.addSubMenu (T("File chooser dialogs"), fileChoosers);

            int result = m.showAt (menuButton);

            if (result != 0)
            {
                // user chose something from the menu..

                if (result >= 100 && result < 105)
                {
                    AlertWindow::AlertIconType icon = AlertWindow::NoIcon;

                    if (result == 101)
                        icon = AlertWindow::WarningIcon;
                    else if (result == 102)
                        icon = AlertWindow::InfoIcon;
                    else if (result == 103)
                        icon = AlertWindow::QuestionIcon;

                    AlertWindow::showMessageBox (icon,
                                                 T("This is an AlertWindow"),
                                                 T("And this is the AlertWindow's message. Blah blah blah blah blah blah blah blah blah blah blah blah blah."),
                                                 T("ok"));
                }
                else if (result == 110)
                {
                    bool userPickedOk
                        = AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon,
                                                        T("This is an ok/cancel AlertWindow"),
                                                        T("And this is the AlertWindow's message. Blah blah blah blah blah blah blah blah blah blah blah blah blah."));
                }
                else if (result == 111)
                {
                    AlertWindow w (T("AlertWindow demo.."),
                                   T("This AlertWindow has a couple of extra components added to show how to add drop-down lists and text entry boxes."),
                                   AlertWindow::QuestionIcon);

                    w.addTextEditor (T("text"), T("enter some text here"), T("text field:"));

                    StringArray options;
                    options.add (T("option 1"));
                    options.add (T("option 2"));
                    options.add (T("option 3"));
                    options.add (T("option 4"));
                    w.addComboBox (T("option"), options, T("some options"));

                    w.addButton (T("ok"), 1, KeyPress (KeyPress::returnKey, 0, 0));
                    w.addButton (T("cancel"), 0, KeyPress (KeyPress::escapeKey, 0, 0));

                    if (w.runModalLoop() != 0) // is they picked 'ok'
                    {
                        // this is the item they chose in the drop-down list..
                        const int optionIndexChosen = w.getComboBoxComponent (T("option"))->getSelectedItemIndex();

                        // this is the text they entered..
                        String text = w.getTextEditorContents (T("text"));

                    }
                }
                else if (result == 112)
                {
                    DemoBackgroundThread demoThread;

                    if (demoThread.runThread())
                    {
                        // thread finished normally..
                        AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                                     T("Progress window"),
                                                     T("Thread finished ok!"));
                    }
                    else
                    {
                        // user pressed the cancel button..
                        AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                                     T("Progress window"),
                                                     T("You pressed cancel!"));
                    }

                }
                else if (result == 120)
                {
                    ColourSelectorDialogWindow colourDialog;

                    // this will run an event loop until the dialog's closeButtonPressed()
                    // method causes the loop to exit.
                    colourDialog.runModalLoop();
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
                    const bool useNativeVersion = result < 130;
                    if (result > 130)
                        result -= 10;

                    if (result == 121)
                    {
                        FileChooser fc (T("Choose a file to open..."),
                                        File::getCurrentWorkingDirectory(),
                                        T("*"),
                                        useNativeVersion);

                        if (fc.browseForFileToOpen())
                        {
                            File chosenFile = fc.getResult();

                            AlertWindow::showMessageBox (AlertWindow::InfoIcon,
                                                         T("File Chooser..."),
                                                         T("You picked: ") + chosenFile.getFullPathName());
                        }
                    }
                    else if (result == 124)
                    {
                        ImagePreviewComponent imagePreview;
                        imagePreview.setSize (200, 200);

                        FileChooser fc (T("Choose an image to open..."),
                                        File::getCurrentWorkingDirectory(),
                                        T("*.jpg;*.jpeg;*.png;*.gif"),
                                        useNativeVersion);

                        if (fc.browseForFileToOpen (&imagePreview))
                        {
                            File chosenFile = fc.getResult();

                            AlertWindow::showMessageBox (AlertWindow::InfoIcon,
                                                         T("File Chooser..."),
                                                         T("You picked: ") + chosenFile.getFullPathName());
                        }
                    }
                    else if (result == 122)
                    {
                        FileChooser fc (T("Choose a file to save..."),
                                        File::getCurrentWorkingDirectory(),
                                        T("*"),
                                        useNativeVersion);

                        if (fc.browseForFileToSave (true))
                        {
                            File chosenFile = fc.getResult();

                            AlertWindow::showMessageBox (AlertWindow::InfoIcon,
                                                         T("File Chooser..."),
                                                         T("You picked: ") + chosenFile.getFullPathName());
                        }
                    }
                    else if (result == 123)
                    {
                        FileChooser fc (T("Choose a directory..."),
                                        File::getCurrentWorkingDirectory(),
                                        T("*"),
                                        useNativeVersion);

                        if (fc.browseForDirectory())
                        {
                            File chosenDirectory = fc.getResult();

                            AlertWindow::showMessageBox (AlertWindow::InfoIcon,
                                                         T("File Chooser..."),
                                                         T("You picked: ") + chosenDirectory.getFullPathName());
                        }
                    }
                }
                else if (result == 1001)
                {
                    tabs->setOrientation (TabbedButtonBar::TabsAtTop);
                }
                else if (result == 1002)
                {
                    tabs->setOrientation (TabbedButtonBar::TabsAtBottom);
                }
                else if (result == 1003)
                {
                    tabs->setOrientation (TabbedButtonBar::TabsAtLeft);
                }
                else if (result == 1004)
                {
                    tabs->setOrientation (TabbedButtonBar::TabsAtRight);
                }
            }
        }
    }
};


//==============================================================================
Component* createWidgetsDemo (ApplicationCommandManager* commandManager)
{
    return new WidgetsDemo (commandManager);
}
