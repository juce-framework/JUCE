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


/** Describes a phyiscal connection between two ports of two block devices. */
struct BlockDeviceConnection
{
    Block::UID device1, device2;
    Block::ConnectionPort connectionPortOnDevice1, connectionPortOnDevice2;

    bool operator== (const BlockDeviceConnection&) const noexcept;
    bool operator!= (const BlockDeviceConnection&) const noexcept;
};


/** Describes a set of blocks and the connections between them. */
struct BlockTopology
{
    Block::Array blocks;
    juce::Array<BlockDeviceConnection> connections;

    bool operator== (const BlockTopology&) const noexcept;
    bool operator!= (const BlockTopology&) const noexcept;
};
