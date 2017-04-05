    //==============================================================================
    public class BluetoothManager extends ScanCallback
    {
        BluetoothManager()
        {
        }

        public String[] getMidiBluetoothAddresses()
        {
            return bluetoothMidiDevices.toArray (new String[bluetoothMidiDevices.size()]);
        }

        public String getHumanReadableStringForBluetoothAddress (String address)
        {
            BluetoothDevice btDevice = BluetoothAdapter.getDefaultAdapter().getRemoteDevice (address);
            return btDevice.getName();
        }

        public int getBluetoothDeviceStatus (String address)
        {
            return getAndroidMidiDeviceManager().getBluetoothDeviceStatus (address);
        }

        public void startStopScan (boolean shouldStart)
        {
            BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

            if (bluetoothAdapter == null)
            {
                Log.d ("JUCE", "BluetoothManager error: could not get default Bluetooth adapter");
                return;
            }

            BluetoothLeScanner bluetoothLeScanner = bluetoothAdapter.getBluetoothLeScanner();

            if (bluetoothLeScanner == null)
            {
                Log.d ("JUCE", "BluetoothManager error: could not get Bluetooth LE scanner");
                return;
            }

            if (shouldStart)
            {
                ScanFilter.Builder scanFilterBuilder = new ScanFilter.Builder();
                scanFilterBuilder.setServiceUuid (ParcelUuid.fromString (bluetoothLEMidiServiceUUID));

                ScanSettings.Builder scanSettingsBuilder = new ScanSettings.Builder();
                scanSettingsBuilder.setCallbackType (ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
                                   .setScanMode (ScanSettings.SCAN_MODE_LOW_POWER)
                                   .setScanMode (ScanSettings.MATCH_MODE_STICKY);

                bluetoothLeScanner.startScan (Arrays.asList (scanFilterBuilder.build()),
                                              scanSettingsBuilder.build(),
                                              this);
            }
            else
            {
                bluetoothLeScanner.stopScan (this);
            }
        }

        public boolean pairBluetoothMidiDevice(String address)
        {
            BluetoothDevice btDevice = BluetoothAdapter.getDefaultAdapter().getRemoteDevice (address);

            if (btDevice == null)
            {
                Log.d ("JUCE", "failed to create buletooth device from address");
                return false;
            }

            return getAndroidMidiDeviceManager().pairBluetoothDevice (btDevice);
        }

        public void unpairBluetoothMidiDevice (String address)
        {
            getAndroidMidiDeviceManager().unpairBluetoothDevice (address);
        }

        public void onScanFailed (int errorCode)
        {
        }

        public void onScanResult (int callbackType, ScanResult result)
        {
            if (callbackType == ScanSettings.CALLBACK_TYPE_ALL_MATCHES
                 || callbackType == ScanSettings.CALLBACK_TYPE_FIRST_MATCH)
            {
                BluetoothDevice device = result.getDevice();

                if (device != null)
                    bluetoothMidiDevices.add (device.getAddress());
            }

            if (callbackType == ScanSettings.CALLBACK_TYPE_MATCH_LOST)
            {
                Log.d ("JUCE", "ScanSettings.CALLBACK_TYPE_MATCH_LOST");
                BluetoothDevice device = result.getDevice();

                if (device != null)
                {
                    bluetoothMidiDevices.remove (device.getAddress());
                    unpairBluetoothMidiDevice (device.getAddress());
                }
            }
        }

        public void onBatchScanResults (List<ScanResult> results)
        {
            for (ScanResult result : results)
                onScanResult (ScanSettings.CALLBACK_TYPE_ALL_MATCHES, result);
        }

        private BluetoothLeScanner scanner;
        private static final String bluetoothLEMidiServiceUUID = "03B80E5A-EDE8-4B33-A751-6CE34EC4C700";

        private HashSet<String> bluetoothMidiDevices = new HashSet<String>();
    }

    public static class JuceMidiInputPort extends MidiReceiver implements JuceMidiPort
    {
        private native void handleReceive (long host, byte[] msg, int offset, int count, long timestamp);

        public JuceMidiInputPort (MidiDeviceManager mm, MidiOutputPort actualPort, MidiPortPath portPathToUse, long hostToUse)
        {
            owner = mm;
            androidPort = actualPort;
            portPath = portPathToUse;
            juceHost = hostToUse;
            isConnected = false;
        }

        @Override
        protected void finalize() throws Throwable
        {
            close();
            super.finalize();
        }

        @Override
        public boolean isInputPort()
        {
            return true;
        }

        @Override
        public void start()
        {
            if (owner != null && androidPort != null && ! isConnected) {
                androidPort.connect(this);
                isConnected = true;
            }
        }

        @Override
        public void stop()
        {
            if (owner != null && androidPort != null && isConnected) {
                androidPort.disconnect(this);
                isConnected = false;
            }
        }

        @Override
        public void close()
        {
            if (androidPort != null) {
                try {
                    androidPort.close();
                } catch (IOException exception) {
                    Log.d("JUCE", "IO Exception while closing port");
                }
            }

            if (owner != null)
                owner.removePort (portPath);

            owner = null;
            androidPort = null;
        }

        @Override
        public void onSend (byte[] msg, int offset, int count, long timestamp)
        {
            if (count > 0)
                handleReceive (juceHost, msg, offset, count, timestamp);
        }

        @Override
        public void onFlush()
        {}

        @Override
        public void sendMidi (byte[] msg, int offset, int count)
        {
        }

        MidiDeviceManager owner;
        MidiOutputPort androidPort;
        MidiPortPath portPath;
        long juceHost;
        boolean isConnected;
    }

    public static class JuceMidiOutputPort implements JuceMidiPort
    {
        public JuceMidiOutputPort (MidiDeviceManager mm, MidiInputPort actualPort, MidiPortPath portPathToUse)
        {
            owner = mm;
            androidPort = actualPort;
            portPath = portPathToUse;
        }

        @Override
        protected void finalize() throws Throwable
        {
            close();
            super.finalize();
        }

        @Override
        public boolean isInputPort()
        {
            return false;
        }

        @Override
        public void start()
        {
        }

        @Override
        public void stop()
        {
        }

        @Override
        public void sendMidi (byte[] msg, int offset, int count)
        {
            if (androidPort != null)
            {
                try {
                    androidPort.send(msg, offset, count);
                } catch (IOException exception)
                {
                    Log.d ("JUCE", "send midi had IO exception");
                }
            }
        }

        @Override
        public void close()
        {
            if (androidPort != null) {
                try {
                    androidPort.close();
                } catch (IOException exception) {
                    Log.d("JUCE", "IO Exception while closing port");
                }
            }

            if (owner != null)
                owner.removePort (portPath);

            owner = null;
            androidPort = null;
        }

        MidiDeviceManager owner;
        MidiInputPort androidPort;
        MidiPortPath portPath;
    }

    private static class MidiPortPath extends Object
    {
        public MidiPortPath (int deviceIdToUse, boolean direction, int androidIndex)
        {
            deviceId = deviceIdToUse;
            isInput = direction;
            portIndex = androidIndex;

        }

        public int deviceId;
        public int portIndex;
        public boolean isInput;

        @Override
        public int hashCode()
        {
            Integer i = new Integer ((deviceId * 128) + (portIndex < 128 ? portIndex : 127));
            return i.hashCode() * (isInput ? -1 : 1);
        }

        @Override
        public boolean equals (Object obj)
        {
            if (obj == null)
                return false;

            if (getClass() != obj.getClass())
                return false;

            MidiPortPath other = (MidiPortPath) obj;
            return (portIndex == other.portIndex && isInput == other.isInput && deviceId == other.deviceId);
        }
    }

    //==============================================================================
    public class MidiDeviceManager extends MidiManager.DeviceCallback implements MidiManager.OnDeviceOpenedListener
    {
        //==============================================================================
        private class MidiDeviceOpenTask extends java.util.TimerTask
        {
            public MidiDeviceOpenTask (MidiDeviceManager deviceManager, MidiDevice device)
            {
                owner = deviceManager;
                midiDevice = device;
            }

            @Override
            public boolean cancel()
            {
                synchronized (MidiDeviceOpenTask.class)
                {
                    owner = null;
                    boolean retval = super.cancel();

                    if (midiDevice != null)
                    {
                        try
                        {
                            midiDevice.close();
                        }
                        catch (IOException e)
                        {}

                        midiDevice = null;
                    }

                    return retval;
                }
            }

            public String getBluetoothAddress()
            {
                synchronized (MidiDeviceOpenTask.class)
                {
                    if (midiDevice != null)
                    {
                        MidiDeviceInfo info = midiDevice.getInfo();
                        if (info.getType() == MidiDeviceInfo.TYPE_BLUETOOTH)
                        {
                            BluetoothDevice btDevice = (BluetoothDevice) info.getProperties().get (info.PROPERTY_BLUETOOTH_DEVICE);
                            if (btDevice != null)
                                return btDevice.getAddress();
                        }
                    }
                }

                return "";
            }

            public int getID()
            {
                return midiDevice.getInfo().getId();
            }

            @Override
            public void run()
            {
                synchronized (MidiDeviceOpenTask.class)
                {
                    if (owner != null && midiDevice != null)
                        owner.onDeviceOpenedDelayed (midiDevice);
                }
            }

            private MidiDeviceManager owner;
            private MidiDevice midiDevice;
        }

        //==============================================================================
        public MidiDeviceManager()
        {
            manager = (MidiManager) getSystemService (MIDI_SERVICE);

            if (manager == null)
            {
                Log.d ("JUCE", "MidiDeviceManager error: could not get MidiManager system service");
                return;
            }

            openPorts = new HashMap<MidiPortPath, WeakReference<JuceMidiPort>> ();
            midiDevices = new ArrayList<MidiDevice>();
            openTasks = new HashMap<Integer, MidiDeviceOpenTask>();
            btDevicesPairing = new HashSet<String>();

            MidiDeviceInfo[] foundDevices = manager.getDevices();
            for (MidiDeviceInfo info : foundDevices)
                onDeviceAdded (info);

            manager.registerDeviceCallback (this, null);
        }

        protected void finalize() throws Throwable
        {
            manager.unregisterDeviceCallback (this);

            synchronized (MidiDeviceManager.class)
            {
                btDevicesPairing.clear();

                for (Integer deviceID : openTasks.keySet())
                    openTasks.get (deviceID).cancel();

                openTasks = null;
            }

            for (MidiPortPath key : openPorts.keySet())
                openPorts.get (key).get().close();

            openPorts = null;

            for (MidiDevice device : midiDevices)
                device.close();

            super.finalize();
        }

        public String[] getJuceAndroidMidiInputDevices()
        {
            return getJuceAndroidMidiDevices (MidiDeviceInfo.PortInfo.TYPE_OUTPUT);
        }

        public String[] getJuceAndroidMidiOutputDevices()
        {
            return getJuceAndroidMidiDevices (MidiDeviceInfo.PortInfo.TYPE_INPUT);
        }

        private String[] getJuceAndroidMidiDevices (int portType)
        {
            // only update the list when JUCE asks for a new list
            synchronized (MidiDeviceManager.class)
            {
                deviceInfos = getDeviceInfos();
            }

            ArrayList<String> portNames = new ArrayList<String>();

            int index = 0;
            for (MidiPortPath portInfo = getPortPathForJuceIndex (portType, index); portInfo != null; portInfo = getPortPathForJuceIndex (portType, ++index))
                portNames.add (getPortName (portInfo));

            String[] names = new String[portNames.size()];
            return portNames.toArray (names);
        }

        private JuceMidiPort openMidiPortWithJuceIndex (int index, long host, boolean isInput)
        {
            synchronized (MidiDeviceManager.class)
            {
                int portTypeToFind = (isInput ? MidiDeviceInfo.PortInfo.TYPE_OUTPUT : MidiDeviceInfo.PortInfo.TYPE_INPUT);
                MidiPortPath portInfo = getPortPathForJuceIndex (portTypeToFind, index);

                if (portInfo != null)
                {
                    // ports must be opened exclusively!
                    if (openPorts.containsKey (portInfo))
                        return null;

                    MidiDevice device = getMidiDeviceForId (portInfo.deviceId);
                    if (device != null)
                    {
                        JuceMidiPort juceMidiPort = null;

                        if (isInput)
                        {
                            MidiOutputPort outputPort = device.openOutputPort (portInfo.portIndex);

                            if (outputPort != null)
                                juceMidiPort = new JuceMidiInputPort(this, outputPort, portInfo, host);
                        }
                        else
                        {
                            MidiInputPort inputPort = device.openInputPort (portInfo.portIndex);

                            if (inputPort != null)
                                juceMidiPort = new JuceMidiOutputPort(this, inputPort, portInfo);
                        }

                        if (juceMidiPort != null) {
                            openPorts.put(portInfo, new WeakReference<JuceMidiPort>(juceMidiPort));

                            return juceMidiPort;
                        }
                    }
                }
            }

            return null;
        }

        public JuceMidiPort openMidiInputPortWithJuceIndex (int index, long host)
        {
            return openMidiPortWithJuceIndex (index, host, true);
        }

        public JuceMidiPort openMidiOutputPortWithJuceIndex (int index)
        {
            return openMidiPortWithJuceIndex (index, 0, false);
        }

        /* 0: unpaired, 1: paired, 2: pairing */
        public int getBluetoothDeviceStatus (String address)
        {
            synchronized (MidiDeviceManager.class)
            {
                if (! address.isEmpty())
                {
                    if (findMidiDeviceForBluetoothAddress (address) != null)
                        return 1;

                    if (btDevicesPairing.contains (address))
                        return 2;

                    if (findOpenTaskForBluetoothAddress (address) != null)
                        return 2;
                }
            }

            return 0;
        }

        public boolean pairBluetoothDevice (BluetoothDevice btDevice)
        {
            String btAddress = btDevice.getAddress();
            if (btAddress.isEmpty())
                return false;

            synchronized (MidiDeviceManager.class)
            {
                if (getBluetoothDeviceStatus (btAddress) != 0)
                    return false;

                btDevicesPairing.add (btDevice.getAddress());
                manager.openBluetoothDevice(btDevice, this, null);
            }

            return true;
        }

        public void unpairBluetoothDevice (String address)
        {
            if (address.isEmpty())
                return;

            synchronized (MidiDeviceManager.class)
            {
                btDevicesPairing.remove (address);

                MidiDeviceOpenTask openTask = findOpenTaskForBluetoothAddress (address);
                if (openTask != null)
                {
                    int deviceID = openTask.getID();
                    openTask.cancel();
                    openTasks.remove (deviceID);
                }

                MidiDevice midiDevice = findMidiDeviceForBluetoothAddress (address);
                if (midiDevice != null)
                {
                    onDeviceRemoved (midiDevice.getInfo());

                    try {
                        midiDevice.close();
                    }
                    catch (IOException exception)
                    {
                        Log.d ("JUCE", "IOException while closing midi device");
                    }
                }
            }
        }

        private MidiDevice findMidiDeviceForBluetoothAddress (String address)
        {
            for (MidiDevice midiDevice : midiDevices)
            {
                MidiDeviceInfo info = midiDevice.getInfo();
                if (info.getType() == MidiDeviceInfo.TYPE_BLUETOOTH)
                {
                    BluetoothDevice btDevice = (BluetoothDevice) info.getProperties().get (info.PROPERTY_BLUETOOTH_DEVICE);
                    if (btDevice != null && btDevice.getAddress().equals (address))
                        return midiDevice;
                }
            }

            return null;
        }

        private MidiDeviceOpenTask findOpenTaskForBluetoothAddress (String address)
        {
            for (Integer deviceID : openTasks.keySet())
            {
                MidiDeviceOpenTask openTask = openTasks.get (deviceID);
                if (openTask.getBluetoothAddress().equals (address))
                    return openTask;
            }

            return null;
        }

        public void removePort (MidiPortPath path)
        {
            openPorts.remove (path);
        }

        public String getInputPortNameForJuceIndex (int index)
        {
            MidiPortPath portInfo = getPortPathForJuceIndex (MidiDeviceInfo.PortInfo.TYPE_OUTPUT, index);
            if (portInfo != null)
                return getPortName (portInfo);

            return "";
        }

        public String getOutputPortNameForJuceIndex (int index)
        {
            MidiPortPath portInfo = getPortPathForJuceIndex (MidiDeviceInfo.PortInfo.TYPE_INPUT, index);
            if (portInfo != null)
                return getPortName (portInfo);

            return "";
        }

        public void onDeviceAdded (MidiDeviceInfo info)
        {
            // only add standard midi devices
            if (info.getType() == info.TYPE_BLUETOOTH)
                return;

            manager.openDevice (info, this, null);
        }

        public void onDeviceRemoved (MidiDeviceInfo info)
        {
            synchronized (MidiDeviceManager.class)
            {
                MidiDevice device = getMidiDeviceForId (info.getId());

                // close all ports that use this device
                boolean removedPort = true;

                while (removedPort == true) {
                    removedPort = false;
                    for (MidiPortPath key : openPorts.keySet()) {
                        if (key.deviceId == info.getId()) {
                            openPorts.get(key).get().close();
                            removedPort = true;
                            break;
                        }
                    }
                }

                if (device != null)
                    midiDevices.remove (device);
            }
        }

        public void onDeviceStatusChanged (MidiDeviceStatus status)
        {
        }

        @Override
        public void onDeviceOpened (MidiDevice theDevice)
        {
            synchronized (MidiDeviceManager.class)
            {
                MidiDeviceInfo info = theDevice.getInfo();
                int deviceID = info.getId();

                if (! openTasks.containsKey (deviceID))
                {
                    if (info.getType() == MidiDeviceInfo.TYPE_BLUETOOTH)
                    {
                        BluetoothDevice btDevice = (BluetoothDevice) info.getProperties().get (info.PROPERTY_BLUETOOTH_DEVICE);
                        if (btDevice != null)
                        {
                            String btAddress = btDevice.getAddress();
                            if (btDevicesPairing.contains (btAddress))
                            {
                                btDevicesPairing.remove (btAddress);
                            }
                            else
                            {
                                // unpair was called in the mean time
                                try
                                {
                                    theDevice.close();
                                }
                                catch (IOException e)
                                {}

                                return;
                            }
                        }
                    }

                    MidiDeviceOpenTask openTask = new MidiDeviceOpenTask (this, theDevice);
                    openTasks.put (deviceID, openTask);

                    new java.util.Timer().schedule (openTask, 3000);
                }
            }
        }

        public void onDeviceOpenedDelayed (MidiDevice theDevice)
        {
            synchronized (MidiDeviceManager.class)
            {
                int deviceID = theDevice.getInfo().getId();

                if (openTasks.containsKey (deviceID))
                {
                    if (! midiDevices.contains(theDevice))
                    {
                        openTasks.remove (deviceID);
                        midiDevices.add (theDevice);
                    }
                }
                else
                {
                    // unpair was called in the mean time
                    try
                    {
                        theDevice.close();
                    }
                    catch (IOException e)
                    {}
                }
            }
        }

        public String getPortName(MidiPortPath path)
        {
            int portTypeToFind = (path.isInput ? MidiDeviceInfo.PortInfo.TYPE_INPUT : MidiDeviceInfo.PortInfo.TYPE_OUTPUT);

            synchronized (MidiDeviceManager.class)
            {
                for (MidiDeviceInfo info : deviceInfos)
                {
                    int localIndex = 0;
                    if (info.getId() == path.deviceId)
                    {
                        for (MidiDeviceInfo.PortInfo portInfo : info.getPorts())
                        {
                            int portType = portInfo.getType();
                            if (portType == portTypeToFind)
                            {
                                int portIndex = portInfo.getPortNumber();
                                if (portIndex == path.portIndex)
                                {
                                    String portName = portInfo.getName();
                                    if (portName.isEmpty())
                                        portName = (String) info.getProperties().get(info.PROPERTY_NAME);

                                    return portName;
                                }
                            }
                        }
                    }
                }
            }

            return "";
        }

        public MidiPortPath getPortPathForJuceIndex (int portType, int juceIndex)
        {
            int portIdx = 0;
            for (MidiDeviceInfo info : deviceInfos)
            {
                for (MidiDeviceInfo.PortInfo portInfo : info.getPorts())
                {
                    if (portInfo.getType() == portType)
                    {
                        if (portIdx == juceIndex)
                            return new MidiPortPath (info.getId(),
                                    (portType == MidiDeviceInfo.PortInfo.TYPE_INPUT),
                                    portInfo.getPortNumber());

                        portIdx++;
                    }
                }
            }

            return null;
        }

        private MidiDeviceInfo[] getDeviceInfos()
        {
            synchronized (MidiDeviceManager.class)
            {
                MidiDeviceInfo[] infos = new MidiDeviceInfo[midiDevices.size()];

                int idx = 0;
                for (MidiDevice midiDevice : midiDevices)
                    infos[idx++] = midiDevice.getInfo();

                return infos;
            }
        }

        private MidiDevice getMidiDeviceForId (int deviceId)
        {
            synchronized (MidiDeviceManager.class)
            {
                for (MidiDevice midiDevice : midiDevices)
                    if (midiDevice.getInfo().getId() == deviceId)
                        return midiDevice;
            }

            return null;
        }

        private MidiManager manager;
        private HashSet<String> btDevicesPairing;
        private HashMap<Integer, MidiDeviceOpenTask> openTasks;
        private ArrayList<MidiDevice> midiDevices;
        private MidiDeviceInfo[] deviceInfos;
        private HashMap<MidiPortPath, WeakReference<JuceMidiPort>> openPorts;
    }

    public MidiDeviceManager getAndroidMidiDeviceManager()
    {
        if (getSystemService (MIDI_SERVICE) == null)
            return null;

        synchronized (JuceAppActivity.class)
        {
            if (midiDeviceManager == null)
                midiDeviceManager = new MidiDeviceManager();
        }

        return midiDeviceManager;
    }

    public BluetoothManager getAndroidBluetoothManager()
    {
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();

        if (adapter == null)
            return null;

        if (adapter.getBluetoothLeScanner() == null)
            return null;

        synchronized (JuceAppActivity.class)
        {
            if (bluetoothManager == null)
                bluetoothManager = new BluetoothManager();
        }

        return bluetoothManager;
    }
