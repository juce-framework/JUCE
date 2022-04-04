/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A component containing a list of toolbar items, which the user can drag onto
    a toolbar to add them.

    You can use this class directly, but it's a lot easier to call Toolbar::showCustomisationDialog(),
    which automatically shows one of these in a dialog box with lots of extra controls.

    @see Toolbar

    @tags{GUI}
*/
class JUCE_API  ToolbarItemPalette    : public Component,
                                        public DragAndDropContainer
{
public:
    //==============================================================================
    /** Creates a palette of items for a given factory, with the aim of adding them
        to the specified toolbar.

        The ToolbarItemFactory::getAllToolbarItemIds() method is used to create the
        set of items that are shown in this palette.

        The toolbar and factory must not be deleted while this object exists.
    */
    ToolbarItemPalette (ToolbarItemFactory& factory,
                        Toolbar& toolbar);

    /** Destructor. */
    ~ToolbarItemPalette() override;

    //==============================================================================
    /** @internal */
    void resized() override;

private:
    ToolbarItemFactory& factory;
    Toolbar& toolbar;
    Viewport viewport;
    OwnedArray<ToolbarItemComponent> items;

    friend class Toolbar;
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;
    void replaceComponent (ToolbarItemComponent&);
    void addComponent (int itemId, int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolbarItemPalette)
};

} // namespace juce
