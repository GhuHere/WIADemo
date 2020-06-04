#include "ControlDlg.h"
#include <QTextCodec>

QTextCodec* g_pCodec = QTextCodec::codecForName("GB18030");

ControlDlg::ControlDlg(QSharedPointer<DeviceModel> dev, QWidget *parent)
	: QDialog(parent)
{
	m_dev = dev;

	ui.setupUi(this);
	ui.sourceCombo->addItem(g_pCodec->toUnicode("平板"));
	ui.sourceCombo->addItem(g_pCodec->toUnicode("文档送纸机"));
	//ui.sizeCombo->addItem(g_pCodec->toUnicode("A4加大 210 x 330(毫米)"));
	ui.sizeCombo->addItem(g_pCodec->toUnicode("A4 210 x 297(毫米)"));
	//ui.sizeCombo->addItem(g_pCodec->toUnicode("A5特大 174 x 235(毫米)"));
	//ui.sizeCombo->addItem(g_pCodec->toUnicode("A5旋转 210 x 148(毫米)"));
	ui.sizeCombo->addItem(g_pCodec->toUnicode("A5 148 x 210(毫米)"));
	//ui.sizeCombo->addItem(g_pCodec->toUnicode("A6旋转 148 x 105(毫米)"));
	ui.sizeCombo->addItem(g_pCodec->toUnicode("A6 105 x 148(毫米)"));
	ui.dpiCombo->addItem(g_pCodec->toUnicode("100"));
	ui.dpiCombo->addItem(g_pCodec->toUnicode("150"));
	ui.dpiCombo->addItem(g_pCodec->toUnicode("200"));
	ui.dpiCombo->addItem(g_pCodec->toUnicode("300"));
	ui.dpiCombo->addItem(g_pCodec->toUnicode("400"));
	ui.dpiCombo->addItem(g_pCodec->toUnicode("600"));
	ui.dpiCombo->addItem(g_pCodec->toUnicode("1200"));
	ui.brightnessSlider->setRange(-50, 50);
	ui.brightnessSlider->setValue(0);
	ui.contrastSlider->setRange(-50, 50);
	ui.contrastSlider->setValue(0);
	ui.colorRabut->setChecked(true);

	connect(ui.startBut, &QPushButton::clicked, this, &ControlDlg::StartButtonClickedSlot);
	connect(ui.CancelBut, &QPushButton::clicked, this, &ControlDlg::deleteLater);
	connect(this, &ControlDlg::Scanning, m_dev.get(), &DeviceModel::ScanningSlot);
}

ControlDlg::~ControlDlg()
{
}

void ControlDlg::StartButtonClickedSlot()
{
	int source = ui.sourceCombo->currentIndex();
	int pictrueType = 0;
	if (ui.grayRabut->isChecked()) pictrueType = 1;
	else if (ui.txtRabut->isChecked()) pictrueType = 2;
	int brightness = ui.brightnessSlider->value();
	int contrast = ui.contrastSlider->value();
	int dpi;
	switch (ui.dpiCombo->currentIndex())
	{
	case 0: dpi = 100; break;
	case 1: dpi = 150; break;
	case 2: dpi = 200; break;
	case 3: dpi = 300; break;
	case 4: dpi = 400; break;
	case 5: dpi = 600; break;
	case 6: dpi = 1200; break;
	default: dpi = 200; break;
	}
	int size = ui.sizeCombo->currentIndex();
	//emit Scanning(source, pictrueType, brightness, contrast, dpi, size);
	m_dev->ScanningSlot(source, pictrueType, brightness, contrast, dpi, size);
}