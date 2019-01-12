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
                   Drawable* normalImage,
                   Drawable* toggledOnImage);

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
    Drawable* currentImage;

    void updateDrawable();
    Drawable* getImageToUse() const;
    void setCurrentImage (Drawable*);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolbarButton)
};

} // namespace juce
