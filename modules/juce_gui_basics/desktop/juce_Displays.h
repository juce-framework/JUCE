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
    Manages details about connected display devices.

    @tags{GUI}
*/
class JUCE_API  Displays
{
private:
    Displays (Desktop&);

public:
    /** Represents a connected display device. */
    struct Display
    {
        /** This will be true if this is the user's main display device. */
        bool isMain;

        /** The total area of this display in logical pixels including any OS-dependent objects
            like the taskbar, menu bar, etc. */
        Rectangle<int> totalArea;

        /** The total area of this display in logical pixels which isn't covered by OS-dependent
            objects like the taskbar, menu bar, etc.
        */
        Rectangle<int> userArea;

        /** The top-left of this display in physical coordinates. */
        Point<int> topLeftPhysical;

        /** The scale factor of this display.

            For higher-resolution displays, or displays with a user-defined scale factor set,
            this may be a value other than 1.0.

            This value is used to convert between physical and logical pixels. For example, a Component
            with size 10x10 will use 20x20 physical pixels on a display with a scale factor of 2.0.
        */
        double scale;

        /** The DPI of the display.

            This is the number of physical pixels per inch. To get the number of logical
            pixels per inch, divide this by the Display::scale value.
        */
        double dpi;
    };

    /** Converts a Rectangle from physical to logical pixels.

        If useScaleFactorOfDisplay is not null then its scale factor will be used for the conversion
        regardless of the display that the Rectangle to be converted is on.
    */
    Rectangle<int> physicalToLogical (Rectangle<int> physicalRect, const Display* useScaleFactorOfDisplay = nullptr) const noexcept;

    /** Converts a Rectangle from logical to physical pixels.

        If useScaleFactorOfDisplay is not null then its scale factor will be used for the conversion
        regardless of the display that the Rectangle to be converted is on.
    */
    Rectangle<int> logicalToPhysical (Rectangle<int> logicalRect, const Display* useScaleFactorOfDisplay = nullptr) const noexcept;

    /** Converts a Point from physical to logical pixels. */
    template <typename ValueType>
    Point<ValueType> physicalToLogical (Point<ValueType> physicalPoint, const Display* useScaleFactorOfDisplay = nullptr) const noexcept;

    /** Converts a Point from logical to physical pixels. */
    template <typename ValueType>
    Point<ValueType> logicalToPhysical (Point<ValueType> logicalPoint, const Display* useScaleFactorOfDisplay = nullptr) const noexcept;

    /** Returns the Display object representing the display containing a given Rectangle (either
        in logical or physical pixels), or nullptr if there are no connected displays.

        If the Rectangle lies outside all the displays then the nearest one will be returned.
    */
    const Display* getDisplayForRect (Rectangle<int> rect, bool isPhysical = false) const noexcept;

    /** Returns the Display object representing the display containing a given Point (either
        in logical or physical pixels), or nullptr if there are no connected displays.

        If the Point lies outside all the displays then the nearest one will be returned.
    */
    const Display* getDisplayForPoint (Point<int> point, bool isPhysical = false) const noexcept;

    /** Returns the Display object representing the display acting as the user's main screen, or nullptr
        if there are no connected displays.
    */
    const Display* getPrimaryDisplay() const noexcept;

    /** Returns a RectangleList made up of all the displays in LOGICAL pixels. */
    RectangleList<int> getRectangleList (bool userAreasOnly) const;

    /** Returns the smallest bounding box which contains all the displays in LOGICAL pixels. */
    Rectangle<int> getTotalBounds (bool userAreasOnly) const;

    /** An Array containing the Display objects for all of the connected displays. */
    Array<Display> displays;

   #ifndef DOXYGEN
    /** @internal */
    void refresh();
    /** @internal */
    ~Displays() = default;
    // This method has been deprecated - use the getDisplayForPoint() or getDisplayForRect() methods instead
    // as they can deal with converting between logical and physical pixels
    JUCE_DEPRECATED (const Display& getDisplayContaining (Point<int> position) const noexcept);

    // These methods have been deprecated - use the methods which return a Display* instead as they will return
    // nullptr on headless systems with no connected displays
    JUCE_DEPRECATED (const Display& findDisplayForRect (Rectangle<int>, bool isPhysical = false) const noexcept);
    JUCE_DEPRECATED (const Display& findDisplayForPoint (Point<int>, bool isPhysical = false) const noexcept);
    JUCE_DEPRECATED (const Display& getMainDisplay() const noexcept);
   #endif

private:
    friend class Desktop;

    void init (Desktop&);
    void findDisplays (float masterScale);

    void updateToLogical();

    Display emptyDisplay;
};

} // namespace juce
