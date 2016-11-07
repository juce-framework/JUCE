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
    Represents an individual BLOCKS device.
*/
class Block   : public juce::ReferenceCountedObject
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~Block();

    /** The different block types.

        @see Block::getType()
    */
    enum Type
    {
        unknown = 0,
        lightPadBlock,
        liveBlock,
        loopBlock,
        developerControlBlock
    };

    /** The Block class is reference-counted, so always use a Block::Ptr when
        you are keeping references to them.
    */
    using Ptr = juce::ReferenceCountedObjectPtr<Block>;

    /** The Block class is reference-counted, so Block::Array is useful when
        you are storing lists of them.
    */
    using Array = juce::ReferenceCountedArray<Block>;

    /** The Block's serial number. */
    const juce::String serialNumber;

    using UID = uint64;

    /** This Block's UID.

        This will be globally unique, and remains constant for a particular device.
    */
    const UID uid;

    //==============================================================================
    /** Returns the type of this device.

        @see Block::Type
    */
    virtual Type getType() const = 0;

    /** Returns a human-readable description of this device type. */
    virtual juce::String getDeviceDescription() const = 0;

    /** Returns the battery level in the range 0.0 to 1.0. */
    virtual float getBatteryLevel() const = 0;

    /** Returns true if the battery is charging. */
    virtual bool isBatteryCharging() const = 0;

    //==============================================================================
    /** Returns true if this block is connected and active. */
    virtual bool isConnected() const = 0;

    /** Returns true if this block is directly connected to the application,
        as opposed to only being connected to a different block via a connection port.

        @see ConnectionPort
    */
    virtual bool isMasterBlock() const = 0;

    //==============================================================================
    /** Returns the width of the device in logical device units. */
    virtual int getWidth() const = 0;

    /** Returns the height of the device in logical device units. */
    virtual int getHeight() const = 0;

    /** Returns true if the device is a physical hardware block (i.e. not a virtual block). */
    virtual bool isHardwareBlock() const = 0;

    /** Returns the length of one logical device unit as physical millimeters. */
    virtual float getMillimetersPerUnit() const = 0;

    //==============================================================================
    /** If this block has a grid of LEDs, this will return an object to control it.
        Note that the pointer that is returned belongs to this object, and the caller must
        neither delete it or use it after the lifetime of this Block object has finished.
        If there are no LEDs, then this method will return nullptr.
    */
    virtual LEDGrid* getLEDGrid() const = 0;

    /** If this block has a row of LEDs, this will return an object to control it.
        Note that the pointer that is returned belongs to this object, and the caller must
        neither delete it or use it after the lifetime of this Block object has finished.
        If there are no LEDs, then this method will return nullptr.
    */
    virtual LEDRow* getLEDRow() const = 0;

    /** If this block has any status LEDs, this will return an array of objects to control them.
        Note that the objects in the array belong to this Block object, and the caller must
        neither delete them or use them after the lifetime of this Block object has finished.
    */
    virtual juce::Array<StatusLight*> getStatusLights() const = 0;

    /** If this block has a pressure-sensitive surface, this will return an object to
        access its data.
        Note that the pointer returned does is owned by this object, and the caller must
        neither delete it or use it after the lifetime of this Block object has finished.
        If the device is not touch-sensitive, then this method will return nullptr.
    */
    virtual TouchSurface* getTouchSurface() const = 0;

    /** If this block has any control buttons, this will return an array of objects to control them.
        Note that the objects in the array belong to this Block object, and the caller must
        neither delete them or use them after the lifetime of this Block object has finished.
    */
    virtual juce::Array<ControlButton*> getButtons() const = 0;

    //==============================================================================
    /** This returns true if the block supports generating graphics by drawing into a JUCE
        Graphics context. This should only be true for virtual on-screen blocks; hardware
        blocks will instead use the LED Grid for visuals.
    */
    virtual bool supportsGraphics() const = 0;

    //==============================================================================
    /** These are the edge-connectors that a device may have. */
    struct ConnectionPort
    {
        enum class DeviceEdge
        {
            north,
            south,
            east,
            west
        };

        /** The side of the device on which this port is located. */
        DeviceEdge edge;

        /** The index of this port along the device edge.
            For north and south edges, index 0 is the left-most port.
            For east and west edges, index 0 is the top-most port.
        */
        int index;

        bool operator== (const ConnectionPort&) const noexcept;
        bool operator!= (const ConnectionPort&) const noexcept;
    };

    /** Returns a list of the connectors that this device has. */
    virtual juce::Array<ConnectionPort> getPorts() const = 0;

    //==============================================================================
    /** Interface for objects listening to input data port. */
    struct DataInputPortListener
    {
        virtual ~DataInputPortListener() {}

        /** Called whenever a message from a block is received. */
        virtual void handleIncomingDataPortMessage (Block& source, const void* messageData, size_t messageSize) = 0;
    };

    /** Adds a new listener of data input port. */
    virtual void addDataInputPortListener (DataInputPortListener*);

    /** Removes a listener of data input port. */
    virtual void removeDataInputPortListener (DataInputPortListener*);

    /** Sends a message to the block. */
    virtual void sendMessage (const void* messageData, size_t messageSize) = 0;

    //==============================================================================
    /** This type is used for timestamping events. It represents a number of milliseconds since the block
        device was booted.
    */
    using Timestamp = uint32;

protected:
    //==============================================================================
    Block (const juce::String& serialNumberToUse);

    juce::ListenerList<DataInputPortListener> dataInputPortListeners;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Block)
};
