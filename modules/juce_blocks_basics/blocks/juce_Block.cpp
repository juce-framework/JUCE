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


static Block::UID getBlockUIDFromSerialNumber (const uint8* serial) noexcept
{
    Block::UID n = {};

    for (int i = 0; i < (int) sizeof (BlocksProtocol::BlockSerialNumber); ++i)
        n += n * 127 + serial[i];

    return n;
}

static Block::UID getBlockUIDFromSerialNumber (const BlocksProtocol::BlockSerialNumber& serial) noexcept
{
    return getBlockUIDFromSerialNumber (serial.serial);
}

static Block::UID getBlockUIDFromSerialNumber (const juce::String& serial) noexcept
{
    if (serial.length() < (int) sizeof (BlocksProtocol::BlockSerialNumber))
    {
        jassertfalse;
        return getBlockUIDFromSerialNumber (serial.paddedRight ('0', sizeof (BlocksProtocol::BlockSerialNumber)));
    }

    return getBlockUIDFromSerialNumber ((const uint8*) serial.toRawUTF8());
}

Block::Block (const juce::String& serial)
   : serialNumber (serial), uid (getBlockUIDFromSerialNumber (serial))
{
}

Block::~Block() {}

void Block::addDataInputPortListener (DataInputPortListener* listener)      { dataInputPortListeners.add (listener); }
void Block::removeDataInputPortListener (DataInputPortListener* listener)   { dataInputPortListeners.remove (listener); }

bool Block::ConnectionPort::operator== (const ConnectionPort& other) const noexcept { return edge == other.edge && index == other.index; }
bool Block::ConnectionPort::operator!= (const ConnectionPort& other) const noexcept { return ! operator== (other); }

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

LEDGrid::Program::Program (LEDGrid& l) : ledGrid (l) {}
LEDGrid::Program::~Program() {}

LEDGrid::Renderer::~Renderer() {}

//==============================================================================
LEDRow::LEDRow (Block& b) : block (b) {}
LEDRow::~LEDRow() {}

//==============================================================================
StatusLight::StatusLight (Block& b) : block (b) {}
StatusLight::~StatusLight() {}
