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

package com.juce.jucedemo;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Looper;
import android.os.Handler;
import android.os.ParcelUuid;
import android.os.Environment;
import android.view.*;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.graphics.*;
import android.text.ClipboardManager;
import android.text.InputType;
import android.util.DisplayMetrics;
import android.util.Log;
import java.lang.Runnable;
import java.util.*;
import java.io.*;
import java.net.URL;
import java.net.HttpURLConnection;
import android.media.AudioManager;
import android.media.MediaScannerConnection;
import android.media.MediaScannerConnection.MediaScannerConnectionClient;
import android.support.v4.content.ContextCompat;
import android.support.v4.app.ActivityCompat;
import android.Manifest;

import android.media.midi.*;
import android.bluetooth.*;
import android.bluetooth.le.*;


//==============================================================================
public class JuceDemo   extends Activity
{
    //==============================================================================
    static
    {
        System.loadLibrary ("juce_jni");
    }

    //==============================================================================
    public boolean isPermissionDeclaredInManifest (int permissionID)
    {
        String permissionToCheck = getAndroidPermissionName(permissionID);

        try
        {
            PackageInfo info = getPackageManager().getPackageInfo(getApplicationContext().getPackageName(), PackageManager.GET_PERMISSIONS);

            if (info.requestedPermissions != null)
                for (String permission : info.requestedPermissions)
                    if (permission.equals (permissionToCheck))
                        return true;
        }
        catch (PackageManager.NameNotFoundException e)
        {
            Log.d ("JUCE", "isPermissionDeclaredInManifest: PackageManager.NameNotFoundException = " + e.toString());
        }

        Log.d ("JUCE", "isPermissionDeclaredInManifest: could not find requested permission " + permissionToCheck);
        return false;
    }

    //==============================================================================
    // these have to match the values of enum PermissionID in C++ class RuntimePermissions:
    private static final int JUCE_PERMISSIONS_RECORD_AUDIO = 1;
    private static final int JUCE_PERMISSIONS_BLUETOOTH_MIDI = 2;

    private static String getAndroidPermissionName (int permissionID)
    {
        switch (permissionID)
        {
            case JUCE_PERMISSIONS_RECORD_AUDIO:     return Manifest.permission.RECORD_AUDIO;
            case JUCE_PERMISSIONS_BLUETOOTH_MIDI:   return Manifest.permission.ACCESS_COARSE_LOCATION;
        }

        // unknown permission ID!
        assert false;
        return new String();
    }

    public boolean isPermissionGranted (int permissionID)
    {
        return ContextCompat.checkSelfPermission (this, getAndroidPermissionName (permissionID)) == PackageManager.PERMISSION_GRANTED;
    }

    private Map<Integer, Long> permissionCallbackPtrMap;

    public void requestRuntimePermission (int permissionID, long ptrToCallback)
    {
        String permissionName = getAndroidPermissionName (permissionID);

        if (ContextCompat.checkSelfPermission (this, permissionName) != PackageManager.PERMISSION_GRANTED)
        {
            // remember callbackPtr, request permissions, and let onRequestPermissionResult call callback asynchronously
            permissionCallbackPtrMap.put (permissionID, ptrToCallback);
            ActivityCompat.requestPermissions (this, new String[]{permissionName}, permissionID);
        }
        else
        {
            // permissions were already granted before, we can call callback directly
            androidRuntimePermissionsCallback (true, ptrToCallback);
        }
    }

    private native void androidRuntimePermissionsCallback (boolean permissionWasGranted, long ptrToCallback);

    @Override
    public void onRequestPermissionsResult (int permissionID, String permissions[], int[] grantResults)
    {
        boolean permissionsGranted = (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED);

        if (! permissionsGranted)
            Log.d ("JUCE", "onRequestPermissionsResult: runtime permission was DENIED: " + getAndroidPermissionName (permissionID));

        Long ptrToCallback = permissionCallbackPtrMap.get (permissionID);
        permissionCallbackPtrMap.remove (permissionID);
        androidRuntimePermissionsCallback (permissionsGranted, ptrToCallback);
    }

    //==============================================================================
    public static class MidiPortID extends Object
    {
        public MidiPortID (int index, boolean direction)
        {
            androidIndex = index;
            isInput = direction;
        }

        public int androidIndex;
        public boolean isInput;

        @Override
        public int hashCode()
        {
            Integer i = new Integer (androidIndex);
            return i.hashCode() * (isInput ? -1 : 1);
        }

        @Override
        public boolean equals (Object obj)
        {
            if (obj == null)
                return false;

            if (getClass() != obj.getClass())
                return false;

            MidiPortID other = (MidiPortID) obj;
            return (androidIndex == other.androidIndex && isInput == other.isInput);
        }
    }

    public interface JuceMidiPort
    {
        boolean isInputPort();

        // start, stop does nothing on an output port
        void start();
        void stop();

        void close();
        MidiPortID getPortId();

        // send will do nothing on an input port
        void sendMidi (byte[] msg, int offset, int count);
    }

    //==============================================================================
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

        synchronized (JuceDemo.class)
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

        synchronized (JuceDemo.class)
        {
            if (bluetoothManager == null)
                bluetoothManager = new BluetoothManager();
        }

        return bluetoothManager;
    }

    //==============================================================================
    @Override
    public void onCreate (Bundle savedInstanceState)
    {
        super.onCreate (savedInstanceState);

        isScreenSaverEnabled = true;
        hideActionBar();
        viewHolder = new ViewHolder (this);
        setContentView (viewHolder);

        setVolumeControlStream (AudioManager.STREAM_MUSIC);

        permissionCallbackPtrMap = new HashMap<Integer, Long>();
    }

    @Override
    protected void onDestroy()
    {
        quitApp();
        super.onDestroy();

        clearDataCache();
    }

    @Override
    protected void onPause()
    {
        suspendApp();

        try
        {
            Thread.sleep (1000); // This is a bit of a hack to avoid some hard-to-track-down
                                 // openGL glitches when pausing/resuming apps..
        } catch (InterruptedException e) {}

        super.onPause();
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        resumeApp();
    }

    @Override
    public void onConfigurationChanged (Configuration cfg)
    {
        super.onConfigurationChanged (cfg);
        setContentView (viewHolder);
    }

    private void callAppLauncher()
    {
        launchApp (getApplicationInfo().publicSourceDir,
                   getApplicationInfo().dataDir);
    }

    private void hideActionBar()
    {
        // get "getActionBar" method
        java.lang.reflect.Method getActionBarMethod = null;
        try
        {
            getActionBarMethod = this.getClass().getMethod ("getActionBar");
        }
        catch (SecurityException e)     { return; }
        catch (NoSuchMethodException e) { return; }
        if (getActionBarMethod == null) return;

        // invoke "getActionBar" method
        Object actionBar = null;
        try
        {
            actionBar = getActionBarMethod.invoke (this);
        }
        catch (java.lang.IllegalArgumentException e) { return; }
        catch (java.lang.IllegalAccessException e) { return; }
        catch (java.lang.reflect.InvocationTargetException e) { return; }
        if (actionBar == null) return;

        // get "hide" method
        java.lang.reflect.Method actionBarHideMethod = null;
        try
        {
            actionBarHideMethod = actionBar.getClass().getMethod ("hide");
        }
        catch (SecurityException e)     { return; }
        catch (NoSuchMethodException e) { return; }
        if (actionBarHideMethod == null) return;

        // invoke "hide" method
        try
        {
            actionBarHideMethod.invoke (actionBar);
        }
        catch (java.lang.IllegalArgumentException e) {}
        catch (java.lang.IllegalAccessException e) {}
        catch (java.lang.reflect.InvocationTargetException e) {}
    }

    //==============================================================================
    private native void launchApp (String appFile, String appDataDir);
    private native void quitApp();
    private native void suspendApp();
    private native void resumeApp();
    private native void setScreenSize (int screenWidth, int screenHeight, int dpi);

    //==============================================================================
    public native void deliverMessage (long value);
    private android.os.Handler messageHandler = new android.os.Handler();

    public final void postMessage (long value)
    {
        messageHandler.post (new MessageCallback (value));
    }

    private final class MessageCallback  implements Runnable
    {
        public MessageCallback (long value_)        { value = value_; }
        public final void run()                     { deliverMessage (value); }

        private long value;
    }

    //==============================================================================
    private ViewHolder viewHolder;
    private MidiDeviceManager midiDeviceManager = null;
    private BluetoothManager bluetoothManager = null;
    private boolean isScreenSaverEnabled;
    private java.util.Timer keepAliveTimer;

    public final ComponentPeerView createNewView (boolean opaque, long host)
    {
        ComponentPeerView v = new ComponentPeerView (this, opaque, host);
        viewHolder.addView (v);
        return v;
    }

    public final void deleteView (ComponentPeerView view)
    {
        ViewGroup group = (ViewGroup) (view.getParent());

        if (group != null)
            group.removeView (view);
    }

    public final void deleteNativeSurfaceView (NativeSurfaceView view)
    {
        ViewGroup group = (ViewGroup) (view.getParent());

        if (group != null)
            group.removeView (view);
    }

    final class ViewHolder  extends ViewGroup
    {
        public ViewHolder (Context context)
        {
            super (context);
            setDescendantFocusability (ViewGroup.FOCUS_AFTER_DESCENDANTS);
            setFocusable (false);
        }

        protected final void onLayout (boolean changed, int left, int top, int right, int bottom)
        {
            setScreenSize (getWidth(), getHeight(), getDPI());

            if (isFirstResize)
            {
                isFirstResize = false;
                callAppLauncher();
            }
        }

        private final int getDPI()
        {
            DisplayMetrics metrics = new DisplayMetrics();
            getWindowManager().getDefaultDisplay().getMetrics (metrics);
            return metrics.densityDpi;
        }

        private boolean isFirstResize = true;
    }

    public final void excludeClipRegion (android.graphics.Canvas canvas, float left, float top, float right, float bottom)
    {
        canvas.clipRect (left, top, right, bottom, android.graphics.Region.Op.DIFFERENCE);
    }

    //==============================================================================
    public final void setScreenSaver (boolean enabled)
    {
        if (isScreenSaverEnabled != enabled)
        {
            isScreenSaverEnabled = enabled;

            if (keepAliveTimer != null)
            {
                keepAliveTimer.cancel();
                keepAliveTimer = null;
            }

            if (enabled)
            {
                getWindow().clearFlags (WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            }
            else
            {
                getWindow().addFlags (WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

                // If no user input is received after about 3 seconds, the OS will lower the
                // task's priority, so this timer forces it to be kept active.
                keepAliveTimer = new java.util.Timer();

                keepAliveTimer.scheduleAtFixedRate (new TimerTask()
                {
                    @Override
                    public void run()
                    {
                        android.app.Instrumentation instrumentation = new android.app.Instrumentation();

                        try
                        {
                            instrumentation.sendKeyDownUpSync (KeyEvent.KEYCODE_UNKNOWN);
                        }
                        catch (Exception e)
                        {
                        }
                    }
                }, 2000, 2000);
            }
        }
    }

    public final boolean getScreenSaver()
    {
        return isScreenSaverEnabled;
    }

    //==============================================================================
    public final String getClipboardContent()
    {
        ClipboardManager clipboard = (ClipboardManager) getSystemService (CLIPBOARD_SERVICE);
        return clipboard.getText().toString();
    }

    public final void setClipboardContent (String newText)
    {
        ClipboardManager clipboard = (ClipboardManager) getSystemService (CLIPBOARD_SERVICE);
        clipboard.setText (newText);
    }

    //==============================================================================
    public final void showMessageBox (String title, String message, final long callback)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder (this);
        builder.setTitle (title)
               .setMessage (message)
               .setCancelable (true)
               .setPositiveButton ("OK", new DialogInterface.OnClickListener()
                    {
                        public void onClick (DialogInterface dialog, int id)
                        {
                            dialog.cancel();
                            JuceDemo.this.alertDismissed (callback, 0);
                        }
                    });

        builder.create().show();
    }

    public final void showOkCancelBox (String title, String message, final long callback)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder (this);
        builder.setTitle (title)
               .setMessage (message)
               .setCancelable (true)
               .setPositiveButton ("OK", new DialogInterface.OnClickListener()
                    {
                        public void onClick (DialogInterface dialog, int id)
                        {
                            dialog.cancel();
                            JuceDemo.this.alertDismissed (callback, 1);
                        }
                    })
               .setNegativeButton ("Cancel", new DialogInterface.OnClickListener()
                    {
                        public void onClick (DialogInterface dialog, int id)
                        {
                            dialog.cancel();
                            JuceDemo.this.alertDismissed (callback, 0);
                        }
                    });

        builder.create().show();
    }

    public final void showYesNoCancelBox (String title, String message, final long callback)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder (this);
        builder.setTitle (title)
               .setMessage (message)
               .setCancelable (true)
               .setPositiveButton ("Yes", new DialogInterface.OnClickListener()
                    {
                        public void onClick (DialogInterface dialog, int id)
                        {
                            dialog.cancel();
                            JuceDemo.this.alertDismissed (callback, 1);
                        }
                    })
               .setNegativeButton ("No", new DialogInterface.OnClickListener()
                    {
                        public void onClick (DialogInterface dialog, int id)
                        {
                            dialog.cancel();
                            JuceDemo.this.alertDismissed (callback, 2);
                        }
                    })
               .setNeutralButton ("Cancel", new DialogInterface.OnClickListener()
                    {
                        public void onClick (DialogInterface dialog, int id)
                        {
                            dialog.cancel();
                            JuceDemo.this.alertDismissed (callback, 0);
                        }
                    });

        builder.create().show();
    }

    public native void alertDismissed (long callback, int id);

    //==============================================================================
    public final class ComponentPeerView extends ViewGroup
                                         implements View.OnFocusChangeListener
    {
        public ComponentPeerView (Context context, boolean opaque_, long host)
        {
            super (context);
            this.host = host;
            setWillNotDraw (false);
            opaque = opaque_;

            setFocusable (true);
            setFocusableInTouchMode (true);
            setOnFocusChangeListener (this);
            requestFocus();

            // swap red and blue colours to match internal opengl texture format
            ColorMatrix colorMatrix = new ColorMatrix();

            float[] colorTransform = { 0,    0,    1.0f, 0,    0,
                                       0,    1.0f, 0,    0,    0,
                                       1.0f, 0,    0,    0,    0,
                                       0,    0,    0,    1.0f, 0 };

            colorMatrix.set (colorTransform);
            paint.setColorFilter (new ColorMatrixColorFilter (colorMatrix));
        }

        //==============================================================================
        private native void handlePaint (long host, Canvas canvas, Paint paint);

        @Override
        public void onDraw (Canvas canvas)
        {
            handlePaint (host, canvas, paint);
        }

        @Override
        public boolean isOpaque()
        {
            return opaque;
        }

        private boolean opaque;
        private long host;
        private Paint paint = new Paint();

        //==============================================================================
        private native void handleMouseDown (long host, int index, float x, float y, long time);
        private native void handleMouseDrag (long host, int index, float x, float y, long time);
        private native void handleMouseUp   (long host, int index, float x, float y, long time);

        @Override
        public boolean onTouchEvent (MotionEvent event)
        {
            int action = event.getAction();
            long time = event.getEventTime();

            switch (action & MotionEvent.ACTION_MASK)
            {
                case MotionEvent.ACTION_DOWN:
                    handleMouseDown (host, event.getPointerId(0), event.getX(), event.getY(), time);
                    return true;

                case MotionEvent.ACTION_CANCEL:
                case MotionEvent.ACTION_UP:
                    handleMouseUp (host, event.getPointerId(0), event.getX(), event.getY(), time);
                    return true;

                case MotionEvent.ACTION_MOVE:
                {
                    int n = event.getPointerCount();
                    for (int i = 0; i < n; ++i)
                        handleMouseDrag (host, event.getPointerId(i), event.getX(i), event.getY(i), time);

                    return true;
                }

                case MotionEvent.ACTION_POINTER_UP:
                {
                    int i = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
                    handleMouseUp (host, event.getPointerId(i), event.getX(i), event.getY(i), time);
                    return true;
                }

                case MotionEvent.ACTION_POINTER_DOWN:
                {
                    int i = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
                    handleMouseDown (host, event.getPointerId(i), event.getX(i), event.getY(i), time);
                    return true;
                }

                default:
                    break;
            }

            return false;
        }

        //==============================================================================
        private native void handleKeyDown (long host, int keycode, int textchar);
        private native void handleKeyUp (long host, int keycode, int textchar);

        public void showKeyboard (String type)
        {
            InputMethodManager imm = (InputMethodManager) getSystemService (Context.INPUT_METHOD_SERVICE);

            if (imm != null)
            {
                if (type.length() > 0)
                {
                    imm.showSoftInput (this, android.view.inputmethod.InputMethodManager.SHOW_IMPLICIT);
                    imm.setInputMethod (getWindowToken(), type);
                }
                else
                {
                    imm.hideSoftInputFromWindow (getWindowToken(), 0);
                }
            }
        }

        @Override
        public boolean onKeyDown (int keyCode, KeyEvent event)
        {
            switch (keyCode)
            {
                case KeyEvent.KEYCODE_VOLUME_UP:
                case KeyEvent.KEYCODE_VOLUME_DOWN:
                    return super.onKeyDown (keyCode, event);

                default: break;
            }

            handleKeyDown (host, keyCode, event.getUnicodeChar());
            return true;
        }

        @Override
        public boolean onKeyUp (int keyCode, KeyEvent event)
        {
            handleKeyUp (host, keyCode, event.getUnicodeChar());
            return true;
        }

        @Override
        public boolean onKeyMultiple (int keyCode, int count, KeyEvent event)
        {
            if (keyCode != KeyEvent.KEYCODE_UNKNOWN || event.getAction() != KeyEvent.ACTION_MULTIPLE)
                return super.onKeyMultiple (keyCode, count, event);

            if (event.getCharacters() != null)
            {
                int utf8Char = event.getCharacters().codePointAt (0);
                handleKeyDown (host, utf8Char, utf8Char);
                return true;
            }

            return false;
        }

        // this is here to make keyboard entry work on a Galaxy Tab2 10.1
        @Override
        public InputConnection onCreateInputConnection (EditorInfo outAttrs)
        {
            outAttrs.actionLabel = "";
            outAttrs.hintText = "";
            outAttrs.initialCapsMode = 0;
            outAttrs.initialSelEnd = outAttrs.initialSelStart = -1;
            outAttrs.label = "";
            outAttrs.imeOptions = EditorInfo.IME_ACTION_DONE | EditorInfo.IME_FLAG_NO_EXTRACT_UI;
            outAttrs.inputType = InputType.TYPE_NULL;

            return new BaseInputConnection (this, false);
        }

        //==============================================================================
        @Override
        protected void onSizeChanged (int w, int h, int oldw, int oldh)
        {
            super.onSizeChanged (w, h, oldw, oldh);
            viewSizeChanged (host);
        }

        @Override
        protected void onLayout (boolean changed, int left, int top, int right, int bottom)
        {
            for (int i = getChildCount(); --i >= 0;)
                requestTransparentRegion (getChildAt (i));
        }

        private native void viewSizeChanged (long host);

        @Override
        public void onFocusChange (View v, boolean hasFocus)
        {
            if (v == this)
                focusChanged (host, hasFocus);
        }

        private native void focusChanged (long host, boolean hasFocus);

        public void setViewName (String newName)    {}

        public boolean isVisible()                  { return getVisibility() == VISIBLE; }
        public void setVisible (boolean b)          { setVisibility (b ? VISIBLE : INVISIBLE); }

        public boolean containsPoint (int x, int y)
        {
            return true; //xxx needs to check overlapping views
        }
    }

    //==============================================================================
    public static class NativeSurfaceView    extends SurfaceView
                                          implements SurfaceHolder.Callback
    {
        private long nativeContext = 0;

        NativeSurfaceView (Context context, long nativeContextPtr)
        {
            super (context);
            nativeContext = nativeContextPtr;
        }

        public Surface getNativeSurface()
        {
            Surface retval = null;

            SurfaceHolder holder = getHolder();
            if (holder != null)
                retval = holder.getSurface();

            return retval;
        }

        //==============================================================================
        @Override
        public void surfaceChanged (SurfaceHolder holder, int format, int width, int height)
        {
            surfaceChangedNative (nativeContext, holder, format, width, height);
        }

        @Override
        public void surfaceCreated (SurfaceHolder holder)
        {
            surfaceCreatedNative (nativeContext, holder);
        }

        @Override
        public void surfaceDestroyed (SurfaceHolder holder)
        {
            surfaceDestroyedNative (nativeContext, holder);
        }

        @Override
        protected void dispatchDraw (Canvas canvas)
        {
            super.dispatchDraw (canvas);
            dispatchDrawNative (nativeContext, canvas);
        }

        //==============================================================================
        @Override
        protected void onAttachedToWindow ()
        {
            super.onAttachedToWindow();
            getHolder().addCallback (this);
        }

        @Override
        protected void onDetachedFromWindow ()
        {
            super.onDetachedFromWindow();
            getHolder().removeCallback (this);
        }

        //==============================================================================
        private native void dispatchDrawNative (long nativeContextPtr, Canvas canvas);
        private native void surfaceCreatedNative (long nativeContextptr, SurfaceHolder holder);
        private native void surfaceDestroyedNative (long nativeContextptr, SurfaceHolder holder);
        private native void surfaceChangedNative (long nativeContextptr, SurfaceHolder holder,
                                                  int format, int width, int height);
    }

    public NativeSurfaceView createNativeSurfaceView (long nativeSurfacePtr)
    {
        return new NativeSurfaceView (this, nativeSurfacePtr);
    }

    //==============================================================================
    public final int[] renderGlyph (char glyph, Paint paint, android.graphics.Matrix matrix, Rect bounds)
    {
        Path p = new Path();
        paint.getTextPath (String.valueOf (glyph), 0, 1, 0.0f, 0.0f, p);

        RectF boundsF = new RectF();
        p.computeBounds (boundsF, true);
        matrix.mapRect (boundsF);

        boundsF.roundOut (bounds);
        bounds.left--;
        bounds.right++;

        final int w = bounds.width();
        final int h = Math.max (1, bounds.height());

        Bitmap bm = Bitmap.createBitmap (w, h, Bitmap.Config.ARGB_8888);

        Canvas c = new Canvas (bm);
        matrix.postTranslate (-bounds.left, -bounds.top);
        c.setMatrix (matrix);
        c.drawPath (p, paint);

        final int sizeNeeded = w * h;
        if (cachedRenderArray.length < sizeNeeded)
            cachedRenderArray = new int [sizeNeeded];

        bm.getPixels (cachedRenderArray, 0, w, 0, 0, w, h);
        bm.recycle();
        return cachedRenderArray;
    }

    private int[] cachedRenderArray = new int [256];

    //==============================================================================
    public static class HTTPStream
    {
        public HTTPStream (HttpURLConnection connection_,
                           int[] statusCode, StringBuffer responseHeaders) throws IOException
        {
            connection = connection_;

            try
            {
                inputStream = new BufferedInputStream (connection.getInputStream());
            }
            catch (IOException e)
            {
                if (connection.getResponseCode() < 400)
                    throw e;
            }
            finally
            {
                statusCode[0] = connection.getResponseCode();
            }

            if (statusCode[0] >= 400)
                inputStream = connection.getErrorStream();
            else
                inputStream = connection.getInputStream();

            for (java.util.Map.Entry<String, java.util.List<String>> entry : connection.getHeaderFields().entrySet())
                if (entry.getKey() != null && entry.getValue() != null)
                    responseHeaders.append (entry.getKey() + ": "
                                             + android.text.TextUtils.join (",", entry.getValue()) + "\n");
        }

        public final void release()
        {
            try
            {
                inputStream.close();
            }
            catch (IOException e)
            {}

            connection.disconnect();
        }

        public final int read (byte[] buffer, int numBytes)
        {
            int num = 0;

            try
            {
                num = inputStream.read (buffer, 0, numBytes);
            }
            catch (IOException e)
            {}

            if (num > 0)
                position += num;

            return num;
        }

        public final long getPosition()                 { return position; }
        public final long getTotalLength()              { return -1; }
        public final boolean isExhausted()              { return false; }
        public final boolean setPosition (long newPos)  { return false; }

        private HttpURLConnection connection;
        private InputStream inputStream;
        private long position;
    }

    public static final HTTPStream createHTTPStream (String address, boolean isPost, byte[] postData,
                                                     String headers, int timeOutMs, int[] statusCode,
                                                     StringBuffer responseHeaders, int numRedirectsToFollow,
                                                     String httpRequestCmd)
    {
        // timeout parameter of zero for HttpUrlConnection is a blocking connect (negative value for juce::URL)
        if (timeOutMs < 0)
            timeOutMs = 0;
        else if (timeOutMs == 0)
            timeOutMs = 30000;

        // headers - if not empty, this string is appended onto the headers that are used for the request. It must therefore be a valid set of HTML header directives, separated by newlines.
        // So convert headers string to an array, with an element for each line
        String headerLines[] = headers.split("\\n");

        for (;;)
        {
            try
            {
                HttpURLConnection connection = (HttpURLConnection) (new URL(address).openConnection());

                if (connection != null)
                {
                    try
                    {
                        connection.setInstanceFollowRedirects (false);
                        connection.setConnectTimeout (timeOutMs);
                        connection.setReadTimeout (timeOutMs);

                        // Set request headers
                        for (int i = 0; i < headerLines.length; ++i)
                        {
                            int pos = headerLines[i].indexOf (":");

                            if (pos > 0 && pos < headerLines[i].length())
                            {
                                String field = headerLines[i].substring (0, pos);
                                String value = headerLines[i].substring (pos + 1);

                                if (value.length() > 0)
                                    connection.setRequestProperty (field, value);
                            }
                        }

                        connection.setRequestMethod (httpRequestCmd);
                        if (isPost)
                        {
                            connection.setDoOutput (true);

                            if (postData != null)
                            {
                                OutputStream out = connection.getOutputStream();
                                out.write(postData);
                                out.flush();
                            }
                        }

                        HTTPStream httpStream = new HTTPStream (connection, statusCode, responseHeaders);

                        // Process redirect & continue as necessary
                        int status = statusCode[0];

                        if (--numRedirectsToFollow >= 0
                             && (status == 301 || status == 302 || status == 303 || status == 307))
                        {
                            // Assumes only one occurrence of "Location"
                            int pos1 = responseHeaders.indexOf ("Location:") + 10;
                            int pos2 = responseHeaders.indexOf ("\n", pos1);

                            if (pos2 > pos1)
                            {
                                String newLocation = responseHeaders.substring(pos1, pos2);
                                // Handle newLocation whether it's absolute or relative
                                URL baseUrl = new URL (address);
                                URL newUrl = new URL (baseUrl, newLocation);
                                String transformedNewLocation = newUrl.toString();

                                if (transformedNewLocation != address)
                                {
                                    address = transformedNewLocation;
                                    // Clear responseHeaders before next iteration
                                    responseHeaders.delete (0, responseHeaders.length());
                                    continue;
                                }
                            }
                        }

                        return httpStream;
                    }
                    catch (Throwable e)
                    {
                        connection.disconnect();
                    }
                }
            }
            catch (Throwable e) {}

            return null;
        }
    }

    public final void launchURL (String url)
    {
        startActivity (new Intent (Intent.ACTION_VIEW, Uri.parse (url)));
    }

    public static final String getLocaleValue (boolean isRegion)
    {
        java.util.Locale locale = java.util.Locale.getDefault();

        return isRegion ? locale.getDisplayCountry  (java.util.Locale.US)
                        : locale.getDisplayLanguage (java.util.Locale.US);
    }

    private static final String getFileLocation (String type)
    {
        return Environment.getExternalStoragePublicDirectory (type).getAbsolutePath();
    }

    public static final String getDocumentsFolder()  { return Environment.getDataDirectory().getAbsolutePath(); }
    public static final String getPicturesFolder()   { return getFileLocation (Environment.DIRECTORY_PICTURES); }
    public static final String getMusicFolder()      { return getFileLocation (Environment.DIRECTORY_MUSIC); }
    public static final String getMoviesFolder()     { return getFileLocation (Environment.DIRECTORY_MOVIES); }
    public static final String getDownloadsFolder()  { return getFileLocation (Environment.DIRECTORY_DOWNLOADS); }

    //==============================================================================
    private final class SingleMediaScanner  implements MediaScannerConnectionClient
    {
        public SingleMediaScanner (Context context, String filename)
        {
            file = filename;
            msc = new MediaScannerConnection (context, this);
            msc.connect();
        }

        @Override
        public void onMediaScannerConnected()
        {
            msc.scanFile (file, null);
        }

        @Override
        public void onScanCompleted (String path, Uri uri)
        {
            msc.disconnect();
        }

        private MediaScannerConnection msc;
        private String file;
    }

    public final void scanFile (String filename)
    {
        new SingleMediaScanner (this, filename);
    }

    public final Typeface getTypeFaceFromAsset (String assetName)
    {
        try
        {
            return Typeface.createFromAsset (this.getResources().getAssets(), assetName);
        }
        catch (Throwable e) {}

        return null;
    }

    final protected static char[] hexArray = "0123456789ABCDEF".toCharArray();

    public static String bytesToHex (byte[] bytes)
    {
        char[] hexChars = new char[bytes.length * 2];

        for (int j = 0; j < bytes.length; ++j)
        {
            int v = bytes[j] & 0xff;
            hexChars[j * 2]     = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0f];
        }

        return new String (hexChars);
    }

    final private java.util.Map dataCache = new java.util.HashMap();

    synchronized private final File getDataCacheFile (byte[] data)
    {
        try
        {
            java.security.MessageDigest digest = java.security.MessageDigest.getInstance ("MD5");
            digest.update (data);

            String key = bytesToHex (digest.digest());

            if (dataCache.containsKey (key))
                return (File) dataCache.get (key);

            File f = new File (this.getCacheDir(), "bindata_" + key);
            f.delete();
            FileOutputStream os = new FileOutputStream (f);
            os.write (data, 0, data.length);
            dataCache.put (key, f);
            return f;
        }
        catch (Throwable e) {}

        return null;
    }

    private final void clearDataCache()
    {
        java.util.Iterator it = dataCache.values().iterator();

        while (it.hasNext())
        {
            File f = (File) it.next();
            f.delete();
        }
    }

    public final Typeface getTypeFaceFromByteArray (byte[] data)
    {
        try
        {
            File f = getDataCacheFile (data);

            if (f != null)
                return Typeface.createFromFile (f);
        }
        catch (Exception e)
        {
            Log.e ("JUCE", e.toString());
        }

        return null;
    }

    public final int getAndroidSDKVersion()
    {
        return android.os.Build.VERSION.SDK_INT;
    }

    public final String audioManagerGetProperty (String property)
    {
        Object obj = getSystemService (AUDIO_SERVICE);
        if (obj == null)
            return null;

        java.lang.reflect.Method method;

        try
        {
            method = obj.getClass().getMethod ("getProperty", String.class);
        }
        catch (SecurityException e)     { return null; }
        catch (NoSuchMethodException e) { return null; }

        if (method == null)
            return null;

        try
        {
            return (String) method.invoke (obj, property);
        }
        catch (java.lang.IllegalArgumentException e) {}
        catch (java.lang.IllegalAccessException e) {}
        catch (java.lang.reflect.InvocationTargetException e) {}

        return null;
    }

    public final int setCurrentThreadPriority (int priority)
    {
        android.os.Process.setThreadPriority (android.os.Process.myTid(), priority);
        return android.os.Process.getThreadPriority (android.os.Process.myTid());
    }

    public final boolean hasSystemFeature (String property)
    {
        return getPackageManager().hasSystemFeature (property);
    }

    private static class JuceThread extends Thread
    {
        public JuceThread (long host, String threadName, long threadStackSize)
        {
            super (null, null, threadName, threadStackSize);
            _this = host;
        }

        public void run()
        {
            runThread(_this);
        }

        private native void runThread (long host);
        private long _this;
    }

    public final Thread createNewThread(long host, String threadName, long threadStackSize)
    {
        return new JuceThread(host, threadName, threadStackSize);
    }
}
