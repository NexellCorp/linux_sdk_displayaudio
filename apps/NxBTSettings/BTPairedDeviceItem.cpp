#include "BTPairedDeviceItem.h"
#include "ui_BTPairedDeviceItem.h"

BTPairedDeviceItem::BTPairedDeviceItem(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::BTPairedDeviceItem)
{
	ui->setupUi(this);

    m_nCurrentRow = -1;
}

BTPairedDeviceItem::~BTPairedDeviceItem()
{

}

void BTPairedDeviceItem::setDeviceName(QString name)
{
	ui->LABEL_DEVICE_NAME->setText(name);
}

QString BTPairedDeviceItem::deviceName()
{
	return ui->LABEL_DEVICE_NAME->text();
}

void BTPairedDeviceItem::setDeviceAddress(QString address)
{
	ui->LABEL_DEVICE_ADDRESS->setText(address);
}

QString BTPairedDeviceItem::deviceAddress()
{
	return ui->LABEL_DEVICE_ADDRESS->text();
}

void BTPairedDeviceItem::setConnectionStatusForAVK(QString status)
{
	ui->BUTTON_AVK_CONNECTION_STATUS->setText(status);
}

QString BTPairedDeviceItem::connectionStatusForAVK()
{
	return ui->BUTTON_AVK_CONNECTION_STATUS->text();
}

void BTPairedDeviceItem::setConnectionStatusForHS(QString status)
{
	ui->BUTTON_HS_CONNECTION_STATUS->setText(status);
}

QString BTPairedDeviceItem::connectionStatusForHS()
{
	return ui->BUTTON_HS_CONNECTION_STATUS->text();
}

void BTPairedDeviceItem::setRow(int row)
{
    m_nCurrentRow = row;
}

int BTPairedDeviceItem::row()
{
    return m_nCurrentRow;
}

void BTPairedDeviceItem::on_BUTTON_AVK_CONNECTION_STATUS_clicked()
{
	emit signalAVKConnectButtonClicked(m_nCurrentRow, this);
}

void BTPairedDeviceItem::on_BUTTON_HS_CONNECTION_STATUS_clicked()
{
	emit signalHSConnectButtonClicked(m_nCurrentRow, this);
}
