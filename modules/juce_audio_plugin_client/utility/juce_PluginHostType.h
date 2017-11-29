/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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
        AdobeAudition,
        AdobePremierePro,
        AppleLogic,
        Ardour,
        BitwigStudio,
        CakewalkSonar8,
        CakewalkSonarGeneric,
        DaVinciResolve,
        DigidesignProTools,
        DigitalPerformer,
        FinalCut,
        FruityLoops,
        MagixSamplitude,
        MergingPyramix,
        MuseReceptorGeneric,
        Reaper,
        Renoise,
        SADiE,
        SteinbergCubase4,
        SteinbergCubase5,
        SteinbergCubase5Bridged,
        SteinbergCubase6,
        SteinbergCubase7,
        SteinbergCubase8,
        SteinbergCubase8_5,
        SteinbergCubase9,
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
        SteinbergTestHost,
        StudioOne,
        Tracktion3,
        TracktionGeneric,
        TracktionWaveform,
        VBVSTScanner,
        WaveBurner
    };

    HostType type;

    //==============================================================================
    bool isAbletonLive() const noexcept       { return type == AbletonLive6 || type == AbletonLive7 || type == AbletonLive8 || type == AbletonLiveGeneric; }
    bool isAdobeAudition() const noexcept     { return type == AdobeAudition; }
    bool isArdour() const noexcept            { return type == Ardour; }
    bool isBitwigStudio() const noexcept      { return type == BitwigStudio; }
    bool isCubase() const noexcept            { return type == SteinbergCubase4 || type == SteinbergCubase5 || type == SteinbergCubase5Bridged || type == SteinbergCubase6 || type == SteinbergCubase7 || type == SteinbergCubase8 || type == SteinbergCubase8_5 || type == SteinbergCubase9 || type == SteinbergCubaseGeneric; }
    bool isCubase7orLater() const noexcept    { return isCubase() && ! (type == SteinbergCubase4 || type == SteinbergCubase5 || type == SteinbergCubase6); }
    bool isCubaseBridged() const noexcept     { return type == SteinbergCubase5Bridged; }
    bool isDaVinciResolve() const noexcept    { return type == DaVinciResolve; }
    bool isDigitalPerformer() const noexcept  { return type == DigitalPerformer; }
    bool isFinalCut() const noexcept          { return type == FinalCut; }
    bool isFruityLoops() const noexcept       { return type == FruityLoops; }
    bool isLogic() const noexcept             { return type == AppleLogic; }
    bool isNuendo() const noexcept            { return type == SteinbergNuendo3 || type == SteinbergNuendo4  || type == SteinbergNuendo5 ||  type == SteinbergNuendoGeneric; }
    bool isPremiere() const noexcept          { return type == AdobePremierePro; }
    bool isProTools() const noexcept          { return type == DigidesignProTools; }
    bool isPyramix() const noexcept           { return type == MergingPyramix; }
    bool isReceptor() const noexcept          { return type == MuseReceptorGeneric; }
    bool isReaper() const noexcept            { return type == Reaper; }
    bool isRenoise() const noexcept           { return type == Renoise; }
    bool isSADiE() const noexcept             { return type == SADiE; }
    bool isSamplitude() const noexcept        { return type == MagixSamplitude; }
    bool isSonar() const noexcept             { return type == CakewalkSonar8 || type == CakewalkSonarGeneric; }
    bool isSteinbergTestHost() const noexcept { return type == SteinbergTestHost; }
    bool isSteinberg() const noexcept         { return isCubase() || isNuendo() || isWavelab() || isSteinbergTestHost(); }
    bool isStudioOne() const noexcept         { return type == StudioOne; }
    bool isTracktion() const noexcept         { return type == Tracktion3 || type == TracktionGeneric || isTracktionWaveform(); }
    bool isTracktionWaveform() const noexcept { return type == TracktionWaveform; }
    bool isVBVSTScanner() const noexcept      { return type == VBVSTScanner; }
    bool isWaveBurner() const noexcept        { return type == WaveBurner; }
    bool isWavelab() const noexcept           { return isWavelabLegacy() || type == SteinbergWavelab7 || type == SteinbergWavelab8 || type == SteinbergWavelabGeneric; }
    bool isWavelabLegacy() const noexcept     { return type == SteinbergWavelab5 || type == SteinbergWavelab6; }

    //==============================================================================
    const char* getHostDescription() const noexcept
    {
        switch (type)
        {
            case AbletonLive6:             return "Ableton Live 6";
            case AbletonLive7:             return "Ableton Live 7";
            case AbletonLive8:             return "Ableton Live 8";
            case AbletonLiveGeneric:       return "Ableton Live";
            case AdobeAudition:            return "Adobe Audition";
            case AdobePremierePro:         return "Adobe Premiere";
            case AppleLogic:               return "Apple Logic";
            case BitwigStudio:             return "Bitwig Studio";
            case CakewalkSonar8:           return "Cakewalk Sonar 8";
            case CakewalkSonarGeneric:     return "Cakewalk Sonar";
            case DaVinciResolve:           return "DaVinci Resolve";
            case DigidesignProTools:       return "ProTools";
            case DigitalPerformer:         return "DigitalPerformer";
            case FinalCut:                 return "Final Cut";
            case FruityLoops:              return "FruityLoops";
            case MagixSamplitude:          return "Magix Samplitude";
            case MergingPyramix:           return "Pyramix";
            case MuseReceptorGeneric:      return "Muse Receptor";
            case Reaper:                   return "Reaper";
            case Renoise:                  return "Renoise";
            case SADiE:                    return "SADiE";
            case SteinbergCubase4:         return "Steinberg Cubase 4";
            case SteinbergCubase5:         return "Steinberg Cubase 5";
            case SteinbergCubase5Bridged:  return "Steinberg Cubase 5 Bridged";
            case SteinbergCubase6:         return "Steinberg Cubase 6";
            case SteinbergCubase7:         return "Steinberg Cubase 7";
            case SteinbergCubase8:         return "Steinberg Cubase 8";
            case SteinbergCubase8_5:       return "Steinberg Cubase 8.5";
            case SteinbergCubase9:         return "Steinberg Cubase 9";
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
            case SteinbergTestHost:        return "Steinberg TestHost";
            case StudioOne:                return "Studio One";
            case Tracktion3:               return "Tracktion 3";
            case TracktionGeneric:         return "Tracktion";
            case VBVSTScanner:             return "VBVSTScanner";
            case WaveBurner:               return "WaveBurner";
            default:                       break;
        }

        return "Unknown";
    }

    //==============================================================================
    bool isInterAppAudioConnected() const;
    void switchToHostApplication() const;

   #if JUCE_MODULE_AVAILABLE_juce_gui_basics
    Image getHostIcon (int size) const;
   #endif

    //==============================================================================
    static String getHostPath()
    {
        return File::getSpecialLocation (File::hostApplicationPath).getFullPathName();
    }

    //==============================================================================
    /**
         Returns the plug-in format via which the plug-in file was loaded. This value is
         identical to AudioProcessor::wrapperType of the main audio processor of this
         plug-in. This function is useful for code that does not have access to the
         plug-in's main audio processor.

         @see AudioProcessor::wrapperType
    */
    static AudioProcessor::WrapperType getPluginLoadedAs() noexcept    { return jucePlugInClientCurrentWrapperType; }

    //==============================================================================

   #ifndef DOXYGEN
    // @internal
    static AudioProcessor::WrapperType jucePlugInClientCurrentWrapperType;
   #endif

private:
    static HostType getHostType()
    {
        auto hostPath = getHostPath();
        auto hostFilename = File (hostPath).getFileName();

       #if JUCE_MAC
        if (hostPath.containsIgnoreCase       ("Final Cut Pro.app")) return FinalCut;
        if (hostPath.containsIgnoreCase       ("Final Cut Pro Trial.app")) return FinalCut;
        if (hostPath.containsIgnoreCase       ("Live 6."))           return AbletonLive6;
        if (hostPath.containsIgnoreCase       ("Live 7."))           return AbletonLive7;
        if (hostPath.containsIgnoreCase       ("Live 8."))           return AbletonLive8;
        if (hostFilename.containsIgnoreCase   ("Live"))              return AbletonLiveGeneric;
        if (hostFilename.containsIgnoreCase   ("Adobe Premiere"))    return AdobePremierePro;
        if (hostFilename.contains             ("Logic"))             return AppleLogic;
        if (hostFilename.containsIgnoreCase   ("Pro Tools"))         return DigidesignProTools;
        if (hostFilename.containsIgnoreCase   ("Nuendo 3"))          return SteinbergNuendo3;
        if (hostFilename.containsIgnoreCase   ("Nuendo 4"))          return SteinbergNuendo4;
        if (hostFilename.containsIgnoreCase   ("Nuendo 5"))          return SteinbergNuendo5;
        if (hostFilename.containsIgnoreCase   ("Nuendo"))            return SteinbergNuendoGeneric;
        if (hostFilename.containsIgnoreCase   ("Cubase 4"))          return SteinbergCubase4;
        if (hostFilename.containsIgnoreCase   ("Cubase 5"))          return SteinbergCubase5;
        if (hostFilename.containsIgnoreCase   ("Cubase 6"))          return SteinbergCubase6;
        if (hostFilename.containsIgnoreCase   ("Cubase 7"))          return SteinbergCubase7;
        if (hostPath.containsIgnoreCase       ("Cubase 8.app"))      return SteinbergCubase8;
        if (hostPath.containsIgnoreCase       ("Cubase 8.5.app"))    return SteinbergCubase8_5;
        if (hostPath.containsIgnoreCase       ("Cubase 9.app"))      return SteinbergCubase9;
        if (hostFilename.containsIgnoreCase   ("Cubase"))            return SteinbergCubaseGeneric;
        if (hostPath.containsIgnoreCase       ("Wavelab 7"))         return SteinbergWavelab7;
        if (hostPath.containsIgnoreCase       ("Wavelab 8"))         return SteinbergWavelab8;
        if (hostFilename.containsIgnoreCase   ("Wavelab"))           return SteinbergWavelabGeneric;
        if (hostFilename.containsIgnoreCase   ("WaveBurner"))        return WaveBurner;
        if (hostPath.containsIgnoreCase       ("Digital Performer")) return DigitalPerformer;
        if (hostFilename.containsIgnoreCase   ("reaper"))            return Reaper;
        if (hostPath.containsIgnoreCase       ("Studio One"))        return StudioOne;
        if (hostFilename.startsWithIgnoreCase ("Waveform"))          return TracktionWaveform;
        if (hostPath.containsIgnoreCase       ("Tracktion 3"))       return Tracktion3;
        if (hostFilename.containsIgnoreCase   ("Tracktion"))         return TracktionGeneric;
        if (hostFilename.containsIgnoreCase   ("Renoise"))           return Renoise;
        if (hostFilename.containsIgnoreCase   ("Resolve"))           return DaVinciResolve;
        if (hostFilename.startsWith           ("Bitwig"))            return BitwigStudio;

       #elif JUCE_WINDOWS
        if (hostFilename.containsIgnoreCase   ("Live 6."))           return AbletonLive6;
        if (hostFilename.containsIgnoreCase   ("Live 7."))           return AbletonLive7;
        if (hostFilename.containsIgnoreCase   ("Live 8."))           return AbletonLive8;
        if (hostFilename.containsIgnoreCase   ("Live "))             return AbletonLiveGeneric;
        if (hostFilename.containsIgnoreCase   ("Audition"))          return AdobeAudition;
        if (hostFilename.containsIgnoreCase   ("Adobe Premiere"))    return AdobePremierePro;
        if (hostFilename.containsIgnoreCase   ("ProTools"))          return DigidesignProTools;
        if (hostPath.containsIgnoreCase       ("SONAR 8"))           return CakewalkSonar8;
        if (hostFilename.containsIgnoreCase   ("SONAR"))             return CakewalkSonarGeneric;
        if (hostFilename.containsIgnoreCase   ("Logic"))             return AppleLogic;
        if (hostFilename.startsWithIgnoreCase ("Waveform"))          return TracktionWaveform;
        if (hostPath.containsIgnoreCase       ("Tracktion 3"))       return Tracktion3;
        if (hostFilename.containsIgnoreCase   ("Tracktion"))         return TracktionGeneric;
        if (hostFilename.containsIgnoreCase   ("reaper"))            return Reaper;
        if (hostFilename.containsIgnoreCase   ("Cubase4"))           return SteinbergCubase4;
        if (hostFilename.containsIgnoreCase   ("Cubase5"))           return SteinbergCubase5;
        if (hostFilename.containsIgnoreCase   ("Cubase6"))           return SteinbergCubase6;
        if (hostFilename.containsIgnoreCase   ("Cubase7"))           return SteinbergCubase7;
        if (hostFilename.containsIgnoreCase   ("Cubase8.exe"))       return SteinbergCubase8;
        if (hostFilename.containsIgnoreCase   ("Cubase8.5.exe"))     return SteinbergCubase8_5;
        // Cubase 9 scans plug-ins with a separate executable "vst2xscanner"
        if (hostFilename.containsIgnoreCase   ("Cubase9.exe")
            || hostPath.containsIgnoreCase    ("Cubase 9"))          return SteinbergCubase9;
        if (hostFilename.containsIgnoreCase   ("Cubase"))            return SteinbergCubaseGeneric;
        if (hostFilename.containsIgnoreCase   ("VSTBridgeApp"))      return SteinbergCubase5Bridged;
        if (hostPath.containsIgnoreCase       ("Wavelab 5"))         return SteinbergWavelab5;
        if (hostPath.containsIgnoreCase       ("Wavelab 6"))         return SteinbergWavelab6;
        if (hostPath.containsIgnoreCase       ("Wavelab 7"))         return SteinbergWavelab7;
        if (hostPath.containsIgnoreCase       ("Wavelab 8"))         return SteinbergWavelab8;
        if (hostPath.containsIgnoreCase       ("Nuendo"))            return SteinbergNuendoGeneric;
        if (hostFilename.containsIgnoreCase   ("Wavelab"))           return SteinbergWavelabGeneric;
        if (hostFilename.containsIgnoreCase   ("TestHost"))          return SteinbergTestHost;
        if (hostFilename.containsIgnoreCase   ("rm-host"))           return MuseReceptorGeneric;
        if (hostFilename.startsWith           ("FL"))                return FruityLoops;
        if (hostFilename.contains             ("ilbridge."))         return FruityLoops;
        if (hostPath.containsIgnoreCase       ("Studio One"))        return StudioOne;
        if (hostPath.containsIgnoreCase       ("Digital Performer")) return DigitalPerformer;
        if (hostFilename.containsIgnoreCase   ("VST_Scanner"))       return VBVSTScanner;
        if (hostPath.containsIgnoreCase       ("Merging Technologies")) return MergingPyramix;
        if (hostFilename.startsWithIgnoreCase ("Sam"))               return MagixSamplitude;
        if (hostFilename.containsIgnoreCase   ("Renoise"))           return Renoise;
        if (hostFilename.containsIgnoreCase   ("Resolve"))           return DaVinciResolve;
        if (hostPath.containsIgnoreCase       ("Bitwig Studio"))     return BitwigStudio;
        if (hostFilename.containsIgnoreCase   ("Sadie"))             return SADiE;

       #elif JUCE_LINUX
        if (hostFilename.containsIgnoreCase   ("Ardour"))            return Ardour;
        if (hostFilename.startsWithIgnoreCase ("Waveform"))          return TracktionWaveform;
        if (hostFilename.containsIgnoreCase   ("Tracktion"))         return TracktionGeneric;
        if (hostFilename.startsWith           ("Bitwig"))            return BitwigStudio;

       #elif JUCE_IOS
       #elif JUCE_ANDROID
       #else
        #error
       #endif
        return UnknownHost;
    }
};

} // namespace juce
