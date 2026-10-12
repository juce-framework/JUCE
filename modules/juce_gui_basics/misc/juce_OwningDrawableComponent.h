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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

/**
    Helper class that owns a Drawable object and exposes it as a DrawableComponent. This class is
    meant to ease the transition from JUCE 8 to JUCE 9, where Drawable no longer inherits from
    Component and DrawableComponent takes no ownership of the Drawable object.

    New projects should avoid using this class if possible.

    The new suggested way to work with Drawable objects is to use them directly and draw them using
    the Drawable::draw() method. This way a heavy dependency on the Component class, and potentially
    the juce_gui_basics module can be avoided.

    @see Drawable

    @tags{GUI}
*/
class JUCE_API  OwningDrawableComponent : private std::unique_ptr<Drawable>,
                                          public DrawableComponent
{
public:
    /** Creates an OwningDrawableComponent that takes ownership of the underlying Drawable object.
    */
    static std::unique_ptr<OwningDrawableComponent> create (std::unique_ptr<Drawable> d, BoundsToEnclose boundsToEnclose)
    {
        if (d == nullptr)
            return {};

        return rawToUniquePtr (new OwningDrawableComponent (std::move (d), boundsToEnclose));
    }

    /** Creates an OwningDrawableComponent that takes ownership of the underlying Drawable object.
    */
    static std::unique_ptr<OwningDrawableComponent> create (std::unique_ptr<Drawable> d)
    {
        return create (std::move (d), BoundsToEnclose::drawableBounds);
    }

    /** Creates an OwningDrawableComponent that copies a Drawable and takes ownership of the copied
        object.
    */
    static std::unique_ptr<OwningDrawableComponent> createFromCopy (const Drawable* const d, BoundsToEnclose boundsToEnclose)
    {
        if (d == nullptr)
            return {};

        auto copy = d->createCopy();

        if (copy == nullptr)
            return {};

        return rawToUniquePtr (new OwningDrawableComponent (std::move (copy), boundsToEnclose));
    }

    /** Creates an OwningDrawableComponent that copies a Drawable and takes ownership of the copied
        object.
    */
    static std::unique_ptr<OwningDrawableComponent> createFromCopy (const Drawable* const d)
    {
        return createFromCopy (d, BoundsToEnclose::drawableBounds);
    }

    /** Attempts to parse an SVG (Scalable Vector Graphics) document from a file.
    */
    static std::unique_ptr<OwningDrawableComponent> createFromSVGFile (const File& svgFile)
    {
        return create (Drawable::createFromSVGFile (svgFile));
    }

    /** Attempts to parse an SVG (Scalable Vector Graphics) document from a string.
    */
    static std::unique_ptr<OwningDrawableComponent> createFromSVGString (const String& svgString)
    {
        return create (Drawable::createFromSVGString (svgString));
    }

private:
    OwningDrawableComponent (std::unique_ptr<Drawable> drawableIn, BoundsToEnclose boundsToEncloseIn)
        : unique_ptr (std::move (drawableIn)),
          DrawableComponent (*get(), boundsToEncloseIn)
    {
    }

    OwningDrawableComponent (std::unique_ptr<Drawable> drawableIn)
        : OwningDrawableComponent (std::move (drawableIn), BoundsToEnclose::drawableBounds)
    {
    }
};

} // namespace juce
