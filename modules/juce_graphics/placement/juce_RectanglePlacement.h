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
    Defines the method used to position some kind of rectangular object within
    a rectangular viewport.

    Although similar to Justification, this is more specific, and has some extra
    options.
*/
class JUCE_API  RectanglePlacement
{
public:
    //==============================================================================
    /** Creates a RectanglePlacement object using a combination of flags from the Flags enum. */
    inline RectanglePlacement (int placementFlags) noexcept  : flags (placementFlags) {}

    /** Creates a default RectanglePlacement object, which is equivalent to using the 'centred' flag. */
    inline RectanglePlacement() noexcept                     : flags (centred) {}

    /** Creates a copy of another RectanglePlacement object. */
    RectanglePlacement (const RectanglePlacement&) noexcept;

    /** Copies another RectanglePlacement object. */
    RectanglePlacement& operator= (const RectanglePlacement&) noexcept;

    bool operator== (const RectanglePlacement&) const noexcept;
    bool operator!= (const RectanglePlacement&) const noexcept;

    //==============================================================================
    /** Flag values that can be combined and used in the constructor. */
    enum Flags
    {
        //==============================================================================
        /** Indicates that the source rectangle's left edge should be aligned with the left edge of the target rectangle. */
        xLeft                                   = 1,

        /** Indicates that the source rectangle's right edge should be aligned with the right edge of the target rectangle. */
        xRight                                  = 2,

        /** Indicates that the source should be placed in the centre between the left and right
            sides of the available space. */
        xMid                                    = 4,

        //==============================================================================
        /** Indicates that the source's top edge should be aligned with the top edge of the
            destination rectangle. */
        yTop                                    = 8,

        /** Indicates that the source's bottom edge should be aligned with the bottom edge of the
            destination rectangle. */
        yBottom                                 = 16,

        /** Indicates that the source should be placed in the centre between the top and bottom
            sides of the available space. */
        yMid                                    = 32,

        //==============================================================================
        /** If this flag is set, then the source rectangle will be resized to completely fill
            the destination rectangle, and all other flags are ignored.
        */
        stretchToFit                            = 64,

        //==============================================================================
        /** If this flag is set, then the source rectangle will be resized so that it is the
            minimum size to completely fill the destination rectangle, without changing its
            aspect ratio. This means that some of the source rectangle may fall outside
            the destination.

            If this flag is not set, the source will be given the maximum size at which none
            of it falls outside the destination rectangle.
        */
        fillDestination                         = 128,

        /** Indicates that the source rectangle can be reduced in size if required, but should
            never be made larger than its original size.
        */
        onlyReduceInSize                        = 256,

        /** Indicates that the source rectangle can be enlarged if required, but should
            never be made smaller than its original size.
        */
        onlyIncreaseInSize                      = 512,

        /** Indicates that the source rectangle's size should be left unchanged.
        */
        doNotResize                             = (onlyIncreaseInSize | onlyReduceInSize),

        //==============================================================================
        /** A shorthand value that is equivalent to (xMid | yMid). */
        centred                                 = 4 + 32
    };

    //==============================================================================
    /** Returns the raw flags that are set for this object. */
    inline int getFlags() const noexcept                            { return flags; }

    /** Tests a set of flags for this object.

        @returns true if any of the flags passed in are set on this object.
    */
    inline bool testFlags (int flagsToTest) const noexcept          { return (flags & flagsToTest) != 0; }


    //==============================================================================
    /** Adjusts the position and size of a rectangle to fit it into a space.

        The source rectangle coordinates will be adjusted so that they fit into
        the destination rectangle based on this object's flags.
    */
    void applyTo (double& sourceX,
                  double& sourceY,
                  double& sourceW,
                  double& sourceH,
                  double destinationX,
                  double destinationY,
                  double destinationW,
                  double destinationH) const noexcept;

    /** Returns the rectangle that should be used to fit the given source rectangle
        into the destination rectangle using the current flags.
    */
    template <typename ValueType>
    Rectangle<ValueType> appliedTo (const Rectangle<ValueType>& source,
                                    const Rectangle<ValueType>& destination) const noexcept
    {
        double x = source.getX(), y = source.getY(), w = source.getWidth(), h = source.getHeight();
        applyTo (x, y, w, h, static_cast<double> (destination.getX()), static_cast<double> (destination.getY()),
                 static_cast<double> (destination.getWidth()), static_cast<double> (destination.getHeight()));
        return Rectangle<ValueType> (static_cast<ValueType> (x), static_cast<ValueType> (y),
                                     static_cast<ValueType> (w), static_cast<ValueType> (h));
    }

    /** Returns the transform that should be applied to these source coordinates to fit them
        into the destination rectangle using the current flags.
    */
    AffineTransform getTransformToFit (const Rectangle<float>& source,
                                       const Rectangle<float>& destination) const noexcept;


private:
    //==============================================================================
    int flags;
};

} // namespace juce
