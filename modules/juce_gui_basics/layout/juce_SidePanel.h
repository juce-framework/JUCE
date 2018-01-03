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
/**
    A component that is positioned on either the left- or right-hand side of its parent,
    containing a header and some content. This sort of component is typically used for
    navigation and forms in mobile applications.

    When triggered with the showOrHide() method, the SidePanel will animate itself to its
    new position. This component also contains some logic to reactively resize and dismiss
    itself when the user drags it.
 */
//==============================================================================
class SidePanel    : public Component,
                     private ComponentListener
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
    ~SidePanel();

    //==============================================================================
    /** Sets the component that this SidePanel will contain.

        This will add the given component to this SidePanel and position it below the title bar.

        (Don't add or remove any child components directly using the normal
        Component::addChildComponent() methods).

        @param newContentComponent   the component to add to this SidePanel, or null to remove
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
    Component* getContent()    { return contentComponent.get(); }

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

    /** Sets the height of the title bar at the top of the SidePanel. */
    void setTitleBarHeight (int newHeight) noexcept    { titleBarHeight = newHeight; }

    //==============================================================================
    void resized() override;
    void paint (Graphics& g) override;

    void parentHierarchyChanged() override;

    void mouseDrag (const MouseEvent&) override;
    void mouseUp (const MouseEvent&) override;

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        SidePanel drawing functionality.
     */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() {}

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
        dismissButtonOverColour   = 0x100f004,
        dismissButtonDownColour   = 0x100f005
    };

private:
    //==========================================================================
    Component* parent = nullptr;
    OptionalScopedPointer<Component> contentComponent;

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

    //==========================================================================
    void lookAndFeelChanged() override;
    void componentMovedOrResized (Component&, bool wasMoved, bool wasResized) override;

    Rectangle<int> calculateBoundsInParent (Component&) const;
    void calculateAndRemoveShadowBounds (Rectangle<int>& bounds);

    bool isMouseEventInThisOrChildren (Component*);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SidePanel)
};

} // namespace juce
