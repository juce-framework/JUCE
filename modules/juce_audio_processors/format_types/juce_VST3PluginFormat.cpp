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

#if JUCE_PLUGINHOST_VST3 && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD)

#include "juce_VST3Headers.h"
#include "juce_VST3Common.h"

namespace juce
{

using namespace Steinberg;

//==============================================================================
#ifndef JUCE_VST3_DEBUGGING
 #define JUCE_VST3_DEBUGGING 0
#endif

#if JUCE_VST3_DEBUGGING
 #define VST3_DBG(a) Logger::writeToLog (a);
#else
 #define VST3_DBG(a)
#endif

#if JUCE_DEBUG
static int warnOnFailure (int result) noexcept
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

static int warnOnFailureIfImplemented (int result) noexcept
{
    if (result != kResultOk && result != kNotImplemented)
        return warnOnFailure (result);

    return result;
}
#else
 #define warnOnFailure(x) x
 #define warnOnFailureIfImplemented(x) x
#endif

static std::vector<Vst::ParamID> getAllParamIDs (Vst::IEditController& controller)
{
    std::vector<Vst::ParamID> result;

    auto count = controller.getParameterCount();

    for (decltype (count) i = 0; i < count; ++i)
    {
        Vst::ParameterInfo info{};
        controller.getParameterInfo (i, info);
        result.push_back (info.id);
    }

    return result;
}

//==============================================================================
/*  Allows parameter updates to be queued up without blocking,
    and automatically dispatches these updates on the main thread.
*/
class EditControllerParameterDispatcher  : private Timer
{
public:
    ~EditControllerParameterDispatcher() override { stopTimer(); }

    void push (size_t index, float value)
    {
        if (controller == nullptr)
            return;

        if (MessageManager::getInstance()->isThisTheMessageThread())
            controller->setParamNormalized (cache.getParamID (index), value);
        else
            cache.set (index, value);
    }

    void start (Vst::IEditController& controllerIn)
    {
        controller = &controllerIn;
        cache = CachedParamValues { getAllParamIDs (controllerIn) };
        startTimerHz (60);
    }

    void flush()
    {
        cache.ifSet ([this] (size_t index, float value)
        {
            controller->setParamNormalized (cache.getParamID (index), value);
        });
    }

private:
    void timerCallback() override
    {
        flush();
    }

    CachedParamValues cache;
    Vst::IEditController* controller = nullptr;
};

//==============================================================================
std::array<uint32, 4> getNormalisedTUID (const TUID& tuid) noexcept
{
    const FUID fuid { tuid };
    return { { fuid.getLong1(), fuid.getLong2(), fuid.getLong3(), fuid.getLong4() } };
}

template <typename Range>
static int getHashForRange (Range&& range) noexcept
{
    uint32 value = 0;

    for (const auto& item : range)
        value = (value * 31) + (uint32) item;

    return (int) value;
}

template <typename ObjectType>
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
    description.lastInfoUpdateTime  = Time::getCurrentTime();
    description.manufacturerName    = company;
    description.name                = name;
    description.descriptiveName     = name;
    description.pluginFormatName    = "VST3";
    description.numInputChannels    = numInputs;
    description.numOutputChannels   = numOutputs;

    description.deprecatedUid       = getHashForRange (info.cid);
    description.uniqueId            = getHashForRange (getNormalisedTUID (info.cid));

    if (infoW != nullptr)      fillDescriptionWith (description, *infoW);
    else if (info2 != nullptr) fillDescriptionWith (description, *info2);

    if (description.category.isEmpty())
        description.category = toString (info.category).trim();

    description.isInstrument = description.category.containsIgnoreCase ("Instrument"); // This seems to be the only way to find that out! ARGH!
}

static int getNumSingleDirectionBusesFor (Vst::IComponent* component,
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
        numChannels += ((busInfo.flags & Vst::BusInfo::kDefaultActive) != 0 ? (int) busInfo.channelCount : 0);
    }

    return numChannels;
}

static void setStateForAllBusesOfType (Vst::IComponent* component,
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
static void toProcessContext (Vst::ProcessContext& context, AudioPlayHead* playHead, double sampleRate)
{
    jassert (sampleRate > 0.0); //Must always be valid, as stated by the VST3 SDK

    using namespace Vst;

    zerostruct (context);
    context.sampleRate = sampleRate;
    auto& fr = context.frameRate;

    if (playHead != nullptr)
    {
        AudioPlayHead::CurrentPositionInfo position;
        playHead->getCurrentPosition (position);

        context.projectTimeSamples  = position.timeInSamples; // Must always be valid, as stated by the VST3 SDK
        context.projectTimeMusic    = position.ppqPosition;   // Does not always need to be valid...
        context.tempo               = position.bpm;
        context.timeSigNumerator    = position.timeSigNumerator;
        context.timeSigDenominator  = position.timeSigDenominator;
        context.barPositionMusic    = position.ppqPositionOfLastBarStart;
        context.cycleStartMusic     = position.ppqLoopStart;
        context.cycleEndMusic       = position.ppqLoopEnd;

        switch (position.frameRate)
        {
            case AudioPlayHead::fps23976:    fr.framesPerSecond = 24; fr.flags = FrameRate::kPullDownRate; break;
            case AudioPlayHead::fps24:       fr.framesPerSecond = 24; fr.flags = 0; break;
            case AudioPlayHead::fps25:       fr.framesPerSecond = 25; fr.flags = 0; break;
            case AudioPlayHead::fps2997:     fr.framesPerSecond = 30; fr.flags = FrameRate::kPullDownRate; break;
            case AudioPlayHead::fps2997drop: fr.framesPerSecond = 30; fr.flags = FrameRate::kPullDownRate | FrameRate::kDropRate; break;
            case AudioPlayHead::fps30:       fr.framesPerSecond = 30; fr.flags = 0; break;
            case AudioPlayHead::fps30drop:   fr.framesPerSecond = 30; fr.flags = FrameRate::kDropRate; break;
            case AudioPlayHead::fps60:       fr.framesPerSecond = 60; fr.flags = 0; break;
            case AudioPlayHead::fps60drop:   fr.framesPerSecond = 60; fr.flags = FrameRate::kDropRate; break;
            case AudioPlayHead::fpsUnknown:  break;
            default:                         jassertfalse; break; // New frame rate?
        }

        if (position.isPlaying)     context.state |= ProcessContext::kPlaying;
        if (position.isRecording)   context.state |= ProcessContext::kRecording;
        if (position.isLooping)     context.state |= ProcessContext::kCycleActive;
    }
    else
    {
        context.tempo               = 120.0;
        context.timeSigNumerator    = 4;
        context.timeSigDenominator  = 4;
        fr.framesPerSecond          = 30;
        fr.flags                    = 0;
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
class VST3PluginInstance;

struct VST3HostContext  : public Vst::IComponentHandler,  // From VST V3.0.0
                          public Vst::IComponentHandler2, // From VST V3.1.0 (a very well named class, of course!)
                          public Vst::IComponentHandler3, // From VST V3.5.0 (also very well named!)
                          public Vst::IContextMenuTarget,
                          public Vst::IHostApplication,
                          public Vst::IUnitHandler
{
    VST3HostContext()
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
    tresult PLUGIN_API beginEdit (Vst::ParamID paramID) override;
    tresult PLUGIN_API performEdit (Vst::ParamID paramID, Vst::ParamValue valueNormalized) override;
    tresult PLUGIN_API endEdit (Vst::ParamID paramID) override;

    tresult PLUGIN_API restartComponent (Steinberg::int32 flags) override;

    //==============================================================================
    tresult PLUGIN_API setDirty (TBool) override
    {
        return kResultFalse;
    }

    tresult PLUGIN_API requestOpenEditor (FIDString name) override
    {
        ignoreUnused (name);
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

    void setPlugin (VST3PluginInstance* instance)
    {
        jassert (plugin == nullptr);
        plugin = instance;
    }

    //==============================================================================
    struct ContextMenu  : public Vst::IContextMenu
    {
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
                auto& item = items.getReference(i);

                if (item.item.tag == toRemove.tag && item.target == target)
                    items.remove (i);
            }

            return kResultOk;
        }

        tresult PLUGIN_API getItem (Steinberg::int32 tag, Item& result, IContextMenuTarget** target) override
        {
            for (int i = 0; i < items.size(); ++i)
            {
                auto& item = items.getReference(i);

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

        tresult PLUGIN_API popup (Steinberg::UCoord x, Steinberg::UCoord y) override;

       #if ! JUCE_MODAL_LOOPS_PERMITTED
        static void menuFinished (int modalResult, VSTComSmartPtr<ContextMenu> menu)  { menu->handleResult (modalResult); }
       #endif

    private:
        enum { zeroTagReplacement = 0x7fffffff };

        Atomic<int> refCount;
        VST3PluginInstance& owner;

        struct ItemAndTarget
        {
            Item item;
            VSTComSmartPtr<IContextMenuTarget> target;
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
                auto& item = items.getReference(i);

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
        if (plugin != nullptr)
            return new ContextMenu (*plugin);

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
            VSTComSmartPtr<Message> m (new Message (attributeList));
            messageMap.add (m);
            m->addRef();
            *obj = m;
            return kResultOk;
        }

        if (doUIDsMatch (cid, Vst::IAttributeList::iid) && doUIDsMatch (iid, Vst::IAttributeList::iid))
        {
            VSTComSmartPtr<AttributeList> l (new AttributeList (this));
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

    tresult PLUGIN_API notifyProgramListChange (Vst::ProgramListID, Steinberg::int32) override;

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
    VST3PluginInstance* plugin = nullptr;
    Atomic<int> refCount;
    String appName;

    //==============================================================================
    struct Message  : public Vst::IMessage
    {
        explicit Message (Vst::IAttributeList* list)
           : attributeList (list)
        {
        }

        Message (Vst::IAttributeList* list, FIDString id)
           : attributeList (list), messageId (toString (id))
        {
        }

        Message (Vst::IAttributeList* list, FIDString id, const var& v)
           : value (v), attributeList (list), messageId (toString (id))
        {
        }

        virtual ~Message() = default;

        JUCE_DECLARE_VST3_COM_REF_METHODS
        JUCE_DECLARE_VST3_COM_QUERY_METHODS

        FIDString PLUGIN_API getMessageID() override              { return messageId.toRawUTF8(); }
        void PLUGIN_API setMessageID (FIDString id) override      { messageId = toString (id); }
        Vst::IAttributeList* PLUGIN_API getAttributes() override  { return attributeList; }

        var value;

    private:
        VSTComSmartPtr<Vst::IAttributeList> attributeList;
        String messageId;
        Atomic<int> refCount;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Message)
    };

    class MessageMap
    {
    public:
        tresult getBinary (const char* id, const void*& data, Steinberg::uint32& size)
        {
            jassert (id != nullptr);

            const ScopedLock lock (mutex);

            const auto it = storage.find (id);

            if (it != storage.cend())
            {
                if (auto* binaryData = it->second->value.getBinaryData())
                {
                    data = binaryData->getData();
                    size = (Steinberg::uint32) binaryData->getSize();
                    return kResultTrue;
                }
            }

            return kResultFalse;
        }

        template <typename Type>
        void addMessageToQueue (const char* id, Vst::IAttributeList* list, const Type& value)
        {
            jassert (id != nullptr);

            const ScopedLock lock (mutex);

            const auto it = storage.find (id);

            if (it != storage.cend())
                it->second->value = value;
            else
                storage.emplace (id, new Message (list, id, value));
        }

        template <typename Type>
        bool findMessageOnQueueWithID (const char* id, Type& value)
        {
            jassert (id != nullptr);

            const ScopedLock lock (mutex);

            const auto it = storage.find (id);

            if (it == storage.cend())
                return false;

            value = it->second->value;
            return true;
        }

        void add (VSTComSmartPtr<Message> message)
        {
            const ScopedLock lock (mutex);

            const auto* id = message->getMessageID();
            storage.erase (id);
            storage.emplace (id, std::move (message));
        }

    private:
        struct Comparator
        {
            bool operator() (const char* a, const char* b) const noexcept
            {
                return std::strcmp (a, b) < 0;
            }
        };

        // Steinberg's docs say:
        // >  Please note that messages from the processor to the controller must not be sent during
        //    the process call, as this would not be fast enough and would break the real time
        //    processing. Such tasks should be handled in a separate timer thread.

        // Using a lock here is fine (plugins should be aware that sending messages is not
        // realtime-safe), and protects the data structure in case the processor sends messages from
        // a background thread rather than from the message thread.

        std::map<const char*, VSTComSmartPtr<Message>, Comparator> storage;
        CriticalSection mutex;
    };

    MessageMap messageMap;

    //==============================================================================
    struct AttributeList  : public Vst::IAttributeList
    {
        AttributeList (VST3HostContext* o)  : owner (o) {}
        virtual ~AttributeList() = default;

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
            jassert (data != nullptr || size == 0);
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
            return owner->messageMap.getBinary (id, data, size);
        }

    private:
        VST3HostContext* owner;
        Atomic<int> refCount;

        //==============================================================================
        template <typename Type>
        void addMessageToQueue (AttrID id, const Type& value)
        {
            owner->messageMap.addMessageToQueue (id, this, value);
        }

        template <typename Type>
        bool findMessageOnQueueWithID (AttrID id, Type& value)
        {
            return owner->messageMap.findMessageOnQueueWithID (id, value);
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeList)
    };

    VSTComSmartPtr<AttributeList> attributeList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3HostContext)
};

//==============================================================================
struct DescriptionFactory
{
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
        auto companyName = toString (factoryInfo.vendor).trim();

        Result result (Result::ok());

        auto numClasses = factory->countClasses();

        for (Steinberg::int32 i = 0; i < numClasses; ++i)
        {
            PClassInfo info;
            factory->getClassInfo (i, &info);

            if (std::strcmp (info.category, kVstAudioEffectClass) != 0)
                continue;

            const String name (toString (info.name).trim());

            if (foundNames.contains (name, true))
                continue;

            std::unique_ptr<PClassInfo2> info2;
            std::unique_ptr<PClassInfoW> infoW;

            {
                VSTComSmartPtr<IPluginFactory2> pf2;
                VSTComSmartPtr<IPluginFactory3> pf3;

                if (pf2.loadFrom (factory))
                {
                    info2.reset (new PClassInfo2());
                    pf2->getClassInfo2 (i, info2.get());
                }

                if (pf3.loadFrom (factory))
                {
                    infoW.reset (new PClassInfoW());
                    pf3->getClassInfoUnicode (i, infoW.get());
                }
            }

            foundNames.add (name);

            PluginDescription desc;

            {
                VSTComSmartPtr<Vst::IComponent> component;

                if (component.loadFrom (factory, info.cid))
                {
                    if (component->initialize (vst3HostContext->getFUnknown()) == kResultOk)
                    {
                        auto numInputs  = getNumSingleDirectionChannelsFor (component, true, true);
                        auto numOutputs = getNumSingleDirectionChannelsFor (component, false, true);

                        createPluginDescription (desc, file, companyName, name,
                                                 info, info2.get(), infoW.get(), numInputs, numOutputs);

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

            if (desc.uniqueId != 0)
                result = performOnDescription (desc);

            if (result.failed())
                break;
        }

        return result;
    }

    virtual Result performOnDescription (PluginDescription&) = 0;

private:
    VSTComSmartPtr<VST3HostContext> vst3HostContext;
    VSTComSmartPtr<IPluginFactory> factory;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DescriptionFactory)
};

struct DescriptionLister  : public DescriptionFactory
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
    DLLHandle (const File& fileToOpen)
       : dllFile (fileToOpen)
    {
        open();
    }

    ~DLLHandle()
    {
       #if JUCE_MAC
        if (bundleRef != nullptr)
       #endif
        {
            if (factory != nullptr)
                factory->release();

            using ExitModuleFn = bool (PLUGIN_API*) ();

            if (auto* exitFn = (ExitModuleFn) getFunction (exitFnName))
                exitFn();

           #if JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
            library.close();
           #endif
        }
    }

    //==============================================================================
    /** The factory should begin with a refCount of 1, so don't increment the reference count
        (ie: don't use a VSTComSmartPtr in here)! Its lifetime will be handled by this DLLHandle.
    */
    IPluginFactory* JUCE_CALLTYPE getPluginFactory()
    {
        if (factory == nullptr)
            if (auto* proc = (GetFactoryProc) getFunction (factoryFnName))
                factory = proc();

        // The plugin NEEDS to provide a factory to be able to be called a VST3!
        // Most likely you are trying to load a 32-bit VST3 from a 64-bit host
        // or vice versa.
        jassert (factory != nullptr);
        return factory;
    }

    void* getFunction (const char* functionName)
    {
       #if JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
        return library.getFunction (functionName);
       #elif JUCE_MAC
        if (bundleRef == nullptr)
            return nullptr;

        CFUniquePtr<CFStringRef> name (String (functionName).toCFString());
        return CFBundleGetFunctionPointerForName (bundleRef.get(), name.get());
       #endif
    }

    File getFile() const noexcept  { return dllFile; }

private:
    File dllFile;
    IPluginFactory* factory = nullptr;

    static constexpr const char* factoryFnName = "GetPluginFactory";

   #if JUCE_WINDOWS
    static constexpr const char* entryFnName = "InitDll";
    static constexpr const char* exitFnName  = "ExitDll";

    using EntryProc = bool (PLUGIN_API*) ();
   #elif JUCE_LINUX || JUCE_BSD
    static constexpr const char* entryFnName = "ModuleEntry";
    static constexpr const char* exitFnName  = "ModuleExit";

    using EntryProc = bool (PLUGIN_API*) (void*);
   #elif JUCE_MAC
    static constexpr const char* entryFnName = "bundleEntry";
    static constexpr const char* exitFnName  = "bundleExit";

    using EntryProc = bool (*) (CFBundleRef);
   #endif

    //==============================================================================
   #if JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
    DynamicLibrary library;

    bool open()
    {
        if (library.open (dllFile.getFullPathName()))
        {
            if (auto* proc = (EntryProc) getFunction (entryFnName))
            {
               #if JUCE_WINDOWS
                if (proc())
               #else
                if (proc (library.getNativeHandle()))
               #endif
                    return true;
            }
            else
            {
                // this is required for some plug-ins which don't export the dll entry point function
                return true;
            }

            library.close();
        }

        return false;
    }
   #elif JUCE_MAC
    CFUniquePtr<CFBundleRef> bundleRef;

    bool open()
    {
        auto* utf8 = dllFile.getFullPathName().toRawUTF8();

        if (auto url = CFUniquePtr<CFURLRef> (CFURLCreateFromFileSystemRepresentation (nullptr,
                                                                                       (const UInt8*) utf8,
                                                                                       (CFIndex) std::strlen (utf8),
                                                                                       dllFile.isDirectory())))
        {
            bundleRef.reset (CFBundleCreate (kCFAllocatorDefault, url.get()));

            if (bundleRef != nullptr)
            {
                CFObjectHolder<CFErrorRef> error;

                if (CFBundleLoadExecutableAndReturnError (bundleRef.get(), &error.object))
                    if (auto* proc = (EntryProc) getFunction (entryFnName))
                        if (proc (bundleRef.get()))
                            return true;

                if (error.object != nullptr)
                    if (auto failureMessage = CFUniquePtr<CFStringRef> (CFErrorCopyFailureReason (error.object)))
                        DBG (String::fromCFString (failureMessage.get()));

                bundleRef = nullptr;
            }
        }

        return false;
    }
   #endif

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DLLHandle)
};

struct DLLHandleCache  : public DeletedAtShutdown
{
    DLLHandleCache() = default;
    ~DLLHandleCache() override { clearSingletonInstance(); }

    JUCE_DECLARE_SINGLETON (DLLHandleCache, false)

    DLLHandle& findOrCreateHandle (const String& modulePath)
    {
       #if JUCE_LINUX || JUCE_BSD
        File file (getDLLFileFromBundle (modulePath));
       #else
        File file (modulePath);
       #endif

        auto it = std::find_if (openHandles.begin(), openHandles.end(),
                                [&] (const std::unique_ptr<DLLHandle>& handle)
                                {
                                     return file == handle->getFile();
                                });

        if (it != openHandles.end())
            return *it->get();

        openHandles.push_back (std::make_unique<DLLHandle> (file));
        return *openHandles.back().get();
    }

private:
   #if JUCE_LINUX || JUCE_BSD
    File getDLLFileFromBundle (const String& bundlePath) const
    {
        auto machineName = []() -> String
        {
            struct utsname unameData;
            auto res = uname (&unameData);

            if (res != 0)
                return {};

            return unameData.machine;
        }();

        File file (bundlePath);

        return file.getChildFile ("Contents")
                   .getChildFile (machineName + "-linux")
                   .getChildFile (file.getFileNameWithoutExtension() + ".so");
    }
   #endif

    std::vector<std::unique_ptr<DLLHandle>> openHandles;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DLLHandleCache)
};


JUCE_IMPLEMENT_SINGLETON (DLLHandleCache)

//==============================================================================
#if JUCE_LINUX || JUCE_BSD

class RunLoop  final  : public Steinberg::Linux::IRunLoop
{
public:
    RunLoop() = default;

    ~RunLoop()
    {
        for (const auto& h : eventHandlerMap)
            LinuxEventLoop::unregisterFdCallback (h.first);
    }

    //==============================================================================
    tresult PLUGIN_API registerEventHandler (Linux::IEventHandler* handler,
                                             Linux::FileDescriptor fd) override
    {
        if (handler == nullptr)
            return kInvalidArgument;

        auto& handlers = eventHandlerMap[fd];

        if (handlers.empty())
        {
            LinuxEventLoop::registerFdCallback (fd, [this] (int descriptor)
            {
                for (auto* h : eventHandlerMap[descriptor])
                    h->onFDIsSet (descriptor);

                return true;
            });
        }

        handlers.push_back (handler);

        return kResultTrue;
    }

    tresult PLUGIN_API unregisterEventHandler (Linux::IEventHandler* handler) override
    {
        if (handler == nullptr)
            return kInvalidArgument;

        for (auto iter = eventHandlerMap.begin(), end = eventHandlerMap.end(); iter != end;)
        {
            auto& handlers = iter->second;

            auto handlersIter = std::find (std::begin (handlers), std::end (handlers), handler);

            if (handlersIter != std::end (handlers))
            {
                handlers.erase (handlersIter);

                if (handlers.empty())
                {
                    LinuxEventLoop::unregisterFdCallback (iter->first);
                    iter = eventHandlerMap.erase (iter);
                    continue;
                }
            }

            ++iter;
        }

        return kResultTrue;
    }

    //==============================================================================
    tresult PLUGIN_API registerTimer (Linux::ITimerHandler* handler, Linux::TimerInterval milliseconds) override
    {
        if (handler == nullptr || milliseconds <= 0)
            return kInvalidArgument;

        timerCallers.emplace_back (handler, (int) milliseconds);
        return kResultTrue;
    }

    tresult PLUGIN_API unregisterTimer (Linux::ITimerHandler* handler) override
    {
        auto iter = std::find (timerCallers.begin(), timerCallers.end(), handler);

        if (iter == timerCallers.end())
            return kInvalidArgument;

        timerCallers.erase (iter);
        return kResultTrue;
    }

    //==============================================================================
    uint32 PLUGIN_API addRef() override                                { return 1000; }
    uint32 PLUGIN_API release() override                               { return 1000; }
    tresult PLUGIN_API queryInterface (const TUID, void**) override    { return kNoInterface; }

private:
    //==============================================================================
    struct TimerCaller  : private Timer
    {
        TimerCaller (Linux::ITimerHandler* h, int interval)  : handler (h)  { startTimer (interval); }
        ~TimerCaller() override { stopTimer(); }

        void timerCallback() override  { handler->onTimer(); }

        bool operator== (Linux::ITimerHandler* other) const noexcept { return handler == other; }

        Linux::ITimerHandler* handler = nullptr;
    };

    std::unordered_map<Linux::FileDescriptor, std::vector<Linux::IEventHandler*>> eventHandlerMap;
    std::list<TimerCaller> timerCallers;

    //==============================================================================
    JUCE_DECLARE_NON_MOVEABLE (RunLoop)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RunLoop)
};

#endif

//==============================================================================
struct VST3ModuleHandle  : public ReferenceCountedObject
{
    explicit VST3ModuleHandle (const File& pluginFile, const PluginDescription& pluginDesc)
        : file (pluginFile)
    {
        if (open (pluginDesc))
        {
            isOpen = true;
            getActiveModules().add (this);
        }
    }

    ~VST3ModuleHandle()
    {
        if (isOpen)
            getActiveModules().removeFirstMatchingValue (this);
    }

    //==============================================================================
    using Ptr = ReferenceCountedObjectPtr<VST3ModuleHandle>;

    static VST3ModuleHandle::Ptr findOrCreateModule (const File& file,
                                                     const PluginDescription& description)
    {
        for (auto* module : getActiveModules())
        {
            // VST3s are basically shells, you must therefore check their name along with their file:
            if (module->file == file && module->name == description.name)
                return module;
        }

        VST3ModuleHandle::Ptr modulePtr (new VST3ModuleHandle (file, description));

        if (! modulePtr->isOpen)
            modulePtr = nullptr;

        return modulePtr;
    }

    //==============================================================================
    IPluginFactory* getPluginFactory()
    {
        return DLLHandleCache::getInstance()->findOrCreateHandle (file.getFullPathName()).getPluginFactory();
    }

    File getFile() const noexcept    { return file; }
    String getName() const noexcept  { return name; }

private:
    //==============================================================================
    static Array<VST3ModuleHandle*>& getActiveModules()
    {
        static Array<VST3ModuleHandle*> activeModules;
        return activeModules;
    }

    //==============================================================================
    bool open (const PluginDescription& description)
    {
        VSTComSmartPtr<IPluginFactory> pluginFactory (DLLHandleCache::getInstance()->findOrCreateHandle (file.getFullPathName())
                                                                                    .getPluginFactory());

        if (pluginFactory != nullptr)
        {
            auto numClasses = pluginFactory->countClasses();

            for (Steinberg::int32 i = 0; i < numClasses; ++i)
            {
                PClassInfo info;
                pluginFactory->getClassInfo (i, &info);

                if (std::strcmp (info.category, kVstAudioEffectClass) != 0)
                    continue;

                if (toString (info.name).trim() == description.name
                    && (getHashForRange (getNormalisedTUID (info.cid)) == description.uniqueId
                        || getHashForRange (info.cid) == description.deprecatedUid))
                {
                    name = description.name;
                    return true;
                }
            }
        }

        return false;
    }

    File file;
    String name;
    bool isOpen = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3ModuleHandle)
};

//==============================================================================
struct VST3PluginWindow : public AudioProcessorEditor,
                          public ComponentMovementWatcher,
                          public ComponentPeer::ScaleFactorListener,
                          public IPlugFrame
{
    VST3PluginWindow (AudioProcessor* owner, IPlugView* pluginView)
      : AudioProcessorEditor (owner),
        ComponentMovementWatcher (this),
        view (pluginView, false)
    {
        setSize (10, 10);
        setOpaque (true);
        setVisible (true);

        warnOnFailure (view->setFrame (this));
        view->queryInterface (Steinberg::IPlugViewContentScaleSupport::iid, (void**) &scaleInterface);
        resizeToFit();
    }

    ~VST3PluginWindow() override
    {
        if (scaleInterface != nullptr)
            scaleInterface->release();

        removeScaleFactorListener();

        #if JUCE_LINUX || JUCE_BSD
         embeddedComponent.removeClient();
        #endif

        warnOnFailure (view->removed());
        warnOnFailure (view->setFrame (nullptr));

        processor.editorBeingDeleted (this);

       #if JUCE_MAC
        embeddedComponent.setView (nullptr);
       #endif

        view = nullptr;
    }

   #if JUCE_LINUX || JUCE_BSD
    Steinberg::tresult PLUGIN_API queryInterface (const Steinberg::TUID iid, void** obj) override
    {
        if (doUIDsMatch (iid, Steinberg::Linux::IRunLoop::iid))
        {
            *obj = &runLoop.get();
            return kResultTrue;
        }

        jassertfalse;
        *obj = nullptr;

        return Steinberg::kNotImplemented;
    }
   #else
    JUCE_DECLARE_VST3_COM_QUERY_METHODS
   #endif

    JUCE_DECLARE_VST3_COM_REF_METHODS

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        view->onWheel (wheel.deltaY);
    }

    void focusGained (FocusChangeType) override     { view->onFocus (true); }
    void focusLost (FocusChangeType) override       { view->onFocus (false); }

    /** It seems that most, if not all, plugins do their own keyboard hooks,
        but IPlugView does have a set of keyboard related methods...
    */
    bool keyStateChanged (bool /*isKeyDown*/) override  { return true; }
    bool keyPressed (const KeyPress& /*key*/) override  { return true; }

    //==============================================================================
    void componentPeerChanged() override
    {
        removeScaleFactorListener();
        currentPeer = getTopLevelComponent()->getPeer();

        if (currentPeer != nullptr)
        {
            currentPeer->addScaleFactorListener (this);
            nativeScaleFactor = (float) currentPeer->getPlatformScaleFactor();
        }
    }

    void componentMovedOrResized (bool, bool wasResized) override
    {
        if (recursiveResize || ! wasResized || getTopLevelComponent()->getPeer() == nullptr)
            return;

        ViewRect rect;

        if (view->canResize() == kResultTrue)
        {
            rect.right  = (Steinberg::int32) roundToInt ((float) getWidth()  * nativeScaleFactor);
            rect.bottom = (Steinberg::int32) roundToInt ((float) getHeight() * nativeScaleFactor);

            view->checkSizeConstraint (&rect);

            {
                const ScopedValueSetter<bool> recursiveResizeSetter (recursiveResize, true);

                setSize (roundToInt ((float) rect.getWidth()  / nativeScaleFactor),
                         roundToInt ((float) rect.getHeight() / nativeScaleFactor));
            }

           #if JUCE_WINDOWS
            setPluginWindowPos (rect);
           #else
            embeddedComponent.setBounds (getLocalBounds());
           #endif

            view->onSize (&rect);
        }
        else
        {
            warnOnFailure (view->getSize (&rect));

           #if JUCE_WINDOWS
            setPluginWindowPos (rect);
           #else
            resizeWithRect (embeddedComponent, rect, nativeScaleFactor);
           #endif
        }

        // Some plugins don't update their cursor correctly when mousing out the window
        Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
    }
    using ComponentMovementWatcher::componentMovedOrResized;

    void componentVisibilityChanged() override
    {
        attachPluginWindow();

        if (! hasDoneInitialResize)
            resizeToFit();

        componentMovedOrResized (true, true);
    }
    using ComponentMovementWatcher::componentVisibilityChanged;

    void nativeScaleFactorChanged (double newScaleFactor) override
    {
        if (pluginHandle == HandleFormat{} || approximatelyEqual ((float) newScaleFactor, nativeScaleFactor))
            return;

        nativeScaleFactor = (float) newScaleFactor;

        if (scaleInterface != nullptr)
            scaleInterface->setContentScaleFactor ((Steinberg::IPlugViewContentScaleSupport::ScaleFactor) nativeScaleFactor);
    }

    void resizeToFit()
    {
        ViewRect rect;
        warnOnFailure (view->getSize (&rect));
        resizeWithRect (*this, rect, nativeScaleFactor);

        hasDoneInitialResize = true;
    }

    tresult PLUGIN_API resizeView (IPlugView* incomingView, ViewRect* newSize) override
    {
        if (incomingView != nullptr && newSize != nullptr && incomingView == view)
        {
            resizeWithRect (embeddedComponent, *newSize, nativeScaleFactor);
            setSize (embeddedComponent.getWidth(), embeddedComponent.getHeight());

            return kResultTrue;
        }

        jassertfalse;
        return kInvalidArgument;
    }

private:
    //==============================================================================
    static void resizeWithRect (Component& comp, const ViewRect& rect, float scaleFactor)
    {
        comp.setBounds (roundToInt ((float) rect.left / scaleFactor),
                        roundToInt ((float) rect.top  / scaleFactor),
                        jmax (10, std::abs (roundToInt ((float) rect.getWidth()  / scaleFactor))),
                        jmax (10, std::abs (roundToInt ((float) rect.getHeight() / scaleFactor))));
    }

    void attachPluginWindow()
    {
        if (pluginHandle == HandleFormat{})
        {
            #if JUCE_WINDOWS
             if (auto* topComp = getTopLevelComponent())
             {
                 peer.reset (embeddedComponent.createNewPeer (0, topComp->getWindowHandle()));
                 pluginHandle = (HandleFormat) peer->getNativeHandle();
             }
            #else
             embeddedComponent.setBounds (getLocalBounds());
             addAndMakeVisible (embeddedComponent);
             #if JUCE_MAC
              pluginHandle = (HandleFormat) embeddedComponent.getView();
             #elif JUCE_LINUX || JUCE_BSD
              pluginHandle = (HandleFormat) embeddedComponent.getHostWindowID();
             #endif
            #endif

            if (pluginHandle == HandleFormat{})
            {
                jassertfalse;
                return;
            }

            warnOnFailure (view->attached ((void*) pluginHandle, defaultVST3WindowType));

            if (scaleInterface != nullptr)
                scaleInterface->setContentScaleFactor ((Steinberg::IPlugViewContentScaleSupport::ScaleFactor) nativeScaleFactor);
        }
    }

    void removeScaleFactorListener()
    {
        if (currentPeer == nullptr)
            return;

         for (int i = 0; i < ComponentPeer::getNumPeers(); ++i)
             if (ComponentPeer::getPeer (i) == currentPeer)
                 currentPeer->removeScaleFactorListener (this);
    }

    //==============================================================================
    Atomic<int> refCount { 1 };
    VSTComSmartPtr<IPlugView> view;

   #if JUCE_WINDOWS
    struct ChildComponent  : public Component
    {
        ChildComponent() {}
        void paint (Graphics& g) override  { g.fillAll (Colours::cornflowerblue); }
        using Component::createNewPeer;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChildComponent)
    };

    void setPluginWindowPos (ViewRect rect)
    {
        if (auto* topComp = getTopLevelComponent())
        {
            auto pos = (topComp->getLocalPoint (this, Point<int>()) * nativeScaleFactor).roundToInt();

            ScopedThreadDPIAwarenessSetter threadDpiAwarenessSetter { pluginHandle };

            SetWindowPos (pluginHandle, 0,
                          pos.x, pos.y,
                          rect.getWidth(), rect.getHeight(),
                          isVisible() ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
        }
    }

    ChildComponent embeddedComponent;
    std::unique_ptr<ComponentPeer> peer;
    using HandleFormat = HWND;
   #elif JUCE_MAC
    AutoResizingNSViewComponentWithParent embeddedComponent;
    using HandleFormat = NSView*;
   #elif JUCE_LINUX || JUCE_BSD
    SharedResourcePointer<RunLoop> runLoop;
    XEmbedComponent embeddedComponent { true, false };
    using HandleFormat = Window;
   #else
    Component embeddedComponent;
    using HandleFormat = void*;
   #endif

    HandleFormat pluginHandle = {};
    bool recursiveResize = false, hasDoneInitialResize = false;

    ComponentPeer* currentPeer = nullptr;
    Steinberg::IPlugViewContentScaleSupport* scaleInterface = nullptr;
    float nativeScaleFactor = 1.0f;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginWindow)
};

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4996) // warning about overriding deprecated methods

//==============================================================================
struct VST3ComponentHolder
{
    VST3ComponentHolder (const VST3ModuleHandle::Ptr& m)  : module (m)
    {
        host = new VST3HostContext();
    }

    ~VST3ComponentHolder()
    {
        terminate();
    }

    // transfers ownership to the plugin instance!
    AudioPluginInstance* createPluginInstance();

    bool fetchController (VSTComSmartPtr<Vst::IEditController>& editController)
    {
        if (! isComponentInitialised && ! initialise())
            return false;

        // Get the IEditController:
        TUID controllerCID = { 0 };

        if (component->getControllerClassId (controllerCID) == kResultTrue && FUID (controllerCID).isValid())
            editController.loadFrom (factory, controllerCID);

        if (editController == nullptr)
        {
            // Try finding the IEditController the long way around:
            auto numClasses = factory->countClasses();

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

        return (editController != nullptr);
    }

    //==============================================================================
    void fillInPluginDescription (PluginDescription& description) const
    {
        jassert (module != nullptr && isComponentInitialised);

        PFactoryInfo factoryInfo;
        factory->getFactoryInfo (&factoryInfo);

        auto classIdx = getClassIndex (module->getName());

        if (classIdx >= 0)
        {
            PClassInfo info;
            bool success = (factory->getClassInfo (classIdx, &info) == kResultOk);
            ignoreUnused (success);
            jassert (success);

            VSTComSmartPtr<IPluginFactory2> pf2;
            VSTComSmartPtr<IPluginFactory3> pf3;

            std::unique_ptr<PClassInfo2> info2;
            std::unique_ptr<PClassInfoW> infoW;

            if (pf2.loadFrom (factory))
            {
                info2.reset (new PClassInfo2());
                pf2->getClassInfo2 (classIdx, info2.get());
            }
            else
            {
                info2.reset();
            }

            if (pf3.loadFrom (factory))
            {
                pf3->setHostContext (host->getFUnknown());
                infoW.reset (new PClassInfoW());
                pf3->getClassInfoUnicode (classIdx, infoW.get());
            }
            else
            {
                infoW.reset();
            }

            Vst::BusInfo bus;
            int totalNumInputChannels = 0, totalNumOutputChannels = 0;

            int n = component->getBusCount(Vst::kAudio, Vst::kInput);
            for (int i = 0; i < n; ++i)
                if (component->getBusInfo (Vst::kAudio, Vst::kInput, i, bus) == kResultOk)
                    totalNumInputChannels += ((bus.flags & Vst::BusInfo::kDefaultActive) != 0 ? bus.channelCount : 0);

            n = component->getBusCount(Vst::kAudio, Vst::kOutput);
            for (int i = 0; i < n; ++i)
                if (component->getBusInfo (Vst::kAudio, Vst::kOutput, i, bus) == kResultOk)
                    totalNumOutputChannels += ((bus.flags & Vst::BusInfo::kDefaultActive) != 0 ? bus.channelCount : 0);

            createPluginDescription (description, module->getFile(),
                                     factoryInfo.vendor, module->getName(),
                                     info, info2.get(), infoW.get(),
                                     totalNumInputChannels,
                                     totalNumOutputChannels);

            return;
        }

        jassertfalse;
    }

    //==============================================================================
    bool initialise()
    {
        if (isComponentInitialised)
            return true;

        // It's highly advisable to create your plugins using the message thread.
        // The VST3 spec requires that many of the functions called during
        // initialisation are only called from the message thread.
        JUCE_ASSERT_MESSAGE_THREAD

        factory = VSTComSmartPtr<IPluginFactory> (module->getPluginFactory());

        int classIdx;
        if ((classIdx = getClassIndex (module->getName())) < 0)
            return false;

        PClassInfo info;
        if (factory->getClassInfo (classIdx, &info) != kResultOk)
            return false;

        if (! component.loadFrom (factory, info.cid) || component == nullptr)
            return false;

        cidOfComponent = FUID (info.cid);

        if (warnOnFailure (component->initialize (host->getFUnknown())) != kResultOk)
            return false;

        isComponentInitialised = true;

        return true;
    }

    void terminate()
    {
        if (isComponentInitialised)
        {
            component->terminate();
            isComponentInitialised = false;
        }

        component = nullptr;
    }

    //==============================================================================
    int getClassIndex (const String& className) const
    {
        PClassInfo info;
        const Steinberg::int32 numClasses = factory->countClasses();

        for (Steinberg::int32 j = 0; j < numClasses; ++j)
            if (factory->getClassInfo (j, &info) == kResultOk
                 && std::strcmp (info.category, kVstAudioEffectClass) == 0
                 && toString (info.name).trim() == className)
                return j;

        return -1;
    }

    //==============================================================================
    VST3ModuleHandle::Ptr module;
    VSTComSmartPtr<IPluginFactory> factory;
    VSTComSmartPtr<VST3HostContext> host;
    VSTComSmartPtr<Vst::IComponent> component;
    FUID cidOfComponent;

    bool isComponentInitialised = false;
};

//==============================================================================
/*  A queue which can store up to one element.

    This is more memory-efficient than storing large vectors of
    parameter changes that we'll just throw away.
*/
class ParamValueQueue : public Vst::IParamValueQueue
{
public:
    ParamValueQueue (Vst::ParamID idIn, size_t parameterIndexIn)
        : paramId (idIn), parameterIndex (parameterIndexIn) {}

    virtual ~ParamValueQueue() = default;

    JUCE_DECLARE_VST3_COM_REF_METHODS
    JUCE_DECLARE_VST3_COM_QUERY_METHODS

    Vst::ParamID PLUGIN_API getParameterId() override { return paramId; }

    size_t getParameterIndex() const noexcept { return parameterIndex; }

    Steinberg::int32 PLUGIN_API getPointCount() override { return size; }

    tresult PLUGIN_API getPoint (Steinberg::int32 index,
                                 Steinberg::int32& sampleOffset,
                                 Vst::ParamValue& value) override
    {
        if (! isPositiveAndBelow (index, size))
            return kResultFalse;

        sampleOffset = 0;
        value = cachedValue;

        return kResultTrue;
    }

    tresult PLUGIN_API addPoint (Steinberg::int32,
                                 Vst::ParamValue value,
                                 Steinberg::int32& index) override
    {
        index = size++;
        set ((float) value);

        return kResultTrue;
    }

    void set (float valueIn)
    {
        cachedValue = valueIn;
        size = 1;
    }

    void clear() { size = 0; }

    float get() const noexcept
    {
        jassert (size > 0);
        return cachedValue;
    }

private:
    const Vst::ParamID paramId;
    const size_t parameterIndex;
    float cachedValue;
    Steinberg::int32 size = 0;
    Atomic<int> refCount;
};

//==============================================================================
/*  An implementation of IParameterChanges with some important characteristics:
    - Lookup by index is O(1)
    - Lookup by paramID is also O(1)
    - addParameterData never allocates, as long you pass a paramID already passed to initialise
*/
class ParameterChanges : public Vst::IParameterChanges
{
    static constexpr Steinberg::int32 notInVector = -1;

    struct Entry
    {
        explicit Entry (std::unique_ptr<ParamValueQueue> queue) : ptr (queue.release()) {}

        VSTComSmartPtr<ParamValueQueue> ptr;
        Steinberg::int32 index = notInVector;
    };

    using Map = std::unordered_map<Vst::ParamID, Entry>;
    using Queues = std::vector<Entry*>;

public:
    virtual ~ParameterChanges() = default;

    JUCE_DECLARE_VST3_COM_REF_METHODS
    JUCE_DECLARE_VST3_COM_QUERY_METHODS

    Steinberg::int32 PLUGIN_API getParameterCount() override
    {
        return (Steinberg::int32) queues.size();
    }

    ParamValueQueue* PLUGIN_API getParameterData (Steinberg::int32 index) override
    {
        if (isPositiveAndBelow (index, queues.size()))
        {
            auto& entry = queues[(size_t) index];
            // If this fails, our container has become internally inconsistent
            jassert (entry->index == index);
            return entry->ptr.get();
        }

        return nullptr;
    }

    ParamValueQueue* PLUGIN_API addParameterData (const Vst::ParamID& id,
                                                  Steinberg::int32& index) override
    {
        const auto it = map.find (id);

        if (it == map.end())
            return nullptr;

        auto& result = it->second;

        if (result.index == notInVector)
        {
            result.index = (Steinberg::int32) queues.size();
            queues.push_back (&result);
        }

        index = result.index;
        return result.ptr.get();
    }

    void set (Vst::ParamID id, float value)
    {
        Steinberg::int32 indexOut = notInVector;

        if (auto* queue = addParameterData (id, indexOut))
            queue->set (value);
    }

    void clear()
    {
        for (auto* item : queues)
            item->index = notInVector;

        queues.clear();
    }

    void initialise (const std::vector<Vst::ParamID>& idsIn)
    {
        size_t index = 0;

        for (const auto& id : idsIn)
            map.emplace (id, Entry { std::make_unique<ParamValueQueue> (id, index++) });

        queues.reserve (map.size());
        queues.clear();
    }

    template <typename Callback>
    void forEach (Callback&& callback) const
    {
        for (const auto* item : queues)
        {
            auto* ptr = item->ptr.get();
            callback (ptr->getParameterIndex(), ptr->get());
        }
    }

private:
    Map map;
    Queues queues;
    Atomic<int> refCount;
};

//==============================================================================
class VST3PluginInstance : public AudioPluginInstance
{
public:
    //==============================================================================
    struct VST3Parameter final  : public Parameter
    {
        VST3Parameter (VST3PluginInstance& parent,
                       int vstParameterIndex,
                       Steinberg::Vst::ParamID parameterID,
                       bool parameterIsAutomatable)
            : pluginInstance (parent),
              vstParamIndex (vstParameterIndex),
              paramID (parameterID),
              automatable (parameterIsAutomatable)
        {
        }

        float getValue() const override
        {
            return pluginInstance.cachedParamValues.get ((size_t) vstParamIndex);
        }

        /*  The 'normal' setValue call, which will update both the processor and editor.
        */
        void setValue (float newValue) override
        {
            pluginInstance.cachedParamValues.set ((size_t) vstParamIndex, newValue);
            pluginInstance.parameterDispatcher.push ((size_t) vstParamIndex, newValue);
        }

        /*  If the editor set the value, there's no need to notify it that the parameter
            value changed. Instead, we set the cachedValue (which will be read by the
            processor during the next processBlock) and notify listeners that the parameter
            has changed.
        */
        void setValueFromEditor (float newValue)
        {
            pluginInstance.cachedParamValues.set ((size_t) vstParamIndex, newValue);
            sendValueChangedMessageToListeners (newValue);
        }

        String getText (float value, int maximumLength) const override
        {
            MessageManagerLock lock;

            if (pluginInstance.editController != nullptr)
            {
                Vst::String128 result;

                if (pluginInstance.editController->getParamStringByValue (paramID, value, result) == kResultOk)
                    return toString (result).substring (0, maximumLength);
            }

            return Parameter::getText (value, maximumLength);
        }

        float getValueForText (const String& text) const override
        {
            MessageManagerLock lock;

            if (pluginInstance.editController != nullptr)
            {
                Vst::ParamValue result;

                if (pluginInstance.editController->getParamValueByString (paramID, toString (text), result) == kResultOk)
                    return (float) result;
            }

            return Parameter::getValueForText (text);
        }

        float getDefaultValue() const override
        {
            return (float) getParameterInfo().defaultNormalizedValue;
        }

        String getName (int /*maximumStringLength*/) const override
        {
            return toString (getParameterInfo().title);
        }

        String getLabel() const override
        {
            return toString (getParameterInfo().units);
        }

        bool isAutomatable() const override
        {
            return automatable;
        }

        bool isDiscrete() const override
        {
            return discrete;
        }

        int getNumSteps() const override
        {
            return numSteps;
        }

        StringArray getAllValueStrings() const override
        {
            return {};
        }

        Steinberg::Vst::ParamID getParamID() const noexcept { return paramID; }

    private:
        Vst::ParameterInfo getParameterInfo() const
        {
            return pluginInstance.getParameterInfoForIndex (vstParamIndex);
        }

        VST3PluginInstance& pluginInstance;
        const int vstParamIndex;
        const Steinberg::Vst::ParamID paramID;
        const bool automatable;
        const bool discrete = getNumSteps() != AudioProcessor::getDefaultNumParameterSteps();
        const int numSteps = [&]
        {
            auto stepCount = getParameterInfo().stepCount;
            return stepCount == 0 ? AudioProcessor::getDefaultNumParameterSteps()
                                  : stepCount + 1;
        }();
    };

    //==============================================================================
    VST3PluginInstance (VST3ComponentHolder* componentHolder)
        : AudioPluginInstance (getBusProperties (componentHolder->component)),
          holder (componentHolder),
          midiInputs (new MidiEventList()),
          midiOutputs (new MidiEventList())
    {
        holder->host->setPlugin (this);
    }

    ~VST3PluginInstance() override
    {
        struct VST3Deleter : public CallbackMessage
        {
            VST3Deleter (VST3PluginInstance& inInstance, WaitableEvent& inEvent)
                : vst3Instance (inInstance), completionSignal (inEvent)
            {}

            void messageCallback() override
            {
                vst3Instance.cleanup();
                completionSignal.signal();
            }

            VST3PluginInstance& vst3Instance;
            WaitableEvent& completionSignal;
        };

        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            cleanup();
        }
        else
        {
            WaitableEvent completionEvent;
            (new VST3Deleter (*this, completionEvent))->post();
            completionEvent.wait();
        }
    }

    void cleanup()
    {
        jassert (getActiveEditor() == nullptr); // You must delete any editors before deleting the plugin instance!

        releaseResources();

        if (editControllerConnection != nullptr && componentConnection != nullptr)
        {
            editControllerConnection->disconnect (componentConnection);
            componentConnection->disconnect (editControllerConnection);
        }

        editController->setComponentHandler (nullptr);

        if (isControllerInitialised)
            editController->terminate();

        holder->terminate();

        componentConnection = nullptr;
        editControllerConnection = nullptr;
        unitData = nullptr;
        unitInfo = nullptr;
        programListData = nullptr;
        componentHandler2 = nullptr;
        componentHandler = nullptr;
        processor = nullptr;
        midiMapping = nullptr;
        editController2 = nullptr;
        editController = nullptr;
    }

    //==============================================================================
    bool initialise()
    {
        // It's highly advisable to create your plugins using the message thread.
        // The VST3 spec requires that many of the functions called during
        // initialisation are only called from the message thread.
        JUCE_ASSERT_MESSAGE_THREAD

        if (! holder->initialise())
            return false;

        if (! (isControllerInitialised || holder->fetchController (editController)))
            return false;

        // (May return an error if the plugin combines the IComponent and IEditController implementations)
        editController->initialize (holder->host->getFUnknown());

        isControllerInitialised = true;
        editController->setComponentHandler (holder->host);
        grabInformationObjects();
        interconnectComponentAndController();

        auto configureParameters = [this]
        {
            refreshParameterList();
            synchroniseStates();
            syncProgramNames();
        };

        configureParameters();
        setupIO();

        // Some plug-ins don't present their parameters until after the IO has been
        // configured, so we need to jump though all these hoops again
        if (getParameters().isEmpty() && editController->getParameterCount() > 0)
            configureParameters();

        parameterDispatcher.start (*editController);

        return true;
    }

    void* getPlatformSpecificData() override   { return holder->component; }

    //==============================================================================
    const String getName() const override
    {
        auto& module = holder->module;
        return module != nullptr ? module->getName() : String();
    }

    void repopulateArrangements (Array<Vst::SpeakerArrangement>& inputArrangements, Array<Vst::SpeakerArrangement>& outputArrangements) const
    {
        inputArrangements.clearQuick();
        outputArrangements.clearQuick();

        auto numInputAudioBuses  = getBusCount (true);
        auto numOutputAudioBuses = getBusCount (false);

        for (int i = 0; i < numInputAudioBuses; ++i)
            inputArrangements.add (getArrangementForBus (processor, true, i));

        for (int i = 0; i < numOutputAudioBuses; ++i)
            outputArrangements.add (getArrangementForBus (processor, false, i));
    }

    void processorLayoutsToArrangements (Array<Vst::SpeakerArrangement>& inputArrangements, Array<Vst::SpeakerArrangement>& outputArrangements)
    {
        inputArrangements.clearQuick();
        outputArrangements.clearQuick();

        auto numInputBuses  = getBusCount (true);
        auto numOutputBuses = getBusCount (false);

        for (int i = 0; i < numInputBuses; ++i)
            inputArrangements.add (getVst3SpeakerArrangement (getBus (true, i)->getLastEnabledLayout()));

        for (int i = 0; i < numOutputBuses; ++i)
            outputArrangements.add (getVst3SpeakerArrangement (getBus (false, i)->getLastEnabledLayout()));
    }

    void prepareToPlay (double newSampleRate, int estimatedSamplesPerBlock) override
    {
        // The VST3 spec requires that IComponent::setupProcessing() is called on the message
        // thread. If you call it from a different thread, some plugins may break.
        JUCE_ASSERT_MESSAGE_THREAD
        MessageManagerLock lock;

        // Avoid redundantly calling things like setActive, which can be a heavy-duty call for some plugins:
        if (isActive
              && getSampleRate() == newSampleRate
              && getBlockSize() == estimatedSamplesPerBlock)
            return;

        using namespace Vst;

        ProcessSetup setup;
        setup.symbolicSampleSize    = isUsingDoublePrecision() ? kSample64 : kSample32;
        setup.maxSamplesPerBlock    = estimatedSamplesPerBlock;
        setup.sampleRate            = newSampleRate;
        setup.processMode           = isNonRealtime() ? kOffline : kRealtime;

        warnOnFailure (processor->setupProcessing (setup));

        holder->initialise();

        Array<Vst::SpeakerArrangement> inputArrangements, outputArrangements;
        processorLayoutsToArrangements (inputArrangements, outputArrangements);

        // Some plug-ins will crash if you pass a nullptr to setBusArrangements!
        SpeakerArrangement nullArrangement = {};
        auto* inputArrangementData  = inputArrangements.isEmpty()  ? &nullArrangement : inputArrangements.getRawDataPointer();
        auto* outputArrangementData = outputArrangements.isEmpty() ? &nullArrangement : outputArrangements.getRawDataPointer();

        warnOnFailure (processor->setBusArrangements (inputArrangementData,  inputArrangements.size(),
                                                      outputArrangementData, outputArrangements.size()));

        Array<Vst::SpeakerArrangement> actualInArr, actualOutArr;
        repopulateArrangements (actualInArr, actualOutArr);

        jassert (actualInArr == inputArrangements && actualOutArr == outputArrangements);

        // Needed for having the same sample rate in processBlock(); some plugins need this!
        setRateAndBufferSizeDetails (newSampleRate, estimatedSamplesPerBlock);

        auto numInputBuses  = getBusCount (true);
        auto numOutputBuses = getBusCount (false);

        for (int i = 0; i < numInputBuses; ++i)
            warnOnFailure (holder->component->activateBus (Vst::kAudio, Vst::kInput,  i, getBus (true,  i)->isEnabled() ? 1 : 0));

        for (int i = 0; i < numOutputBuses; ++i)
            warnOnFailure (holder->component->activateBus (Vst::kAudio, Vst::kOutput, i, getBus (false, i)->isEnabled() ? 1 : 0));

        setLatencySamples (jmax (0, (int) processor->getLatencySamples()));
        cachedBusLayouts = getBusesLayout();

        setStateForAllMidiBuses (true);

        warnOnFailure (holder->component->setActive (true));
        warnOnFailureIfImplemented (processor->setProcessing (true));

        isActive = true;
    }

    void releaseResources() override
    {
        if (! isActive)
            return; // Avoids redundantly calling things like setActive

        isActive = false;

        setStateForAllMidiBuses (false);

        if (processor != nullptr)
            warnOnFailureIfImplemented (processor->setProcessing (false));

        if (holder->component != nullptr)
            warnOnFailure (holder->component->setActive (false));
    }

    bool supportsDoublePrecisionProcessing() const override
    {
        return (processor->canProcessSampleSize (Vst::kSample64) == kResultTrue);
    }

    //==============================================================================
    VST3Parameter* getParameterForID (Vst::ParamID paramID)
    {
        const auto index = getIndexOfParamID (paramID);
        return index < 0 ? nullptr : static_cast<VST3Parameter*> (getParameters()[index]);
    }

    //==============================================================================
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (! isUsingDoublePrecision());

        if (isActive && processor != nullptr)
            processAudio (buffer, midiMessages, Vst::kSample32, false);
    }

    void processBlock (AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (isUsingDoublePrecision());

        if (isActive && processor != nullptr)
            processAudio (buffer, midiMessages, Vst::kSample64, false);
    }

    void processBlockBypassed (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (! isUsingDoublePrecision());

        if (bypassParam != nullptr)
        {
            if (isActive && processor != nullptr)
                processAudio (buffer, midiMessages, Vst::kSample32, true);
        }
        else
        {
            AudioProcessor::processBlockBypassed (buffer, midiMessages);
        }
    }

    void processBlockBypassed (AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (isUsingDoublePrecision());

        if (bypassParam != nullptr)
        {
            if (isActive && processor != nullptr)
                processAudio (buffer, midiMessages, Vst::kSample64, true);
        }
        else
        {
            AudioProcessor::processBlockBypassed (buffer, midiMessages);
        }
    }

    //==============================================================================
    template <typename FloatType>
    void processAudio (AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages,
                       Vst::SymbolicSampleSizes sampleSize, bool isProcessBlockBypassedCall)
    {
        using namespace Vst;
        auto numSamples = buffer.getNumSamples();

        auto numInputAudioBuses  = getBusCount (true);
        auto numOutputAudioBuses = getBusCount (false);

        updateBypass (isProcessBlockBypassedCall);

        ProcessData data;
        data.processMode            = isNonRealtime() ? kOffline : kRealtime;
        data.symbolicSampleSize     = sampleSize;
        data.numInputs              = numInputAudioBuses;
        data.numOutputs             = numOutputAudioBuses;
        data.inputParameterChanges  = inputParameterChanges;
        data.outputParameterChanges = outputParameterChanges;
        data.numSamples             = (Steinberg::int32) numSamples;

        updateTimingInformation (data, getSampleRate());

        for (int i = getTotalNumInputChannels(); i < buffer.getNumChannels(); ++i)
            buffer.clear (i, 0, numSamples);

        associateWith (data, buffer);
        associateWith (data, midiMessages);

        inputParameterChanges ->clear();
        outputParameterChanges->clear();

        cachedParamValues.ifSet ([&] (size_t index, float value)
        {
            inputParameterChanges->set (cachedParamValues.getParamID (index), value);
        });

        processor->process (data);

        outputParameterChanges->forEach ([&] (size_t index, float value)
        {
            parameterDispatcher.push (index, value);
        });

        midiMessages.clear();
        MidiEventList::toMidiBuffer (midiMessages, *midiOutputs);
    }

    //==============================================================================
    bool canAddBus (bool) const override                                       { return false; }
    bool canRemoveBus (bool) const override                                    { return false; }

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        // if the processor is not active, we ask the underlying plug-in if the
        // layout is actually supported
        if (! isActive)
            return canApplyBusesLayout (layouts);

        // not much we can do to check the layout while the audio processor is running
        // Let's at least check if it is a VST3 compatible layout
        for (int dir = 0; dir < 2; ++dir)
        {
            bool isInput = (dir == 0);
            auto n = getBusCount (isInput);

            for (int i = 0; i < n; ++i)
                if (getChannelLayoutOfBus (isInput, i).isDiscreteLayout())
                    return false;
        }

        return true;
    }

    bool syncBusLayouts (const BusesLayout& layouts) const
    {
        for (int dir = 0; dir < 2; ++dir)
        {
            bool isInput = (dir == 0);
            auto n = getBusCount (isInput);
            const Vst::BusDirection vstDir = (isInput ? Vst::kInput : Vst::kOutput);

            for (int busIdx = 0; busIdx < n; ++busIdx)
            {
                const bool isEnabled = (! layouts.getChannelSet (isInput, busIdx).isDisabled());

                if (holder->component->activateBus (Vst::kAudio, vstDir, busIdx, (isEnabled ? 1 : 0)) != kResultOk)
                    return false;
            }
        }

        Array<Vst::SpeakerArrangement> inputArrangements, outputArrangements;

        for (int i = 0; i < layouts.inputBuses.size(); ++i)
        {
            const auto& requested = layouts.getChannelSet (true, i);
            inputArrangements.add (getVst3SpeakerArrangement (requested.isDisabled() ? getBus (true, i)->getLastEnabledLayout() : requested));
        }

        for (int i = 0; i < layouts.outputBuses.size(); ++i)
        {
            const auto& requested = layouts.getChannelSet (false, i);
            outputArrangements.add (getVst3SpeakerArrangement (requested.isDisabled() ? getBus (false, i)->getLastEnabledLayout() : requested));
        }

        // Some plug-ins will crash if you pass a nullptr to setBusArrangements!
        Vst::SpeakerArrangement nullArrangement = {};
        auto* inputArrangementData  = inputArrangements.isEmpty()  ? &nullArrangement : inputArrangements.getRawDataPointer();
        auto* outputArrangementData = outputArrangements.isEmpty() ? &nullArrangement : outputArrangements.getRawDataPointer();

        if (processor->setBusArrangements (inputArrangementData, inputArrangements.size(),
                                           outputArrangementData, outputArrangements.size()) != kResultTrue)
            return false;

        // check if the layout matches the request
        Array<Vst::SpeakerArrangement> actualIn, actualOut;
        repopulateArrangements (actualIn, actualOut);

        return (actualIn == inputArrangements && actualOut == outputArrangements);
    }

    bool canApplyBusesLayout (const BusesLayout& layouts) const override
    {
        // someone tried to change the layout while the AudioProcessor is running
        // call releaseResources first!
        jassert (! isActive);

        bool result = syncBusLayouts (layouts);

        // didn't succeed? Make sure it's back in it's original state
        if (! result)
            syncBusLayouts (getBusesLayout());

        return result;
    }

    //==============================================================================
    void updateTrackProperties (const TrackProperties& properties) override
    {
        if (trackInfoListener != nullptr)
        {
            VSTComSmartPtr<Vst::IAttributeList> l (new TrackPropertiesAttributeList (properties));
            trackInfoListener->setChannelContextInfos (l);
        }
    }

    struct TrackPropertiesAttributeList    : public Vst::IAttributeList
    {
        TrackPropertiesAttributeList (const TrackProperties& properties) : props (properties) {}
        virtual ~TrackPropertiesAttributeList() {}

        JUCE_DECLARE_VST3_COM_REF_METHODS

        tresult PLUGIN_API queryInterface (const TUID queryIid, void** obj) override
        {
            TEST_FOR_AND_RETURN_IF_VALID (queryIid, Vst::IAttributeList)
            TEST_FOR_COMMON_BASE_AND_RETURN_IF_VALID (queryIid, FUnknown, Vst::IAttributeList)

            *obj = nullptr;
            return kNotImplemented;
        }

        tresult PLUGIN_API setInt    (AttrID, Steinberg::int64) override                 { return kOutOfMemory; }
        tresult PLUGIN_API setFloat  (AttrID, double) override                           { return kOutOfMemory; }
        tresult PLUGIN_API setString (AttrID, const Vst::TChar*) override                { return kOutOfMemory; }
        tresult PLUGIN_API setBinary (AttrID, const void*, Steinberg::uint32) override   { return kOutOfMemory; }
        tresult PLUGIN_API getFloat  (AttrID, double&) override                          { return kResultFalse; }
        tresult PLUGIN_API getBinary (AttrID, const void*&, Steinberg::uint32&) override { return kResultFalse; }

        tresult PLUGIN_API getString (AttrID id, Vst::TChar* string, Steinberg::uint32 size) override
        {
            if (! std::strcmp (id, Vst::ChannelContext::kChannelNameKey))
            {
                Steinberg::String str (props.name.toRawUTF8());
                str.copyTo (string, 0, (Steinberg::int32) jmin (size, (Steinberg::uint32) std::numeric_limits<Steinberg::int32>::max()));

                return kResultTrue;
            }

            return kResultFalse;
        }

        tresult PLUGIN_API getInt (AttrID id, Steinberg::int64& value) override
        {
            if      (! std::strcmp (Vst::ChannelContext::kChannelNameLengthKey, id)) value = props.name.length();
            else if (! std::strcmp (Vst::ChannelContext::kChannelColorKey,      id)) value = static_cast<Steinberg::int64> (props.colour.getARGB());
            else return kResultFalse;

            return kResultTrue;
        }

        Atomic<int> refCount;
        TrackProperties props;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackPropertiesAttributeList)
    };

    //==============================================================================
    String getChannelName (int channelIndex, bool forInput, bool forAudioChannel) const
    {
        auto numBuses = getNumSingleDirectionBusesFor (holder->component, forInput, forAudioChannel);
        int numCountedChannels = 0;

        for (int i = 0; i < numBuses; ++i)
        {
            auto busInfo = getBusInfo (forInput, forAudioChannel, i);

            numCountedChannels += busInfo.channelCount;

            if (channelIndex < numCountedChannels)
                return toString (busInfo.name);
        }

        return {};
    }

    const String getInputChannelName  (int channelIndex) const override   { return getChannelName (channelIndex, true, true); }
    const String getOutputChannelName (int channelIndex) const override   { return getChannelName (channelIndex, false, true); }

    bool isInputChannelStereoPair (int channelIndex) const override
    {
        int busIdx;
        return getOffsetInBusBufferForAbsoluteChannelIndex (true, channelIndex, busIdx) >= 0
                 && getBusInfo (true, true, busIdx).channelCount == 2;
    }

    bool isOutputChannelStereoPair (int channelIndex) const override
    {
        int busIdx;
        return getOffsetInBusBufferForAbsoluteChannelIndex (false, channelIndex, busIdx) >= 0
                 && getBusInfo (false, true, busIdx).channelCount == 2;
    }

    bool acceptsMidi() const override    { return getNumSingleDirectionBusesFor (holder->component, true,  false) > 0; }
    bool producesMidi() const override   { return getNumSingleDirectionBusesFor (holder->component, false, false) > 0; }

    //==============================================================================
    AudioProcessorParameter* getBypassParameter() const override         { return bypassParam; }

    //==============================================================================
    /** May return a negative value as a means of informing us that the plugin has "infinite tail," or 0 for "no tail." */
    double getTailLengthSeconds() const override
    {
        if (processor != nullptr)
        {
            auto sampleRate = getSampleRate();

            if (sampleRate > 0.0)
            {
                auto tailSamples = processor->getTailSamples();

                if (tailSamples == Vst::kInfiniteTail)
                    return std::numeric_limits<double>::infinity();

                return jlimit (0, 0x7fffffff, (int) processor->getTailSamples()) / sampleRate;
            }
        }

        return 0.0;
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override
    {
        if (auto* view = tryCreatingView())
            return new VST3PluginWindow (this, view);

        return nullptr;
    }

    bool hasEditor() const override
    {
        // (if possible, avoid creating a second instance of the editor, because that crashes some plugins)
        if (getActiveEditor() != nullptr)
            return true;

        VSTComSmartPtr<IPlugView> view (tryCreatingView(), false);
        return view != nullptr;
    }

    //==============================================================================
    int getNumPrograms() override                        { return programNames.size(); }
    const String getProgramName (int index) override     { return index >= 0 ? programNames[index] : String(); }
    void changeProgramName (int, const String&) override {}

    int getCurrentProgram() override
    {
        if (programNames.size() > 0 && editController != nullptr)
            if (auto* param = getParameterForID (programParameterID))
                return jmax (0, roundToInt (param->getValue() * (float) (programNames.size() - 1)));

        return 0;
    }

    void setCurrentProgram (int program) override
    {
        if (programNames.size() > 0 && editController != nullptr)
        {
            auto value = static_cast<Vst::ParamValue> (program) / static_cast<Vst::ParamValue> (jmax (1, programNames.size() - 1));

            if (auto* param = getParameterForID (programParameterID))
                param->setValue ((float) value);
        }
    }

    //==============================================================================
    void reset() override
    {
        if (holder->component != nullptr && processor != nullptr)
        {
            processor->setProcessing (false);
            holder->component->setActive (false);

            holder->component->setActive (true);
            processor->setProcessing (true);
        }
    }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        // The VST3 plugin format requires that get/set state calls are made
        // from the message thread.
        // We'll lock the message manager here as a safety precaution, but some
        // plugins may still misbehave!

        JUCE_ASSERT_MESSAGE_THREAD
        MessageManagerLock lock;

        parameterDispatcher.flush();

        XmlElement state ("VST3PluginState");

        appendStateFrom (state, holder->component, "IComponent");
        appendStateFrom (state, editController, "IEditController");

        AudioProcessor::copyXmlToBinary (state, destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        // The VST3 plugin format requires that get/set state calls are made
        // from the message thread.
        // We'll lock the message manager here as a safety precaution, but some
        // plugins may still misbehave!

        JUCE_ASSERT_MESSAGE_THREAD
        MessageManagerLock lock;

        parameterDispatcher.flush();

        if (auto head = AudioProcessor::getXmlFromBinary (data, sizeInBytes))
        {
            auto componentStream (createMemoryStreamForState (*head, "IComponent"));

            if (componentStream != nullptr && holder->component != nullptr)
                holder->component->setState (componentStream);

            if (editController != nullptr)
            {
                if (componentStream != nullptr)
                {
                    int64 result;
                    componentStream->seek (0, IBStream::kIBSeekSet, &result);
                    setComponentStateAndResetParameters (*componentStream);
                }

                auto controllerStream (createMemoryStreamForState (*head, "IEditController"));

                if (controllerStream != nullptr)
                    editController->setState (controllerStream);
            }
        }
    }

    void setComponentStateAndResetParameters (Steinberg::MemoryStream& stream)
    {
        jassert (editController != nullptr);

        warnOnFailureIfImplemented (editController->setComponentState (&stream));

        for (auto* parameter : getParameters())
        {
            auto* vst3Param = static_cast<VST3Parameter*> (parameter);
            vst3Param->setValueFromEditor ((float) editController->getParamNormalized (vst3Param->getParamID()));
        }
    }

    bool setStateFromPresetFile (const MemoryBlock& rawData)
    {
        MemoryBlock rawDataCopy (rawData);
        VSTComSmartPtr<Steinberg::MemoryStream> memoryStream = new Steinberg::MemoryStream (rawDataCopy.getData(), (int) rawDataCopy.getSize());

        if (memoryStream == nullptr || holder->component == nullptr)
            return false;

        return Steinberg::Vst::PresetFile::loadPreset (memoryStream, holder->cidOfComponent,
                                                       holder->component, editController, nullptr);
    }

    //==============================================================================
    void fillInPluginDescription (PluginDescription& description) const override
    {
        holder->fillInPluginDescription (description);
    }

    /** @note Not applicable to VST3 */
    void getCurrentProgramStateInformation (MemoryBlock& destData) override
    {
        destData.setSize (0, true);
    }

    /** @note Not applicable to VST3 */
    void setCurrentProgramStateInformation (const void* data, int sizeInBytes) override
    {
        ignoreUnused (data, sizeInBytes);
    }


private:
    //==============================================================================
   #if JUCE_LINUX || JUCE_BSD
    SharedResourcePointer<RunLoop> runLoop;
   #endif

    std::unique_ptr<VST3ComponentHolder> holder;

    friend VST3HostContext;

    // Information objects:
    String company;
    std::unique_ptr<PClassInfo> info;
    std::unique_ptr<PClassInfo2> info2;
    std::unique_ptr<PClassInfoW> infoW;

    // Rudimentary interfaces:
    VSTComSmartPtr<Vst::IEditController> editController;
    VSTComSmartPtr<Vst::IEditController2> editController2;
    VSTComSmartPtr<Vst::IMidiMapping> midiMapping;
    VSTComSmartPtr<Vst::IAudioProcessor> processor;
    VSTComSmartPtr<Vst::IComponentHandler> componentHandler;
    VSTComSmartPtr<Vst::IComponentHandler2> componentHandler2;
    VSTComSmartPtr<Vst::IUnitInfo> unitInfo;
    VSTComSmartPtr<Vst::IUnitData> unitData;
    VSTComSmartPtr<Vst::IProgramListData> programListData;
    VSTComSmartPtr<Vst::IConnectionPoint> componentConnection, editControllerConnection;
    VSTComSmartPtr<Vst::ChannelContext::IInfoListener> trackInfoListener;

    /** The number of IO buses MUST match that of the plugin,
        even if there aren't enough channels to process,
        as very poorly specified by the Steinberg SDK
    */
    VST3FloatAndDoubleBusMapComposite inputBusMap, outputBusMap;
    Array<Vst::AudioBusBuffers> inputBuses, outputBuses;
    AudioProcessor::BusesLayout cachedBusLayouts;

    StringArray programNames;
    Vst::ParamID programParameterID = (Vst::ParamID) -1;

    std::map<Vst::ParamID, int> paramToIndexMap;

    int getMappedParamID (Vst::ParamID paramID) const
    {
        auto it = paramToIndexMap.find (paramID);
        return it != paramToIndexMap.end() ? it->second : -1;
    }

    int getIndexOfParamID (Vst::ParamID paramID)
    {
        if (editController == nullptr)
            return -1;

        auto result = getMappedParamID (paramID);

        if (result < 0)
        {
            auto numParams = editController->getParameterCount();

            for (int i = 0; i < numParams; ++i)
            {
                Vst::ParameterInfo paramInfo;
                editController->getParameterInfo (i, paramInfo);
                paramToIndexMap[paramInfo.id] = i;
            }

            result = getMappedParamID (paramID);
        }

        return result;
    }

    EditControllerParameterDispatcher parameterDispatcher;

    //==============================================================================
    template <typename Type>
    static void appendStateFrom (XmlElement& head, VSTComSmartPtr<Type>& object, const String& identifier)
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

    static VSTComSmartPtr<Steinberg::MemoryStream> createMemoryStreamForState (XmlElement& head, StringRef identifier)
    {
        if (auto* state = head.getChildByName (identifier))
        {
            MemoryBlock mem;

            if (mem.fromBase64Encoding (state->getAllSubText()))
            {
                VSTComSmartPtr<Steinberg::MemoryStream> stream (new Steinberg::MemoryStream(), false);
                stream->setSize ((TSize) mem.getSize());
                mem.copyTo (stream->getData(), 0, mem.getSize());
                return stream;
            }
        }

        return nullptr;
    }

    CachedParamValues cachedParamValues;
    VSTComSmartPtr<ParameterChanges> inputParameterChanges  { new ParameterChanges };
    VSTComSmartPtr<ParameterChanges> outputParameterChanges { new ParameterChanges };
    VSTComSmartPtr<MidiEventList> midiInputs, midiOutputs;
    Vst::ProcessContext timingInfo; //< Only use this in processBlock()!
    bool isControllerInitialised = false, isActive = false, lastProcessBlockCallWasBypass = false;
    VST3Parameter* bypassParam = nullptr;

    //==============================================================================
    /** Some plugins need to be "connected" to intercommunicate between their implemented classes */
    void interconnectComponentAndController()
    {
        componentConnection.loadFrom (holder->component);
        editControllerConnection.loadFrom (editController);

        if (componentConnection != nullptr && editControllerConnection != nullptr)
        {
            warnOnFailure (componentConnection->connect (editControllerConnection));
            warnOnFailure (editControllerConnection->connect (componentConnection));
        }
    }

    void refreshParameterList() override
    {
        AudioProcessorParameterGroup newParameterTree;

        // We're going to add parameter groups to the tree recursively in the same order as the
        // first parameters contained within them.
        std::map<Vst::UnitID, Vst::UnitInfo> infoMap;
        std::map<Vst::UnitID, AudioProcessorParameterGroup*> groupMap;
        groupMap[Vst::kRootUnitId] = &newParameterTree;

        if (unitInfo != nullptr)
        {
            const auto numUnits = unitInfo->getUnitCount();

            for (int i = 1; i < numUnits; ++i)
            {
                Vst::UnitInfo ui{};
                unitInfo->getUnitInfo (i, ui);
                infoMap[ui.id] = std::move (ui);
            }
        }

        {
            auto allIds = getAllParamIDs (*editController);
            inputParameterChanges ->initialise (allIds);
            outputParameterChanges->initialise (allIds);
            cachedParamValues = CachedParamValues { std::move (allIds) };
        }

        for (int i = 0; i < editController->getParameterCount(); ++i)
        {
            auto paramInfo = getParameterInfoForIndex (i);
            auto* param = new VST3Parameter (*this,
                                             i,
                                             paramInfo.id,
                                             (paramInfo.flags & Vst::ParameterInfo::kCanAutomate) != 0);

            if ((paramInfo.flags & Vst::ParameterInfo::kIsBypass) != 0)
                bypassParam = param;

            std::function<AudioProcessorParameterGroup*(Vst::UnitID)> findOrCreateGroup;
            findOrCreateGroup = [&groupMap, &infoMap, &findOrCreateGroup] (Vst::UnitID groupID)
            {
                auto existingGroup = groupMap.find (groupID);

                if (existingGroup != groupMap.end())
                    return existingGroup->second;

                auto groupInfo = infoMap.find (groupID);

                if (groupInfo == infoMap.end())
                    return groupMap[Vst::kRootUnitId];

                auto* group = new AudioProcessorParameterGroup (String (groupInfo->first),
                                                                toString (groupInfo->second.name),
                                                                {});
                groupMap[groupInfo->first] = group;

                auto* parentGroup = findOrCreateGroup (groupInfo->second.parentUnitId);
                parentGroup->addChild (std::unique_ptr<AudioProcessorParameterGroup> (group));

                return group;
            };

            auto* group = findOrCreateGroup (paramInfo.unitId);
            group->addChild (std::unique_ptr<AudioProcessorParameter> (param));
        }

        setParameterTree (std::move (newParameterTree));
    }

    void synchroniseStates()
    {
        Steinberg::MemoryStream stream;

        if (holder->component->getState (&stream) == kResultTrue)
            if (stream.seek (0, Steinberg::IBStream::kIBSeekSet, nullptr) == kResultTrue)
                setComponentStateAndResetParameters (stream);
    }

    void grabInformationObjects()
    {
        processor.loadFrom (holder->component);
        unitInfo.loadFrom (holder->component);
        programListData.loadFrom (holder->component);
        unitData.loadFrom (holder->component);
        editController2.loadFrom (holder->component);
        midiMapping.loadFrom (holder->component);
        componentHandler.loadFrom (holder->component);
        componentHandler2.loadFrom (holder->component);
        trackInfoListener.loadFrom (holder->component);

        if (processor == nullptr)           processor.loadFrom (editController);
        if (unitInfo == nullptr)            unitInfo.loadFrom (editController);
        if (programListData == nullptr)     programListData.loadFrom (editController);
        if (unitData == nullptr)            unitData.loadFrom (editController);
        if (editController2 == nullptr)     editController2.loadFrom (editController);
        if (midiMapping == nullptr)         midiMapping.loadFrom (editController);
        if (componentHandler == nullptr)    componentHandler.loadFrom (editController);
        if (componentHandler2 == nullptr)   componentHandler2.loadFrom (editController);
        if (trackInfoListener == nullptr)   trackInfoListener.loadFrom (editController);
    }

    void setStateForAllMidiBuses (bool newState)
    {
        setStateForAllBusesOfType (holder->component, newState, true, false);   // Activate/deactivate MIDI inputs
        setStateForAllBusesOfType (holder->component, newState, false, false);  // Activate/deactivate MIDI outputs
    }

    void setupIO()
    {
        setStateForAllMidiBuses (true);

        Vst::ProcessSetup setup;
        setup.symbolicSampleSize   = Vst::kSample32;
        setup.maxSamplesPerBlock   = 1024;
        setup.sampleRate           = 44100.0;
        setup.processMode          = Vst::kRealtime;

        warnOnFailure (processor->setupProcessing (setup));

        cachedBusLayouts = getBusesLayout();
        setRateAndBufferSizeDetails (setup.sampleRate, (int) setup.maxSamplesPerBlock);
    }

    static AudioProcessor::BusesProperties getBusProperties (VSTComSmartPtr<Vst::IComponent>& component)
    {
        AudioProcessor::BusesProperties busProperties;
        VSTComSmartPtr<Vst::IAudioProcessor> processor;
        processor.loadFrom (component.get());

        for (int dirIdx = 0; dirIdx < 2; ++dirIdx)
        {
            const bool isInput = (dirIdx == 0);
            const Vst::BusDirection dir = (isInput ? Vst::kInput : Vst::kOutput);
            const int numBuses = component->getBusCount (Vst::kAudio, dir);

            for (int i = 0; i < numBuses; ++i)
            {
                Vst::BusInfo info;

                if (component->getBusInfo (Vst::kAudio, dir, (Steinberg::int32) i, info) != kResultOk)
                    continue;

                AudioChannelSet layout = (info.channelCount == 0 ? AudioChannelSet::disabled()
                                                                 : AudioChannelSet::discreteChannels (info.channelCount));

                Vst::SpeakerArrangement arr;
                if (processor != nullptr && processor->getBusArrangement (dir, i, arr) == kResultOk)
                    layout = getChannelSetForSpeakerArrangement (arr);

                busProperties.addBus (isInput, toString (info.name), layout,
                                      (info.flags & Vst::BusInfo::kDefaultActive) != 0);
            }
        }

        return busProperties;
    }

    //==============================================================================
    Vst::BusInfo getBusInfo (bool forInput, bool forAudio, int index = 0) const
    {
        Vst::BusInfo busInfo;
        busInfo.mediaType = forAudio ? Vst::kAudio : Vst::kEvent;
        busInfo.direction = forInput ? Vst::kInput : Vst::kOutput;
        busInfo.channelCount = 0;

        holder->component->getBusInfo (busInfo.mediaType, busInfo.direction,
                                       (Steinberg::int32) index, busInfo);
        return busInfo;
    }

    //==============================================================================
    void updateBypass (bool processBlockBypassedCalled)
    {
        // to remain backward compatible, the logic needs to be the following:
        // - if processBlockBypassed was called then definitely bypass the VST3
        // - if processBlock was called then only un-bypass the VST3 if the previous
        //   call was processBlockBypassed, otherwise do nothing
        if (processBlockBypassedCalled)
        {
            if (bypassParam != nullptr && (bypassParam->getValue() == 0.0f || ! lastProcessBlockCallWasBypass))
                bypassParam->setValue (1.0f);
        }
        else
        {
            if (lastProcessBlockCallWasBypass && bypassParam != nullptr)
                bypassParam->setValue (0.0f);

        }

        lastProcessBlockCallWasBypass = processBlockBypassedCalled;
    }

    //==============================================================================
    /** @note An IPlugView, when first created, should start with a ref-count of 1! */
    IPlugView* tryCreatingView() const
    {
        JUCE_ASSERT_MESSAGE_THREAD

        IPlugView* v = editController->createView (Vst::ViewType::kEditor);

        if (v == nullptr) v = editController->createView (nullptr);
        if (v == nullptr) editController->queryInterface (IPlugView::iid, (void**) &v);

        return v;
    }

    //==============================================================================
    template <typename FloatType>
    void associateWith (Vst::ProcessData& destination, AudioBuffer<FloatType>& buffer)
    {
        VST3BufferExchange<FloatType>::mapBufferToBuses (inputBuses,  inputBusMap.get<FloatType>(),  cachedBusLayouts.inputBuses,  buffer);
        VST3BufferExchange<FloatType>::mapBufferToBuses (outputBuses, outputBusMap.get<FloatType>(), cachedBusLayouts.outputBuses, buffer);

        destination.inputs  = inputBuses.getRawDataPointer();
        destination.outputs = outputBuses.getRawDataPointer();
    }

    void associateWith (Vst::ProcessData& destination, MidiBuffer& midiBuffer)
    {
        midiInputs->clear();
        midiOutputs->clear();

        if (acceptsMidi())
            MidiEventList::hostToPluginEventList (*midiInputs,
                                                  midiBuffer,
                                                  destination.inputParameterChanges,
                                                  midiMapping);

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
        Vst::ParameterInfo paramInfo{};

        if (processor != nullptr)
            editController->getParameterInfo (index, paramInfo);

        return paramInfo;
    }

    Vst::ProgramListInfo getProgramListInfo (int index) const
    {
        Vst::ProgramListInfo paramInfo{};

        if (unitInfo != nullptr)
            unitInfo->getProgramListInfo (index, paramInfo);

        return paramInfo;
    }

    void syncProgramNames()
    {
        programNames.clear();

        if (processor == nullptr || editController == nullptr)
            return;

        Vst::UnitID programUnitID;
        Vst::ParameterInfo paramInfo{};

        {
            int idx, num = editController->getParameterCount();

            for (idx = 0; idx < num; ++idx)
                if (editController->getParameterInfo (idx, paramInfo) == kResultOk
                     && (paramInfo.flags & Steinberg::Vst::ParameterInfo::kIsProgramChange) != 0)
                    break;

            if (idx >= num)
                return;

            programParameterID = paramInfo.id;
            programUnitID = paramInfo.unitId;
        }

        if (unitInfo != nullptr)
        {
            Vst::UnitInfo uInfo{};
            const int unitCount = unitInfo->getUnitCount();

            for (int idx = 0; idx < unitCount; ++idx)
            {
                if (unitInfo->getUnitInfo(idx, uInfo) == kResultOk
                      && uInfo.id == programUnitID)
                {
                    const int programListCount = unitInfo->getProgramListCount();

                    for (int j = 0; j < programListCount; ++j)
                    {
                        Vst::ProgramListInfo programListInfo{};

                        if (unitInfo->getProgramListInfo (j, programListInfo) == kResultOk
                              && programListInfo.id == uInfo.programListId)
                        {
                            Vst::String128 name;

                            for (int k = 0; k < programListInfo.programCount; ++k)
                                if (unitInfo->getProgramName (programListInfo.id, k, name) == kResultOk)
                                    programNames.add (toString (name));

                            return;
                        }
                    }

                    break;
                }
            }
        }

        if (editController != nullptr && paramInfo.stepCount > 0)
        {
            auto numPrograms = paramInfo.stepCount + 1;

            for (int i = 0; i < numPrograms; ++i)
            {
                auto valueNormalized = static_cast<Vst::ParamValue> (i) / static_cast<Vst::ParamValue> (paramInfo.stepCount);

                Vst::String128 programName;
                if (editController->getParamStringByValue (paramInfo.id, valueNormalized, programName) == kResultOk)
                    programNames.add (toString (programName));
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginInstance)
};

JUCE_END_IGNORE_WARNINGS_MSVC

//==============================================================================
AudioPluginInstance* VST3ComponentHolder::createPluginInstance()
{
    if (! initialise())
        return nullptr;

    auto* plugin = new VST3PluginInstance (this);
    host->setPlugin (plugin);
    return plugin;
}

//==============================================================================
tresult VST3HostContext::beginEdit (Vst::ParamID paramID)
{
    if (plugin != nullptr)
    {
        if (auto* param = plugin->getParameterForID (paramID))
        {
            param->beginChangeGesture();
            return kResultTrue;
        }

        jassertfalse; // Invalid parameter index!
        return kResultFalse;
    }

    return kResultTrue;
}

tresult VST3HostContext::performEdit (Vst::ParamID paramID, Vst::ParamValue valueNormalised)
{
    if (plugin != nullptr)
    {
        if (auto* param = plugin->getParameterForID (paramID))
            param->setValueFromEditor ((float) valueNormalised);
        else
            jassertfalse; // Invalid parameter index!

        // did the plug-in already update the parameter internally
        if (plugin->editController->getParamNormalized (paramID) != (float) valueNormalised)
            return plugin->editController->setParamNormalized (paramID, valueNormalised);
    }

    return kResultTrue;
}

tresult VST3HostContext::endEdit (Vst::ParamID paramID)
{
    if (plugin != nullptr)
    {
        if (auto* param = plugin->getParameterForID (paramID))
        {
            param->endChangeGesture();
            return kResultTrue;
        }

        jassertfalse; // Invalid parameter index!
        return kResultFalse;
    }

    return kResultTrue;
}

tresult VST3HostContext::restartComponent (Steinberg::int32 flags)
{
    if (plugin != nullptr)
    {
        if (hasFlag (flags, Vst::kReloadComponent))
            plugin->reset();

        if (hasFlag (flags, Vst::kIoChanged))
        {
            auto sampleRate = plugin->getSampleRate();
            auto blockSize  = plugin->getBlockSize();

            plugin->prepareToPlay (sampleRate >= 8000 ? sampleRate : 44100.0,
                                   blockSize > 0 ? blockSize : 1024);
        }

        if (hasFlag (flags, Vst::kLatencyChanged))
            if (plugin->processor != nullptr)
                plugin->setLatencySamples (jmax (0, (int) plugin->processor->getLatencySamples()));

        plugin->updateHostDisplay (AudioProcessorListener::ChangeDetails().withProgramChanged (true)
                                                                          .withParameterInfoChanged (true));

        return kResultTrue;
    }

    jassertfalse;
    return kResultFalse;
}

//==============================================================================
tresult VST3HostContext::ContextMenu::popup (Steinberg::UCoord x, Steinberg::UCoord y)
{
    Array<const Item*> subItemStack;
    OwnedArray<PopupMenu> menuStack;
    PopupMenu* topLevelMenu = menuStack.add (new PopupMenu());

    for (int i = 0; i < items.size(); ++i)
    {
        auto& item = items.getReference (i).item;
        auto* menuToUse = menuStack.getLast();

        if (hasFlag (item.flags, Item::kIsGroupStart & ~Item::kIsDisabled))
        {
            subItemStack.add (&item);
            menuStack.add (new PopupMenu());
        }
        else if (hasFlag (item.flags, Item::kIsGroupEnd))
        {
            if (auto* subItem = subItemStack.getLast())
            {
                if (auto* m = menuStack [menuStack.size() - 2])
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

    if (auto* ed = owner.getActiveEditor())
    {
       #if JUCE_WINDOWS && JUCE_WIN_PER_MONITOR_DPI_AWARE
        if (auto* peer = ed->getPeer())
        {
            auto scale = peer->getPlatformScaleFactor();

            x = roundToInt (x / scale);
            y = roundToInt (y / scale);
        }
       #endif

        options = options.withTargetScreenArea (ed->getScreenBounds().translated ((int) x, (int) y).withSize (1, 1));
    }

   #if JUCE_MODAL_LOOPS_PERMITTED
    // Unfortunately, Steinberg's docs explicitly say this should be modal..
    handleResult (topLevelMenu->showMenu (options));
   #else
    topLevelMenu->showMenuAsync (options, ModalCallbackFunction::create (menuFinished, VSTComSmartPtr<ContextMenu> (this)));
   #endif

    return kResultOk;
}

//==============================================================================
tresult VST3HostContext::notifyProgramListChange (Vst::ProgramListID, Steinberg::int32)
{
    if (plugin != nullptr)
        plugin->syncProgramNames();

    return kResultTrue;
}

//==============================================================================
//==============================================================================
VST3PluginFormat::VST3PluginFormat() {}
VST3PluginFormat::~VST3PluginFormat() {}

bool VST3PluginFormat::setStateFromVSTPresetFile (AudioPluginInstance* api, const MemoryBlock& rawData)
{
    if (auto vst3 = dynamic_cast<VST3PluginInstance*> (api))
        return vst3->setStateFromPresetFile (rawData);

    return false;
}

void VST3PluginFormat::findAllTypesForFile (OwnedArray<PluginDescription>& results, const String& fileOrIdentifier)
{
    if (fileMightContainThisPluginType (fileOrIdentifier))
    {
        /**
            Since there is no apparent indication if a VST3 plugin is a shell or not,
            we're stuck iterating through a VST3's factory, creating a description
            for every housed plugin.
        */

        VSTComSmartPtr<IPluginFactory> pluginFactory (DLLHandleCache::getInstance()->findOrCreateHandle (fileOrIdentifier)
                                                                                    .getPluginFactory());

        if (pluginFactory != nullptr)
        {
            VSTComSmartPtr<VST3HostContext> host (new VST3HostContext());
            DescriptionLister lister (host, pluginFactory);
            lister.findDescriptionsAndPerform (File (fileOrIdentifier));

            results.addCopiesOf (lister.list);
        }
        else
        {
            jassertfalse;
        }
    }
}

void VST3PluginFormat::createPluginInstance (const PluginDescription& description,
                                             double, int, PluginCreationCallback callback)
{
    std::unique_ptr<VST3PluginInstance> result;

    if (fileMightContainThisPluginType (description.fileOrIdentifier))
    {
        File file (description.fileOrIdentifier);

        auto previousWorkingDirectory = File::getCurrentWorkingDirectory();
        file.getParentDirectory().setAsCurrentWorkingDirectory();

        if (const VST3ModuleHandle::Ptr module = VST3ModuleHandle::findOrCreateModule (file, description))
        {
            std::unique_ptr<VST3ComponentHolder> holder (new VST3ComponentHolder (module));

            if (holder->initialise())
            {
                result.reset (new VST3PluginInstance (holder.release()));

                if (! result->initialise())
                    result.reset();
            }
        }

        previousWorkingDirectory.setAsCurrentWorkingDirectory();
    }

    String errorMsg;

    if (result == nullptr)
        errorMsg = TRANS ("Unable to load XXX plug-in file").replace ("XXX", "VST-3");

    callback (std::move (result), errorMsg);
}

bool VST3PluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const
{
    return false;
}

bool VST3PluginFormat::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    auto f = File::createFileWithoutCheckingPath (fileOrIdentifier);

    return f.hasFileExtension (".vst3")
          #if JUCE_MAC || JUCE_LINUX || JUCE_BSD
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

StringArray VST3PluginFormat::searchPathsForPlugins (const FileSearchPath& directoriesToSearch, const bool recursive, bool)
{
    StringArray results;

    for (int i = 0; i < directoriesToSearch.getNumPaths(); ++i)
        recursiveFileSearch (results, directoriesToSearch[i], recursive);

    return results;
}

void VST3PluginFormat::recursiveFileSearch (StringArray& results, const File& directory, const bool recursive)
{
    for (const auto& iter : RangedDirectoryIterator (directory, false, "*", File::findFilesAndDirectories))
    {
        auto f = iter.getFile();
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
    auto programFiles = File::getSpecialLocation (File::globalApplicationsDirectory).getFullPathName();
    return FileSearchPath (programFiles + "\\Common Files\\VST3");
   #elif JUCE_MAC
    return FileSearchPath ("/Library/Audio/Plug-Ins/VST3;~/Library/Audio/Plug-Ins/VST3");
   #else
    return FileSearchPath ("/usr/lib/vst3/;/usr/local/lib/vst3/;~/.vst3/");
   #endif
}

} // namespace juce

#endif // JUCE_PLUGINHOST_VST3
