#include "DeviceModel.h"

HRESULT EnumerateItems(IWiaItem* pWiaItem, vector<IWiaItem*>* pItems)
{
    //
    // Validate arguments
    //
    if (NULL == pWiaItem)
    {
        return E_INVALIDARG;
    }

    //
    // Get the item type for this item.
    //
    LONG lItemType = 0;
    HRESULT hr = pWiaItem->GetItemType(&lItemType);
    if (SUCCEEDED(hr))
    {
        //
        // If it is a folder, or it has attachments, enumerate its children.
        //
        if (lItemType & WiaItemTypeFolder || lItemType & WiaItemTypeHasAttachments)
        {
            //
            // Get the child item enumerator for this item.
            //
            IEnumWiaItem* pEnumWiaItem = NULL; //vista and later

            hr = pWiaItem->EnumChildItems(&pEnumWiaItem);
            if (SUCCEEDED(hr))
            {
                //
                // Loop until you get an error or pEnumWiaItem->Next returns
                // S_FALSE to signal the end of the list.
                //
                while (S_OK == hr)
                {
                    //
                    // Get the next child item.
                    //
                    IWiaItem* pChildWiaItem = NULL; //vista and laster


                    hr = pEnumWiaItem->Next(1, &pChildWiaItem, NULL);

                    //
                    // pEnumWiaItem->Next will return S_FALSE when the list is
                    // exhausted, so check for S_OK before using the returned
                    // value.
                    //
                    if (S_OK == hr)
                    {
                        //
                        // Recurse into this item.
                        //
                        hr = EnumerateItems(pChildWiaItem, pItems);

                        //
                        // Release this item.
                        //
                        //pChildWiaItem->Release();
                        //pChildWiaItem = NULL;
                    }
                }

                //
                // If the result of the enumeration is S_FALSE (which
                // is normal), change it to S_OK.
                //
                if (S_FALSE == hr)
                {
                    hr = S_OK;
                }

                //
                // Release the enumerator.
                //
                //pEnumWiaItem->Release();
                //pEnumWiaItem = NULL;
            }
        }
        else pItems->push_back(pWiaItem);
    }
    return  hr;
}

//
// The application must instantiate the CDataCallback object using
// the "new" operator, and call QueryInterface to retrieve the 
// IWiaDataCallback interface.
//
// In this example, using in-memory transfer, the application then
// calls the IWiaDataTransfer::idtGetBandedData method and passes
// it the IWiaDataCallback interface pointer.
//
// If the application performs a file transfer using
// IWiaDataTransfer::idtGetData, only status messages are sent,
// and the data is transferred in a file.
//
class CDataCallback : public IWiaDataCallback
{
private:
    LONG  m_cRef;               // Object reference count 
    PBYTE m_pBuffer;            // Data buffer
    LONG  m_nBufferLength;      // Length of buffer
    LONG  m_nBytesTransfered;   // Total number of bytes transferred
    GUID  m_guidFormat;         // Data format

public:

    //
    // Constructor and destructor
    //
    CDataCallback()
        : m_cRef(1),
        m_pBuffer(NULL),
        m_nBufferLength(0),
        m_nBytesTransfered(0),
        m_guidFormat(IID_NULL)
    {
    }
    ~CDataCallback()
    {
        //
        // Free the item buffer
        //
        if (m_pBuffer)
        {
            LocalFree(m_pBuffer);
            m_pBuffer = NULL;
        }
        m_nBufferLength = 0;
        m_nBytesTransfered = 0;
    }

    //
    // IUnknown methods
    //
    HRESULT CALLBACK QueryInterface(REFIID riid, void** ppvObject)
    {
        //
        // Validate arguments
        //
        if (NULL == ppvObject)
        {
            return E_INVALIDARG;
        }

        //
        // Return the appropriate interface
        //
        if (IsEqualIID(riid, IID_IUnknown))
        {
            *ppvObject = static_cast<CDataCallback*>(this);
        }
        else if (IsEqualIID(riid, IID_IWiaDataCallback))
        {
            *ppvObject = static_cast<CDataCallback*>(this);
        }
        else
        {
            *ppvObject = NULL;
            return(E_NOINTERFACE);
        }

        //
        // Increment the reference count before returning the interface.
        //
        reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
        return S_OK;
    }
    ULONG CALLBACK AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }
    ULONG CALLBACK Release()
    {
        LONG cRef = InterlockedDecrement(&m_cRef);
        if (0 == cRef)
        {
            delete this;
        }
        return cRef;
    }

    //
    // The IWiaDataTransfer::idtGetBandedData method periodically 
    // calls the IWiaDataCallback::BandedDataCallback method with
    // status messages. It sends the callback method a data header
    // message followed by one or more data messages to transfer 
    // data. It concludes by sending a termination message.
    //

    HRESULT _stdcall BandedDataCallback(
        LONG lMessage,
        LONG lStatus,
        LONG lPercentComplete,
        LONG lOffset,
        LONG lLength,
        LONG lReserved,
        LONG lResLength,
        BYTE* pbData)
    {
        UNREFERENCED_PARAMETER(lReserved);
        UNREFERENCED_PARAMETER(lResLength);
        switch (lMessage)
        {
        case IT_MSG_DATA_HEADER:
        {
            //
            // The data header contains the image's final size.
            //
            PWIA_DATA_CALLBACK_HEADER pHeader = reinterpret_cast<PWIA_DATA_CALLBACK_HEADER>(pbData);
            if (pHeader && pHeader->lBufferSize)
            {
                //
                // Allocate a block of memory to hold the image
                //
                m_pBuffer = reinterpret_cast<PBYTE>(LocalAlloc(LPTR, pHeader->lBufferSize));
                if (m_pBuffer)
                {
                    //
                    // Save the buffer size.
                    //
                    m_nBufferLength = pHeader->lBufferSize;

                    //
                    // Initialize the bytes transferred count.
                    //
                    m_nBytesTransfered = 0;

                    //
                    // Save the file format.
                    //
                    m_guidFormat = pHeader->guidFormatID;
                }
            }
        }
        break;

        case IT_MSG_DATA:
        {
            //
            // Make sure a block of memory has been created.
            //
            if (NULL != m_pBuffer)
            {
                //
                // Copy the new band.
                //
                CopyMemory(m_pBuffer + lOffset, pbData, lLength);

                //
                // Increment the byte count.
                //
                m_nBytesTransfered += lLength;
            }
        }
        break;

        case IT_MSG_STATUS:
        {
            //
            // Display transfer phase
            //
            if (lStatus & IT_STATUS_TRANSFER_FROM_DEVICE)
            {
                //_tprintf(TEXT("Transfer from device\n"));
            }
            else if (lStatus & IT_STATUS_PROCESSING_DATA)
            {
                //_tprintf(TEXT("Processing Data\n"));
            }
            else if (lStatus & IT_STATUS_TRANSFER_TO_CLIENT)
            {
                //_tprintf(TEXT("Transfer to Client\n"));
            }

            //
            // Display percent complete
            //
            //_tprintf(TEXT("lPercentComplete: %d\n"), lPercentComplete);

        }
        break;
        }

        return S_OK;
    }
    void WriteFile()
    {

        ofstream out;
        out.open("out.bmp", ios::binary);
        if (out.is_open())
        {
            char bufferHeader[14] = { 0 };//BMP file header.
            union {
                DWORD size;
                char by[4];
            }siby;
            siby.size = m_nBufferLength;
            bufferHeader[0] = 0x42;
            bufferHeader[1] = 0x4d;
            bufferHeader[2] = siby.by[0];
            bufferHeader[3] = siby.by[1];
            bufferHeader[4] = siby.by[2];
            bufferHeader[5] = siby.by[3];
            bufferHeader[10] = 0x36;

            out.write(bufferHeader, 14);
            out.write((char*)m_pBuffer, m_nBytesTransfered);

            out.close();
        }

    }
};

DeviceModel::DeviceModel(IWiaItem* pWiaItem, QString deviceName, QString deviceDesc, QObject *parent)
	: QObject(parent)
{
    m_pWiaItemRoot = pWiaItem;
    m_deviceName = deviceName;
    m_deviceDesc = deviceDesc;
}

DeviceModel::~DeviceModel()
{
    m_pWiaItemRoot->Release();
}

QString DeviceModel::GetDeviceName()
{
    return m_deviceName;
}

QString DeviceModel::GetDeviceDesc()
{
    return m_deviceDesc;
}