#ifndef TABTEST_H
#define TABTEST_H

#include <QWidget>
#include <pthread.h>

namespace Ui {
class TabTest;
}

class TabTest : public QWidget
{
	Q_OBJECT

public:
	explicit TabTest(QWidget *parent = 0);
	~TabTest();

private slots:
	void on_btnAudioFocusTest_clicked();
    void on_btnAVMStart_clicked();
    void on_btnAVMStop_clicked();

private:
	Ui::TabTest *ui;

	// Audio Focus Test Thread
private:
	bool m_bStartedAudFocusTest;
	bool m_bExitAudioFocusTestLoop;
	pthread_t m_hThreadAudFocus;
	static void *ThreadStubAudFocus(void *arg)
	{
		TabTest *pObj = (TabTest *)arg;
		pObj->ThreadProcAudFocus();
		return (void*)0xDeadDead;
	}
	void ThreadProcAudFocus();

private:
    bool m_bTestDualDisplay;
};

#endif // TABTEST_H
