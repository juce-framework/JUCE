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

/** Specifies the source bounds that a DrawableComponent should fit itself around.

    New projects should use contentBounds, as this will respect the designer's intent when
    specifying a viewBox in an SVG file. Even better, avoid using DrawableComponent
    entirely, which is meant as a backwards compatibility class for projects migrating from
    JUCE 8.

    @see DrawableComponent
*/
enum class BoundsToEnclose
{
    drawableBounds, ///< The bounds returned by Drawable::getDrawableBounds(). These are the
                    ///< smallest rectangular bounds covering visible features of the Drawable.
                    ///< The Drawable class prior to JUCE 9 used these bounds for fitting.
    contentBounds   ///< The bounds returned by Drawable::getContentBounds(). For Drawables
                    ///< that were created by parsing an SVG, this will return the viewBox, which
                    ///< expresses the designer's intent for fitting purposes. The fallback
                    ///< value is the same as the one returned by Drawable::getDrawableBounds().
};

class JUCE_API DrawableComponent : public Component,
                                   private Drawable::Listener
{
public:
    /** Wraps a Drawable object in a Component. */
    explicit DrawableComponent (Drawable& drawableIn);

    /** Wraps a Drawable object in a Component. */
    DrawableComponent (Drawable& drawableIn, BoundsToEnclose boundsToEnclose);

    /** Destructor. */
    ~DrawableComponent() override;

    /** Sets a transform for this drawable that will position it within the specified
        area of its parent component.
    */
    void setTransformToFit (const Rectangle<float>& areaInParent, RectanglePlacement placement);

    /** Creates a path that describes the outline of this DrawableComponent. */
    Path getOutlineAsPath() const;

    /** Returns a reference to the Drawable object displayed by this Component. */
    Drawable& getDrawable()
    {
        return drawable;
    }

    /** Returns a reference to the Drawable object displayed by this Component. */
    const Drawable& getDrawable() const
    {
        return drawable;
    }

    //==============================================================================
    void paint (Graphics& g) override;

    bool hitTest (int x, int y) override;

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

    //==============================================================================
    /** @internal

        Intended to replace Drawable::setBoundsToEnclose which is only ever called in conjunction
        with getDrawableBounds().
    */
    void resetComponentBoundsTo (Rectangle<float> bounds);

private:
    void drawableBoundsChanged (Drawable*) override;

    Drawable& drawable;
    Point<int> originRelativeToComponent;
    BoundsToEnclose boundsToEnclose;
};

} // namespace juce
