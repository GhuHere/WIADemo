#pragma once

#include <QWidget>
#include <QSharedPointer>
#include "ui_ControlDlg.h"
#include "DeviceModel.h"

class ControlDlg : public QDialog
{
	Q_OBJECT

public:
	ControlDlg(QSharedPointer<DeviceModel> dev, QWidget *parent = Q_NULLPTR);
	~ControlDlg();

private:
	Ui::ControlDlg ui;
	QSharedPointer<DeviceModel> m_dev;

signals:
	void Scanning(int source, int pictrueType, int brightness, int contrast, int dpi, int size);

public slots:
	void StartButtonClickedSlot();
};
