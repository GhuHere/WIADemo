#include "DeviceModel.h"
#include <QFile>
#include <QFileDialog>
#include <QThread>
#include <QDateTime>

DeviceModel::DeviceModel(IWiaItem* pWiaItem, QString deviceName, QString deviceDesc, QObject *parent)
	: QObject(parent), 
    m_cRef(1),
    m_pBuffer(NULL),
    m_nBufferLength(0),
    m_nBytesTransfered(0),
    m_guidFormat(IID_NULL)
{
    m_pWiaItemRoot = pWiaItem;
    m_deviceName = deviceName;
    m_deviceDesc = deviceDesc;
    
    /*QThread* thread = new QThread();
    moveToThread(thread);
    connect(this, &DeviceModel::destroyed, thread, &QThread::deleteLater);
    thread->start();*/
}

DeviceModel::~DeviceModel()
{
    m_pWiaItemRoot->Release();
    if (m_pBuffer)
    {
        LocalFree(m_pBuffer);
        m_pBuffer = NULL;
    }
    m_nBufferLength = 0;
    m_nBytesTransfered = 0;
}

QString DeviceModel::GetDeviceName()
{
    return m_deviceName;
}

QString DeviceModel::GetDeviceDesc()
{
    return m_deviceDesc;
}

void DeviceModel::ScanningSlot(int source, int pictrueType, int brightness, int contrast, int dpi, int size)
{
    //CoInitialize(NULL);
    vector<IWiaItem*> imageWiaItems;
    m_pWiaItemRoot->AnalyzeItem(0);
    HRESULT hr = EnumerateItems(m_pWiaItemRoot, &imageWiaItems);
    if (hr == S_OK)
    {
        for (int i = 0; i < imageWiaItems.size(); ++i)
        {
            IWiaPropertyStorage* pGetWiaProperty = nullptr;
            hr = imageWiaItems[i]->QueryInterface(IID_IWiaPropertyStorage, (void**)&pGetWiaProperty);
            if (hr != S_OK) continue;
            PROPSPEC propSpec[9];
            PROPVARIANT propVar[9];
            propSpec[0].ulKind = PRSPEC_PROPID;
            propSpec[0].propid = WIA_IPA_TYMED;
            propSpec[1].ulKind = PRSPEC_PROPID;
            propSpec[1].propid = WIA_IPA_FORMAT;
            propSpec[2].ulKind = PRSPEC_PROPID;
            propSpec[2].propid = WIA_DPS_PREVIEW;
            //propSpec[3].ulKind = PRSPEC_PROPID;
            //propSpec[3].propid = WIA_DPS_DOCUMENT_HANDLING_SELECT;
            propSpec[3].ulKind = PRSPEC_PROPID;
            propSpec[3].propid = WIA_IPS_CUR_INTENT;
            propSpec[4].ulKind = PRSPEC_PROPID;
            propSpec[4].propid = WIA_IPS_BRIGHTNESS;
            propSpec[5].ulKind = PRSPEC_PROPID;
            propSpec[5].propid = WIA_IPS_CONTRAST;
            propSpec[6].ulKind = PRSPEC_PROPID;
            propSpec[6].propid = WIA_IPS_XRES;
            propSpec[7].ulKind = PRSPEC_PROPID;
            propSpec[7].propid = WIA_IPS_YRES;
            propSpec[8].ulKind = PRSPEC_PROPID;
            propSpec[8].propid = WIA_IPS_PAGE_SIZE;

            CLSID guidOutputFormat1 = WiaImgFmt_MEMORYBMP;
            propVar[0].vt = VT_I4;
            propVar[0].lVal = TYMED_CALLBACK;
            propVar[1].vt = VT_CLSID;
            propVar[1].puuid = &guidOutputFormat1;
            propVar[2].vt = VT_I4;
            propVar[2].lVal = 0;
            //propVar[3].vt = VT_I4;
            //if (source == 0) propVar[3].lVal = FLATBED;
            //else propVar[3].lVal = FEEDER;
            propVar[3].vt = VT_I4;
            if (pictrueType == 0) propVar[3].lVal = WIA_INTENT_IMAGE_TYPE_COLOR;
            else if (pictrueType == 1) propVar[3].lVal = WIA_INTENT_IMAGE_TYPE_GRAYSCALE;
            else propVar[3].lVal = WIA_INTENT_IMAGE_TYPE_TEXT;
            propVar[4].vt = VT_I4;
            propVar[4].lVal = brightness;
            propVar[5].vt = VT_I4;
            propVar[5].lVal = contrast;
            propVar[6].vt = VT_I4;
            propVar[6].lVal = dpi;
            propVar[7].vt = VT_I4;
            propVar[7].lVal = dpi;
            propVar[8].vt = VT_I4;
            if (size == 0) propVar[8].lVal = WIA_PAGE_A4;
            else if (size == 1) propVar[8].lVal = WIA_PAGE_ISO_A5;
            else propVar[8].lVal = WIA_PAGE_ISO_A6;

            hr = pGetWiaProperty->WriteMultiple(sizeof(propSpec) / sizeof(propSpec[0]), propSpec, propVar, WIA_IPA_FIRST);
            if (hr != S_OK) continue;

            IWiaDataTransfer* pIWiaDataTransfer = nullptr;
            hr = imageWiaItems[i]->QueryInterface(IID_IWiaDataTransfer, (void**)&pIWiaDataTransfer);
            if (hr != S_OK) continue;

			PWIA_DATA_TRANSFER_INFO pdti = new WIA_DATA_TRANSFER_INFO;
			pdti->ulSize = sizeof(WIA_DATA_TRANSFER_INFO);
			pdti->ulBufferSize = 1024 * 1024;
			pdti->ulSection = 0;
			pdti->bDoubleBuffer = true;
			pdti->ulReserved1 = 0;
			pdti->ulReserved2 = 0;
			pdti->ulReserved3 = 0;

            while (true)
            {
                hr = pIWiaDataTransfer->idtGetBandedData(pdti, this);
                if (hr == S_OK) WriteFile();
                else break;
            }
			
            if (pGetWiaProperty) pGetWiaProperty->Release();
            if (pIWiaDataTransfer) pIWiaDataTransfer->Release();
            imageWiaItems[i]->Release();
        }
    }
}

void DeviceModel::WriteFile()
{
    QFile file(QDateTime::currentDateTime().toString() + ".bmp");
    if (file.open(QIODevice::WriteOnly))
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

        file.write(bufferHeader, 14);
        file.write((char*)m_pBuffer, m_nBytesTransfered);
        file.close();
    }
}

HRESULT DeviceModel::EnumerateItems(IWiaItem* pWiaItem, vector<IWiaItem*>* pItems)
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

HRESULT CALLBACK DeviceModel::QueryInterface(REFIID riid, void** ppvObject)
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
        *ppvObject = (IWiaDataCallback*)(this);
    }
    else if (IsEqualIID(riid, IID_IWiaDataCallback))
    {
        *ppvObject = (IWiaDataCallback*)(this);
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

ULONG CALLBACK DeviceModel::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CALLBACK DeviceModel::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}

    // The IWiaDataTransfer::idtGetBandedData method periodically 
    // calls the IWiaDataCallback::BandedDataCallback method with
    // status messages. It sends the callback method a data header
    // message followed by one or more data messages to transfer 
    // data. It concludes by sending a termination message.

HRESULT _stdcall DeviceModel::BandedDataCallback(
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