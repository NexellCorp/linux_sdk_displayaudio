#ifndef BTPAIREDDEVICEITEM_H
#define BTPAIREDDEVICEITEM_H

#include <QWidget>

namespace Ui {
class BTPairedDeviceItem;
}

class BTPairedDeviceItem : public QWidget
{
    Q_OBJECT

signals:
	void signalHSConnectButtonClicked(int row, BTPairedDeviceItem* item);

	void signalAVKConnectButtonClicked(int row, BTPairedDeviceItem* item);

public:
	explicit BTPairedDeviceItem(QWidget *parent = 0);
    ~BTPairedDeviceItem();

	// setter for device name
    void setDeviceName(QString name);
	// getter for device name
	QString deviceName();
	// setter for device address
	void setDeviceAddress(QString address);
	// getter for device address
	QString deviceAddress();
	// setter for AVK service connection status
	void setConnectionStatusForAVK(QString status);
	// getter for AVK service connection status
	QString connectionStatusForAVK();
	// setter for AVK service connection status
	void setConnectionStatusForHS(QString status);
	// getter for AVK service connection status
	QString connectionStatusForHS();

    void setRow(int row);

    int row();

private slots:
	void on_BUTTON_AVK_CONNECTION_STATUS_clicked();

	void on_BUTTON_HS_CONNECTION_STATUS_clicked();

private:
    int m_nCurrentRow;

private:
	Ui::BTPairedDeviceItem *ui;
};

#endif // BTPAIREDDEVICEITEM_H
