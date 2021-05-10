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
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    Image image;
    RectanglePlacement placement;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageComponent)
};

} // namespace juce
