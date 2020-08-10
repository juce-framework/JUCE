/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
/**
    A resizable window with a title bar and maximise, minimise and close buttons.

    This subclass of ResizableWindow creates a fairly standard type of window with
    a title bar and various buttons. The name of the component is shown in the
    title bar, and an icon can optionally be specified with setIcon().

    All the methods available to a ResizableWindow are also available to this,
    so it can easily be made resizable, minimised, maximised, etc.

    It's not advisable to add child components directly to a DocumentWindow: put them
    inside your content component instead. And overriding methods like resized(), moved(), etc
    is also not recommended - instead override these methods for your content component.
    (If for some obscure reason you do need to override these methods, always remember to
    call the super-class's resized() method too, otherwise it'll fail to lay out the window
    decorations correctly).

    You can also automatically add a menu bar to the window, using the setMenuBar()
    method.

    @see ResizableWindow, DialogWindow

    @tags{GUI}
*/
class JUCE_API  DocumentWindow   : public ResizableWindow
{
public:
    //==============================================================================
    /** The set of available button-types that can be put on the title bar.

        @see setTitleBarButtonsRequired
    */
    enum TitleBarButtons
    {
        minimiseButton = 1,
        maximiseButton = 2,
        closeButton = 4,

        /** A combination of all the buttons above. */
        allButtons = 7
    };

    //==============================================================================
    /** Creates a DocumentWindow.

        @param name             the name to give the component - this is also
                                the title shown at the top of the window. To change
                                this later, use setName()
        @param backgroundColour the colour to use for filling the window's background.
        @param requiredButtons  specifies which of the buttons (close, minimise, maximise)
                                should be shown on the title bar. This value is a bitwise
                                combination of values from the TitleBarButtons enum. Note
                                that it can be "allButtons" to get them all. You
                                can change this later with the setTitleBarButtonsRequired()
                                method, which can also specify where they are positioned.
        @param addToDesktop     if true, the window will be automatically added to the
                                desktop; if false, you can use it as a child component
        @see TitleBarButtons
    */
    DocumentWindow (const String& name,
                    Colour backgroundColour,
                    int requiredButtons,
                    bool addToDesktop = true);

    /** Destructor.
        If a content component has been set with setContentOwned(), it will be deleted.
    */
    ~DocumentWindow() override;

    //==============================================================================
    /** Changes the component's name.

        (This is overridden from Component::setName() to cause a repaint, as
        the name is what gets drawn across the window's title bar).
    */
    void setName (const String& newName) override;

    /** Sets an icon to show in the title bar, next to the title.

        A copy is made internally of the image, so the caller can delete the
        image after calling this. If an empty Image is passed-in, any existing icon
        will be removed.
    */
    void setIcon (const Image& imageToUse);

    /** Changes the height of the title-bar. */
    void setTitleBarHeight (int newHeight);

    /** Returns the current title bar height. */
    int getTitleBarHeight() const;

    /** Changes the set of title-bar buttons being shown.

        @param requiredButtons  specifies which of the buttons (close, minimise, maximise)
                                should be shown on the title bar. This value is a bitwise
                                combination of values from the TitleBarButtons enum. Note
                                that it can be "allButtons" to get them all.
        @param positionTitleBarButtonsOnLeft    if true, the buttons should go at the
                                left side of the bar; if false, they'll be placed at the right
    */
    void setTitleBarButtonsRequired (int requiredButtons,
                                     bool positionTitleBarButtonsOnLeft);

    /** Sets whether the title should be centred within the window.

        If true, the title text is shown in the middle of the title-bar; if false,
        it'll be shown at the left of the bar.
    */
    void setTitleBarTextCentred (bool textShouldBeCentred);

    //==============================================================================
    /** Creates a menu inside this window.

        @param menuBarModel     this specifies a MenuBarModel that should be used to
                                generate the contents of a menu bar that will be placed
                                just below the title bar, and just above any content
                                component. If this value is a nullptr, any existing menu bar
                                will be removed from the component; if it is not a nullptr,
                                one will be added if it's required.
        @param menuBarHeight    the height of the menu bar component, if one is needed. Pass a value of zero
                                or less to use the look-and-feel's default size.
    */
    void setMenuBar (MenuBarModel* menuBarModel,
                     int menuBarHeight = 0);

    /** Returns the current menu bar component, or null if there isn't one.
        This is probably a MenuBarComponent, unless a custom one has been set using
        setMenuBarComponent().
    */
    Component* getMenuBarComponent() const noexcept;

    /** Replaces the current menu bar with a custom component.
        The component will be owned and deleted by the document window.
    */
    void setMenuBarComponent (Component* newMenuBarComponent);

    //==============================================================================
    /** This method is called when the user tries to close the window.

        This is triggered by the user clicking the close button, or using some other
        OS-specific key shortcut or OS menu for getting rid of a window.

        If the window is just a pop-up, you should override this closeButtonPressed()
        method and make it delete the window in whatever way is appropriate for your
        app. E.g. you might just want to call "delete this".

        If your app is centred around this window such that the whole app should quit when
        the window is closed, then you will probably want to use this method as an opportunity
        to call JUCEApplicationBase::quit(), and leave the window to be deleted later by your
        JUCEApplicationBase::shutdown() method. (Doing it this way means that your window will
        still get cleaned-up if the app is quit by some other means (e.g. a cmd-Q on the mac
        or closing it via the taskbar icon on Windows).

        (Note that the DocumentWindow class overrides Component::userTriedToCloseWindow() and
        redirects it to call this method, so any methods of closing the window that are
        caught by userTriedToCloseWindow() will also end up here).
    */
    virtual void closeButtonPressed();

    /** Callback that is triggered when the minimise button is pressed.

        The default implementation of this calls ResizableWindow::setMinimised(), but
        you can override it to do more customised behaviour.
    */
    virtual void minimiseButtonPressed();

    /** Callback that is triggered when the maximise button is pressed, or when the
        title-bar is double-clicked.

        The default implementation of this calls ResizableWindow::setFullScreen(), but
        you can override it to do more customised behaviour.
    */
    virtual void maximiseButtonPressed();

    //==============================================================================
    /** Returns the close button, (or nullptr if there isn't one). */
    Button* getCloseButton() const noexcept;

    /** Returns the minimise button, (or nullptr if there isn't one). */
    Button* getMinimiseButton() const noexcept;

    /** Returns the maximise button, (or nullptr if there isn't one). */
    Button* getMaximiseButton() const noexcept;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the window.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        textColourId                = 0x1005701,  /**< The colour to draw any text with. It's up to the look
                                                       and feel class how this is used. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        window drawing functionality.
    */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual void drawDocumentWindowTitleBar (DocumentWindow&,
                                                 Graphics&, int w, int h,
                                                 int titleSpaceX, int titleSpaceW,
                                                 const Image* icon,
                                                 bool drawTitleTextOnLeft) = 0;

        virtual Button* createDocumentWindowButton (int buttonType) = 0;

        virtual void positionDocumentWindowButtons (DocumentWindow&,
                                                    int titleBarX, int titleBarY, int titleBarW, int titleBarH,
                                                    Button* minimiseButton,
                                                    Button* maximiseButton,
                                                    Button* closeButton,
                                                    bool positionTitleBarButtonsOnLeft) = 0;
    };

    //==============================================================================
   #ifndef DOXYGEN
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void lookAndFeelChanged() override;
    /** @internal */
    BorderSize<int> getBorderThickness() override;
    /** @internal */
    BorderSize<int> getContentComponentBorder() override;
    /** @internal */
    void mouseDoubleClick (const MouseEvent&) override;
    /** @internal */
    void userTriedToCloseWindow() override;
    /** @internal */
    void activeWindowStatusChanged() override;
    /** @internal */
    int getDesktopWindowStyleFlags() const override;
    /** @internal */
    void parentHierarchyChanged() override;
    /** @internal */
    Rectangle<int> getTitleBarArea();
   #endif

private:
    //==============================================================================
    int titleBarHeight = 26, menuBarHeight = 24, requiredButtons;
    bool positionTitleBarButtonsOnLeft, drawTitleTextCentred = true;
    std::unique_ptr<Button> titleBarButtons [3];
    Image titleBarIcon;
    std::unique_ptr<Component> menuBar;
    MenuBarModel* menuBarModel = nullptr;

    class ButtonListenerProxy;
    std::unique_ptr<ButtonListenerProxy> buttonListener;

    void repaintTitleBar();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocumentWindow)
};

} // namespace juce
