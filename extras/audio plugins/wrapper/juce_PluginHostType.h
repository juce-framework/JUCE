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
        EmagicLogic,
        DigidesignProTools,
        CakewalkSonar8,
        CakewalkSonarGeneric,
        Reaper,
        MackieTracktion3,
        MackieTracktionGeneric,
        SteinbergCubase4,
        SteinbergCubase5,
        SteinbergCubaseGeneric,
        SteinbergWavelab5,
        SteinbergWavelab6,
        SteinbergWavelabGeneric,
        MuseReceptorGeneric
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

    bool isLogic() const throw()
    {
        return type == AppleLogic || type == EmagicLogic;
    }

    bool isWavelab() const throw()
    {
        return type == SteinbergWavelabGeneric || type == SteinbergWavelab5 || type == SteinbergWavelab6;
    }

    bool isReceptor() const throw()
    {
        return type == MuseReceptorGeneric;
    }

    //==============================================================================
private:
    static HostType getHostType() throw()
    {
        const String hostPath (File::getSpecialLocation (File::hostApplicationPath).getFullPathName());
        const String hostFilename (File (hostPath).getFileName());

#if JUCE_MAC
        if (hostPath.containsIgnoreCase ("Live 6."))        return AbletonLive6;
        if (hostPath.containsIgnoreCase ("Live 7."))        return AbletonLive7;
        if (hostPath.containsIgnoreCase ("Live 8."))        return AbletonLive8;
        if (hostFilename.containsIgnoreCase ("Live"))       return AbletonLiveGeneric;
        if (hostFilename.containsIgnoreCase ("Pro Tools"))  return DigidesignProTools;
        if (hostFilename.containsIgnoreCase ("Cubase 4"))   return SteinbergCubase4;
        if (hostFilename.containsIgnoreCase ("Cubase 5"))   return SteinbergCubase5;
        if (hostFilename.contains ("Logic"))                return AppleLogic;

#elif JUCE_WINDOWS
        if (hostFilename.containsIgnoreCase ("Live 6."))    return AbletonLive6;
        if (hostFilename.containsIgnoreCase ("Live 7."))    return AbletonLive7;
        if (hostFilename.containsIgnoreCase ("Live 8."))    return AbletonLive8;
        if (hostFilename.containsIgnoreCase ("Live "))      return AbletonLiveGeneric;
        if (hostFilename.containsIgnoreCase ("ProTools"))   return DigidesignProTools;
        if (hostPath.containsIgnoreCase ("SONAR 8"))        return CakewalkSonar8;
        if (hostFilename.containsIgnoreCase ("SONAR"))      return CakewalkSonarGeneric;
        if (hostPath.containsIgnoreCase ("Tracktion 3"))    return MackieTracktion3;
        if (hostFilename.containsIgnoreCase ("Tracktion"))  return MackieTracktionGeneric;
        if (hostFilename.containsIgnoreCase ("Cubase4"))    return SteinbergCubase4;
        if (hostFilename.containsIgnoreCase ("Cubase5"))    return SteinbergCubase5;
        if (hostFilename.containsIgnoreCase ("Cubase"))     return SteinbergCubaseGeneric;
        if (hostFilename.containsIgnoreCase ("Wavelab 5"))  return SteinbergWavelab5;
        if (hostFilename.containsIgnoreCase ("Wavelab 6" )) return SteinbergWavelab6;
        if (hostFilename.containsIgnoreCase ("Wavelab"))    return SteinbergWavelabGeneric;
        if (hostFilename.containsIgnoreCase ("reaper"))     return Reaper;
        if (hostFilename.containsIgnoreCase ("Logic"))      return EmagicLogic;
        if (hostFilename.containsIgnoreCase ("rm-host"))    return MuseReceptorGeneric;


#elif JUCE_LINUX
        jassertfalse   // not yet done!
#else
        #error
#endif
        return UnknownHost;
    }
};
