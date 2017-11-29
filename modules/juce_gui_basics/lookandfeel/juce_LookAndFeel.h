/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/** This class is used to hold a few look and feel base classes which are associated
    with classes that may not be present because they're from modules other than
    juce_gui_basics.
*/
struct JUCE_API  ExtraLookAndFeelBaseClasses
{
    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct JUCE_API  LassoComponentMethods
    {
        virtual ~LassoComponentMethods() {}

        virtual void drawLasso (Graphics&, Component& lassoComp) = 0;
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct JUCE_API  KeyMappingEditorComponentMethods
    {
        virtual ~KeyMappingEditorComponentMethods() {}

        virtual void drawKeymapChangeButton (Graphics&, int width, int height, Button&, const String& keyDescription) = 0;
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct JUCE_API  AudioDeviceSelectorComponentMethods
    {
        virtual ~AudioDeviceSelectorComponentMethods() {}

        virtual void drawLevelMeter (Graphics&, int width, int height, float level) = 0;
    };
};


//==============================================================================
/**
    LookAndFeel objects define the appearance of all the JUCE widgets, and subclasses
    can be used to apply different 'skins' to the application.

    This class is an abstract base-class - for actual look-and-feels that you can
    instantiate, see LookAndFeel_V1, LookAndFeel_V2 and LookAndFeel_V3.

    @see LookAndFeel_V1, LookAndFeel_V2, LookAndFeel_V3
*/
class JUCE_API  LookAndFeel   : public ScrollBar::LookAndFeelMethods,
                                public Button::LookAndFeelMethods,
                                public ImageButton::LookAndFeelMethods,
                                public TextEditor::LookAndFeelMethods,
                                public FileBrowserComponent::LookAndFeelMethods,
                                public TreeView::LookAndFeelMethods,
                                public BubbleComponent::LookAndFeelMethods,
                                public AlertWindow::LookAndFeelMethods,
                                public PopupMenu::LookAndFeelMethods,
                                public ComboBox::LookAndFeelMethods,
                                public Label::LookAndFeelMethods,
                                public Slider::LookAndFeelMethods,
                                public ResizableWindow::LookAndFeelMethods,
                                public DocumentWindow::LookAndFeelMethods,
                                public TooltipWindow::LookAndFeelMethods,
                                public TabbedButtonBar::LookAndFeelMethods,
                                public PropertyComponent::LookAndFeelMethods,
                                public FilenameComponent::LookAndFeelMethods,
                                public GroupComponent::LookAndFeelMethods,
                                public TableHeaderComponent::LookAndFeelMethods,
                                public CallOutBox::LookAndFeelMethods,
                                public Toolbar::LookAndFeelMethods,
                                public ConcertinaPanel::LookAndFeelMethods,
                                public ProgressBar::LookAndFeelMethods,
                                public StretchableLayoutResizerBar::LookAndFeelMethods,
                                public ExtraLookAndFeelBaseClasses::KeyMappingEditorComponentMethods,
                                public ExtraLookAndFeelBaseClasses::AudioDeviceSelectorComponentMethods,
                                public ExtraLookAndFeelBaseClasses::LassoComponentMethods,
                                public SidePanel::LookAndFeelMethods
{
public:
    //==============================================================================
    /** Creates the default JUCE look and feel. */
    LookAndFeel();

    /** Destructor. */
    virtual ~LookAndFeel();

    //==============================================================================
    /** Returns the current default look-and-feel for a component to use when it
        hasn't got one explicitly set.

        @see setDefaultLookAndFeel
    */
    static LookAndFeel& getDefaultLookAndFeel() noexcept;

    /** Changes the default look-and-feel.

        @param newDefaultLookAndFeel    the new look-and-feel object to use - if this is
                                        set to null, it will revert to using the default one. The
                                        object passed-in must be deleted by the caller when
                                        it's no longer needed.
        @see getDefaultLookAndFeel
    */
    static void setDefaultLookAndFeel (LookAndFeel* newDefaultLookAndFeel) noexcept;

    //==============================================================================
    /** Looks for a colour that has been registered with the given colour ID number.

        If a colour has been set for this ID number using setColour(), then it is
        returned. If none has been set, it will just return Colours::black.

        The colour IDs for various purposes are stored as enums in the components that
        they are relevant to - for an example, see Slider::ColourIds,
        Label::ColourIds, TextEditor::ColourIds, TreeView::ColourIds, etc.

        If you're looking up a colour for use in drawing a component, it's usually
        best not to call this directly, but to use the Component::findColour() method
        instead. That will first check whether a suitable colour has been registered
        directly with the component, and will fall-back on calling the component's
        LookAndFeel's findColour() method if none is found.

        @see setColour, Component::findColour, Component::setColour
    */
    Colour findColour (int colourId) const noexcept;

    /** Registers a colour to be used for a particular purpose.
        For more details, see the comments for findColour().
        @see findColour, Component::findColour, Component::setColour
    */
    void setColour (int colourId, Colour colour) noexcept;

    /** Returns true if the specified colour ID has been explicitly set using the
        setColour() method.
    */
    bool isColourSpecified (int colourId) const noexcept;

    //==============================================================================
    /** Returns the typeface that should be used for a given font.
        The default implementation just does what you'd expect it to, but you can override
        this if you want to intercept fonts and use your own custom typeface object.
    */
    virtual Typeface::Ptr getTypefaceForFont (const Font&);

    /** Allows you to change the default sans-serif font.

        If you need to supply your own Typeface object for any of the default fonts, rather
        than just supplying the name (e.g. if you want to use an embedded font), then
        you should instead override getTypefaceForFont() to create and return the typeface.
    */
    void setDefaultSansSerifTypefaceName (const String& newName);

    //==============================================================================
    /** Override this to get the chance to swap a component's mouse cursor for a
        customised one.
    */
    virtual MouseCursor getMouseCursorFor (Component&);

    //==============================================================================
    /** Creates a new graphics context object. */
    virtual LowLevelGraphicsContext* createGraphicsContext (const Image& imageToRenderOn,
                                                            const Point<int>& origin,
                                                            const RectangleList<int>& initialClip);

    void setUsingNativeAlertWindows (bool shouldUseNativeAlerts);
    bool isUsingNativeAlertWindows();

    //==============================================================================
    /** Draws a small image that spins to indicate that something's happening.
        This method should use the current time to animate itself, so just keep
        repainting it every so often.
    */
    virtual void drawSpinningWaitAnimation (Graphics&, const Colour& colour,
                                            int x, int y, int w, int h) = 0;

    //==============================================================================
    /** Returns a tick shape for use in yes/no boxes, etc. */
    virtual Path getTickShape (float height) = 0;
    /** Returns a cross shape for use in yes/no boxes, etc. */
    virtual Path getCrossShape (float height) = 0;

    //==============================================================================
    virtual DropShadower* createDropShadowerForComponent (Component*) = 0;

    //==============================================================================
    /** Plays the system's default 'beep' noise, to alert the user about something very important. */
    virtual void playAlertSound();

private:
    //==============================================================================
    struct ColourSetting
    {
        int colourID;
        Colour colour;

        bool operator<  (const ColourSetting& other) const noexcept  { return colourID <  other.colourID; }
        bool operator== (const ColourSetting& other) const noexcept  { return colourID == other.colourID; }
    };

    SortedSet<ColourSetting> colours;
    String defaultSans, defaultSerif, defaultFixed;
    bool useNativeAlertWindows = false;

    JUCE_DECLARE_WEAK_REFERENCEABLE (LookAndFeel)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel)
};

} // namespace juce
