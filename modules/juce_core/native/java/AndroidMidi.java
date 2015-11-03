    //==============================================================================
    public class BluetoothManager extends ScanCallback
    {
        BluetoothManager()
        {
            ScanFilter.Builder scanFilterBuilder = new ScanFilter.Builder();
            scanFilterBuilder.setServiceUuid (ParcelUuid.fromString (bluetoothLEMidiServiceUUID));

            ScanSettings.Builder scanSettingsBuilder = new ScanSettings.Builder();
            scanSettingsBuilder.setCallbackType (ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
                               .setScanMode (ScanSettings.SCAN_MODE_LOW_POWER)
                               .setScanMode (ScanSettings.MATCH_MODE_STICKY);

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

            bluetoothLeScanner.startScan (Arrays.asList (scanFilterBuilder.build()),
                                          scanSettingsBuilder.build(),
                                          this);
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

        public boolean isBluetoothDevicePaired (String address)
        {
            return getAndroidMidiDeviceManager().isBluetoothDevicePaired (address);
        }

        public boolean pairBluetoothMidiDevice(String address)
        {
            BluetoothDevice btDevice = BluetoothAdapter.getDefaultAdapter().getRemoteDevice (address);

            if (btDevice == null)
            {
                Log.d ("JUCE", "failed to create buletooth device from address");
                return false;
            }

            MidiManager mm = (MidiManager) getSystemService (MIDI_SERVICE);

            PhysicalMidiDevice midiDevice = PhysicalMidiDevice.fromBluetoothLeDevice (btDevice, mm);

            if (midiDevice != null)
            {
                getAndroidMidiDeviceManager().addDeviceToList (midiDevice);
                return true;
            }

            return false;
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

        public JuceMidiInputPort (PhysicalMidiDevice device, long host, MidiOutputPort midiPort)
        {
            parent = device;
            juceHost = host;
            port = midiPort;
        }

        @Override
        public boolean isInputPort()
        {
            return true;
        }

        @Override
        public void start()
        {
            port.connect (this);
        }

        @Override
        public void stop()
        {
            port.disconnect (this);
        }

        @Override
        public void close()
        {
            stop();

            try
            {
                port.close();
            }
            catch (IOException e)
            {
                Log.d ("JUCE", "JuceMidiInputPort::close: IOException = " + e.toString());
            }

            if (parent != null)
            {
                parent.removePort (port.getPortNumber(), true);
                parent = null;
            }
        }

        public void onSend (byte[] msg, int offset, int count, long timestamp)
        {
            if (count > 0)
                handleReceive (juceHost, msg, offset, count, timestamp);
        }

        @Override
        public MidiPortID getPortId()
        {
            return new MidiPortID (port.getPortNumber(), true);
        }

        @Override
        public void sendMidi (byte[] msg, int offset, int count)
        {
        }

        private PhysicalMidiDevice parent = null;
        private long juceHost = 0;
        private MidiOutputPort port;
    }

    public static class JuceMidiOutputPort implements JuceMidiPort
    {
        public JuceMidiOutputPort (PhysicalMidiDevice device, MidiInputPort midiPort)
        {
            parent = device;
            port = midiPort;
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
            try
            {
                port.send(msg, offset, count);
            }
            catch (IOException e)
            {
                Log.d ("JUCE", "JuceMidiOutputPort::sendMidi: IOException = " + e.toString());
            }
        }

        @Override
        public void close()
        {
            try
            {
                port.close();
            }
            catch (IOException e)
            {
                Log.d ("JUCE", "JuceMidiOutputPort::close: IOException = " + e.toString());
            }

            if (parent != null)
            {
                parent.removePort (port.getPortNumber(), false);
                parent = null;
            }
        }


        @Override
        public MidiPortID getPortId()
        {
            return new MidiPortID (port.getPortNumber(), false);
        }

        private PhysicalMidiDevice parent = null;
        private MidiInputPort port;
    }

    public static class PhysicalMidiDevice
    {
        private static class MidiDeviceThread extends Thread
        {
            public Handler handler = null;
            public Object sync = null;

            public MidiDeviceThread (Object syncrhonization)
            {
                sync = syncrhonization;
            }

            public void run()
            {
                Looper.prepare();

                synchronized (sync)
                {
                    handler = new Handler();
                    sync.notifyAll();
                }

                Looper.loop();
            }
        }

        private static class MidiDeviceOpenCallback implements MidiManager.OnDeviceOpenedListener
        {
            public Object sync = null;
            public boolean isWaiting = true;
            public android.media.midi.MidiDevice theDevice = null;

            public MidiDeviceOpenCallback (Object waiter)
            {
                sync = waiter;
            }

            public void onDeviceOpened (MidiDevice device)
            {
                synchronized (sync)
                {
                    theDevice = device;
                    isWaiting = false;
                    sync.notifyAll();
                }
            }
        }

        public static PhysicalMidiDevice fromBluetoothLeDevice (BluetoothDevice bluetoothDevice, MidiManager mm)
        {
            Object waitForCreation = new Object();
            MidiDeviceThread thread = new MidiDeviceThread (waitForCreation);
            thread.start();

            synchronized (waitForCreation)
            {
                while (thread.handler == null)
                {
                    try
                    {
                        waitForCreation.wait();
                    }
                    catch (InterruptedException e)
                    {
                        Log.d ("JUCE", "Wait was interrupted but we don't care");
                    }
                }
            }

            Object waitForDevice = new Object();

            MidiDeviceOpenCallback openCallback = new MidiDeviceOpenCallback (waitForDevice);

            synchronized (waitForDevice)
            {
                mm.openBluetoothDevice (bluetoothDevice, openCallback, thread.handler);

                while (openCallback.isWaiting)
                {
                    try
                    {
                        waitForDevice.wait();
                    }
                    catch (InterruptedException e)
                    {
                        Log.d ("JUCE", "Wait was interrupted but we don't care");
                    }
                }
            }

            if (openCallback.theDevice == null)
            {
                Log.d ("JUCE", "openBluetoothDevice failed");
                return null;
            }

            PhysicalMidiDevice device = new PhysicalMidiDevice();

            device.handle = openCallback.theDevice;
            device.info = device.handle.getInfo();
            device.bluetoothAddress = bluetoothDevice.getAddress();
            device.midiManager = mm;

            return device;
        }

        public void unpair()
        {
            if (! bluetoothAddress.equals ("") && handle != null)
            {
                JuceMidiPort ports[] = new JuceMidiPort[0];
                ports = juceOpenedPorts.values().toArray(ports);

                for (int i = 0; i < ports.length; ++i)
                    ports[i].close();

                juceOpenedPorts.clear();

                try
                {
                    handle.close();
                }
                catch (IOException e)
                {
                    Log.d ("JUCE", "handle.close(): IOException = " + e.toString());
                }

                handle = null;
            }
        }

        public static PhysicalMidiDevice fromMidiDeviceInfo (MidiDeviceInfo info, MidiManager mm)
        {
            PhysicalMidiDevice device = new PhysicalMidiDevice();
            device.info = info;
            device.midiManager = mm;
            return device;
        }

        public PhysicalMidiDevice()
        {
            bluetoothAddress = "";
            juceOpenedPorts = new Hashtable<MidiPortID, JuceMidiPort>();
            handle = null;
        }

        public MidiDeviceInfo.PortInfo[] getPorts()
        {
            return info.getPorts();
        }

        public String getHumanReadableNameForPort (MidiDeviceInfo.PortInfo port, int portIndexToUseInName)
        {
            String portName = port.getName();

            if (portName.equals (""))
                portName = ((port.getType() == MidiDeviceInfo.PortInfo.TYPE_OUTPUT) ? "Out " : "In ")
                              + Integer.toString (portIndexToUseInName);

            return getHumanReadableDeviceName() + " " + portName;
        }

        public String getHumanReadableNameForPort (int portType, int androidPortID, int portIndexToUseInName)
        {
            MidiDeviceInfo.PortInfo[] ports = info.getPorts();

            for (MidiDeviceInfo.PortInfo port : ports)
            {
                if (port.getType() == portType)
                {
                    if (port.getPortNumber() == androidPortID)
                        return getHumanReadableNameForPort (port, portIndexToUseInName);
                }
            }

            return "Unknown";
        }

        public String getHumanReadableDeviceName()
        {
            Bundle bundle = info.getProperties();
            return bundle.getString (MidiDeviceInfo.PROPERTY_NAME , "Unknown device");
        }

        public void checkIfDeviceCanBeClosed()
        {
            if (juceOpenedPorts.size() == 0)
            {
                // never close bluetooth LE devices, otherwise they unpair and we have
                // no idea how many ports they have.
                // Only remove bluetooth devices when we specifically unpair
                if (bluetoothAddress.equals (""))
                {
                    try
                    {
                        handle.close();
                        handle = null;
                    }
                    catch (IOException e)
                    {
                        Log.d ("JUCE", "PhysicalMidiDevice::checkIfDeviceCanBeClosed: IOException = " + e.toString());
                    }
                }
            }
        }

        public void removePort (int portIdx, boolean isInput)
        {
            MidiPortID portID = new MidiPortID (portIdx, isInput);
            JuceMidiPort port = juceOpenedPorts.get (portID);

            if (port != null)
            {
                juceOpenedPorts.remove (portID);
                checkIfDeviceCanBeClosed();
                return;
            }

            // tried to remove a port that was never added
            assert false;
        }

        public JuceMidiPort openPort (int portIdx, boolean isInput, long host)
        {
            open();

            if (handle == null)
            {
                Log.d ("JUCE", "PhysicalMidiDevice::openPort: handle = null, device not open");
                return null;
            }

            // make sure that the port is not already open
            if (findPortForIdx (portIdx, isInput) != null)
            {
                Log.d ("JUCE", "PhysicalMidiDevice::openInputPort: port already open, not opening again!");
                return null;
            }

            JuceMidiPort retval = null;

            if (isInput)
            {
                MidiOutputPort androidPort = handle.openOutputPort (portIdx);

                if (androidPort == null)
                {
                    Log.d ("JUCE", "PhysicalMidiDevice::openPort: MidiDevice::openOutputPort (portIdx = "
                           + Integer.toString (portIdx) + ") failed!");
                    return null;
                }

                retval = new JuceMidiInputPort (this, host, androidPort);
            }
            else
            {
                MidiInputPort androidPort = handle.openInputPort (portIdx);

                if (androidPort == null)
                {
                    Log.d ("JUCE", "PhysicalMidiDevice::openPort: MidiDevice::openInputPort (portIdx = "
                           + Integer.toString (portIdx) + ") failed!");
                    return null;
                }

                retval = new JuceMidiOutputPort (this, androidPort);
            }

            juceOpenedPorts.put (new MidiPortID (portIdx, isInput), retval);
            return retval;
        }

        private JuceMidiPort findPortForIdx (int idx, boolean isInput)
        {
            return juceOpenedPorts.get (new MidiPortID (idx, isInput));
        }

        // opens the device
        private synchronized void open()
        {
            if (handle != null)
                return;

            Object waitForCreation = new Object();
            MidiDeviceThread thread = new MidiDeviceThread (waitForCreation);
            thread.start();

            synchronized(waitForCreation)
            {
                while (thread.handler == null)
                {
                    try
                    {
                        waitForCreation.wait();
                    }
                    catch (InterruptedException e)
                    {
                        Log.d ("JUCE", "wait was interrupted but we don't care");
                    }
                }
            }

            Object waitForDevice = new Object();

            MidiDeviceOpenCallback openCallback = new MidiDeviceOpenCallback (waitForDevice);

            synchronized (waitForDevice)
            {
                midiManager.openDevice (info, openCallback, thread.handler);

                while (openCallback.isWaiting)
                {
                    try
                    {
                        waitForDevice.wait();
                    }
                    catch (InterruptedException e)
                    {
                        Log.d ("JUCE", "wait was interrupted but we don't care");
                    }
                }
            }

            handle = openCallback.theDevice;
        }

        private MidiDeviceInfo info;
        private Hashtable<MidiPortID, JuceMidiPort> juceOpenedPorts;
        public MidiDevice handle;
        public String bluetoothAddress;
        private MidiManager midiManager;
    }

    //==============================================================================
    public class MidiDeviceManager extends MidiManager.DeviceCallback
    {
        public class MidiPortPath
        {
            public PhysicalMidiDevice midiDevice;
            public int androidMidiPortID;
            public int portIndexToUseInName;
        }

        public class JuceDeviceList
        {
            public ArrayList<MidiPortPath> inPorts = new ArrayList<MidiPortPath>();
            public ArrayList<MidiPortPath> outPorts = new ArrayList<MidiPortPath>();
        }

        // We need to keep a thread local copy of the devices
        // which we returned the last time
        // getJuceAndroidMidiIn/OutputDevices() was called
        private final ThreadLocal<JuceDeviceList> lastDevicesReturned =
            new ThreadLocal<JuceDeviceList>()
            {
                @Override protected JuceDeviceList initialValue()
                {
                    return new JuceDeviceList();
                }
            };

        public MidiDeviceManager()
        {
            physicalMidiDevices = new ArrayList<PhysicalMidiDevice>();
            manager = (MidiManager) getSystemService (MIDI_SERVICE);

            if (manager == null)
            {
                Log.d ("JUCE", "MidiDeviceManager error: could not get MidiManager system service");
                return;
            }

            manager.registerDeviceCallback (this, null);

            MidiDeviceInfo[] foundDevices = manager.getDevices();

            for (MidiDeviceInfo info : foundDevices)
                physicalMidiDevices.add (PhysicalMidiDevice.fromMidiDeviceInfo (info, manager));
        }

        // specifically add a device to the list
        public void addDeviceToList (PhysicalMidiDevice device)
        {
            physicalMidiDevices.add (device);
        }

        public void unpairBluetoothDevice (String address)
        {
            for (int i = 0; i < physicalMidiDevices.size(); ++i)
            {
                PhysicalMidiDevice device = physicalMidiDevices.get(i);

                if (device.bluetoothAddress.equals (address))
                {
                    physicalMidiDevices.remove (i);
                    device.unpair();
                    return;
                }
            }
        }

        public boolean isBluetoothDevicePaired (String address)
        {
            for (int i = 0; i < physicalMidiDevices.size(); ++i)
            {
                PhysicalMidiDevice device = physicalMidiDevices.get(i);

                if (device.bluetoothAddress.equals (address))
                    return true;
            }

            return false;
        }

        public String[] getJuceAndroidMidiInputDevices()
        {
            return getJuceAndroidMidiDevices (MidiDeviceInfo.PortInfo.TYPE_INPUT);
        }

        public String[] getJuceAndroidMidiOutputDevices()
        {
            return getJuceAndroidMidiDevices (MidiDeviceInfo.PortInfo.TYPE_OUTPUT);
        }

        private String[] getJuceAndroidMidiDevices (int portType)
        {
            ArrayList<MidiPortPath> listOfReturnedDevices = new ArrayList<MidiPortPath>();
            List<String> deviceNames = new ArrayList<String>();

            for (PhysicalMidiDevice physicalMidiDevice : physicalMidiDevices)
            {
                int portIdx = 0;
                MidiDeviceInfo.PortInfo[] ports = physicalMidiDevice.getPorts();

                for (MidiDeviceInfo.PortInfo port : ports)
                {
                    if (port.getType() == portType)
                    {
                        MidiPortPath path = new MidiPortPath();
                        path.midiDevice = physicalMidiDevice;
                        path.androidMidiPortID = port.getPortNumber();
                        path.portIndexToUseInName = ++portIdx;
                        listOfReturnedDevices.add (path);

                        deviceNames.add (physicalMidiDevice.getHumanReadableNameForPort (port,
                                                                                         path.portIndexToUseInName));
                    }
                }
            }

            String[] deviceNamesArray = new String[deviceNames.size()];

            if (portType == MidiDeviceInfo.PortInfo.TYPE_INPUT)
            {
                lastDevicesReturned.get().inPorts.clear();
                lastDevicesReturned.get().inPorts.addAll (listOfReturnedDevices);
            }
            else
            {
                lastDevicesReturned.get().outPorts.clear();
                lastDevicesReturned.get().outPorts.addAll (listOfReturnedDevices);
            }

            return deviceNames.toArray(deviceNamesArray);
        }

        public JuceMidiPort openMidiInputPortWithJuceIndex (int index, long host)
        {
            ArrayList<MidiPortPath> lastDevices = lastDevicesReturned.get().inPorts;

            if (index >= lastDevices.size() || index < 0)
                return null;

            MidiPortPath path = lastDevices.get (index);
            return path.midiDevice.openPort (path.androidMidiPortID, true, host);
        }

        public JuceMidiPort openMidiOutputPortWithJuceIndex (int index)
        {
            ArrayList<MidiPortPath> lastDevices = lastDevicesReturned.get().outPorts;

            if (index >= lastDevices.size() || index < 0)
                return null;

            MidiPortPath path = lastDevices.get (index);
            return path.midiDevice.openPort (path.androidMidiPortID, false, 0);
        }

        public String getInputPortNameForJuceIndex (int index)
        {
            ArrayList<MidiPortPath> lastDevices = lastDevicesReturned.get().inPorts;

            if (index >= lastDevices.size() || index < 0)
                return "";

            MidiPortPath path = lastDevices.get (index);

            return path.midiDevice.getHumanReadableNameForPort (MidiDeviceInfo.PortInfo.TYPE_INPUT,
                                                                path.androidMidiPortID,
                                                                path.portIndexToUseInName);
        }

        public String getOutputPortNameForJuceIndex (int index)
        {
            ArrayList<MidiPortPath> lastDevices = lastDevicesReturned.get().outPorts;

            if (index >= lastDevices.size() || index < 0)
                return "";

            MidiPortPath path = lastDevices.get (index);

            return path.midiDevice.getHumanReadableNameForPort (MidiDeviceInfo.PortInfo.TYPE_OUTPUT,
                                                                path.androidMidiPortID,
                                                                path.portIndexToUseInName);
        }

        public void onDeviceAdded (MidiDeviceInfo info)
        {
            PhysicalMidiDevice device = PhysicalMidiDevice.fromMidiDeviceInfo (info, manager);

            // Do not add bluetooth devices as they are already added by the native bluetooth dialog
            if (info.getType() != MidiDeviceInfo.TYPE_BLUETOOTH)
                physicalMidiDevices.add (device);
        }

        public void onDeviceRemoved (MidiDeviceInfo info)
        {
            for (int i = 0; i < physicalMidiDevices.size(); ++i)
            {
                if (physicalMidiDevices.get(i).info.getId() == info.getId())
                {
                    physicalMidiDevices.remove (i);
                    return;
                }
            }
            // Don't assert here as this may be called again after a bluetooth device is unpaired
        }

        public void onDeviceStatusChanged (MidiDeviceStatus status)
        {
        }

        private ArrayList<PhysicalMidiDevice> physicalMidiDevices;
        private MidiManager manager;
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
