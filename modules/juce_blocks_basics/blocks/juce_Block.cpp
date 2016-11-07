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
