#ifndef NXLAUNCHER_H
#define NXLAUNCHER_H

#include <QMainWindow>
#include <QProcess>
#include <QEvent>
#include <QQuickWidget>
#include <QWidget>
#include <QListWidgetItem>
#include <QPushButton>

#include <CNX_StatusBar.h>

#include <NX_Type.h>
#include <INX_IpcManager.h>
#include <NX_IpcPacket.h>
#include <NX_IpcUtils.h>

#include "nxpackagemanager.h"
#include "nxprocessmanager.h"

#include <page/PageStackFrame.h>

namespace Ui {
class NxLauncher;
}

class NxLauncher : public QMainWindow
{
    Q_OBJECT
#ifdef CONFIG_USE_NO_QML
private slots:
	void onButtonClicked(NxAppInfo* pInfo);

	void onPrevPageButtonClicked();

	void onNextPageButtonClicked();
#endif
public:
    explicit NxLauncher(QWidget *parent = 0);
    ~NxLauncher();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);

public slots:
    void raise();
    void lower();

#if defined(QT_X11) && defined(CONFIG_USING_LISTWIDGET)
    void onListWidgetItemClicked( QListWidgetItem *pItem );
#endif

private:
    Ui::NxLauncher *ui;

    CNX_StatusBar* m_pStatusBar;
    NxPackageManager* m_pPackageManager;
    NxProcessManager* m_pProcessManager;

#ifdef CONFIG_USE_NO_QML
	PageStackFrame* m_pPageStackFrame;
	QPushButton *m_pPrevPageButton;
	QPushButton *m_pNextPageButton;
#else
    QQuickWidget* m_pChild;
    QObject* m_pObject;
#endif
};

class NxEvent : QEvent
{
public:
    int32_t m_iEventType;

    NxEvent(QEvent::Type type) : QEvent(type)
    {
    }
};

#endif // NXLAUNCHER_H
