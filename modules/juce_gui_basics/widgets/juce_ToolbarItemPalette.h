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
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    ToolbarItemFactory& factory;
    Toolbar& toolbar;
    Viewport viewport;
    OwnedArray<ToolbarItemComponent> items;

    friend class Toolbar;
    void replaceComponent (ToolbarItemComponent&);
    void addComponent (int itemId, int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolbarItemPalette)
};

} // namespace juce
