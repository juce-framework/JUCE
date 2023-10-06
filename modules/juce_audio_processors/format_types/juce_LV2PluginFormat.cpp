/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#if JUCE_PLUGINHOST_LV2 && (! (JUCE_ANDROID || JUCE_IOS))

#include "juce_LV2Common.h"
#include "juce_LV2Resources.h"

#include <juce_gui_extra/native/juce_NSViewFrameWatcher_mac.h>

#include <thread>

namespace juce
{
namespace lv2_host
{

template <typename Struct, typename Value>
auto with (Struct s, Value Struct::* member, Value value) noexcept
{
    s.*member = std::move (value);
    return s;
}

/*  Converts a void* to an LV2_Atom* if the buffer looks like it holds a well-formed Atom, or
    returns nullptr otherwise.
*/
static const LV2_Atom* convertToAtomPtr (const void* ptr, size_t size)
{
    if (size < sizeof (LV2_Atom))
    {
        jassertfalse;
        return nullptr;
    }

    const auto header = readUnaligned<LV2_Atom> (ptr);

    if (size < header.size + sizeof (LV2_Atom))
    {
        jassertfalse;
        return nullptr;
    }

    // This is UB _if_ the ptr doesn't really point to an LV2_Atom.
    return reinterpret_cast<const LV2_Atom*> (ptr);
}

// Allows mutable access to the items in a vector, without allowing the vector itself
// to be modified.
template <typename T>
class SimpleSpan
{
public:
    constexpr SimpleSpan (T* beginIn, T* endIn) : b (beginIn), e (endIn) {}

    constexpr auto begin() const { return b; }
    constexpr auto end()   const { return e; }

    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4814)
    constexpr auto& operator[] (size_t index)       { return b[index]; }
    JUCE_END_IGNORE_WARNINGS_MSVC

    constexpr auto size() const { return (size_t) (e - b); }

private:
    T* b;
    T* e;
};

template <typename T>
constexpr auto makeSimpleSpan (T* b, T* e) { return SimpleSpan<T> { b, e }; }

template <typename R>
constexpr auto makeSimpleSpan (R& r) { return makeSimpleSpan (r.data(), r.data() + r.size()); }

struct PhysicalResizeListener
{
    virtual ~PhysicalResizeListener() = default;
    virtual void viewRequestedResizeInPhysicalPixels (int width, int height) = 0;
};

struct LogicalResizeListener
{
    virtual ~LogicalResizeListener() = default;
    virtual void viewRequestedResizeInLogicalPixels (int width, int height) = 0;
};

#if JUCE_WINDOWS
class WindowSizeChangeDetector
{
public:
    WindowSizeChangeDetector()
        : hook (SetWindowsHookEx (WH_CALLWNDPROC,
                                  callWndProc,
                                  (HINSTANCE) juce::Process::getCurrentModuleInstanceHandle(),
                                  GetCurrentThreadId()))
    {}

    ~WindowSizeChangeDetector() noexcept
    {
        UnhookWindowsHookEx (hook);
    }

    static void addListener (HWND hwnd, PhysicalResizeListener& listener)
    {
        getActiveEditors().emplace (hwnd, &listener);
    }

    static void removeListener (HWND hwnd)
    {
        getActiveEditors().erase (hwnd);
    }

private:
    static std::map<HWND, PhysicalResizeListener*>& getActiveEditors()
    {
        static std::map<HWND, PhysicalResizeListener*> map;
        return map;
    }

    static void processMessage (int nCode, const CWPSTRUCT* info)
    {
        if (nCode < 0 || info == nullptr)
            return;

        constexpr UINT events[] { WM_SIZING, WM_SIZE, WM_WINDOWPOSCHANGING, WM_WINDOWPOSCHANGED };

        if (std::find (std::begin (events), std::end (events), info->message) == std::end (events))
            return;

        auto& map = getActiveEditors();
        auto iter = map.find (info->hwnd);

        if (iter == map.end())
            return;

        RECT rect;
        GetWindowRect (info->hwnd, &rect);
        iter->second->viewRequestedResizeInPhysicalPixels (rect.right - rect.left, rect.bottom - rect.top);
    }

    static LRESULT CALLBACK callWndProc (int nCode, WPARAM wParam, LPARAM lParam)
    {
        processMessage (nCode, lv2_shared::wordCast<CWPSTRUCT*> (lParam));
        return CallNextHookEx ({}, nCode, wParam, lParam);
    }

    HHOOK hook;
};

class WindowSizeChangeListener
{
public:
    WindowSizeChangeListener (HWND hwndIn, PhysicalResizeListener& l)
        : hwnd (hwndIn)
    {
        detector->addListener (hwnd, l);
    }

    ~WindowSizeChangeListener()
    {
        detector->removeListener (hwnd);
    }

private:
    SharedResourcePointer<WindowSizeChangeDetector> detector;
    HWND hwnd;

    JUCE_LEAK_DETECTOR (WindowSizeChangeListener)
};
#endif

struct FreeLilvNode
{
    void operator() (LilvNode* ptr) const noexcept { lilv_node_free (ptr); }
};

using OwningNode = std::unique_ptr<LilvNode, FreeLilvNode>;

template <typename Traits>
class TypesafeLilvNode
{
public:
    template <typename... Ts>
    TypesafeLilvNode (Ts&&... ts)
        : node (Traits::construct (std::forward<Ts> (ts)...)) {}

    bool equals (const TypesafeLilvNode& other) const noexcept
    {
        return lilv_node_equals (node.get(), other.node.get());
    }

    const LilvNode* get() const noexcept { return node.get(); }

    auto getTyped() const noexcept -> decltype (Traits::access (nullptr))
    {
        return Traits::access (node.get());
    }

    static TypesafeLilvNode claim (LilvNode* node)
    {
        return TypesafeLilvNode { node };
    }

    static TypesafeLilvNode copy (const LilvNode* node)
    {
        return TypesafeLilvNode { lilv_node_duplicate (node) };
    }

private:
    explicit TypesafeLilvNode (LilvNode* ptr)
        : node (ptr)
    {
        jassert (ptr == nullptr || Traits::verify (node.get()));
    }

    OwningNode node;

    JUCE_LEAK_DETECTOR (TypesafeLilvNode)
};

struct UriConstructorTrait
{
    static LilvNode* construct (LilvWorld* world, const char* uri) noexcept
    {
        return lilv_new_uri (world, uri);
    }

    static LilvNode* construct (LilvWorld* world, const char* host, const char* path) noexcept
    {
        return lilv_new_file_uri (world, host, path);
    }

    static constexpr auto verify = lilv_node_is_uri;
    static constexpr auto access = lilv_node_as_uri;
};

struct StringConstructorTrait { static constexpr auto construct  = lilv_new_string;
                                static constexpr auto verify     = lilv_node_is_string;
                                static constexpr auto access     = lilv_node_as_string; };

using NodeUri    = TypesafeLilvNode<UriConstructorTrait>;
using NodeString = TypesafeLilvNode<StringConstructorTrait>;

struct UsefulUris
{
    explicit UsefulUris (LilvWorld* worldIn)
        : world (worldIn) {}

    LilvWorld* const world = nullptr;

   #define X(str) const NodeUri m##str { world, str };
    X (LV2_ATOM__AtomPort)
    X (LV2_ATOM__atomTransfer)
    X (LV2_ATOM__eventTransfer)
    X (LV2_CORE__AudioPort)
    X (LV2_CORE__CVPort)
    X (LV2_CORE__ControlPort)
    X (LV2_CORE__GeneratorPlugin)
    X (LV2_CORE__InputPort)
    X (LV2_CORE__InstrumentPlugin)
    X (LV2_CORE__OutputPort)
    X (LV2_CORE__enumeration)
    X (LV2_CORE__integer)
    X (LV2_CORE__toggled)
    X (LV2_RESIZE_PORT__minimumSize)
    X (LV2_UI__floatProtocol)
    X (LV2_WORKER__interface)
   #undef X
};

template <typename Ptr, typename Free>
struct OwningPtrTraits
{
    using type = std::unique_ptr<Ptr, Free>;
    static const Ptr* get (const type& t) noexcept { return t.get(); }
};

template <typename Ptr>
struct NonOwningPtrTraits
{
    using type = const Ptr*;
    static const Ptr* get (const type& t) noexcept { return t; }
};

struct PluginsIteratorTraits
{
    using Container                 = const LilvPlugins*;
    using Iter                      = LilvIter*;
    static constexpr auto begin     = lilv_plugins_begin;
    static constexpr auto next      = lilv_plugins_next;
    static constexpr auto isEnd     = lilv_plugins_is_end;
    static constexpr auto get       = lilv_plugins_get;
};

using PluginsIterator = lv2_shared::Iterator<PluginsIteratorTraits>;

struct PluginClassesIteratorTraits
{
    using Container                 = const LilvPluginClasses*;
    using Iter                      = LilvIter*;
    static constexpr auto begin     = lilv_plugin_classes_begin;
    static constexpr auto next      = lilv_plugin_classes_next;
    static constexpr auto isEnd     = lilv_plugin_classes_is_end;
    static constexpr auto get       = lilv_plugin_classes_get;
};

using PluginClassesIterator = lv2_shared::Iterator<PluginClassesIteratorTraits>;

struct NodesIteratorTraits
{
    using Container                 = const LilvNodes*;
    using Iter                      = LilvIter*;
    static constexpr auto begin     = lilv_nodes_begin;
    static constexpr auto next      = lilv_nodes_next;
    static constexpr auto isEnd     = lilv_nodes_is_end;
    static constexpr auto get       = lilv_nodes_get;
};

using NodesIterator = lv2_shared::Iterator<NodesIteratorTraits>;

struct ScalePointsIteratorTraits
{
    using Container                 = const LilvScalePoints*;
    using Iter                      = LilvIter*;
    static constexpr auto begin     = lilv_scale_points_begin;
    static constexpr auto next      = lilv_scale_points_next;
    static constexpr auto isEnd     = lilv_scale_points_is_end;
    static constexpr auto get       = lilv_scale_points_get;
};

using ScalePointsIterator = lv2_shared::Iterator<ScalePointsIteratorTraits>;

struct UisIteratorTraits
{
    using Container                 = const LilvUIs*;
    using Iter                      = LilvIter*;
    static constexpr auto begin     = lilv_uis_begin;
    static constexpr auto next      = lilv_uis_next;
    static constexpr auto isEnd     = lilv_uis_is_end;
    static constexpr auto get       = lilv_uis_get;
};

using UisIterator = lv2_shared::Iterator<UisIteratorTraits>;

template <typename PtrTraits>
class NodesImpl
{
public:
    using type = typename PtrTraits::type;

    template <typename Ptr>
    explicit NodesImpl (Ptr* ptr)
        : nodes (type { ptr }) {}

    explicit NodesImpl (type ptr)
        : nodes (std::move (ptr)) {}

    unsigned size() const noexcept { return lilv_nodes_size (PtrTraits::get (nodes)); }

    NodesIterator begin() const noexcept
    {
        return nodes == nullptr ? NodesIterator{}
                                : NodesIterator { PtrTraits::get (nodes) };
    }

    NodesIterator end()   const noexcept { return {}; }

private:
    type nodes{};
};

struct NodesFree
{
    void operator() (LilvNodes* ptr) const noexcept { lilv_nodes_free (ptr); }
};

using OwningNodes    = NodesImpl<OwningPtrTraits<LilvNodes, NodesFree>>;
using NonOwningNodes = NodesImpl<NonOwningPtrTraits<LilvNodes>>;

class ScalePoints
{
public:
    explicit ScalePoints (const LilvScalePoints* pt)
        : points (pt) {}

    ScalePointsIterator begin() const noexcept
    {
        return points == nullptr ? ScalePointsIterator{}
                                 : ScalePointsIterator { points };
    }

    ScalePointsIterator end() const noexcept { return {}; }

private:
    const LilvScalePoints* points = nullptr;
};

class ScalePoint
{
public:
    explicit ScalePoint (const LilvScalePoint* pt)
        : point (pt) {}

    const LilvNode* getLabel() const noexcept { return lilv_scale_point_get_label (point); }
    const LilvNode* getValue() const noexcept { return lilv_scale_point_get_value (point); }

private:
    const LilvScalePoint* point = nullptr;
};

struct PortRange
{
    float defaultValue, min, max;
};

class Port
{
public:
    enum class Kind
    {
        control,
        audio,
        cv,
        atom,
        unknown,
    };

    enum class Direction
    {
        input,
        output,
        unknown,
    };

    Port (const LilvPlugin* pluginIn, const LilvPort* portIn)
        : plugin (pluginIn), port (portIn) {}

    Direction getDirection (const UsefulUris& uris) const noexcept
    {
        if (isA (uris.mLV2_CORE__InputPort))
            return Direction::input;

        if (isA (uris.mLV2_CORE__OutputPort))
            return Direction::output;

        return Direction::unknown;
    }

    Kind getKind (const UsefulUris& uris) const noexcept
    {
        if (isA (uris.mLV2_CORE__ControlPort))
            return Kind::control;

        if (isA (uris.mLV2_CORE__AudioPort))
            return Kind::audio;

        if (isA (uris.mLV2_CORE__CVPort))
            return Kind::cv;

        if (isA (uris.mLV2_ATOM__AtomPort))
            return Kind::atom;

        return Kind::unknown;
    }

    OwningNode get (const LilvNode* predicate) const noexcept
    {
        return OwningNode { lilv_port_get (plugin, port, predicate) };
    }

    NonOwningNodes getClasses() const noexcept
    {
        return NonOwningNodes { lilv_port_get_classes (plugin, port) };
    }

    NodeString getName() const noexcept
    {
        return NodeString::claim (lilv_port_get_name (plugin, port));
    }

    NodeString getSymbol() const noexcept
    {
        return NodeString::copy (lilv_port_get_symbol (plugin, port));
    }

    OwningNodes getProperties() const noexcept
    {
        return OwningNodes { lilv_port_get_properties (plugin, port) };
    }

    ScalePoints getScalePoints() const noexcept
    {
        return ScalePoints { lilv_port_get_scale_points (plugin, port) };
    }

    bool hasProperty (const NodeUri& uri) const noexcept
    {
        return lilv_port_has_property (plugin, port, uri.get());
    }

    uint32_t getIndex() const noexcept { return lilv_port_get_index (plugin, port); }

    static float getFloatValue (const LilvNode* node, float fallback)
    {
        if (lilv_node_is_float (node) || lilv_node_is_int (node))
            return lilv_node_as_float (node);

        return fallback;
    }

    bool supportsEvent (const LilvNode* node) const noexcept
    {
        return lilv_port_supports_event (plugin, port, node);
    }

    PortRange getRange() const noexcept
    {
        LilvNode* def = nullptr;
        LilvNode* min = nullptr;
        LilvNode* max = nullptr;

        lilv_port_get_range (plugin, port, &def, &min, &max);

        const OwningNode defOwner { def };
        const OwningNode minOwner { min };
        const OwningNode maxOwner { max };

        return { getFloatValue (def, 0.0f),
                 getFloatValue (min, 0.0f),
                 getFloatValue (max, 1.0f) };
    }

    bool isValid() const noexcept { return port != nullptr; }

private:
    bool isA (const NodeUri& uri) const noexcept
    {
        return lilv_port_is_a (plugin, port, uri.get());
    }

    const LilvPlugin* plugin = nullptr;
    const LilvPort* port = nullptr;

    JUCE_LEAK_DETECTOR (Port)
};

class Plugin
{
public:
    explicit Plugin (const LilvPlugin* p) : plugin (p) {}

    bool verify() const noexcept              { return lilv_plugin_verify (plugin); }
    NodeUri getUri() const noexcept           { return NodeUri::copy (lilv_plugin_get_uri (plugin)); }
    NodeUri getBundleUri() const noexcept     { return NodeUri::copy (lilv_plugin_get_bundle_uri (plugin)); }
    NodeUri getLibraryUri() const noexcept    { return NodeUri::copy (lilv_plugin_get_library_uri (plugin)); }
    NodeString getName() const noexcept       { return NodeString::claim (lilv_plugin_get_name (plugin)); }
    NodeString getAuthorName() const noexcept { return NodeString::claim (lilv_plugin_get_author_name (plugin)); }
    uint32_t getNumPorts() const noexcept { return lilv_plugin_get_num_ports (plugin); }
    const LilvPluginClass* getClass() const noexcept { return lilv_plugin_get_class (plugin); }
    OwningNodes getValue (const LilvNode* predicate) const noexcept { return OwningNodes { lilv_plugin_get_value (plugin, predicate) }; }

    Port getPortByIndex (uint32_t index) const noexcept
    {
        return Port { plugin, lilv_plugin_get_port_by_index (plugin, index) };
    }

    Port getPortByDesignation (const LilvNode* portClass, const LilvNode* designation) const noexcept
    {
        return Port { plugin, lilv_plugin_get_port_by_designation (plugin, portClass, designation) };
    }

    OwningNodes getRequiredFeatures() const noexcept
    {
        return OwningNodes { lilv_plugin_get_required_features (plugin) };
    }

    OwningNodes getOptionalFeatures() const noexcept
    {
        return OwningNodes { lilv_plugin_get_optional_features (plugin) };
    }

    bool hasExtensionData (const NodeUri& uri) const noexcept
    {
        return lilv_plugin_has_extension_data (plugin, uri.get());
    }

    bool hasFeature (const NodeUri& uri) const noexcept
    {
        return lilv_plugin_has_feature (plugin, uri.get());
    }

    template <typename... Classes>
    uint32_t getNumPortsOfClass (const Classes&... classes) const noexcept
    {
        return lilv_plugin_get_num_ports_of_class (plugin, classes.get()..., 0);
    }

    const LilvPlugin* get() const noexcept { return plugin; }

    bool hasLatency() const noexcept { return lilv_plugin_has_latency (plugin); }
    uint32_t getLatencyPortIndex() const noexcept { return lilv_plugin_get_latency_port_index (plugin); }

private:
    const LilvPlugin* plugin = nullptr;

    JUCE_LEAK_DETECTOR (Plugin)
};

/*
    This is very similar to the symap implementation in jalv.
*/
class SymbolMap
{
public:
    SymbolMap() = default;

    SymbolMap (std::initializer_list<const char*> uris)
    {
        for (const auto* str : uris)
            map (str);
    }

    LV2_URID map (const char* uri)
    {
        const auto comparator = [this] (size_t index, const String& str)
        {
            return strings[index] < str;
        };

        const auto uriString = String::fromUTF8 (uri);
        const auto it = std::lower_bound (indices.cbegin(), indices.cend(), uriString, comparator);

        if (it != indices.cend() && strings[*it] == uriString)
            return static_cast<LV2_URID> (*it + 1);

        const auto index = strings.size();
        indices.insert (it, index);
        strings.push_back (uriString);
        return static_cast<LV2_URID> (index + 1);
    }

    const char* unmap (LV2_URID urid) const
    {
        const auto index = urid - 1;
        return index < strings.size() ? strings[index].toRawUTF8()
                                      : nullptr;
    }

    static LV2_URID mapUri (LV2_URID_Map_Handle handle, const char* uri)
    {
        return static_cast<SymbolMap*> (handle)->map (uri);
    }

    static const char* unmapUri (LV2_URID_Unmap_Handle handle, LV2_URID urid)
    {
        return static_cast<SymbolMap*> (handle)->unmap (urid);
    }

    LV2_URID_Map   getMapFeature()      { return { this, mapUri }; }
    LV2_URID_Unmap getUnmapFeature()    { return { this, unmapUri }; }

private:
    std::vector<String> strings;
    std::vector<size_t> indices;

    JUCE_LEAK_DETECTOR (SymbolMap)
};

struct UsefulUrids
{
    explicit UsefulUrids (SymbolMap& m) : symap (m) {}

    SymbolMap& symap;

   #define X(token) const LV2_URID m##token = symap.map (token);
    X (LV2_ATOM__Bool)
    X (LV2_ATOM__Double)
    X (LV2_ATOM__Float)
    X (LV2_ATOM__Int)
    X (LV2_ATOM__Long)
    X (LV2_ATOM__Object)
    X (LV2_ATOM__Sequence)
    X (LV2_ATOM__atomTransfer)
    X (LV2_ATOM__beatTime)
    X (LV2_ATOM__eventTransfer)
    X (LV2_ATOM__frameTime)
    X (LV2_LOG__Error)
    X (LV2_LOG__Note)
    X (LV2_LOG__Trace)
    X (LV2_LOG__Warning)
    X (LV2_MIDI__MidiEvent)
    X (LV2_PATCH__Set)
    X (LV2_PATCH__property)
    X (LV2_PATCH__value)
    X (LV2_STATE__StateChanged)
    X (LV2_TIME__Position)
    X (LV2_TIME__barBeat)
    X (LV2_TIME__beat)
    X (LV2_TIME__beatUnit)
    X (LV2_TIME__beatsPerBar)
    X (LV2_TIME__beatsPerMinute)
    X (LV2_TIME__frame)
    X (LV2_TIME__speed)
    X (LV2_TIME__bar)
    X (LV2_UI__floatProtocol)
    X (LV2_UNITS__beat)
    X (LV2_UNITS__frame)
   #undef X
};

class Log
{
public:
    explicit Log (const UsefulUrids* u) : urids (u) {}

    LV2_Log_Log* getLogFeature() { return &logFeature; }

private:
    int vprintfCallback ([[maybe_unused]] LV2_URID type, const char* fmt, va_list ap) const
    {
        // If this is hit, the plugin has encountered some kind of error
        ignoreUnused (urids);
        jassert (type != urids->mLV2_LOG__Error && type != urids->mLV2_LOG__Warning);
        return std::vfprintf (stderr, fmt, ap);
    }

    static int vprintfCallback (LV2_Log_Handle handle,
                                LV2_URID type,
                                const char* fmt,
                                va_list ap)
    {
        return static_cast<const Log*> (handle)->vprintfCallback (type, fmt, ap);
    }

    static int printfCallback (LV2_Log_Handle handle, LV2_URID type, const char* fmt, ...)
    {
        va_list list;
        va_start (list, fmt);
        auto result = vprintfCallback (handle, type, fmt, list);
        va_end (list);
        return result;
    }

    const UsefulUrids* urids = nullptr;
    LV2_Log_Log logFeature { this, printfCallback, vprintfCallback };

    JUCE_LEAK_DETECTOR (Log)
};

struct Features
{
    explicit Features (std::vector<LV2_Feature>&& f)
        : features (std::move (f)) {}

    static std::vector<String> getUris (const std::vector<LV2_Feature>& features)
    {
        std::vector<String> result;
        result.reserve (features.size());

        for (const auto& feature : features)
            result.push_back (String::fromUTF8 (feature.URI));

        return result;
    }

    std::vector<LV2_Feature> features;
    std::vector<const LV2_Feature*> pointers = makeNullTerminatedArray();

private:
    std::vector<const LV2_Feature*> makeNullTerminatedArray()
    {
        std::vector<const LV2_Feature*> result;
        result.reserve (features.size() + 1);

        for (const auto& feature : features)
            result.push_back (&feature);

        result.push_back (nullptr);

        return result;
    }

    JUCE_LEAK_DETECTOR (Features)
};

template <typename Extension>
struct OptionalExtension
{
    OptionalExtension() = default;

    explicit OptionalExtension (Extension extensionIn) : extension (extensionIn), valid (true) {}

    Extension extension;
    bool valid = false;
};

class Instance
{
    struct Free
    {
        void operator() (LilvInstance* ptr) const noexcept { lilv_instance_free (ptr); }
    };

public:
    using Ptr = std::unique_ptr<LilvInstance, Free>;
    using GetExtensionData = const void* (*) (const char*);

    Instance (const Plugin& pluginIn, double sampleRate, const LV2_Feature* const* features)
        : plugin (pluginIn),
          instance (lilv_plugin_instantiate (plugin.get(), sampleRate, features)) {}

    void activate() { lilv_instance_activate (instance.get()); }
    void run (uint32_t sampleCount) { lilv_instance_run (instance.get(), sampleCount); }
    void deactivate() { lilv_instance_deactivate (instance.get()); }

    const char* getUri() const noexcept { return lilv_instance_get_uri (instance.get()); }

    LV2_Handle getHandle() const noexcept { return lilv_instance_get_handle (instance.get()); }

    LilvInstance* get() const noexcept { return instance.get(); }

    void connectPort (uint32_t index, void* data)
    {
        lilv_instance_connect_port (instance.get(), index, data);
    }

    template <typename Extension>
    OptionalExtension<Extension> getExtensionData (const NodeUri& uri) const noexcept
    {
        if (plugin.get() == nullptr || ! plugin.hasExtensionData (uri) || instance.get() == nullptr)
            return {};

        return OptionalExtension<Extension> { readUnaligned<Extension> (lilv_instance_get_extension_data (instance.get(), uri.getTyped())) };
    }

    GetExtensionData getExtensionDataCallback() const noexcept
    {
        return instance->lv2_descriptor->extension_data;
    }

    bool operator== (std::nullptr_t) const noexcept { return instance == nullptr; }
    bool operator!= (std::nullptr_t) const noexcept { return ! (*this == nullptr); }

private:
    Plugin plugin;
    Ptr instance;

    JUCE_LEAK_DETECTOR (Instance)
};

enum class Realtime { no, yes };

// Must be trivial!
struct WorkResponder
{
    static WorkResponder getDefault() { return { nullptr, nullptr }; }

    LV2_Worker_Status processResponse (uint32_t size, const void* data) const
    {
        return worker->work_response (handle, size, data);
    }

    bool isValid() const { return handle != nullptr && worker != nullptr; }

    LV2_Handle handle;
    const LV2_Worker_Interface* worker;
};

struct WorkerResponseListener
{
    virtual ~WorkerResponseListener() = default;
    virtual LV2_Worker_Status responseGenerated (WorkResponder, uint32_t, const void*) = 0;
};

struct RespondHandle
{
    LV2_Worker_Status respond (uint32_t size, const void* data) const
    {
        if (realtime == Realtime::yes)
            return listener.responseGenerated (responder, size, data);

        return responder.processResponse (size, data);
    }

    static LV2_Worker_Status respond (LV2_Worker_Respond_Handle handle,
                                      uint32_t size,
                                      const void* data)
    {
        return static_cast<const RespondHandle*> (handle)->respond (size, data);
    }

    WorkResponder responder;
    WorkerResponseListener& listener;
    Realtime realtime;
};

// Must be trivial!
struct WorkSubmitter
{
    static WorkSubmitter getDefault() { return { nullptr, nullptr, nullptr, nullptr }; }

    LV2_Worker_Status doWork (Realtime realtime, uint32_t size, const void* data) const
    {
        // The Worker spec says that the host "MUST NOT make concurrent calls to [work] from
        // several threads".
        // Taking the work mutex here ensures that only one piece of work is done at a time.
        // If we didn't take the work mutex, there would be a danger of work happening
        // simultaneously on the worker thread and the render thread when switching between
        // realtime/offline modes (in realtime mode, work happens on the worker thread; in
        // offline mode, work happens immediately on the render/audio thread).
        const ScopedLock lock (*workMutex);

        RespondHandle respondHandle { WorkResponder { handle, worker }, *listener, realtime };
        return worker->work (handle, RespondHandle::respond, &respondHandle, size, data);
    }

    bool isValid() const { return handle != nullptr && worker != nullptr && listener != nullptr && workMutex != nullptr; }

    LV2_Handle handle;
    const LV2_Worker_Interface* worker;
    WorkerResponseListener* listener;
    CriticalSection* workMutex;
};

template <typename Trivial>
static auto toChars (Trivial value)
{
    static_assert (std::is_trivial_v<Trivial>);
    std::array<char, sizeof (Trivial)> result;
    writeUnaligned (result.data(), value);
    return result;
}

template <typename Context>
class WorkQueue
{
public:
    static_assert (std::is_trivial_v<Context>, "Context must be copyable as bytes");

    explicit WorkQueue (int size)
        : fifo (size), data (static_cast<size_t> (size)) {}

    LV2_Worker_Status push (Context context, size_t size, const void* contents)
    {
        const auto* bytes = static_cast<const char*> (contents);
        const auto numToWrite = sizeof (Header) + size;

        if (static_cast<size_t> (fifo.getFreeSpace()) < numToWrite)
            return LV2_WORKER_ERR_NO_SPACE;

        Header header { size, context };
        const auto headerBuffer = toChars (header);

        const auto scope = fifo.write (static_cast<int> (numToWrite));
        jassert (scope.blockSize1 + scope.blockSize2 == static_cast<int> (numToWrite));

        size_t index = 0;
        scope.forEach ([&] (int i)
        {
            data[static_cast<size_t> (i)] = index < headerBuffer.size() ? headerBuffer[index]
                                                                        : bytes[index - headerBuffer.size()];
            ++index;
        });

        return LV2_WORKER_SUCCESS;
    }

    Context pop (std::vector<char>& dest)
    {
        // If the vector is too small we'll have to resize it on the audio thread
        jassert (dest.capacity() >= data.size());
        dest.clear();

        const auto numReady = fifo.getNumReady();

        if (static_cast<size_t> (numReady) < sizeof (Header))
        {
            jassert (numReady == 0);
            return Context::getDefault();
        }

        std::array<char, sizeof (Header)> headerBuffer;

        {
            size_t index = 0;
            fifo.read (sizeof (Header)).forEach ([&] (int i)
            {
                headerBuffer[index++] = data[static_cast<size_t> (i)];
            });
        }

        const auto header = readUnaligned<Header> (headerBuffer.data());

        jassert (static_cast<size_t> (fifo.getNumReady()) >= header.size);

        dest.resize (header.size);

        {
            size_t index = 0;
            fifo.read (static_cast<int> (header.size)).forEach ([&] (int i)
            {
                dest[index++] = data[static_cast<size_t> (i)];
            });
        }

        return header.context;
    }

private:
    struct Header
    {
        size_t size;
        Context context;
    };

    AbstractFifo fifo;
    std::vector<char> data;

    JUCE_LEAK_DETECTOR (WorkQueue)
};

/*
    Keeps track of active plugin instances, so that we can avoid sending work
    messages to dead plugins.
*/
class HandleRegistry
{
public:
    void insert (LV2_Handle handle)
    {
        const SpinLock::ScopedLockType lock (mutex);
        handles.insert (handle);
    }

    void erase (LV2_Handle handle)
    {
        const SpinLock::ScopedLockType lock (mutex);
        handles.erase (handle);
    }

    template <typename Fn>
    LV2_Worker_Status ifContains (LV2_Handle handle, Fn&& callback)
    {
        const SpinLock::ScopedLockType lock (mutex);

        if (handles.find (handle) != handles.cend())
            return callback();

        return LV2_WORKER_ERR_UNKNOWN;
    }

private:
    std::set<LV2_Handle> handles;
    SpinLock mutex;

    JUCE_LEAK_DETECTOR (HandleRegistry)
};

/*
    Implements an LV2 Worker, allowing work to be scheduled in realtime
    by the plugin instance.

    IMPORTANT this will die pretty hard if `getExtensionData (LV2_WORKER__interface)`
    returns garbage, so make sure to check that the plugin `hasExtensionData` before
    constructing one of these!
*/
class SharedThreadedWorker final : public WorkerResponseListener
{
public:
    ~SharedThreadedWorker() noexcept override
    {
        shouldExit = true;
        thread.join();
    }

    LV2_Worker_Status schedule (WorkSubmitter submitter,
                                uint32_t size,
                                const void* data)
    {
        return registry.ifContains (submitter.handle, [&]
        {
            return incoming.push (submitter, size, data);
        });
    }

    LV2_Worker_Status responseGenerated (WorkResponder responder,
                                         uint32_t size,
                                         const void* data) override
    {
        return registry.ifContains (responder.handle, [&]
        {
            return outgoing.push (responder, size, data);
        });
    }

    void processResponses()
    {
        for (;;)
        {
            auto workerResponder = outgoing.pop (message);

            if (! message.empty() && workerResponder.isValid())
                workerResponder.processResponse (static_cast<uint32_t> (message.size()), message.data());
            else
                break;
        }
    }

    void registerHandle   (LV2_Handle handle) { registry.insert (handle); }
    void deregisterHandle (LV2_Handle handle) { registry.erase  (handle); }

private:
    static constexpr auto queueSize = 8192;
    std::atomic<bool> shouldExit { false };
    WorkQueue<WorkSubmitter> incoming { queueSize };
    WorkQueue<WorkResponder> outgoing { queueSize };
    std::vector<char> message = std::vector<char> (queueSize);
    std::thread thread { [this]
    {
        std::vector<char> buffer (queueSize);

        while (! shouldExit)
        {
            const auto submitter = incoming.pop (buffer);

            if (! buffer.empty() && submitter.isValid())
                submitter.doWork (Realtime::yes, (uint32_t) buffer.size(), buffer.data());
            else
                std::this_thread::sleep_for (std::chrono::milliseconds (1));
        }
    } };
    HandleRegistry registry;

    JUCE_LEAK_DETECTOR (SharedThreadedWorker)
};

struct HandleHolder
{
    virtual ~HandleHolder() = default;
    virtual LV2_Handle getHandle() const = 0;
    virtual const LV2_Worker_Interface* getWorkerInterface() const = 0;
};

class WorkScheduler
{
public:
    explicit WorkScheduler (HandleHolder& handleHolderIn)
        : handleHolder (handleHolderIn) {}

    void processResponses() { workerThread->processResponses(); }

    LV2_Worker_Schedule& getWorkerSchedule() { return schedule; }

    void setNonRealtime (bool nonRealtime) { realtime = ! nonRealtime; }

    void registerHandle   (LV2_Handle handle) { workerThread->registerHandle   (handle); }
    void deregisterHandle (LV2_Handle handle) { workerThread->deregisterHandle (handle); }

private:
    LV2_Worker_Status scheduleWork (uint32_t size, const void* data)
    {
        WorkSubmitter submitter { handleHolder.getHandle(),
                                  handleHolder.getWorkerInterface(),
                                  workerThread,
                                  &workMutex };

        // If we're in realtime mode, the work should go onto a background thread,
        // and we'll process it later.
        // If we're offline, we can just do the work immediately, without worrying about
        // drop-outs
        return realtime ? workerThread->schedule (submitter, size, data)
                        : submitter.doWork (Realtime::no, size, data);
    }

    static LV2_Worker_Status scheduleWork (LV2_Worker_Schedule_Handle handle,
                                           uint32_t size,
                                           const void* data)
    {
        return static_cast<WorkScheduler*> (handle)->scheduleWork (size, data);
    }

    SharedResourcePointer<SharedThreadedWorker> workerThread;
    HandleHolder& handleHolder;
    LV2_Worker_Schedule schedule { this, scheduleWork };
    CriticalSection workMutex;
    bool realtime = true;

    JUCE_LEAK_DETECTOR (WorkScheduler)
};

struct FeaturesDataListener
{
    virtual ~FeaturesDataListener() = default;
    virtual LV2_Resize_Port_Status resizeCallback (uint32_t index, size_t size) = 0;
};

class Resize
{
public:
    explicit Resize (FeaturesDataListener& l)
        : listener (l) {}

    LV2_Resize_Port_Resize& getFeature() { return resize; }

private:
    LV2_Resize_Port_Status resizeCallback (uint32_t index, size_t size)
    {
        return listener.resizeCallback (index, size);
    }

    static LV2_Resize_Port_Status resizeCallback (LV2_Resize_Port_Feature_Data data, uint32_t index, size_t size)
    {
        return static_cast<Resize*> (data)->resizeCallback (index, size);
    }

    FeaturesDataListener& listener;
    LV2_Resize_Port_Resize resize { this, resizeCallback };
};

class FeaturesData
{
public:
    FeaturesData (HandleHolder& handleHolder,
                  FeaturesDataListener& l,
                  int32_t maxBlockSizeIn,
                  int32_t sequenceSizeIn,
                  const UsefulUrids* u)
        : urids (u),
          resize (l),
          maxBlockSize (maxBlockSizeIn),
          sequenceSize (sequenceSizeIn),
          workScheduler (handleHolder)
    {}

    LV2_Options_Option* getOptions() noexcept { return options.data(); }

    int32_t getMaxBlockSize() const noexcept { return maxBlockSize; }

    void setNonRealtime (bool newValue) { realtime = ! newValue; }

    const LV2_Feature* const* getFeatureArray() const noexcept { return features.pointers.data(); }

    static std::vector<String> getFeatureUris()
    {
        return Features::getUris (makeFeatures ({}, {}, {}, {}, {}, {}));
    }

    void processResponses() { workScheduler.processResponses(); }

    void registerHandle   (LV2_Handle handle) { workScheduler.registerHandle   (handle); }
    void deregisterHandle (LV2_Handle handle) { workScheduler.deregisterHandle (handle); }

private:
    static std::vector<LV2_Feature> makeFeatures (LV2_URID_Map* map,
                                                  LV2_URID_Unmap* unmap,
                                                  LV2_Options_Option* options,
                                                  LV2_Worker_Schedule* schedule,
                                                  LV2_Resize_Port_Resize* resize,
                                                  [[maybe_unused]] LV2_Log_Log* log)
    {
        return { LV2_Feature { LV2_STATE__loadDefaultState,         nullptr },
                 LV2_Feature { LV2_BUF_SIZE__boundedBlockLength,    nullptr },
                 LV2_Feature { LV2_URID__map,                       map },
                 LV2_Feature { LV2_URID__unmap,                     unmap },
                 LV2_Feature { LV2_OPTIONS__options,                options },
                 LV2_Feature { LV2_WORKER__schedule,                schedule },
                 LV2_Feature { LV2_STATE__threadSafeRestore,        nullptr },
                #if JUCE_DEBUG
                 LV2_Feature { LV2_LOG__log,                        log },
                #endif
                 LV2_Feature { LV2_RESIZE_PORT__resize,             resize } };
    }

    LV2_Options_Option makeOption (const char* uid, const int32_t* ptr)
    {
        return { LV2_OPTIONS_INSTANCE,
                 0,                         // INSTANCE kinds must have a subject of 0
                 urids->symap.map (uid),
                 sizeof (int32_t),
                 urids->symap.map (LV2_ATOM__Int),
                 ptr };
    }

    const UsefulUrids* urids;
    Resize resize;
    Log log { urids };

    const int32_t minBlockSize = 0, maxBlockSize = 0, sequenceSize = 0;

    std::vector<LV2_Options_Option> options
    {
        makeOption (LV2_BUF_SIZE__minBlockLength, &minBlockSize),
        makeOption (LV2_BUF_SIZE__maxBlockLength, &maxBlockSize),
        makeOption (LV2_BUF_SIZE__sequenceSize,   &sequenceSize),
        { LV2_OPTIONS_INSTANCE, 0, 0, 0, 0, nullptr }, // The final entry must be nulled out
    };

    WorkScheduler workScheduler;

    LV2_URID_Map        map       = urids->symap.getMapFeature();
    LV2_URID_Unmap      unmap     = urids->symap.getUnmapFeature();
    Features features { makeFeatures (&map,
                                      &unmap,
                                      options.data(),
                                      &workScheduler.getWorkerSchedule(),
                                      &resize.getFeature(),
                                      log.getLogFeature()) };

    bool realtime = true;

    JUCE_LEAK_DETECTOR (FeaturesData)
};

//==============================================================================
struct TryLockAndCall
{
    template <typename Fn>
    void operator() (SpinLock& mutex, Fn&& fn)
    {
        const SpinLock::ScopedTryLockType lock (mutex);

        if (lock.isLocked())
            fn();
    }
};

struct LockAndCall
{
    template <typename Fn>
    void operator() (SpinLock& mutex, Fn&& fn)
    {
        const SpinLock::ScopedLockType lock (mutex);
        fn();
    }
};

struct RealtimeReadTrait
{
    using Read  = TryLockAndCall;
    using Write = LockAndCall;
};

struct RealtimeWriteTrait
{
    using Read  = LockAndCall;
    using Write = TryLockAndCall;
};

struct MessageHeader
{
    uint32_t portIndex;
    uint32_t protocol;
};

template <typename Header>
struct MessageBufferInterface
{
    virtual ~MessageBufferInterface() = default;
    virtual void pushMessage (Header header, uint32_t size, const void* buffer) = 0;
};

template <typename Header, typename LockTraits>
class Messages final : public MessageBufferInterface<Header>
{
    using Read  = typename LockTraits::Read;
    using Write = typename LockTraits::Write;

    struct FullHeader
    {
        Header header;
        uint32_t size;
    };

public:
    Messages() { data.reserve (initialBufferSize); }

    void pushMessage (Header header, uint32_t size, const void* buffer) override
    {
        Write{} (mutex, [&]
        {
            const auto chars = toChars (FullHeader { header, size });
            const auto bufferAsChars = static_cast<const char*> (buffer);
            data.insert (data.end(), chars.begin(), chars.end());
            data.insert (data.end(), bufferAsChars, bufferAsChars + size);
        });
    }

    template <typename Callback>
    void readAllAndClear (Callback&& callback)
    {
        Read{} (mutex, [&]
        {
            if (data.empty())
                return;

            const auto end = data.data() + data.size();

            for (auto ptr = data.data(); ptr < end;)
            {
                const auto header = readUnaligned<FullHeader> (ptr);
                callback (header.header, header.size, ptr + sizeof (header));
                ptr += sizeof (header) + header.size;
            }

            data.clear();
        });
    }

private:
    static constexpr auto initialBufferSize = 8192;
    SpinLock mutex;
    std::vector<char> data;

    JUCE_LEAK_DETECTOR (Messages)
};

//==============================================================================
class LambdaTimer final : private Timer
{
public:
    explicit LambdaTimer (std::function<void()> c) : callback (c) {}

    ~LambdaTimer() noexcept override { stopTimer(); }

    using Timer::startTimer;
    using Timer::startTimerHz;
    using Timer::stopTimer;

private:
    void timerCallback() override { callback(); }

    std::function<void()> callback;
};

struct UiEventListener : public MessageBufferInterface<MessageHeader>
{
    virtual int idle() = 0;
};

struct UiMessageHeader
{
    UiEventListener* listener;
    MessageHeader header;
};

class ProcessorToUi final : public MessageBufferInterface<UiMessageHeader>
{
public:
    ProcessorToUi() { timer.startTimerHz (60); }

    void addUi    (UiEventListener& l)      { JUCE_ASSERT_MESSAGE_THREAD; activeUis.insert (&l); }
    void removeUi (UiEventListener& l)      { JUCE_ASSERT_MESSAGE_THREAD; activeUis.erase (&l); }

    void pushMessage (UiMessageHeader header, uint32_t size, const void* buffer) override
    {
        processorToUi.pushMessage (header, size, buffer);
    }

private:
    Messages<UiMessageHeader, RealtimeWriteTrait> processorToUi;
    std::set<UiEventListener*> activeUis;
    LambdaTimer timer { [this]
    {
        for (auto* l : activeUis)
            if (l->idle() != 0)
                return;

        processorToUi.readAllAndClear ([&] (const UiMessageHeader& header, uint32_t size, const char* data)
        {
            if (activeUis.find (header.listener) != activeUis.cend())
                header.listener->pushMessage (header.header, size, data);
        });
    } };
};

/*  These type identifiers may be used to check the type of the incoming data. */
struct StatefulPortUrids
{
    explicit StatefulPortUrids (SymbolMap& map)
        : Float  (map.map (LV2_ATOM__Float)),
          Double (map.map (LV2_ATOM__Double)),
          Int    (map.map (LV2_ATOM__Int)),
          Long   (map.map (LV2_ATOM__Long))
    {}

    const LV2_URID Float, Double, Int, Long;
};

/*
    A bit like SortedSet, but only requires `operator<` and not `operator==`, so
    it behaves a bit more like a std::set.
*/
template <typename Value>
class SafeSortedSet
{
public:
    using       iterator = typename std::vector<Value>::      iterator;
    using const_iterator = typename std::vector<Value>::const_iterator;

    template <typename Other>
    const_iterator find (const Other& other) const noexcept
    {
        const auto it = std::lower_bound (storage.cbegin(), storage.cend(), other);

        if (it != storage.cend() && ! (other < *it))
            return it;

        return storage.cend();
    }

    void insert (Value&& value)      { insertImpl (std::move (value)); }
    void insert (const Value& value) { insertImpl (value); }

    size_t size()  const noexcept { return storage.size(); }
    bool   empty() const noexcept { return storage.empty(); }

    iterator        begin()       noexcept { return storage. begin(); }
    const_iterator  begin() const noexcept { return storage. begin(); }
    const_iterator cbegin() const noexcept { return storage.cbegin(); }

    iterator          end()       noexcept { return storage. end(); }
    const_iterator    end() const noexcept { return storage. end(); }
    const_iterator   cend() const noexcept { return storage.cend(); }

    auto& operator[] (size_t index) const { return storage[index]; }

private:
    template <typename Arg>
    void insertImpl (Arg&& value)
    {
        const auto it = std::lower_bound (storage.cbegin(), storage.cend(), value);

        if (it == storage.cend() || value < *it)
            storage.insert (it, std::forward<Arg> (value));
    }

    std::vector<Value> storage;
};

struct StoredScalePoint
{
    String label;
    float value;

    bool operator< (const StoredScalePoint& other) const noexcept { return value < other.value; }
};

inline bool operator< (const StoredScalePoint& a, float b) noexcept { return a.value < b; }
inline bool operator< (float a, const StoredScalePoint& b) noexcept { return a < b.value; }

struct ParameterInfo
{
    ParameterInfo() = default;

    ParameterInfo (SafeSortedSet<StoredScalePoint> scalePointsIn,
                   String identifierIn,
                   float defaultValueIn,
                   float minIn,
                   float maxIn,
                   bool isToggleIn,
                   bool isIntegerIn,
                   bool isEnumIn)
        : scalePoints (std::move (scalePointsIn)),
          identifier (std::move (identifierIn)),
          defaultValue (defaultValueIn),
          min (minIn),
          max (maxIn),
          isToggle (isToggleIn),
          isInteger (isIntegerIn),
          isEnum (isEnumIn)
    {}

    static SafeSortedSet<StoredScalePoint> getScalePoints (const Port& port)
    {
        SafeSortedSet<StoredScalePoint> scalePoints;

        for (const LilvScalePoint* p : port.getScalePoints())
        {
            const ScalePoint wrapper { p };
            const auto value = wrapper.getValue();
            const auto label = wrapper.getLabel();

            if (lilv_node_is_float (value) || lilv_node_is_int (value))
                scalePoints.insert ({ lilv_node_as_string (label), lilv_node_as_float (value) });
        }

        return scalePoints;
    }

    static ParameterInfo getInfoForPort (const UsefulUris& uris, const Port& port)
    {
        const auto range = port.getRange();

        return { getScalePoints (port),
                 "sym:" + String::fromUTF8 (port.getSymbol().getTyped()),
                 range.defaultValue,
                 range.min,
                 range.max,
                 port.hasProperty (uris.mLV2_CORE__toggled),
                 port.hasProperty (uris.mLV2_CORE__integer),
                 port.hasProperty (uris.mLV2_CORE__enumeration) };
    }

    SafeSortedSet<StoredScalePoint> scalePoints;

    /*  This is the 'symbol' of a port, or the 'designation' of a parameter without a symbol. */
    String identifier;

    float defaultValue = 0.0f, min = 0.0f, max = 1.0f;
    bool isToggle = false, isInteger = false, isEnum = false;

    JUCE_LEAK_DETECTOR (ParameterInfo)
};

struct PortHeader
{
    String name;
    String symbol;
    uint32_t index;
    Port::Direction direction;
};

struct ControlPort
{
    ControlPort (const PortHeader& headerIn, const ParameterInfo& infoIn)
        : header (headerIn), info (infoIn) {}

    PortHeader header;
    ParameterInfo info;
    float currentValue = info.defaultValue;
};

struct CVPort
{
    PortHeader header;
};

struct AudioPort
{
    PortHeader header;
};

template <size_t Alignment>
class SingleSizeAlignedStorage
{
public:
    SingleSizeAlignedStorage() = default;

    explicit SingleSizeAlignedStorage (size_t sizeInBytes)
        : storage (new char[sizeInBytes + Alignment]),
          alignedPointer (storage.get()),
          space (sizeInBytes + Alignment)
    {
        alignedPointer = std::align (Alignment, sizeInBytes, alignedPointer, space);
    }

    void*  data() const     { return alignedPointer; }
    size_t size() const     { return space; }

private:
    std::unique_ptr<char[]> storage;
    void* alignedPointer = nullptr;
    size_t space = 0;
};

template <size_t Alignment>
static SingleSizeAlignedStorage<Alignment> grow (SingleSizeAlignedStorage<Alignment> storage, size_t size)
{
    if (size <= storage.size())
        return storage;

    SingleSizeAlignedStorage<Alignment> newStorage { jmax (size, (storage.size() * 3) / 2) };
    std::memcpy (newStorage.data(), storage.data(), storage.size());
    return newStorage;
}

enum class SupportsTime { no, yes };

class AtomPort
{
public:
    AtomPort (PortHeader h, size_t bytes, SymbolMap& map, SupportsTime supportsTime)
        : header (h), contents (bytes), forge (map.getMapFeature()), time (supportsTime) {}

    PortHeader header;

    void replaceWithChunk()
    {
        forge.setBuffer (data(), size());
        forge.writeChunk ((uint32_t) (size() - sizeof (LV2_Atom)));
    }

    void replaceBufferWithAtom (const LV2_Atom* atom)
    {
        const auto totalSize = atom->size + sizeof (LV2_Atom);

        if (totalSize <= size())
            std::memcpy (data(), atom, totalSize);
        else
            replaceWithChunk();
    }

    void beginSequence()
    {
        forge.setBuffer (data(), size());
        lv2_atom_forge_sequence_head (forge.get(), &frame, 0);
    }

    void endSequence()
    {
        lv2_atom_forge_pop (forge.get(), &frame);
    }

    /*  For this to work, the 'atom' pointer must be well-formed.

        It must be followed by an atom header, then at least 'size' bytes of body.
    */
    void addAtomToSequence (int64_t timestamp, const LV2_Atom* atom)
    {
        // This reinterpret_cast is not UB, casting to a char* is acceptable.
        // Doing arithmetic on this pointer is dubious, but I can't think of a better alternative
        // given that we don't have any way of knowing the concrete type of the atom.
        addEventToSequence (timestamp,
                            atom->type,
                            atom->size,
                            reinterpret_cast<const char*> (atom) + sizeof (LV2_Atom));
    }

    void addEventToSequence (int64_t timestamp, uint32_t type, uint32_t size, const void* content)
    {
        lv2_atom_forge_frame_time (forge.get(), timestamp);
        lv2_atom_forge_atom (forge.get(), size, type);
        lv2_atom_forge_write (forge.get(), content, size);
    }

    void ensureSizeInBytes (size_t size)
    {
        contents = grow (std::move (contents), size);
    }

          char* data()       noexcept { return data (*this); }
    const char* data() const noexcept { return data (*this); }

    size_t size() const noexcept { return contents.size(); }

          lv2_shared::AtomForge& getForge()       { return forge; }
    const lv2_shared::AtomForge& getForge() const { return forge; }

    bool getSupportsTime() const { return time == SupportsTime::yes; }

private:
    template <typename This>
    static auto data (This& t) -> decltype (t.data())
    {
        return unalignedPointerCast<decltype (t.data())> (t.contents.data());
    }

    // Atoms are required to be 64-bit aligned
    SingleSizeAlignedStorage<8> contents;
    lv2_shared::AtomForge forge;
    LV2_Atom_Forge_Frame frame;
    SupportsTime time = SupportsTime::no;
};

class Plugins
{
public:
    explicit Plugins (const LilvPlugins* list) noexcept : plugins (list) {}

    unsigned size() const noexcept { return lilv_plugins_size (plugins); }

    PluginsIterator begin() const noexcept { return PluginsIterator { plugins }; }
    PluginsIterator end()   const noexcept { return PluginsIterator{}; }

    const LilvPlugin* getByUri (const NodeUri& uri) const
    {
        return lilv_plugins_get_by_uri (plugins, uri.get());
    }

private:
    const LilvPlugins* plugins = nullptr;
};

template <typename PtrTraits>
class PluginClassesImpl
{
public:
    using type = typename PtrTraits::type;

    explicit PluginClassesImpl (type ptr)
            : classes (std::move (ptr)) {}

    unsigned size() const noexcept { return lilv_plugin_classes_size (PtrTraits::get (classes)); }

    PluginClassesIterator begin() const noexcept { return PluginClassesIterator { PtrTraits::get (classes) }; }
    PluginClassesIterator end()   const noexcept { return PluginClassesIterator{}; }

    const LilvPluginClass* getByUri (const NodeUri& uri) const noexcept
    {
        return lilv_plugin_classes_get_by_uri (PtrTraits::get (classes), uri.get());
    }

private:
    type classes{};
};

struct PluginClassesFree
{
    void operator() (LilvPluginClasses* ptr) const noexcept { lilv_plugin_classes_free (ptr); }
};

using OwningPluginClasses    = PluginClassesImpl<OwningPtrTraits<LilvPluginClasses, PluginClassesFree>>;
using NonOwningPluginClasses = PluginClassesImpl<NonOwningPtrTraits<LilvPluginClasses>>;

class World
{
public:
    World() : world (lilv_world_new()) {}

    void loadAllFromPaths (const NodeString& paths)
    {
        lilv_world_set_option (world.get(), LILV_OPTION_LV2_PATH, paths.get());
        lilv_world_load_all (world.get());
    }

    void loadBundle   (const NodeUri& uri)      { lilv_world_load_bundle   (world.get(), uri.get()); }
    void unloadBundle (const NodeUri& uri)      { lilv_world_unload_bundle (world.get(), uri.get()); }

    void loadResource   (const NodeUri& uri)    { lilv_world_load_resource   (world.get(), uri.get()); }
    void unloadResource (const NodeUri& uri)    { lilv_world_unload_resource (world.get(), uri.get()); }

    void loadSpecifications() { lilv_world_load_specifications (world.get()); }
    void loadPluginClasses()  { lilv_world_load_plugin_classes (world.get()); }

    Plugins getAllPlugins()                   const { return Plugins { lilv_world_get_all_plugins (world.get()) }; }
    NonOwningPluginClasses getPluginClasses() const { return NonOwningPluginClasses { lilv_world_get_plugin_classes (world.get()) }; }

    NodeUri newUri (const char* uri)                        { return NodeUri    { world.get(), uri }; }
    NodeUri newFileUri (const char* host, const char* path) { return NodeUri    { world.get(), host, path }; }
    NodeString newString (const char* str)                  { return NodeString { world.get(), str }; }

    bool ask (const LilvNode* subject, const LilvNode* predicate, const LilvNode* object) const
    {
        return lilv_world_ask (world.get(), subject, predicate, object);
    }

    OwningNode get (const LilvNode* subject, const LilvNode* predicate, const LilvNode* object) const
    {
        return OwningNode { lilv_world_get (world.get(), subject, predicate, object) };
    }

    OwningNodes findNodes (const LilvNode* subject, const LilvNode* predicate, const LilvNode* object) const
    {
        return OwningNodes { lilv_world_find_nodes (world.get(), subject, predicate, object) };
    }

    LilvWorld* get() const { return world.get(); }

private:
    struct Free
    {
        void operator() (LilvWorld* ptr) const noexcept { lilv_world_free (ptr); }
    };

    std::unique_ptr<LilvWorld, Free> world;
};

class Ports
{
public:
    static constexpr auto sequenceSize = 8192;

    template <typename Callback>
    void forEachPort (Callback&& callback) const
    {
        for (const auto& port : controlPorts)
            callback (port.header);

        for (const auto& port : cvPorts)
            callback (port.header);

        for (const auto& port : audioPorts)
            callback (port.header);

        for (const auto& port : atomPorts)
            callback (port.header);
    }

    auto getControlPorts()       { return makeSimpleSpan (controlPorts); }
    auto getControlPorts() const { return makeSimpleSpan (controlPorts); }
    auto getCvPorts()            { return makeSimpleSpan (cvPorts); }
    auto getCvPorts()      const { return makeSimpleSpan (cvPorts); }
    auto getAudioPorts()         { return makeSimpleSpan (audioPorts); }
    auto getAudioPorts()   const { return makeSimpleSpan (audioPorts); }
    auto getAtomPorts()          { return makeSimpleSpan (atomPorts); }
    auto getAtomPorts()    const { return makeSimpleSpan (atomPorts); }

    static Optional<Ports> getPorts (World& world, const UsefulUris& uris, const Plugin& plugin, SymbolMap& symap)
    {
        Ports value;
        bool successful = true;

        const auto numPorts = plugin.getNumPorts();
        const auto timeNode = world.newUri (LV2_TIME__Position);

        for (uint32_t i = 0; i != numPorts; ++i)
        {
            const auto port = plugin.getPortByIndex (i);

            const PortHeader header { String::fromUTF8 (port.getName().getTyped()),
                                      String::fromUTF8 (port.getSymbol().getTyped()),
                                      i,
                                      port.getDirection (uris) };

            switch (port.getKind (uris))
            {
                case Port::Kind::control:
                {
                    value.controlPorts.push_back ({ header, ParameterInfo::getInfoForPort (uris, port) });
                    break;
                }

                case Port::Kind::cv:
                    value.cvPorts.push_back ({ header });
                    break;

                case Port::Kind::audio:
                {
                    value.audioPorts.push_back ({ header });
                    break;
                }

                case Port::Kind::atom:
                {
                    const auto supportsTime = port.supportsEvent (timeNode.get());
                    value.atomPorts.push_back ({ header,
                                                 (size_t) Ports::sequenceSize,
                                                 symap,
                                                 supportsTime ? SupportsTime::yes : SupportsTime::no });
                    break;
                }

                case Port::Kind::unknown:
                    successful = false;
                    break;
            }
        }

        for (auto& atomPort : value.atomPorts)
        {
            const auto port    = plugin.getPortByIndex (atomPort.header.index);
            const auto minSize = port.get (uris.mLV2_RESIZE_PORT__minimumSize.get());

            if (minSize != nullptr)
                atomPort.ensureSizeInBytes ((size_t) lilv_node_as_int (minSize.get()));
        }

        return successful ? makeOptional (std::move (value)) : nullopt;
    }

private:
    std::vector<ControlPort> controlPorts;
    std::vector<CVPort> cvPorts;
    std::vector<AudioPort> audioPorts;
    std::vector<AtomPort> atomPorts;
};

class InstanceWithSupports final : private FeaturesDataListener,
                                   private HandleHolder
{
public:
    InstanceWithSupports (World& world,
                          std::unique_ptr<SymbolMap>&& symapIn,
                          const Plugin& plugin,
                          Ports portsIn,
                          int32_t initialBufferSize,
                          double sampleRate)
        : symap (std::move (symapIn)),
          ports (std::move (portsIn)),
          features (*this, *this, initialBufferSize, lv2_host::Ports::sequenceSize, &urids),
          instance (plugin, sampleRate, features.getFeatureArray()),
          workerInterface (instance.getExtensionData<LV2_Worker_Interface> (world.newUri (LV2_WORKER__interface)))
    {
        if (instance == nullptr)
            return;

        for (auto& port : ports.getControlPorts())
            instance.connectPort (port.header.index, &port.currentValue);

        for (auto& port : ports.getAtomPorts())
            instance.connectPort (port.header.index, port.data());

        for (auto& port : ports.getCvPorts())
            instance.connectPort (port.header.index, nullptr);

        for (auto& port : ports.getAudioPorts())
            instance.connectPort (port.header.index, nullptr);

        features.registerHandle (instance.getHandle());
    }

    ~InstanceWithSupports() override
    {
        if (instance != nullptr)
            features.deregisterHandle (instance.getHandle());
    }

    std::unique_ptr<SymbolMap> symap;
    const UsefulUrids urids { *symap };
    Ports ports;
    FeaturesData features;
    Instance instance;
    Messages<MessageHeader, RealtimeReadTrait> uiToProcessor;
    SharedResourcePointer<ProcessorToUi> processorToUi;

private:
    LV2_Handle handle = instance == nullptr ? nullptr : instance.getHandle();
    OptionalExtension<LV2_Worker_Interface> workerInterface;

    LV2_Handle getHandle() const override { return handle; }
    const LV2_Worker_Interface* getWorkerInterface() const override { return workerInterface.valid ? &workerInterface.extension : nullptr; }

    LV2_Resize_Port_Status resizeCallback (uint32_t index, size_t size) override
    {
        if (ports.getAtomPorts().size() <= index)
            return LV2_RESIZE_PORT_ERR_UNKNOWN;

        auto& port = ports.getAtomPorts()[index];

        if (port.header.direction != Port::Direction::output)
            return LV2_RESIZE_PORT_ERR_UNKNOWN;

        port.ensureSizeInBytes (size);
        instance.connectPort (port.header.index, port.data());

        return LV2_RESIZE_PORT_SUCCESS;
    }

    JUCE_DECLARE_NON_COPYABLE (InstanceWithSupports)
    JUCE_DECLARE_NON_MOVEABLE (InstanceWithSupports)
    JUCE_LEAK_DETECTOR (InstanceWithSupports)
};

struct PortState
{
    const void* data;
    uint32_t size;
    uint32_t kind;
};

class PortMap
{
public:
    explicit PortMap (Ports& ports)
    {
        for (auto& port : ports.getControlPorts())
            symbolToControlPortMap.emplace (port.header.symbol, &port);
    }

    PortState getState (const String& symbol, const StatefulPortUrids& urids)
    {
        if (auto* port = getControlPortForSymbol (symbol))
            return { &port->currentValue, sizeof (float), urids.Float };

        // At time of writing, lilv_state_new_from_instance did not attempt to store
        // the state of non-control ports. Perhaps that has changed?
        jassertfalse;
        return { nullptr, 0, 0 };
    }

    void restoreState (const String& symbol, const StatefulPortUrids& urids, PortState ps)
    {
        if (auto* port = getControlPortForSymbol (symbol))
        {
            port->currentValue = [&]() -> float
            {
                if (ps.kind == urids.Float)
                    return getValueFrom<float> (ps.data, ps.size);

                if (ps.kind == urids.Double)
                    return getValueFrom<double> (ps.data, ps.size);

                if (ps.kind == urids.Int)
                    return getValueFrom<int32_t> (ps.data, ps.size);

                if (ps.kind == urids.Long)
                    return getValueFrom<int64_t> (ps.data, ps.size);

                jassertfalse;
                return {};
            }();
        }
        else
            jassertfalse; // Restoring state for non-control ports is not currently supported.
    }

private:
    template <typename Value>
    static float getValueFrom (const void* data, [[maybe_unused]] uint32_t size)
    {
        jassert (size == sizeof (Value));
        return (float) readUnaligned<Value> (data);
    }

    ControlPort* getControlPortForSymbol (const String& symbol) const
    {
        const auto iter = symbolToControlPortMap.find (symbol);
        return iter != symbolToControlPortMap.cend() ? iter->second : nullptr;
    }

    std::map<String, ControlPort*> symbolToControlPortMap;
    JUCE_LEAK_DETECTOR (PortMap)
};

struct FreeString { void operator() (void* ptr) const noexcept { lilv_free (ptr); } };

class PluginState
{
public:
    PluginState() = default;

    explicit PluginState (LilvState* ptr)
        : state (ptr) {}

    const LilvState* get() const noexcept { return state.get(); }

    void restore (InstanceWithSupports& instance, PortMap& portMap) const
    {
        if (state != nullptr)
            SaveRestoreHandle { instance, portMap }.restore (state.get());
    }

    std::string toString (LilvWorld* world, LV2_URID_Map* map, LV2_URID_Unmap* unmap, const char* uri) const
    {
        std::unique_ptr<char, FreeString> result { lilv_state_to_string (world,
                                                                         map,
                                                                         unmap,
                                                                         state.get(),
                                                                         uri,
                                                                         nullptr) };
        return std::string { result.get() };
    }

    String getLabel() const
    {
        return String::fromUTF8 (lilv_state_get_label (state.get()));
    }

    void setLabel (const String& label)
    {
        lilv_state_set_label (state.get(), label.toRawUTF8());
    }

    class SaveRestoreHandle
    {
    public:
        explicit SaveRestoreHandle (InstanceWithSupports& instanceIn, PortMap& portMap)
            : instance (instanceIn.instance.get()),
              features (instanceIn.features.getFeatureArray()),
              urids (*instanceIn.symap),
              map (portMap)
        {}

        PluginState save (const LilvPlugin* plugin, LV2_URID_Map* mapFeature)
        {
            return PluginState { lilv_state_new_from_instance (plugin,
                                                               instance,
                                                               mapFeature,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               getPortValue,
                                                               this,
                                                               LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE,
                                                               features ) };
        }

        void restore (const LilvState* stateIn)
        {
            lilv_state_restore (stateIn,
                                instance,
                                setPortValue,
                                this,
                                0,
                                features);
        }

    private:
        static const void* getPortValue (const char* portSymbol,
                                         void* userData,
                                         uint32_t* size,
                                         uint32_t* type)
        {
            auto& handle = *static_cast<SaveRestoreHandle*> (userData);

            const auto state = handle.map.getState (portSymbol, handle.urids);
            *size = state.size;
            *type = state.kind;
            return state.data;
        }

        static void setPortValue (const char* portSymbol,
                                  void* userData,
                                  const void* value,
                                  uint32_t size,
                                  uint32_t type)
        {
            const auto& handle = *static_cast<const SaveRestoreHandle*> (userData);
            handle.map.restoreState (portSymbol, handle.urids, { static_cast<const char*> (value), size, type });
        }

        LilvInstance* instance = nullptr;
        const LV2_Feature* const* features = nullptr;
        const StatefulPortUrids urids;
        PortMap& map;
    };

private:
    struct Free
    {
        void operator() (LilvState* ptr) const noexcept { lilv_state_free (ptr); }
    };

    std::unique_ptr<LilvState, Free> state;

    JUCE_LEAK_DETECTOR (PluginState)
};

/*
    Wraps an LV2 UI bundle, providing access to the descriptor (if available).
*/
struct UiDescriptorLibrary
{
    using GetDescriptor = LV2UI_Descriptor* (*) (uint32_t);

    UiDescriptorLibrary() = default;

    explicit UiDescriptorLibrary (const String& libraryPath)
        : library (std::make_unique<DynamicLibrary> (libraryPath)),
          getDescriptor (lv2_shared::wordCast<GetDescriptor> (library->getFunction ("lv2ui_descriptor"))) {}

    std::unique_ptr<DynamicLibrary> library;
    GetDescriptor getDescriptor = nullptr;
};

class UiDescriptorArgs
{
public:
    String libraryPath;
    String uiUri;

    auto withLibraryPath (String v) const noexcept { return with (&UiDescriptorArgs::libraryPath, v); }
    auto withUiUri       (String v) const noexcept { return with (&UiDescriptorArgs::uiUri,       v); }

private:
    UiDescriptorArgs with (String UiDescriptorArgs::* member, String value) const noexcept
    {
        return juce::lv2_host::with (*this, member, std::move (value));
    }
};

/*
    Stores a pointer to the descriptor for a specific UI bundle and UI URI.
*/
class UiDescriptor
{
public:
    UiDescriptor() = default;

    explicit UiDescriptor (const UiDescriptorArgs& args)
        : library (args.libraryPath),
          descriptor (extractUiDescriptor (library, args.uiUri.toRawUTF8()))
    {}

    void portEvent (LV2UI_Handle ui,
                    uint32_t portIndex,
                    uint32_t bufferSize,
                    uint32_t format,
                    const void* buffer) const
    {
        JUCE_ASSERT_MESSAGE_THREAD

        if (auto* lv2Descriptor = get())
            if (auto* callback = lv2Descriptor->port_event)
                callback (ui, portIndex, bufferSize, format, buffer);
    }

    bool hasExtensionData (World& world, const char* uid) const
    {
        return world.ask (world.newUri (descriptor->URI).get(),
                          world.newUri (LV2_CORE__extensionData).get(),
                          world.newUri (uid).get());
    }

    template <typename Extension>
    OptionalExtension<Extension> getExtensionData (World& world, const char* uid) const
    {
        if (! hasExtensionData (world, uid))
            return {};

        if (auto* lv2Descriptor = get())
            if (auto* extension = lv2Descriptor->extension_data)
                return OptionalExtension<Extension> (readUnaligned<Extension> (extension (uid)));

        return {};
    }

    const LV2UI_Descriptor* get() const noexcept { return descriptor; }

private:
    static const LV2UI_Descriptor* extractUiDescriptor (const UiDescriptorLibrary& lib, const char* uiUri)
    {
        if (lib.getDescriptor == nullptr)
            return nullptr;

        for (uint32_t i = 0;; ++i)
        {
            const auto* descriptor = lib.getDescriptor (i);

            if (descriptor == nullptr)
                return nullptr;

            if (strcmp (uiUri, descriptor->URI) == 0)
                return descriptor;
        }
    }

    UiDescriptorLibrary library;
    const LV2UI_Descriptor* descriptor = nullptr;

    JUCE_LEAK_DETECTOR (UiDescriptor)
};

enum class Update { no, yes };

/*  A bit like the FlaggedFloatCache used by the VST3 host/client.

    While the FlaggedFloatCache always clears all set flags during the ifSet() call,
    this class stores the "value changed" flags for the processor and UI separately,
    so that they can be read at different rates.
*/
class ParameterValuesAndFlags
{
public:
    ParameterValuesAndFlags() = default;

    explicit ParameterValuesAndFlags (size_t sizeIn)
        : values (sizeIn),
          needsUiUpdate (sizeIn),
          needsProcessorUpdate (sizeIn)
    {
        std::fill (values.begin(), values.end(), 0.0f);
    }

    size_t size() const noexcept { return values.size(); }

    void set (size_t index, float value, Update update)
    {
        jassert (index < size());
        values[index].store (value, std::memory_order_relaxed);
        needsUiUpdate       .set (index, update == Update::yes ? 1 : 0);
        needsProcessorUpdate.set (index, update == Update::yes ? 1 : 0);
    }

    float get (size_t index) const noexcept
    {
        jassert (index < size());
        return values[index].load (std::memory_order_relaxed);
    }

    template <typename Callback>
    void ifProcessorValuesChanged (Callback&& callback)
    {
        ifChanged (needsProcessorUpdate, std::forward<Callback> (callback));
    }

    template <typename Callback>
    void ifUiValuesChanged (Callback&& callback)
    {
        ifChanged (needsUiUpdate, std::forward<Callback> (callback));
    }

    void clearUiFlags() { needsUiUpdate.clear(); }

private:
    template <typename Callback>
    void ifChanged (FlagCache<1>& flags, Callback&& callback)
    {
        flags.ifSet ([this, &callback] (size_t groupIndex, uint32_t)
        {
            callback (groupIndex, values[groupIndex].load (std::memory_order_relaxed));
        });
    }

    std::vector<std::atomic<float>> values;
    FlagCache<1> needsUiUpdate;
    FlagCache<1> needsProcessorUpdate;

    JUCE_LEAK_DETECTOR (ParameterValuesAndFlags)
};

class LV2Parameter : public AudioPluginInstance::HostedParameter
{
public:
    LV2Parameter (const String& nameIn,
                  const ParameterInfo& infoIn,
                  ParameterValuesAndFlags& floatCache)
        : cache (floatCache),
          info (infoIn),
          range (info.min, info.max),
          name (nameIn),
          normalisedDefault (range.convertTo0to1 (infoIn.defaultValue))
    {}

    float getValue() const noexcept override
    {
        return range.convertTo0to1 (getDenormalisedValue());
    }

    void setValue (float f) override
    {
        cache.set ((size_t) getParameterIndex(), range.convertFrom0to1 (f), Update::yes);
    }

    void setDenormalisedValue (float denormalised)
    {
        cache.set ((size_t) getParameterIndex(), denormalised, Update::yes);
        sendValueChangedMessageToListeners (range.convertTo0to1 (denormalised));
    }

    void setDenormalisedValueWithoutTriggeringUpdate (float denormalised)
    {
        cache.set ((size_t) getParameterIndex(), denormalised, Update::no);
        sendValueChangedMessageToListeners (range.convertTo0to1 (denormalised));
    }

    float getDenormalisedValue() const noexcept
    {
        return cache.get ((size_t) getParameterIndex());
    }

    float getDefaultValue() const override { return normalisedDefault; }
    float getDenormalisedDefaultValue() const { return info.defaultValue; }

    float getValueForText (const String& text) const override
    {
        if (! info.isEnum)
            return range.convertTo0to1 (text.getFloatValue());

        const auto it = std::find_if (info.scalePoints.begin(),
                                      info.scalePoints.end(),
                                      [&] (const StoredScalePoint& stored) { return stored.label == text; });
        return it != info.scalePoints.end() ? range.convertTo0to1 (it->value) : normalisedDefault;
    }

    int getNumSteps() const override
    {
        if (info.isToggle)
            return 2;

        if (info.isEnum)
            return static_cast<int> (info.scalePoints.size());

        if (info.isInteger)
            return static_cast<int> (range.getRange().getLength()) + 1;

        return AudioProcessorParameter::getNumSteps();
    }

    bool isDiscrete() const override { return info.isEnum || info.isInteger || info.isToggle; }
    bool isBoolean() const override { return info.isToggle; }

    StringArray getAllValueStrings() const override
    {
        if (! info.isEnum)
            return {};

        return AudioProcessorParameter::getAllValueStrings();
    }

    String getText (float normalisedValue, int) const override
    {
        const auto denormalised = range.convertFrom0to1 (normalisedValue);

        if (info.isEnum && ! info.scalePoints.empty())
        {
            // The normalised value might not correspond to the exact value of a scale point.
            // In this case, we find the closest label by searching the midpoints of the scale
            // point values.
            const auto index = std::distance (midPoints.begin(),
                                              std::lower_bound (midPoints.begin(), midPoints.end(), denormalised));
            jassert (isPositiveAndBelow (index, info.scalePoints.size()));
            return info.scalePoints[(size_t) index].label;
        }

        return getFallbackParameterString (denormalised);
    }

    String getParameterID() const override
    {
        return info.identifier;
    }

    String getName (int maxLength) const override
    {
        return name.substring (0, maxLength);
    }

    String getLabel() const override
    {
        // TODO
        return {};
    }

private:
    String getFallbackParameterString (float denormalised) const
    {
        if (info.isToggle)
            return denormalised > 0.0f ? "On" : "Off";

        if (info.isInteger)
            return String { static_cast<int> (denormalised) };

        return String { denormalised };
    }

    static std::vector<float> findScalePointMidPoints (const SafeSortedSet<StoredScalePoint>& set)
    {
        if (set.size() < 2)
            return {};

        std::vector<float> result;
        result.reserve (set.size() - 1);

        for (auto it = std::next (set.begin()); it != set.end(); ++it)
            result.push_back ((std::prev (it)->value + it->value) * 0.5f);

        jassert (std::is_sorted (result.begin(), result.end()));
        jassert (result.size() + 1 == set.size());
        return result;
    }

    ParameterValuesAndFlags& cache;
    const ParameterInfo info;
    const std::vector<float> midPoints = findScalePointMidPoints (info.scalePoints);
    const NormalisableRange<float> range;
    const String name;
    const float normalisedDefault;

    JUCE_LEAK_DETECTOR (LV2Parameter)
};

class UiInstanceArgs
{
public:
    File bundlePath;
    URL pluginUri;

    auto withBundlePath (File v) const noexcept { return withMember (*this, &UiInstanceArgs::bundlePath, std::move (v)); }
    auto withPluginUri  (URL v)  const noexcept { return withMember (*this, &UiInstanceArgs::pluginUri,  std::move (v)); }
};

static File bundlePathFromUri (const char* uri)
{
    return File { std::unique_ptr<char, FreeString> { lilv_file_uri_parse (uri, nullptr) }.get() };
}

/*
    Creates and holds a UI instance for a plugin with a specific URI, using the provided descriptor.
*/
class UiInstance
{
public:
    UiInstance (World& world,
                const UiDescriptor* descriptorIn,
                const UiInstanceArgs& args,
                const LV2_Feature* const* features,
                MessageBufferInterface<MessageHeader>& messagesIn,
                SymbolMap& map,
                PhysicalResizeListener& rl)
        : descriptor (descriptorIn),
          resizeListener (rl),
          uiToProcessor (messagesIn),
          mLV2_UI__floatProtocol   (map.map (LV2_UI__floatProtocol)),
          mLV2_ATOM__atomTransfer  (map.map (LV2_ATOM__atomTransfer)),
          mLV2_ATOM__eventTransfer (map.map (LV2_ATOM__eventTransfer)),
          instance (makeInstance (args, features)),
          idleCallback (getExtensionData<LV2UI_Idle_Interface> (world, LV2_UI__idleInterface))
    {
        jassert (descriptor != nullptr);
        jassert (widget != nullptr);

        ignoreUnused (resizeListener);
    }

    LV2UI_Handle getHandle() const noexcept { return instance.get(); }

    void pushMessage (MessageHeader header, uint32_t size, const void* buffer)
    {
        descriptor->portEvent (getHandle(), header.portIndex, size, header.protocol, buffer);
    }

    int idle()
    {
        if (idleCallback.valid && idleCallback.extension.idle != nullptr)
            return idleCallback.extension.idle (getHandle());

        return 0;
    }

    template <typename Extension>
    OptionalExtension<Extension> getExtensionData (World& world, const char* uid) const
    {
        return descriptor->getExtensionData<Extension> (world, uid);
    }

    Rectangle<int> getDetectedViewBounds() const
    {
       #if JUCE_MAC
        const auto frame = [(NSView*) widget frame];
        return { (int) frame.size.width, (int) frame.size.height };
       #elif JUCE_LINUX || JUCE_BSD
        Window root = 0;
        int wx = 0, wy = 0;
        unsigned int ww = 0, wh = 0, bw = 0, bitDepth = 0;

        XWindowSystemUtilities::ScopedXLock xLock;
        auto* display = XWindowSystem::getInstance()->getDisplay();
        X11Symbols::getInstance()->xGetGeometry (display,
                                                 (::Drawable) widget,
                                                 &root,
                                                 &wx,
                                                 &wy,
                                                 &ww,
                                                 &wh,
                                                 &bw,
                                                 &bitDepth);

        return { (int) ww, (int) wh };
       #elif JUCE_WINDOWS
        RECT rect;
        GetWindowRect ((HWND) widget, &rect);
        return { rect.right - rect.left, rect.bottom - rect.top };
       #else
        return {};
       #endif
    }

    const UiDescriptor* descriptor = nullptr;

private:
    using Instance = std::unique_ptr<void, void (*) (LV2UI_Handle)>;
    using Idle = int (*) (LV2UI_Handle);

    Instance makeInstance (const UiInstanceArgs& args, const LV2_Feature* const* features)
    {
        if (descriptor->get() == nullptr)
            return { nullptr, [] (LV2UI_Handle) {} };

        return Instance { descriptor->get()->instantiate (descriptor->get(),
                                                          args.pluginUri.toString (true).toRawUTF8(),
                                                          File::addTrailingSeparator (args.bundlePath.getFullPathName()).toRawUTF8(),
                                                          writeFunction,
                                                          this,
                                                          &widget,
                                                          features),
                          descriptor->get()->cleanup };
    }

    void write (uint32_t portIndex, uint32_t bufferSize, uint32_t protocol, const void* buffer)
    {
        const LV2_URID protocols[] { 0, mLV2_UI__floatProtocol, mLV2_ATOM__atomTransfer, mLV2_ATOM__eventTransfer };
        const auto it = std::find (std::begin (protocols), std::end (protocols), protocol);

        if (it != std::end (protocols))
        {
            uiToProcessor.pushMessage ({ portIndex, protocol }, bufferSize, buffer);
        }
    }

    static void writeFunction (LV2UI_Controller controller,
                               uint32_t portIndex,
                               uint32_t bufferSize,
                               uint32_t portProtocol,
                               const void* buffer)
    {
        jassert (controller != nullptr);
        static_cast<UiInstance*> (controller)->write (portIndex, bufferSize, portProtocol, buffer);
    }

    PhysicalResizeListener& resizeListener;
    MessageBufferInterface<MessageHeader>& uiToProcessor;
    LV2UI_Widget widget = nullptr;
    const LV2_URID mLV2_UI__floatProtocol;
    const LV2_URID mLV2_ATOM__atomTransfer;
    const LV2_URID mLV2_ATOM__eventTransfer;
    Instance instance;
    OptionalExtension<LV2UI_Idle_Interface> idleCallback;

   #if JUCE_MAC
    NSViewFrameWatcher frameWatcher { (NSView*) widget, [this]
    {
        const auto bounds = getDetectedViewBounds();
        resizeListener.viewRequestedResizeInPhysicalPixels (bounds.getWidth(), bounds.getHeight());
    } };
   #elif JUCE_WINDOWS
    WindowSizeChangeListener frameWatcher { (HWND) widget, resizeListener };
   #endif

    JUCE_LEAK_DETECTOR (UiInstance)
};

struct TouchListener
{
    virtual ~TouchListener() = default;
    virtual void controlGrabbed (uint32_t port, bool grabbed) = 0;
};

class AsyncFn final : public AsyncUpdater
{
public:
    explicit AsyncFn (std::function<void()> callbackIn)
        : callback (std::move (callbackIn)) {}

    ~AsyncFn() override { cancelPendingUpdate(); }

    void handleAsyncUpdate() override { callback(); }

private:
    std::function<void()> callback;
};

class UiFeaturesDataOptions
{
public:
    float initialScaleFactor = 0.0f, sampleRate = 0.0f;

    auto withInitialScaleFactor (float v) const { return with (&UiFeaturesDataOptions::initialScaleFactor, v); }
    auto withSampleRate         (float v) const { return with (&UiFeaturesDataOptions::sampleRate,         v); }

private:
    UiFeaturesDataOptions with (float UiFeaturesDataOptions::* member, float value) const
    {
        return juce::lv2_host::with (*this, member, value);
    }
};

class UiFeaturesData
{
public:
    UiFeaturesData (PhysicalResizeListener& rl,
                    TouchListener& tl,
                    LV2_Handle instanceIn,
                    LV2UI_Widget parentIn,
                    Instance::GetExtensionData getExtensionData,
                    const Ports& ports,
                    SymbolMap& symapIn,
                    const UiFeaturesDataOptions& optIn)
        : opts (optIn),
          resizeListener (rl),
          touchListener (tl),
          instance (instanceIn),
          parent (parentIn),
          symap (symapIn),
          dataAccess { getExtensionData },
          portIndices (makePortIndices (ports))
    {
    }

    const LV2_Feature* const* getFeatureArray() const noexcept { return features.pointers.data(); }

    static std::vector<String> getFeatureUris()
    {
        return Features::getUris (makeFeatures ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}));
    }

    Rectangle<int> getLastRequestedBounds() const   { return { lastRequestedWidth, lastRequestedHeight }; }

private:
    static std::vector<LV2_Feature> makeFeatures (LV2UI_Resize* resize,
                                                  LV2UI_Widget parent,
                                                  LV2_Handle handle,
                                                  LV2_Extension_Data_Feature* data,
                                                  LV2_URID_Map* map,
                                                  LV2_URID_Unmap* unmap,
                                                  LV2UI_Port_Map* portMap,
                                                  LV2UI_Touch* touch,
                                                  LV2_Options_Option* options,
                                                  LV2_Log_Log* log)
    {
        return { LV2_Feature { LV2_UI__resize,          resize },
                 LV2_Feature { LV2_UI__parent,          parent },
                 LV2_Feature { LV2_UI__idleInterface,   nullptr },
                 LV2_Feature { LV2_INSTANCE_ACCESS_URI, handle },
                 LV2_Feature { LV2_DATA_ACCESS_URI,     data },
                 LV2_Feature { LV2_URID__map,           map },
                 LV2_Feature { LV2_URID__unmap,         unmap},
                 LV2_Feature { LV2_UI__portMap,         portMap },
                 LV2_Feature { LV2_UI__touch,           touch },
                 LV2_Feature { LV2_OPTIONS__options,    options },
                 LV2_Feature { LV2_LOG__log,            log } };
    }

    int resizeCallback (int width, int height)
    {
        lastRequestedWidth = width;
        lastRequestedHeight = height;
        resizeListener.viewRequestedResizeInPhysicalPixels (width, height);
        return 0;
    }

    static int resizeCallback (LV2UI_Feature_Handle handle, int width, int height)
    {
        return static_cast<UiFeaturesData*> (handle)->resizeCallback (width, height);
    }

    uint32_t portIndexCallback (const char* symbol) const
    {
        const auto it = portIndices.find (symbol);
        return it != portIndices.cend() ? it->second : LV2UI_INVALID_PORT_INDEX;
    }

    static uint32_t portIndexCallback (LV2UI_Feature_Handle handle, const char* symbol)
    {
        return static_cast<const UiFeaturesData*> (handle)->portIndexCallback (symbol);
    }

    void touchCallback (uint32_t portIndex, bool grabbed) const
    {
        touchListener.controlGrabbed (portIndex, grabbed);
    }

    static void touchCallback (LV2UI_Feature_Handle handle, uint32_t index, bool b)
    {
        return static_cast<const UiFeaturesData*> (handle)->touchCallback (index, b);
    }

    static std::map<String, uint32_t> makePortIndices (const Ports& ports)
    {
        std::map<String, uint32_t> result;

        ports.forEachPort ([&] (const PortHeader& header)
        {
            [[maybe_unused]] const auto emplaced = result.emplace (header.symbol, header.index);

            // This will complain if there are duplicate port symbols.
            jassert (emplaced.second);
        });

        return result;
    }

    const UiFeaturesDataOptions opts;
    PhysicalResizeListener& resizeListener;
    TouchListener& touchListener;
    LV2_Handle instance{};
    LV2UI_Widget parent{};
    SymbolMap& symap;
    const UsefulUrids urids { symap };
    Log log { &urids };
    int lastRequestedWidth = 0, lastRequestedHeight = 0;
    std::vector<LV2_Options_Option> options { { LV2_OPTIONS_INSTANCE,
                                                0,
                                                symap.map (LV2_UI__scaleFactor),
                                                sizeof (float),
                                                symap.map (LV2_ATOM__Float),
                                                &opts.initialScaleFactor },
                                              { LV2_OPTIONS_INSTANCE,
                                                0,
                                                symap.map (LV2_PARAMETERS__sampleRate),
                                                sizeof (float),
                                                symap.map (LV2_ATOM__Float),
                                                &opts.sampleRate },
                                              { LV2_OPTIONS_INSTANCE, 0, 0, 0, 0, nullptr } }; // The final entry must be nulled out
    LV2UI_Resize resize { this, resizeCallback };
    LV2_URID_Map map        = symap.getMapFeature();
    LV2_URID_Unmap unmap    = symap.getUnmapFeature();
    LV2UI_Port_Map portMap { this, portIndexCallback };
    LV2UI_Touch touch { this, touchCallback };
    LV2_Extension_Data_Feature dataAccess;
    std::map<String, uint32_t> portIndices;
    Features features { makeFeatures (&resize,
                                      parent,
                                      instance,
                                      &dataAccess,
                                      &map,
                                      &unmap,
                                      &portMap,
                                      &touch,
                                      options.data(),
                                      log.getLogFeature()) };

    JUCE_LEAK_DETECTOR (UiFeaturesData)
};

class UiInstanceWithSupports
{
public:
    UiInstanceWithSupports (World& world,
                            PhysicalResizeListener& resizeListener,
                            TouchListener& touchListener,
                            const UiDescriptor* descriptor,
                            const UiInstanceArgs& args,
                            LV2UI_Widget parent,
                            InstanceWithSupports& engineInstance,
                            const UiFeaturesDataOptions& opts)
        : features (resizeListener,
                    touchListener,
                    engineInstance.instance.getHandle(),
                    parent,
                    engineInstance.instance.getExtensionDataCallback(),
                    engineInstance.ports,
                    *engineInstance.symap,
                    opts),
          instance (world,
                    descriptor,
                    args,
                    features.getFeatureArray(),
                    engineInstance.uiToProcessor,
                    *engineInstance.symap,
                    resizeListener)
    {}

    UiFeaturesData features;
    UiInstance instance;

    JUCE_LEAK_DETECTOR (UiInstanceWithSupports)
};

struct RequiredFeatures
{
    explicit RequiredFeatures (OwningNodes nodes)
        : values (std::move (nodes)) {}

    OwningNodes values;
};

struct OptionalFeatures
{
    explicit OptionalFeatures (OwningNodes nodes)
        : values (std::move (nodes)) {}

    OwningNodes values;
};

template <typename Range, typename Predicate>
static bool noneOf (Range&& range, Predicate&& pred)
{
    // Not a mistake, this is for ADL
    using std::begin;
    using std::end;
    return std::none_of (begin (range), end (range), std::forward<Predicate> (pred));
}

class PeerChangedListener final : private ComponentMovementWatcher
{
public:
    PeerChangedListener (Component& c, std::function<void()> peerChangedIn)
        : ComponentMovementWatcher (&c), peerChanged (std::move (peerChangedIn))
    {
    }

    void componentMovedOrResized (bool, bool) override {}
    void componentPeerChanged() override { NullCheckedInvocation::invoke (peerChanged); }
    void componentVisibilityChanged() override {}

    using ComponentMovementWatcher::componentVisibilityChanged;
    using ComponentMovementWatcher::componentMovedOrResized;

private:
    std::function<void()> peerChanged;
};

struct ViewSizeListener final : private ComponentMovementWatcher
{
    ViewSizeListener (Component& c, PhysicalResizeListener& l)
        : ComponentMovementWatcher (&c), listener (l)
    {
    }

    void componentMovedOrResized (bool, bool wasResized) override
    {
        if (wasResized)
        {
            const auto physicalSize = Desktop::getInstance().getDisplays()
                                                            .logicalToPhysical (getComponent()->localAreaToGlobal (getComponent()->getLocalBounds()));
            const auto width  = physicalSize.getWidth();
            const auto height = physicalSize.getHeight();

            if (width > 10 && height > 10)
                listener.viewRequestedResizeInPhysicalPixels (width, height);
        }
    }

    void componentPeerChanged() override {}
    void componentVisibilityChanged() override {}

    using ComponentMovementWatcher::componentVisibilityChanged;
    using ComponentMovementWatcher::componentMovedOrResized;

    PhysicalResizeListener& listener;
};

class ConfiguredEditorComponent final : public Component,
                                        private PhysicalResizeListener
{
public:
    ConfiguredEditorComponent (World& world,
                               InstanceWithSupports& instance,
                               UiDescriptor& uiDescriptor,
                               LogicalResizeListener& resizeListenerIn,
                               TouchListener& touchListener,
                               const String& uiBundleUri,
                               const UiFeaturesDataOptions& opts)
        : resizeListener (resizeListenerIn),
          floatUrid (instance.symap->map (LV2_ATOM__Float)),
          scaleFactorUrid (instance.symap->map (LV2_UI__scaleFactor)),
          uiInstance (new UiInstanceWithSupports (world,
                                                  *this,
                                                  touchListener,
                                                  &uiDescriptor,
                                                  UiInstanceArgs{}.withBundlePath (bundlePathFromUri (uiBundleUri.toRawUTF8()))
                                                                  .withPluginUri (URL (instance.instance.getUri())),
                                                  viewComponent.getWidget(),
                                                  instance,
                                                  opts)),
          resizeClient (uiInstance->instance.getExtensionData<LV2UI_Resize> (world, LV2_UI__resize)),
          optionsInterface (uiInstance->instance.getExtensionData<LV2_Options_Interface> (world, LV2_OPTIONS__interface))
    {
        jassert (uiInstance != nullptr);

        setOpaque (true);
        addAndMakeVisible (viewComponent);

        const auto boundsToUse = [&]
        {
            const auto requested = uiInstance->features.getLastRequestedBounds();

            if (requested.getWidth() > 10 && requested.getHeight() > 10)
                return requested;

            return uiInstance->instance.getDetectedViewBounds();
        }();

        const auto scaled = lv2ToComponentRect (boundsToUse);
        lastWidth  = scaled.getWidth();
        lastHeight = scaled.getHeight();
        setSize (lastWidth, lastHeight);
    }

    ~ConfiguredEditorComponent() override
    {
        viewComponent.prepareForDestruction();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        viewComponent.setBounds (getLocalBounds());
    }

    void updateViewBounds()
    {
        // If the editor changed size as a result of a request from the client,
        // we shouldn't send a notification back to the client.
        if (uiInstance != nullptr)
        {
            if (resizeClient.valid && resizeClient.extension.ui_resize != nullptr)
            {
                const auto physicalSize = componentToLv2Rect (getLocalBounds());

                resizeClient.extension.ui_resize (uiInstance->instance.getHandle(),
                                                  physicalSize.getWidth(),
                                                  physicalSize.getHeight());
            }
        }
    }

    void pushMessage (MessageHeader header, uint32_t size, const void* buffer)
    {
        if (uiInstance != nullptr)
            uiInstance->instance.pushMessage (header, size, buffer);
    }

    int idle()
    {
        if (uiInstance != nullptr)
            return uiInstance->instance.idle();

        return 0;
    }

    void childBoundsChanged (Component* c) override
    {
        if (c == nullptr)
            resizeToFitView();
    }

    void setUserScaleFactor (float userScale) { userScaleFactor = userScale; }

    void sendScaleFactorToPlugin()
    {
        const auto factor = getEffectiveScale();

        const LV2_Options_Option options[]
        {
            { LV2_OPTIONS_INSTANCE, 0, scaleFactorUrid, sizeof (float), floatUrid, &factor },
            { {}, {}, {}, {}, {}, {} }
        };

        if (optionsInterface.valid)
            optionsInterface.extension.set (uiInstance->instance.getHandle(), options);

        applyLastRequestedPhysicalSize();
    }

private:
    void viewRequestedResizeInPhysicalPixels (int width, int height) override
    {
        lastWidth = width;
        lastHeight = height;
        const auto logical = lv2ToComponentRect ({ width, height });
        resizeListener.viewRequestedResizeInLogicalPixels (logical.getWidth(), logical.getHeight());
    }

    void resizeToFitView()
    {
        viewComponent.fitToView();
        resizeListener.viewRequestedResizeInLogicalPixels (viewComponent.getWidth(), viewComponent.getHeight());
    }

    void applyLastRequestedPhysicalSize()
    {
        viewRequestedResizeInPhysicalPixels (lastWidth, lastHeight);
        viewComponent.forceViewToSize();
    }

    /*  Convert from the component's coordinate system to the hosted LV2's coordinate system. */
    Rectangle<int> componentToLv2Rect (Rectangle<int> r) const
    {
        return localAreaToGlobal (r) * nativeScaleFactor * getDesktopScaleFactor();
    }

    /*  Convert from the hosted LV2's coordinate system to the component's coordinate system. */
    Rectangle<int> lv2ToComponentRect (Rectangle<int> vr) const
    {
        return getLocalArea (nullptr, vr / (nativeScaleFactor * getDesktopScaleFactor()));
    }

    float getEffectiveScale() const     { return nativeScaleFactor * userScaleFactor; }

    // If possible, try to keep platform-specific handing restricted to the implementation of
    // ViewComponent. Keep the interface of ViewComponent consistent on all platforms.
   #if JUCE_LINUX || JUCE_BSD
    struct InnerHolder
    {
        struct Inner final : public XEmbedComponent
        {
            Inner() : XEmbedComponent (true, true)
            {
                setOpaque (true);
                addToDesktop (0);
            }
        };

        Inner inner;
    };

    struct ViewComponent final : public InnerHolder,
                                 public XEmbedComponent
    {
        explicit ViewComponent (PhysicalResizeListener& l)
            : XEmbedComponent ((unsigned long) inner.getPeer()->getNativeHandle(), true, false),
              listener (inner, l)
        {
            setOpaque (true);
        }

        ~ViewComponent()
        {
            removeClient();
        }

        void prepareForDestruction()
        {
            inner.removeClient();
        }

        LV2UI_Widget getWidget() { return lv2_shared::wordCast<LV2UI_Widget> (inner.getHostWindowID()); }
        void forceViewToSize() {}
        void fitToView() {}

        ViewSizeListener listener;
    };
   #elif JUCE_MAC
    struct ViewComponent final : public NSViewComponentWithParent
    {
        explicit ViewComponent (PhysicalResizeListener&)
            : NSViewComponentWithParent (WantsNudge::no) {}
        LV2UI_Widget getWidget() { return getView(); }
        void forceViewToSize() {}
        void fitToView() { resizeToFitView(); }
        void prepareForDestruction() {}
    };
   #elif JUCE_WINDOWS
    struct ViewComponent final : public HWNDComponent
    {
        explicit ViewComponent (PhysicalResizeListener&)
        {
            setOpaque (true);
            inner.addToDesktop (0);

            if (auto* peer = inner.getPeer())
                setHWND (peer->getNativeHandle());
        }

        void paint (Graphics& g) override { g.fillAll (Colours::black); }

        LV2UI_Widget getWidget() { return getHWND(); }

        void forceViewToSize() { updateHWNDBounds(); }
        void fitToView() { resizeToFit(); }

        void prepareForDestruction() {}

    private:
        struct Inner final : public Component
        {
            Inner() { setOpaque (true); }
            void paint (Graphics& g) override { g.fillAll (Colours::black); }
        };

        Inner inner;
    };
   #else
    struct ViewComponent final : public Component
    {
        explicit ViewComponent (PhysicalResizeListener&) {}
        void* getWidget() { return nullptr; }
        void forceViewToSize() {}
        void fitToView() {}
        void prepareForDestruction() {}
    };
   #endif

    struct ScaleNotifierCallback
    {
        ConfiguredEditorComponent& window;

        void operator() (float platformScale) const
        {
            MessageManager::callAsync ([ref = Component::SafePointer<ConfiguredEditorComponent> (&window), platformScale]
            {
                if (auto* r = ref.getComponent())
                {
                    if (approximatelyEqual (std::exchange (r->nativeScaleFactor, platformScale), platformScale))
                        return;

                    r->nativeScaleFactor = platformScale;
                    r->sendScaleFactorToPlugin();
                }
            });
        }
    };

    LogicalResizeListener& resizeListener;
    int lastWidth = 0, lastHeight = 0;
    float nativeScaleFactor = 1.0f, userScaleFactor = 1.0f;
    NativeScaleFactorNotifier scaleNotifier { this, ScaleNotifierCallback { *this } };
    ViewComponent viewComponent { *this };
    LV2_URID floatUrid, scaleFactorUrid;
    std::unique_ptr<UiInstanceWithSupports> uiInstance;
    OptionalExtension<LV2UI_Resize> resizeClient;
    OptionalExtension<LV2_Options_Interface> optionsInterface;
    PeerChangedListener peerListener { *this, [this]
    {
        applyLastRequestedPhysicalSize();
    } };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfiguredEditorComponent)
};

//==============================================================================
/*  Interface to receive notifications when the Editor changes. */
struct EditorListener
{
    virtual ~EditorListener() = default;

    /*  The editor needs to be recreated in a few different scenarios, such as:
        - When the scale factor of the window changes, because we can only provide the
          scale factor to the view during construction
        - When the sample rate changes, because the processor also needs to be destroyed
          and recreated in this case

        This function will be called whenever the editor has been recreated, in order to
        allow the processor (or other listeners) to respond, e.g. by sending all of the
        current port/parameter values to the view.
    */
    virtual void viewCreated (UiEventListener* newListener) = 0;

    virtual void notifyEditorBeingDeleted() = 0;
};

/*  We can't pass the InstanceWithSupports directly to the editor, because
    it might be destroyed and reconstructed if the sample rate changes.
*/
struct InstanceProvider
{
    virtual ~InstanceProvider() noexcept = default;

    virtual InstanceWithSupports* getInstanceWithSupports() const = 0;
};

class Editor final : public AudioProcessorEditor,
                     public UiEventListener,
                     private LogicalResizeListener
{
public:
    Editor (World& worldIn,
            AudioPluginInstance& p,
            InstanceProvider& instanceProviderIn,
            UiDescriptor& uiDescriptorIn,
            TouchListener& touchListenerIn,
            EditorListener& listenerIn,
            const String& uiBundleUriIn,
            RequiredFeatures requiredIn,
            OptionalFeatures optionalIn)
        : AudioProcessorEditor (p),
          world (worldIn),
          instanceProvider (&instanceProviderIn),
          uiDescriptor (&uiDescriptorIn),
          touchListener (&touchListenerIn),
          listener (&listenerIn),
          uiBundleUri (uiBundleUriIn),
          required (std::move (requiredIn)),
          optional (std::move (optionalIn))
    {
        setResizable (isResizable (required, optional), false);
        setSize (10, 10);
        setOpaque (true);

        createView();

        instanceProvider->getInstanceWithSupports()->processorToUi->addUi (*this);
    }

    ~Editor() noexcept override
    {
        instanceProvider->getInstanceWithSupports()->processorToUi->removeUi (*this);

        listener->notifyEditorBeingDeleted();
    }

    void createView()
    {
        const auto initialScale = userScaleFactor * (float) [&]
        {
            if (auto* p = getPeer())
                return p->getPlatformScaleFactor();

            return 1.0;
        }();

        const auto opts = UiFeaturesDataOptions{}.withInitialScaleFactor (initialScale)
                                                 .withSampleRate ((float) processor.getSampleRate());
        configuredEditor = nullptr;
        configuredEditor = rawToUniquePtr (new ConfiguredEditorComponent (world,
                                                                          *instanceProvider->getInstanceWithSupports(),
                                                                          *uiDescriptor,
                                                                          *this,
                                                                          *touchListener,
                                                                          uiBundleUri,
                                                                          opts));
        parentHierarchyChanged();
        const auto initialSize = configuredEditor->getBounds();
        setSize (initialSize.getWidth(), initialSize.getHeight());

        listener->viewCreated (this);
    }

    void destroyView()
    {
        configuredEditor = nullptr;
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        const ScopedValueSetter<bool> scope (resizeFromHost, true);

        if (auto* inner = configuredEditor.get())
        {
            inner->setBounds (getLocalBounds());
            inner->updateViewBounds();
        }
    }

    void parentHierarchyChanged() override
    {
        if (auto* comp = configuredEditor.get())
        {
            if (isShowing())
                addAndMakeVisible (comp);
            else
                removeChildComponent (comp);
        }
    }

    void pushMessage (MessageHeader header, uint32_t size, const void* buffer) override
    {
        if (auto* comp = configuredEditor.get())
            comp->pushMessage (header, size, buffer);
    }

    int idle() override
    {
        if (auto* comp = configuredEditor.get())
            return comp->idle();

        return 0;
    }

    void setScaleFactor (float newScale) override
    {
        userScaleFactor = newScale;

        if (configuredEditor != nullptr)
        {
            configuredEditor->setUserScaleFactor (userScaleFactor);
            configuredEditor->sendScaleFactorToPlugin();
        }
    }

private:
    bool isResizable (const RequiredFeatures& requiredFeatures,
                      const OptionalFeatures& optionalFeatures) const
    {
        const auto uriMatches = [] (const LilvNode* node)
        {
            const auto* uri = lilv_node_as_uri (node);
            return std::strcmp (uri, LV2_UI__noUserResize) == 0;
        };

        return uiDescriptor->hasExtensionData (world, LV2_UI__resize)
               && ! uiDescriptor->hasExtensionData (world, LV2_UI__noUserResize)
               && noneOf (requiredFeatures.values, uriMatches)
               && noneOf (optionalFeatures.values, uriMatches);
    }

    bool isScalable() const
    {
        return uiDescriptor->hasExtensionData (world, LV2_OPTIONS__interface);
    }

    void viewRequestedResizeInLogicalPixels (int width, int height) override
    {
        if (! resizeFromHost)
            setSize (width, height);
    }

    World& world;
    InstanceProvider* instanceProvider;
    UiDescriptor* uiDescriptor;
    TouchListener* touchListener;
    EditorListener* listener;
    String uiBundleUri;
    const RequiredFeatures required;
    const OptionalFeatures optional;
    std::unique_ptr<ConfiguredEditorComponent> configuredEditor;
    float userScaleFactor = 1.0f;
    bool resizeFromHost = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
};

class Uis
{
public:
    explicit Uis (const LilvPlugin* plugin) noexcept : uis (lilv_plugin_get_uis (plugin)) {}

    unsigned size() const noexcept { return lilv_uis_size (uis.get()); }

    UisIterator begin() const noexcept { return UisIterator { uis.get() }; }
    UisIterator end()   const noexcept { return UisIterator{}; }

    const LilvUI* getByUri (const NodeUri& uri) const
    {
        return lilv_uis_get_by_uri (uis.get(), uri.get());
    }

private:
    struct Free
    {
        void operator() (LilvUIs* ptr) const noexcept { lilv_uis_free (ptr); }
    };

    std::unique_ptr<LilvUIs, Free> uis;
};

//==============================================================================
class PluginClass
{
public:
    explicit PluginClass (const LilvPluginClass* c) : pluginClass (c) {}

    NodeUri getParentUri() const noexcept   { return NodeUri::copy (lilv_plugin_class_get_parent_uri (pluginClass)); }
    NodeUri getUri() const noexcept         { return NodeUri::copy (lilv_plugin_class_get_uri (pluginClass)); }
    NodeString getLabel() const noexcept    { return NodeString::copy (lilv_plugin_class_get_label (pluginClass)); }
    OwningPluginClasses getChildren() const noexcept
    {
        return OwningPluginClasses { OwningPluginClasses::type { lilv_plugin_class_get_children (pluginClass) } };
    }

private:
    const LilvPluginClass* pluginClass = nullptr;
};

using FloatWriter = void (*) (LV2_Atom_Forge*, float);

struct ParameterWriterUrids
{
    LV2_URID mLV2_PATCH__Set;
    LV2_URID mLV2_PATCH__property;
    LV2_URID mLV2_PATCH__value;
    LV2_URID mLV2_ATOM__eventTransfer;
};

struct MessageHeaderAndSize
{
    MessageHeader header;
    uint32_t size;
};

class ParameterWriter
{
public:
    ParameterWriter (ControlPort* p)
        : data (PortBacking { p }), kind (Kind::port) {}

    ParameterWriter (FloatWriter write, LV2_URID urid, uint32_t controlPortIndex)
        : data (PatchBacking { write, urid, controlPortIndex }), kind (Kind::patch) {}

    void writeToProcessor (const ParameterWriterUrids urids, LV2_Atom_Forge* forge, float value) const
    {
        switch (kind)
        {
            case Kind::patch:
            {
                if (forge != nullptr)
                {
                    lv2_atom_forge_frame_time (forge, 0);
                    writeSetToForge (urids, *forge, value);
                }

                break;
            }

            case Kind::port:
                data.port.port->currentValue = value;
                break;
        }
    }

    MessageHeaderAndSize writeToUi (const ParameterWriterUrids urids, LV2_Atom_Forge& forge, float value) const
    {
        const auto getWrittenBytes = [&]() -> uint32_t
        {
            if (const auto* atom = convertToAtomPtr (forge.buf, forge.size))
                return (uint32_t) (atom->size + sizeof (LV2_Atom));

            jassertfalse;
            return 0;
        };

        switch (kind)
        {
            case Kind::patch:
                writeSetToForge (urids, forge, value);
                return { { data.patch.controlPortIndex, urids.mLV2_ATOM__eventTransfer }, getWrittenBytes() };

            case Kind::port:
                lv2_atom_forge_raw (&forge, &value, sizeof (value));
                return { { data.port.port->header.index, 0 }, sizeof (value) };
        }

        return { { 0, 0 }, 0 };
    }

    const LV2_URID* getUrid() const
    {
        return kind == Kind::patch ? &data.patch.urid : nullptr;
    }

    const uint32_t* getPortIndex() const
    {
        return kind == Kind::port ? &data.port.port->header.index : nullptr;
    }

private:
    void writeSetToForge (const ParameterWriterUrids urids, LV2_Atom_Forge& forge, float value) const
    {
        lv2_shared::ObjectFrame object { &forge, (uint32_t) 0, urids.mLV2_PATCH__Set };

        lv2_atom_forge_key (&forge, urids.mLV2_PATCH__property);
        lv2_atom_forge_urid (&forge, data.patch.urid);

        lv2_atom_forge_key (&forge, urids.mLV2_PATCH__value);
        data.patch.write (&forge, value);
    }

    struct PortBacking
    {
        ControlPort* port;
    };

    struct PatchBacking
    {
        FloatWriter write;
        LV2_URID urid;
        uint32_t controlPortIndex;
    };

    union Data
    {
        static_assert (std::is_trivial_v<PortBacking>,  "PortBacking must be trivial");
        static_assert (std::is_trivial_v<PatchBacking>, "PatchBacking must be trivial");

        explicit Data (PortBacking p)  : port  (p) {}
        explicit Data (PatchBacking p) : patch (p) {}

        PortBacking port;
        PatchBacking patch;
    };

    enum class Kind { port, patch };

    Data data;
    Kind kind;

    JUCE_LEAK_DETECTOR (ParameterWriter)
};

static String lilvNodeToUriString (const LilvNode* node)
{
    return node != nullptr ? String::fromUTF8 (lilv_node_as_uri (node)) : String{};
}

static String lilvNodeToString (const LilvNode* node)
{
    return node != nullptr ? String::fromUTF8 (lilv_node_as_string (node)) : String{};
}

/*  This holds all of the discovered groups in the plugin's manifest, and allows us to
    add parameters to these groups as we discover them.

    Once all the parameters have been added with addParameter(), you can call
    getTree() to convert this class' contents (which are optimised for fast lookup
    and modification) into a plain old AudioProcessorParameterGroup.
*/
class IntermediateParameterTree
{
public:
    explicit IntermediateParameterTree (World& worldIn)
        : world (worldIn)
    {
        const auto groups = getGroups (world);
        const auto symbolNode = world.newUri (LV2_CORE__symbol);
        const auto nameNode   = world.newUri (LV2_CORE__name);

        for (const auto& group : groups)
        {
            const auto symbol = lilvNodeToString (world.get (group.get(), symbolNode.get(), nullptr).get());
            const auto name   = lilvNodeToString (world.get (group.get(), nameNode  .get(), nullptr).get());
            owning.emplace (lilvNodeToUriString (group.get()),
                            std::make_unique<AudioProcessorParameterGroup> (symbol, name, "|"));
        }
    }

    void addParameter (StringRef group, std::unique_ptr<LV2Parameter> param)
    {
        if (param == nullptr)
            return;

        const auto it = owning.find (group);
        (it != owning.cend() ? *it->second : topLevel).addChild (std::move (param));
    }

    static AudioProcessorParameterGroup getTree (IntermediateParameterTree tree)
    {
        std::map<String, AudioProcessorParameterGroup*> nonowning;

        for (const auto& pair : tree.owning)
            nonowning.emplace (pair.first, pair.second.get());

        const auto groups = getGroups (tree.world);
        const auto subgroupNode = tree.world.newUri (LV2_PORT_GROUPS__subGroupOf);

        for (const auto& group : groups)
        {
            const auto innerIt = tree.owning.find (lilvNodeToUriString (group.get()));

            if (innerIt == tree.owning.cend())
                continue;

            const auto outer = lilvNodeToUriString (tree.world.get (group.get(), subgroupNode.get(), nullptr).get());
            const auto outerIt = nonowning.find (outer);

            if (outerIt != nonowning.cend() && containsParameters (outerIt->second))
                outerIt->second->addChild (std::move (innerIt->second));
        }

        for (auto& subgroup : tree.owning)
            if (containsParameters (subgroup.second.get()))
                tree.topLevel.addChild (std::move (subgroup.second));

        return std::move (tree.topLevel);
    }

private:
    static std::vector<OwningNode> getGroups (World& world)
    {
        std::vector<OwningNode> names;

        for (auto* uri : { LV2_PORT_GROUPS__Group, LV2_PORT_GROUPS__InputGroup, LV2_PORT_GROUPS__OutputGroup })
            for (const auto* group : world.findNodes (nullptr, world.newUri (LILV_NS_RDF "type").get(), world.newUri (uri).get()))
                names.push_back (OwningNode { lilv_node_duplicate (group) });

        return names;
    }

    static bool containsParameters (const AudioProcessorParameterGroup* g)
    {
        if (g == nullptr)
            return false;

        for (auto* node : *g)
        {
            if (node->getParameter() != nullptr)
                return true;

            if (auto* group = node->getGroup())
                if (containsParameters (group))
                    return true;
        }

        return false;
    }

    World& world;
    AudioProcessorParameterGroup topLevel;
    std::map<String, std::unique_ptr<AudioProcessorParameterGroup>> owning;

    JUCE_LEAK_DETECTOR (IntermediateParameterTree)
};

struct BypassParameter final : public LV2Parameter
{
    BypassParameter (const ParameterInfo& parameterInfo, ParameterValuesAndFlags& cacheIn)
        : LV2Parameter ("Bypass", parameterInfo, cacheIn) {}

    float getValue() const noexcept override
    {
        return LV2Parameter::getValue() > 0.0f ? 0.0f : 1.0f;
    }

    void setValue (float newValue) override
    {
        LV2Parameter::setValue (newValue > 0.0f ? 0.0f : 1.0f);
    }

    float getDefaultValue() const override                              { return 0.0f; }
    bool isAutomatable() const override                                 { return true; }
    bool isDiscrete() const override                                    { return true; }
    bool isBoolean() const override                                     { return true; }
    int getNumSteps() const override                                    { return 2; }
    StringArray getAllValueStrings() const override                     { return { TRANS ("Off"), TRANS ("On") }; }
};

struct ParameterData
{
    ParameterInfo info;
    ParameterWriter writer;
    String group;
    String name;
};

template <typename T>
static auto getPortPointers (SimpleSpan<T> range)
{
    using std::begin;
    std::vector<decltype (&(*begin (range)))> result;

    for (auto& port : range)
    {
        result.resize (std::max ((size_t) (port.header.index + 1), result.size()), nullptr);
        result[port.header.index] = &port;
    }

    return result;
}

static std::unique_ptr<LV2Parameter> makeParameter (const uint32_t* enabledPortIndex,
                                                    const ParameterData& data,
                                                    ParameterValuesAndFlags& cache)
{
    // The bypass parameter is a bit special, in that JUCE expects the parameter to be a bypass
    // (where 0 is active, 1 is inactive), but the LV2 version is called "enabled" and has
    // different semantics (0 is inactive, 1 is active).
    // To work around this, we wrap the LV2 parameter in a special inverting JUCE parameter.

    if (enabledPortIndex != nullptr)
        if (auto* index = data.writer.getPortIndex())
            if (*index == *enabledPortIndex)
                return std::make_unique<BypassParameter> (data.info, cache);

    return std::make_unique<LV2Parameter> (data.name, data.info, cache);
}

class ControlPortAccelerationStructure
{
public:
    ControlPortAccelerationStructure (SimpleSpan<ControlPort> controlPorts)
        : indexedControlPorts (getPortPointers (controlPorts))
    {
        for (const auto& port : controlPorts)
            if (port.header.direction == Port::Direction::output)
                outputPorts.push_back (&port);
    }

    const std::vector<ControlPort*>& getIndexedControlPorts() { return indexedControlPorts; }

    ControlPort* getControlPortByIndex (uint32_t index) const
    {
        if (isPositiveAndBelow (index, indexedControlPorts.size()))
            return indexedControlPorts[index];

        return nullptr;
    }

    void writeOutputPorts (UiEventListener* target, MessageBufferInterface<UiMessageHeader>& uiMessages) const
    {
        if (target == nullptr)
            return;

        for (const auto* port : outputPorts)
        {
            const auto chars = toChars (port->currentValue);
            uiMessages.pushMessage ({ target, { port->header.index, 0 } }, (uint32_t) chars.size(), chars.data());
        }
    }

private:
    std::vector<ControlPort*> indexedControlPorts;
    std::vector<const ControlPort*> outputPorts;
};

class ParameterValueCache
{
public:
    /*  This takes some information about all the parameters that this plugin wants to expose,
        then builds and installs the actual parameters.
    */
    ParameterValueCache (AudioPluginInstance& processor,
                         World& world,
                         LV2_URID_Map mapFeature,
                         const std::vector<ParameterData>& data,
                         ControlPort* enabledPort)
        : uiForge (mapFeature),
          cache (data.size())
    {
        // Parameter indices are unknown until we add the parameters to the processor.
        // This map lets us keep track of which ParameterWriter corresponds to each parameter.
        // After the parameters have been added to the processor, we'll convert this
        // to a simple vector that stores each ParameterWriter at the same index
        // as the corresponding parameter.
        std::map<AudioProcessorParameter*, ParameterWriter> writerForParameter;

        IntermediateParameterTree tree { world };

        const auto* enabledPortIndex = enabledPort != nullptr ? &enabledPort->header.index
                                                              : nullptr;

        for (const auto& item : data)
        {
            auto param = makeParameter (enabledPortIndex, item, cache);

            if (auto* urid = item.writer.getUrid())
                urids.emplace (*urid, param.get());

            if (auto* index = item.writer.getPortIndex())
                portIndices.emplace (*index, param.get());

            writerForParameter.emplace (param.get(), item.writer);

            tree.addParameter (item.group, std::move (param));
        }

        processor.setHostedParameterTree (IntermediateParameterTree::getTree (std::move (tree)));

        // Build the vector of writers
        writers.reserve (data.size());

        for (auto* param : processor.getParameters())
        {
            const auto it = writerForParameter.find (param);
            jassert (it != writerForParameter.end());
            writers.push_back (it->second); // The writer must exist at the same index as the parameter!
        }

        // Duplicate port indices or urids?
        jassert (processor.getParameters().size() == (int) (urids.size() + portIndices.size()));

        // Set parameters to default values
        const auto setToDefault = [] (auto& container)
        {
            for (auto& item : container)
                item.second->setDenormalisedValueWithoutTriggeringUpdate (item.second->getDenormalisedDefaultValue());
        };

        setToDefault (urids);
        setToDefault (portIndices);
    }

    void postChangedParametersToProcessor (const ParameterWriterUrids helperUrids,
                                           LV2_Atom_Forge* forge)
    {
        cache.ifProcessorValuesChanged ([&] (size_t index, float value)
                                        {
                                            writers[index].writeToProcessor (helperUrids, forge, value);
                                        });
    }

    void postChangedParametersToUi (UiEventListener* target,
                                    const ParameterWriterUrids helperUrids,
                                    MessageBufferInterface<UiMessageHeader>& uiMessages)
    {
        if (target == nullptr)
            return;

        cache.ifUiValuesChanged ([&] (size_t index, float value)
                                 {
                                     writeParameterToUi (target, writers[index], value, helperUrids, uiMessages);
                                 });
    }

    void postAllParametersToUi (UiEventListener* target,
                                const ParameterWriterUrids helperUrids,
                                MessageBufferInterface<UiMessageHeader>& uiMessages)
    {
        if (target == nullptr)
            return;

        const auto numWriters = writers.size();

        for (size_t i = 0; i < numWriters; ++i)
            writeParameterToUi (target, writers[i], cache.get (i), helperUrids, uiMessages);

        cache.clearUiFlags();
    }

    LV2Parameter* getParamByUrid (LV2_URID urid) const
    {
        const auto it = urids.find (urid);
        return it != urids.end() ? it->second : nullptr;
    }

    LV2Parameter* getParamByPortIndex (uint32_t portIndex) const
    {
        const auto it = portIndices.find (portIndex);
        return it != portIndices.end() ? it->second : nullptr;
    }

    void updateFromControlPorts (const ControlPortAccelerationStructure& ports) const
    {
        for (const auto& pair : portIndices)
            if (auto* port = ports.getControlPortByIndex (pair.first))
                if (auto* param = pair.second)
                    param->setDenormalisedValueWithoutTriggeringUpdate (port->currentValue);
    }

private:
    void writeParameterToUi (UiEventListener* target,
                             const ParameterWriter& writer,
                             float value,
                             const ParameterWriterUrids helperUrids,
                             MessageBufferInterface<UiMessageHeader>& uiMessages)
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        uiForge.setBuffer (forgeStorage.data(), forgeStorage.size());
        const auto messageHeader = writer.writeToUi (helperUrids, *uiForge.get(), value);
        uiMessages.pushMessage ({ target, messageHeader.header }, messageHeader.size, forgeStorage.data());
    }

    SingleSizeAlignedStorage<8> forgeStorage { 256 };
    lv2_shared::AtomForge uiForge;

    ParameterValuesAndFlags cache;
    std::vector<ParameterWriter> writers;
    std::map<LV2_URID, LV2Parameter*> urids;
    std::map<uint32_t, LV2Parameter*> portIndices;

    JUCE_LEAK_DETECTOR (ParameterValueCache)
};

struct PatchSetCallback
{
    explicit PatchSetCallback (ParameterValueCache& x) : cache (x) {}

    // If we receive a patch set from the processor, we can assume that the UI will
    // put itself into the correct state when it receives the message.
    void setParameter (LV2_URID property, float value) const noexcept
    {
        if (auto* param = cache.getParamByUrid (property))
            param->setDenormalisedValueWithoutTriggeringUpdate (value);
    }

    // TODO gesture support will probably go here, once it's part of the LV2 spec

    ParameterValueCache& cache;
};

struct SupportedParameter
{
    ParameterInfo info;
    bool supported;
    LV2_URID type;
};

static SupportedParameter getInfoForPatchParameter (World& worldIn,
                                                    const UsefulUrids& urids,
                                                    const NodeUri& property)
{
    const auto rangeUri = worldIn.newUri (LILV_NS_RDFS "range");
    const auto type = worldIn.get (property.get(), rangeUri.get(), nullptr);

    if (type == nullptr)
        return { {}, false, {} };

    const auto typeUrid = urids.symap.map (lilv_node_as_uri (type.get()));

    const LV2_URID types[] { urids.mLV2_ATOM__Int,
                             urids.mLV2_ATOM__Long,
                             urids.mLV2_ATOM__Float,
                             urids.mLV2_ATOM__Double,
                             urids.mLV2_ATOM__Bool };

    if (std::find (std::begin (types), std::end (types), typeUrid) == std::end (types))
        return { {}, false, {} };

    const auto getValue = [&] (const char* uri, float fallback)
    {
        return Port::getFloatValue (worldIn.get (property.get(), worldIn.newUri (uri).get(), nullptr).get(), fallback);
    };

    const auto hasPortProperty = [&] (const char* uri)
    {
        return worldIn.ask (property.get(),
                            worldIn.newUri (LV2_CORE__portProperty).get(),
                            worldIn.newUri (uri).get());
    };

    const auto metadataScalePoints = worldIn.findNodes (property.get(),
                                                        worldIn.newUri (LV2_CORE__scalePoint).get(),
                                                        nullptr);
    SafeSortedSet<StoredScalePoint> parsedScalePoints;

    for (const auto* scalePoint : metadataScalePoints)
    {
        const auto label = worldIn.get (scalePoint, worldIn.newUri (LILV_NS_RDFS "label").get(), nullptr);
        const auto value = worldIn.get (scalePoint, worldIn.newUri (LILV_NS_RDF "value").get(), nullptr);

        if (label != nullptr && value != nullptr)
            parsedScalePoints.insert ({ lilv_node_as_string (label.get()), lilv_node_as_float (value.get()) });
        else
            jassertfalse; // A ScalePoint must have both a rdfs:label and a rdf:value
    }

    const auto minimum = getValue (LV2_CORE__minimum, 0.0f);
    const auto maximum = getValue (LV2_CORE__maximum, 1.0f);

    return { { std::move (parsedScalePoints),
               "des:" + String::fromUTF8 (property.getTyped()),
               getValue (LV2_CORE__default, (minimum + maximum) * 0.5f),
               minimum,
               maximum,
               typeUrid == urids.mLV2_ATOM__Bool || hasPortProperty (LV2_CORE__toggled),
               typeUrid == urids.mLV2_ATOM__Int || typeUrid == urids.mLV2_ATOM__Long,
               hasPortProperty (LV2_CORE__enumeration) },
             true,
             typeUrid };
}

static std::vector<ParameterData> getPortBasedParameters (World& world,
                                                          const Plugin& plugin,
                                                          std::initializer_list<const ControlPort*> hiddenPorts,
                                                          SimpleSpan<ControlPort> controlPorts)
{
    std::vector<ParameterData> result;

    const auto groupNode = world.newUri (LV2_PORT_GROUPS__group);

    for (auto& port : controlPorts)
    {
        if (port.header.direction != Port::Direction::input)
            continue;

        if (std::find (std::begin (hiddenPorts), std::end (hiddenPorts), &port) != std::end (hiddenPorts))
            continue;

        const auto lilvPort = plugin.getPortByIndex (port.header.index);
        const auto group = lilvNodeToUriString (lilvPort.get (groupNode.get()).get());

        result.push_back ({ port.info, ParameterWriter { &port }, group, port.header.name });
    }

    return result;
}

static void writeFloatToForge  (LV2_Atom_Forge* forge, float value) { lv2_atom_forge_float  (forge, value); }
static void writeDoubleToForge (LV2_Atom_Forge* forge, float value) { lv2_atom_forge_double (forge, (double) value); }
static void writeIntToForge    (LV2_Atom_Forge* forge, float value) { lv2_atom_forge_int    (forge, (int32_t) value); }
static void writeLongToForge   (LV2_Atom_Forge* forge, float value) { lv2_atom_forge_long   (forge, (int64_t) value); }
static void writeBoolToForge   (LV2_Atom_Forge* forge, float value) { lv2_atom_forge_bool   (forge, value > 0.5f); }

static std::vector<ParameterData> getPatchBasedParameters (World& world,
                                                           const Plugin& plugin,
                                                           const UsefulUrids& urids,
                                                           uint32_t controlPortIndex)
{
    // This returns our writable parameters in an indeterminate order.
    // We want our parameters to be in a consistent order between runs, so
    // we'll create all the parameters in one pass, sort them, and then
    // add them in a separate pass.
    const auto writableControls = world.findNodes (plugin.getUri().get(),
                                                   world.newUri (LV2_PATCH__writable).get(),
                                                   nullptr);

    struct DataAndUri
    {
        ParameterData data;
        String uri;
    };

    std::vector<DataAndUri> resultWithUris;

    const auto groupNode = world.newUri (LV2_PORT_GROUPS__group);

    for (auto* ctrl : writableControls)
    {
        const auto labelString = [&]
        {
            if (auto label = world.get (ctrl, world.newUri (LILV_NS_RDFS "label").get(), nullptr))
                return String::fromUTF8 (lilv_node_as_string (label.get()));

            return String();
        }();

        const auto uri = String::fromUTF8 (lilv_node_as_uri (ctrl));
        const auto info = getInfoForPatchParameter (world, urids, world.newUri (uri.toRawUTF8()));

        if (! info.supported)
            continue;

        const auto write = [&]
        {
            if (info.type == urids.mLV2_ATOM__Int)
                return writeIntToForge;

            if (info.type == urids.mLV2_ATOM__Long)
                return writeLongToForge;

            if (info.type == urids.mLV2_ATOM__Double)
                return writeDoubleToForge;

            if (info.type == urids.mLV2_ATOM__Bool)
                return writeBoolToForge;

            return writeFloatToForge;
        }();

        const auto group = lilvNodeToUriString (world.get (ctrl, groupNode.get(), nullptr).get());
        resultWithUris.push_back ({ { info.info,
                                      ParameterWriter { write, urids.symap.map (uri.toRawUTF8()), controlPortIndex },
                                      group,
                                      labelString },
                                    uri });
    }

    const auto compareUris = [] (const DataAndUri& a, const DataAndUri& b) { return a.uri < b.uri; };
    std::sort (resultWithUris.begin(), resultWithUris.end(), compareUris);

    std::vector<ParameterData> result;

    for (const auto& item : resultWithUris)
        result.push_back (item.data);

    return result;
}

static std::vector<ParameterData> getJuceParameterInfo (World& world,
                                                        const Plugin& plugin,
                                                        const UsefulUrids& urids,
                                                        std::initializer_list<const ControlPort*> hiddenPorts,
                                                        SimpleSpan<ControlPort> controlPorts,
                                                        uint32_t controlPortIndex)
{
    auto port  = getPortBasedParameters  (world, plugin, hiddenPorts, controlPorts);
    auto patch = getPatchBasedParameters (world, plugin, urids, controlPortIndex);

    port.insert (port.end(), patch.begin(), patch.end());
    return port;
}

// Rather than sprinkle #ifdef everywhere, risking the wrath of the entire C++
// standards committee, we put all of our conditionally-compiled stuff into a
// specialised template that compiles away to nothing when editor support is
// not available.
#if JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
 constexpr auto editorFunctionalityEnabled = true;
#else
 constexpr auto editorFunctionalityEnabled = false;
#endif

template <bool editorEnabled = editorFunctionalityEnabled> class OptionalEditor;

template <>
class OptionalEditor<true>
{
public:
    OptionalEditor (String uiBundleUriIn, UiDescriptor uiDescriptorIn, std::function<void()> timerCallback)
        : uiBundleUri (std::move (uiBundleUriIn)),
          uiDescriptor (std::move (uiDescriptorIn)),
          changedParameterFlusher (std::move (timerCallback)) {}

    void createView()
    {
        if (auto* editor = editorPointer.getComponent())
            editor->createView();
    }

    void destroyView()
    {
        if (auto* editor = editorPointer.getComponent())
            editor->destroyView();
    }

    std::unique_ptr<AudioProcessorEditor> createEditor (World& world,
                                                        AudioPluginInstance& p,
                                                        InstanceProvider& instanceProviderIn,
                                                        TouchListener& touchListenerIn,
                                                        EditorListener& listenerIn)
    {
        if (! hasEditor())
            return nullptr;

        const auto queryFeatures = [this, &world] (const char* kind)
        {
            return world.findNodes (world.newUri (uiDescriptor.get()->URI).get(),
                                    world.newUri (kind).get(),
                                    nullptr);
        };

        auto newEditor = std::make_unique<Editor> (world,
                                                   p,
                                                   instanceProviderIn,
                                                   uiDescriptor,
                                                   touchListenerIn,
                                                   listenerIn,
                                                   uiBundleUri,
                                                   RequiredFeatures { queryFeatures (LV2_CORE__requiredFeature) },
                                                   OptionalFeatures { queryFeatures (LV2_CORE__optionalFeature) });

        editorPointer = newEditor.get();

        changedParameterFlusher.startTimerHz (60);

        return newEditor;
    }

    bool hasEditor() const
    {
        return uiDescriptor.get() != nullptr;
    }

    void prepareToDestroyEditor()
    {
        changedParameterFlusher.stopTimer();
    }

private:
    Component::SafePointer<Editor> editorPointer = nullptr;
    String uiBundleUri;
    UiDescriptor uiDescriptor;
    LambdaTimer changedParameterFlusher;
};

template <>
class OptionalEditor<false>
{
public:
    OptionalEditor (String, UiDescriptor, std::function<void()>) {}

    void createView() {}
    void destroyView() {}

    std::unique_ptr<AudioProcessorEditor> createEditor (World&,
                                                        AudioPluginInstance&,
                                                        InstanceProvider&,
                                                        TouchListener&,
                                                        EditorListener&)
    {
        return nullptr;
    }

    bool hasEditor() const { return false; }
    void prepareToDestroyEditor() {}
};

//==============================================================================
class LV2AudioPluginInstance final : public AudioPluginInstance,
                                     private TouchListener,
                                     private EditorListener,
                                     private InstanceProvider
{
public:
    LV2AudioPluginInstance (std::shared_ptr<World> worldIn,
                            const Plugin& pluginIn,
                            const UsefulUris& uris,
                            std::unique_ptr<InstanceWithSupports>&& in,
                            PluginDescription&& desc,
                            std::vector<String> knownPresetUris,
                            PluginState stateToApply,
                            String uiBundleUriIn,
                            UiDescriptor uiDescriptorIn)
        : LV2AudioPluginInstance (worldIn,
                                  pluginIn,
                                  std::move (in),
                                  std::move (desc),
                                  std::move (knownPresetUris),
                                  std::move (stateToApply),
                                  std::move (uiBundleUriIn),
                                  std::move (uiDescriptorIn),
                                  getParsedBuses (*worldIn, pluginIn, uris)) {}

    void fillInPluginDescription (PluginDescription& d) const override { d = description; }

    const String getName() const override { return description.name; }

    void prepareToPlay (double sampleRate, int numSamples) override
    {
        // In REAPER, changing the sample rate will deactivate the plugin,
        // save its state, destroy it, create a new instance, restore the
        // state, and then activate the new instance.
        // We'll do the same, because there's no way to retroactively change the
        // plugin sample rate.
        // This is a bit expensive, so try to avoid changing the sample rate too
        // frequently.

        // In addition to the above, we also need to destroy the custom view,
        // and recreate it after creating the new plugin instance.
        // Ideally this should all happen in the same Component.

        deactivate();
        destroyView();

        MemoryBlock mb;
        getStateInformation (mb);

        instance = std::make_unique<InstanceWithSupports> (*world,
                                                           std::move (instance->symap),
                                                           plugin,
                                                           std::move (instance->ports),
                                                           numSamples,
                                                           sampleRate);

        // prepareToPlay is *guaranteed* not to be called concurrently with processBlock
        setStateInformationImpl (mb.getData(), (int) mb.getSize(), ConcurrentWithAudioCallback::no);

        jassert (numSamples == instance->features.getMaxBlockSize());

        optionalEditor.createView();
        activate();
    }

    void releaseResources() override { deactivate(); }

    using AudioPluginInstance::processBlock;
    using AudioPluginInstance::processBlockBypassed;

    void processBlock (AudioBuffer<float>& audio, MidiBuffer& midi) override
    {
        processBlockImpl (audio, midi);
    }

    void processBlockBypassed (AudioBuffer<float>& audio, MidiBuffer& midi) override
    {
        if (bypassParam != nullptr)
            processBlockImpl (audio, midi);
        else
            AudioPluginInstance::processBlockBypassed (audio, midi);
    }

    double getTailLengthSeconds() const override { return {}; } // TODO

    bool acceptsMidi() const override
    {
        if (instance == nullptr)
            return false;

        auto ports = instance->ports.getAtomPorts();

        return std::any_of (ports.begin(), ports.end(), [&] (const AtomPort& a)
        {
            if (a.header.direction != Port::Direction::input)
                return false;

            return portAtIndexSupportsMidi (a.header.index);
        });
    }

    bool producesMidi() const override
    {
        if (instance == nullptr)
            return false;

        auto ports = instance->ports.getAtomPorts();

        return std::any_of (ports.begin(), ports.end(), [&] (const AtomPort& a)
        {
            if (a.header.direction != Port::Direction::output)
                return false;

            return portAtIndexSupportsMidi (a.header.index);
        });
    }

    AudioProcessorEditor* createEditor() override
    {
        return optionalEditor.createEditor (*world, *this, *this, *this, *this).release();
    }

    bool hasEditor() const override
    {
        return optionalEditor.hasEditor();
    }

    int getNumPrograms() override { return (int) presetUris.size(); }

    int getCurrentProgram() override
    {
        return lastAppliedPreset;
    }

    void setCurrentProgram (int newProgram) override
    {
        JUCE_ASSERT_MESSAGE_THREAD;

        if (! isPositiveAndBelow (newProgram, presetUris.size()))
            return;

        lastAppliedPreset = newProgram;
        applyStateWithAppropriateLocking (loadStateWithUri (presetUris[(size_t) newProgram]),
                                          ConcurrentWithAudioCallback::yes);
    }

    const String getProgramName (int program) override
    {
        JUCE_ASSERT_MESSAGE_THREAD;

        if (isPositiveAndBelow (program, presetUris.size()))
            return loadStateWithUri (presetUris[(size_t) program]).getLabel();

        return {};
    }

    void changeProgramName (int program, const String& label) override
    {
        JUCE_ASSERT_MESSAGE_THREAD;

        if (isPositiveAndBelow (program, presetUris.size()))
            loadStateWithUri (presetUris[(size_t) program]).setLabel (label);
    }

    void getStateInformation (MemoryBlock& block) override
    {
        JUCE_ASSERT_MESSAGE_THREAD;

        // TODO where should the state URI come from?
        PortMap portStateManager (instance->ports);
        const auto stateUri = String::fromUTF8 (instance->instance.getUri()) + "/savedState";
        auto mapFeature = instance->symap->getMapFeature();
        auto unmapFeature = instance->symap->getUnmapFeature();
        const auto state = PluginState::SaveRestoreHandle (*instance, portStateManager).save (plugin.get(), &mapFeature);
        const auto string = state.toString (world->get(), &mapFeature, &unmapFeature, stateUri.toRawUTF8());
        block.replaceAll (string.data(), string.size());
    }

    void setStateInformation (const void* data, int size) override
    {
        setStateInformationImpl (data, size, ConcurrentWithAudioCallback::yes);
    }

    void setNonRealtime (bool newValue) noexcept override
    {
        JUCE_ASSERT_MESSAGE_THREAD;

        AudioPluginInstance::setNonRealtime (newValue);
        instance->features.setNonRealtime (newValue);
    }

    bool isBusesLayoutSupported (const BusesLayout& layout) const override
    {
        for (const auto& pair : { std::make_tuple (&layout.inputBuses,  &declaredBusLayout.inputs),
                                  std::make_tuple (&layout.outputBuses, &declaredBusLayout.outputs) })
        {
            const auto& requested = *std::get<0> (pair);
            const auto& allowed   = *std::get<1> (pair);

            if ((size_t) requested.size() != allowed.size())
                return false;

            for (size_t busIndex = 0; busIndex < allowed.size(); ++busIndex)
            {
                const auto& requestedBus = requested[(int) busIndex];
                const auto& allowedBus = allowed[busIndex];

                if (! allowedBus.isCompatible (requestedBus))
                    return false;
            }
        }

        return true;
    }

    void processorLayoutsChanged() override { ioMap = lv2_shared::PortToAudioBufferMap { getBusesLayout(), declaredBusLayout }; }

    AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

private:
    enum class ConcurrentWithAudioCallback { no, yes };

    LV2AudioPluginInstance (std::shared_ptr<World> worldIn,
                            const Plugin& pluginIn,
                            std::unique_ptr<InstanceWithSupports>&& in,
                            PluginDescription&& desc,
                            std::vector<String> knownPresetUris,
                            PluginState stateToApply,
                            String uiBundleUriIn,
                            UiDescriptor uiDescriptorIn,
                            const lv2_shared::ParsedBuses& parsedBuses)
        : AudioPluginInstance (getBusesProperties (parsedBuses, *worldIn)),
          declaredBusLayout (parsedBuses),
          world (std::move (worldIn)),
          plugin (pluginIn),
          description (std::move (desc)),
          presetUris (std::move (knownPresetUris)),
          instance (std::move (in)),
          optionalEditor (std::move (uiBundleUriIn),
                          std::move (uiDescriptorIn),
                          [this] { postChangedParametersToUi(); })
    {
        applyStateWithAppropriateLocking (std::move (stateToApply), ConcurrentWithAudioCallback::no);
    }

    void setStateInformationImpl (const void* data, int size, ConcurrentWithAudioCallback concurrent)
    {
        JUCE_ASSERT_MESSAGE_THREAD;

        if (data == nullptr || size == 0)
            return;

        auto begin = static_cast<const char*> (data);
        std::vector<char> copy (begin, begin + size);
        copy.push_back (0);
        auto mapFeature = instance->symap->getMapFeature();
        applyStateWithAppropriateLocking (PluginState { lilv_state_new_from_string (world->get(), &mapFeature, copy.data()) },
                                          concurrent);
    }

    // This does *not* destroy the editor component.
    // If we destroy the processor, the view must also be destroyed to avoid dangling pointers.
    // However, JUCE clients expect their editors to remain valid for the duration of the
    // AudioProcessor's lifetime.
    // As a compromise, this will create a new LV2 view into an existing editor component.
    void destroyView()
    {
        optionalEditor.destroyView();
    }

    void activate()
    {
        if (! active)
            instance->instance.activate();

        active = true;
    }

    void deactivate()
    {
        if (active)
            instance->instance.deactivate();

        active = false;
    }

    void processBlockImpl (AudioBuffer<float>& audio, MidiBuffer& midi)
    {
        preparePortsForRun (audio, midi);

        instance->instance.run (static_cast<uint32_t> (audio.getNumSamples()));
        instance->features.processResponses();

        processPortsAfterRun (midi);
    }

    bool portAtIndexSupportsMidi (uint32_t index) const noexcept
    {
        const auto port = plugin.getPortByIndex (index);

        if (! port.isValid())
            return false;

        return port.supportsEvent (world->newUri (LV2_MIDI__MidiEvent).get());
    }

    void controlGrabbed (uint32_t port, bool grabbed) override
    {
        if (auto* param = parameterValues.getParamByPortIndex (port))
        {
            if (grabbed)
                param->beginChangeGesture();
            else
                param->endChangeGesture();
        }
    }

    void viewCreated (UiEventListener* newListener) override
    {
        uiEventListener = newListener;
        postAllParametersToUi();
    }

    ParameterWriterUrids getParameterWriterUrids() const
    {
        return { instance->urids.mLV2_PATCH__Set,
                 instance->urids.mLV2_PATCH__property,
                 instance->urids.mLV2_PATCH__value,
                 instance->urids.mLV2_ATOM__eventTransfer };
    }

    void postAllParametersToUi()
    {
        parameterValues.postAllParametersToUi (uiEventListener, getParameterWriterUrids(), *instance->processorToUi);
        controlPortStructure.writeOutputPorts (uiEventListener, *instance->processorToUi);
    }

    void postChangedParametersToUi()
    {
        parameterValues.postChangedParametersToUi (uiEventListener, getParameterWriterUrids(), *instance->processorToUi);
        controlPortStructure.writeOutputPorts (uiEventListener, *instance->processorToUi);
    }

    void notifyEditorBeingDeleted() override
    {
        optionalEditor.prepareToDestroyEditor();
        uiEventListener = nullptr;
        editorBeingDeleted (getActiveEditor());
    }

    InstanceWithSupports* getInstanceWithSupports() const override
    {
        return instance.get();
    }

    void applyStateWithAppropriateLocking (PluginState&& state, ConcurrentWithAudioCallback concurrent)
    {
        PortMap portStateManager (instance->ports);

        // If a plugin supports threadSafeRestore, its restore method is thread-safe
        // and may be called concurrently with audio class functions.
        if (hasThreadSafeRestore || concurrent == ConcurrentWithAudioCallback::no)
        {
            state.restore (*instance, portStateManager);
        }
        else
        {
            const ScopedLock lock (getCallbackLock());
            state.restore (*instance, portStateManager);
        }

        parameterValues.updateFromControlPorts (controlPortStructure);
        asyncFullUiParameterUpdate.triggerAsyncUpdate();
    }

    PluginState loadStateWithUri (const String& str)
    {
        auto mapFeature = instance->symap->getMapFeature();
        const auto presetUri = world->newUri (str.toRawUTF8());
        lilv_world_load_resource (world->get(), presetUri.get());
        return PluginState { lilv_state_new_from_world (world->get(), &mapFeature, presetUri.get()) };
    }

    void connectPorts (AudioBuffer<float>& audio)
    {
        // Plugins that cannot process in-place will require the feature "inPlaceBroken".
        // We don't support that feature, so if we made it to this point we can assume that
        // in-place processing works.
        for (const auto& port : instance->ports.getAudioPorts())
        {
            const auto channel = ioMap.getChannelForPort (port.header.index);
            auto* ptr = isPositiveAndBelow (channel, audio.getNumChannels()) ? audio.getWritePointer (channel)
                                                                             : nullptr;
            instance->instance.connectPort (port.header.index, ptr);
        }

        for (const auto& port : instance->ports.getCvPorts())
            instance->instance.connectPort (port.header.index, nullptr);

        for (auto& port : instance->ports.getAtomPorts())
            instance->instance.connectPort (port.header.index, port.data());
    }

    void writeTimeInfoToPort (AtomPort& port)
    {
        if (port.header.direction != Port::Direction::input || ! port.getSupportsTime())
            return;

        auto* forge = port.getForge().get();
        auto* playhead = getPlayHead();

        if (playhead == nullptr)
            return;

        // Write timing info to the control port
        const auto info = playhead->getPosition();

        if (! info.hasValue())
            return;

        const auto& urids = instance->urids;

        lv2_atom_forge_frame_time (forge, 0);

        lv2_shared::ObjectFrame object { forge, (uint32_t) 0, urids.mLV2_TIME__Position };

        lv2_atom_forge_key (forge, urids.mLV2_TIME__speed);
        lv2_atom_forge_float (forge, info->getIsPlaying() ? 1.0f : 0.0f);

        if (const auto samples = info->getTimeInSamples())
        {
            lv2_atom_forge_key (forge, urids.mLV2_TIME__frame);
            lv2_atom_forge_long (forge, *samples);
        }

        if (const auto bar = info->getBarCount())
        {
            lv2_atom_forge_key (forge, urids.mLV2_TIME__bar);
            lv2_atom_forge_long (forge, *bar);
        }

        if (const auto beat = info->getPpqPosition())
        {
            if (const auto barStart = info->getPpqPositionOfLastBarStart())
            {
                lv2_atom_forge_key (forge, urids.mLV2_TIME__barBeat);
                lv2_atom_forge_float (forge, (float) (*beat - *barStart));
            }

            lv2_atom_forge_key (forge, urids.mLV2_TIME__beat);
            lv2_atom_forge_double (forge, *beat);
        }

        if (const auto sig = info->getTimeSignature())
        {
            lv2_atom_forge_key (forge, urids.mLV2_TIME__beatUnit);
            lv2_atom_forge_int (forge, sig->denominator);

            lv2_atom_forge_key (forge, urids.mLV2_TIME__beatsPerBar);
            lv2_atom_forge_float (forge, (float) sig->numerator);
        }

        if (const auto bpm = info->getBpm())
        {
            lv2_atom_forge_key (forge, urids.mLV2_TIME__beatsPerMinute);
            lv2_atom_forge_float (forge, (float) *bpm);
        }
    }

    void preparePortsForRun (AudioBuffer<float>& audio, MidiBuffer& midiBuffer)
    {
        connectPorts (audio);

        for (auto& port : instance->ports.getAtomPorts())
        {
            switch (port.header.direction)
            {
                case Port::Direction::input:
                    port.beginSequence();
                    break;

                case Port::Direction::output:
                    port.replaceWithChunk();
                    break;

                case Port::Direction::unknown:
                    jassertfalse;
                    break;
            }
        }

        for (auto& port : instance->ports.getAtomPorts())
            writeTimeInfoToPort (port);

        const auto controlPortForge = controlPort != nullptr ? controlPort->getForge().get()
                                                             : nullptr;

        parameterValues.postChangedParametersToProcessor (getParameterWriterUrids(), controlPortForge);

        instance->uiToProcessor.readAllAndClear ([this] (MessageHeader header, uint32_t size, const void* buffer)
        {
            pushMessage (header, size, buffer);
        });

        for (auto& port : instance->ports.getAtomPorts())
        {
            if (port.header.direction == Port::Direction::input)
            {
                for (const auto meta : midiBuffer)
                {
                    port.addEventToSequence (meta.samplePosition,
                                             instance->urids.mLV2_MIDI__MidiEvent,
                                             static_cast<uint32_t> (meta.numBytes),
                                             meta.data);
                }

                port.endSequence();
            }
        }

        if (freeWheelingPort != nullptr)
            freeWheelingPort->currentValue = isNonRealtime() ? freeWheelingPort->info.max
                                                             : freeWheelingPort->info.min;
    }

    void pushMessage (MessageHeader header, [[maybe_unused]] uint32_t size, const void* data)
    {
        if (header.protocol == 0 || header.protocol == instance->urids.mLV2_UI__floatProtocol)
        {
            const auto value = readUnaligned<float> (data);

            if (auto* param = parameterValues.getParamByPortIndex (header.portIndex))
            {
                param->setDenormalisedValue (value);
            }
            else if (auto* port = controlPortStructure.getControlPortByIndex (header.portIndex))
            {
                // No parameter corresponds to this port, write to the port directly
                port->currentValue = value;
            }
        }
        else if (auto* atomPort = header.portIndex < atomPorts.size() ? atomPorts[header.portIndex] : nullptr)
        {
            if (header.protocol == instance->urids.mLV2_ATOM__eventTransfer)
            {
                if (const auto* atom = convertToAtomPtr (data, (size_t) size))
                {
                    atomPort->addAtomToSequence (0, atom);

                    // Not UB; LV2_Atom_Object has LV2_Atom as its first member
                    if (atom->type == instance->urids.mLV2_ATOM__Object)
                        patchSetHelper.processPatchSet (reinterpret_cast<const LV2_Atom_Object*> (data), PatchSetCallback { parameterValues });
                }
            }
            else if (header.protocol == instance->urids.mLV2_ATOM__atomTransfer)
            {
                if (const auto* atom = convertToAtomPtr (data, (size_t) size))
                    atomPort->replaceBufferWithAtom (atom);
            }
        }
    }

    void processPortsAfterRun (MidiBuffer& midi)
    {
        midi.clear();

        for (auto& port : instance->ports.getAtomPorts())
            processAtomPort (port, midi);

        if (latencyPort != nullptr)
            setLatencySamples ((int) latencyPort->currentValue);
    }

    void processAtomPort (const AtomPort& port, MidiBuffer& midi)
    {
        if (port.header.direction != Port::Direction::output)
            return;

        // The port holds an Atom, by definition
        const auto* atom = reinterpret_cast<const LV2_Atom*> (port.data());

        if (atom->type != instance->urids.mLV2_ATOM__Sequence)
            return;

        // The Atom said that it was of sequence type, so this isn't UB
        const auto* sequence = reinterpret_cast<const LV2_Atom_Sequence*> (port.data());

        // http://lv2plug.in/ns/ext/atom#Sequence - run() stamps are always audio frames
        jassert (sequence->body.unit == 0 || sequence->body.unit == instance->urids.mLV2_UNITS__frame);

        for (const auto* event : lv2_shared::SequenceIterator { lv2_shared::SequenceWithSize { sequence } })
        {
            // At the moment, we forward all outgoing events to the UI.
            instance->processorToUi->pushMessage ({ uiEventListener, { port.header.index, instance->urids.mLV2_ATOM__eventTransfer } },
                                                  (uint32_t) (event->body.size + sizeof (LV2_Atom)),
                                                  &event->body);

            if (event->body.type == instance->urids.mLV2_MIDI__MidiEvent)
                midi.addEvent (event + 1, static_cast<int> (event->body.size), static_cast<int> (event->time.frames));

            if (lv2_atom_forge_is_object_type (port.getForge().get(), event->body.type))
                if (reinterpret_cast<const LV2_Atom_Object_Body*> (event + 1)->otype == instance->urids.mLV2_STATE__StateChanged)
                    updateHostDisplay (ChangeDetails{}.withNonParameterStateChanged (true));

            patchSetHelper.processPatchSet (event, PatchSetCallback { parameterValues });
        }
    }

    // Check for duplicate channel designations, and convert the set to a discrete channel layout
    // if any designations are duplicated.
    static std::set<lv2_shared::SinglePortInfo> validateAndRedesignatePorts (std::set<lv2_shared::SinglePortInfo> info)
    {
        const auto channelSet = lv2_shared::ParsedGroup::getEquivalentSet (info);

        if ((int) info.size() == channelSet.size())
            return info;

        std::set<lv2_shared::SinglePortInfo> result;
        auto designation = (int) AudioChannelSet::discreteChannel0;

        for (auto& item : info)
        {
            auto copy = item;
            copy.designation = (AudioChannelSet::ChannelType) designation++;
            result.insert (copy);
        }

        return result;
    }

    static AudioChannelSet::ChannelType getPortDesignation (World& world, const Port& port, size_t indexInGroup)
    {
        const auto defaultResult = (AudioChannelSet::ChannelType) (AudioChannelSet::discreteChannel0 + indexInGroup);
        const auto node = port.get (world.newUri (LV2_CORE__designation).get());

        if (node == nullptr)
            return defaultResult;

        const auto it = lv2_shared::channelDesignationMap.find (lilvNodeToUriString (node.get()));

        if (it == lv2_shared::channelDesignationMap.end())
            return defaultResult;

        return it->second;
    }

    static lv2_shared::ParsedBuses getParsedBuses (World& world, const Plugin& p, const UsefulUris& uris)
    {
        const auto groupPropertyUri = world.newUri (LV2_PORT_GROUPS__group);
        const auto optionalUri = world.newUri (LV2_CORE__connectionOptional);

        std::map<String, std::set<lv2_shared::SinglePortInfo>> inputGroups, outputGroups;
        std::set<lv2_shared::SinglePortInfo> ungroupedInputs, ungroupedOutputs;

        for (uint32_t i = 0, numPorts = p.getNumPorts(); i < numPorts; ++i)
        {
            const auto port = p.getPortByIndex (i);

            if (port.getKind (uris) != Port::Kind::audio)
                continue;

            const auto groupUri = lilvNodeToUriString (port.get (groupPropertyUri.get()).get());

            auto& set = [&]() -> auto&
            {
                if (groupUri.isEmpty())
                    return port.getDirection (uris) == Port::Direction::input ? ungroupedInputs : ungroupedOutputs;

                auto& group = port.getDirection (uris) == Port::Direction::input ? inputGroups : outputGroups;
                return group[groupUri];
            }();

            set.insert ({ port.getIndex(), getPortDesignation (world, port, set.size()), port.hasProperty (optionalUri) });
        }

        for (auto* groups : { &inputGroups, &outputGroups })
            for (auto& pair : *groups)
                pair.second = validateAndRedesignatePorts (std::move (pair.second));

        JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4702)
        const auto getMainGroupName = [&] (const char* propertyName)
        {
            for (const auto* item : p.getValue (world.newUri (propertyName).get()))
                return lilvNodeToUriString (item);

            return String{};
        };
        JUCE_END_IGNORE_WARNINGS_MSVC

        return { findStableBusOrder (getMainGroupName (LV2_PORT_GROUPS__mainInput),  inputGroups,  ungroupedInputs),
                 findStableBusOrder (getMainGroupName (LV2_PORT_GROUPS__mainOutput), outputGroups, ungroupedOutputs) };
    }

    static String getNameForUri (World& world, StringRef uri)
    {
        if (uri.isEmpty())
            return String();

        const auto node = world.get (world.newUri (uri).get(),
                                     world.newUri (LV2_CORE__name).get(),
                                     nullptr);

        if (node == nullptr)
            return String();

        return String::fromUTF8 (lilv_node_as_string (node.get()));
    }

    static BusesProperties getBusesProperties (const lv2_shared::ParsedBuses& parsedBuses, World& world)
    {
        BusesProperties result;

        for (const auto& pair : { std::make_tuple (&parsedBuses.inputs,  &result.inputLayouts),
                                  std::make_tuple (&parsedBuses.outputs, &result.outputLayouts) })
        {
            const auto& buses = *std::get<0> (pair);
            auto& layout = *std::get<1> (pair);

            for (const auto& bus : buses)
            {
                layout.add (AudioProcessor::BusProperties { getNameForUri (world, bus.uid),
                                                            bus.getEquivalentSet(),
                                                            bus.isRequired() });
            }
        }

        return result;
    }

    LV2_URID map (const char* str) const
    {
        return instance != nullptr ? instance->symap->map (str)
                                   : LV2_URID();
    }

    ControlPort* findControlPortWithIndex (uint32_t index) const
    {
        auto ports = instance->ports.getControlPorts();
        const auto indexMatches = [&] (const ControlPort& p) { return p.header.index == index; };
        const auto it = std::find_if (ports.begin(), ports.end(), indexMatches);

        return it != ports.end() ? &(*it) : nullptr;
    }

    const lv2_shared::ParsedBuses declaredBusLayout;
    lv2_shared::PortToAudioBufferMap ioMap { getBusesLayout(), declaredBusLayout };
    std::shared_ptr<World> world;
    Plugin plugin;
    PluginDescription description;
    std::vector<String> presetUris;
    std::unique_ptr<InstanceWithSupports> instance;
    AsyncFn asyncFullUiParameterUpdate { [this] { postAllParametersToUi(); } };

    std::vector<AtomPort*> atomPorts = getPortPointers (instance->ports.getAtomPorts());

    AtomPort* const controlPort = [&]() -> AtomPort*
    {
        const auto port = plugin.getPortByDesignation (world->newUri (LV2_CORE__InputPort).get(),
                                                       world->newUri (LV2_CORE__control).get());

        if (! port.isValid())
            return nullptr;

        const auto index = port.getIndex();

        if (! isPositiveAndBelow (index, atomPorts.size()))
            return nullptr;

        return atomPorts[index];
    }();

    ControlPort* const latencyPort = [&]() -> ControlPort*
    {
        if (! plugin.hasLatency())
            return nullptr;

        return findControlPortWithIndex (plugin.getLatencyPortIndex());
    }();

    ControlPort* const freeWheelingPort = [&]() -> ControlPort*
    {
        const auto port = plugin.getPortByDesignation (world->newUri (LV2_CORE__InputPort).get(),
                                                       world->newUri (LV2_CORE__freeWheeling).get());

        if (! port.isValid())
            return nullptr;

        return findControlPortWithIndex (port.getIndex());
    }();

    ControlPort* const enabledPort = [&]() -> ControlPort*
    {
        const auto port = plugin.getPortByDesignation (world->newUri (LV2_CORE__InputPort).get(),
                                                       world->newUri (LV2_CORE_PREFIX "enabled").get());

        if (! port.isValid())
            return nullptr;

        return findControlPortWithIndex (port.getIndex());
    }();

    lv2_shared::PatchSetHelper patchSetHelper { instance->symap->getMapFeature(), plugin.getUri().getTyped() };
    ControlPortAccelerationStructure controlPortStructure { instance->ports.getControlPorts() };
    ParameterValueCache parameterValues { *this,
                                          *world,
                                          instance->symap->getMapFeature(),
                                          getJuceParameterInfo (*world,
                                                                plugin,
                                                                instance->urids,
                                                                { latencyPort, freeWheelingPort },
                                                                instance->ports.getControlPorts(),
                                                                controlPort != nullptr ? controlPort->header.index : 0),
                                          enabledPort };
    LV2Parameter* bypassParam = enabledPort != nullptr ? parameterValues.getParamByPortIndex (enabledPort->header.index)
                                                       : nullptr;

    std::atomic<UiEventListener*> uiEventListener { nullptr };
    OptionalEditor<> optionalEditor;
    int lastAppliedPreset = 0;
    bool hasThreadSafeRestore = plugin.hasExtensionData (world->newUri (LV2_STATE__threadSafeRestore));
    bool active { false };

    JUCE_LEAK_DETECTOR (LV2AudioPluginInstance)
};

} // namespace lv2_host

//==============================================================================
class LV2PluginFormat::Pimpl
{
public:
    Pimpl()
    {
        loadAllPluginsFromPaths (getDefaultLocationsToSearch());

        const auto tempFile = lv2ResourceFolder.getFile();

        if (tempFile.createDirectory())
        {
            for (const auto& bundle : lv2::Bundle::getAllBundles())
            {
                const auto pathToBundle = tempFile.getChildFile (bundle.name + String (".lv2"));

                if (! pathToBundle.createDirectory())
                    continue;

                for (const auto& resource : bundle.contents)
                    pathToBundle.getChildFile (resource.name).replaceWithText (resource.contents);

                const auto pathString = File::addTrailingSeparator (pathToBundle.getFullPathName());
                world->loadBundle (world->newFileUri (nullptr, pathString.toRawUTF8()));
            }
        }
    }

    ~Pimpl()
    {
        lv2ResourceFolder.getFile().deleteRecursively();
    }

    void findAllTypesForFile (OwnedArray<PluginDescription>& result,
                              const String& identifier)
    {
        auto desc = getDescription (findPluginByUri (identifier));

        if (desc.fileOrIdentifier.isNotEmpty())
            result.add (std::make_unique<PluginDescription> (desc));
    }

    bool fileMightContainThisPluginType (const String& file) const
    {
        // If the string looks like a URI, then it could be a valid LV2 identifier
        const auto* data = file.toRawUTF8();
        const auto numBytes = file.getNumBytesAsUTF8();
        std::vector<uint8_t> vec (numBytes + 1, 0);
        std::copy (data, data + numBytes, vec.begin());
        return serd_uri_string_has_scheme (vec.data());
    }

    String getNameOfPluginFromIdentifier (const String& identifier)
    {
        // We would have to actually load the bundle to get its name,
        // and the bundle may contain multiple plugins
        return identifier;
    }

    bool pluginNeedsRescanning (const PluginDescription&)
    {
        return true;
    }

    bool doesPluginStillExist (const PluginDescription& description)
    {
        return findPluginByUri (description.fileOrIdentifier) != nullptr;
    }

    StringArray searchPathsForPlugins (const FileSearchPath& paths, bool, bool)
    {
        loadAllPluginsFromPaths (paths);

        StringArray result;

        for (const auto* plugin : world->getAllPlugins())
            result.add (lv2_host::Plugin { plugin }.getUri().getTyped());

        return result;
    }

    FileSearchPath getDefaultLocationsToSearch()
    {
      #if JUCE_MAC
        return { "~/Library/Audio/Plug-Ins/LV2;"
                 "~/.lv2;"
                 "/usr/local/lib/lv2;"
                 "/usr/lib/lv2;"
                 "/Library/Audio/Plug-Ins/LV2;" };
      #elif JUCE_WINDOWS
        return { "%APPDATA%\\LV2;"
                 "%COMMONPROGRAMFILES%\\LV2" };
      #else
       #if JUCE_64BIT
        if (File ("/usr/lib64/lv2").exists() || File ("/usr/local/lib64/lv2").exists())
            return { "~/.lv2;"
                     "/usr/lib64/lv2;"
                     "/usr/local/lib64/lv2" };
       #endif

        return { "~/.lv2;"
                 "/usr/lib/lv2;"
                 "/usr/local/lib/lv2" };
      #endif
    }

    const LilvUI* findEmbeddableUi (const lv2_host::Uis* pluginUis, std::true_type)
    {
        if (pluginUis == nullptr)
            return nullptr;

        const std::vector<const LilvUI*> allUis (pluginUis->begin(), pluginUis->end());

        if (allUis.empty())
            return nullptr;

        constexpr const char* rawUri =
               #if JUCE_MAC
                LV2_UI__CocoaUI;
               #elif JUCE_WINDOWS
                LV2_UI__WindowsUI;
               #elif JUCE_LINUX || JUCE_BSD
                LV2_UI__X11UI;
               #else
                nullptr;
               #endif

        jassert (rawUri != nullptr);
        const auto nativeUiUri = world->newUri (rawUri);

        struct UiWithSuitability
        {
            const LilvUI* ui;
            unsigned suitability;

            bool operator< (const UiWithSuitability& other) const noexcept
            {
                return suitability < other.suitability;
            }

            static unsigned uiIsSupported (const char* hostUri, const char* pluginUri)
            {
                if (strcmp (hostUri, pluginUri) == 0)
                    return 1;

                return 0;
            }
        };

        std::vector<UiWithSuitability> uisWithSuitability;
        uisWithSuitability.reserve (allUis.size());

        std::transform (allUis.cbegin(), allUis.cend(), std::back_inserter (uisWithSuitability), [&] (const LilvUI* ui)
        {
            const LilvNode* type = nullptr;
            return UiWithSuitability { ui, lilv_ui_is_supported (ui, UiWithSuitability::uiIsSupported, nativeUiUri.get(), &type) };
        });

        std::sort (uisWithSuitability.begin(), uisWithSuitability.end());

        if (uisWithSuitability.back().suitability != 0)
            return uisWithSuitability.back().ui;

        return nullptr;
    }

    const LilvUI* findEmbeddableUi (const lv2_host::Uis*, std::false_type)
    {
        return nullptr;
    }

    const LilvUI* findEmbeddableUi (const lv2_host::Uis* pluginUis)
    {
        return findEmbeddableUi (pluginUis, std::integral_constant<bool, lv2_host::editorFunctionalityEnabled>{});
    }

    static lv2_host::UiDescriptor getUiDescriptor (const LilvUI* ui)
    {
        if (ui == nullptr)
            return {};

        const auto libraryFile = StringPtr { lilv_file_uri_parse (lilv_node_as_uri (lilv_ui_get_binary_uri (ui)), nullptr) };

        return lv2_host::UiDescriptor { lv2_host::UiDescriptorArgs{}.withLibraryPath (libraryFile.get())
                                                                    .withUiUri (lilv_node_as_uri (lilv_ui_get_uri (ui))) };
    }

    // Returns the name of a missing feature, if any.
    template <typename RequiredFeatures, typename AvailableFeatures>
    static std::vector<String> findMissingFeatures (RequiredFeatures&& required,
                                                    AvailableFeatures&& available)
    {
        std::vector<String> result;

        for (const auto* node : required)
        {
            const auto nodeString = String::fromUTF8 (lilv_node_as_uri (node));

            if (std::find (std::begin (available), std::end (available), nodeString) == std::end (available))
                result.push_back (nodeString);
        }

        return result;
    }

    void createPluginInstance (const PluginDescription& desc,
                               double initialSampleRate,
                               int initialBufferSize,
                               PluginCreationCallback callback)
    {
        const auto* pluginPtr = findPluginByUri (desc.fileOrIdentifier);

        if (pluginPtr == nullptr)
            return callback (nullptr, "Unable to locate plugin with the requested URI");

        const lv2_host::Plugin plugin { pluginPtr };

        auto symap = std::make_unique<lv2_host::SymbolMap>();

        const auto missingFeatures = findMissingFeatures (plugin.getRequiredFeatures(),
                                                          lv2_host::FeaturesData::getFeatureUris());

        if (! missingFeatures.empty())
        {
            const auto missingFeaturesString = StringArray (missingFeatures.data(), (int) missingFeatures.size()).joinIntoString (", ");

            return callback (nullptr, "plugin requires missing features: " + missingFeaturesString);
        }

        auto stateToApply = [&]
        {
            if (! plugin.hasFeature (world->newUri (LV2_STATE__loadDefaultState)))
                return lv2_host::PluginState{};

            auto map = symap->getMapFeature();
            return lv2_host::PluginState { lilv_state_new_from_world (world->get(), &map, plugin.getUri().get()) };
        }();

        auto ports = lv2_host::Ports::getPorts (*world, uris, plugin, *symap);

        if (! ports.hasValue())
            return callback (nullptr, "Plugin has ports of an unsupported type");

        auto instance = std::make_unique<lv2_host::InstanceWithSupports> (*world,
                                                                          std::move (symap),
                                                                          plugin,
                                                                          std::move (*ports),
                                                                          (int32_t) initialBufferSize,
                                                                          initialSampleRate);

        if (instance->instance == nullptr)
            return callback (nullptr, "Plugin was located, but could not be opened");

        auto potentialPresets = world->findNodes (nullptr,
                                                  world->newUri (LV2_CORE__appliesTo).get(),
                                                  plugin.getUri().get());

        const lv2_host::Uis pluginUis { plugin.get() };

        const auto uiToUse = [&]() -> const LilvUI*
        {
            const auto bestMatch = findEmbeddableUi (&pluginUis);

            if (bestMatch == nullptr)
                return bestMatch;

            const auto uiUri = lilv_ui_get_uri (bestMatch);
            lilv_world_load_resource (world->get(), uiUri);

            const auto queryUi = [&] (const char* featureUri)
            {
                const auto featureUriNode = world->newUri (featureUri);
                return world->findNodes (uiUri, featureUriNode.get(), nullptr);
            };

            const auto missingUiFeatures = findMissingFeatures (queryUi (LV2_CORE__requiredFeature),
                                                                lv2_host::UiFeaturesData::getFeatureUris());

            return missingUiFeatures.empty() ? bestMatch : nullptr;
        }();

        auto uiBundleUri = uiToUse != nullptr ? String::fromUTF8 (lilv_node_as_uri (lilv_ui_get_bundle_uri (uiToUse)))
                                              : String();

        auto wrapped = std::make_unique<lv2_host::LV2AudioPluginInstance> (world,
                                                                           plugin,
                                                                           uris,
                                                                           std::move (instance),
                                                                           getDescription (pluginPtr),
                                                                           findPresetUrisForPlugin (plugin.get()),
                                                                           std::move (stateToApply),
                                                                           std::move (uiBundleUri),
                                                                           getUiDescriptor (uiToUse));
        callback (std::move (wrapped), {});
    }

private:
    void loadAllPluginsFromPaths (const FileSearchPath& path)
    {
        const auto joined = path.toStringWithSeparator (LILV_PATH_SEP);
        world->loadAllFromPaths (world->newString (joined.toRawUTF8()));
    }

    struct Free { void operator() (char* ptr) const noexcept { free (ptr); } };
    using StringPtr = std::unique_ptr<char, Free>;

    const LilvPlugin* findPluginByUri (const String& s)
    {
        return world->getAllPlugins().getByUri (world->newUri (s.toRawUTF8()));
    }

    template <typename Fn>
    void visitParentClasses (const LilvPluginClass* c, Fn&& fn) const
    {
        if (c == nullptr)
            return;

        const lv2_host::PluginClass wrapped { c };
        fn (wrapped);

        const auto parentUri = wrapped.getParentUri();

        if (parentUri.get() != nullptr)
            visitParentClasses (world->getPluginClasses().getByUri (parentUri), fn);
    }

    std::vector<lv2_host::NodeUri> collectPluginClassUris (const LilvPluginClass* c) const
    {
        std::vector<lv2_host::NodeUri> results;

        visitParentClasses (c, [&results] (const lv2_host::PluginClass& wrapped)
        {
            results.emplace_back (wrapped.getUri());
        });

        return results;
    }

    PluginDescription getDescription (const LilvPlugin* plugin)
    {
        if (plugin == nullptr)
            return {};

        const auto wrapped      = lv2_host::Plugin { plugin };
        const auto bundle       = wrapped.getBundleUri().getTyped();
        const auto bundleFile   = File { StringPtr { lilv_file_uri_parse (bundle, nullptr) }.get() };

        const auto numInputs  = wrapped.getNumPortsOfClass (uris.mLV2_CORE__AudioPort, uris.mLV2_CORE__InputPort);
        const auto numOutputs = wrapped.getNumPortsOfClass (uris.mLV2_CORE__AudioPort, uris.mLV2_CORE__OutputPort);

        PluginDescription result;
        result.name                 = wrapped.getName().getTyped();
        result.descriptiveName      = wrapped.getName().getTyped();
        result.lastFileModTime      = bundleFile.getLastModificationTime();
        result.lastInfoUpdateTime   = Time::getCurrentTime();
        result.manufacturerName     = wrapped.getAuthorName().getTyped();
        result.pluginFormatName     = LV2PluginFormat::getFormatName();
        result.numInputChannels     = static_cast<int> (numInputs);
        result.numOutputChannels    = static_cast<int> (numOutputs);

        const auto classPtr     = wrapped.getClass();
        const auto classes      = collectPluginClassUris (classPtr);
        const auto isInstrument = std::any_of (classes.cbegin(),
                                               classes.cend(),
                                               [this] (const lv2_host::NodeUri& uri)
                                               {
                                                   return uri.equals (uris.mLV2_CORE__GeneratorPlugin);
                                               });

        result.category         = lv2_host::PluginClass { classPtr }.getLabel().getTyped();
        result.isInstrument     = isInstrument;

        // The plugin URI is required to be globally unique, so a hash of it should be too
        result.fileOrIdentifier = wrapped.getUri().getTyped();

        const auto uid = DefaultHashFunctions::generateHash (result.fileOrIdentifier, std::numeric_limits<int>::max());;
        result.deprecatedUid = result.uniqueId = uid;
        return result;
    }

    std::vector<String> findPresetUrisForPlugin (const LilvPlugin* plugin)
    {
        std::vector<String> presetUris;

        lv2_host::OwningNodes potentialPresets { lilv_plugin_get_related (plugin, world->newUri (LV2_PRESETS__Preset).get()) };

        for (const auto* potentialPreset : potentialPresets)
            presetUris.push_back (lilv_node_as_string (potentialPreset));

        return presetUris;
    }

    TemporaryFile lv2ResourceFolder;
    std::shared_ptr<lv2_host::World> world = std::make_shared<lv2_host::World>();
    lv2_host::UsefulUris uris { world->get() };
};

//==============================================================================
LV2PluginFormat::LV2PluginFormat()
    : pimpl (std::make_unique<Pimpl>()) {}

LV2PluginFormat::~LV2PluginFormat() = default;

void LV2PluginFormat::findAllTypesForFile (OwnedArray<PluginDescription>& results,
                                           const String& fileOrIdentifier)
{
    pimpl->findAllTypesForFile (results, fileOrIdentifier);
}

bool LV2PluginFormat::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    return pimpl->fileMightContainThisPluginType (fileOrIdentifier);
}

String LV2PluginFormat::getNameOfPluginFromIdentifier (const String& fileOrIdentifier)
{
    return pimpl->getNameOfPluginFromIdentifier (fileOrIdentifier);
}

bool LV2PluginFormat::pluginNeedsRescanning (const PluginDescription& desc)
{
    return pimpl->pluginNeedsRescanning (desc);
}

bool LV2PluginFormat::doesPluginStillExist (const PluginDescription& desc)
{
    return pimpl->doesPluginStillExist (desc);
}

bool LV2PluginFormat::canScanForPlugins() const { return true; }
bool LV2PluginFormat::isTrivialToScan() const { return true; }

StringArray LV2PluginFormat::searchPathsForPlugins (const FileSearchPath& directoriesToSearch,
                                                    bool recursive,
                                                    bool allowAsync)
{
    return pimpl->searchPathsForPlugins (directoriesToSearch, recursive, allowAsync);
}

FileSearchPath LV2PluginFormat::getDefaultLocationsToSearch()
{
    return pimpl->getDefaultLocationsToSearch();
}

bool LV2PluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const
{
    return false;
}

void LV2PluginFormat::createPluginInstance (const PluginDescription& desc,
                                            double sampleRate,
                                            int bufferSize,
                                            PluginCreationCallback callback)
{
    pimpl->createPluginInstance (desc, sampleRate, bufferSize, std::move (callback));
}

} // namespace juce

#endif
