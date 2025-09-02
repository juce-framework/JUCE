/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A component that is positioned on either the left- or right-hand side of its parent,
    containing a header and some content. This sort of component is typically used for
    navigation and forms in mobile applications.

    When triggered with the showOrHide() method, the SidePanel will animate itself to its
    new position. This component also contains some logic to reactively resize and dismiss
    itself when the user drags it.

    @tags{GUI}
*/
class SidePanel    : public Component,
                     private ComponentListener,
                     private ChangeListener
{
public:
    //==============================================================================
    /** Creates a SidePanel component.

        @param title               the text to use for the SidePanel's title bar
        @param width               the width of the SidePanel
        @param positionOnLeft      if true, the SidePanel will be positioned on the left of its parent component and
                                   if false, the SidePanel will be positioned on the right of its parent component
        @param contentComponent    the component to add to this SidePanel - this content will take up the full
                                   size of the SidePanel, minus the height of the title bar. You can pass nullptr
                                   to this if you like and set the content component later using the setContent() method
        @param deleteComponentWhenNoLongerNeeded    if true, the component will be deleted automatically when
                                   the SidePanel is deleted or when a different component is added. If false,
                                   the caller must manage the lifetime of the component
    */
    SidePanel (StringRef title, int width, bool positionOnLeft,
               Component* contentComponent = nullptr,
               bool deleteComponentWhenNoLongerNeeded = true);

    /** Destructor */
    ~SidePanel() override;

    //==============================================================================
    /** Sets the component that this SidePanel will contain.

        This will add the given component to this SidePanel and position it below the title bar.

        (Don't add or remove any child components directly using the normal
        Component::addChildComponent() methods).

        @param newContentComponent   the component to add to this SidePanel, or nullptr to remove
                                     the current component.
        @param deleteComponentWhenNoLongerNeeded    if true, the component will be deleted automatically when
                                   the SidePanel is deleted or when a different component is added. If false,
                                   the caller must manage the lifetime of the component

        @see getContent
    */
    void setContent (Component* newContentComponent,
                     bool deleteComponentWhenNoLongerNeeded = true);

    /** Returns the component that's currently being used inside the SidePanel.

        @see setViewedComponent
    */
    Component* getContent() const noexcept    { return contentComponent.get(); }

    /** Sets a custom component to be used for the title bar of this SidePanel, replacing
        the default. You can pass a nullptr to revert to the default title bar.

        @param titleBarComponentToUse  the component to use as the title bar, or nullptr to use
                                       the default
        @param keepDismissButton       if false the specified component will take up the full width of
                                       the title bar including the dismiss button but if true, the default
                                       dismiss button will be kept
        @param deleteComponentWhenNoLongerNeeded  if true, the component will be deleted automatically when
                                       the SidePanel is deleted or when a different component is added. If false,
                                       the caller must manage the lifetime of the component

        @see getTitleBarComponent
    */
    void setTitleBarComponent (Component* titleBarComponentToUse,
                               bool keepDismissButton,
                               bool deleteComponentWhenNoLongerNeeded = true);

    /** Returns the component that is currently being used as the title bar of the SidePanel.

        @see setTitleBarComponent
    */
    Component* getTitleBarComponent() const noexcept    { return titleBarComponent.get(); }

    /** Shows or hides the SidePanel.

        This will animate the SidePanel to either its full width or to be hidden on the
        left- or right-hand side of its parent component depending on the value of positionOnLeft
        that was passed to the constructor.

        @param show    if true, this will show the SidePanel and if false the SidePanel will be hidden
    */
    void showOrHide (bool show);

    //==============================================================================
    /** Returns true if the SidePanel is currently showing. */
    bool isPanelShowing() const noexcept               { return isShowing; }

    /** Returns true if the SidePanel is positioned on the left of its parent. */
    bool isPanelOnLeft() const noexcept                { return isOnLeft; }

    /** Sets the width of the shadow that will be drawn on the side of the panel. */
    void setShadowWidth (int newWidth) noexcept        { shadowWidth = newWidth; }

    /** Returns the width of the shadow that will be drawn on the side of the panel. */
    int getShadowWidth() const noexcept                { return shadowWidth; }

    /** Sets the height of the title bar at the top of the SidePanel. */
    void setTitleBarHeight (int newHeight) noexcept    { titleBarHeight = newHeight; }

    /** Returns the height of the title bar at the top of the SidePanel. */
    int getTitleBarHeight() const noexcept             { return titleBarHeight; }

    /** Returns the text that is displayed in the title bar at the top of the SidePanel. */
    String getTitleText() const noexcept               { return titleLabel.getText(); }

    /** @see isContentRestrictedToSafeArea() */
    void setContentRestrictedToSafeArea (bool x) noexcept { restrictToSafeArea = x; }

    /** When true, will avoid displaying menu content within areas of the screen that may be
        obscured by display cutouts or operating system decorations. When false, the menu's
        content will entirely fill the menu bounds. True by default.

        @see setContentRestrictedToSafeArea()
    */
    bool isContentRestrictedToSafeArea() const noexcept { return restrictToSafeArea; }

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        SidePanel drawing functionality.
     */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual Font getSidePanelTitleFont (SidePanel&) = 0;
        virtual Justification getSidePanelTitleJustification (SidePanel&) = 0;
        virtual Path getSidePanelDismissButtonShape (SidePanel&) = 0;
    };

    /** A set of colour IDs to use to change the colour of various aspects of the SidePanel.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColour          = 0x100f001,
        titleTextColour           = 0x100f002,
        shadowBaseColour          = 0x100f003,
        dismissButtonNormalColour = 0x100f004,
        dismissButtonOverColour   = 0x100f005,
        dismissButtonDownColour   = 0x100f006
    };

    //==============================================================================
    /** You can assign a lambda to this callback object and it will be called when the panel is moved. */
    std::function<void()> onPanelMove;

    /** You can assign a lambda to this callback object and it will be called when the panel is shown or hidden. */
    std::function<void (bool)> onPanelShowHide;

    //==============================================================================
    /** @internal */
    void moved() override;
    /** @internal */
    void resized() override;
    /** @internal */
    void paint (Graphics& g) override;
    /** @internal */
    void parentHierarchyChanged() override;
    /** @internal */
    void mouseDrag (const MouseEvent&) override;
    /** @internal */
    void mouseUp (const MouseEvent&) override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    Component* parent = nullptr;
    OptionalScopedPointer<Component> contentComponent;
    OptionalScopedPointer<Component> titleBarComponent;

    Label titleLabel;
    ShapeButton dismissButton { "dismissButton", Colours::lightgrey, Colours::lightgrey, Colours::white };

    Rectangle<int> shadowArea;

    bool isOnLeft = false;
    bool isShowing = false;

    int panelWidth = 0;
    int shadowWidth = 15;
    int titleBarHeight = 40;

    Rectangle<int> startingBounds;
    bool shouldResize = false;
    int amountMoved = 0;

    bool shouldShowDismissButton = true;
    bool restrictToSafeArea = true;

    //==============================================================================
    void lookAndFeelChanged() override;
    void componentMovedOrResized (Component&, bool wasMoved, bool wasResized) override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    Rectangle<int> calculateShowingBoundsInParent (Component&) const;
    Point<int> getCurrentOffset() const;
    Rectangle<int> calculateBoundsInParent (Component&) const;
    void calculateAndRemoveShadowBounds (Rectangle<int>& bounds);

    bool isMouseEventInThisOrChildren (Component*);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SidePanel)
};

} // namespace juce
