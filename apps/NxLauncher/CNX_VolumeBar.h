#ifndef CNX_VOLUMEBAR_H
#define CNX_VOLUMEBAR_H

#include <QFrame>
#include <QMouseEvent>
#include <QTimer>

namespace Ui {
class CNX_VolumeBar;
}

class CNX_VolumeBar : public QFrame
{
	Q_OBJECT

signals:
	void signalSetVolume(int value);

private slots:
	void slotTimer();

public:
	explicit CNX_VolumeBar(QWidget *parent = 0);
	~CNX_VolumeBar();

	void raise();

	void SetValue(int value);

private slots:
	void on_slider_sliderReleased();

	void on_slider_sliderMoved(int position);

	void on_slider_sliderPressed();

private:
	void ResetCountDown();

private:
	QTimer m_Timer;
	int m_iCountDown;

private:
	Ui::CNX_VolumeBar *ui;
};

#endif // CNX_VOLUMEBAR_H
