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

namespace DragAndDropHelpers
{
    //==============================================================================
    class JuceDropSource   : public ComBaseClassHelper <IDropSource>
    {
    public:
        JuceDropSource() {}

        JUCE_COMRESULT QueryContinueDrag (BOOL escapePressed, DWORD keys)
        {
            if (escapePressed)
                return DRAGDROP_S_CANCEL;

            if ((keys & (MK_LBUTTON | MK_RBUTTON)) == 0)
                return DRAGDROP_S_DROP;

            return S_OK;
        }

        JUCE_COMRESULT GiveFeedback (DWORD)
        {
            return DRAGDROP_S_USEDEFAULTCURSORS;
        }
    };

    //==============================================================================
    class JuceEnumFormatEtc   : public ComBaseClassHelper <IEnumFORMATETC>
    {
    public:
        JuceEnumFormatEtc (const FORMATETC* const format_)
            : format (format_),
              index (0)
        {
        }

        JUCE_COMRESULT Clone (IEnumFORMATETC** result)
        {
            if (result == 0)
                return E_POINTER;

            JuceEnumFormatEtc* const newOne = new JuceEnumFormatEtc (format);
            newOne->index = index;

            *result = newOne;
            return S_OK;
        }

        JUCE_COMRESULT Next (ULONG celt, LPFORMATETC lpFormatEtc, ULONG* pceltFetched)
        {
            if (pceltFetched != nullptr)
                *pceltFetched = 0;
            else if (celt != 1)
                return S_FALSE;

            if (index == 0 && celt > 0 && lpFormatEtc != 0)
            {
                copyFormatEtc (lpFormatEtc [0], *format);
                ++index;

                if (pceltFetched != nullptr)
                    *pceltFetched = 1;

                return S_OK;
            }

            return S_FALSE;
        }

        JUCE_COMRESULT Skip (ULONG celt)
        {
            if (index + (int) celt >= 1)
                return S_FALSE;

            index += celt;
            return S_OK;
        }

        JUCE_COMRESULT Reset()
        {
            index = 0;
            return S_OK;
        }

    private:
        const FORMATETC* const format;
        int index;

        static void copyFormatEtc (FORMATETC& dest, const FORMATETC& source)
        {
            dest = source;

            if (source.ptd != 0)
            {
                dest.ptd = (DVTARGETDEVICE*) CoTaskMemAlloc (sizeof (DVTARGETDEVICE));
                *(dest.ptd) = *(source.ptd);
            }
        }

        JUCE_DECLARE_NON_COPYABLE (JuceEnumFormatEtc)
    };

    //==============================================================================
    class JuceDataObject  : public ComBaseClassHelper <IDataObject>
    {
    public:
        JuceDataObject (JuceDropSource* const dropSource_,
                        const FORMATETC* const format_,
                        const STGMEDIUM* const medium_)
            : dropSource (dropSource_),
              format (format_),
              medium (medium_)
        {
        }

        ~JuceDataObject()
        {
            jassert (refCount == 0);
        }

        JUCE_COMRESULT GetData (FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
        {
            if ((pFormatEtc->tymed & format->tymed) != 0
                 && pFormatEtc->cfFormat == format->cfFormat
                 && pFormatEtc->dwAspect == format->dwAspect)
            {
                pMedium->tymed = format->tymed;
                pMedium->pUnkForRelease = 0;

                if (format->tymed == TYMED_HGLOBAL)
                {
                    const SIZE_T len = GlobalSize (medium->hGlobal);
                    void* const src = GlobalLock (medium->hGlobal);
                    void* const dst = GlobalAlloc (GMEM_FIXED, len);

                    memcpy (dst, src, len);

                    GlobalUnlock (medium->hGlobal);

                    pMedium->hGlobal = dst;
                    return S_OK;
                }
            }

            return DV_E_FORMATETC;
        }

        JUCE_COMRESULT QueryGetData (FORMATETC* f)
        {
            if (f == 0)
                return E_INVALIDARG;

            if (f->tymed == format->tymed
                  && f->cfFormat == format->cfFormat
                  && f->dwAspect == format->dwAspect)
                return S_OK;

            return DV_E_FORMATETC;
        }

        JUCE_COMRESULT GetCanonicalFormatEtc (FORMATETC*, FORMATETC* pFormatEtcOut)
        {
            pFormatEtcOut->ptd = 0;
            return E_NOTIMPL;
        }

        JUCE_COMRESULT EnumFormatEtc (DWORD direction, IEnumFORMATETC** result)
        {
            if (result == 0)
                return E_POINTER;

            if (direction == DATADIR_GET)
            {
                *result = new JuceEnumFormatEtc (format);
                return S_OK;
            }

            *result = 0;
            return E_NOTIMPL;
        }

        JUCE_COMRESULT GetDataHere (FORMATETC*, STGMEDIUM*)                  { return DATA_E_FORMATETC; }
        JUCE_COMRESULT SetData (FORMATETC*, STGMEDIUM*, BOOL)                { return E_NOTIMPL; }
        JUCE_COMRESULT DAdvise (FORMATETC*, DWORD, IAdviseSink*, DWORD*)     { return OLE_E_ADVISENOTSUPPORTED; }
        JUCE_COMRESULT DUnadvise (DWORD)                                     { return E_NOTIMPL; }
        JUCE_COMRESULT EnumDAdvise (IEnumSTATDATA**)                         { return OLE_E_ADVISENOTSUPPORTED; }

    private:
        JuceDropSource* const dropSource;
        const FORMATETC* const format;
        const STGMEDIUM* const medium;

        JUCE_DECLARE_NON_COPYABLE (JuceDataObject)
    };

    //==============================================================================
    HDROP createHDrop (const StringArray& fileNames)
    {
        int totalBytes = 0;
        for (int i = fileNames.size(); --i >= 0;)
            totalBytes += (int) CharPointer_UTF16::getBytesRequiredFor (fileNames[i].getCharPointer()) + sizeof (WCHAR);

        HDROP hDrop = (HDROP) GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof (DROPFILES) + totalBytes + 4);

        if (hDrop != 0)
        {
            LPDROPFILES pDropFiles = (LPDROPFILES) GlobalLock (hDrop);
            pDropFiles->pFiles = sizeof (DROPFILES);
            pDropFiles->fWide = true;

            WCHAR* fname = reinterpret_cast<WCHAR*> (addBytesToPointer (pDropFiles, sizeof (DROPFILES)));

            for (int i = 0; i < fileNames.size(); ++i)
            {
                const size_t bytesWritten = fileNames[i].copyToUTF16 (fname, 2048);
                fname = reinterpret_cast<WCHAR*> (addBytesToPointer (fname, bytesWritten));
            }

            *fname = 0;

            GlobalUnlock (hDrop);
        }

        return hDrop;
    }

    bool performDragDrop (FORMATETC* const format, STGMEDIUM* const medium, const DWORD whatToDo)
    {
        JuceDropSource* const source = new JuceDropSource();
        JuceDataObject* const data = new JuceDataObject (source, format, medium);

        DWORD effect;
        const HRESULT res = DoDragDrop (data, source, whatToDo, &effect);

        data->Release();
        source->Release();

        return res == DRAGDROP_S_DROP;
    }
}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMove)
{
    FORMATETC format = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium = { TYMED_HGLOBAL, { 0 }, 0 };

    medium.hGlobal = DragAndDropHelpers::createHDrop (files);

    return DragAndDropHelpers::performDragDrop (&format, &medium, canMove ? (DWORD) (DROPEFFECT_COPY | DROPEFFECT_MOVE)
                                                                          : (DWORD) DROPEFFECT_COPY);
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text)
{
    FORMATETC format = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium = { TYMED_HGLOBAL, { 0 }, 0 };

    const size_t numBytes = CharPointer_UTF16::getBytesRequiredFor (text.getCharPointer());

    medium.hGlobal = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, numBytes + 2);
    WCHAR* const data = static_cast <WCHAR*> (GlobalLock (medium.hGlobal));

    text.copyToUTF16 (data, numBytes);
    format.cfFormat = CF_UNICODETEXT;

    GlobalUnlock (medium.hGlobal);

    return DragAndDropHelpers::performDragDrop (&format, &medium, DROPEFFECT_COPY | DROPEFFECT_MOVE);
}
