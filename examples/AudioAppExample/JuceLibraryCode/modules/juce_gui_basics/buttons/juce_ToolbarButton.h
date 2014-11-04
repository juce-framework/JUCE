/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_TOOLBARBUTTON_H_INCLUDED
#define JUCE_TOOLBARBUTTON_H_INCLUDED


//==============================================================================
/**
    A type of button designed to go on a toolbar.

    This simple button can have two Drawable objects specified - one for normal
    use and another one (optionally) for the button's "on" state if it's a
    toggle button.

    @see Toolbar, ToolbarItemFactory, ToolbarItemComponent, Drawable, Button
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
    ~ToolbarButton();


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
    ScopedPointer<Drawable> normalImage, toggledOnImage;
    Drawable* currentImage;

    void updateDrawable();
    Drawable* getImageToUse() const;
    void setCurrentImage (Drawable*);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolbarButton)
};


#endif   // JUCE_TOOLBARBUTTON_H_INCLUDED
