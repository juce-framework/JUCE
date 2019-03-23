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

static Block::UID getBlockUIDFromSerialNumber (const uint8* serial) noexcept
{
    Block::UID n = {};

    for (int i = 0; i < int (BlocksProtocol::BlockSerialNumber::maxLength); ++i)
        n += n * 127 + serial[i];

    return n;
}

static Block::UID getBlockUIDFromSerialNumber (const BlocksProtocol::BlockSerialNumber& serial) noexcept
{
    return getBlockUIDFromSerialNumber (serial.data);
}

static Block::UID getBlockUIDFromSerialNumber (const String& serial) noexcept
{
    if (serial.length() < int (BlocksProtocol::BlockSerialNumber::maxLength))
    {
        jassertfalse;
        return getBlockUIDFromSerialNumber (serial.paddedRight ('0', BlocksProtocol::BlockSerialNumber::maxLength));
    }

    return getBlockUIDFromSerialNumber ((const uint8*) serial.toRawUTF8());
}

Block::Block (const String& serial)
   : serialNumber (serial), uid (getBlockUIDFromSerialNumber (serial))
{
}

Block::Block (const String& serial, const String& version, const String& blockName)
   : serialNumber (serial), versionNumber (version), name (blockName), uid (getBlockUIDFromSerialNumber (serial))
{
}

Block::~Block() {}

bool Block::isControlBlock() const
{
    return isControlBlock (getType());
}

bool Block::isControlBlock (Block::Type type)
{
    return type == Block::Type::liveBlock
        || type == Block::Type::loopBlock
        || type == Block::Type::touchBlock
        || type == Block::Type::developerControlBlock;
}

void Block::addDataInputPortListener (DataInputPortListener* listener)      { dataInputPortListeners.add (listener); }
void Block::removeDataInputPortListener (DataInputPortListener* listener)   { dataInputPortListeners.remove (listener); }

void Block::addProgramEventListener (ProgramEventListener* listener)        { programEventListeners.add (listener); }
void Block::removeProgramEventListener (ProgramEventListener* listener)     { programEventListeners.remove (listener); }


bool Block::ConnectionPort::operator== (const ConnectionPort& other) const noexcept { return edge == other.edge && index == other.index; }
bool Block::ConnectionPort::operator!= (const ConnectionPort& other) const noexcept { return ! operator== (other); }

Block::Program::Program (Block& b) : block (b) {}
Block::Program::~Program() {}

//==============================================================================
TouchSurface::TouchSurface (Block& b) : block (b) {}
TouchSurface::~TouchSurface() {}

TouchSurface::Listener::~Listener() {}

void TouchSurface::addListener (Listener* l)            { listeners.add (l); }
void TouchSurface::removeListener (Listener* l)         { listeners.remove (l); }

//==============================================================================
ControlButton::ControlButton (Block& b) : block (b) {}
ControlButton::~ControlButton() {}

ControlButton::Listener::~Listener() {}

void ControlButton::addListener (Listener* l)           { listeners.add (l); }
void ControlButton::removeListener (Listener* l)        { listeners.remove (l); }


//==============================================================================
LEDGrid::LEDGrid (Block& b) : block (b) {}
LEDGrid::~LEDGrid() {}

LEDGrid::Renderer::~Renderer() {}

//==============================================================================
LEDRow::LEDRow (Block& b) : block (b) {}
LEDRow::~LEDRow() {}

//==============================================================================
StatusLight::StatusLight (Block& b) : block (b) {}
StatusLight::~StatusLight() {}

} // namespace juce
