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
    A component that simply displays an image.

    Use setImage to give it an image, and it'll display it - simple as that!

    @tags{GUI}
*/
class JUCE_API  ImageComponent  : public Component,
                                  public SettableTooltipClient
{
public:
    //==============================================================================
    /** Creates an ImageComponent. */
    ImageComponent (const String& componentName = String());

    /** Destructor. */
    ~ImageComponent() override;

    //==============================================================================
    /** Sets the image that should be displayed. */
    void setImage (const Image& newImage);

    /** Sets the image that should be displayed, and its placement within the component. */
    void setImage (const Image& newImage,
                   RectanglePlacement placementToUse);

    /** Returns the current image. */
    const Image& getImage() const;

    /** Sets the method of positioning that will be used to fit the image within the component's bounds.
        By default the positioning is centred, and will fit the image inside the component's bounds
        whilst keeping its aspect ratio correct, but you can change it to whatever layout you need.
    */
    void setImagePlacement (RectanglePlacement newPlacement);

    /** Returns the current image placement. */
    RectanglePlacement getImagePlacement() const;

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

    Image image;
    RectanglePlacement placement;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageComponent)
};

} // namespace juce
