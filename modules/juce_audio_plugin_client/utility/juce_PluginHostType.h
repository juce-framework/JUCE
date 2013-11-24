/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

//==============================================================================
class PluginHostType
{
public:
    //==============================================================================
    PluginHostType()  : type (getHostType()) {}
    PluginHostType (const PluginHostType& other) noexcept  : type (other.type) {}
    PluginHostType& operator= (const PluginHostType& other) noexcept { type = other.type; return *this; }

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
        Reaper,
        Tracktion3,
        TracktionGeneric,
        SteinbergCubase4,
        SteinbergCubase5,
        SteinbergCubase5Bridged,
        SteinbergCubase6,
        SteinbergCubase7,
        SteinbergCubaseGeneric,
        SteinbergNuendo3,
        SteinbergNuendo4,
        SteinbergNuendo5,
        SteinbergNuendoGeneric,
        SteinbergWavelab5,
        SteinbergWavelab6,
        SteinbergWavelab7,
        SteinbergWavelab8,
        SteinbergWavelabGeneric,
        MuseReceptorGeneric,
        MagixSamplitude,
        FruityLoops,
        WaveBurner,
        DigitalPerformer,
        StudioOne,
        MergingPyramix,
        VBVSTScanner
    };

    HostType type;

    //==============================================================================
    bool isAbletonLive() const noexcept      { return type == AbletonLive6 || type == AbletonLive7 || type == AbletonLive8 || type == AbletonLiveGeneric; }
    bool isNuendo() const noexcept           { return type == SteinbergNuendo3 || type == SteinbergNuendo4  || type == SteinbergNuendo5 ||  type == SteinbergNuendoGeneric; }
    bool isCubase() const noexcept           { return type == SteinbergCubase4 || type == SteinbergCubase5 || type == SteinbergCubase5Bridged || type == SteinbergCubase6 || type == SteinbergCubase7 || type == SteinbergCubaseGeneric; }
    bool isCubaseBridged() const noexcept    { return type == SteinbergCubase5Bridged; }
    bool isSteinberg() const noexcept        { return isCubase() || isNuendo() || isWavelab(); }
    bool isTracktion() const noexcept        { return type == Tracktion3 || type == TracktionGeneric; }
    bool isSonar() const noexcept            { return type == CakewalkSonar8 || type == CakewalkSonarGeneric; }
    bool isWavelab() const noexcept          { return isWavelabLegacy() || type == SteinbergWavelab7 || type == SteinbergWavelab8 || type == SteinbergWavelabGeneric; }
    bool isWavelabLegacy() const noexcept    { return type == SteinbergWavelab5 || type == SteinbergWavelab6; }
    bool isPremiere() const noexcept         { return type == AdobePremierePro; }
    bool isLogic() const noexcept            { return type == AppleLogic; }
    bool isReceptor() const noexcept         { return type == MuseReceptorGeneric; }
    bool isSamplitude() const noexcept       { return type == MagixSamplitude; }
    bool isFruityLoops() const noexcept      { return type == FruityLoops; }
    bool isWaveBurner() const noexcept       { return type == WaveBurner; }
    bool isDigitalPerformer() const noexcept { return type == DigitalPerformer; }
    bool isReaper() const noexcept           { return type == Reaper; }
    bool isStudioOne() const noexcept        { return type == StudioOne; }
    bool isPyramix() const noexcept          { return type == MergingPyramix; }
    bool isVBVSTScanner() const noexcept     { return type == VBVSTScanner; }

    //==============================================================================
    const char* getHostDescription() const noexcept
    {
        switch (type)
        {
            case AbletonLive6:             return "Ableton Live 6";
            case AbletonLive7:             return "Ableton Live 7";
            case AbletonLive8:             return "Ableton Live 8";
            case AbletonLiveGeneric:       return "Ableton Live";
            case AdobePremierePro:         return "Adobe Premiere";
            case AppleLogic:               return "Apple Logic";
            case CakewalkSonar8:           return "Cakewalk Sonar 8";
            case CakewalkSonarGeneric:     return "Cakewalk Sonar";
            case DigidesignProTools:       return "ProTools";
            case Reaper:                   return "Reaper";
            case Tracktion3:               return "Tracktion 3";
            case TracktionGeneric:         return "Tracktion";
            case SteinbergCubase4:         return "Steinberg Cubase 4";
            case SteinbergCubase5:         return "Steinberg Cubase 5";
            case SteinbergCubase5Bridged:  return "Steinberg Cubase 5 Bridged";
            case SteinbergCubase6:         return "Steinberg Cubase 6";
            case SteinbergCubase7:         return "Steinberg Cubase 7";
            case SteinbergCubaseGeneric:   return "Steinberg Cubase";
            case SteinbergNuendo3:         return "Steinberg Nuendo 3";
            case SteinbergNuendo4:         return "Steinberg Nuendo 4";
            case SteinbergNuendo5:         return "Steinberg Nuendo 5";
            case SteinbergNuendoGeneric:   return "Steinberg Nuendo";
            case SteinbergWavelab5:        return "Steinberg Wavelab 5";
            case SteinbergWavelab6:        return "Steinberg Wavelab 6";
            case SteinbergWavelab7:        return "Steinberg Wavelab 7";
            case SteinbergWavelab8:        return "Steinberg Wavelab 8";
            case SteinbergWavelabGeneric:  return "Steinberg Wavelab";
            case MuseReceptorGeneric:      return "Muse Receptor";
            case MagixSamplitude:          return "Magix Samplitude";
            case FruityLoops:              return "FruityLoops";
            case WaveBurner:               return "WaveBurner";
            case DigitalPerformer:         return "DigitalPerformer";
            case StudioOne:                return "Studio One";
            case MergingPyramix:           return "Pyramix";
            case VBVSTScanner:             return "VBVSTScanner";
            default:                       break;
        }

        return "Unknown";
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
        if (hostPath.containsIgnoreCase     ("Live 6."))           return AbletonLive6;
        if (hostPath.containsIgnoreCase     ("Live 7."))           return AbletonLive7;
        if (hostPath.containsIgnoreCase     ("Live 8."))           return AbletonLive8;
        if (hostFilename.containsIgnoreCase ("Live"))              return AbletonLiveGeneric;
        if (hostFilename.containsIgnoreCase ("Adobe Premiere"))    return AdobePremierePro;
        if (hostFilename.contains           ("Logic"))             return AppleLogic;
        if (hostFilename.containsIgnoreCase ("Pro Tools"))         return DigidesignProTools;
        if (hostFilename.containsIgnoreCase ("Nuendo 3"))          return SteinbergNuendo3;
        if (hostFilename.containsIgnoreCase ("Nuendo 4"))          return SteinbergNuendo4;
        if (hostFilename.containsIgnoreCase ("Nuendo 5"))          return SteinbergNuendo5;
        if (hostFilename.containsIgnoreCase ("Nuendo"))            return SteinbergNuendoGeneric;
        if (hostFilename.containsIgnoreCase ("Cubase 4"))          return SteinbergCubase4;
        if (hostFilename.containsIgnoreCase ("Cubase 5"))          return SteinbergCubase5;
        if (hostFilename.containsIgnoreCase ("Cubase 6"))          return SteinbergCubase6;
        if (hostFilename.containsIgnoreCase ("Cubase 7"))          return SteinbergCubase7;
        if (hostFilename.containsIgnoreCase ("Cubase"))            return SteinbergCubaseGeneric;
        if (hostPath.containsIgnoreCase     ("Wavelab 7"))         return SteinbergWavelab7;
        if (hostPath.containsIgnoreCase     ("Wavelab 8"))         return SteinbergWavelab8;
        if (hostFilename.containsIgnoreCase ("Wavelab"))           return SteinbergWavelabGeneric;
        if (hostFilename.containsIgnoreCase ("WaveBurner"))        return WaveBurner;
        if (hostFilename.contains           ("Digital Performer")) return DigitalPerformer;
        if (hostFilename.containsIgnoreCase ("reaper"))            return Reaper;
        if (hostPath.containsIgnoreCase     ("Studio One"))        return StudioOne;
        if (hostPath.containsIgnoreCase     ("Tracktion 3"))       return Tracktion3;
        if (hostFilename.containsIgnoreCase ("Tracktion"))         return TracktionGeneric;

      #elif JUCE_WINDOWS
        if (hostFilename.containsIgnoreCase ("Live 6."))           return AbletonLive6;
        if (hostFilename.containsIgnoreCase ("Live 7."))           return AbletonLive7;
        if (hostFilename.containsIgnoreCase ("Live 8."))           return AbletonLive8;
        if (hostFilename.containsIgnoreCase ("Live "))             return AbletonLiveGeneric;
        if (hostFilename.containsIgnoreCase ("Adobe Premiere"))    return AdobePremierePro;
        if (hostFilename.containsIgnoreCase ("ProTools"))          return DigidesignProTools;
        if (hostPath.containsIgnoreCase     ("SONAR 8"))           return CakewalkSonar8;
        if (hostFilename.containsIgnoreCase ("SONAR"))             return CakewalkSonarGeneric;
        if (hostFilename.containsIgnoreCase ("Logic"))             return AppleLogic;
        if (hostPath.containsIgnoreCase     ("Tracktion 3"))       return Tracktion3;
        if (hostFilename.containsIgnoreCase ("Tracktion"))         return TracktionGeneric;
        if (hostFilename.containsIgnoreCase ("reaper"))            return Reaper;
        if (hostFilename.containsIgnoreCase ("Cubase4"))           return SteinbergCubase4;
        if (hostFilename.containsIgnoreCase ("Cubase5"))           return SteinbergCubase5;
        if (hostFilename.containsIgnoreCase ("Cubase6"))           return SteinbergCubase6;
        if (hostFilename.containsIgnoreCase ("Cubase7"))           return SteinbergCubase7;
        if (hostFilename.containsIgnoreCase ("Cubase"))            return SteinbergCubaseGeneric;
        if (hostFilename.containsIgnoreCase ("VSTBridgeApp"))      return SteinbergCubase5Bridged;
        if (hostPath.containsIgnoreCase     ("Wavelab 5"))         return SteinbergWavelab5;
        if (hostPath.containsIgnoreCase     ("Wavelab 6"))         return SteinbergWavelab6;
        if (hostPath.containsIgnoreCase     ("Wavelab 7"))         return SteinbergWavelab7;
        if (hostPath.containsIgnoreCase     ("Wavelab 8"))         return SteinbergWavelab8;
        if (hostPath.containsIgnoreCase     ("Nuendo"))            return SteinbergNuendoGeneric;
        if (hostFilename.containsIgnoreCase ("Wavelab"))           return SteinbergWavelabGeneric;
        if (hostFilename.containsIgnoreCase ("rm-host"))           return MuseReceptorGeneric;
        if (hostFilename.startsWith         ("FL"))                return FruityLoops;
        if (hostPath.containsIgnoreCase     ("Studio One"))        return StudioOne;
        if (hostPath.containsIgnoreCase     ("Digital Performer")) return DigitalPerformer;
        if (hostFilename.containsIgnoreCase ("VST_Scanner"))       return VBVSTScanner;
        if (hostPath.containsIgnoreCase     ("Merging Technologies")) return MergingPyramix;
        if (hostFilename.startsWithIgnoreCase ("Sam"))             return MagixSamplitude;

       #elif JUCE_LINUX
        jassertfalse   // not yet done!
       #else
        #error
       #endif
        return UnknownHost;
    }
};
