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
// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

class PluginBusUtilities
{
public:
    //==============================================================================
    typedef Array<AudioProcessor::AudioProcessorBus> AudioBusArray;

    //==============================================================================
    PluginBusUtilities (AudioProcessor& plugin, bool markDiscreteLayoutsAsSupported)
        : juceFilter (plugin),
          dynamicInBuses (false),
          dynamicOutBuses (false),
          addDiscreteLayouts (markDiscreteLayoutsAsSupported)
    {
    }

    //==============================================================================
    // the first layout is the default layout
    struct SupportedBusLayouts
    {
        enum
        {
            pseudoChannelBitNum = 90 // use this bit index to check if plug-in really doesn't care about layouts
        };

        //==============================================================================
        SupportedBusLayouts() : defaultLayoutIndex (0), busIgnoresLayout (true), canBeDisabled (false) {}
        AudioChannelSet&       getDefault() noexcept                             { return supportedLayouts.getReference (defaultLayoutIndex); }
        const AudioChannelSet& getDefault() const noexcept                       { return supportedLayouts.getReference (defaultLayoutIndex); }
        void updateDefaultLayout (const AudioChannelSet& defaultLayout) noexcept { defaultLayoutIndex = jmax (supportedLayouts.indexOf (defaultLayout), 0); }
        bool busSupportsNumChannels (int numChannels) const noexcept             { return getDefaultLayoutForChannelNum (numChannels) != nullptr; }

        //==============================================================================
        const AudioChannelSet* getDefaultLayoutForChannelNum (int channelNum) const noexcept
        {
            const AudioChannelSet& dflt = getDefault();

            if (dflt.size() == channelNum)
                return &dflt;

            for (int i = 0; i < supportedLayouts.size(); ++i)
            {
                const AudioChannelSet& layout = supportedLayouts.getReference (i);

                if (layout.size() == channelNum)
                    return &layout;
            }

            return nullptr;
        }

        int defaultLayoutIndex;
        bool busIgnoresLayout, canBeDisabled, isEnabledByDefault;
        SortedSet<AudioChannelSet> supportedLayouts;
    };

    //==============================================================================
    AudioBusArray&       getFilterBus (bool inputBus) noexcept         { return inputBus ? juceFilter.busArrangement.inputBuses : juceFilter.busArrangement.outputBuses; }
    const AudioBusArray& getFilterBus (bool inputBus) const noexcept   { return inputBus ? juceFilter.busArrangement.inputBuses : juceFilter.busArrangement.outputBuses; }
    int getBusCount (bool inputBus) const noexcept                     { return getFilterBus (inputBus).size(); }
    AudioChannelSet getChannelSet (bool inputBus, int bus) noexcept    { return getFilterBus (inputBus).getReference (bus).channels; }
    int getNumChannels (bool inp, int bus) const noexcept              { return isPositiveAndBelow (bus, getBusCount (inp)) ? getFilterBus (inp).getReference (bus).channels.size() : 0; }
    bool isBusEnabled (bool inputBus, int bus) const noexcept          { return (getNumChannels (inputBus, bus) > 0); }
    bool hasInputs  (int bus) const noexcept                           { return isBusEnabled (true,  bus); }
    bool hasOutputs (int bus) const noexcept                           { return isBusEnabled (false, bus); }
    int getNumEnabledBuses (bool inputBus) const noexcept              { int i; for (i = 0; i < getBusCount (inputBus); ++i) if (! isBusEnabled (inputBus, i)) break; return i; }

    int findTotalNumChannels (bool isInput) const noexcept
    {
        int total = 0;
        const AudioBusArray& ioBuses = getFilterBus (isInput);

        for (int i = 0; i < ioBuses.size(); ++i)
            total += ioBuses.getReference (i).channels.size();

        return total;
    }

    //==============================================================================
    void restoreBusArrangement (const AudioProcessor::AudioBusArrangement& original) const
    {
        const int numInputBuses  = getBusCount (true);
        const int numOutputBuses = getBusCount (false);

        jassert (original.inputBuses. size() == numInputBuses);
        jassert (original.outputBuses.size() == numOutputBuses);

        for (int busNr = 0; busNr < numInputBuses;  ++busNr)
            juceFilter.setPreferredBusArrangement (true,  busNr, original.inputBuses.getReference  (busNr).channels);

        for (int busNr = 0; busNr < numOutputBuses; ++busNr)
            juceFilter.setPreferredBusArrangement (false, busNr, original.outputBuses.getReference (busNr).channels);
    }

    //==============================================================================
    Array<SupportedBusLayouts>&       getSupportedLayouts (bool isInput) noexcept              { return isInput ? inputLayouts : outputLayouts; }
    const Array<SupportedBusLayouts>& getSupportedLayouts (bool isInput) const noexcept        { return isInput ? inputLayouts : outputLayouts; }

    SupportedBusLayouts&       getSupportedBusLayouts (bool isInput, int busNr) noexcept       { return getSupportedLayouts (isInput).getReference (busNr); }
    const SupportedBusLayouts& getSupportedBusLayouts (bool isInput, int busNr) const noexcept { return getSupportedLayouts (isInput).getReference (busNr); }
    bool busIgnoresLayout (bool inp, int bus) const noexcept
    {
        return isPositiveAndBelow (bus, getSupportedLayouts (inp).size()) ? getSupportedBusLayouts (inp, bus).busIgnoresLayout : true;
    }

    const AudioChannelSet& getDefaultLayoutForBus (bool isInput, int busIdx) const noexcept    { return getSupportedBusLayouts (isInput, busIdx).getDefault(); }
    bool hasDynamicInBuses()  const noexcept                                                   { return dynamicInBuses; }
    bool hasDynamicOutBuses() const noexcept                                                   { return dynamicOutBuses; }

    void clear (int inputCount, int outputCount)
    {
        inputLayouts.clear();
        inputLayouts.resize (inputCount);
        outputLayouts.clear();
        outputLayouts.resize (outputCount);
    }

    //==============================================================================
    AudioChannelSet getDefaultLayoutForChannelNumAndBus (bool isInput, int busIdx, int channelNum) const noexcept
    {
        if (const AudioChannelSet* set = getSupportedBusLayouts (isInput, busIdx).getDefaultLayoutForChannelNum (channelNum))
            return *set;

        return AudioChannelSet::canonicalChannelSet (channelNum);
    }

    void findAllCompatibleLayouts()
    {
        {
            ScopedBusRestorer restorer (*this);
            clear (getBusCount (true), getBusCount (false));

            for (int i = 0; i < getBusCount (true);  ++i) findAllCompatibleLayoutsForBus (true,  i);
            for (int i = 0; i < getBusCount (false); ++i) findAllCompatibleLayoutsForBus (false, i);
        }

        // find the defaults
        for (int i = 0; i < getBusCount (true); ++i)
            updateDefaultLayout (true, i);

        for (int i = 0; i < getBusCount (false); ++i)
            updateDefaultLayout (false, i);

        // can any of the buses be disabled/enabled
        dynamicInBuses  = doesPlugInHaveDynamicBuses (true);
        dynamicOutBuses = doesPlugInHaveDynamicBuses (false);
    }

    //==============================================================================
    void enableAllBuses()
    {
        for (int busIdx = 1; busIdx < getBusCount (true); ++busIdx)
            if (getChannelSet (true, busIdx) == AudioChannelSet::disabled())
                juceFilter.setPreferredBusArrangement (true, busIdx, getDefaultLayoutForBus (true, busIdx));

        for (int busIdx = 1; busIdx < getBusCount (false); ++busIdx)
            if (getChannelSet (false, busIdx) == AudioChannelSet::disabled())
                juceFilter.setPreferredBusArrangement (false, busIdx, getDefaultLayoutForBus (false, busIdx));
    }

    //==============================================================================
    // Helper class which restores the original arrangement when it leaves scope
    class ScopedBusRestorer
    {
    public:
        ScopedBusRestorer (PluginBusUtilities& bUtils)
            : busUtils (bUtils),
              originalArr (bUtils.juceFilter.busArrangement),
              shouldRestore (true)
        {}

        ~ScopedBusRestorer()
        {
            if (shouldRestore)
                busUtils.restoreBusArrangement (originalArr);
        }

        void release() noexcept      { shouldRestore = false; }

    private:
        PluginBusUtilities& busUtils;
        const AudioProcessor::AudioBusArrangement originalArr;
        bool shouldRestore;

        JUCE_DECLARE_NON_COPYABLE (ScopedBusRestorer)
    };

    //==============================================================================
    AudioProcessor& juceFilter;

private:
    friend class ScopedBusRestorer;

    //==============================================================================
    Array<SupportedBusLayouts> inputLayouts, outputLayouts;
    bool dynamicInBuses, dynamicOutBuses, addDiscreteLayouts;

    //==============================================================================
    bool busIgnoresLayoutForChannelNum (bool isInput, int busNr, int channelNum)
    {
        AudioChannelSet set;

        // If the plug-in does not complain about setting it's layout to an undefined layout
        // then we assume that the plug-in ignores the layout alltogether
        for (int i = 0; i < channelNum; ++i)
            set.addChannel (static_cast<AudioChannelSet::ChannelType> (SupportedBusLayouts::pseudoChannelBitNum + i));

        return juceFilter.setPreferredBusArrangement (isInput, busNr, set);
    }

    void findAllCompatibleLayoutsForBus (bool isInput, int busNr)
    {
        const int maxNumChannels = 9;

        SupportedBusLayouts& layouts = getSupportedBusLayouts (isInput, busNr);
        layouts.supportedLayouts.clear();

        // check if the plug-in bus can be disabled
        layouts.canBeDisabled = juceFilter.setPreferredBusArrangement (isInput, busNr, AudioChannelSet());
        layouts.busIgnoresLayout = true;

        for (int i = 1; i <= maxNumChannels; ++i)
        {
            const bool ignoresLayoutForChannel = busIgnoresLayoutForChannelNum (isInput, busNr, i);

            Array<AudioChannelSet> sets = layoutListCompatibleWithChannelCount (addDiscreteLayouts, i);

            for (int j = 0; j < sets.size(); ++j)
            {
                const AudioChannelSet& layout = sets.getReference (j);

                if (juceFilter.setPreferredBusArrangement (isInput, busNr, layout))
                {
                    if (! ignoresLayoutForChannel)
                        layouts.busIgnoresLayout = false;

                    layouts.supportedLayouts.add (layout);
                }
            }
        }

        // You cannot add a bus in your processor wich does not support any layouts! It must at least support one.
        jassert (layouts.supportedLayouts.size() > 0);
    }

    bool doesPlugInHaveDynamicBuses (bool isInput) const
    {
        for (int i = 0; i < getBusCount (isInput); ++i)
            if (getSupportedBusLayouts (isInput, i).canBeDisabled)
                return true;

        return false;
    }

    void updateDefaultLayout (bool isInput, int busIdx)
    {
        SupportedBusLayouts& layouts = getSupportedBusLayouts (isInput, busIdx);
        AudioChannelSet set = getChannelSet (isInput, busIdx);

        layouts.isEnabledByDefault = (set.size() > 0);
        if (layouts.isEnabledByDefault)
            layouts.supportedLayouts.add (set);

        // If you hit this assertion then you are disabling the main bus by default
        // which is unsupported
        jassert (layouts.isEnabledByDefault || busIdx >= 0);

        if (set == AudioChannelSet())
        {
            const bool mainBusHasInputs  = hasInputs (0);
            const bool mainBusHasOutputs = hasOutputs (0);

            if (busIdx != 0 && (mainBusHasInputs || mainBusHasOutputs))
            {
                // the AudioProcessor does not give us any default layout
                // for an aux bus. Use the same number of channels as the
                // default layout on the main bus as a sensible default for
                // the aux bus

                const bool useInput = mainBusHasInputs && mainBusHasOutputs ? isInput : mainBusHasInputs;
                const AudioChannelSet& dfltLayout = getSupportedBusLayouts (useInput, 0).getDefault();

                if ((layouts.defaultLayoutIndex = layouts.supportedLayouts.indexOf (dfltLayout)) >= 0)
                    return;

                // no exact match: try at least to match the number of channels
                for (int i = 0; i < layouts.supportedLayouts.size(); ++i)
                {
                    if (layouts.supportedLayouts.getReference (i).size() == dfltLayout.size())
                    {
                        layouts.defaultLayoutIndex = i;
                        return;
                    }
                }
            }

            if (layouts.busIgnoresLayout)
                set = AudioChannelSet::discreteChannels (set.size());
        }

        layouts.updateDefaultLayout (set);
    }

    static Array<AudioChannelSet> layoutListCompatibleWithChannelCount (bool addDiscrete, const int channelCount) noexcept
    {
        jassert (channelCount > 0);

        Array<AudioChannelSet> sets;

        if (addDiscrete)
            sets.add (AudioChannelSet::discreteChannels (channelCount));

        switch (channelCount)
        {
            case 1:
                sets.add (AudioChannelSet::mono());
                break;
            case 2:
                sets.add (AudioChannelSet::stereo());
                break;
            case 4:
                sets.add (AudioChannelSet::quadraphonic());
                sets.add (AudioChannelSet::ambisonic());
                break;
            case 5:
                sets.add (AudioChannelSet::pentagonal());
                sets.add (AudioChannelSet::create5point0());
                break;
            case 6:
                sets.add (AudioChannelSet::hexagonal());
                sets.add (AudioChannelSet::create6point0());
                break;
            case 7:
                sets.add (AudioChannelSet::create6point1());
                sets.add (AudioChannelSet::create7point0());
                sets.add (AudioChannelSet::createFront7point0());
                break;
            case 8:
                sets.add (AudioChannelSet::octagonal());
                sets.add (AudioChannelSet::create7point1());
                sets.add (AudioChannelSet::createFront7point1());
                break;
        }

        return sets;
    }
};
