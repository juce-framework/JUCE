/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

struct PluginBusUtilities
{
    //==============================================================================
    typedef Array<AudioProcessor::AudioProcessorBus> AudioBusArray;

    //==============================================================================
    PluginBusUtilities (AudioProcessor& plugin, bool markDiscreteLayoutsAsSupported, int maxProbeChannels = kDefaultMaxChannels)
        : processor (plugin),
          dynamicInBuses (false),
          dynamicOutBuses (false),
          plugInFormatSupportsDiscreteLayouts (markDiscreteLayoutsAsSupported),
          maxChannelsToProbe (maxProbeChannels)
    {
    }

    // this will invoke setPreferredLayout many times
    void init()    { populateLayoutDetails(); }

    //==============================================================================
    // Useful short-cuts
    AudioBusArray&       getFilterBus (bool inputBus) noexcept         { return inputBus ? processor.busArrangement.inputBuses : processor.busArrangement.outputBuses; }
    const AudioBusArray& getFilterBus (bool inputBus) const noexcept   { return inputBus ? processor.busArrangement.inputBuses : processor.busArrangement.outputBuses; }
    int getBusCount (bool inputBus) const noexcept                     { return getFilterBus (inputBus).size(); }
    AudioChannelSet getChannelSet (bool inputBus, int bus) noexcept    { return getFilterBus (inputBus).getReference (bus).channels; }
    int getNumChannels (bool inp, int bus) const noexcept              { return isPositiveAndBelow (bus, getBusCount (inp)) ? getFilterBus (inp).getReference (bus).channels.size() : 0; }
    bool isBusEnabled (bool inputBus, int bus) const noexcept          { return (getNumChannels (inputBus, bus) > 0); }
    bool hasInputs  (int bus) const noexcept                           { return isBusEnabled (true,  bus); }
    bool hasOutputs (int bus) const noexcept                           { return isBusEnabled (false, bus); }
    bool hasDynamicInBuses()  const noexcept                           { return dynamicInBuses; }
    bool hasDynamicOutBuses() const noexcept                           { return dynamicOutBuses; }

    //==============================================================================
    // Channel Counters
    int getNumEnabledBuses (bool inputBus) const noexcept              { int i; for (i = 0; i < getBusCount (inputBus); ++i) if (! isBusEnabled (inputBus, i)) break; return i; }

    int findTotalNumChannels (bool isInput, int busOffset = 0) const noexcept
    {
        int total = 0;
        const AudioBusArray& ioBuses = getFilterBus (isInput);

        for (int i = busOffset; i < ioBuses.size(); ++i)
            total += ioBuses.getReference (i).channels.size();

        return total;
    }

    int getBusIdxForChannelIdx (bool isInput, int channelIdx, int& totalChannels, int startBusIdx)
    {
        const int numBuses = getBusCount (isInput);

        for (int busIdx = startBusIdx; busIdx < numBuses; ++busIdx)
        {
            const int numChannels = getNumChannels (isInput, busIdx);
            if ((totalChannels + numChannels) > channelIdx)
                return busIdx;

            totalChannels += numChannels;
        }

        return -1;
    }

    int getBusIdxForChannelIdx (bool isInput, int channelIdx)
    {
        int totalChannels = 0;
        return getBusIdxForChannelIdx (isInput, channelIdx, totalChannels, 0);
    }

    //==============================================================================
    // Bus properties & defaults
    bool busIgnoresLayout (bool inp, int bus) const noexcept
    {
        return isPositiveAndBelow (bus, getLayoutDetails (inp).size()) ? getBusLayoutDetails (inp, bus).busIgnoresLayout : true;
    }

    bool busCanBeDisabled (bool inp, int bus) const noexcept
    {
        return isPositiveAndBelow (bus, getLayoutDetails (inp).size()) ? getBusLayoutDetails (inp, bus).canBeDisabled : false;
    }

    bool isBusEnabledByDefault (bool inp, int bus) const noexcept
    {
        return isPositiveAndBelow (bus, getLayoutDetails (inp).size()) ? getBusLayoutDetails (inp, bus).isEnabledByDefault : true;
    }

    bool checkBusFormatsAreNotDiscrete() const  { return (checkBusFormatsAreNotDiscrete (true) && checkBusFormatsAreNotDiscrete (false)); }

    const AudioChannelSet& getDefaultLayoutForBus (bool isInput, int busIdx) const noexcept    { return getBusLayoutDetails (isInput, busIdx).defaultLayout; }

    AudioChannelSet getDefaultLayoutForChannelNumAndBus (bool isInput, int busIdx, int channelNum) const noexcept
    {
        if (busIdx < 0 || busIdx >= getBusCount (isInput) || channelNum == 0)
            return AudioChannelSet::disabled();

        const BusLayoutDetails& layouts = getBusLayoutDetails (isInput, busIdx);

        const AudioChannelSet& dflt = layouts.defaultLayout;
        const AudioChannelSet discreteChannels = AudioChannelSet::discreteChannels (channelNum);

        if (dflt.size() == channelNum && (plugInFormatSupportsDiscreteLayouts || (! dflt.isDiscreteLayout())))
            return dflt;

        Array<AudioChannelSet> potentialLayouts = layoutListCompatibleWithChannelCount (channelNum);

        ScopedBusRestorer busRestorer (*this);

        // prefer non-discrete layouts if no explicit default layout is given
        const int n = potentialLayouts.size();
        for (int i = 0; i < n; ++i)
        {
            const AudioChannelSet& layout = potentialLayouts.getReference (i);
            if (processor.setPreferredBusArrangement (isInput, busIdx, layout))
                return layout;
        }


        if (plugInFormatSupportsDiscreteLayouts && processor.setPreferredBusArrangement (isInput, busIdx, discreteChannels))
            return discreteChannels;

        // we are out of options, bail out
        return AudioChannelSet();
    }

    //==============================================================================
    // This function is quite heavy so please cache the return value
    int findMaxNumberOfChannelsForBus (bool isInput, int busNr, int upperlimit = std::numeric_limits<int>::max())
    {
        int maxChannelsPreprocessorDefs = -1;
       #ifdef  JucePlugin_MaxNumInputChannels
        if (isInput)
            maxChannelsPreprocessorDefs = jmin (upperlimit, JucePlugin_MaxNumInputChannels);
       #endif

       #ifdef  JucePlugin_MaxNumOutputChannels
        if (! isInput)
            maxChannelsPreprocessorDefs = jmin (upperlimit, JucePlugin_MaxNumOutputChannels);
       #endif

       #ifdef JucePlugin_PreferredChannelConfigurations
        if (busNr == 0)
        {
            int maxChannelCount = 0;
            const short channelConfigs[][2] = { JucePlugin_PreferredChannelConfigurations };
            const int numChannelConfigs = sizeof (channelConfigs) / sizeof (*channelConfigs);

            for (int i = 0; i < numChannelConfigs; ++i)
            {
                const int numChannels = channelConfigs [i][isInput ? 0 : 1];
                if (numChannels < 0)
                    return -1;

                maxChannelCount = jmax (maxChannelCount, numChannels);
            }

            return jmin (upperlimit, maxChannelCount);
        }
       #endif

        ScopedBusRestorer busRestorer (*this);

        if (plugInFormatSupportsDiscreteLayouts &&
               processor.setPreferredBusArrangement(isInput, busNr, AudioChannelSet::discreteChannels (insaneNumberOfChannels)))
            return -1; // no limit in channels

        int n = maxChannelsPreprocessorDefs > 0 ? maxChannelsPreprocessorDefs
                                                : (plugInFormatSupportsDiscreteLayouts ? maxChannelsToProbe
                                                                                       : maxNumChannelsOfNonDiscreteLayouts);

        n = jmin (upperlimit, n);

        for (int i = n; i > 0; --i)
        {
            if (plugInFormatSupportsDiscreteLayouts && processor.setPreferredBusArrangement (isInput, busNr, AudioChannelSet::discreteChannels (i)))
                return i;

            Array<AudioChannelSet> sets = layoutListCompatibleWithChannelCount (i);

            for (int j = 0; j < sets.size(); ++j)
            {
                const AudioChannelSet& layout = sets.getReference (j);

                if (processor.setPreferredBusArrangement (isInput, busNr, layout))
                    return i;
            }
        }

        return 0;
    }

    //==============================================================================
    void restoreBusArrangement (const AudioProcessor::AudioBusArrangement& original) const
    {
        const int numInputBuses  = getBusCount (true);
        const int numOutputBuses = getBusCount (false);

        jassert (original.inputBuses. size() == numInputBuses);
        jassert (original.outputBuses.size() == numOutputBuses);

        for (int busNr = 0; busNr < numInputBuses;  ++busNr)
            processor.setPreferredBusArrangement (true,  busNr, original.inputBuses.getReference  (busNr).channels);

        for (int busNr = 0; busNr < numOutputBuses; ++busNr)
            processor.setPreferredBusArrangement (false, busNr, original.outputBuses.getReference (busNr).channels);
    }

    void enableAllBuses()
    {
        for (int busIdx = 1; busIdx < getBusCount (true); ++busIdx)
            if (getChannelSet (true, busIdx) == AudioChannelSet::disabled())
                processor.setPreferredBusArrangement (true, busIdx, getDefaultLayoutForBus (true, busIdx));

        for (int busIdx = 1; busIdx < getBusCount (false); ++busIdx)
            if (getChannelSet (false, busIdx) == AudioChannelSet::disabled())
                processor.setPreferredBusArrangement (false, busIdx, getDefaultLayoutForBus (false, busIdx));
    }

    //==============================================================================
    // Helper class which restores the original arrangement when it leaves scope
    class ScopedBusRestorer
    {
    public:
        ScopedBusRestorer (const PluginBusUtilities& bUtils)
            : busUtils (bUtils),
              originalArr (bUtils.processor.busArrangement),
              shouldRestore (true)
        {}

        ~ScopedBusRestorer()
        {
            if (shouldRestore)
                busUtils.restoreBusArrangement (originalArr);
        }

        void release() noexcept      { shouldRestore = false; }

    private:
        const PluginBusUtilities& busUtils;
        const AudioProcessor::AudioBusArrangement originalArr;
        bool shouldRestore;

        JUCE_DECLARE_NON_COPYABLE (ScopedBusRestorer)
    };

    //==============================================================================
    AudioProcessor& processor;

    enum
    {
        kDefaultMaxChannels = 64
    };

private:
    friend class ScopedBusRestorer;

    enum
    {
        maxNumChannelsOfNonDiscreteLayouts = 8, // surround 7.1 has the maximum amount of channels
        pseudoChannelBitNum = 90,                // use this bit index to check if plug-in really doesn't care about layouts
        insaneNumberOfChannels = 512
    };

    //==============================================================================
    // the first layout is the default layout
    struct BusLayoutDetails
    {
        BusLayoutDetails() : busIgnoresLayout (true), canBeDisabled (false), isEnabledByDefault (false) {}

        AudioChannelSet defaultLayout;
        bool busIgnoresLayout, canBeDisabled, isEnabledByDefault;
    };

    Array<BusLayoutDetails>&       getLayoutDetails (bool isInput) noexcept              { return isInput ? inputLayouts : outputLayouts; }
    const Array<BusLayoutDetails>& getLayoutDetails (bool isInput) const noexcept        { return isInput ? inputLayouts : outputLayouts; }
    BusLayoutDetails&       getBusLayoutDetails (bool isInput, int busNr) noexcept       { return getLayoutDetails (isInput).getReference (busNr); }
    const BusLayoutDetails& getBusLayoutDetails (bool isInput, int busNr) const noexcept { return getLayoutDetails (isInput).getReference (busNr); }

    //==============================================================================
    Array<BusLayoutDetails> inputLayouts, outputLayouts;
    bool dynamicInBuses, dynamicOutBuses, plugInFormatSupportsDiscreteLayouts;
    int maxChannelsToProbe;

    //==============================================================================
    void populateLayoutDetails()
    {
        clear (getBusCount (true), getBusCount (false));

        // save the default layouts
        for (int i = 0; i < getBusCount (true);   ++i)
            getBusLayoutDetails (true,  i).defaultLayout = getChannelSet (true,  i);

        for (int i = 0; i < getBusCount (false);  ++i)
            getBusLayoutDetails (false, i).defaultLayout = getChannelSet (false, i);

        {
            ScopedBusRestorer restorer (*this);


            for (int i = 0; i < getBusCount (true);  ++i) addLayoutDetails (true,  i);
            for (int i = 0; i < getBusCount (false); ++i) addLayoutDetails (false, i);

            // find the defaults
            for (int i = 0; i < getBusCount (true); ++i)
                updateDefaultLayout (true, i);

            for (int i = 0; i < getBusCount (false); ++i)
                updateDefaultLayout (false, i);
        }

        // can any of the buses be disabled/enabled
        dynamicInBuses  = doesPlugInHaveDynamicBuses (true);
        dynamicOutBuses = doesPlugInHaveDynamicBuses (false);
    }

    //==============================================================================
    bool busIgnoresLayoutForChannelNum (bool isInput, int busNr, int channelNum)
    {
        AudioChannelSet set;

        // If the plug-in does not complain about setting it's layout to an undefined layout
        // then we assume that the plug-in ignores the layout altogether
        for (int i = 0; i < channelNum; ++i)
            set.addChannel (static_cast<AudioChannelSet::ChannelType> (pseudoChannelBitNum + i));

        return processor.setPreferredBusArrangement (isInput, busNr, set);
    }

    void addLayoutDetails (bool isInput, int busNr)
    {
        BusLayoutDetails& layouts = getBusLayoutDetails (isInput, busNr);

        // check if the plug-in bus can be disabled
        layouts.canBeDisabled = processor.setPreferredBusArrangement (isInput, busNr, AudioChannelSet());
        layouts.busIgnoresLayout = true;

        for (int i = 1; i <= maxNumChannelsOfNonDiscreteLayouts; ++i)
        {
            const bool ignoresLayoutForChannel = busIgnoresLayoutForChannelNum (isInput, busNr, i);

            Array<AudioChannelSet> sets = layoutListCompatibleWithChannelCount (i);

            for (int j = 0; j < sets.size(); ++j)
            {
                const AudioChannelSet& layout = sets.getReference (j);

                if (processor.setPreferredBusArrangement (isInput, busNr, layout))
                {
                    if (! ignoresLayoutForChannel)
                    {
                        layouts.busIgnoresLayout = false;
                        return;
                    }
                }
            }
        }
    }

    bool doesPlugInHaveDynamicBuses (bool isInput) const
    {
        for (int i = 0; i < getBusCount (isInput); ++i)
            if (getBusLayoutDetails (isInput, i).canBeDisabled)
                return true;

        return false;
    }

    bool checkBusFormatsAreNotDiscrete (bool isInput) const
    {
        const int n = getBusCount (isInput);
        const Array<AudioProcessor::AudioProcessorBus>& bus = isInput ? processor.busArrangement.inputBuses
                                                                      : processor.busArrangement.outputBuses;

        for (int busIdx = 0; busIdx < n; ++busIdx)
            if (bus.getReference (busIdx).channels.isDiscreteLayout())
                return false;

        return true;
    }

    void updateDefaultLayout (bool isInput, int busIdx)
    {
        BusLayoutDetails& layouts = getBusLayoutDetails (isInput, busIdx);
        AudioChannelSet& dfltLayout = layouts.defaultLayout;

        layouts.isEnabledByDefault = (dfltLayout.size() > 0);

        // If you hit this assertion then you are disabling the main bus by default
        // which is unsupported
        jassert (layouts.isEnabledByDefault || busIdx >= 0);

        if ((! plugInFormatSupportsDiscreteLayouts) && dfltLayout.isDiscreteLayout())
        {
            // The default layout is a discrete channel layout, yet some plug-in formats (VST-3)
            // do not support this format. We need to find a different default with the same
            // number of channels

            dfltLayout = getDefaultLayoutForChannelNumAndBus (isInput, busIdx, dfltLayout.size());
        }

        // are we done?
        if (dfltLayout != AudioChannelSet())
            return;

        const bool mainBusHasInputs  = hasInputs (0);
        const bool mainBusHasOutputs = hasOutputs (0);

        if (busIdx != 0 && (mainBusHasInputs || mainBusHasOutputs))
        {
            // the AudioProcessor does not give us any default layout
            // for an aux bus. Use the same number of channels as the
            // default layout on the main bus as a sensible default for
            // the aux bus

            const bool useInput = mainBusHasInputs && mainBusHasOutputs ? isInput : mainBusHasInputs;
            dfltLayout = getBusLayoutDetails (useInput, 0).defaultLayout;

            const int numChannels = dfltLayout.size();
            const AudioChannelSet discreteChannelLayout = AudioChannelSet::discreteChannels (numChannels);

            if ((plugInFormatSupportsDiscreteLayouts || dfltLayout != discreteChannelLayout) &&
                processor.setPreferredBusArrangement (isInput, busIdx, dfltLayout))
                return;

            // no exact match: try at least to match the number of channels
            dfltLayout = getDefaultLayoutForChannelNumAndBus (isInput, busIdx, dfltLayout.size());
            if (dfltLayout != AudioChannelSet())
                return;
        }

        // check stereo first as this is often the more sensible default than mono
        if (processor.setPreferredBusArrangement (isInput, busIdx, (dfltLayout = AudioChannelSet::stereo())))
            return;

        if (plugInFormatSupportsDiscreteLayouts &&
            processor.setPreferredBusArrangement (isInput, busIdx, (dfltLayout = AudioChannelSet::discreteChannels (2))))
            return;

        // let's guess
        for (int numChans = 1; numChans < findMaxNumberOfChannelsForBus (isInput, busIdx); ++numChans)
        {
            Array<AudioChannelSet> sets = layoutListCompatibleWithChannelCount (numChans);
            for (int j = 0; j < sets.size(); ++j)
                if (processor.setPreferredBusArrangement (isInput, busIdx, (dfltLayout = sets.getReference (j))))
                    return;

            if (plugInFormatSupportsDiscreteLayouts &&
                processor.setPreferredBusArrangement (isInput, busIdx, (dfltLayout = AudioChannelSet::discreteChannels (numChans))))
                return;
        }

        // Your bus must support at least a single possible layout
        jassertfalse;
    }

    void clear (int inputCount, int outputCount)
    {
        inputLayouts.clear();
        inputLayouts.resize (inputCount);
        outputLayouts.clear();
        outputLayouts.resize (outputCount);
    }

    //==============================================================================
    static Array<AudioChannelSet> layoutListCompatibleWithChannelCount (const int channelCount) noexcept
    {
        jassert (channelCount > 0);

        Array<AudioChannelSet> sets;

        switch (channelCount)
        {
            case 1:
                sets.add (AudioChannelSet::mono());
                break;
            case 2:
                sets.add (AudioChannelSet::stereo());
                break;
            case 3:
                sets.add (AudioChannelSet::createLCR());
                sets.add (AudioChannelSet::createLRS());
                break;
            case 4:
                sets.add (AudioChannelSet::createLCRS());
                sets.add (AudioChannelSet::quadraphonic());
                sets.add (AudioChannelSet::ambisonic());
                break;
            case 5:
                sets.add (AudioChannelSet::pentagonal());
                sets.add (AudioChannelSet::create5point0());
                break;
            case 6:
                sets.add (AudioChannelSet::hexagonal());
                sets.add (AudioChannelSet::create5point1());
                sets.add (AudioChannelSet::create6point0());
                sets.add (AudioChannelSet::create6point0Music());
                break;
            case 7:
                sets.add (AudioChannelSet::create6point1());
                sets.add (AudioChannelSet::create7point0());
                break;
            case 8:
                sets.add (AudioChannelSet::octagonal());
                sets.add (AudioChannelSet::create7point1());
                sets.add (AudioChannelSet::create7point1AC3());
                sets.add (AudioChannelSet::createFront7point1());
                break;
        }

        return sets;
    }

    JUCE_DECLARE_NON_COPYABLE (PluginBusUtilities)
};
