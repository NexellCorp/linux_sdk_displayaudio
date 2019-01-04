#ifndef TABSETTING_H
#define TABSETTING_H

#include <QWidget>
#include "CSettings.h"

namespace Ui {
class TabSetting;
}

class TabSetting : public QWidget
{
	Q_OBJECT

public:
	explicit TabSetting(QWidget *parent = 0);
	~TabSetting();

private slots:
	void on_comboControlMethods_currentIndexChanged(const QString &arg1);

	void on_lineEdtAdbConCmd_textChanged(const QString &arg1);

	void on_lineEdtIPAddress_textChanged(const QString &arg1);

	void on_lineEditPortNumber_textChanged(const QString &arg1);

	void on_lineEdtUartPort_textChanged(const QString &arg1);

private:
	// Update UI
	void UpdateUIMode( uint32_t uiMode );
	void UpdateUIADB( bool enable );
	void UpdateUINetwork( bool enable );
	void UpdateUIUart( bool enable );

	// Update Setting Value
	void UpdateSetting();

	enum { UI_MOD_ADB, UI_MOD_NETWORK, UI_MOD_UART };
	uint32_t m_CurUIMode;
	Ui::TabSetting *ui;
};

#endif // TABSETTING_H
