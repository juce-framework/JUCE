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
    A panel which holds a vertical stack of components which can be expanded
    and contracted.

    Each section has its own header bar which can be dragged up and down
    to resize it, or double-clicked to fully expand that section.

    @tags{GUI}
*/
class JUCE_API  ConcertinaPanel   : public Component
{
public:
    /** Creates an empty concertina panel.
        You can call addPanel() to add some components to it.
    */
    ConcertinaPanel();

    /** Destructor. */
    ~ConcertinaPanel() override;

    /** Adds a component to the panel.
        @param insertIndex          the index at which this component will be inserted, or
                                    -1 to append it to the end of the list.
        @param component            the component that will be shown
        @param takeOwnership        if true, then the ConcertinaPanel will take ownership
                                    of the content component, and will delete it later when
                                    it's no longer needed. If false, it won't delete it, and
                                    you must make sure it doesn't get deleted while in use.
    */
    void addPanel (int insertIndex, Component* component, bool takeOwnership);

    /** Removes one of the panels.
        If the takeOwnership flag was set when the panel was added, then
        this will also delete the component.
    */
    void removePanel (Component* panelComponent);

    /** Returns the number of panels.
        @see getPanel
    */
    int getNumPanels() const noexcept;

    /** Returns one of the panels.
        @see getNumPanels()
    */
    Component* getPanel (int index) const noexcept;

    /** Resizes one of the panels.
        The panelComponent must point to  a valid panel component.
        If animate is true, the panels will be animated into their new positions;
        if false, they will just be immediately resized.
    */
    bool setPanelSize (Component* panelComponent, int newHeight, bool animate);

    /** Attempts to make one of the panels full-height.
        The panelComponent must point to  a valid panel component.
        If this component has had a maximum size set, then it will be
        expanded to that size. Otherwise, it'll fill as much of the total
        space as possible.
    */
    bool expandPanelFully (Component* panelComponent, bool animate);

    /** Sets a maximum size for one of the panels. */
    void setMaximumPanelSize (Component* panelComponent, int maximumSize);

    /** Sets the height of the header section for one of the panels. */
    void setPanelHeaderSize (Component* panelComponent, int headerSize);

    /** Sets a custom header Component for one of the panels.

        @param panelComponent           the panel component to add the custom header to.
        @param customHeaderComponent    the custom component to use for the panel header.
                                        This can be nullptr to clear the custom header component
                                        and just use the standard LookAndFeel panel.
        @param takeOwnership            if true, then the PanelHolder will take ownership
                                        of the custom header component, and will delete it later when
                                        it's no longer needed. If false, it won't delete it, and
                                        you must make sure it doesn't get deleted while in use.
     */
    void setCustomPanelHeader (Component* panelComponent, Component* customHeaderComponent, bool takeOwnership);

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual void drawConcertinaPanelHeader (Graphics&, const Rectangle<int>& area,
                                                bool isMouseOver, bool isMouseDown,
                                                ConcertinaPanel&, Component&) = 0;
    };

    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    void resized() override;

    class PanelHolder;
    struct PanelSizes;
    std::unique_ptr<PanelSizes> currentSizes;
    OwnedArray<PanelHolder> holders;
    ComponentAnimator animator;
    int headerHeight;

    int indexOfComp (Component*) const noexcept;
    PanelSizes getFittedSizes() const;
    void applyLayout (const PanelSizes&, bool animate);
    void setLayout (const PanelSizes&, bool animate);
    void panelHeaderDoubleClicked (Component*);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaPanel)
};

} // namespace juce
