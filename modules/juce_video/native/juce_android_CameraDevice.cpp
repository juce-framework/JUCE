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

struct CameraDevice::Pimpl
{
    Pimpl (const String&, int /*index*/, int /*minWidth*/, int /*minHeight*/, int /*maxWidth*/, int /*maxHeight*/)
    {
    }

    ~Pimpl()
    {
    }

    void startRecordingToFile (const File&, int /*quality*/)
    {
    }

    void stopRecording()
    {
    }

    Time getTimeOfFirstRecordedFrame() const
    {
        return Time();
    }

    void addListener (CameraDevice::Listener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);
        listeners.addIfNotAlreadyThere (listenerToAdd);
    }

    void removeListener (CameraDevice::Listener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.removeFirstMatchingValue (listenerToRemove);
    }

    static StringArray getAvailableDevices()
    {
        StringArray results;

        return results;
    }

private:
    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};

struct CameraDevice::ViewerComponent  : public Component
{
    ViewerComponent (CameraDevice&)
    {
    }

    JUCE_DECLARE_NON_COPYABLE (ViewerComponent)
};

String CameraDevice::getFileExtension()
{
    return ".mov";
}
