/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_IMAGECOMPONENT_JUCEHEADER__
#define __JUCE_IMAGECOMPONENT_JUCEHEADER__

#include "../components/juce_Component.h"
#include "../mouse/juce_TooltipClient.h"


//==============================================================================
/**
    A component that simply displays an image.

    Use setImage to give it an image, and it'll display it - simple as that!
*/
class JUCE_API  ImageComponent  : public Component,
                                  public SettableTooltipClient
{
public:
    //==============================================================================
    /** Creates an ImageComponent. */
    ImageComponent (const String& componentName = String::empty);

    /** Destructor. */
    ~ImageComponent();

    //==============================================================================
    /** Sets the image that should be displayed. */
    void setImage (const Image& newImage);

    /** Sets the image that should be displayed, and its placement within the component. */
    void setImage (const Image& newImage,
                   const RectanglePlacement& placementToUse);

    /** Returns the current image. */
    const Image& getImage() const;

    /** Sets the method of positioning that will be used to fit the image within the component's bounds.
        By default the positioning is centred, and will fit the image inside the component's bounds
        whilst keeping its aspect ratio correct, but you can change it to whatever layout you need.
    */
    void setImagePlacement (const RectanglePlacement& newPlacement);

    /** Returns the current image placement. */
    const RectanglePlacement getImagePlacement() const;

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);

private:
    Image image;
    RectanglePlacement placement;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageComponent)
};


#endif   // __JUCE_IMAGECOMPONENT_JUCEHEADER__
