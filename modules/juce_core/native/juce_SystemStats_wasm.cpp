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

namespace juce
{

void Logger::outputDebugString (const String& text)
{
    std::cerr << text << std::endl;
}

//==============================================================================
SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()  { return WASM; }
String SystemStats::getOperatingSystemName()    { return "WASM"; }
bool SystemStats::isOperatingSystem64Bit()      { return true; }
String SystemStats::getDeviceDescription()      { return "Web-browser"; }
String SystemStats::getDeviceManufacturer()     { return {}; }
String SystemStats::getCpuVendor()              { return {}; }
String SystemStats::getCpuModel()               { return {}; }
int SystemStats::getCpuSpeedInMegahertz()       { return 0; }
int SystemStats::getMemorySizeInMegabytes()     { return 0; }
int SystemStats::getPageSize()                  { return 0; }
String SystemStats::getLogonName()              { return {}; }
String SystemStats::getFullUserName()           { return {}; }
String SystemStats::getComputerName()           { return {}; }
String SystemStats::getUserLanguage()           { return {}; }
String SystemStats::getUserRegion()             { return {}; }
String SystemStats::getDisplayLanguage()        { return {}; }

//==============================================================================
void CPUInformation::initialise() noexcept
{
    numLogicalCPUs = 1;
    numPhysicalCPUs = 1;
}

//==============================================================================
uint32 juce_millisecondsSinceStartup() noexcept
{
    return static_cast<uint32> (emscripten_get_now());
}

int64 Time::getHighResolutionTicks() noexcept
{
    return static_cast<int64> (emscripten_get_now() * 1000.0);
}

int64 Time::getHighResolutionTicksPerSecond() noexcept
{
    return 1000000;  // (microseconds)
}

double Time::getMillisecondCounterHiRes() noexcept
{
    return emscripten_get_now();
}

bool Time::setSystemTimeToThisTime() const
{
    return false;
}

JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger() noexcept
{
    return false;
}

} // namespace juce
