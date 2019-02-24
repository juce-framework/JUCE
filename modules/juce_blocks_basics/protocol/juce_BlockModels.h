/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace BlocksProtocol
{

#ifndef DOXYGEN

// This file isn't part of the public API, it's where we encode the knowledge base
// of all the different types of block we know about..
struct BlockDataSheet
{
    BlockDataSheet (const BlocksProtocol::BlockSerialNumber& serial)  : serialNumber (serial)
    {
        if (serialNumber.isPadBlock())      initialiseForPadBlock2x2();
        if (serialNumber.isLiveBlock())     initialiseForControlBlockLive();
        if (serialNumber.isLoopBlock())     initialiseForControlBlockLoop();
        if (serialNumber.isDevCtrlBlock())  initialiseForControlBlockDeveloper();
        if (serialNumber.isTouchBlock())    initialiseForControlBlockTouch();
        if (serialNumber.isSeaboardBlock()) initialiseForSeaboardBlock();
    }

    Block::ConnectionPort convertPortIndexToConnectorPort (BlocksProtocol::ConnectorPort port) const noexcept
    {
        return ports[(int) port.get()];
    }

    const BlocksProtocol::BlockSerialNumber serialNumber;
    Block::Type apiType = Block::Type::unknown;

    const char* description = nullptr;

    int widthUnits = 0, heightUnits = 0;

    int lightGridWidth = 0, lightGridHeight = 0, lightGridStartIndex = 0;

    bool hasTouchSurface = false;
    int numKeywaves = 0;

    int numLEDRowLEDs = 0;

    uint32 programAndHeapSize = 0;

    struct ButtonInfo
    {
        ControlButton::ButtonFunction type;
        float x, y;
    };

    struct StatusLEDInfo
    {
        String name;
        float x, y;
    };

    Array<ButtonInfo> buttons;
    Array<StatusLEDInfo> statusLEDs;
    Array<Block::ConnectionPort> ports;
    Array<const char*> dials;

private:
    //==============================================================================
    void initialiseForPadBlock2x2()
    {
        apiType = Block::Type::lightPadBlock;

        description = "Pad BLOCK (2x2)";

        widthUnits  = 2;
        heightUnits = 2;

        lightGridWidth = 15;
        lightGridHeight = 15;

        addPorts (2, 2, 2, 2);

        hasTouchSurface = true;
        programAndHeapSize = BlocksProtocol::padBlockProgramAndHeapSize;

        addModeButton();
    }

    void initialiseForControlBlockLoop()
    {
        initialiseControlBlock ("Loop BLOCK", Block::Type::loopBlock,
                                ControlButton::ButtonFunction::mode,
                                ControlButton::ButtonFunction::volume,
                                ControlButton::ButtonFunction::click,
                                ControlButton::ButtonFunction::snap,
                                ControlButton::ButtonFunction::back,
                                ControlButton::ButtonFunction::playOrPause,
                                ControlButton::ButtonFunction::record,
                                ControlButton::ButtonFunction::learn,
                                ControlButton::ButtonFunction::down,
                                ControlButton::ButtonFunction::up);
    }

    void initialiseForControlBlockLive()
    {
        initialiseControlBlock ("Live BLOCK", Block::Type::liveBlock,
                                ControlButton::ButtonFunction::mode,
                                ControlButton::ButtonFunction::volume,
                                ControlButton::ButtonFunction::scale,
                                ControlButton::ButtonFunction::chord,
                                ControlButton::ButtonFunction::arp,
                                ControlButton::ButtonFunction::sustain,
                                ControlButton::ButtonFunction::octave,
                                ControlButton::ButtonFunction::love,
                                ControlButton::ButtonFunction::down,
                                ControlButton::ButtonFunction::up);
    }

    void initialiseForControlBlockDeveloper()
    {
        initialiseControlBlock ("Dev Ctrl BLOCK", Block::Type::developerControlBlock,
                                ControlButton::ButtonFunction::button0,
                                ControlButton::ButtonFunction::button1,
                                ControlButton::ButtonFunction::button2,
                                ControlButton::ButtonFunction::button3,
                                ControlButton::ButtonFunction::button4,
                                ControlButton::ButtonFunction::button5,
                                ControlButton::ButtonFunction::button6,
                                ControlButton::ButtonFunction::button7,
                                ControlButton::ButtonFunction::down,
                                ControlButton::ButtonFunction::up);
    }

    void initialiseForControlBlockTouch()
    {
        initialiseControlBlock ("Touch BLOCK", Block::Type::touchBlock,
                                ControlButton::ButtonFunction::velocitySensitivity,
                                ControlButton::ButtonFunction::glideSensitivity,
                                ControlButton::ButtonFunction::slideSensitivity,
                                ControlButton::ButtonFunction::pressSensitivity,
                                ControlButton::ButtonFunction::liftSensitivity,
                                ControlButton::ButtonFunction::fixedVelocity,
                                ControlButton::ButtonFunction::glideLock,
                                ControlButton::ButtonFunction::pianoMode,
                                ControlButton::ButtonFunction::down,
                                ControlButton::ButtonFunction::up);
    }

    void initialiseControlBlock (const char* name, Block::Type type,
                                 ControlButton::ButtonFunction b1, ControlButton::ButtonFunction b2,
                                 ControlButton::ButtonFunction b3, ControlButton::ButtonFunction b4,
                                 ControlButton::ButtonFunction b5, ControlButton::ButtonFunction b6,
                                 ControlButton::ButtonFunction b7, ControlButton::ButtonFunction b8,
                                 ControlButton::ButtonFunction b9, ControlButton::ButtonFunction b10)
    {
        apiType = type;

        description = name;

        widthUnits  = 2;
        heightUnits = 1;

        programAndHeapSize = BlocksProtocol::controlBlockProgramAndHeapSize;

        addPorts (2, 1, 2, 1);

        float x1 = 0.2f;
        float x2 = 0.6f;
        float x3 = 1.0f;
        float x4 = 1.4f;
        float x5 = 1.8f;
        float y1 = 0.405f;
        float y2 = 0.798f;

        addButtons (b1,  x1, y1,
                    b2,  x2, y1,
                    b3,  x3, y1,
                    b4,  x4, y1,
                    b5,  x5, y1,
                    b6,  x1, y2,
                    b7,  x2, y2,
                    b8,  x3, y2,
                    b9,  x4, y2,
                    b10, x5, y2);

        numLEDRowLEDs = 15;
    }

    void initialiseForSeaboardBlock()
    {
        apiType = Block::Type::seaboardBlock;

        description = "Seaboard BLOCK (6x3)";

        widthUnits  = 6;
        heightUnits = 3;

        lightGridWidth = 0;
        lightGridHeight = 0;
        numKeywaves = 24;

        addPortsSW (Block::ConnectionPort::DeviceEdge::west,  1);
        addPortsNE (Block::ConnectionPort::DeviceEdge::north, 2);
        addPortsNE (Block::ConnectionPort::DeviceEdge::east,  1);

        hasTouchSurface = true;
        programAndHeapSize = BlocksProtocol::padBlockProgramAndHeapSize;

        addModeButton();
    }

    //==============================================================================
    void addStatusLED (const char* name, float x, float y)
    {
        statusLEDs.add ({ name, x, y });
    }

    template <typename... Args>
    void addButtons (ControlButton::ButtonFunction fn, float x, float y, Args... others)
    {
        addButtons (fn, x, y);
        addButtons (others...);
    }

    void addButtons (ControlButton::ButtonFunction fn, float x, float y)
    {
        buttons.add ({ fn, x, y });
    }

    void addModeButton()
    {
        addButtons (ControlButton::ButtonFunction::mode, -1.0f, -1.0f);
    }

    void addPorts (int numNorth, int numEast, int numSouth, int numWest)
    {
        addPortsNE (Block::ConnectionPort::DeviceEdge::north, numNorth);
        addPortsNE (Block::ConnectionPort::DeviceEdge::east,  numEast);
        addPortsSW (Block::ConnectionPort::DeviceEdge::south, numSouth);
        addPortsSW (Block::ConnectionPort::DeviceEdge::west,  numWest);
    }

    void addPortsNE (Block::ConnectionPort::DeviceEdge edge, int num)
    {
        for (int i = 0; i < num; ++i)
            ports.add ({ edge, i});
    }

    void addPortsSW (Block::ConnectionPort::DeviceEdge edge, int num)
    {
        for (int i = 0; i < num; ++i)
            ports.add ({ edge, num - i - 1});
    }
};

//==============================================================================
static const char* getButtonNameForFunction (ControlButton::ButtonFunction fn) noexcept
{
    using BF = ControlButton::ButtonFunction;

    switch (fn)
    {
        case BF::mode:          return "Mode";

        case BF::volume:        return "Volume";
        case BF::up:            return "Up";
        case BF::down:          return "Down";

        case BF::scale:         return "Scale";
        case BF::chord:         return "Chord";
        case BF::arp:           return "Arp";
        case BF::sustain:       return "Sustain";
        case BF::octave:        return "Octave";
        case BF::love:          return "Love";

        case BF::click:         return "Click";
        case BF::snap:          return "Snap";
        case BF::back:          return "Back";
        case BF::playOrPause:   return "Play/Pause";
        case BF::record:        return "Record";
        case BF::learn:         return "Learn";

        case BF::button0:       return "0";
        case BF::button1:       return "1";
        case BF::button2:       return "2";
        case BF::button3:       return "3";
        case BF::button4:       return "4";
        case BF::button5:       return "5";
        case BF::button6:       return "6";
        case BF::button7:       return "7";

        case BF::velocitySensitivity:   return "Velocity Sensitivity";
        case BF::glideSensitivity:      return "Glide Sensitivity";
        case BF::slideSensitivity:      return "Slide Sensitivity";
        case BF::pressSensitivity:      return "Press Sensitivity";
        case BF::liftSensitivity:       return "Lift Sensitivity";
        case BF::fixedVelocity: return "Fixed Velocity";
        case BF::glideLock:     return "Glide Lock";
        case BF::pianoMode:     return "Piano Mode";
    }

    jassertfalse;
    return nullptr;
}

#endif

} // namespace BlocksProtocol
} // namespace juce
