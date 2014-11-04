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

namespace WindowsMediaCodec
{

class JuceIStream   : public ComBaseClassHelper <IStream>
{
public:
    JuceIStream (InputStream& in) noexcept
        : ComBaseClassHelper <IStream> (0), source (in)
    {
    }

    JUCE_COMRESULT Commit (DWORD)                        { return S_OK; }
    JUCE_COMRESULT Write (const void*, ULONG, ULONG*)    { return E_NOTIMPL; }
    JUCE_COMRESULT Clone (IStream**)                     { return E_NOTIMPL; }
    JUCE_COMRESULT SetSize (ULARGE_INTEGER)              { return E_NOTIMPL; }
    JUCE_COMRESULT Revert()                              { return E_NOTIMPL; }
    JUCE_COMRESULT LockRegion (ULARGE_INTEGER, ULARGE_INTEGER, DWORD)    { return E_NOTIMPL; }
    JUCE_COMRESULT UnlockRegion (ULARGE_INTEGER, ULARGE_INTEGER, DWORD)  { return E_NOTIMPL; }

    JUCE_COMRESULT Read (void* dest, ULONG numBytes, ULONG* bytesRead)
    {
        const int numRead = source.read (dest, numBytes);

        if (bytesRead != nullptr)
            *bytesRead = numRead;

        return (numRead == (int) numBytes) ? S_OK : S_FALSE;
    }

    JUCE_COMRESULT Seek (LARGE_INTEGER position, DWORD origin, ULARGE_INTEGER* resultPosition)
    {
        int64 newPos = (int64) position.QuadPart;

        if (origin == STREAM_SEEK_CUR)
        {
            newPos += source.getPosition();
        }
        else if (origin == STREAM_SEEK_END)
        {
            const int64 len = source.getTotalLength();
            if (len < 0)
                return E_NOTIMPL;

            newPos += len;
        }

        if (resultPosition != nullptr)
            resultPosition->QuadPart = newPos;

        return source.setPosition (newPos) ? S_OK : E_NOTIMPL;
    }

    JUCE_COMRESULT CopyTo (IStream* destStream, ULARGE_INTEGER numBytesToDo,
                           ULARGE_INTEGER* bytesRead, ULARGE_INTEGER* bytesWritten)
    {
        uint64 totalCopied = 0;
        int64 numBytes = numBytesToDo.QuadPart;

        while (numBytes > 0 && ! source.isExhausted())
        {
            char buffer [1024];

            const int numToCopy = (int) jmin ((int64) sizeof (buffer), (int64) numBytes);
            const int numRead = source.read (buffer, numToCopy);

            if (numRead <= 0)
                break;

            destStream->Write (buffer, numRead, nullptr);
            totalCopied += numRead;
        }

        if (bytesRead != nullptr)      bytesRead->QuadPart = totalCopied;
        if (bytesWritten != nullptr)   bytesWritten->QuadPart = totalCopied;

        return S_OK;
    }

    JUCE_COMRESULT Stat (STATSTG* stat, DWORD)
    {
        if (stat == nullptr)
            return STG_E_INVALIDPOINTER;

        zerostruct (*stat);
        stat->type = STGTY_STREAM;
        stat->cbSize.QuadPart = jmax ((int64) 0, source.getTotalLength());
        return S_OK;
    }

private:
    InputStream& source;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceIStream)
};

//==============================================================================
static const char* wmFormatName = "Windows Media";
static const char* const extensions[] = { ".mp3", ".wmv", ".asf", ".wm", ".wma", 0 };

//==============================================================================
class WMAudioReader   : public AudioFormatReader
{
public:
    WMAudioReader (InputStream* const input_)
        : AudioFormatReader (input_, TRANS (wmFormatName)),
          wmvCoreLib ("Wmvcore.dll")
    {
        JUCE_LOAD_WINAPI_FUNCTION (wmvCoreLib, WMCreateSyncReader, wmCreateSyncReader,
                                   HRESULT, (IUnknown*, DWORD, IWMSyncReader**))

        if (wmCreateSyncReader != nullptr)
        {
            checkCoInitialiseCalled();

            HRESULT hr = wmCreateSyncReader (nullptr, WMT_RIGHT_PLAYBACK, wmSyncReader.resetAndGetPointerAddress());

            if (SUCCEEDED (hr))
                hr = wmSyncReader->OpenStream (new JuceIStream (*input));

            if (SUCCEEDED (hr))
            {
                WORD streamNum = 1;
                hr = wmSyncReader->GetStreamNumberForOutput (0, &streamNum);
                hr = wmSyncReader->SetReadStreamSamples (streamNum, false);

                scanFileForDetails();
            }
        }
    }

    ~WMAudioReader()
    {
        if (wmSyncReader != nullptr)
            wmSyncReader->Close();
    }

    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples) override
    {
        if (sampleRate <= 0)
            return false;

        checkCoInitialiseCalled();

        const int stride = numChannels * sizeof (int16);

        while (numSamples > 0)
        {
            if (! bufferedRange.contains (startSampleInFile))
            {
                const bool hasJumped = (startSampleInFile != bufferedRange.getEnd());

                if (hasJumped)
                    wmSyncReader->SetRange ((QWORD) (startSampleInFile * 10000000 / (int64) sampleRate), 0);

                ComSmartPtr<INSSBuffer> sampleBuffer;
                QWORD sampleTime, duration;
                DWORD flags, outputNum;
                WORD streamNum;

                HRESULT hr = wmSyncReader->GetNextSample (1, sampleBuffer.resetAndGetPointerAddress(),
                                                          &sampleTime, &duration, &flags, &outputNum, &streamNum);

                if (sampleBuffer != nullptr)
                {
                    BYTE* rawData = nullptr;
                    DWORD dataLength = 0;
                    hr = sampleBuffer->GetBufferAndLength (&rawData, &dataLength);

                    if (dataLength == 0)
                        return false;

                    if (hasJumped)
                        bufferedRange.setStart ((int64) ((sampleTime * (int64) sampleRate) / 10000000));
                    else
                        bufferedRange.setStart (bufferedRange.getEnd()); // (because the positions returned often aren't continguous)

                    bufferedRange.setLength ((int64) (dataLength / stride));

                    buffer.ensureSize ((int) dataLength);
                    memcpy (buffer.getData(), rawData, (size_t) dataLength);
                }
                else if (hr == NS_E_NO_MORE_SAMPLES)
                {
                    bufferedRange.setStart (startSampleInFile);
                    bufferedRange.setLength (256);
                    buffer.ensureSize (256 * stride);
                    buffer.fillWith (0);
                }
                else
                {
                    return false;
                }
            }

            const int offsetInBuffer = (int) (startSampleInFile - bufferedRange.getStart());
            const int16* const rawData = static_cast<const int16*> (addBytesToPointer (buffer.getData(), offsetInBuffer * stride));
            const int numToDo = jmin (numSamples, (int) (bufferedRange.getLength() - offsetInBuffer));

            for (int i = 0; i < numDestChannels; ++i)
            {
                jassert (destSamples[i] != nullptr);

                const int srcChan = jmin (i, (int) numChannels - 1);
                const int16* src = rawData + srcChan;
                int* const dst = destSamples[i] + startOffsetInDestBuffer;

                for (int j = 0; j < numToDo; ++j)
                {
                    dst[j] = ((uint32) *src) << 16;
                    src += numChannels;
                }
            }

            startSampleInFile += numToDo;
            startOffsetInDestBuffer += numToDo;
            numSamples -= numToDo;
        }

        return true;
    }

private:
    DynamicLibrary wmvCoreLib;
    ComSmartPtr<IWMSyncReader> wmSyncReader;
    MemoryBlock buffer;
    Range<int64> bufferedRange;

    void checkCoInitialiseCalled()
    {
        CoInitialize (0);
    }

    void scanFileForDetails()
    {
        ComSmartPtr<IWMHeaderInfo> wmHeaderInfo;
        HRESULT hr = wmSyncReader.QueryInterface (wmHeaderInfo);

        if (SUCCEEDED (hr))
        {
            QWORD lengthInNanoseconds = 0;
            WORD lengthOfLength = sizeof (lengthInNanoseconds);
            WORD streamNum = 0;
            WMT_ATTR_DATATYPE wmAttrDataType;
            hr = wmHeaderInfo->GetAttributeByName (&streamNum, L"Duration", &wmAttrDataType,
                                                   (BYTE*) &lengthInNanoseconds, &lengthOfLength);

            ComSmartPtr<IWMProfile> wmProfile;
            hr = wmSyncReader.QueryInterface (wmProfile);

            if (SUCCEEDED (hr))
            {
                ComSmartPtr<IWMStreamConfig> wmStreamConfig;
                hr = wmProfile->GetStream (0, wmStreamConfig.resetAndGetPointerAddress());

                if (SUCCEEDED (hr))
                {
                    ComSmartPtr<IWMMediaProps> wmMediaProperties;
                    hr = wmStreamConfig.QueryInterface (wmMediaProperties);

                    if (SUCCEEDED (hr))
                    {
                        DWORD sizeMediaType;
                        hr = wmMediaProperties->GetMediaType (0, &sizeMediaType);

                        HeapBlock<WM_MEDIA_TYPE> mediaType;
                        mediaType.malloc (sizeMediaType, 1);
                        hr = wmMediaProperties->GetMediaType (mediaType, &sizeMediaType);

                        if (mediaType->majortype == WMMEDIATYPE_Audio)
                        {
                            const WAVEFORMATEX* const inputFormat = reinterpret_cast<WAVEFORMATEX*> (mediaType->pbFormat);

                            sampleRate = inputFormat->nSamplesPerSec;
                            numChannels = inputFormat->nChannels;
                            bitsPerSample = inputFormat->wBitsPerSample;
                            lengthInSamples = (lengthInNanoseconds * (int) sampleRate) / 10000000;
                        }
                    }
                }
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WMAudioReader)
};

}

//==============================================================================
WindowsMediaAudioFormat::WindowsMediaAudioFormat()
    : AudioFormat (TRANS (WindowsMediaCodec::wmFormatName),
                   StringArray (WindowsMediaCodec::extensions))
{
}

WindowsMediaAudioFormat::~WindowsMediaAudioFormat() {}

Array<int> WindowsMediaAudioFormat::getPossibleSampleRates()    { return Array<int>(); }
Array<int> WindowsMediaAudioFormat::getPossibleBitDepths()      { return Array<int>(); }

bool WindowsMediaAudioFormat::canDoStereo()     { return true; }
bool WindowsMediaAudioFormat::canDoMono()       { return true; }
bool WindowsMediaAudioFormat::isCompressed()    { return true; }

//==============================================================================
AudioFormatReader* WindowsMediaAudioFormat::createReaderFor (InputStream* sourceStream, bool deleteStreamIfOpeningFails)
{
    ScopedPointer<WindowsMediaCodec::WMAudioReader> r (new WindowsMediaCodec::WMAudioReader (sourceStream));

    if (r->sampleRate > 0)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = nullptr;

    return nullptr;
}

AudioFormatWriter* WindowsMediaAudioFormat::createWriterFor (OutputStream* /*streamToWriteTo*/, double /*sampleRateToUse*/,
                                                             unsigned int /*numberOfChannels*/, int /*bitsPerSample*/,
                                                             const StringPairArray& /*metadataValues*/, int /*qualityOptionIndex*/)
{
    jassertfalse; // not yet implemented!
    return nullptr;
}
