/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

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

#include "juce_UMPProtocols.h"
#include "juce_UMPUtils.h"
#include "juce_UMPacket.h"
#include "juce_UMPSysEx7.h"
#include "juce_UMPView.h"
#include "juce_UMPIterator.h"
#include "juce_UMPackets.h"
#include "juce_UMPFactory.h"
#include "juce_UMPConversion.h"
#include "juce_UMPMidi1ToBytestreamTranslator.h"
#include "juce_UMPMidi1ToMidi2DefaultTranslator.h"
#include "juce_UMPConverters.h"
#include "juce_UMPDispatcher.h"
#include "juce_UMPReceiver.h"

#ifndef DOXYGEN

namespace juce
{
namespace ump = universal_midi_packets;
}

#endif
