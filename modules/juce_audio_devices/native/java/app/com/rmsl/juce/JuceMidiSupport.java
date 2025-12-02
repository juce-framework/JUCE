/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

package com.rmsl.juce;


import android.annotation.TargetApi;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.icu.text.IDNA;
import android.media.midi.MidiDevice;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiDeviceService;
import android.media.midi.MidiDeviceStatus;
import android.media.midi.MidiInputPort;
import android.media.midi.MidiManager;
import android.media.midi.MidiOutputPort;
import android.media.midi.MidiReceiver;
import android.media.midi.MidiUmpDeviceService;
import android.os.Build;
import android.os.ParcelUuid;
import android.util.Log;
import android.util.Pair;

import java.io.IOException;
import java.lang.annotation.Native;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.Timer;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.function.Predicate;

import static android.content.Context.MIDI_SERVICE;
import static android.content.Context.BLUETOOTH_SERVICE;
import static android.content.pm.PackageManager.COMPONENT_ENABLED_STATE_DISABLED;
import static android.content.pm.PackageManager.COMPONENT_ENABLED_STATE_ENABLED;
import static android.content.pm.PackageManager.DONT_KILL_APP;

public class JuceMidiSupport
{
    public static class PortPath
    {
        int deviceId;
        int portIndex;
        int type;

        @Override
        public boolean equals (Object other)
        {
            if (other.getClass() != getClass())
                return false;

            PortPath p = (PortPath) other;

            return p.deviceId == deviceId
                && p.portIndex == portIndex
                && p.type == type;
        }

        @Override
        public int hashCode()
        {
            return Objects.hash (deviceId, portIndex, type);
        }
    }

    static public abstract class JuceMidiPort extends MidiReceiver
    {
        // start, stop does nothing on an output port
        abstract void start();

        abstract void stop();

        abstract void close();

        abstract boolean isActive();

        abstract PortPath getPortPath();
    }

    static BluetoothAdapter getDefaultBluetoothAdapter (Context ctx)
    {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S_V2)
        {
            @SuppressWarnings ("deprecation")
            BluetoothAdapter result = BluetoothAdapter.getDefaultAdapter();
            return result;
        }

        return ((BluetoothManager) ctx.getSystemService (BLUETOOTH_SERVICE)).getAdapter();
    }

    // These correspond to the constants in MidiManager, but those may not be available depending
    // on the current SDK.
    private static final int TRANSPORT_BYTESTREAM = 1;
    private static final int TRANSPORT_UMP = 2;

    static Set<MidiDeviceInfo> getDevicesForTransport (MidiManager mm, int transport)
    {
        if (Build.VERSION_CODES.TIRAMISU <= Build.VERSION.SDK_INT)
            return mm.getDevicesForTransport (transport);

        if (transport == TRANSPORT_BYTESTREAM)
        {
            @SuppressWarnings ("deprecation")
            HashSet<MidiDeviceInfo> result = new HashSet<> (Arrays.asList (mm.getDevices()));
            return result;
        }

        return new HashSet<>();
    }

    public static class InfoWithTransport
    {
        InfoWithTransport (MidiDeviceInfo i, int t)
        {
            info = i;
            transport = t;
        }

        public final MidiDeviceInfo info;
        public final int transport;
    }

    static List<InfoWithTransport> getDevicesPreferringUmp (MidiManager mm)
    {
        ArrayList<InfoWithTransport> result = new ArrayList<>();

        for (int transport : Arrays.asList (TRANSPORT_UMP, TRANSPORT_BYTESTREAM))
        {
            for (MidiDeviceInfo info : getDevicesForTransport (mm, transport))
            {
                result.add (new InfoWithTransport (info, transport));
            }
        }

        return result;
    }

    //==============================================================================
    public static class BluetoothMidiManager extends ScanCallback
    {
        BluetoothMidiManager (Context contextToUse)
        {
            appContext = contextToUse;
        }

        public List<String> getMidiBluetoothAddresses()
        {
            return new ArrayList<String> (bluetoothMidiDevices);
        }

        public String getHumanReadableStringForBluetoothAddress (String address)
        {
            BluetoothDevice btDevice = getDefaultBluetoothAdapter (appContext).getRemoteDevice (address);
            return btDevice.getName();
        }

        public int getBluetoothDeviceStatus (String address)
        {
            return getAndroidMidiDeviceManager (appContext).getBluetoothDeviceStatus (address);
        }

        public void startStopScan (boolean shouldStart)
        {
            BluetoothAdapter bluetoothAdapter = getDefaultBluetoothAdapter (appContext);

            if (bluetoothAdapter == null)
            {
                Log.d ("JUCE", "BluetoothMidiManager error: could not get default Bluetooth adapter");
                return;
            }

            BluetoothLeScanner bluetoothLeScanner = bluetoothAdapter.getBluetoothLeScanner();

            if (bluetoothLeScanner == null)
            {
                Log.d ("JUCE", "BluetoothMidiManager error: could not get Bluetooth LE scanner");
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

        public boolean pairBluetoothMidiDevice (String address)
        {
            BluetoothDevice btDevice = getDefaultBluetoothAdapter (appContext).getRemoteDevice (address);

            if (btDevice == null)
            {
                Log.d ("JUCE", "failed to create bluetooth device from address");
                return false;
            }

            return getAndroidMidiDeviceManager (appContext).pairBluetoothDevice (btDevice);
        }

        public void unpairBluetoothMidiDevice (String address)
        {
            getAndroidMidiDeviceManager (appContext).unpairBluetoothDevice (address);
        }

        @Override
        public void onScanFailed (int errorCode)
        {
        }

        @Override
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

        @Override
        public void onBatchScanResults (List<ScanResult> results)
        {
            for (ScanResult result : results)
                onScanResult (ScanSettings.CALLBACK_TYPE_ALL_MATCHES, result);
        }

        private BluetoothLeScanner scanner;
        private static final String bluetoothLEMidiServiceUUID = "03B80E5A-EDE8-4B33-A751-6CE34EC4C700";

        private HashSet<String> bluetoothMidiDevices = new HashSet<>();
        private Context appContext = null;
    }

    public static class NativeMidiReceiver extends MidiReceiver
    {
        NativeMidiReceiver (long h)
        {
            host = h;
        }

        @Override
        public void onSend (byte[] msg, int offset, int count, long timestamp) throws IOException
        {
            if (host != 0 && count > 0)
                handleReceive (host, msg, offset, count, timestamp);
        }

        public long host = 0;
        private native void handleReceive (long host, byte[] msg, int offset, int count, long timestamp);
    }

    private native static void handleDeviceAdded (JuceMidiDeviceInfo info);
    private native static void handleDeviceRemoved (int id);

    public static class JuceMidiInputPort extends JuceMidiPort
    {
        public JuceMidiInputPort (MidiDeviceManager mm, MidiOutputPort actualPort, int d, int t, long hostToUse)
        {
            receiver = new NativeMidiReceiver (hostToUse);
            owner = mm;
            androidPort = actualPort;
            deviceId = d;
            transport = t;
            isConnected = false;
        }

        @Override
        protected void finalize() throws Throwable
        {
            close();
            super.finalize();
        }

        @Override
        public void start()
        {
            if (owner != null && androidPort != null && ! isConnected)
                androidPort.connect (receiver);

            isConnected = true;
        }

        @Override
        public void stop()
        {
            if (owner != null && androidPort != null && isConnected)
                androidPort.disconnect (receiver);

            isConnected = false;
        }

        @Override
        public boolean isActive()
        {
            return isConnected;
        }

        @Override
        public void close()
        {
            PortPath p = null;

            if (androidPort != null)
            {
                p = getPortPath();

                try
                {
                    androidPort.close();
                }
                catch (IOException exception)
                {
                    Log.d ("JUCE", "IO Exception while closing port");
                }
            }

            if (owner != null && p != null)
                owner.removePort (p);

            owner = null;
            androidPort = null;
        }

        @Override
        public PortPath getPortPath()
        {
            PortPath p = new PortPath();
            p.deviceId = deviceId;
            p.portIndex = androidPort.getPortNumber();
            p.type = MidiDeviceInfo.PortInfo.TYPE_OUTPUT;
            return p;
        }

        @Override
        public void onFlush()
        {}

        @Override
        public void onSend(byte[] msg, int offset, int count, long timestamp) throws IOException
        {
        }

        NativeMidiReceiver receiver;
        MidiDeviceManager owner;
        MidiOutputPort androidPort;
        int deviceId;
        int transport;
        boolean isConnected;
    }

    public static class JuceMidiOutputPort extends JuceMidiPort
    {
        public JuceMidiOutputPort (MidiDeviceManager mm, MidiInputPort actualPort, int d, int t)
        {
            owner = mm;
            androidPort = actualPort;
            deviceId = d;
            transport = t;
        }

        @Override
        protected void finalize() throws Throwable
        {
            close();
            super.finalize();
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
        public boolean isActive()
        {
            return false;
        }

        @Override
        public void onSend (byte[] msg, int offset, int count, long timestamp)
        {
            if (androidPort != null)
            {
                try
                {
                    androidPort.send (msg, offset, count, timestamp);
                }
                catch (IOException exception)
                {
                    Log.d ("JUCE", "send midi had IO exception");
                }
            }
        }

        @Override
        public void close()
        {
            PortPath p = null;

            if (androidPort != null)
            {
                p = getPortPath();

                try
                {
                    androidPort.close();
                }
                catch (IOException exception)
                {
                    Log.d ("JUCE", "IO Exception while closing port");
                }
            }

            if (owner != null && p != null)
                owner.removePort (p);

            owner = null;
            androidPort = null;
        }

        @Override
        public PortPath getPortPath()
        {
            PortPath p = new PortPath();
            p.deviceId = deviceId;
            p.portIndex = androidPort.getPortNumber();
            p.type = MidiDeviceInfo.PortInfo.TYPE_INPUT;
            return p;
        }

        MidiDeviceManager owner;
        MidiInputPort androidPort;
        int deviceId;
        int transport;
    }

    //==============================================================================
    public static class JuceMidiDeviceInfo
    {
        public String name;
        public String manufacturer;
        public String product;
        public String serialNumber;
        public int id;
        public int defaultProtocol;
        public int transport;
        public int type;

        public ArrayList<String> dst; // input port names
        public ArrayList<String> src; // output port names
    }

    public static class TransportDeviceCallback
    {
        public void onDeviceAdded (InfoWithTransport info) {}
        public void onDeviceRemoved (MidiDeviceInfo info) {}
    }

    public static class DeviceCallbackImpl extends MidiManager.DeviceCallback
    {
        public DeviceCallbackImpl (TransportDeviceCallback c, int t)
        {
            callback = c;
            transport = t;
        }

        @Override
        public void onDeviceAdded (MidiDeviceInfo device)
        {
            super.onDeviceAdded (device);
            callback.onDeviceAdded (new InfoWithTransport (device, transport));
        }

        @Override
        public void onDeviceRemoved (MidiDeviceInfo device)
        {
            super.onDeviceRemoved (device);
            callback.onDeviceRemoved (device);
        }

        public final TransportDeviceCallback callback;
        public final int transport;
    }

    //==============================================================================
    public static class MidiDeviceManager
            extends TransportDeviceCallback
            implements MidiManager.OnDeviceOpenedListener
    {
        //==============================================================================
        private class InertBluetoothGattCallback extends BluetoothGattCallback
        {
            public InertBluetoothGattCallback (MidiDeviceManager mm)
            {
                super();
                owner = mm;
            }

            public void onConnectionStateChange (BluetoothGatt gatt, int status, int newState)
            {
                if (newState == BluetoothProfile.STATE_CONNECTED)
                {
                    gatt.requestConnectionPriority (BluetoothGatt.CONNECTION_PRIORITY_HIGH);
                    owner.pairBluetoothDeviceStepTwo (gatt.getDevice());
                }
            }

            private MidiDeviceManager owner;
        }

        // get(String) is deprecated but there doesn't seem to be an alternative to fetch a BluetoothDevice
        @SuppressWarnings ("deprecation")
        private static BluetoothDevice getBluetoothDevice (MidiDeviceInfo info)
        {
            return (BluetoothDevice) info.getProperties().get (MidiDeviceInfo.PROPERTY_BLUETOOTH_DEVICE);
        }

        //==============================================================================
        private class MidiDeviceOpenTask extends java.util.TimerTask
        {
            public MidiDeviceOpenTask (MidiDeviceManager deviceManager, DiscoveredDevice deviceIn)
            {
                owner = deviceManager;
                device = deviceIn;
            }

            @Override
            public boolean cancel()
            {
                synchronized (MidiDeviceOpenTask.class)
                {
                    owner = null;
                    boolean retval = super.cancel();

                    if (device.gatt != null)
                    {
                        device.gatt.disconnect();
                        device.gatt.close();

                        device.gatt = null;
                    }

                    if (device.device != null)
                    {
                        try
                        {
                            device.device.close();
                        }
                        catch (IOException e)
                        {
                        }

                        device.device = null;
                    }

                    return retval;
                }
            }

            public String getBluetoothAddress()
            {
                synchronized (MidiDeviceOpenTask.class)
                {
                    if (device.device != null)
                    {
                        MidiDeviceInfo info = device.device.getInfo();
                        if (info.getType() == MidiDeviceInfo.TYPE_BLUETOOTH)
                        {
                            BluetoothDevice btDevice = getBluetoothDevice (info);
                            if (btDevice != null)
                                return btDevice.getAddress();
                        }
                    }
                }

                return "";
            }

            public BluetoothGatt getGatt()
            {
                return device.gatt;
            }

            public int getID()
            {
                return device.device.getInfo().getId();
            }

            @Override
            public void run()
            {
                synchronized (MidiDeviceOpenTask.class)
                {
                    if (owner != null && device != null)
                        owner.onDeviceOpenedDelayed (device);
                }
            }

            private MidiDeviceManager owner;
            private DiscoveredDevice device;
        }

        //==============================================================================
        public MidiDeviceManager (Context contextToUse)
        {
            appContext = contextToUse;
            manager = (MidiManager) appContext.getSystemService (MIDI_SERVICE);

            if (manager == null)
            {
                Log.d ("JUCE", "MidiDeviceManager error: could not get MidiManager system service");
                return;
            }

            for (InfoWithTransport info : getDevicesPreferringUmp (manager))
            {
                try
                {
                    onDeviceAdded (info);
                }
                catch (IllegalArgumentException e)
                {
                    Log.d ("JUCE", "MidiDeviceManager warning: Skipping MIDI UMP device " + info.toString());
                }
            }

            registerDeviceCallback();
        }

        @SuppressWarnings ("deprecation")
        private void registerDeviceCallback()
        {
            if (Build.VERSION_CODES.TIRAMISU <= Build.VERSION.SDK_INT)
            {
                for (int transport : Arrays.asList (TRANSPORT_UMP, TRANSPORT_BYTESTREAM))
                    deviceCallbacks.add (new DeviceCallbackImpl (this, transport));

                for (DeviceCallbackImpl callback : deviceCallbacks)
                    manager.registerDeviceCallback (callback.transport, executor, callback);
            }
            else
            {
                deviceCallbacks.add (new DeviceCallbackImpl (this, TRANSPORT_BYTESTREAM));

                for (DeviceCallbackImpl callback : deviceCallbacks)
                    manager.registerDeviceCallback (callback, null);
            }
        }

        protected void finalize() throws Throwable
        {
            for (DeviceCallbackImpl callback : deviceCallbacks)
                manager.unregisterDeviceCallback (callback);

            deviceCallbacks.clear();

            synchronized (MidiDeviceManager.class)
            {
                btDevicesPairing.clear();

                for (Integer deviceID : openTasks.keySet())
                    openTasks.get (deviceID).cancel();

                openTasks = null;
            }

            for (PortPath key : openPorts.keySet())
                openPorts.get (key).get().close();

            openPorts = null;

            for (DiscoveredDevice device : midiDevices)
            {
                if (device.gatt != null)
                {
                    device.gatt.disconnect();
                    device.gatt.close();
                }

                device.device.close();
            }

            midiDevices.clear();

            super.finalize();
        }

        private JuceMidiPort makeMidiPort (int deviceId, java.util.function.Function<DiscoveredDevice, JuceMidiPort> callback)
        {
            synchronized (MidiDeviceManager.class)
            {
                DiscoveredDevice device = getDiscoveredDeviceForId (deviceId);

                if (device == null || device.device == null)
                    return null;

                JuceMidiPort juceMidiPort = callback.apply (device);

                if (juceMidiPort == null)
                    return null;

                openPorts.put (juceMidiPort.getPortPath(), new WeakReference<JuceMidiPort> (juceMidiPort));
                return juceMidiPort;
            }
        }

        public JuceMidiPort openMidiInputPortWithID (int deviceId, int portIndex, long host)
        {
            return makeMidiPort (deviceId, (device) ->
            {
                MidiOutputPort port = device.device.openOutputPort (portIndex);

                if (port == null)
                    return null;

                return new JuceMidiInputPort (this, port, device.device.getInfo().getId(), device.transport, host);
            });
        }

        public JuceMidiPort openMidiOutputPortWithID (int deviceId, int portIndex)
        {
            return makeMidiPort (deviceId, (device) ->
            {
                MidiInputPort port = device.device.openInputPort (portIndex);

                if (port == null)
                    return null;

                return new JuceMidiOutputPort (this, port, device.device.getInfo().getId(), device.transport);
            });
        }

        /* 0: unpaired, 1: paired, 2: pairing */
        public int getBluetoothDeviceStatus (String address)
        {
            synchronized (MidiDeviceManager.class)
            {
                if (!address.isEmpty())
                {
                    if (findMidiDeviceForBluetoothAddress (address) != null)
                        return 1;

                    if (btDevicesPairing.containsKey (address))
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

                btDevicesPairing.put (btDevice.getAddress(), null);
                BluetoothGatt gatt = btDevice.connectGatt (appContext.getApplicationContext(), true, new InertBluetoothGattCallback (this));

                if (gatt != null)
                {
                    btDevicesPairing.put (btDevice.getAddress(), gatt);
                }
                else
                {
                    pairBluetoothDeviceStepTwo (btDevice);
                }
            }

            return true;
        }

        public void pairBluetoothDeviceStepTwo (BluetoothDevice btDevice)
        {
            manager.openBluetoothDevice (btDevice, this, null);
        }

        public void unpairBluetoothDevice (String address)
        {
            if (address.isEmpty())
                return;

            synchronized (MidiDeviceManager.class)
            {
                if (btDevicesPairing.containsKey (address))
                {
                    BluetoothGatt gatt = btDevicesPairing.get (address);
                    if (gatt != null)
                    {
                        gatt.disconnect();
                        gatt.close();
                    }

                    btDevicesPairing.remove (address);
                }

                MidiDeviceOpenTask openTask = findOpenTaskForBluetoothAddress (address);
                if (openTask != null)
                {
                    int deviceID = openTask.getID();
                    openTask.cancel();
                    openTasks.remove (deviceID);
                }

                DiscoveredDevice discoveredDevice = findMidiDeviceForBluetoothAddress (address);

                if (discoveredDevice != null)
                {
                    MidiDevice midiDevice = discoveredDevice.device;
                    onDeviceRemoved (midiDevice.getInfo());

                    try
                    {
                        midiDevice.close();
                    }
                    catch (IOException exception)
                    {
                        Log.d ("JUCE", "IOException while closing midi device");
                    }
                }
            }
        }

        private DiscoveredDevice findMidiDeviceForBluetoothAddress (String address)
        {
            for (DiscoveredDevice midiDevice : midiDevices)
            {
                MidiDeviceInfo info = midiDevice.device.getInfo();
                if (info.getType() == MidiDeviceInfo.TYPE_BLUETOOTH)
                {
                    BluetoothDevice btDevice = getBluetoothDevice (info);
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

        public void removePort (PortPath path)
        {
            openPorts.remove (path);
        }

        @Override
        public void onDeviceAdded (InfoWithTransport info)
        {
            synchronized (transportForDevice)
            {
                transportForDevice.put (info.info.getId(), info.transport);
            }

            try
            {
                manager.openDevice (info.info, this, null);
            }
            catch (Exception e)
            {
                // Something went wrong opening the device
            }
        }

        @Override
        public void onDeviceRemoved (MidiDeviceInfo info)
        {
            synchronized (transportForDevice)
            {
                transportForDevice.remove (info.getId());
            }

            int discoveredId = 0;

            synchronized (MidiDeviceManager.class)
            {
                DiscoveredDevice discovered = getDiscoveredDeviceForId (info.getId());

                if (discovered == null)
                    return;

                discoveredId = discovered.device.getInfo().getId();

                for (;;)
                {
                    boolean removedPort = false;

                    for (PortPath key : openPorts.keySet())
                    {
                        if (key.deviceId == info.getId())
                        {
                            openPorts.get (key).get().close();
                            removedPort = true;
                            break;
                        }
                    }

                    if (! removedPort)
                        break;
                }

                if (discovered.gatt != null)
                {
                    discovered.gatt.disconnect();
                    discovered.gatt.close();
                }

                midiDevices.remove (discovered);
            }

            handleDeviceRemoved (discoveredId);
        }

        private int getTransportForDevice (int deviceId)
        {
            synchronized (transportForDevice)
            {
                try
                {
                    return transportForDevice.getOrDefault (deviceId, 0);
                }
                catch (NullPointerException e)
                {
                    return 0;
                }
            }
        }

        @Override
        public void onDeviceOpened (MidiDevice theDevice)
        {
            final int transport = getTransportForDevice (theDevice.getInfo().getId());

            if (transport == 0)
                return;

            synchronized (MidiDeviceManager.class)
            {
                MidiDeviceInfo info = theDevice.getInfo();
                int deviceID = info.getId();

                // This device is already being opened
                if (openTasks.containsKey (deviceID))
                    return;

                boolean isBluetooth = false;
                BluetoothGatt gatt = null;

                if (info.getType() == MidiDeviceInfo.TYPE_BLUETOOTH)
                {
                    isBluetooth = true;
                    BluetoothDevice btDevice = getBluetoothDevice (info);

                    if (btDevice != null)
                    {
                        String btAddress = btDevice.getAddress();

                        if (btDevicesPairing.containsKey (btAddress))
                        {
                            gatt = btDevicesPairing.get (btAddress);
                            btDevicesPairing.remove (btAddress);
                        }
                    }
                }

                DiscoveredDevice discovered = new DiscoveredDevice (theDevice, gatt, transport);
                MidiDeviceOpenTask openTask = new MidiDeviceOpenTask (this, discovered);
                openTasks.put (deviceID, openTask);
                new Timer().schedule (openTask, (isBluetooth ? 2000 : 100));
            }
        }

        public void onDeviceOpenedDelayed (DiscoveredDevice theDevice)
        {
            synchronized (MidiDeviceManager.class)
            {
                int deviceID = theDevice.device.getInfo().getId();

                if (openTasks.containsKey (deviceID))
                {
                    if (! midiDevices.contains (theDevice))
                    {
                        openTasks.remove (deviceID);
                        midiDevices.add (theDevice);

                        JuceMidiDeviceInfo info = new JuceMidiDeviceInfo();
                        info.name         = theDevice.device.getInfo().getProperties().getString (MidiDeviceInfo.PROPERTY_NAME);
                        info.manufacturer = theDevice.device.getInfo().getProperties().getString (MidiDeviceInfo.PROPERTY_MANUFACTURER);
                        info.product      = theDevice.device.getInfo().getProperties().getString (MidiDeviceInfo.PROPERTY_PRODUCT);
                        info.serialNumber = theDevice.device.getInfo().getProperties().getString (MidiDeviceInfo.PROPERTY_SERIAL_NUMBER);
                        info.id = theDevice.device.getInfo().getId();
                        info.transport = theDevice.transport;
                        info.type = theDevice.device.getInfo().getType();

                        info.dst = new ArrayList<>();
                        info.src = new ArrayList<>();

                        for (int i = 0; i < theDevice.device.getInfo().getInputPortCount(); ++i)
                            info.dst.add ("");

                        for (int i = 0; i < theDevice.device.getInfo().getOutputPortCount(); ++i)
                            info.src.add ("");

                        for (MidiDeviceInfo.PortInfo i : theDevice.device.getInfo().getPorts())
                        {
                            ArrayList<String> target = i.getType() == MidiDeviceInfo.PortInfo.TYPE_INPUT ? info.dst : info.src;
                            target.set (i.getPortNumber(), i.getName());
                        }

                        info.defaultProtocol = Build.VERSION_CODES.TIRAMISU <= Build.VERSION.SDK_INT
                                             ? theDevice.device.getInfo().getDefaultProtocol()
                                             : -1;

                        handleDeviceAdded (info);
                    }
                }
                else
                {
                    // unpair was called in the meantime
                    MidiDeviceInfo info = theDevice.device.getInfo();
                    BluetoothDevice btDevice = getBluetoothDevice (info);

                    if (btDevice != null)
                    {
                        DiscoveredDevice discoveredDevice = findMidiDeviceForBluetoothAddress (btDevice.getAddress());
                        if (discoveredDevice != null)
                        {
                            BluetoothGatt gatt = discoveredDevice.gatt;

                            if (gatt != null)
                            {
                                gatt.disconnect();
                                gatt.close();
                            }
                        }
                    }

                    try
                    {
                        theDevice.device.close();
                    }
                    catch (IOException e)
                    {
                    }
                }
            }
        }

        private DiscoveredDevice getDiscoveredDeviceForId (int deviceId)
        {
            synchronized (MidiDeviceManager.class)
            {
                for (DiscoveredDevice midiDevice : midiDevices)
                    if (midiDevice.device.getInfo().getId() == deviceId)
                        return midiDevice;
            }

            return null;
        }

        class DiscoveredDevice
        {
            DiscoveredDevice (MidiDevice deviceIn, BluetoothGatt gattIn, int transportIn)
            {
                device = deviceIn;
                gatt = gattIn;
                transport = transportIn;
            }

            MidiDevice device;
            BluetoothGatt gatt;
            int transport;
        }

        private final Executor executor = Executors.newSingleThreadExecutor();
        private final MidiManager manager;
        private final HashMap<String, BluetoothGatt> btDevicesPairing = new HashMap<String, BluetoothGatt>();
        private HashMap<Integer, MidiDeviceOpenTask> openTasks = new HashMap<Integer, MidiDeviceOpenTask>();
        private final HashMap<Integer, Integer> transportForDevice = new HashMap<>();
        private final ArrayList<DiscoveredDevice> midiDevices = new ArrayList<>();
        private final ArrayList<DeviceCallbackImpl> deviceCallbacks = new ArrayList<>();
        private HashMap<PortPath, WeakReference<JuceMidiPort>> openPorts = new HashMap<PortPath, WeakReference<JuceMidiPort>>();
        private Context appContext = null;
    }

    public static MidiDeviceManager getAndroidMidiDeviceManager (Context context)
    {
        if (context.getSystemService (MIDI_SERVICE) == null)
            return null;

        synchronized (JuceMidiSupport.class)
        {
            if (midiDeviceManager == null)
                midiDeviceManager = new MidiDeviceManager (context);
        }

        return midiDeviceManager;
    }

    public static BluetoothMidiManager getAndroidBluetoothManager (Context context)
    {
        BluetoothAdapter adapter = getDefaultBluetoothAdapter (context);

        if (adapter == null)
            return null;

        if (adapter.getBluetoothLeScanner() == null)
            return null;

        synchronized (JuceMidiSupport.class)
        {
            if (bluetoothManager == null)
                bluetoothManager = new BluetoothMidiManager (context);
        }

        return bluetoothManager;
    }

    private static MidiDeviceManager midiDeviceManager = null;
    private static BluetoothMidiManager bluetoothManager = null;
}
