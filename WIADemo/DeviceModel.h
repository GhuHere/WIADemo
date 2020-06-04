#pragma once

#include <QObject>
#include <WiaVideo.h>
#include <Wia.h>
#include <windows.h>
#include <fstream>
#include <vector>
#include <QMessageBox>

using namespace std;

class DeviceModel : public QObject, IWiaDataCallback
{
	Q_OBJECT

public:
	DeviceModel(IWiaItem* pWiaItem, QString deviceName, QString deviceDesc, QObject *parent = nullptr);
	~DeviceModel();
	QString GetDeviceName();
	QString GetDeviceDesc();
	HRESULT EnumerateItems(IWiaItem* pWiaItem, vector<IWiaItem*>* pItems);
	void WriteFile();
	// Í¨¹ý IWiaDataCallback ¼Ì³Ð
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
	virtual ULONG __stdcall AddRef(void) override;
	virtual ULONG __stdcall Release(void) override;
	virtual HRESULT __stdcall BandedDataCallback(LONG lMessage, LONG lStatus, LONG lPercentComplete, LONG lOffset, LONG lLength, LONG lReserved, LONG lResLength, BYTE* pbBuffer) override;

private:
	IWiaItem* m_pWiaItemRoot;
	LONG  m_cRef;               // Object reference count 
	PBYTE m_pBuffer;            // Data buffer
	LONG  m_nBufferLength;      // Length of buffer
	LONG  m_nBytesTransfered;   // Total number of bytes transferred
	GUID  m_guidFormat;         // Data format

	QString m_deviceName;
	QString m_deviceDesc;

public slots:
	void ScanningSlot(int source, int pictrueType, int brightness, int contrast, int dpi, int size);
};
