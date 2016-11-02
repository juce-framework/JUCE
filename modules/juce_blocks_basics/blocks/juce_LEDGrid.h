/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

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

    /** A program that can be loaded onto an LEDGrid.

        This class facilitates the execution of a LittleFoot program on a BLOCKS
        device with an LEDGrid.
    */
    struct Program
    {
        /** Creates a Program for the corresponding LEDGrid. */
        Program (LEDGrid&);

        /** Destructor. */
        virtual ~Program();

        /** Returns the LittleFoot program to execute on the BLOCKS device. */
        virtual juce::String getLittleFootProgram() = 0;

        /** Sets the size of the shared area of memory used to communicate with
            the host computer.
        */
        virtual uint32 getHeapSize() = 0;

        LEDGrid& ledGrid;
    };

    /** Sets the Program to run on this LEDGrid.

        The supplied Program's lifetime will be managed by this class, so do not
        use the Program in other places in your code.
    */
    virtual juce::Result setProgram (Program*) = 0;

    /** Returns a pointer to the currently loaded program. */
    virtual Program* getProgram() const = 0;

    /** A message that can be sent to the currently loaded program. */
    struct ProgramEventMessage
    {
        int32 values[2];
    };

    /** Sends a message to the currently loaded program.

        To receive the message the program must provide a function called
        handleMessage with the following form:
        @code
        void handleMessage (int param1, int param2)
        {
            // Do something with the two integer parameters that the app has sent...
        }
        @endcode
    */
    virtual void sendProgramEvent (const ProgramEventMessage&) = 0;

    /** Sets a single byte on the heap. */
    virtual void setDataByte (size_t offset, uint8 value) = 0;

    /** Sets multiple bytes on the heap. */
    virtual void setDataBytes (size_t offset, const void* data, size_t num) = 0;

    /** Sets multiple bits on the heap. */
    virtual void setDataBits (uint32 startBit, uint32 numBits, uint32 value) = 0;

    /** Gets a byte from the heap. */
    virtual uint8 getDataByte (size_t offset) = 0;

    //==============================================================================
    struct Renderer
    {
        virtual ~Renderer();
        virtual void renderLEDGrid (LEDGrid&) = 0;
    };

    /** Set the visualiser that will create visuals for this block (nullptr for none).
        Note that the LEDGrid will NOT take ownership of this object, so the caller
        must ensure that it doesn't get deleted while in use here.
    */
    void setRenderer (Renderer* newRenderer) noexcept   { renderer = newRenderer; }

    /** Returns the visualiser currently attached to this block (nullptr for none). */
    Renderer* getRenderer() const noexcept              { return renderer; }

    /** The device that this LEDGrid belongs to. */
    Block& block;

private:
    Renderer* renderer = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEDGrid)
};
