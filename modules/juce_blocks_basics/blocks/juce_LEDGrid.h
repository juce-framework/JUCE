/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/


/**
    A simple ARGB colour class for setting LEDs.
*/
struct LEDColour
{
    LEDColour() noexcept = default;
    LEDColour (const LEDColour&) noexcept = default;
    LEDColour& operator= (const LEDColour&) noexcept = default;

    LEDColour (uint32 argbColour) noexcept : argb (argbColour) {}

    template <typename ColourType>
    LEDColour (const ColourType& colour)  : LEDColour (colour.getARGB()) {}

    uint8 getAlpha() const noexcept         { return (uint8) (argb >> 24); }
    uint8 getRed() const noexcept           { return (uint8) (argb >> 16); }
    uint8 getGreen() const noexcept         { return (uint8) (argb >> 8); }
    uint8 getBlue() const noexcept          { return (uint8) argb; }

    uint32 getARGB() const noexcept         { return argb; }

    uint32 argb = 0;
};


//==============================================================================
/**
    Represents a 2D grid of LEDs on a block device.
*/
class LEDGrid
{
public:
    LEDGrid (Block&);

    /** Destructor. */
    virtual ~LEDGrid();

    //==============================================================================
    /** Returns the number of columns in the LED grid. */
    virtual int getNumColumns() const = 0;

    /** Returns the number of rows in the LED grid. */
    virtual int getNumRows() const = 0;

    //==============================================================================
    struct Renderer     : public juce::ReferenceCountedObject
    {
        virtual ~Renderer();
        virtual void renderLEDGrid (LEDGrid&) = 0;

        /** The Renderer class is reference-counted, so always use a Renderer::Ptr when
            you are keeping references to them.
         */
        using Ptr = juce::ReferenceCountedObjectPtr<Renderer>;
    };

    /** Set the visualiser that will create visuals for this block (nullptr for none).
        Note that the LEDGrid will NOT take ownership of this object, so the caller
        must ensure that it doesn't get deleted while in use here.
    */
    void setRenderer (Renderer::Ptr newRenderer) noexcept   { renderer = newRenderer; }

    /** Returns the visualiser currently attached to this block (nullptr for none). */
    Renderer::Ptr getRenderer() const noexcept              { return renderer; }

    /** The device that this LEDGrid belongs to. */
    Block& block;

private:
    Renderer::Ptr renderer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEDGrid)
};
