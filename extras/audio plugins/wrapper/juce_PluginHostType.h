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
    PluginHostType()  : type (getHostType())
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
        AdobePremierePro,
        AppleLogic,
        CakewalkSonar8,
        CakewalkSonarGeneric,
        DigidesignProTools,
        EmagicLogic,
        Reaper,
        MackieTracktion3,
        MackieTracktionGeneric,
        SteinbergCubase4,
        SteinbergCubase5,
        SteinbergCubase5Bridged,
        SteinbergCubaseGeneric,
        SteinbergWavelab5,
        SteinbergWavelab6,
        SteinbergWavelab7,
        SteinbergWavelabGeneric,
        MuseReceptorGeneric,
        MagixSamplitude
    };

    const HostType type;

    //==============================================================================
    bool isAbletonLive() const throw()
    {
        return type == AbletonLive6 || type == AbletonLive7 || type == AbletonLive8 || type == AbletonLiveGeneric;
    }

    bool isCubase() const throw()
    {
        return type == SteinbergCubase4 || type == SteinbergCubase5 || type == SteinbergCubase5Bridged || type == SteinbergCubaseGeneric;
    }

    bool isCubaseBridged() const throw()
    {
        return type == SteinbergCubase5Bridged;
    }

    bool isTracktion() const throw()
    {
        return type == MackieTracktion3 || type == MackieTracktionGeneric;
    }

    bool isSonar() const throw()
    {
        return type == CakewalkSonar8 || type == CakewalkSonarGeneric;
    }

    bool isWavelab() const throw()
    {
        return type == SteinbergWavelab5 || type == SteinbergWavelab6 || type == SteinbergWavelab7 || type == SteinbergWavelabGeneric;
    }

    bool isWavelabLegacy() const throw()
    {
        return type == SteinbergWavelab5 || type == SteinbergWavelab6;
    }

    bool isPremiere() const throw()
    {
        return type == AdobePremierePro;
    }

    bool isLogic() const throw()
    {
        return type == AppleLogic || type == EmagicLogic;
    }

    bool isReceptor() const throw()
    {
        return type == MuseReceptorGeneric;
    }

    bool isSamplitude() const throw()
    {
        return type == MagixSamplitude;
    }

    //==============================================================================
    static String getHostPath()
    {
        return File::getSpecialLocation (File::hostApplicationPath).getFullPathName();
    }

    //==============================================================================
private:
    static HostType getHostType()
    {
        const String hostPath (getHostPath());
        const String hostFilename (File (hostPath).getFileName());

      #if JUCE_MAC
        if (hostPath.containsIgnoreCase     ("Live 6."))        return AbletonLive6;
        if (hostPath.containsIgnoreCase     ("Live 7."))        return AbletonLive7;
        if (hostPath.containsIgnoreCase     ("Live 8."))        return AbletonLive8;
        if (hostFilename.containsIgnoreCase ("Live"))           return AbletonLiveGeneric;
        if (hostFilename.containsIgnoreCase ("Adobe Premiere")) return AdobePremierePro;
        if (hostFilename.contains           ("Logic"))          return AppleLogic;
        if (hostFilename.containsIgnoreCase ("Pro Tools"))      return DigidesignProTools;
        if (hostFilename.containsIgnoreCase ("Cubase 4"))       return SteinbergCubase4;
        if (hostFilename.containsIgnoreCase ("Cubase 5"))       return SteinbergCubase5;
        if (hostPath.containsIgnoreCase     ("Wavelab 7"))      return SteinbergWavelab7;
        if (hostFilename.containsIgnoreCase ("Wavelab"))        return SteinbergWavelabGeneric;

      #elif JUCE_WINDOWS
        if (hostFilename.containsIgnoreCase ("Live 6."))        return AbletonLive6;
        if (hostFilename.containsIgnoreCase ("Live 7."))        return AbletonLive7;
        if (hostFilename.containsIgnoreCase ("Live 8."))        return AbletonLive8;
        if (hostFilename.containsIgnoreCase ("Live "))          return AbletonLiveGeneric;
        if (hostFilename.containsIgnoreCase ("Adobe Premiere")) return AdobePremierePro;
        if (hostFilename.containsIgnoreCase ("ProTools"))       return DigidesignProTools;
        if (hostPath.containsIgnoreCase     ("SONAR 8"))        return CakewalkSonar8;
        if (hostFilename.containsIgnoreCase ("SONAR"))          return CakewalkSonarGeneric;
        if (hostFilename.containsIgnoreCase ("Logic"))          return EmagicLogic;
        if (hostPath.containsIgnoreCase     ("Tracktion 3"))    return MackieTracktion3;
        if (hostFilename.containsIgnoreCase ("Tracktion"))      return MackieTracktionGeneric;
        if (hostFilename.containsIgnoreCase ("reaper"))         return Reaper;
        if (hostFilename.containsIgnoreCase ("Cubase4"))        return SteinbergCubase4;
        if (hostFilename.containsIgnoreCase ("Cubase5"))        return SteinbergCubase5;
        if (hostFilename.containsIgnoreCase ("Cubase"))         return SteinbergCubaseGeneric;
        if (hostFilename.containsIgnoreCase ("VSTBridgeApp"))   return SteinbergCubase5Bridged;
        if (hostPath.containsIgnoreCase     ("Wavelab 5"))      return SteinbergWavelab5;
        if (hostPath.containsIgnoreCase     ("Wavelab 6"))      return SteinbergWavelab6;
        if (hostPath.containsIgnoreCase     ("Wavelab 7"))      return SteinbergWavelab7;
        if (hostFilename.containsIgnoreCase ("Wavelab"))        return SteinbergWavelabGeneric;
        if (hostFilename.containsIgnoreCase ("rm-host"))        return MuseReceptorGeneric;
        if (hostFilename.startsWithIgnoreCase ("Sam"))          return MagixSamplitude;

      #elif JUCE_LINUX
        jassertfalse   // not yet done!
      #else
        #error
      #endif
        return UnknownHost;
    }

    JUCE_DECLARE_NON_COPYABLE (PluginHostType);
};
