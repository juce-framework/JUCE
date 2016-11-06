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


#include "juce_blocks_basics.h"

namespace juce
{
    namespace BlocksProtocol
    {
        #include "protocol/juce_BitPackingUtilities.h"
        #include "protocol/juce_BlocksProtocolDefinitions.h"
        #include "protocol/juce_HostPacketDecoder.h"
        #include "protocol/juce_HostPacketBuilder.h"
        #include "protocol/juce_BlockModels.h"
    }

    #include "blocks/juce_Block.cpp"
    #include "topology/juce_PhysicalTopologySource.cpp"
    #include "topology/juce_RuleBasedTopologySource.cpp"
    #include "visualisers/juce_DrumPadLEDProgram.cpp"
    #include "visualisers/juce_BitmapLEDProgram.cpp"
}
