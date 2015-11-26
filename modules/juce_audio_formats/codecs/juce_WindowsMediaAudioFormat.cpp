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

#if JUCE_MINGW

} // close juce namespace

#include <guiddef.h>
#include <tuple>

using	tupluuid = std::tuple<uint32_t, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t>;

inline
tupluuid	UUID_split(const char *s)
{	
	GUID	g = juce::uuidFromString(s);
	
	return std::make_tuple(g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
	// __CRT_UUID_DECL(name, g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
}

/*
struct bite
{
	bite(tupluuid tup)
		: m_tupid(tup)
	{
	}
	
	auto&	operator[](const size_t idx)
	{	return std::get<idx>(m_tupid);
	}
	
	tupluuid	m_tupid;
};
*/

/*
#define	gh(s)	gx = UUID_split(s);
#define	DeclUUID(typ, s)	__CRT_UUID_DECL(typ,	get<0>(gx), get<1>(gx), get<2>(gx), get<3>(gx), get<4>(gx), \
							get<5>(gx), get<6>(gx), get<7>(gx), get<8>(gx), get<9>(gx), get<10>(gx));
*/

typedef 
enum WMT_RIGHTS
{
	WMT_RIGHT_PLAYBACK			= 0x1,
	WMT_RIGHT_COPY_TO_NON_SDMI_DEVICE	= 0x2,
	WMT_RIGHT_COPY_TO_CD			= 0x8,
	WMT_RIGHT_COPY_TO_SDMI_DEVICE		= 0x10,
	WMT_RIGHT_ONE_TIME			= 0x20,
	WMT_RIGHT_SAVE_STREAM_PROTECTED		= 0x40,
	WMT_RIGHT_COPY				= 0x80,
	WMT_RIGHT_COLLABORATIVE_PLAY		= 0x100,
	WMT_RIGHT_SDMI_TRIGGER			= 0x10000,
	WMT_RIGHT_SDMI_NOMORECOPIES		= 0x20000
} WMT_RIGHTS;
    
#include "C:/Program Files (x86)/Windows Kits/8.1/Include/um/nserror.h"

typedef LPCWSTR LPCWSTR_WMSDK_TYPE_SAFE;

    // MIDL_INTERFACE("96406BDA-2B2B-11d3-B36B-00C04F6108FF")
    struct IWMHeaderInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAttributeCount( 
            /* [in] */ WORD wStreamNum,
            /* [out] */ WORD *pcAttributes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributeByIndex( 
            /* [in] */ WORD wIndex,
            /* [out][in] */ WORD *pwStreamNum,
            /* [size_is][out] */ WCHAR *pwszName,
            /* [out][in] */ WORD *pcchNameLen,
            /* [out] */ WMT_ATTR_DATATYPE *pType,
            /* [size_is][out] */ BYTE *pValue,
            /* [out][in] */ WORD *pcbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributeByName( 
            /* [out][in] */ WORD *pwStreamNum,
            /* [in] */ LPCWSTR pszName,
            /* [out] */ WMT_ATTR_DATATYPE *pType,
            /* [size_is][out] */ BYTE *pValue,
            /* [out][in] */ WORD *pcbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAttribute( 
            /* [in] */ WORD wStreamNum,
            /* [in] */ LPCWSTR pszName,
            /* [in] */ WMT_ATTR_DATATYPE Type,
            /* [size_is][in] */ const BYTE *pValue,
            /* [in] */ WORD cbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMarkerCount( 
            /* [out] */ WORD *pcMarkers) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMarker( 
            /* [in] */ WORD wIndex,
            /* [size_is][out] */ WCHAR *pwszMarkerName,
            /* [out][in] */ WORD *pcchMarkerNameLen,
            /* [out] */ QWORD *pcnsMarkerTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddMarker( 
            /* [in] */ LPCWSTR_WMSDK_TYPE_SAFE pwszMarkerName,
            /* [in] */ QWORD cnsMarkerTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveMarker( 
            /* [in] */ WORD wIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScriptCount( 
            /* [out] */ WORD *pcScripts) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScript( 
            /* [in] */ WORD wIndex,
            /* [size_is][out] */ WCHAR *pwszType,
            /* [out][in] */ WORD *pcchTypeLen,
            /* [size_is][out] */ WCHAR *pwszCommand,
            /* [out][in] */ WORD *pcchCommandLen,
            /* [out] */ QWORD *pcnsScriptTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddScript( 
            /* [in] */ LPCWSTR_WMSDK_TYPE_SAFE pwszType,
            /* [in] */ LPCWSTR_WMSDK_TYPE_SAFE pwszCommand,
            /* [in] */ QWORD cnsScriptTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveScript( 
            /* [in] */ WORD wIndex) = 0;
    };
    __CRT_UUID_DECL(IWMHeaderInfo, 0x96406BDA, 0x2B2B,0x11d3, 0xB3, 0x6B, 0x00, 0xC0, 0x4F, 0x61, 0x08, 0xFF);
    // DeclUUID(IWMHeaderInfo, "96406BDA-2B2B-11d3-B36B-00C04F6108FF");

    // MIDL_INTERFACE("96406BDC-2B2B-11d3-B36B-00C04F6108FF")
    struct IWMStreamConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStreamType( 
            /* [out] */ GUID *pguidStreamType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStreamNumber( 
            /* [out] */ WORD *pwStreamNum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStreamNumber( 
            /* [in] */ WORD wStreamNum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStreamName( 
            /* [size_is][out] */ WCHAR *pwszStreamName,
            /* [out][in] */ WORD *pcchStreamName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStreamName( 
            /* [in] */ LPCWSTR_WMSDK_TYPE_SAFE pwszStreamName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectionName( 
            /* [size_is][out] */ WCHAR *pwszInputName,
            /* [out][in] */ WORD *pcchInputName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetConnectionName( 
            /* [in] */ LPCWSTR_WMSDK_TYPE_SAFE pwszInputName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBitrate( 
            /* [out] */ DWORD *pdwBitrate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBitrate( 
            /* [in] */ DWORD pdwBitrate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBufferWindow( 
            /* [out] */ DWORD *pmsBufferWindow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBufferWindow( 
            /* [in] */ DWORD msBufferWindow) = 0;
    };
    
    // MIDL_INTERFACE("96406BDD-2B2B-11d3-B36B-00C04F6108FF")
    struct IWMStreamList : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStreams( 
            /* [size_is][out] */ WORD *pwStreamNumArray,
            /* [out][in] */ WORD *pcStreams) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddStream( 
            /* [in] */ WORD wStreamNum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveStream( 
            /* [in] */ WORD wStreamNum) = 0;
        
    };

    // MIDL_INTERFACE("96406BDE-2B2B-11d3-B36B-00C04F6108FF")
    struct IWMMutualExclusion : public IWMStreamList
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetType( 
            /* [out] */ GUID *pguidType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetType( 
            /* [in] */ REFGUID guidType) = 0;
        
    };

    // MIDL_INTERFACE("96406BDB-2B2B-11d3-B36B-00C04F6108FF")
    struct IWMProfile : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetVersion( 
            /* [out] */ DWORD *pdwVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
            /* [size_is][out] */ WCHAR *pwszName,
            /* [out][in] */ DWORD *pcchName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetName( 
            /* [in] */ const WCHAR *pwszName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
            /* [size_is][out] */ WCHAR *pwszDescription,
            /* [out][in] */ DWORD *pcchDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDescription( 
            /* [in] */ const WCHAR *pwszDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStreamCount( 
            /* [out] */ DWORD *pcStreams) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStream( 
            /* [in] */ DWORD dwStreamIndex,
            /* [out] */ IWMStreamConfig **ppConfig) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStreamByNumber( 
            /* [in] */ WORD wStreamNum,
            /* [out] */ IWMStreamConfig **ppConfig) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveStream( 
            /* [in] */ IWMStreamConfig *pConfig) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveStreamByNumber( 
            /* [in] */ WORD wStreamNum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddStream( 
            /* [in] */ IWMStreamConfig *pConfig) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReconfigStream( 
            /* [in] */ IWMStreamConfig *pConfig) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateNewStream( 
            /* [in] */ REFGUID guidStreamType,
            /* [out] */ IWMStreamConfig **ppConfig) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMutualExclusionCount( 
            /* [out] */ DWORD *pcME) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMutualExclusion( 
            /* [in] */ DWORD dwMEIndex,
            /* [out] */ IWMMutualExclusion **ppME) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveMutualExclusion( 
            /* [in] */ IWMMutualExclusion *pME) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddMutualExclusion( 
            /* [in] */ IWMMutualExclusion *pME) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateNewMutualExclusion( 
            /* [out] */ IWMMutualExclusion **ppME) = 0;
    };
    // DeclUUID(IWMProfile, "96406BDB-2B2B-11d3-B36B-00C04F6108FF");
    __CRT_UUID_DECL(IWMProfile, 0x96406BDB, 0x2B2B, 0x11d3, 0xB3, 0x6B, 0x00, 0xC0, 0x4F, 0x61, 0x08, 0xFF);
    
    // MIDL_INTERFACE("d16679f2-6ca0-472d-8d31-2f5d55aee155")
    struct IWMProfileManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateEmptyProfile( 
            /* [in] */ DWORD dwVersion,
            /* [out] */ IWMProfile **ppProfile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadProfileByID( 
            /* [in] */ REFGUID guidProfile,
            /* [out] */ IWMProfile **ppProfile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadProfileByData( 
            /* [in] */ const WCHAR *pwszProfile,
            /* [out] */ IWMProfile **ppProfile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveProfile( 
            /* [in] */ IWMProfile *pIWMProfile,
            /* [in] */ WCHAR *pwszProfile,
            /* [out][in] */ DWORD *pdwLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSystemProfileCount( 
            /* [out] */ DWORD *pcProfiles) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadSystemProfile( 
            /* [in] */ DWORD dwProfileIndex,
            /* [out] */ IWMProfile **ppProfile) = 0;
        
    };

namespace juce
{

#endif

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
