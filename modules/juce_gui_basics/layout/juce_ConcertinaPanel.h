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
