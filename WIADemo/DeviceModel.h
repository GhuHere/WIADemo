#pragma once

#include <QObject>
#include <WiaVideo.h>
#include <Wia.h>
#include <windows.h>
#include <fstream>
#include <vector>
#include <QMessageBox>

using namespace std;

class DeviceModel : public QObject
{
	Q_OBJECT

public:
	DeviceModel(IWiaItem* pWiaItem, QString deviceName, QString deviceDesc, QObject *parent = nullptr);
	~DeviceModel();
	QString GetDeviceName();
	QString GetDeviceDesc();

private:
	IWiaItem* m_pWiaItemRoot;

	QString m_deviceName;
	QString m_deviceDesc;
};
