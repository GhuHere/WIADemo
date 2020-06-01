#pragma once

#include <QtWidgets/QMainWindow>
#include <WiaVideo.h>
#include <Wia.h>
#include <windows.h>
#include <QMessageBox>
#include <QSharedPointer>
#include <QVector>
#include <QResizeEvent>
#include "ui_WIADemo.h"
#include "DeviceModel.h"

#pragma comment (lib, "WiaGuid.lib")

using namespace std;

class WIADemo : public QMainWindow
{
    Q_OBJECT

public:
    WIADemo(QWidget *parent = Q_NULLPTR);
    void resizeEvent(QResizeEvent* ev);

private:
    Ui::WIADemoClass ui;
    IWiaDevMgr* m_pWiaDevMgr;
    QVector<QSharedPointer<DeviceModel>> m_devices;

public slots:
    void Uninitialize();
};
