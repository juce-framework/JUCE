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
    A type of button designed to go on a toolbar.

    This simple button can have two Drawable objects specified - one for normal
    use and another one (optionally) for the button's "on" state if it's a
    toggle button.

    @see Toolbar, ToolbarItemFactory, ToolbarItemComponent, Drawable, Button

    @tags{GUI}
*/
class JUCE_API  ToolbarButton   : public ToolbarItemComponent
{
public:
    //==============================================================================
    /** Creates a ToolbarButton.

        @param itemId       the ID for this toolbar item type. This is passed through to the
                            ToolbarItemComponent constructor
        @param labelText    the text to display on the button (if the toolbar is using a style
                            that shows text labels). This is passed through to the
                            ToolbarItemComponent constructor
        @param normalImage  a drawable object that the button should use as its icon. The object
                            that is passed-in here will be kept by this object and will be
                            deleted when no longer needed or when this button is deleted.
        @param toggledOnImage  a drawable object that the button can use as its icon if the button
                            is in a toggled-on state (see the Button::getToggleState() method). If
                            nullptr is passed-in here, then the normal image will be used instead,
                            regardless of the toggle state. The object that is passed-in here will be
                            owned by this object and will be deleted when no longer needed or when
                            this button is deleted.
    */
    ToolbarButton (int itemId,
                   const String& labelText,
                   std::unique_ptr<Drawable> normalImage,
                   std::unique_ptr<Drawable> toggledOnImage);

    /** Destructor. */
    ~ToolbarButton() override;


    //==============================================================================
    /** @internal */
    bool getToolbarItemSizes (int toolbarDepth, bool isToolbarVertical, int& preferredSize,
                              int& minSize, int& maxSize) override;
    /** @internal */
    void paintButtonArea (Graphics&, int width, int height, bool isMouseOver, bool isMouseDown) override;
    /** @internal */
    void contentAreaChanged (const Rectangle<int>&) override;
    /** @internal */
    void buttonStateChanged() override;
    /** @internal */
    void resized() override;
    /** @internal */
    void enablementChanged() override;

private:
    //==============================================================================
    std::unique_ptr<Drawable> normalImage, toggledOnImage;
    Drawable* currentImage = nullptr;

    void updateDrawable();
    Drawable* getImageToUse() const;
    void setCurrentImage (Drawable*);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolbarButton)
};

} // namespace juce
