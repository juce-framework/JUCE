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

#if JUCE_PLUGINHOST_VST3

} // namespace juce

#if JucePlugin_Build_VST3
 #undef JUCE_VST3HEADERS_INCLUDE_HEADERS_ONLY
 #define JUCE_VST3HEADERS_INCLUDE_HEADERS_ONLY 1
#endif

#include <map>
#include "juce_VST3Headers.h"

#undef JUCE_VST3HEADERS_INCLUDE_HEADERS_ONLY

namespace juce
{

#include "juce_VST3Common.h"

using namespace Steinberg;

//==============================================================================
struct VST3Classes
{

#ifndef JUCE_VST3_DEBUGGING
 #define JUCE_VST3_DEBUGGING 0
#endif

#if JUCE_VST3_DEBUGGING
 #define VST3_DBG(a) Logger::writeToLog (a);
#else
 #define VST3_DBG(a)
#endif

#if JUCE_DEBUG
static int warnOnFailure (int result)
{
    const char* message = "Unknown result!";

    switch (result)
    {
        case kResultOk:         return result;
        case kNotImplemented:   message = "kNotImplemented";  break;
        case kNoInterface:      message = "kNoInterface";     break;
        case kResultFalse:      message = "kResultFalse";     break;
        case kInvalidArgument:  message = "kInvalidArgument"; break;
        case kInternalError:    message = "kInternalError";   break;
        case kNotInitialized:   message = "kNotInitialized";  break;
        case kOutOfMemory:      message = "kOutOfMemory";     break;
        default:                break;
    }

    DBG (message);
    return result;
}
#else
 #define warnOnFailure(x) x
#endif

//==============================================================================
static int getHashForTUID (const TUID& tuid) noexcept
{
    int value = 0;

    for (int i = 0; i < numElementsInArray (tuid); ++i)
        value = (value * 31) + tuid[i];

    return value;
}

template<typename ObjectType>
static void fillDescriptionWith (PluginDescription& description, ObjectType& object)
{
    description.version  = toString (object.version).trim();
    description.category = toString (object.subCategories).trim();

    if (description.manufacturerName.trim().isEmpty())
        description.manufacturerName = toString (object.vendor).trim();
}

static void createPluginDescription (PluginDescription& description,
                                     const File& pluginFile, const String& company, const String& name,
                                     const PClassInfo& info, PClassInfo2* info2, PClassInfoW* infoW,
                                     int numInputs, int numOutputs)
{
    description.fileOrIdentifier    = pluginFile.getFullPathName();
    description.lastFileModTime     = pluginFile.getLastModificationTime();
    description.manufacturerName    = company;
    description.name                = name;
    description.descriptiveName     = name;
    description.pluginFormatName    = "VST3";
    description.numInputChannels    = numInputs;
    description.numOutputChannels   = numOutputs;
    description.uid                 = getHashForTUID (info.cid);

    if (infoW != nullptr)      fillDescriptionWith (description, *infoW);
    else if (info2 != nullptr) fillDescriptionWith (description, *info2);

    if (description.category.isEmpty())
        description.category = toString (info.category).trim();

    description.isInstrument = description.category.containsIgnoreCase ("Instrument"); // This seems to be the only way to find that out! ARGH!
}

static int getNumSingleDirectionBussesFor (Vst::IComponent* component,
                                           bool checkInputs,
                                           bool checkAudioChannels)
{
    jassert (component != nullptr);

    return (int) component->getBusCount (checkAudioChannels ? Vst::kAudio : Vst::kEvent,
                                         checkInputs ? Vst::kInput : Vst::kOutput);
}

/** Gives the total number of channels for a particular type of bus direction and media type */
static int getNumSingleDirectionChannelsFor (Vst::IComponent* component,
                                             bool checkInputs,
                                             bool checkAudioChannels)
{
    jassert (component != nullptr);

    const Vst::BusDirections direction  = checkInputs ? Vst::kInput : Vst::kOutput;
    const Vst::MediaTypes mediaType     = checkAudioChannels ? Vst::kAudio : Vst::kEvent;
    const Steinberg::int32 numBuses     = component->getBusCount (mediaType, direction);

    int numChannels = 0;

    for (Steinberg::int32 i = numBuses; --i >= 0;)
    {
        Vst::BusInfo busInfo;
        warnOnFailure (component->getBusInfo (mediaType, direction, i, busInfo));
        numChannels += (int) busInfo.channelCount;
    }

    return numChannels;
}

static void setStateForAllBussesOfType (Vst::IComponent* component,
                                        bool state,
                                        bool activateInputs,
                                        bool activateAudioChannels)
{
    jassert (component != nullptr);

    const Vst::BusDirections direction  = activateInputs ? Vst::kInput : Vst::kOutput;
    const Vst::MediaTypes mediaType     = activateAudioChannels ? Vst::kAudio : Vst::kEvent;
    const Steinberg::int32 numBuses     = component->getBusCount (mediaType, direction);

    for (Steinberg::int32 i = numBuses; --i >= 0;)
        warnOnFailure (component->activateBus (mediaType, direction, i, state));
}

//==============================================================================
/** Assigns a complete AudioSampleBuffer's channels to an AudioBusBuffers' */
static void associateWholeBufferTo (Vst::AudioBusBuffers& vstBuffers, AudioSampleBuffer& buffer) noexcept
{
    vstBuffers.channelBuffers32 = buffer.getArrayOfWritePointers();
    vstBuffers.numChannels      = buffer.getNumChannels();
    vstBuffers.silenceFlags     = 0;
}

//==============================================================================
static void toProcessContext (Vst::ProcessContext& context, AudioPlayHead* playHead, double sampleRate)
{
    jassert (sampleRate > 0.0); //Must always be valid, as stated by the VST3 SDK

    using namespace Vst;

    zerostruct (context);
    context.sampleRate = sampleRate;

    if (playHead != nullptr)
    {
        AudioPlayHead::CurrentPositionInfo position;
        playHead->getCurrentPosition (position);

        context.projectTimeSamples  = position.timeInSamples; //Must always be valid, as stated by the VST3 SDK
        context.projectTimeMusic    = position.timeInSeconds; //Does not always need to be valid...
        context.tempo               = position.bpm;
        context.timeSigNumerator    = position.timeSigNumerator;
        context.timeSigDenominator  = position.timeSigDenominator;
        context.barPositionMusic    = position.ppqPositionOfLastBarStart;
        context.cycleStartMusic     = position.ppqLoopStart;
        context.cycleEndMusic       = position.ppqLoopEnd;

        switch (position.frameRate)
        {
            case AudioPlayHead::fps24: context.frameRate.framesPerSecond = 24; break;
            case AudioPlayHead::fps25: context.frameRate.framesPerSecond = 25; break;
            case AudioPlayHead::fps30: context.frameRate.framesPerSecond = 30; break;

            case AudioPlayHead::fps2997:
            case AudioPlayHead::fps2997drop:
            case AudioPlayHead::fps30drop:
            {
                context.frameRate.framesPerSecond = 30;
                context.frameRate.flags = FrameRate::kDropRate;

                if (position.frameRate == AudioPlayHead::fps2997drop)
                    context.frameRate.flags |= FrameRate::kPullDownRate;
            }
            break;

            case AudioPlayHead::fpsUnknown: break;

            default:    jassertfalse; break; // New frame rate?
        }

        if (position.isPlaying)     context.state |= ProcessContext::kPlaying;
        if (position.isRecording)   context.state |= ProcessContext::kRecording;
        if (position.isLooping)     context.state |= ProcessContext::kCycleActive;
    }
    else
    {
        context.tempo                       = 120.0;
        context.frameRate.framesPerSecond   = 30;
        context.timeSigNumerator            = 4;
        context.timeSigDenominator          = 4;
    }

    if (context.projectTimeMusic >= 0.0)        context.state |= ProcessContext::kProjectTimeMusicValid;
    if (context.barPositionMusic >= 0.0)        context.state |= ProcessContext::kBarPositionValid;
    if (context.tempo > 0.0)                    context.state |= ProcessContext::kTempoValid;
    if (context.frameRate.framesPerSecond > 0)  context.state |= ProcessContext::kSmpteValid;

    if (context.cycleStartMusic >= 0.0
         && context.cycleEndMusic > 0.0
         && context.cycleEndMusic > context.cycleStartMusic)
    {
        context.state |= ProcessContext::kCycleValid;
    }

    if (context.timeSigNumerator > 0 && context.timeSigDenominator > 0)
        context.state |= ProcessContext::kTimeSigValid;
}

//==============================================================================
/** Get a list of speaker arrangements as per their speaker names

    (e.g.: 2 regular channels, aliased as 'kStringStereoS', is "L R")
*/
static StringArray getSpeakerArrangements()
{
    using namespace Vst::SpeakerArr;

    const Vst::CString arrangements[] =
    {
        kStringMonoS,       kStringStereoS,         kStringStereoRS,    kStringStereoCS,
        kStringStereoSS,    kStringStereoCLfeS,     kString30CineS,     kString30MusicS,
        kString31CineS,     kString31MusicS,        kString40CineS,     kString40MusicS,
        kString41CineS,     kString41MusicS,        kString50S,         kString51S,
        kString60CineS,     kString60MusicS,        kString61CineS,     kString61MusicS,
        kString70CineS,     kString70MusicS,        kString71CineS,     kString71MusicS,
        kString80CineS,     kString80MusicS,        kString81CineS,     kString81MusicS,
        kString80CubeS,             kStringBFormat1stOrderS,    kString71CineTopCenterS,
        kString71CineCenterHighS,   kString71CineFrontHighS,    kString71CineSideHighS,
        kString71CineFullRearS,     kString90S,                 kString91S,
        kString100S,        kString101S,            kString110S,        kString111S,
        kString130S,        kString131S,            kString102S,        kString122S,
        nullptr
    };

    return StringArray (arrangements);
}

/** Get a list of speaker arrangements as per their named configurations

    (e.g.: 2 regular channels, aliased as 'kStringStereoS', is "L R")
*/
static StringArray getNamedSpeakerArrangements()
{
    using namespace Vst::SpeakerArr;

    const Vst::CString arrangements[] =
    {
        kStringEmpty,       kStringMono,            kStringStereo,              kStringStereoR,
        kStringStereoC,     kStringStereoSide,      kStringStereoCLfe,          kString30Cine,
        kString30Music,     kString31Cine,          kString31Music,             kString40Cine,
        kString40Music,     kString41Cine,          kString41Music,             kString50,
        kString51,          kString60Cine,          kString60Music,             kString61Cine,
        kString61Music,     kString70Cine,          kString70Music,             kString71Cine,
        kString71Music,     kString71CineTopCenter, kString71CineCenterHigh,
        kString71CineFrontHigh,         kString71CineSideHigh,          kString71CineFullRear,
        kString80Cine,      kString80Music,         kString80Cube,              kString81Cine,
        kString81Music,     kString102,             kString122,                 kString90,
        kString91,          kString100,             kString101,                 kString110,
        kString111,         kString130,             kString131,
        nullptr
    };

    return StringArray (arrangements);
}

static Vst::SpeakerArrangement getSpeakerArrangementFrom (const String& string)
{
    return Vst::SpeakerArr::getSpeakerArrangementFromString (string.toUTF8());
}

//==============================================================================
static StringArray getPluginEffectCategories()
{
    using namespace Vst::PlugType;

    const Vst::CString categories[] =
    {
        kFxAnalyzer,            kFxDelay,       kFxDistortion,      kFxDynamics,
        kFxEQ,                  kFxFilter,      kFx,                kFxInstrument,
        kFxInstrumentExternal,  kFxSpatial,     kFxGenerator,       kFxMastering,
        kFxModulation,          kFxPitchShift,  kFxRestoration,     kFxReverb,
        kFxSurround,            kFxTools,       kSpatial,           kSpatialFx,
        nullptr
    };

    return StringArray (categories);
}

static StringArray getPluginInstrumentCategories()
{
    using namespace Vst::PlugType;

    const Vst::CString categories[] =
    {
        kInstrumentSynthSampler,    kInstrumentDrum,
        kInstrumentSampler,         kInstrumentSynth,
        kInstrumentExternal,        kFxInstrument,
        kFxInstrumentExternal,      kFxSpatial,
        kFxGenerator,
        nullptr
    };

    return StringArray (categories);
}

//==============================================================================
class VST3PluginInstance;

class VST3HostContext  : public Vst::IComponentHandler,  // From VST V3.0.0
                         public Vst::IComponentHandler2, // From VST V3.1.0 (a very well named class, of course!)
                         public Vst::IComponentHandler3, // From VST V3.5.0 (also very well named!)
                         public Vst::IContextMenuTarget,
                         public Vst::IHostApplication,
                         public Vst::IUnitHandler
{
public:
    VST3HostContext (VST3PluginInstance* pluginInstance)  : owner (pluginInstance)
    {
        appName = File::getSpecialLocation (File::currentApplicationFile).getFileNameWithoutExtension();
        attributeList = new AttributeList (this);
    }

    virtual ~VST3HostContext() {}

    JUCE_DECLARE_VST3_COM_REF_METHODS

    FUnknown* getFUnknown()     { return static_cast<Vst::IComponentHandler*> (this); }

    static bool hasFlag (Steinberg::int32 source, Steinberg::int32 flag) noexcept
    {
        return (source & flag) == flag;
    }

    //==============================================================================
    tresult PLUGIN_API beginEdit (Vst::ParamID paramID) override
    {
        const int index = getIndexOfParamID (paramID);

        if (index < 0)
            return kResultFalse;

        owner->beginParameterChangeGesture (index);
        return kResultTrue;
    }

    tresult PLUGIN_API performEdit (Vst::ParamID paramID, Vst::ParamValue valueNormalized) override
    {
        const int index = getIndexOfParamID (paramID);

        if (index < 0)
            return kResultFalse;

        owner->sendParamChangeMessageToListeners (index, (float) valueNormalized);
        return owner->editController->setParamNormalized (paramID, valueNormalized);
    }

    tresult PLUGIN_API endEdit (Vst::ParamID paramID) override
    {
        const int index = getIndexOfParamID (paramID);

        if (index < 0)
            return kResultFalse;

        owner->endParameterChangeGesture (index);
        return kResultTrue;
    }

    tresult PLUGIN_API restartComponent (Steinberg::int32 flags) override
    {
        if (owner != nullptr)
        {
            if (hasFlag (flags, Vst::kReloadComponent))
                owner->reset();

            if (hasFlag (flags, Vst::kIoChanged))
            {
                const double sampleRate = owner->getSampleRate();
                const int blockSize = owner->getBlockSize();

                owner->prepareToPlay (sampleRate >= 8000 ? sampleRate : 44100.0,
                                      blockSize > 0 ? blockSize : 1024);
            }

            if (hasFlag (flags, Vst::kLatencyChanged))
                if (owner->processor != nullptr)
                    owner->setLatencySamples (jmax (0, (int) owner->processor->getLatencySamples()));

            owner->updateHostDisplay();
            return kResultTrue;
        }

        jassertfalse;
        return kResultFalse;
    }

    //==============================================================================
    tresult PLUGIN_API setDirty (TBool) override
    {
        return kResultFalse;
    }

    tresult PLUGIN_API requestOpenEditor (FIDString name) override
    {
        (void) name;
        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API startGroupEdit() override
    {
        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API finishGroupEdit() override
    {
        jassertfalse;
        return kResultFalse;
    }

    //==============================================================================
    class ContextMenu  : public Vst::IContextMenu
    {
    public:
        ContextMenu (VST3PluginInstance& pluginInstance)  : owner (pluginInstance) {}
        virtual ~ContextMenu() {}

        JUCE_DECLARE_VST3_COM_REF_METHODS
        JUCE_DECLARE_VST3_COM_QUERY_METHODS

        Steinberg::int32 PLUGIN_API getItemCount() override     { return (Steinberg::int32) items.size(); }

        tresult PLUGIN_API addItem (const Item& item, IContextMenuTarget* target) override
        {
            jassert (target != nullptr);

            ItemAndTarget newItem;
            newItem.item = item;
            newItem.target = target;

            items.add (newItem);
            return kResultOk;
        }

        tresult PLUGIN_API removeItem (const Item& toRemove, IContextMenuTarget* target) override
        {
            for (int i = items.size(); --i >= 0;)
            {
                ItemAndTarget& item = items.getReference(i);

                if (item.item.tag == toRemove.tag && item.target == target)
                    items.remove (i);
            }

            return kResultOk;
        }

        tresult PLUGIN_API getItem (Steinberg::int32 tag, Item& result, IContextMenuTarget** target) override
        {
            for (int i = 0; i < items.size(); ++i)
            {
                const ItemAndTarget& item = items.getReference(i);

                if (item.item.tag == tag)
                {
                    result = item.item;

                    if (target != nullptr)
                        *target = item.target;

                    return kResultTrue;
                }
            }

            zerostruct (result);
            return kResultFalse;
        }

        tresult PLUGIN_API popup (Steinberg::UCoord x, Steinberg::UCoord y) override
        {
            Array<const Item*> subItemStack;
            OwnedArray<PopupMenu> menuStack;
            PopupMenu* topLevelMenu = menuStack.add (new PopupMenu());

            for (int i = 0; i < items.size(); ++i)
            {
                const Item& item = items.getReference (i).item;

                PopupMenu* menuToUse = menuStack.getLast();

                if (hasFlag (item.flags, Item::kIsGroupStart & ~Item::kIsDisabled))
                {
                    subItemStack.add (&item);
                    menuStack.add (new PopupMenu());
                }
                else if (hasFlag (item.flags, Item::kIsGroupEnd))
                {
                    if (const Item* subItem = subItemStack.getLast())
                    {
                        if (PopupMenu* m = menuStack [menuStack.size() - 2])
                            m->addSubMenu (toString (subItem->name), *menuToUse,
                                           ! hasFlag (subItem->flags, Item::kIsDisabled),
                                           nullptr,
                                           hasFlag (subItem->flags, Item::kIsChecked));

                        menuStack.removeLast (1);
                        subItemStack.removeLast (1);
                    }
                }
                else if (hasFlag (item.flags, Item::kIsSeparator))
                {
                    menuToUse->addSeparator();
                }
                else
                {
                    menuToUse->addItem (item.tag != 0 ? (int) item.tag : (int) zeroTagReplacement,
                                        toString (item.name),
                                        ! hasFlag (item.flags, Item::kIsDisabled),
                                        hasFlag (item.flags, Item::kIsChecked));
                }
            }

            PopupMenu::Options options;

            if (AudioProcessorEditor* ed = owner.getActiveEditor())
                options = options.withTargetScreenArea (ed->getScreenBounds().translated ((int) x, (int) y).withSize (1, 1));

           #if JUCE_MODAL_LOOPS_PERMITTED
            // Unfortunately, Steinberg's docs explicitly say this should be modal..
            handleResult (topLevelMenu->showMenu (options));
           #else
            topLevelMenu->showMenuAsync (options, ModalCallbackFunction::create (menuFinished, ComSmartPtr<ContextMenu> (this)));
           #endif

            return kResultOk;
        }

       #if ! JUCE_MODAL_LOOPS_PERMITTED
        static void menuFinished (int modalResult, ComSmartPtr<ContextMenu> menu)  { menu->handleResult (modalResult); }
       #endif

    private:
        enum { zeroTagReplacement = 0x7fffffff };

        Atomic<int> refCount;
        VST3PluginInstance& owner;

        struct ItemAndTarget
        {
            Item item;
            ComSmartPtr<IContextMenuTarget> target;
        };

        Array<ItemAndTarget> items;

        void handleResult (int result)
        {
            if (result == 0)
                return;

            if (result == zeroTagReplacement)
                result = 0;

            for (int i = 0; i < items.size(); ++i)
            {
                const ItemAndTarget& item = items.getReference(i);

                if ((int) item.item.tag == result)
                {
                    if (item.target != nullptr)
                        item.target->executeMenuItem ((Steinberg::int32) result);

                    break;
                }
            }
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContextMenu)
    };

    Vst::IContextMenu* PLUGIN_API createContextMenu (IPlugView*, const Vst::ParamID*) override
    {
        if (owner != nullptr)
            return new ContextMenu (*owner);

        return nullptr;
    }

    tresult PLUGIN_API executeMenuItem (Steinberg::int32) override
    {
        jassertfalse;
        return kResultFalse;
    }

    //==============================================================================
    tresult PLUGIN_API getName (Vst::String128 name) override
    {
        Steinberg::String str (appName.toUTF8());
        str.copyTo (name, 0, 127);
        return kResultOk;
    }

    tresult PLUGIN_API createInstance (TUID cid, TUID iid, void** obj) override
    {
        *obj = nullptr;

        if (! doUIDsMatch (cid, iid))
        {
            jassertfalse;
            return kInvalidArgument;
        }

        if (doUIDsMatch (cid, Vst::IMessage::iid) && doUIDsMatch (iid, Vst::IMessage::iid))
        {
            ComSmartPtr<Message> m (new Message (*this, attributeList));
            messageQueue.add (m);
            m->addRef();
            *obj = m;
            return kResultOk;
        }
        else if (doUIDsMatch (cid, Vst::IAttributeList::iid) && doUIDsMatch (iid, Vst::IAttributeList::iid))
        {
            ComSmartPtr<AttributeList> l (new AttributeList (this));
            l->addRef();
            *obj = l;
            return kResultOk;
        }

        jassertfalse;
        return kNotImplemented;
    }

    //==============================================================================
    tresult PLUGIN_API notifyUnitSelection (Vst::UnitID) override
    {
        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API notifyProgramListChange (Vst::ProgramListID, Steinberg::int32) override
    {
        jassertfalse;
        return kResultFalse;
    }

    //==============================================================================
    tresult PLUGIN_API queryInterface (const TUID iid, void** obj) override
    {
        if (doUIDsMatch (iid, Vst::IAttributeList::iid))
        {
            *obj = attributeList.get();
            return kResultOk;
        }

        TEST_FOR_AND_RETURN_IF_VALID (iid, Vst::IComponentHandler)
        TEST_FOR_AND_RETURN_IF_VALID (iid, Vst::IComponentHandler2)
        TEST_FOR_AND_RETURN_IF_VALID (iid, Vst::IComponentHandler3)
        TEST_FOR_AND_RETURN_IF_VALID (iid, Vst::IContextMenuTarget)
        TEST_FOR_AND_RETURN_IF_VALID (iid, Vst::IHostApplication)
        TEST_FOR_AND_RETURN_IF_VALID (iid, Vst::IUnitHandler)
        TEST_FOR_COMMON_BASE_AND_RETURN_IF_VALID (iid, FUnknown, Vst::IComponentHandler)

        *obj = nullptr;
        return kNotImplemented;
    }

private:
    //==============================================================================
    VST3PluginInstance* const owner;
    Atomic<int> refCount;
    String appName;

    typedef std::map<Vst::ParamID, int> ParamMapType;
    ParamMapType paramToIndexMap;

    int getIndexOfParamID (Vst::ParamID paramID)
    {
        if (owner == nullptr || owner->editController == nullptr)
            return -1;

        int result = getMappedParamID (paramID);

        if (result < 0)
        {
            const int numParams = owner->editController->getParameterCount();

            for (int i = 0; i < numParams; ++i)
            {
                Vst::ParameterInfo paramInfo;
                owner->editController->getParameterInfo (i, paramInfo);
                paramToIndexMap[paramInfo.id] = i;
            }

            result = getMappedParamID (paramID);
        }

        return result;
    }

    int getMappedParamID (Vst::ParamID paramID)
    {
        const ParamMapType::iterator it (paramToIndexMap.find (paramID));
        return it != paramToIndexMap.end() ? it->second : -1;
    }

    //==============================================================================
    class Message  : public Vst::IMessage
    {
    public:
        Message (VST3HostContext& o, Vst::IAttributeList* list)
           : owner (o), attributeList (list)
        {
        }

        Message (VST3HostContext& o, Vst::IAttributeList* list, FIDString id)
           : owner (o), attributeList (list), messageId (toString (id))
        {
        }

        Message (VST3HostContext& o, Vst::IAttributeList* list, FIDString id, const var& v)
           : value (v), owner (o), attributeList (list), messageId (toString (id))
        {
        }

        virtual ~Message() {}

        JUCE_DECLARE_VST3_COM_REF_METHODS
        JUCE_DECLARE_VST3_COM_QUERY_METHODS

        FIDString PLUGIN_API getMessageID() override              { return messageId.toRawUTF8(); }
        void PLUGIN_API setMessageID (FIDString id) override      { messageId = toString (id); }
        Vst::IAttributeList* PLUGIN_API getAttributes() override  { return attributeList; }

        var value;

    private:
        VST3HostContext& owner;
        ComSmartPtr<Vst::IAttributeList> attributeList;
        String messageId;
        Atomic<int> refCount;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Message)
    };

    Array<ComSmartPtr<Message>, CriticalSection> messageQueue;

    //==============================================================================
    class AttributeList  : public Vst::IAttributeList
    {
    public:
        AttributeList (VST3HostContext* o)  : owner (o) {}
        virtual ~AttributeList() {}

        JUCE_DECLARE_VST3_COM_REF_METHODS
        JUCE_DECLARE_VST3_COM_QUERY_METHODS

        //==============================================================================
        tresult PLUGIN_API setInt (AttrID id, Steinberg::int64 value) override
        {
            addMessageToQueue (id, value);
            return kResultTrue;
        }

        tresult PLUGIN_API setFloat (AttrID id, double value) override
        {
            addMessageToQueue (id, value);
            return kResultTrue;
        }

        tresult PLUGIN_API setString (AttrID id, const Vst::TChar* string) override
        {
            addMessageToQueue (id, toString (string));
            return kResultTrue;
        }

        tresult PLUGIN_API setBinary (AttrID id, const void* data, Steinberg::uint32 size) override
        {
            jassert (size >= 0 && (data != nullptr || size == 0));
            addMessageToQueue (id, MemoryBlock (data, (size_t) size));
            return kResultTrue;
        }

        //==============================================================================
        tresult PLUGIN_API getInt (AttrID id, Steinberg::int64& result) override
        {
            jassert (id != nullptr);

            if (findMessageOnQueueWithID (id, result))
                return kResultTrue;

            jassertfalse;
            return kResultFalse;
        }

        tresult PLUGIN_API getFloat (AttrID id, double& result) override
        {
            jassert (id != nullptr);

            if (findMessageOnQueueWithID (id, result))
                return kResultTrue;

            jassertfalse;
            return kResultFalse;
        }

        tresult PLUGIN_API getString (AttrID id, Vst::TChar* result, Steinberg::uint32 length) override
        {
            jassert (id != nullptr);

            String stringToFetch;
            if (findMessageOnQueueWithID (id, stringToFetch))
            {
                Steinberg::String str (stringToFetch.toRawUTF8());
                str.copyTo (result, 0, (Steinberg::int32) jmin (length, (Steinberg::uint32) std::numeric_limits<Steinberg::int32>::max()));

                return kResultTrue;
            }

            jassertfalse;
            return kResultFalse;
        }

        tresult PLUGIN_API getBinary (AttrID id, const void*& data, Steinberg::uint32& size) override
        {
            jassert (id != nullptr);

            for (int i = owner->messageQueue.size(); --i >= 0;)
            {
                Message* const message = owner->messageQueue.getReference (i);

                if (std::strcmp (message->getMessageID(), id) == 0)
                {
                    if (MemoryBlock* binaryData = message->value.getBinaryData())
                    {
                        data = binaryData->getData();
                        size = (Steinberg::uint32) binaryData->getSize();
                        return kResultTrue;
                    }
                }
            }

            return kResultFalse;
        }

    private:
        VST3HostContext* owner;
        Atomic<int> refCount;

        //==============================================================================
        template<typename Type>
        void addMessageToQueue (AttrID id, const Type& value)
        {
            jassert (id != nullptr);

            for (int i = owner->messageQueue.size(); --i >= 0;)
            {
                VST3HostContext::Message* const message = owner->messageQueue.getReference (i);

                if (std::strcmp (message->getMessageID(), id) == 0)
                {
                    message->value = value;
                    return;
                }
            }

            owner->messageQueue.add (ComSmartPtr<Message> (new Message (*owner, this, id, value)));
        }

        template<typename Type>
        bool findMessageOnQueueWithID (AttrID id, Type& value)
        {
            jassert (id != nullptr);

            for (int i = owner->messageQueue.size(); --i >= 0;)
            {
                VST3HostContext::Message* const message = owner->messageQueue.getReference (i);

                if (std::strcmp (message->getMessageID(), id) == 0)
                {
                    value = message->value;
                    return true;
                }
            }

            return false;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeList)
    };

    ComSmartPtr<AttributeList> attributeList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3HostContext)
};

//==============================================================================
class DescriptionFactory
{
public:
    DescriptionFactory (VST3HostContext* host, IPluginFactory* pluginFactory)
        : vst3HostContext (host), factory (pluginFactory)
    {
        jassert (pluginFactory != nullptr);
    }

    virtual ~DescriptionFactory() {}

    Result findDescriptionsAndPerform (const File& file)
    {
        StringArray foundNames;
        PFactoryInfo factoryInfo;
        factory->getFactoryInfo (&factoryInfo);
        const String companyName (toString (factoryInfo.vendor).trim());

        Result result (Result::ok());

        const Steinberg::int32 numClasses = factory->countClasses();

        for (Steinberg::int32 i = 0; i < numClasses; ++i)
        {
            PClassInfo info;
            factory->getClassInfo (i, &info);

            if (std::strcmp (info.category, kVstAudioEffectClass) != 0)
                continue;

            const String name (toString (info.name).trim());

            if (foundNames.contains (name, true))
                continue;

            ScopedPointer<PClassInfo2> info2;
            ScopedPointer<PClassInfoW> infoW;

            {
                ComSmartPtr<IPluginFactory2> pf2;
                ComSmartPtr<IPluginFactory3> pf3;

                if (pf2.loadFrom (factory))
                {
                    info2 = new PClassInfo2();
                    pf2->getClassInfo2 (i, info2);
                }

                if (pf3.loadFrom (factory))
                {
                    infoW = new PClassInfoW();
                    pf3->getClassInfoUnicode (i, infoW);
                }
            }

            foundNames.add (name);

            PluginDescription desc;

            {
                ComSmartPtr<Vst::IComponent> component;

                if (component.loadFrom (factory, info.cid))
                {
                    if (component->initialize (vst3HostContext->getFUnknown()) == kResultOk)
                    {
                        const int numInputs  = getNumSingleDirectionChannelsFor (component, true, true);
                        const int numOutputs = getNumSingleDirectionChannelsFor (component, false, true);

                        createPluginDescription (desc, file, companyName, name,
                                                 info, info2, infoW, numInputs, numOutputs);

                        component->terminate();
                    }
                    else
                    {
                        jassertfalse;
                    }
                }
                else
                {
                    jassertfalse;
                }
            }

            result = performOnDescription (desc);

            if (result.failed())
                break;
        }

        return result;
    }

protected:
    virtual Result performOnDescription (PluginDescription& description) = 0;

private:
    ComSmartPtr<VST3HostContext> vst3HostContext;
    ComSmartPtr<IPluginFactory> factory;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DescriptionFactory)
};

struct MatchingDescriptionFinder : public DescriptionFactory
{
    MatchingDescriptionFinder (VST3HostContext* host, IPluginFactory* pluginFactory, const PluginDescription& desc)
       : DescriptionFactory (host, pluginFactory), description (desc)
    {
    }

    static const char* getSuccessString() noexcept { return "Found Description"; }

    Result performOnDescription (PluginDescription& desc)
    {
        if (description.isDuplicateOf (desc))
            return Result::fail (getSuccessString());

        return Result::ok();
    }

private:
    const PluginDescription& description;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MatchingDescriptionFinder)
};

struct DescriptionLister : public DescriptionFactory
{
    DescriptionLister (VST3HostContext* host, IPluginFactory* pluginFactory)
        : DescriptionFactory (host, pluginFactory)
    {
    }

    Result performOnDescription (PluginDescription& desc)
    {
        list.add (new PluginDescription (desc));
        return Result::ok();
    }

    OwnedArray<PluginDescription> list;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DescriptionLister)
};

//==============================================================================
struct DLLHandle
{
    DLLHandle (const String& modulePath)
        : factory (nullptr)
    {
        if (modulePath.trim().isNotEmpty())
            open (modulePath);
    }

    ~DLLHandle()
    {
        typedef bool (PLUGIN_API *ExitModuleFn) ();

       #if JUCE_WINDOWS
        if (ExitModuleFn exitFn = (ExitModuleFn) getFunction ("ExitDll"))
            exitFn();

        releaseFactory();
        library.close();

       #else
        if (bundleRef != nullptr)
        {
            if (ExitModuleFn exitFn = (ExitModuleFn) getFunction ("bundleExit"))
                exitFn();

            releaseFactory();

            CFRelease (bundleRef);
            bundleRef = nullptr;
        }
       #endif
    }

    void open (const PluginDescription& description)
    {
       #if JUCE_WINDOWS
        jassert (description.fileOrIdentifier.isNotEmpty());
        jassert (File (description.fileOrIdentifier).existsAsFile());
        library.open (description.fileOrIdentifier);
       #else
        open (description.fileOrIdentifier);
       #endif
    }

    /** @note The factory should begin with a refCount of 1,
              so don't increment the reference count
              (ie: don't use a ComSmartPtr in here)!
              Its lifetime will be handled by this DllHandle,
              when such will be destroyed.

        @see releaseFactory
    */
    IPluginFactory* JUCE_CALLTYPE getPluginFactory()
    {
        if (factory == nullptr)
            if (GetFactoryProc proc = (GetFactoryProc) getFunction ("GetPluginFactory"))
                factory = proc();

        jassert (factory != nullptr); // The plugin NEEDS to provide a factory to be able to be called a VST3!
        return factory;
    }

    void* getFunction (const char* functionName)
    {
       #if JUCE_WINDOWS
        return library.getFunction (functionName);
       #else
        if (bundleRef == nullptr)
            return nullptr;

        CFStringRef name = String (functionName).toCFString();
        void* fn = CFBundleGetFunctionPointerForName (bundleRef, name);
        CFRelease (name);
        return fn;
       #endif
    }

private:
    IPluginFactory* factory;

    void releaseFactory()
    {
        if (factory != nullptr)
            factory->release();
    }

   #if JUCE_WINDOWS
    DynamicLibrary library;

    bool open (const String& filePath)
    {
        if (library.open (filePath))
        {
            typedef bool (PLUGIN_API *InitModuleProc) ();
            if (InitModuleProc proc = (InitModuleProc) getFunction ("InitDll"))
            {
                if (proc())
                    return true;
            }
            else
            {
                return true;
            }

            library.close();
        }

        return false;
    }

   #else
    CFBundleRef bundleRef;

    bool open (const String& filePath)
    {
        const File file (filePath);
        const char* const utf8 = file.getFullPathName().toRawUTF8();

        if (CFURLRef url = CFURLCreateFromFileSystemRepresentation (0, (const UInt8*) utf8, (CFIndex) std::strlen (utf8), file.isDirectory()))
        {
            bundleRef = CFBundleCreate (kCFAllocatorDefault, url);
            CFRelease (url);

            if (bundleRef != nullptr)
            {
                CFErrorRef error = nullptr;

                if (CFBundleLoadExecutableAndReturnError (bundleRef, &error))
                {
                    typedef bool (*BundleEntryProc)(CFBundleRef);

                    if (BundleEntryProc proc = (BundleEntryProc) getFunction ("bundleEntry"))
                    {
                        if (proc (bundleRef))
                            return true;
                    }
                    else
                    {
                        return true;
                    }
                }

                if (error != nullptr)
                {
                    if (CFStringRef failureMessage = CFErrorCopyFailureReason (error))
                    {
                        DBG (String::fromCFString (failureMessage));
                        CFRelease (failureMessage);
                    }

                    CFRelease (error);
                }

                CFRelease (bundleRef);
                bundleRef = nullptr;
            }
        }

        return false;
    }
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DLLHandle)
};

//==============================================================================
class VST3ModuleHandle  : public ReferenceCountedObject
{
public:
    explicit VST3ModuleHandle (const File& pluginFile)  : file (pluginFile)
    {
        getActiveModules().add (this);
    }

    ~VST3ModuleHandle()
    {
        getActiveModules().removeFirstMatchingValue (this);
    }

    /**
        Since there is no apparent indication if a VST3 plugin is a shell or not,
        we're stuck iterating through a VST3's factory, creating a description
        for every housed plugin.
    */
    static bool getAllDescriptionsForFile (OwnedArray<PluginDescription>& results,
                                           const String& fileOrIdentifier)
    {
        DLLHandle tempModule (fileOrIdentifier);

        ComSmartPtr<IPluginFactory> pluginFactory (tempModule.getPluginFactory());

        if (pluginFactory != nullptr)
        {
            ComSmartPtr<VST3HostContext> host (new VST3HostContext (nullptr));
            DescriptionLister lister (host, pluginFactory);
            const Result result (lister.findDescriptionsAndPerform (File (fileOrIdentifier)));

            results.addCopiesOf (lister.list);

            return result.wasOk();
        }

        jassertfalse;
        return false;
    }

    //==============================================================================
    typedef ReferenceCountedObjectPtr<VST3ModuleHandle> Ptr;

    static VST3ModuleHandle::Ptr findOrCreateModule (const File& file, const PluginDescription& description)
    {
        Array<VST3ModuleHandle*>& activeModules = getActiveModules();

        for (int i = activeModules.size(); --i >= 0;)
        {
            VST3ModuleHandle* const module = activeModules.getUnchecked (i);

            // VST3s are basically shells, you must therefore check their name along with their file:
            if (module->file == file && module->name == description.name)
                return module;
        }

        VST3ModuleHandle::Ptr m (new VST3ModuleHandle (file));

        if (! m->open (file, description))
            m = nullptr;

        return m;
    }

    //==============================================================================
    IPluginFactory* getPluginFactory()      { return dllHandle->getPluginFactory(); }

    File file;
    String name;

private:
    ScopedPointer<DLLHandle> dllHandle;

    //==============================================================================
    static Array<VST3ModuleHandle*>& getActiveModules()
    {
        static Array<VST3ModuleHandle*> activeModules;
        return activeModules;
    }

    //==============================================================================
    bool open (const File& f, const PluginDescription& description)
    {
        dllHandle = new DLLHandle (f.getFullPathName());

        ComSmartPtr<IPluginFactory> pluginFactory (dllHandle->getPluginFactory());

        if (pluginFactory != nullptr)
        {
            ComSmartPtr<VST3HostContext> host (new VST3HostContext (nullptr));
            MatchingDescriptionFinder finder (host, pluginFactory, description);

            const Result result (finder.findDescriptionsAndPerform (f));

            if (result.getErrorMessage() == MatchingDescriptionFinder::getSuccessString())
            {
                name = description.name;
                return true;
            }
        }

        return false;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3ModuleHandle)
};

//==============================================================================
class VST3PluginWindow : public AudioProcessorEditor,
                         public ComponentMovementWatcher,
                         public IPlugFrame
{
public:
    VST3PluginWindow (AudioProcessor* owner, IPlugView* pluginView)
      : AudioProcessorEditor (owner),
        ComponentMovementWatcher (this),
        refCount (1),
        view (pluginView, false),
        pluginHandle (nullptr),
        recursiveResize (false)
    {
        setSize (10, 10);
        setOpaque (true);
        setVisible (true);

        warnOnFailure (view->setFrame (this));

        ViewRect rect;
        warnOnFailure (view->getSize (&rect));
        resizeWithRect (*this, rect);
    }

    ~VST3PluginWindow()
    {
        warnOnFailure (view->removed());
        warnOnFailure (view->setFrame (nullptr));

        processor.editorBeingDeleted (this);

       #if JUCE_MAC
        dummyComponent.setView (nullptr);
       #endif

        view = nullptr;
    }

    JUCE_DECLARE_VST3_COM_REF_METHODS
    JUCE_DECLARE_VST3_COM_QUERY_METHODS

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        view->onWheel (wheel.deltaY);
    }

    void focusGained (FocusChangeType) override { view->onFocus (true); }
    void focusLost (FocusChangeType) override   { view->onFocus (false); }

    /** It seems that most, if not all, plugins do their own keyboard hooks,
        but IPlugView does have a set of keyboard related methods...
    */
    bool keyStateChanged (bool /*isKeyDown*/) override { return true; }
    bool keyPressed (const KeyPress& /*key*/) override { return true; }

    //==============================================================================
    void componentMovedOrResized (bool, bool wasResized) override
    {
        if (recursiveResize)
            return;

        Component* const topComp = getTopLevelComponent();

        if (topComp->getPeer() != nullptr)
        {
           #if JUCE_WINDOWS
            const Point<int> pos (topComp->getLocalPoint (this, Point<int>()));
           #endif

            recursiveResize = true;

            ViewRect rect;

            if (wasResized && view->canResize() == kResultTrue)
            {
                rect.right  = (Steinberg::int32) getWidth();
                rect.bottom = (Steinberg::int32) getHeight();
                view->checkSizeConstraint (&rect);

                setSize ((int) rect.getWidth(), (int) rect.getHeight());

               #if JUCE_WINDOWS
                SetWindowPos (pluginHandle, 0,
                              pos.x, pos.y, rect.getWidth(), rect.getHeight(),
                              isVisible() ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
               #elif JUCE_MAC
                dummyComponent.setBounds (getLocalBounds());
               #endif

                view->onSize (&rect);
            }
            else
            {
                warnOnFailure (view->getSize (&rect));

               #if JUCE_WINDOWS
                SetWindowPos (pluginHandle, 0,
                              pos.x, pos.y, rect.getWidth(), rect.getHeight(),
                              isVisible() ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
               #elif JUCE_MAC
                dummyComponent.setBounds (0, 0, (int) rect.getWidth(), (int) rect.getHeight());
               #endif
            }

            // Some plugins don't update their cursor correctly when mousing out the window
            Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();

            recursiveResize = false;
        }
    }

    void componentPeerChanged() override { }

    void componentVisibilityChanged() override
    {
        attachPluginWindow();
        componentMovedOrResized (true, true);
    }

    tresult PLUGIN_API resizeView (IPlugView* incomingView, ViewRect* newSize) override
    {
        if (incomingView != nullptr
             && newSize != nullptr
             && incomingView == view)
        {
            resizeWithRect (dummyComponent, *newSize);
            setSize (dummyComponent.getWidth(), dummyComponent.getHeight());
            return kResultTrue;
        }

        jassertfalse;
        return kInvalidArgument;
    }

private:
    //==============================================================================
    Atomic<int> refCount;
    ComSmartPtr<IPlugView> view;

   #if JUCE_WINDOWS
    class ChildComponent  : public Component
    {
    public:
        ChildComponent() {}
        void paint (Graphics& g) override  { g.fillAll (Colours::cornflowerblue); }

        using Component::createNewPeer;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChildComponent)
    };

    ChildComponent dummyComponent;
    ScopedPointer<ComponentPeer> peer;
    typedef HWND HandleFormat;
   #elif JUCE_MAC
    AutoResizingNSViewComponentWithParent dummyComponent;
    typedef NSView* HandleFormat;
   #else
    Component dummyComponent;
    typedef void* HandleFormat;
   #endif

    HandleFormat pluginHandle;
    bool recursiveResize;

    //==============================================================================
    static void resizeWithRect (Component& comp, const ViewRect& rect)
    {
        comp.setBounds ((int) rect.left, (int) rect.top,
                        jmax (10, std::abs ((int) rect.getWidth())),
                        jmax (10, std::abs ((int) rect.getHeight())));
    }

    void attachPluginWindow()
    {
        if (pluginHandle == nullptr)
        {
           #if JUCE_WINDOWS
            if (Component* topComp = getTopLevelComponent())
                peer = dummyComponent.createNewPeer (0, topComp->getWindowHandle());
            else
                peer = nullptr;

            if (peer != nullptr)
                pluginHandle = (HandleFormat) peer->getNativeHandle();
           #elif JUCE_MAC
            dummyComponent.setBounds (getLocalBounds());
            addAndMakeVisible (dummyComponent);
            pluginHandle = (NSView*) dummyComponent.getView();
            jassert (pluginHandle != nil);
           #endif

            if (pluginHandle != nullptr)
                warnOnFailure (view->attached (pluginHandle, defaultVST3WindowType));
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginWindow)
};

//==============================================================================
class VST3PluginInstance : public AudioPluginInstance
{
public:
    VST3PluginInstance (const VST3ModuleHandle::Ptr& handle)
      : module (handle),
        numInputAudioBusses (0),
        numOutputAudioBusses (0),
        inputParameterChanges (new ParamValueQueueList()),
        outputParameterChanges (new ParamValueQueueList()),
        midiInputs (new MidiEventList()),
        midiOutputs (new MidiEventList()),
        isComponentInitialised (false),
        isControllerInitialised (false),
        isActive (false)
    {
        host = new VST3HostContext (this);
    }

    ~VST3PluginInstance()
    {
        jassert (getActiveEditor() == nullptr); // You must delete any editors before deleting the plugin instance!

        releaseResources();

        if (editControllerConnection != nullptr && componentConnection != nullptr)
        {
            editControllerConnection->disconnect (componentConnection);
            componentConnection->disconnect (editControllerConnection);
        }

        editController->setComponentHandler (nullptr);

        if (isControllerInitialised)    editController->terminate();
        if (isComponentInitialised)     component->terminate();

        componentConnection = nullptr;
        editControllerConnection = nullptr;
        unitData = nullptr;
        unitInfo = nullptr;
        programListData = nullptr;
        componentHandler2 = nullptr;
        componentHandler = nullptr;
        processor = nullptr;
        editController2 = nullptr;
        editController = nullptr;
        component = nullptr;
        host = nullptr;
        module = nullptr;
    }

    bool initialise()
    {
       #if JUCE_WINDOWS
        // On Windows it's highly advisable to create your plugins using the message thread,
        // because many plugins need a chance to create HWNDs that will get their messages
        // delivered by the main message thread, and that's not possible from a background thread.
        jassert (MessageManager::getInstance()->isThisTheMessageThread());
       #endif

        ComSmartPtr<IPluginFactory> factory (module->getPluginFactory());

        PFactoryInfo factoryInfo;
        factory->getFactoryInfo (&factoryInfo);
        company = toString (factoryInfo.vendor).trim();

        if (! fetchComponentAndController (factory, factory->countClasses()))
            return false;

        // (May return an error if the plugin combines the IComponent and IEditController implementations)
        editController->initialize (host->getFUnknown());

        isControllerInitialised = true;
        editController->setComponentHandler (host);
        grabInformationObjects();
        synchroniseStates();
        interconnectComponentAndController();
        setupIO();
        return true;
    }

    //==============================================================================
    void fillInPluginDescription (PluginDescription& description) const override
    {
        jassert (module != nullptr);

        createPluginDescription (description, module->file,
                                 company, module->name,
                                 *info, info2, infoW,
                                 getNumInputChannels(),
                                 getNumOutputChannels());
    }

    void* getPlatformSpecificData() override   { return component; }
    void refreshParameterList() override {}

    //==============================================================================
    const String getName() const override
    {
        return module != nullptr ? module->name : String::empty;
    }

    void repopulateArrangements()
    {
        inputArrangements.clearQuick();
        outputArrangements.clearQuick();

        // NB: Some plugins need a valid arrangement despite specifying 0 for their I/O busses
        for (int i = 0; i < jmax (1, numInputAudioBusses); ++i)
            inputArrangements.add (getArrangementForBus (processor, true, i));

        for (int i = 0; i < jmax (1, numOutputAudioBusses); ++i)
            outputArrangements.add (getArrangementForBus (processor, false, i));
    }

    void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock) override
    {
        // Avoid redundantly calling things like setActive, which can be a heavy-duty call for some plugins:
        if (isActive
              && getSampleRate() == sampleRate
              && getBlockSize() == estimatedSamplesPerBlock)
            return;

        using namespace Vst;

        ProcessSetup setup;
        setup.symbolicSampleSize    = kSample32;
        setup.maxSamplesPerBlock    = estimatedSamplesPerBlock;
        setup.sampleRate            = sampleRate;
        setup.processMode           = isNonRealtime() ? kOffline : kRealtime;

        warnOnFailure (processor->setupProcessing (setup));

        if (! isComponentInitialised)
            isComponentInitialised = component->initialize (host->getFUnknown()) == kResultTrue;

        editController->setComponentHandler (host);

        if (inputArrangements.size() <= 0 || outputArrangements.size() <= 0)
            repopulateArrangements();

        warnOnFailure (processor->setBusArrangements (inputArrangements.getRawDataPointer(), numInputAudioBusses,
                                                      outputArrangements.getRawDataPointer(), numOutputAudioBusses));

        // Update the num. busses in case the configuration has been modified by the plugin. (May affect number of channels!):
        const int newNumInputAudioBusses = getNumSingleDirectionBussesFor (component, true, true);
        const int newNumOutputAudioBusses = getNumSingleDirectionBussesFor (component, false, true);

        // Repopulate arrangements if the number of busses have changed:
        if (numInputAudioBusses != newNumInputAudioBusses
             || numOutputAudioBusses != newNumOutputAudioBusses)
        {
            numInputAudioBusses = newNumInputAudioBusses;
            numOutputAudioBusses = newNumOutputAudioBusses;

            repopulateArrangements();
        }

        // Needed for having the same sample rate in processBlock(); some plugins need this!
        setPlayConfigDetails (getNumSingleDirectionChannelsFor (component, true, true),
                              getNumSingleDirectionChannelsFor (component, false, true),
                              sampleRate, estimatedSamplesPerBlock);

        setStateForAllBusses (true);

        setLatencySamples (jmax (0, (int) processor->getLatencySamples()));

        warnOnFailure (component->setActive (true));
        warnOnFailure (processor->setProcessing (true));

        isActive = true;
    }

    void releaseResources() override
    {
        if (! isActive)
            return; // Avoids redundantly calling things like setActive

        JUCE_TRY
        {
            isActive = false;

            setStateForAllBusses (false);

            if (processor != nullptr)
                warnOnFailure (processor->setProcessing (false));

            if (component != nullptr)
                warnOnFailure (component->setActive (false));
        }
        JUCE_CATCH_ALL_ASSERT
    }

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override
    {
        using namespace Vst;

        if (isActive
             && processor != nullptr
             && processor->canProcessSampleSize (kSample32) == kResultTrue)
        {
            const int numSamples = buffer.getNumSamples();

            ProcessData data;
            data.processMode            = isNonRealtime() ? kOffline : kRealtime;
            data.symbolicSampleSize     = kSample32;
            data.numInputs              = numInputAudioBusses;
            data.numOutputs             = numOutputAudioBusses;
            data.inputParameterChanges  = inputParameterChanges;
            data.outputParameterChanges = outputParameterChanges;
            data.numSamples             = (Steinberg::int32) numSamples;

            updateTimingInformation (data, getSampleRate());

            for (int i = getNumInputChannels(); i < buffer.getNumChannels(); ++i)
                buffer.clear (i, 0, numSamples);

            associateTo (data, buffer);
            associateTo (data, midiMessages);

            processor->process (data);

            MidiEventList::toMidiBuffer (midiMessages, *midiOutputs);

            inputParameterChanges->clearAllQueues();
        }
    }

    //==============================================================================
    String getChannelName (int channelIndex, bool forInput, bool forAudioChannel) const
    {
        const int numBusses = getNumSingleDirectionBussesFor (component, forInput, forAudioChannel);
        int numCountedChannels = 0;

        for (int i = 0; i < numBusses; ++i)
        {
            Vst::BusInfo busInfo (getBusInfo (forInput, forAudioChannel, i));

            numCountedChannels += busInfo.channelCount;

            if (channelIndex < numCountedChannels)
                return toString (busInfo.name);
        }

        return String::empty;
    }

    const String getInputChannelName  (int channelIndex) const override   { return getChannelName (channelIndex, true, true); }
    const String getOutputChannelName (int channelIndex) const override   { return getChannelName (channelIndex, false, true); }

    bool isInputChannelStereoPair (int channelIndex) const override
    {
        if (channelIndex < 0 || channelIndex >= getNumInputChannels())
            return false;

        return getBusInfo (true, true).channelCount == 2;
    }

    bool isOutputChannelStereoPair (int channelIndex) const override
    {
        if (channelIndex < 0 || channelIndex >= getNumOutputChannels())
            return false;

        return getBusInfo (false, true).channelCount == 2;
    }

    bool acceptsMidi() const override    { return getBusInfo (true,  false).channelCount > 0; }
    bool producesMidi() const override   { return getBusInfo (false, false).channelCount > 0; }

    //==============================================================================
    bool silenceInProducesSilenceOut() const override
    {
        if (processor != nullptr)
            return processor->getTailSamples() == Vst::kNoTail;

        return true;
    }

    /** May return a negative value as a means of informing us that the plugin has "infinite tail," or 0 for "no tail." */
    double getTailLengthSeconds() const override
    {
        if (processor != nullptr)
        {
            const double sampleRate = getSampleRate();

            if (sampleRate > 0.0)
                return jlimit (0, 0x7fffffff, (int) processor->getTailSamples()) / sampleRate;
        }

        return 0.0;
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override
    {
        if (IPlugView* view = tryCreatingView())
            return new VST3PluginWindow (this, view);

        return nullptr;
    }

    bool hasEditor() const override
    {
        // (if possible, avoid creating a second instance of the editor, because that crashes some plugins)
        if (getActiveEditor() != nullptr)
            return true;

        ComSmartPtr<IPlugView> view (tryCreatingView(), false);
        return view != nullptr;
    }

    //==============================================================================
    int getNumParameters() override
    {
        if (editController != nullptr)
            return (int) editController->getParameterCount();

        return 0;
    }

    const String getParameterName (int parameterIndex) override
    {
        return toString (getParameterInfoForIndex (parameterIndex).title);
    }

    float getParameter (int parameterIndex) override
    {
        if (editController != nullptr)
        {
            const uint32 id = getParameterInfoForIndex (parameterIndex).id;
            return (float) editController->getParamNormalized (id);
        }

        return 0.0f;
    }

    const String getParameterText (int parameterIndex) override
    {
        if (editController != nullptr)
        {
            const uint32 id = getParameterInfoForIndex (parameterIndex).id;

            Vst::String128 result;
            warnOnFailure (editController->getParamStringByValue (id, editController->getParamNormalized (id), result));

            return toString (result);
        }

        return String::empty;
    }

    void setParameter (int parameterIndex, float newValue) override
    {
        if (editController != nullptr)
        {
            const uint32 paramID = getParameterInfoForIndex (parameterIndex).id;
            editController->setParamNormalized (paramID, (double) newValue);

            Steinberg::int32 index;
            inputParameterChanges->addParameterData (paramID, index)->addPoint (0, newValue, index);
        }
    }

    //==============================================================================
    int getNumPrograms() override                        { return getProgramListInfo (0).programCount; }
    int getCurrentProgram() override                     { return 0; }
    void setCurrentProgram (int) override                {}
    void changeProgramName (int, const String&) override {}

    const String getProgramName (int index) override
    {
        Vst::String128 result;
        unitInfo->getProgramName (getProgramListInfo (0).id, index, result);
        return toString (result);
    }

    //==============================================================================
    void reset() override
    {
        if (component != nullptr)
        {
            component->setActive (false);
            component->setActive (true);
        }
    }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        XmlElement state ("VST3PluginState");

        appendStateFrom (state, component, "IComponent");
        appendStateFrom (state, editController, "IEditController");

        AudioProcessor::copyXmlToBinary (state, destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        ScopedPointer<XmlElement> head (AudioProcessor::getXmlFromBinary (data, sizeInBytes));

        if (head != nullptr)
        {
            ComSmartPtr<Steinberg::MemoryStream> s (createMemoryStreamForState (*head, "IComponent"));

            if (s != nullptr && component != nullptr)
                component->setState (s);

            if (editController != nullptr)
            {
                if (s != nullptr)
                    editController->setComponentState (s);

                s = createMemoryStreamForState (*head, "IEditController");

                if (s != nullptr)
                    editController->setState (s);
            }
        }
    }

    /** @note Not applicable to VST3 */
    void getCurrentProgramStateInformation (MemoryBlock& destData) override
    {
        destData.setSize (0, true);
    }

    /** @note Not applicable to VST3 */
    void setCurrentProgramStateInformation (const void* data, int sizeInBytes) override
    {
        (void) data;
        (void) sizeInBytes;
    }

    //==============================================================================
    // NB: this class and its subclasses must be public to avoid problems in
    // DLL builds under MSVC.
    class ParamValueQueueList  : public Vst::IParameterChanges
    {
    public:
        ParamValueQueueList() {}
        virtual ~ParamValueQueueList() {}

        JUCE_DECLARE_VST3_COM_REF_METHODS
        JUCE_DECLARE_VST3_COM_QUERY_METHODS

        Steinberg::int32 PLUGIN_API getParameterCount() override                                { return (Steinberg::int32) queues.size(); }
        Vst::IParamValueQueue* PLUGIN_API getParameterData (Steinberg::int32 index) override    { return queues[(int) index]; }

        Vst::IParamValueQueue* PLUGIN_API addParameterData (const Vst::ParamID& id, Steinberg::int32& index) override
        {
            for (int i = queues.size(); --i >= 0;)
            {
                if (queues.getUnchecked (i)->getParameterId() == id)
                {
                    index = (Steinberg::int32) i;
                    return queues.getUnchecked (i);
                }
            }

            index = getParameterCount();
            return queues.add (new ParamValueQueue (id));
        }

        void clearAllQueues() noexcept
        {
            for (int i = queues.size(); --i >= 0;)
                queues.getUnchecked (i)->clear();
        }

        struct ParamValueQueue  : public Vst::IParamValueQueue
        {
            ParamValueQueue (Vst::ParamID parameterID) : paramID (parameterID)
            {
                points.ensureStorageAllocated (1024);
            }

            virtual ~ParamValueQueue() {}

            JUCE_DECLARE_VST3_COM_REF_METHODS
            JUCE_DECLARE_VST3_COM_QUERY_METHODS

            Steinberg::Vst::ParamID PLUGIN_API getParameterId() override    { return paramID; }
            Steinberg::int32 PLUGIN_API getPointCount() override            { return (Steinberg::int32) points.size(); }

            Steinberg::tresult PLUGIN_API getPoint (Steinberg::int32 index,
                                                    Steinberg::int32& sampleOffset,
                                                    Steinberg::Vst::ParamValue& value) override
            {
                const ScopedLock sl (pointLock);

                if (isPositiveAndBelow ((int) index, points.size()))
                {
                    ParamPoint e (points.getUnchecked ((int) index));
                    sampleOffset = e.sampleOffset;
                    value = e.value;
                    return kResultTrue;
                }

                sampleOffset = -1;
                value = 0.0;
                return kResultFalse;
            }

            Steinberg::tresult PLUGIN_API addPoint (Steinberg::int32 sampleOffset,
                                                    Steinberg::Vst::ParamValue value,
                                                    Steinberg::int32& index) override
            {
                ParamPoint p = { sampleOffset, value };

                const ScopedLock sl (pointLock);
                index = (Steinberg::int32) points.size();
                points.add (p);
                return kResultTrue;
            }

            void clear() noexcept
            {
                const ScopedLock sl (pointLock);
                points.clearQuick();
            }

        private:
            struct ParamPoint
            {
                Steinberg::int32 sampleOffset;
                Steinberg::Vst::ParamValue value;
            };

            Atomic<int> refCount;
            const Vst::ParamID paramID;
            Array<ParamPoint> points;
            CriticalSection pointLock;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamValueQueue)
        };

        Atomic<int> refCount;
        OwnedArray<ParamValueQueue> queues;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamValueQueueList)
    };

private:
    //==============================================================================
    VST3ModuleHandle::Ptr module;

    friend VST3HostContext;
    ComSmartPtr<VST3HostContext> host;

    // Information objects:
    String company;
    ScopedPointer<PClassInfo> info;
    ScopedPointer<PClassInfo2> info2;
    ScopedPointer<PClassInfoW> infoW;

    // Rudimentary interfaces:
    ComSmartPtr<Vst::IComponent> component;
    ComSmartPtr<Vst::IEditController> editController;
    ComSmartPtr<Vst::IEditController2> editController2;
    ComSmartPtr<Vst::IAudioProcessor> processor;
    ComSmartPtr<Vst::IComponentHandler> componentHandler;
    ComSmartPtr<Vst::IComponentHandler2> componentHandler2;
    ComSmartPtr<Vst::IUnitInfo> unitInfo;
    ComSmartPtr<Vst::IUnitData> unitData;
    ComSmartPtr<Vst::IProgramListData> programListData;
    ComSmartPtr<Vst::IConnectionPoint> componentConnection;
    ComSmartPtr<Vst::IConnectionPoint> editControllerConnection;

    /** The number of IO busses MUST match that of the plugin,
        even if there aren't enough channels to process,
        as very poorly specified by the Steinberg SDK
    */
    int numInputAudioBusses, numOutputAudioBusses;
    Array<Vst::SpeakerArrangement> inputArrangements, outputArrangements; // Caching to improve performance and to avoid possible non-thread-safe calls to getBusArrangements().
    VST3BufferExchange::BusMap inputBusMap, outputBusMap;
    Array<Vst::AudioBusBuffers> inputBusses, outputBusses;

    //==============================================================================
    template <typename Type>
    static void appendStateFrom (XmlElement& head, ComSmartPtr<Type>& object, const String& identifier)
    {
        if (object != nullptr)
        {
            Steinberg::MemoryStream stream;

            if (object->getState (&stream) == kResultTrue)
            {
                MemoryBlock info (stream.getData(), (size_t) stream.getSize());
                head.createNewChildElement (identifier)->addTextElement (info.toBase64Encoding());
            }
        }
    }

    static Steinberg::MemoryStream* createMemoryStreamForState (XmlElement& head, StringRef identifier)
    {
        Steinberg::MemoryStream* stream = nullptr;

        if (XmlElement* const state = head.getChildByName (identifier))
        {
            MemoryBlock mem;

            if (mem.fromBase64Encoding (state->getAllSubText()))
            {
                stream = new Steinberg::MemoryStream();
                stream->setSize ((TSize) mem.getSize());
                mem.copyTo (stream->getData(), 0, mem.getSize());
            }
        }

        return stream;
    }

    ComSmartPtr<ParamValueQueueList> inputParameterChanges, outputParameterChanges;
    ComSmartPtr<MidiEventList> midiInputs, midiOutputs;
    Vst::ProcessContext timingInfo; //< Only use this in processBlock()!
    bool isComponentInitialised, isControllerInitialised, isActive;

    //==============================================================================
    bool fetchComponentAndController (IPluginFactory* factory, const Steinberg::int32 numClasses)
    {
        jassert (numClasses >= 0); // The plugin must provide at least an IComponent and IEditController!

        for (Steinberg::int32 j = 0; j < numClasses; ++j)
        {
            info = new PClassInfo();
            factory->getClassInfo (j, info);

            if (std::strcmp (info->category, kVstAudioEffectClass) != 0)
                continue;

            const String name (toString (info->name).trim());

            if (module->name != name)
                continue;

            {
                ComSmartPtr<IPluginFactory2> pf2;
                ComSmartPtr<IPluginFactory3> pf3;

                if (pf2.loadFrom (factory))
                {
                    info2 = new PClassInfo2();
                    pf2->getClassInfo2 (j, info2);
                }
                else
                {
                    info2 = nullptr;
                }

                if (pf3.loadFrom (factory))
                {
                    pf3->setHostContext (host->getFUnknown());
                    infoW = new PClassInfoW();
                    pf3->getClassInfoUnicode (j, infoW);
                }
                else
                {
                    infoW = nullptr;
                }
            }

            bool failed = true;

            if (component.loadFrom (factory, info->cid) && component != nullptr)
            {
                warnOnFailure (component->setIoMode (isNonRealtime() ? Vst::kOffline : Vst::kRealtime));

                if (warnOnFailure (component->initialize (host->getFUnknown())) != kResultOk)
                    return false;

                isComponentInitialised = true;

                // Get the IEditController:
                TUID controllerCID = { 0 };

                if (component->getControllerClassId (controllerCID) == kResultTrue && FUID (controllerCID).isValid())
                    editController.loadFrom (factory, controllerCID);

                if (editController == nullptr)
                {
                    // Try finding the IEditController the long way around:
                    for (Steinberg::int32 i = 0; i < numClasses; ++i)
                    {
                        PClassInfo classInfo;
                        factory->getClassInfo (i, &classInfo);

                        if (std::strcmp (classInfo.category, kVstComponentControllerClass) == 0)
                            editController.loadFrom (factory, classInfo.cid);
                    }
                }

                if (editController == nullptr)
                    editController.loadFrom (component);

                failed = editController == nullptr;
            }

            if (failed)
            {
                jassertfalse; // The plugin won't function without a valid IComponent and IEditController implementation!

                if (component != nullptr)
                {
                    component->terminate();
                    component = nullptr;
                }

                if (editController != nullptr)
                {
                    editController->terminate();
                    editController = nullptr;
                }

                break;
            }

            return true;
        }

        return false;
    }

    /** Some plugins need to be "connected" to intercommunicate between their implemented classes */
    void interconnectComponentAndController()
    {
        componentConnection.loadFrom (component);
        editControllerConnection.loadFrom (editController);

        if (componentConnection != nullptr && editControllerConnection != nullptr)
        {
            warnOnFailure (editControllerConnection->connect (componentConnection));
            warnOnFailure (componentConnection->connect (editControllerConnection));
        }
    }

    void synchroniseStates()
    {
        Steinberg::MemoryStream stream;

        if (component->getState (&stream) == kResultTrue)
            if (stream.seek (0, Steinberg::IBStream::kIBSeekSet, nullptr) == kResultTrue)
                warnOnFailure (editController->setComponentState (&stream));
    }

    void grabInformationObjects()
    {
        processor.loadFrom (component);
        unitInfo.loadFrom (component);
        programListData.loadFrom (component);
        unitData.loadFrom (component);
        editController2.loadFrom (component);
        componentHandler.loadFrom (component);
        componentHandler2.loadFrom (component);

        if (processor == nullptr)           processor.loadFrom (editController);
        if (unitInfo == nullptr)            unitInfo.loadFrom (editController);
        if (programListData == nullptr)     programListData.loadFrom (editController);
        if (unitData == nullptr)            unitData.loadFrom (editController);
        if (editController2 == nullptr)     editController2.loadFrom (editController);
        if (componentHandler == nullptr)    componentHandler.loadFrom (editController);
        if (componentHandler2 == nullptr)   componentHandler2.loadFrom (editController);
    }

    void setStateForAllBusses (bool newState)
    {
        setStateForAllBussesOfType (component, newState, true, true);    // Activate/deactivate audio inputs
        setStateForAllBussesOfType (component, newState, false, true);   // Activate/deactivate audio outputs
        setStateForAllBussesOfType (component, newState, true, false);   // Activate/deactivate MIDI inputs
        setStateForAllBussesOfType (component, newState, false, false);  // Activate/deactivate MIDI outputs
    }

    void setupIO()
    {
        setStateForAllBusses (true);

        Vst::ProcessSetup setup;
        setup.symbolicSampleSize   = Vst::kSample32;
        setup.maxSamplesPerBlock   = 1024;
        setup.sampleRate           = 44100.0;
        setup.processMode          = Vst::kRealtime;

        warnOnFailure (processor->setupProcessing (setup));

        numInputAudioBusses = getNumSingleDirectionBussesFor (component, true, true);
        numOutputAudioBusses = getNumSingleDirectionBussesFor (component, false, true);

        setPlayConfigDetails (getNumSingleDirectionChannelsFor (component, true, true),
                              getNumSingleDirectionChannelsFor (component, false, true),
                              setup.sampleRate, (int) setup.maxSamplesPerBlock);
    }

    //==============================================================================
    Vst::BusInfo getBusInfo (bool forInput, bool forAudio, int index = 0) const
    {
        Vst::BusInfo busInfo;
        busInfo.mediaType = forAudio ? Vst::kAudio : Vst::kEvent;
        busInfo.direction = forInput ? Vst::kInput : Vst::kOutput;

        component->getBusInfo (busInfo.mediaType, busInfo.direction,
                               (Steinberg::int32) index, busInfo);
        return busInfo;
    }

    //==============================================================================
    /** @note An IPlugView, when first created, should start with a ref-count of 1! */
    IPlugView* tryCreatingView() const
    {
        IPlugView* v = editController->createView (Vst::ViewType::kEditor);

        if (v == nullptr) v = editController->createView (nullptr);
        if (v == nullptr) editController->queryInterface (IPlugView::iid, (void**) &v);

        return v;
    }

    //==============================================================================
    void associateTo (Vst::ProcessData& destination, AudioSampleBuffer& buffer)
    {
        using namespace VST3BufferExchange;

        mapBufferToBusses (inputBusses, inputBusMap, inputArrangements, buffer);
        mapBufferToBusses (outputBusses, outputBusMap, outputArrangements, buffer);

        destination.inputs  = inputBusses.getRawDataPointer();
        destination.outputs = outputBusses.getRawDataPointer();
    }

    void associateTo (Vst::ProcessData& destination, MidiBuffer& midiBuffer)
    {
        midiInputs->clear();
        midiOutputs->clear();

        MidiEventList::toEventList (*midiInputs, midiBuffer);

        destination.inputEvents = midiInputs;
        destination.outputEvents = midiOutputs;
    }

    void updateTimingInformation (Vst::ProcessData& destination, double processSampleRate)
    {
        toProcessContext (timingInfo, getPlayHead(), processSampleRate);
        destination.processContext = &timingInfo;
    }

    Vst::ParameterInfo getParameterInfoForIndex (int index) const
    {
        Vst::ParameterInfo paramInfo = { 0 };

        if (processor != nullptr)
            editController->getParameterInfo (index, paramInfo);

        return paramInfo;
    }

    Vst::ProgramListInfo getProgramListInfo (int index) const
    {
        Vst::ProgramListInfo paramInfo = { 0 };

        if (unitInfo != nullptr)
            unitInfo->getProgramListInfo (index, paramInfo);

        return paramInfo;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginInstance)
};

};

//==============================================================================
VST3PluginFormat::VST3PluginFormat() {}
VST3PluginFormat::~VST3PluginFormat() {}

void VST3PluginFormat::findAllTypesForFile (OwnedArray<PluginDescription>& results, const String& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    VST3Classes::VST3ModuleHandle::getAllDescriptionsForFile (results, fileOrIdentifier);
}

AudioPluginInstance* VST3PluginFormat::createInstanceFromDescription (const PluginDescription& description, double, int)
{
    ScopedPointer<VST3Classes::VST3PluginInstance> result;

    if (fileMightContainThisPluginType (description.fileOrIdentifier))
    {
        File file (description.fileOrIdentifier);

        const File previousWorkingDirectory (File::getCurrentWorkingDirectory());
        file.getParentDirectory().setAsCurrentWorkingDirectory();

        if (const VST3Classes::VST3ModuleHandle::Ptr module = VST3Classes::VST3ModuleHandle::findOrCreateModule (file, description))
        {
            result = new VST3Classes::VST3PluginInstance (module);

            if (! result->initialise())
                result = nullptr;
        }

        previousWorkingDirectory.setAsCurrentWorkingDirectory();
    }

    return result.release();
}

bool VST3PluginFormat::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    const File f (File::createFileWithoutCheckingPath (fileOrIdentifier));

    return f.hasFileExtension (".vst3")
          #if JUCE_MAC
           && f.exists();
          #else
           && f.existsAsFile();
          #endif
}

String VST3PluginFormat::getNameOfPluginFromIdentifier (const String& fileOrIdentifier)
{
    return fileOrIdentifier; //Impossible to tell because every VST3 is a type of shell...
}

bool VST3PluginFormat::pluginNeedsRescanning (const PluginDescription& description)
{
    return File (description.fileOrIdentifier).getLastModificationTime() != description.lastFileModTime;
}

bool VST3PluginFormat::doesPluginStillExist (const PluginDescription& description)
{
    return File (description.fileOrIdentifier).exists();
}

StringArray VST3PluginFormat::searchPathsForPlugins (const FileSearchPath& directoriesToSearch, const bool recursive)
{
    StringArray results;

    for (int i = 0; i < directoriesToSearch.getNumPaths(); ++i)
        recursiveFileSearch (results, directoriesToSearch[i], recursive);

    return results;
}

void VST3PluginFormat::recursiveFileSearch (StringArray& results, const File& directory, const bool recursive)
{
    DirectoryIterator iter (directory, false, "*", File::findFilesAndDirectories);

    while (iter.next())
    {
        const File f (iter.getFile());
        bool isPlugin = false;

        if (fileMightContainThisPluginType (f.getFullPathName()))
        {
            isPlugin = true;
            results.add (f.getFullPathName());
        }

        if (recursive && (! isPlugin) && f.isDirectory())
            recursiveFileSearch (results, f, true);
    }
}

FileSearchPath VST3PluginFormat::getDefaultLocationsToSearch()
{
   #if JUCE_WINDOWS
    const String programFiles (File::getSpecialLocation (File::globalApplicationsDirectory).getFullPathName());
    return FileSearchPath (programFiles + "\\Common Files\\VST3");
   #elif JUCE_MAC
    return FileSearchPath ("/Library/Audio/Plug-Ins/VST3;~/Library/Audio/Plug-Ins/VST3");
   #else
    return FileSearchPath();
   #endif
}

#endif //JUCE_PLUGINHOST_VST3
