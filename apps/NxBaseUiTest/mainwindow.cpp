#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTimer>

#define NX_ENABLE_CHECK_STATUS		0

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    installEventFilter( this );

    resize( 1024, 600 );
	m_pStatusBar = new CNX_StatusBar( this );
    m_pStatusBar->resize( this->size().width(), this->size().height() * 1 / 10 );

#if NX_ENABLE_CHECK_STATUS
     connect(&m_Timer, SIGNAL(timeout()), this, SLOT(CheckStatus()));
     m_Timer.start( 1000 );
#endif

    // NxMediaController *pMediaController = new NxMediaController( this );
    // m_pStatusBar->show();
}

MainWindow::~MainWindow()
{
    removeEventFilter( this );
    delete ui;
}


void MainWindow::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED( event );
    m_pStatusBar->resize( this->size().width(), this->size().height() * 1 / 10 );
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	switch( event->type() )
    {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
            qDebug() << ">> MainWindow:" << event;
            break;
        default:
            break;
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::CheckStatus()
{
    qDebug(">> Parents( %s ), Child( %s )",
        isHidden() ? "Hidden" : "Shown",
        m_pStatusBar->isHidden() ? "Hidden" : "Shown" );
}

void MainWindow::on_BUTTON_STATUSBAR_ENABLED_toggled(bool checked)
{
	m_pStatusBar->setEnabled(checked);
}

void MainWindow::on_BUTTON_SET_BUTTON_ALL_ENABLED_toggled(bool checked)
{
	m_pStatusBar->SetButtonEnabled(CNX_StatusBar::ButtonType_All, checked);
}

void MainWindow::on_BUTTON_SET_BUTTON_HOME_ENABLED_toggled(bool checked)
{
	m_pStatusBar->SetButtonEnabled(CNX_StatusBar::ButtonType_Home, checked);
}

void MainWindow::on_BUTTON_SET_BUTTON_BACK_ENABLED_toggled(bool checked)
{
	m_pStatusBar->SetButtonEnabled(CNX_StatusBar::ButtonType_Back, checked);
}
