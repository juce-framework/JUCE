/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
/**
     A useful utility class to determine the host or DAW in which your plugin is
     loaded.

     Declare a PluginHostType object in your class to use it.

    @tags{Audio}
*/
class PluginHostType
{
public:
    //==============================================================================
    PluginHostType()  : type (getHostType()) {}
    PluginHostType (const PluginHostType& other) = default;
    PluginHostType& operator= (const PluginHostType& other) = default;

    //==============================================================================
    /** Represents the host type and also its version for some hosts. */
    enum HostType
    {
        UnknownHost,                /**< Represents an unknown host. */
        AbletonLive6,               /**< Represents Ableton Live 6. */
        AbletonLive7,               /**< Represents Ableton Live 7. */
        AbletonLive8,               /**< Represents Ableton Live 8. */
        AbletonLive9,               /**< Represents Ableton Live 9. */
        AbletonLive10,              /**< Represents Ableton Live 10. */
        AbletonLive11,              /**< Represents Ableton Live 11. */
        AbletonLiveGeneric,         /**< Represents Ableton Live. */
        AdobeAudition,              /**< Represents Adobe Audition. */
        AdobePremierePro,           /**< Represents Adobe Premiere Pro. */
        AppleGarageBand,            /**< Represents Apple GarageBand. */
        AppleLogic,                 /**< Represents Apple Logic Pro. */
        AppleMainStage,             /**< Represents Apple Main Stage. */
        Ardour,                     /**< Represents Ardour. */
        AULab,                      /**< Represents AU Lab. */
        AvidProTools,               /**< Represents Avid Pro Tools. */
        BitwigStudio,               /**< Represents Bitwig Studio. */
        CakewalkSonar8,             /**< Represents Cakewalk Sonar 8. */
        CakewalkSonarGeneric,       /**< Represents Cakewalk Sonar. */
        CakewalkByBandlab,          /**< Represents Cakewalk by Bandlab. */
        DaVinciResolve,             /**< Represents DaVinci Resolve. */
        DigitalPerformer,           /**< Represents Digital Performer. */
        FinalCut,                   /**< Represents Apple Final Cut Pro. */
        FruityLoops,                /**< Represents Fruity Loops. */
        JUCEPluginHost,             /**< Represents the JUCE AudioPluginHost */
        MagixSamplitude,            /**< Represents Magix Samplitude. */
        MagixSequoia,               /**< Represents Magix Sequoia. */
        MergingPyramix,             /**< Represents Merging Pyramix. */
        MuseReceptorGeneric,        /**< Represents Muse Receptor. */
        pluginval,                  /**< Represents pluginval. */
        Reaper,                     /**< Represents Cockos Reaper. */
        Reason,                     /**< Represents Reason. */
        Renoise,                    /**< Represents Renoise. */
        SADiE,                      /**< Represents SADiE. */
        SteinbergCubase4,           /**< Represents Steinberg Cubase 4. */
        SteinbergCubase5,           /**< Represents Steinberg Cubase 5. */
        SteinbergCubase5Bridged,    /**< Represents Steinberg Cubase 5 Bridged. */
        SteinbergCubase6,           /**< Represents Steinberg Cubase 6. */
        SteinbergCubase7,           /**< Represents Steinberg Cubase 7. */
        SteinbergCubase8,           /**< Represents Steinberg Cubase 8. */
        SteinbergCubase8_5,         /**< Represents Steinberg Cubase 8.5. */
        SteinbergCubase9,           /**< Represents Steinberg Cubase 9. */
        SteinbergCubase9_5,         /**< Represents Steinberg Cubase 9.5. */
        SteinbergCubase10,          /**< Represents Steinberg Cubase 10. */
        SteinbergCubase10_5,        /**< Represents Steinberg Cubase 10.5. */
        SteinbergCubaseGeneric,     /**< Represents Steinberg Cubase. */
        SteinbergNuendo3,           /**< Represents Steinberg Nuendo 3. */
        SteinbergNuendo4,           /**< Represents Steinberg Nuendo 4. */
        SteinbergNuendo5,           /**< Represents Steinberg Nuendo 5. */
        SteinbergNuendoGeneric,     /**< Represents Steinberg Nuendo. */
        SteinbergWavelab5,          /**< Represents Steinberg Wavelab 5. */
        SteinbergWavelab6,          /**< Represents Steinberg Wavelab 6. */
        SteinbergWavelab7,          /**< Represents Steinberg Wavelab 7. */
        SteinbergWavelab8,          /**< Represents Steinberg Wavelab 8. */
        SteinbergWavelabGeneric,    /**< Represents Steinberg Wavelab. */
        SteinbergTestHost,          /**< Represents Steinberg's VST3 Test Host. */
        StudioOne,                  /**< Represents PreSonus Studio One. */
        Tracktion3,                 /**< Represents Tracktion 3. */
        TracktionGeneric,           /**< Represents Tracktion. */
        TracktionWaveform,          /**< Represents Tracktion Waveform. */
        VBVSTScanner,               /**< Represents VB Audio VST Scanner. */
        ViennaEnsemblePro,          /**< Represents Vienna Ensemble Pro. */
        WaveBurner                  /**< Represents Apple WaveBurner. */
    };

    HostType type;

    //==============================================================================
    /** Returns true if the host is any version of Ableton Live. */
    bool isAbletonLive() const noexcept       { return type == AbletonLive6 || type == AbletonLive7 || type == AbletonLive8
                                                      || type == AbletonLive9 || type == AbletonLive10 || type == AbletonLive11
                                                      || type == AbletonLiveGeneric; }
    /** Returns true if the host is Adobe Audition. */
    bool isAdobeAudition() const noexcept     { return type == AdobeAudition; }
    /** Returns true if the host is Ardour. */
    bool isArdour() const noexcept            { return type == Ardour; }
    /** Returns true if the host is AU Lab. */
    bool isAULab() const noexcept             { return type == AULab; }
    /** Returns true if the host is Bitwig Studio. */
    bool isBitwigStudio() const noexcept      { return type == BitwigStudio; }
    /** Returns true if the host is any version of Steinberg Cubase. */
    bool isCubase() const noexcept            { return type == SteinbergCubase4 || type == SteinbergCubase5 || type == SteinbergCubase5Bridged || type == SteinbergCubase6
                                                      || type == SteinbergCubase7 || type == SteinbergCubase8 || type == SteinbergCubase8_5 || type == SteinbergCubase9
                                                      || type == SteinbergCubase9_5 || type == SteinbergCubase10 || type == SteinbergCubase10_5 || type == SteinbergCubaseGeneric; }
    /** Returns true if the host is Steinberg Cubase 7 or later. */
    bool isCubase7orLater() const noexcept    { return isCubase() && ! (type == SteinbergCubase4 || type == SteinbergCubase5 || type == SteinbergCubase6); }
    /** Returns true if the host is Steinberg Cubase 5 Bridged. */
    bool isCubaseBridged() const noexcept     { return type == SteinbergCubase5Bridged; }
    /** Returns true if the host is DaVinci Resolve. */
    bool isDaVinciResolve() const noexcept    { return type == DaVinciResolve; }
    /** Returns true if the host is Digital Performer. */
    bool isDigitalPerformer() const noexcept  { return type == DigitalPerformer; }
    /** Returns true if the host is Apple Final Cut Pro. */
    bool isFinalCut() const noexcept          { return type == FinalCut; }
    /** Returns true if the host is Fruity Loops. */
    bool isFruityLoops() const noexcept       { return type == FruityLoops; }
    /** Returns true if the host is Apple GarageBand. */
    bool isGarageBand() const noexcept        { return type == AppleGarageBand; }
    /** Returns true if the host is the JUCE AudioPluginHost */
    bool isJUCEPluginHost() const noexcept    { return type == JUCEPluginHost; }
    /** Returns true if the host is Apple Logic Pro. */
    bool isLogic() const noexcept             { return type == AppleLogic; }
    /** Returns true if the host is Apple MainStage. */
    bool isMainStage() const noexcept         { return type == AppleMainStage; }
    /** Returns true if the host is any version of Steinberg Nuendo. */
    bool isNuendo() const noexcept            { return type == SteinbergNuendo3 || type == SteinbergNuendo4  || type == SteinbergNuendo5 ||  type == SteinbergNuendoGeneric; }
    /** Returns true if the host is pluginval. */
    bool isPluginval() const noexcept         { return type == pluginval; }
    /** Returns true if the host is Adobe Premiere Pro. */
    bool isPremiere() const noexcept          { return type == AdobePremierePro; }
    /** Returns true if the host is Avid Pro Tools. */
    bool isProTools() const noexcept          { return type == AvidProTools; }
    /** Returns true if the host is Merging Pyramix. */
    bool isPyramix() const noexcept           { return type == MergingPyramix; }
    /** Returns true if the host is Muse Receptor. */
    bool isReceptor() const noexcept          { return type == MuseReceptorGeneric; }
    /** Returns true if the host is Cockos Reaper. */
    bool isReaper() const noexcept            { return type == Reaper; }
    /** Returns true if the host is Reason. */
    bool isReason() const noexcept            { return type == Reason; }
    /** Returns true if the host is Renoise. */
    bool isRenoise() const noexcept           { return type == Renoise; }
    /** Returns true if the host is SADiE. */
    bool isSADiE() const noexcept             { return type == SADiE; }
    /** Returns true if the host is Magix Samplitude. */
    bool isSamplitude() const noexcept        { return type == MagixSamplitude; }
    /** Returns true if the host is Magix Sequoia. */
    bool isSequoia() const noexcept           { return type == MagixSequoia; }
    /** Returns true if the host is any version of Cakewalk Sonar. */
    bool isSonar() const noexcept             { return type == CakewalkSonar8 || type == CakewalkSonarGeneric || type == CakewalkByBandlab; }
    /** Returns true if the host is Steinberg's VST3 Test Host. */
    bool isSteinbergTestHost() const noexcept { return type == SteinbergTestHost; }
    /** Returns true if the host is any product from Steinberg. */
    bool isSteinberg() const noexcept         { return isCubase() || isNuendo() || isWavelab() || isSteinbergTestHost(); }
    /** Returns true if the host is PreSonus Studio One. */
    bool isStudioOne() const noexcept         { return type == StudioOne; }
    /** Returns true if the host is any version of Tracktion. */
    bool isTracktion() const noexcept         { return type == Tracktion3 || type == TracktionGeneric || isTracktionWaveform(); }
    /** Returns true if the host is Tracktion Waveform. */
    bool isTracktionWaveform() const noexcept { return type == TracktionWaveform; }
    /** Returns true if the host is VB Audio VST Scanner. */
    bool isVBVSTScanner() const noexcept      { return type == VBVSTScanner; }
    /** Returns true if the host is Vienna Ensemble Pro. */
    bool isViennaEnsemblePro() const noexcept { return type == ViennaEnsemblePro; }
    /** Returns true if the host is Apple WaveBurner. */
    bool isWaveBurner() const noexcept        { return type == WaveBurner; }
    /** Returns true if the host is any version of Steinberg WaveLab. */
    bool isWavelab() const noexcept           { return isWavelabLegacy() || type == SteinbergWavelab7 || type == SteinbergWavelab8 || type == SteinbergWavelabGeneric; }
    /** Returns true if the host is Steinberg WaveLab 6 or below. */
    bool isWavelabLegacy() const noexcept     { return type == SteinbergWavelab5 || type == SteinbergWavelab6; }

    //==============================================================================
    /** Returns a human-readable description of the host. */
    const char* getHostDescription() const noexcept;

    //==============================================================================
    /** Returns true if the plugin is connected with Inter-App Audio on iOS. */
    bool isInterAppAudioConnected() const;
    /** Switches to the host application when Inter-App Audio is used on iOS. */
    void switchToHostApplication() const;
    /** Gets the host app's icon when Inter-App Audio is used on iOS. */
    Image getHostIcon (int size) const;

    //==============================================================================
    /** Returns the complete absolute path of the host application executable. */
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

    /** Returns true if the AudioProcessor instance is an AAX plug-in running in AudioSuite. */
    static bool isInAAXAudioSuite (AudioProcessor&);

    //==============================================================================

  #ifndef DOXYGEN
    // @internal
    static AudioProcessor::WrapperType jucePlugInClientCurrentWrapperType;
    static std::function<bool (AudioProcessor&)> jucePlugInIsRunningInAudioSuiteFn;
    static String hostIdReportedByWrapper;
  #endif

private:
    static HostType getHostType();
};

} // namespace juce
