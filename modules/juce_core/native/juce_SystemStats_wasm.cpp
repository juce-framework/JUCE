/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
