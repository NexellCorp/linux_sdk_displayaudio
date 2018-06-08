#ifndef TABBUTTONS_H
#define TABBUTTONS_H

#include <QWidget>
#include <stdint.h>
#include "CSettings.h"

namespace Ui {
class TabButtons;
}

class TabButtons : public QWidget
{
	Q_OBJECT

public:
	explicit TabButtons(QWidget *parent = 0);
	~TabButtons();

	bool SendKey( int32_t key, int32_t value );

private slots:
	// Power
	void on_btnPower_pressed();
	void on_btnPower_released();

	// Mode
	void on_btnMode_released();
	void on_btnVideo_released();
	void on_btnAudio_released();
	void on_btnRadio_released();
	void on_btnPhone_released();
	void on_btnAVIn_released();
	void on_btnBluetooth_released();
	void on_btnSettings_released();
	void on_btnMode_pressed();
	void on_btnVideo_pressed();
	void on_btnAudio_pressed();
	void on_btnRadio_pressed();
	void on_btnAVIn_pressed();
	void on_btnBluetooth_pressed();
	void on_btnPhone_pressed();
	void on_btnSettings_pressed();

	// Volume Up/Down
	void on_btnVolUp_released();
	void on_btnVolDown_released();
	void on_btnVolMute_released();
	void on_btnVolUp_pressed();
	void on_btnVolDown_pressed();
	void on_btnVolMute_pressed();

	// Navigation Up/Down/Left/Right
	void on_btnUp_released();
	void on_btnDown_released();
	void on_btnLeft_released();
	void on_btnRight_released();

	void on_btnUp_pressed();
	void on_btnDown_pressed();
	void on_btnLeft_pressed();
	void on_btnRight_pressed();

private:
	Ui::TabButtons *ui;
};

#endif // TABBUTTONS_H
