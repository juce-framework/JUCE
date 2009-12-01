/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#if JUCE_MAC
 #include <mach-o/dyld.h>
#endif

//==============================================================================
class PluginHostType
{
public:
    //==============================================================================
    PluginHostType() throw()  : type (getHostType())
    {
    }

    //==============================================================================
    enum HostType
    {
        UnknownHost,
        AbletonLive6,
        AbletonLive7,
        AbletonLive8,
        AbletonLiveGeneric,
        AppleLogic,
        DigidesignProTools,
        CakewalkSonar8,
        CakewalkSonarGeneric,
        Reaper,
        MackieTracktion3,
        MackieTracktionGeneric,
        SteinbergCubase4,
        SteinbergCubase5,
        SteinbergCubaseGeneric,
    };

    const HostType type;

    //==============================================================================
    bool isAbletonLive() const throw()
    {
        return type == AbletonLive6 || type == AbletonLive7 || type == AbletonLive8 || type == AbletonLiveGeneric;
    }

    bool isCubase() const throw()
    {
        return type == SteinbergCubase4 || type == SteinbergCubase5 || type == SteinbergCubaseGeneric;
    }

    bool isTracktion() const throw()
    {
        return type == MackieTracktion3 || type == MackieTracktionGeneric;
    }

    bool isSonar() const throw()
    {
        return type == CakewalkSonar8 || type == CakewalkSonarGeneric;
    }

    //==============================================================================
private:
    static HostType getHostType() throw()
    {
        const String hostPath (getHostPath());
        const String hostFilename (File (hostPath).getFileName());

#if JUCE_MAC
        if (hostPath.containsIgnoreCase (T("Live 6.")))        return AbletonLive6;
        if (hostPath.containsIgnoreCase (T("Live 7.")))        return AbletonLive7;
        if (hostPath.containsIgnoreCase (T("Live 8.")))        return AbletonLive8;
        if (hostFilename.containsIgnoreCase (T("Live")))       return AbletonLiveGeneric;
        if (hostFilename.containsIgnoreCase (T("Pro Tools")))  return DigidesignProTools;
        if (hostFilename.containsIgnoreCase (T("Cubase 4")))   return SteinbergCubase4;
        if (hostFilename.containsIgnoreCase (T("Cubase 5")))   return SteinbergCubase5;
        if (hostFilename.contains (T("Logic")))                return AppleLogic;

#elif JUCE_WINDOWS
        if (hostFilename.containsIgnoreCase (T("Live 6.")))    return AbletonLive6;
        if (hostFilename.containsIgnoreCase (T("Live 7.")))    return AbletonLive7;
        if (hostFilename.containsIgnoreCase (T("Live 8.")))    return AbletonLive8;
        if (hostFilename.containsIgnoreCase (T("Live ")))      return AbletonLiveGeneric;
        if (hostFilename.containsIgnoreCase (T("ProTools")))   return DigidesignProTools;
        if (hostPath.containsIgnoreCase (T("SONAR 8")))        return CakewalkSonar8;
        if (hostFilename.containsIgnoreCase (T("SONAR")))      return CakewalkSonarGeneric;
        if (hostPath.containsIgnoreCase (T("Tracktion 3")))    return MackieTracktion3;
        if (hostFilename.containsIgnoreCase (T("Tracktion")))  return MackieTracktionGeneric;
        if (hostFilename.containsIgnoreCase (T("Cubase4")))    return SteinbergCubase4;
        if (hostFilename.containsIgnoreCase (T("Cubase5")))    return SteinbergCubase5;
        if (hostFilename.containsIgnoreCase (T("Cubase")))     return SteinbergCubaseGeneric;
        if (hostFilename.containsIgnoreCase (T("reaper")))     return Reaper;

#elif JUCE_LINUX
        jassertfalse   // not yet done!
#else
        #error
#endif
        return UnknownHost;
    }

    static const String getHostPath() throw()
    {
        unsigned int size = 8192;
        MemoryBlock buffer (size + 8);
        buffer.fillWith (0);

#if JUCE_WINDOWS
        GetModuleFileNameW (0, (WCHAR*) buffer.getData(), size / sizeof (WCHAR));
#elif JUCE_MAC
        _NSGetExecutablePath ((char*) buffer.getData(), &size);
#elif JUCE_LINUX
        readlink ("/proc/self/exe", (char*) buffer.getData(), size);
#else
        #error
#endif
        return String::fromUTF8 ((const JUCE_NAMESPACE::uint8*) buffer.getData(), size);
    }
};
