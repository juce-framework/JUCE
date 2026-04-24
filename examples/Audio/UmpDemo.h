/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             UmpDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Demonstrates techniques for sending and receiving MIDI
                   messages in Universal MIDI Packet format.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra, juce_audio_processors_headless
 exporters:        xcode_mac, vs2022, vs2026, linux_make, androidstudio,
                   xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1 JUCE_USE_WINRT_MIDI=1

 type:             Component
 mainClass:        UmpDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

static std::unique_ptr<Label> makeListRowLabel (StringRef text)
{
    auto label = std::make_unique<Label>();
    label->setText (text, dontSendNotification);
    label->setFont (FontOptions { Font::getDefaultMonospacedFontName(), 12, 0 });
    label->setMinimumHorizontalScale (1.0f);
    label->setInterceptsMouseClicks (false, false);
    return label;
}

//==============================================================================
struct InputCallback
{
    virtual ~InputCallback() = default;
    virtual void inputReceived (const ump::EndpointId&, ump::Iterator, ump::Iterator, double) = 0;
};

//==============================================================================
struct EndpointRowModelListener
{
    virtual ~EndpointRowModelListener() = default;
    virtual void needsRepaint() = 0;
};

class EndpointRowModel : private ump::Consumer,
                         private Timer
{
public:
    EndpointRowModel (const ump::EndpointId& i, InputCallback& cb)
        : id (i), callback (cb) {}

    ~EndpointRowModel() override
    {
        input.removeConsumer (*this);
    }

    void connectInput (ump::Session s)
    {
        input = s.connectInput (id, ump::PacketProtocol::MIDI_2_0);
        input.addConsumer (*this);
    }

    void disconnectInput()
    {
        input = {};
    }

    void connectOutput (ump::Session s)
    {
        output = s.connectOutput (id);
    }

    void disconnectOutput()
    {
        output = {};
    }

    void send (ump::Iterator b, ump::Iterator e)
    {
        if (output.isAlive())
        {
            output.send (b, e);
            dstLight = 1.0f;
            startTimerHz (60);
        }
    }

    ump::EndpointId getId() const { return id; }
    bool isInputConnected() const { return input.isAlive(); }
    bool isOutputConnected() const { return output.isAlive(); }

    float getSrcLight() const { return srcLight; }
    float getDstLight() const { return dstLight; }

    void addListener (EndpointRowModelListener& l)
    {
        listeners.add (&l);
    }

    void removeListener (EndpointRowModelListener& l)
    {
        listeners.remove (&l);
    }

private:
    void consume (ump::Iterator b, ump::Iterator e, double t) override
    {
        srcLight = 1.0f;
        callback.inputReceived (id, b, e, t);
        startTimerHz (60);
    }

    void timerCallback() override
    {
        constexpr auto coeff = 0.9f;
        srcLight *= coeff;
        dstLight *= coeff;

        constexpr auto limit = 0.01f;

        if (srcLight < limit && dstLight < limit)
            stopTimer();

        listeners.call ([] (auto& l)
        {
            l.needsRepaint();
        });
    }

    float srcLight{}, dstLight{};
    ump::EndpointId id;
    ump::Input input;
    ump::Output output;
    ListenerList<EndpointRowModelListener> listeners;
    InputCallback& callback;
};

class EndpointRowComponent : public Component,
                             private EndpointRowModelListener
{
public:
    EndpointRowComponent (ump::Session s, EndpointRowModel& m, bool select)
        : session (s), model (m), selected (select)
    {
        addAndMakeVisible (nameLabel);
        addAndMakeVisible (inputButton);
        addAndMakeVisible (outputButton);

        inputButton.setClickingTogglesState (true);
        outputButton.setClickingTogglesState (true);

        inputButton.onClick = [this]
        {
            if (inputButton.getToggleState())
                model.connectInput (session);
            else
                model.disconnectInput();

            updateButtonState();
        };

        outputButton.onClick = [this]
        {
            if (outputButton.getToggleState())
                model.connectOutput (session);
            else
                model.disconnectOutput();

            updateButtonState();
        };

        updateButtonState();

        nameLabel.setInterceptsMouseClicks (false, false);
        setInterceptsMouseClicks (false, true);

        m.addListener (*this);
        setButtonColours();

        auto* endpoints = ump::Endpoints::getInstance();

        if (const auto endpoint = endpoints->getEndpoint (model.getId()))
        {
            nameLabel.setText (endpoint->getName(), dontSendNotification);
            inputButton.setVisible (hasFunctionBlockInDirection (*endpoint, ump::IOKind::src));
            outputButton.setVisible (hasFunctionBlockInDirection (*endpoint, ump::IOKind::dst));
        }
        else if (const auto info = endpoints->getStaticDeviceInfo (model.getId()))
        {
            nameLabel.setText (info->getName(), dontSendNotification);
            inputButton.setVisible (info->hasSource());
            outputButton.setVisible (info->hasDestination());
        }
    }

    ~EndpointRowComponent() override
    {
        model.removeListener (*this);
    }

    void resized() override
    {
        FlexBox fb;
        fb.items = { FlexItem { nameLabel }.withFlex (1.0f),
                     FlexItem { inputButton }.withWidth (50).withMargin ({ 2 }),
                     FlexItem { outputButton }.withWidth (50).withMargin ({ 2 }) };
        fb.performLayout (getLocalBounds());
    }

    void paint (Graphics& g) override
    {
        if (selected)
            g.fillAll (findColour (TextEditor::ColourIds::highlightColourId));
    }

private:
    void setButtonColours()
    {
        inputButton .setColour (ToggleButton::tickDisabledColourId,
                                Colours::white.interpolatedWith (Colours::limegreen, model.getSrcLight()));
        outputButton.setColour (ToggleButton::tickDisabledColourId,
                                Colours::white.interpolatedWith (Colours::limegreen, model.getDstLight()));

    }

    void needsRepaint() override
    {
        setButtonColours();
        repaint();
    }

    static bool matches (ump::BlockUiHint ui, ump::IOKind dir)
    {
        if (ui == ump::BlockUiHint::bidirectional)
            return true;

        if (dir == ump::IOKind::src)
            return ui == ump::BlockUiHint::sender;

        return ui == ump::BlockUiHint::receiver;
    }

    static bool matches (ump::BlockDirection bd, ump::IOKind dir)
    {
        if (bd == ump::BlockDirection::bidirectional)
            return true;

        if (dir == ump::IOKind::src)
            return bd == ump::BlockDirection::sender;

        return bd == ump::BlockDirection::receiver;
    }

    static bool hasFunctionBlockInDirection (const ump::Endpoint& e, ump::IOKind direction)
    {
        const auto blocks = e.getBlocks();

        return std::any_of (blocks.begin(), blocks.end(), [&] (const ump::Block& x)
        {
            return matches (x.getUiHint(), direction) || matches (x.getDirection(), direction);
        });
    }

    void updateButtonState()
    {
        inputButton.setToggleState (model.isInputConnected(), dontSendNotification);
        outputButton.setToggleState (model.isOutputConnected(), dontSendNotification);
    }

    ump::Session session;
    EndpointRowModel& model;
    Label nameLabel;
    ToggleButton inputButton { "in" }, outputButton { "out" };
    bool selected;
};

struct IOPickerModelDelegate
{
    virtual ~IOPickerModelDelegate() = default;
    virtual void selectedRowsChanged() = 0;
};

class UmpIOPickerModel : public ListBoxModel
{
public:
    using Delegate = IOPickerModelDelegate;

    UmpIOPickerModel (ump::Session s, Delegate& cb, InputCallback& ic)
        : delegate (cb), callback (ic), session (s)
    {
    }

    int getNumRows() override
    {
        return (int) model.size();
    }

    void paintListBoxItem (int, Graphics&, int, int, bool) override {}

    Component* refreshComponentForRow (int rowNumber,
                                       bool rowIsSelected,
                                       Component* existingComponentToUpdate) override
    {
        const auto toDelete = rawToUniquePtr (existingComponentToUpdate);

        if (isPositiveAndBelow (rowNumber, model.size()))
            return new EndpointRowComponent { session, *model[(size_t) rowNumber], rowIsSelected };

        return nullptr;
    }

    void selectedRowsChanged (int) override
    {
        delegate.selectedRowsChanged();
    }

    [[nodiscard]] std::vector<std::unique_ptr<EndpointRowModel>> update()
    {
        std::map<ump::EndpointId, std::unique_ptr<EndpointRowModel>> toKeep;

        for (auto& m : model)
            toKeep.emplace (m->getId(), std::move (m));

        model.clear();

        auto* singleton = ump::Endpoints::getInstance();
        auto ids = singleton->getEndpoints();

        for (auto id : ids)
        {
            const auto iter = toKeep.find (id);
            model.push_back (iter != toKeep.end()
                             ? std::move (iter->second)
                             : rawToUniquePtr (new EndpointRowModel { id, callback }));
        }

        std::vector<std::unique_ptr<EndpointRowModel>> remainder;

        for (auto& pair : toKeep)
            if (pair.second != nullptr)
                remainder.push_back (std::move (pair.second));

        return remainder;
    }

    void sendPacketToAllOutputs (ump::View v)
    {
        const ump::Iterator begin { v.data(), v.size() };
        const auto end = std::next (begin);

        for (auto& item : model)
            item->send (begin, end);
    }

    ump::EndpointId getIdForIndex (int index) const
    {
        return model[(size_t) index]->getId();
    }

    int getIndexForId (const ump::EndpointId& i) const
    {
        return (int) std::distance (model.begin(),
                                    std::find_if (model.begin(),
                                                  model.end(),
                                                  [&] (auto& x) { return x->getId() == i; }));
    }

private:
    Delegate& delegate;
    InputCallback& callback;
    ump::Session session;
    std::vector<std::unique_ptr<EndpointRowModel>> model;
};

//==============================================================================
class IOPicker : public Component
{
public:
    IOPicker (ump::Session& s, IOPickerModelDelegate& d, InputCallback& cb)
        : model { s, d, cb }
    {
        refreshContent();

        addAndMakeVisible (list);
    }

    void resized() override
    {
        list.setBounds (getLocalBounds());
    }

    void sendPacketToAllOutputs (ump::View v)
    {
        model.sendPacketToAllOutputs (v);
    }

    std::optional<ump::EndpointId> getSelectedId() const
    {
        const auto selectedRow = list.getSelectedRow();
        return 0 <= selectedRow ? std::make_optional (model.getIdForIndex (selectedRow)) : std::nullopt;
    }

    void refreshContent()
    {
        const auto selectedId = getSelectedId();

        {
            // All model entries need to outlive all list row components, so we keep the unused
            // model entries alive until after list.updateContent() has returned.
            const auto remainder = model.update();
            list.updateContent();
        }

        if (selectedId.has_value())
            list.selectRow (model.getIndexForId (*selectedId), dontSendNotification);
    }

private:
    UmpIOPickerModel model;
    ListBox list { "", &model };
};

//==============================================================================
class UmpInfoView : public Component
{
public:
    UmpInfoView()
    {
        text.setFont (FontOptions{}.withName (Font::getDefaultMonospacedFontName()));
        text.setReadOnly (true);
        text.setCaretVisible (false);
        text.setMultiLine (true, false);
        text.setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
        text.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
        text.setColour (TextEditor::shadowColourId, Colours::transparentBlack);
        displayInfo (std::nullopt);

        addAndMakeVisible (text);
    }

    void resized() override
    {
        text.setBounds (getLocalBounds());
    }

    void displayInfo (const std::optional<ump::EndpointId>& id)
    {
        auto* endpoints = ump::Endpoints::getInstance();

        const auto description = std::invoke ([&]
        {
            if (! id.has_value())
                return String();

            return getInfoString (endpoints->getStaticDeviceInfo (*id), endpoints->getEndpoint (*id));
        });

        const auto makePlaceholder = [&]
        {
            return "Current backend: " + format (endpoints->getBackend()) + "\n\n"
                   "Select an item above to see details.\n\n"
                   "Connect using the 'in' and 'out' toggles.\n\n"
                   "View MIDI traffic below.\n\n"
                   "Play the keyboard to send messages.\n";
        };

        text.setText (description.isNotEmpty() ? description : makePlaceholder());
    }

private:
    struct Field
    {
        String key, value;
    };

    struct Separator {};

    using Row = std::variant<Field, Separator>;

    static String formatTable (Span<const Row> rows)
    {
        MemoryOutputStream stream;
        stream.setNewLineString ("\n");

        constexpr auto extraPadding = 2;

        const auto leftColumnWidth = extraPadding + std::invoke ([&]
        {
            int maxWidth = 0;

            for (auto& row : rows)
                if (auto* field = std::get_if<Field> (&row))
                    maxWidth = std::max (maxWidth, field->key.length());

            return maxWidth;
        });

        MemoryOutputStream separator;
        separator.setNewLineString ("\n");
        separator << newLine << '|';

        for (auto i = 1; i < extraPadding; ++i)
            separator << ' ';

        for (const auto& row : rows)
        {
            if (std::get_if<Separator> (&row) != nullptr)
            {
                if (stream.getPosition() != 0)
                    stream << newLine;
            }
            else if (auto* field = std::get_if<Field> (&row))
            {
                stream << field->key;

                if (field->value.endsWithChar ('\n'))
                {
                    auto lines = StringArray::fromLines (field->value);
                    lines.removeEmptyStrings();

                    for (const auto& line : lines)
                        stream << separator.toString() << line;

                    stream << newLine;
                }
                else
                {
                    for (auto i = field->key.length(); i < leftColumnWidth; ++i)
                        stream << ' ';

                    stream << field->value << newLine;
                }
            }
        }

        return stream.toString();
    }

    static String getInfoString (const std::optional<ump::StaticDeviceInfo>& i,
                                 const std::optional<ump::Endpoint>& e)
    {
        std::vector<Row> rows;

        const auto push = [&] (auto key, const auto& value)
        {
            rows.push_back (Field { key, format (value) });
        };

        if (i.has_value())
        {
            rows.push_back (Separator{});
            push ("device name", i->getName());
            push ("device manufacturer", i->getManufacturer());
            push ("product name", i->getProduct());
            push ("transport", i->getTransport());
        }
        else
        {
            push ("WARNING", "no static device info");
        }

        if (e.has_value())
        {
            rows.push_back (Separator{});
            push ("name", e->getName());
            push ("protocol", e->getProtocol());
            push ("product instance", e->getProductInstanceId());
            push ("supports MIDI 1.0", e->hasMidi1Support());
            push ("supports MIDI 2.0", e->hasMidi2Support());
            push ("supports txjr", e->hasTransmitJRSupport());
            push ("supports rxjr", e->hasReceiveJRSupport());
            push ("static function blocks", e->hasStaticBlocks());

            rows.push_back (Separator{});

            for (const auto [index, block] : enumerate (e->getBlocks(), 0))
                push ("block " + String (index), block);
        }
        else
        {
            push ("", "connect to the device to fetch more info");
        }

        return formatTable (rows);
    }

    static String format (const String& x)
    {
        return x;
    }

    static String format (const char* x)
    {
        return x;
    }

    static String format (bool x)
    {
        return x ? "true" : "false";
    }

    static String format (int x)
    {
        return String (x);
    }

    static String format (ump::BlockMIDI1ProxyKind x)
    {
        switch (x)
        {
            case ump::BlockMIDI1ProxyKind::inapplicable:
                return "n/a";
            case ump::BlockMIDI1ProxyKind::restrictedBandwidth:
                return "MIDI 1.0 slow";
            case ump::BlockMIDI1ProxyKind::unrestrictedBandwidth:
                return "MIDI 1.0 fast";
        }

        return {};
    }

    static String format (ump::BlockDirection x)
    {
        switch (x)
        {
            case ump::BlockDirection::unknown:
                return "unknown";
            case ump::BlockDirection::bidirectional:
                return "bidirectional";
            case ump::BlockDirection::sender:
                return "sender";
            case ump::BlockDirection::receiver:
                return "receiver";
        }

        return {};
    }

    static String format (ump::BlockUiHint x)
    {
        switch (x)
        {
            case ump::BlockUiHint::unknown:
                return "unknown";
            case ump::BlockUiHint::bidirectional:
                return "bidirectional";
            case ump::BlockUiHint::sender:
                return "sender";
            case ump::BlockUiHint::receiver:
                return "receiver";
        }

        return {};
    }

    static String format (ump::PacketProtocol x)
    {
        return x == ump::PacketProtocol::MIDI_2_0 ? "MIDI 2.0" : "MIDI 1.0";
    }

    static String format (ump::Transport x)
    {
        return x == ump::Transport::ump ? "UMP" : "bytestream";
    }

    template <typename T>
    static String format (const std::optional<T>& x)
    {
        return x.has_value() ? format (*x) : "unknown";
    }

    static String format (const ump::Block& x)
    {
        std::vector<Row> rows;

        const auto push = [&] (auto key, const auto& value)
        {
            rows.push_back (Field { key, format (value) });
        };

        push ("name", x.getName());
        push ("enabled", x.isEnabled());
        push ("first group (zero-based)", x.getFirstGroup());
        push ("num groups", x.getNumGroups());
        push ("max num sysex 8 streams", x.getMaxSysex8Streams());
        push ("MIDI 1.0 proxy", x.getMIDI1ProxyKind());
        push ("UI Hint", x.getUiHint());
        push ("direction", x.getDirection());

        return formatTable (rows);
    }

    static String format (ump::Backend b)
    {
        switch (b)
        {
            case ump::Backend::alsa:        return "ALSA";
            case ump::Backend::android:     return "Android";
            case ump::Backend::coremidi:    return "CoreMIDI";
            case ump::Backend::winmm:       return "WinMM";
            case ump::Backend::winrt:       return "Legacy WinRT";
            case ump::Backend::wms:         return "Windows MIDI Services";
        }

        return {};
    }

    TextEditor text;
};

//==============================================================================
enum class Direction
{
    in,
    out,
};

struct LogEntry
{
    LogEntry (ump::View v, const String& p, uint32 t, Direction d)
        : port (p), millis (t), direction (d)
    {
        std::copy (v.begin(), v.end(), packet.begin());
    }

    ump::View getView() const { return ump::View { packet.data() }; }

    std::array<uint32_t, 4> packet;
    String port;
    uint32 millis;
    Direction direction;
};

//==============================================================================
class LoggingData : private AsyncUpdater
{
public:
    explicit LoggingData (std::function<void()> cb)
        : onChange (std::move (cb))
    {
    }

    template <typename It>
    void addMessages (It begin, It end)
    {
        if (begin == end)
            return;

        {
            const std::scoped_lock lock { mutex };

            const auto numNewMessages = (int) std::distance (begin, end);
            const auto numToAdd = juce::jmin (numToStore, numNewMessages);
            const auto numToRemove = jmax (0, (int) messages.size() + numToAdd - numToStore);
            messages.erase (messages.begin(), std::next (messages.begin(), numToRemove));
            messages.insert (messages.end(), std::prev (end, numToAdd), end);
        }

        updateListener();
    }

    void clear()
    {
        {
            const std::scoped_lock lock { mutex };
            messages.clear();
        }

        updateListener();
    }

    void setFilter (std::optional<Direction> d)
    {
        {
            const std::scoped_lock lock { mutex };
            filter = d;
        }

        updateListener();
    }

    std::optional<Direction> getFilter() const
    {
        const std::scoped_lock lock { mutex };
        return filter;
    }

    std::deque<LogEntry> getEntries() const
    {
        auto [messagesCopy, filterCopy] = std::invoke ([this]
        {
            const std::scoped_lock lock { mutex };
            return std::tuple (messages, filter);
        });

        if (! filterCopy.has_value())
            return messagesCopy;

        const auto iter = std::remove_if (messagesCopy.begin(),
                                          messagesCopy.end(),
                                          [f = *filterCopy] (const auto& e) { return e.direction != f; });
        messagesCopy.erase (iter, messagesCopy.end());
        return messagesCopy;
    }

private:
    void updateListener()
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
            handleAsyncUpdate();
        else
            triggerAsyncUpdate();
    }

    void handleAsyncUpdate() override
    {
        NullCheckedInvocation::invoke (onChange);
    }

    static constexpr auto numToStore = 1000;
    mutable std::mutex mutex;
    std::deque<LogEntry> messages;
    std::optional<Direction> filter;
    std::function<void()> onChange;
};

//==============================================================================
class UmpLoggingModel : public TableListBoxModel
{
public:
    enum Columns
    {
        messageTime = 1,
        direction,
        port,
        words,
        description,
    };

    explicit UmpLoggingModel (LoggingData& s) : state (s) {}

    Component* refreshComponentForCell (int rowNumber,
                                        int columnId,
                                        bool,
                                        Component* existingComponentToUpdate) override
    {
        auto owned = rawToUniquePtr (existingComponentToUpdate);

        if (! isPositiveAndBelow (rowNumber, cachedEntries.size()))
            return nullptr;

        auto ownedLabel = [&owned]
        {
            auto* label = dynamic_cast<Label*> (owned.get());

            if (label == nullptr)
                return makeListRowLabel ("");

            owned.release();
            return rawToUniquePtr (label);
        }();

        const auto& row = cachedEntries[(size_t) rowNumber];

        const auto text = [&]() -> String
        {
            switch (columnId)
            {
                case messageTime:
                    return String (row.millis);

                case direction:
                    return row.direction == Direction::in ? "in" : "out";

                case port:
                    return row.port;

                case words:
                    return ump::StringUtils::getHexString (row.getView());

                case description:
                    return ump::StringUtils::getDescription (row.getView());
            }

            return {};
        }();

        ownedLabel->setText (text, dontSendNotification);
        return ownedLabel.release();
    }

    int getNumRows() override { return (int) cachedEntries.size(); }

    void paintRowBackground (Graphics&, int, int, int, bool) override {}
    void paintCell (Graphics&, int, int, int, int, bool) override {}

    void updateCache()
    {
        cachedEntries = state.getEntries();
    }

private:
    LoggingData& state;
    std::deque<LogEntry> cachedEntries;
};

//==============================================================================
class UmpLoggingList : public Component,
                       private Timer
{
public:
    UmpLoggingList()
    {
        addAndMakeVisible (list);
        addAndMakeVisible (messageDirLabel);
        addAndMakeVisible (allButton);
        addAndMakeVisible (inButton);
        addAndMakeVisible (outButton);
        addAndMakeVisible (clearButton);

        allButton.setConnectedEdges (TextButton::ConnectedOnRight);
        inButton .setConnectedEdges (TextButton::ConnectedOnRight | TextButton::ConnectedOnLeft);
        outButton.setConnectedEdges (TextButton::ConnectedOnLeft);

        messageDirLabel.setJustificationType (Justification::right);

        auto& header = list.getHeader();
        header.addColumn ("Time",
                          UmpLoggingModel::Columns::messageTime,
                          100,
                          100);
        header.addColumn ("IO",
                          UmpLoggingModel::Columns::direction,
                          50,
                          50);
        header.addColumn ("Port",
                          UmpLoggingModel::Columns::port,
                          60,
                          50);
        header.addColumn ("Words",
                          UmpLoggingModel::Columns::words,
                          250,
                          50);
        header.addColumn ("Description",
                          UmpLoggingModel::Columns::description,
                          500,
                          50);

        allButton.onClick = [this] { state.setFilter (std::nullopt); };
        inButton .onClick = [this] { state.setFilter (Direction::in); };
        outButton.onClick = [this] { state.setFilter (Direction::out); };

        clearButton.onClick = [this] { state.clear(); };

        updateContent();
    }

    void resized() override
    {
        auto b = getLocalBounds();

        FlexBox fb;
        fb.items = { FlexItem { messageDirLabel }.withWidth (70),
                     FlexItem{}.withWidth (210),
                     FlexItem{}.withFlex (1.0f),
                     FlexItem { clearButton }.withWidth (70) };

        fb.performLayout (b.removeFromTop (30).reduced (4));

        doGridButtonLayout (fb.items[1].currentBounds, allButton, inButton, outButton);

        list.setBounds (b);
    }

    template <typename... Buttons>
    static void doGridButtonLayout (Rectangle<float> bounds, Buttons&... buttons)
    {
        Grid grid;
        grid.items = { GridItem { buttons }... };
        grid.autoFlow = Grid::AutoFlow::column;
        grid.autoColumns = Grid::TrackInfo { Grid::Fr { 1 } };
        grid.autoRows = Grid::TrackInfo { Grid::Fr { 1 } };
        grid.performLayout (bounds.getLargestIntegerWithin());
    }

    void addEntry (const LogEntry& entry)
    {
        state.addMessages (&entry, &entry + 1);
    }

private:
    void updateContent()
    {
        allButton.setToggleState (state.getFilter() == std::nullopt,   dontSendNotification);
        inButton .setToggleState (state.getFilter() == Direction::in,  dontSendNotification);
        outButton.setToggleState (state.getFilter() == Direction::out, dontSendNotification);

        // Using a timer here means that we only repaint the UI after there haven't been any
        // new messages for a while, which avoids doing redundant expensive list-layouts.
        startTimer (16);
    }

    void timerCallback() override
    {
        const auto& vbar = list.getVerticalScrollBar();
        const auto endShowing = vbar.getCurrentRange().getEnd() >= vbar.getMaximumRangeLimit();

        stopTimer();
        model.updateCache();
        list.updateContent();

        if (endShowing)
            list.scrollToEnsureRowIsOnscreen (list.getNumRows() - 1);
    }

    Label messageDirLabel { "", "display:" };
    TextButton allButton { "all" },
               inButton { "incoming" },
               outButton { "outgoing" },
               clearButton { "clear log" };
    LoggingData state { [this] { updateContent(); } };
    UmpLoggingModel model { state };
    TableListBox list { "Logs", &model };
};

//==============================================================================
class PanelHeader : public Component
{
public:
    explicit PanelHeader (const String& text)
        : label ("", text)
    {
        addAndMakeVisible (label);
        label.setJustificationType (Justification::centredLeft);
        addAndMakeVisible (dragGrip);
        dragGrip.setJustificationType (Justification::centredRight);
        setInterceptsMouseClicks (false, false);

        dragGrip.setColour (Label::textColourId, Colours::white.withAlpha (0.5f));
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black.withAlpha (0.5f));

        g.setColour (Colours::white.withAlpha (0.2f));
        g.drawHorizontalLine (0, 0, (float) getWidth());
        g.setColour (Colours::black.withAlpha (0.2f));
        g.drawHorizontalLine (getHeight() - 1, 0, (float) getWidth());
    }

    void resized() override
    {
        label.setBounds (getLocalBounds());
        dragGrip.setBounds (getLocalBounds());
    }

private:
    Label label;
    Label dragGrip { "", "=" };
};

//==============================================================================
class UmpDemo : public Component,
                private IOPickerModelDelegate,
                private InputCallback,
                private MidiKeyboardState::Listener,
                private ump::EndpointsListener,
                private ump::Consumer
{
public:
    UmpDemo()
    {
        state.addListener (this);

        constexpr auto headerSize = 24;

        panel.addPanel (-1, &ioPicker, false);
        panel.setCustomPanelHeader (&ioPicker, &headerIoPicker, false);
        panel.setPanelHeaderSize (&ioPicker, headerSize);

        panel.addPanel (-1, &infoView, false);
        panel.setCustomPanelHeader (&infoView, &headerInfo, false);
        panel.setPanelHeaderSize (&infoView, headerSize);

        panel.addPanel (-1, &log, false);
        panel.setCustomPanelHeader (&log, &headerLog, false);
        panel.setPanelHeaderSize (&log, headerSize);

        panel.addPanel (-1, &keyboard, false);
        panel.setCustomPanelHeader (&keyboard, &headerKeyboard, false);
        panel.setPanelHeaderSize (&keyboard, headerSize);

        panel.setMaximumPanelSize (&keyboard, 100);

        panel.setPanelSize (&ioPicker, 100, false);
        panel.setPanelSize (&infoView, 200, false);
        panel.setPanelSize (&log, 200, false);
        panel.setPanelSize (&keyboard, 100, true);

        addAndMakeVisible (panel);

        setSize (390, 844);

        ump::Endpoints::getInstance()->setVirtualMidiUmpServiceActive (true);
        ump::Endpoints::getInstance()->addListener (*this);

        updateVirtualPorts();
    }

    ~UmpDemo() override
    {
        ump::Endpoints::getInstance()->removeListener (*this);
    }

    void resized() override
    {
        panel.setBounds (getLocalBounds());
    }

private:
    void selectedRowsChanged() override
    {
        infoView.displayInfo (ioPicker.getSelectedId());
    }

    void endpointsChanged() override
    {
        ioPicker.refreshContent();
        infoView.displayInfo (ioPicker.getSelectedId());
    }

    void virtualMidiServiceActiveChanged() override
    {
        if (ump::Endpoints::getInstance()->isVirtualMidiUmpServiceActive())
        {
            if (! virtualEndpoint.isAlive())
                updateVirtualPorts();
        }
        else
        {
            virtualEndpoint = {};
            virtualInput = {};
            virtualOutput = {};
        }
    }

    void consume (ump::Iterator b, ump::Iterator e, double time) override
    {
        if (auto& c = virtualInput)
            inputReceived (c.getEndpointId(), b, e, time);
    }

    void inputReceived (const ump::EndpointId& endpoint, ump::Iterator b, ump::Iterator e, double time) override
    {
        const auto info = ump::Endpoints::getInstance()->getEndpoint (endpoint);
        const auto name = info.has_value() ? info->getName() : "unknown name";

        for (auto item : makeRange (b, e))
            log.addEntry ({ item, name, (uint32) (time * 1000), Direction::in });
    }

    void sendToAllOutputs (ump::View v)
    {
        log.addEntry ({ v, "all", Time::getMillisecondCounter(), Direction::out });
        ioPicker.sendPacketToAllOutputs (v);

        if (auto& c = virtualOutput)
        {
            const ump::Iterator begin { v.data(), v.size() };
            const auto end = std::next (begin);
            c.send (begin, end);
        }
    }

    void handleNoteOn (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
    {
        const auto v1 = ump::Factory::makeNoteOnV1 (0,
                                                    (uint8_t) midiChannel - 1,
                                                    (uint8_t) midiNoteNumber,
                                                    (uint8_t) (velocity * (1 << 7)));
        sendToAllOutputs (ump::View { v1.data() });
    }

    void handleNoteOff (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
    {
        const auto v1 = ump::Factory::makeNoteOffV1 (0,
                                                     (uint8_t) midiChannel - 1,
                                                     (uint8_t) midiNoteNumber,
                                                     (uint8_t) (velocity * (1 << 7)));
        sendToAllOutputs (ump::View { v1.data() });
    }

    void updateVirtualPorts()
    {
        const auto name = "UMPDemo Virtual Endpoint";
        const auto id = "JUCE-UMP-DEMO";
        const auto proto = ump::PacketProtocol::MIDI_1_0;
        const auto staticBlocks = ump::BlocksAreStatic::yes;

        const std::array blocks { ump::Block{}.withName ("Block A")
                                              .withDirection (ump::BlockDirection::bidirectional)
                                              .withUiHint (ump::BlockUiHint::bidirectional)
                                              .withEnabled (true)
                                              .withFirstGroup (0)
                                              .withNumGroups (1)
                                              .withMIDI1ProxyKind (ump::BlockMIDI1ProxyKind::inapplicable),
                                  ump::Block{}.withName ("Block B")
                                              .withDirection (ump::BlockDirection::bidirectional)
                                              .withUiHint (ump::BlockUiHint::bidirectional)
                                              .withEnabled (true)
                                              .withFirstGroup (1)
                                              .withNumGroups (1)
                                              .withMIDI1ProxyKind (ump::BlockMIDI1ProxyKind::inapplicable) };
        virtualEndpoint = session.createVirtualEndpoint (name, {}, id, proto, blocks, staticBlocks);

        if (! virtualEndpoint.isAlive())
            return;

        virtualInput = virtualEndpoint
                     ? session.connectInput (virtualEndpoint.getId(), ump::PacketProtocol::MIDI_2_0)
                     : ump::Input{};
        virtualInput.addConsumer (*this);

        virtualOutput = virtualEndpoint
                      ? session.connectOutput (virtualEndpoint.getId())
                      : ump::Output{};

        // If this is hit, we created a virtual endpoint but failed to connect to it
        jassert (virtualInput.isAlive() && virtualOutput.isAlive());
    }

    ump::Session session = ump::Endpoints::getInstance()->makeSession ("Demo Session");

    ump::VirtualEndpoint virtualEndpoint;
    ump::Input virtualInput;
    ump::Output virtualOutput;

    MidiKeyboardState state;

    PanelHeader headerIoPicker { "Endpoints" };
    PanelHeader headerInfo { "Info" };
    PanelHeader headerLog { "Log" };
    PanelHeader headerKeyboard { "Keyboard" };

    IOPicker ioPicker { session, *this, *this };
    UmpInfoView infoView;
    UmpLoggingList log;
    MidiKeyboardComponent keyboard { state, MidiKeyboardComponent::horizontalKeyboard };
    ConcertinaPanel panel;
};
