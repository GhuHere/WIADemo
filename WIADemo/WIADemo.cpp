#include "WIADemo.h"
#include "ControlDlg.h"

WIADemo::WIADemo(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    HRESULT hResult;
    IEnumWIA_DEV_INFO* pIEnum = nullptr;
    IWiaPropertyStorage* pWiaPropertyStorage = nullptr;
    
    CoInitialize(NULL);
   
	// Create WIA Device Manager instance
	m_pWiaDevMgr = nullptr;
	hResult = CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void**)&m_pWiaDevMgr);

	if (hResult == S_OK)
	{
		hResult = m_pWiaDevMgr->EnumDeviceInfo(WIA_DEVINFO_ENUM_ALL, &pIEnum);
		if (hResult == S_FALSE)
		{
			QMessageBox::critical(this, QString("Error"), QString("Enum Device failed!"));
		}
		else if (hResult == WIA_S_NO_DEVICE_AVAILABLE)
		{
			QMessageBox::information(this, QString("None"), QString("No device available."));

		}
		else if (hResult == S_OK)
		{
			while (true)
			{
				hResult = pIEnum->Next(1, &pWiaPropertyStorage, nullptr);
				if (hResult != S_OK) break;
				PROPSPEC propSpec[3] = { 0 };
				PROPVARIANT propVar[3] = { 0 };
				// How many properties are you querying for?
				const ULONG c_nPropertyCount = sizeof(propSpec) / sizeof(propSpec[0]);
				// Define which properties you want to read:
				// Device ID.  This is what you would use to create the device.
				propSpec[0].ulKind = PRSPEC_PROPID;
				propSpec[0].propid = WIA_DIP_DEV_ID;
				// Device Name
				propSpec[1].ulKind = PRSPEC_PROPID;
				propSpec[1].propid = WIA_DIP_DEV_NAME;
				// Device description
				propSpec[2].ulKind = PRSPEC_PROPID;
				propSpec[2].propid = WIA_DIP_DEV_DESC;

				// Ask for the property values
				hResult = pWiaPropertyStorage->ReadMultiple(c_nPropertyCount, propSpec, propVar);
				if (hResult == S_OK)
				{
					IWiaItem* pWiaDeviceItem = nullptr;
					hResult = m_pWiaDevMgr->CreateDevice(propVar[0].bstrVal, &pWiaDeviceItem);
					if (hResult == S_OK)
					{
						QString name = QString::fromWCharArray(propVar[1].bstrVal);
						QString desc = QString::fromWCharArray(propVar[2].bstrVal);
						QSharedPointer<DeviceModel> pDeviceModel(new DeviceModel(pWiaDeviceItem, name, desc));
						m_devices.push_back(pDeviceModel);
					}
				}
			}
		}
	}
	else
	{
		QMessageBox::critical(this, QString("Error"), QString("CoCreateInstance WIA failed!"));
	}
	
	// Release COM interface.
	if (pWiaPropertyStorage) pWiaPropertyStorage->Release();
	if (pIEnum) pIEnum->Release();

	//create wpd device manager
	hResult = CoCreateInstance(CLSID_PortableDeviceManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pPortableDeviceManager));
	if (SUCCEEDED(hResult))
	{
		DWORD cPnPDeviceIDs = 0;
		hResult = m_pPortableDeviceManager->GetDevices(nullptr, &cPnPDeviceIDs);
		if (SUCCEEDED(hResult) && cPnPDeviceIDs > 0)
		{
			LPWSTR *pPnpDeviceIDs = new (std::nothrow) LPWSTR[cPnPDeviceIDs];
			if (pPnpDeviceIDs != nullptr)
			{
				hResult = m_pPortableDeviceManager->GetDevices(pPnpDeviceIDs, &cPnPDeviceIDs);
				if (SUCCEEDED(hResult))
				{
					for (DWORD i = 0; i < cPnPDeviceIDs; ++i)
					{
						IPortableDevice* pDevice = nullptr;
						IPortableDeviceValues* pDeviceValues = nullptr;
						hResult = CoCreateInstance(CLSID_PortableDeviceFTM, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevice));
						HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues, NULL, CLSCTX_INPROC_SERVER, IID_IPortableDeviceValues, (VOID**)&pDeviceValues);
						if (FAILED(hResult) || FAILED(hr)) continue;
						hResult = pDevice->Open(pPnpDeviceIDs[i], pDeviceValues);
						if(FAILED(hResult)) continue;
						DWORD cchDeviceFriendlyName = 0;
						hResult = m_pPortableDeviceManager->GetDeviceFriendlyName(pPnpDeviceIDs[i], nullptr, &cchDeviceFriendlyName);
						if (FAILED(hResult)) continue;
						WCHAR* pDeviceFriendlyName = new WCHAR[cchDeviceFriendlyName + 1];
						hResult = m_pPortableDeviceManager->GetDeviceFriendlyName(pPnpDeviceIDs[i], pDeviceFriendlyName, &cchDeviceFriendlyName);
						pDeviceFriendlyName[cchDeviceFriendlyName] = '\0';
						
					}
				}
			}
		}
	}
	else
	{
		QMessageBox::critical(this, QString("Error"), QString("CoCreateInstance WPD failed!"));
	}
    
    for (int i = 0; i < m_devices.size(); ++i)
    {
		QSharedPointer<DeviceModel> dev = m_devices.at(i);
		
		ui.listWidget->addItem(dev->GetDeviceName());
    }

	connect(ui.listWidget, &QListWidget::itemDoubleClicked, this, &WIADemo::ItemDoubleClickedSlot);
}

WIADemo::~WIADemo()
{
	m_devices.clear();
	if (m_pWiaDevMgr) m_pWiaDevMgr->Release();
	//CoUninitialize();
}

void WIADemo::resizeEvent(QResizeEvent* ev)
{
	QSize size = this->size();
	ui.listWidget->setGeometry(0, 0, size.width(), size.height());
}

void WIADemo::ItemDoubleClickedSlot(QListWidgetItem* item)
{
	int index = ui.listWidget->row(item);
	QSharedPointer<DeviceModel> dev = m_devices.at(index);
	ControlDlg* dlg = new ControlDlg(dev, this);
	dlg->show();
}
