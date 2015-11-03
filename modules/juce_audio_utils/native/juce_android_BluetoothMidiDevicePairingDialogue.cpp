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


//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getMidiBluetoothAddresses, "getMidiBluetoothAddresses", "()[Ljava/lang/String;") \
 METHOD (pairBluetoothMidiDevice, "pairBluetoothMidiDevice", "(Ljava/lang/String;)Z") \
 METHOD (unpairBluetoothMidiDevice, "unpairBluetoothMidiDevice", "(Ljava/lang/String;)V") \
 METHOD (getHumanReadableStringForBluetoothAddress, "getHumanReadableStringForBluetoothAddress", "(Ljava/lang/String;)Ljava/lang/String;") \
 METHOD (isBluetoothDevicePaired, "isBluetoothDevicePaired", "(Ljava/lang/String;)Z")

DECLARE_JNI_CLASS (AndroidBluetoothManager, JUCE_ANDROID_ACTIVITY_CLASSPATH "$BluetoothManager");
#undef JNI_CLASS_MEMBERS

//==============================================================================
struct AndroidBluetoothMidiInterface
{
    static StringArray getBluetoothMidiDevicesNearby()
    {
        StringArray retval;

        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (android.activity.callObjectMethod (JuceAppActivity.getAndroidBluetoothManager));

        // if this is null then bluetooth is not enabled
        if (btManager.get() == nullptr)
            return StringArray();

        jobjectArray jDevices = (jobjectArray) env->CallObjectMethod (btManager.get(),
                                                                      AndroidBluetoothManager.getMidiBluetoothAddresses);
        LocalRef<jobjectArray> devices (jDevices);

        const int count = env->GetArrayLength (devices.get());

        for (int i = 0; i < count; ++i)
        {
            LocalRef<jstring> string ((jstring)  env->GetObjectArrayElement (devices.get(), i));
            retval.add (juceString (string));
        }

        return retval;
    }

    //==========================================================================
    static bool pairBluetoothMidiDevice (const String& bluetoothAddress)
    {
        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (android.activity.callObjectMethod (JuceAppActivity.getAndroidBluetoothManager));
        if (btManager.get() == nullptr)
            return false;

        jboolean result = env->CallBooleanMethod (btManager.get(), AndroidBluetoothManager.pairBluetoothMidiDevice,
                                                  javaString (bluetoothAddress).get());

        return result;
    }

    static void unpairBluetoothMidiDevice (const String& bluetoothAddress)
    {
        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (android.activity.callObjectMethod (JuceAppActivity.getAndroidBluetoothManager));

        if (btManager.get() != nullptr)
            env->CallVoidMethod (btManager.get(), AndroidBluetoothManager.unpairBluetoothMidiDevice,
                                 javaString (bluetoothAddress).get());
    }

    //==========================================================================
    static String getHumanReadableStringForBluetoothAddress (const String& address)
    {
        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (android.activity.callObjectMethod (JuceAppActivity.getAndroidBluetoothManager));

        if (btManager.get() == nullptr)
            return address;

        LocalRef<jstring> string ((jstring) env->CallObjectMethod (btManager.get(),
                                                                   AndroidBluetoothManager.getHumanReadableStringForBluetoothAddress,
                                                                   javaString (address).get()));


        if (string.get() == nullptr)
            return address;

        return juceString (string);
    }

    //==========================================================================
    static bool isBluetoothDevicePaired (const String& address)
    {
        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (android.activity.callObjectMethod (JuceAppActivity.getAndroidBluetoothManager));

        if (btManager.get() == nullptr)
            return false;

        return env->CallBooleanMethod (btManager.get(), AndroidBluetoothManager.isBluetoothDevicePaired,
                                       javaString (address).get());
    }
};

//==============================================================================
struct AndroidBluetoothMidiDevice
{
    enum ConnectionStatus
    {
        offline,
        connected,
        disconnected,
        connecting,
        disconnecting
    };

    AndroidBluetoothMidiDevice (String deviceName, String address, ConnectionStatus status)
        : name (deviceName), bluetoothAddress (address), connectionStatus (status)
    {
        // can't create a device without a valid name and bluetooth address!
        jassert (! name.isEmpty());
        jassert (! bluetoothAddress.isEmpty());
    }

    bool operator== (const AndroidBluetoothMidiDevice& other) const noexcept
    {
        return bluetoothAddress == other.bluetoothAddress;
    }

    bool operator!= (const AndroidBluetoothMidiDevice& other) const noexcept
    {
        return ! operator== (other);
    }

    const String name, bluetoothAddress;
    ConnectionStatus connectionStatus;
};

//==============================================================================
class AndroidBluetoothMidiDevicesListBox   : public ListBox,
                                             private ListBoxModel,
                                             private Timer
{
public:
    //==========================================================================
    AndroidBluetoothMidiDevicesListBox()
        : timerPeriodInMs (1000)
    {
        setRowHeight (40);
        setModel (this);
        setOutlineThickness (1);
        updateDeviceList();
        startTimer (timerPeriodInMs);
    }

    void pairDeviceThreadFinished() // callback from PairDeviceThread
    {
        updateDeviceList();
        startTimer (timerPeriodInMs);
    }

private:
    //==========================================================================
    typedef AndroidBluetoothMidiDevice::ConnectionStatus DeviceStatus;

    int getNumRows() override
    {
        return devices.size();
    }

    void paintListBoxItem (int rowNumber, Graphics& g,
                           int width, int height, bool rowIsSelected) override
    {
        if (isPositiveAndBelow (rowNumber, devices.size()))
        {
            const AndroidBluetoothMidiDevice& device = devices.getReference (rowNumber);
            const String statusString (getDeviceStatusString (device.connectionStatus));

            g.fillAll (Colours::white);

            const float xmargin = 3.0f;
            const float ymargin = 3.0f;
            const float fontHeight = 0.4f * height;
            const float deviceNameWidth = 0.6f * width;

            g.setFont (fontHeight);

            g.setColour (getDeviceNameFontColour (device.connectionStatus));
            g.drawText (device.name,
                        xmargin, ymargin,
                        deviceNameWidth - (2.0f * xmargin), height - (2.0f * ymargin),
                        Justification::topLeft, true);

            g.setColour (getDeviceStatusFontColour (device.connectionStatus));
            g.drawText (statusString,
                        deviceNameWidth + xmargin, ymargin,
                        width - deviceNameWidth - (2.0f * xmargin), height - (2.0f * ymargin),
                        Justification::topRight, true);

            g.setColour (Colours::grey);
            g.drawHorizontalLine (height - 1, xmargin, width);
        }
    }

    //==========================================================================
    static Colour getDeviceNameFontColour (DeviceStatus deviceStatus) noexcept
    {
        if (deviceStatus == AndroidBluetoothMidiDevice::offline)
            return Colours::grey;

        return Colours::black;
    }

    static Colour getDeviceStatusFontColour (DeviceStatus deviceStatus) noexcept
    {
        if (deviceStatus == AndroidBluetoothMidiDevice::offline
            || deviceStatus == AndroidBluetoothMidiDevice::connecting
            || deviceStatus == AndroidBluetoothMidiDevice::disconnecting)
            return Colours::grey;

        if (deviceStatus == AndroidBluetoothMidiDevice::connected)
            return Colours::green;

        return Colours::black;
    }

    static String getDeviceStatusString (DeviceStatus deviceStatus) noexcept
    {
        if (deviceStatus == AndroidBluetoothMidiDevice::offline)        return "Offline";
        if (deviceStatus == AndroidBluetoothMidiDevice::connected)      return "Connected";
        if (deviceStatus == AndroidBluetoothMidiDevice::disconnected)   return "Not connected";
        if (deviceStatus == AndroidBluetoothMidiDevice::connecting)     return "Connecting...";
        if (deviceStatus == AndroidBluetoothMidiDevice::disconnecting)  return "Disconnecting...";

        // unknown device state!
        jassertfalse;
        return "Status unknown";
    }

    //==========================================================================
    void listBoxItemClicked (int row, const MouseEvent&) override
    {
        const AndroidBluetoothMidiDevice& device = devices.getReference (row);

        if (device.connectionStatus == AndroidBluetoothMidiDevice::disconnected)
            disconnectedDeviceClicked (row);

        else if (device.connectionStatus == AndroidBluetoothMidiDevice::connected)
            connectedDeviceClicked (row);
    }

    void timerCallback() override
    {
        updateDeviceList();
    }

    //==========================================================================
    struct PairDeviceThread  : public Thread,
                               private AsyncUpdater
    {
        PairDeviceThread (const String& bluetoothAddressOfDeviceToPair,
                          AndroidBluetoothMidiDevicesListBox& ownerListBox)
            : Thread ("JUCE Bluetooth MIDI Device Pairing Thread"),
              bluetoothAddress (bluetoothAddressOfDeviceToPair),
              owner (&ownerListBox)
        {
            startThread();
        }

        void run() override
        {
            AndroidBluetoothMidiInterface::pairBluetoothMidiDevice (bluetoothAddress);
            triggerAsyncUpdate();
        }

        void handleAsyncUpdate() override
        {
            if (owner != nullptr)
                owner->pairDeviceThreadFinished();

            delete this;
        }

    private:
        String bluetoothAddress;
        Component::SafePointer<AndroidBluetoothMidiDevicesListBox> owner;
    };

    //==========================================================================
    void disconnectedDeviceClicked (int row)
    {
        stopTimer();

        AndroidBluetoothMidiDevice& device = devices.getReference (row);
        device.connectionStatus = AndroidBluetoothMidiDevice::connecting;
        updateContent();
        repaint();

        new PairDeviceThread (device.bluetoothAddress, *this);
    }

    void connectedDeviceClicked (int row)
    {
        AndroidBluetoothMidiDevice& device = devices.getReference (row);
        device.connectionStatus = AndroidBluetoothMidiDevice::disconnecting;
        updateContent();
        repaint();
        AndroidBluetoothMidiInterface::unpairBluetoothMidiDevice (device.bluetoothAddress);
    }

    //==========================================================================
    void updateDeviceList()
    {
        StringArray bluetoothAddresses = AndroidBluetoothMidiInterface::getBluetoothMidiDevicesNearby();

        Array<AndroidBluetoothMidiDevice> newDevices;

        for (String* address = bluetoothAddresses.begin();
             address != bluetoothAddresses.end(); ++address)
        {
            String name = AndroidBluetoothMidiInterface::getHumanReadableStringForBluetoothAddress (*address);
            DeviceStatus status =  AndroidBluetoothMidiInterface::isBluetoothDevicePaired (*address)
                                      ? AndroidBluetoothMidiDevice::connected
                                      : AndroidBluetoothMidiDevice::disconnected;

            newDevices.add (AndroidBluetoothMidiDevice (name, *address, status));
        }

        devices.swapWith (newDevices);
        updateContent();
        repaint();
    }

    Array<AndroidBluetoothMidiDevice> devices;
    const int timerPeriodInMs;
};

//==============================================================================
class BluetoothMidiSelectorOverlay  : public Component
{
public:
    BluetoothMidiSelectorOverlay()
    {
        setAlwaysOnTop (true);
        setVisible (true);
        addToDesktop (ComponentPeer::windowHasDropShadow);
        setBounds (0, 0, getParentWidth(), getParentHeight());
        toFront (true);

        addAndMakeVisible (bluetoothDevicesList);
        enterModalState (true, nullptr, true);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black.withAlpha (0.6f));

        g.setColour (Colour (0xffdfdfdf));
        Rectangle<int> overlayBounds = getOverlayBounds();
        g.fillRect (overlayBounds);

        g.setColour (Colours::black);
        g.setFont (16);
        g.drawText ("Bluetooth MIDI Devices",
                    overlayBounds.removeFromTop (20).reduced (3, 3),
                    Justification::topLeft, true);

        overlayBounds.removeFromTop (2);

        g.setFont (12);
        g.drawText ("tap to connect/disconnect",
                    overlayBounds.removeFromTop (18).reduced (3, 3),
                    Justification::topLeft, true);
    }

    void inputAttemptWhenModal() override           { exitModalState (0); }
    void mouseDrag (const MouseEvent&) override     {}
    void mouseDown (const MouseEvent&) override     { exitModalState (0); }
    void resized() override                         { update(); }
    void parentSizeChanged() override               { update(); }

private:
    void update()
    {
        setBounds (0, 0, getParentWidth(), getParentHeight());
        bluetoothDevicesList.setBounds (getOverlayBounds().withTrimmedTop (40));
    }

    Rectangle<int> getOverlayBounds() const noexcept
    {
        const int pw = getParentWidth();
        const int ph = getParentHeight();

        return Rectangle<int> (pw, ph).withSizeKeepingCentre (jmin (400, pw - 14),
                                                              jmin (300, ph - 40));
    }

    AndroidBluetoothMidiDevicesListBox bluetoothDevicesList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BluetoothMidiSelectorOverlay)
};

//==============================================================================
bool BluetoothMidiDevicePairingDialogue::open()
{
    BluetoothMidiSelectorOverlay* overlay = new BluetoothMidiSelectorOverlay;
    return true;
}

bool BluetoothMidiDevicePairingDialogue::isAvailable()
{
    jobject btManager (android.activity.callObjectMethod (JuceAppActivity.getAndroidBluetoothManager));
    return btManager != nullptr;
}
