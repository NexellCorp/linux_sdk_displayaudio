#include "nxlauncher.h"
#include "ui_nxlauncher.h"
#include <CNX_StatusBar.h>

#include <QQuickView>
#include <QObject>
#include <QWidget>
#include <QQuickItem>
#include <QQmlProperty>
#include <QProcess>
#include <QDirIterator>
#include <QQmlContext>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQuickWidget>

#include <DAudioKeyDef.h>
#include <NX_PacpClient.h>
#include "NX_DAudioUtils.h"

#define NX_ENABLE_BGAPP         0

#ifdef Q_PROCESSOR_X86
#define NX_APP_PATH         "/home/doriya/working/solution/display-audio/bin"
#else
// #define NX_APP_PATH         "/podo/apps"
#define NX_APP_PATH         "/nexell/daudio"
#endif

NxLauncher::NxLauncher(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::NxLauncher)
{
	ui->setupUi(this);

	installEventFilter( this );

	//
	//  Status Bar
	//
	m_pStatusBar = new CNX_StatusBar( this );
	m_pStatusBar->move( 0, 0 );
	m_pStatusBar->resize( this->size().width(), this->size().height() * 1 / 10 );
	m_pStatusBar->SetTitleName( "Home" );

	//
	//  Launcher
	//
	m_pPackageManager = new NxPackageManager( NULL, NX_APP_PATH );
	m_pProcessManager = new NxProcessManager( this );

#ifdef CONFIG_USE_NO_QML
	NxAppInfoList list = m_pPackageManager->getAppInfoList();

	this->setStyleSheet("background:rgb(195,195,195);");

	m_pPrevPageButton = new QPushButton(this);
	connect(m_pPrevPageButton, SIGNAL(clicked(bool)), this, SLOT(onPrevPageButtonClicked()));
	m_pPrevPageButton->setFixedSize(50, 100);

	m_pNextPageButton = new QPushButton(this);
	connect(m_pNextPageButton, SIGNAL(clicked(bool)), this, SLOT(onNextPageButtonClicked()));
	m_pNextPageButton->setFixedSize(50, 100);

	m_pPageStackFrame = new PageStackFrame(this);
	connect(m_pPageStackFrame, SIGNAL(onButtonClicked(NxAppInfo*)), this, SLOT(onButtonClicked(NxAppInfo*)));
	connect(m_pPackageManager, SIGNAL(signalStateChanged(NxAppInfo*)), m_pPageStackFrame, SLOT(onButtonStateChnaged(NxAppInfo*)));
	m_pPageStackFrame->setStyleSheet("background: transparent;");
	m_pPageStackFrame->setCellSize(QSize(200, 200));
	m_pPageStackFrame->setSpacing(25);
//	m_pPageStackFrame->setStyleSheet("background:rgb(195,195,195);");
	m_pPageStackFrame->move(m_pPrevPageButton->width(), this->size().height() * 1 / 10);
	m_pPageStackFrame->resize( this->size().width()-m_pPrevPageButton->width()-m_pNextPageButton->width(), this->size().height() * 9 / 10 );
	for( int32_t i = 0; i < list.count(); i++ )
	{
		m_pPageStackFrame->pushItem(list.at(i));
	}
#else
	m_pChild = new QQuickWidget(this);
	m_pChild->setSource( QUrl("qrc:/qml/NxLauncher.qml") );
	m_pChild->rootContext()->setContextProperty("packageManager", m_pPackageManager);

	m_pChild->setResizeMode(QQuickWidget::SizeRootObjectToView);
	m_pChild->move(0, this->size().height() * 1 / 10);
	m_pChild->resize( this->size().width(), this->size().height() * 9 / 10 );

	m_pObject = (QObject*)m_pChild->rootObject();
	if( m_pObject )
	{
		connect( m_pObject, SIGNAL(launchProgram(QString)), m_pProcessManager, SLOT(execute(QString)) );

//        m_pObject->setProperty("imageSource", QUrl("qrc:/image/bg_back_hover.png"));
//        qDebug() << "imageSource: " + QQmlProperty::read(m_pObject, "imageSource").toString();
	}
#endif

#if NX_ENABLE_BGAPP
	//
	//  Run Background application
	//
	QString szProcess[] = {
		"/nexell/daudio/NxQuickRearCam/NxQuickRearCam",
	};

	for( uint32_t i = 0; i < (sizeof(szProcess) / sizeof(szProcess[0])) ; i++)
	{
		m_pProcessManager->execute( szProcess[i], true );
	}
#endif

	NX_PacpClientStart( this );
}

NxLauncher::~NxLauncher()
{
	removeEventFilter(this);

	delete ui;
}

bool NxLauncher::eventFilter(QObject *watched, QEvent *event)
{
	if( QEvent::Resize == event->type() )
	{
		m_pStatusBar->move( 0, 0 );
		m_pStatusBar->resize( this->size().width(), this->size().height() * 1 / 10 );

#ifdef CONFIG_USE_NO_QML
		m_pPrevPageButton->move(0, height()/2-m_pPrevPageButton->height()/2);
		m_pNextPageButton->move(width()-m_pNextPageButton->width(), height()/2-m_pNextPageButton->height()/2);

		m_pPageStackFrame->move(m_pPrevPageButton->width(), this->size().height() * 1 / 10);
		m_pPageStackFrame->resize( this->size().width()-m_pPrevPageButton->width()-m_pNextPageButton->width(), this->size().height() * 9 / 10 );
#else
		m_pChild->move(0, this->size().height() * 1 / 10);
		m_pChild->resize( this->size().width(), this->size().height() * 9 / 10 );

		QMetaObject::invokeMethod(m_pObject, "updateAppList" );
#endif
	}

	if( NX_QT_CUSTOM_EVENT_TYPE == event->type() )
	{
		static uint32_t bluetoothMode = 0;	//	0 : Phone, 1 : Audio, 2 : Setting
		NxEvent *pEvent = reinterpret_cast<NxEvent*>(event);
		// printf("pressed key emulator. ( %s )\n", GetKeyName(pEvent->m_iEventType) );

		switch ( pEvent->m_iEventType )
		{
		case DAUD_KEY_MODE_AUDIO:
			m_pProcessManager->execute("/nexell/daudio/NxAudioPlayer/NxAudioPlayer");
			break;
		case DAUD_KEY_MODE_VIDEO:
			m_pProcessManager->execute("/nexell/daudio/NxVideoPlayer/NxVideoPlayer");
			break;
		case DAUD_KEY_MODE_BLUETOOTH:
			switch (bluetoothMode)
			{
			case 0:	//	Phone
				m_pProcessManager->execute("/nexell/daudio/NxBTPhoneR/NxBTPhoneR");
				break;
			case 1:	//	Audio
				m_pProcessManager->execute("/nexell/daudio/NxBTAudioR/NxBTAudioR");
				break;
			case 2:	//	Settings
				m_pProcessManager->execute("/nexell/daudio/NxBTSettingsR/NxBTSettingsR");
				break;
			}
			bluetoothMode = (bluetoothMode+1) % 3;
			break;
		case DAUD_KEY_MODE_AVIN:
			m_pProcessManager->execute("/nexell/daudio/NxAVIn/NxAVIn");
			break;
		case DAUD_KEY_MODE_3DAVM:
			m_pProcessManager->hide("/nexell/daudio/Nx3DAvm/Nx3DAvm");
			m_pProcessManager->execute("/nexell/daudio/Nx3DAvm/nx_3d_avm");
			break;
		case DAUD_KEY_MODE_3DAVM_CLOSE:
			NX_KillProcess("/nexell/daudio/Nx3DAvm/nx_3d_avm");
			m_pProcessManager->show("/nexell/daudio/Nx3DAvm/Nx3DAvm");
			break;

		}

		if( NX_REQUEST_PROCESS_SHOW == pEvent->m_iEventType )
		{
			NX_PacpClientRequestRaise();
		}
	}

	if( event->type() == QEvent::ActivationChange )
	{
		if( isActiveWindow() )
		{
			printf(">>>>> NX_ReplyWait(). ( %d )\n", NX_ReplyDone() );
		}
		else
		{
			NX_RequestCommand( NX_REQUEST_FOCUS_VIDEO_LOSS );
		}
	}

	return QObject::eventFilter(watched, event);
}

void NxLauncher::raise()
{
	printf("NxLauncher: receive raise().\n");
	QMainWindow::raise();
}

void NxLauncher::lower()
{
	printf("NxLauncher: receive lower().\n");
	QMainWindow::lower();
}

void NxLauncher::showEvent(QShowEvent* event)
{
	Q_UNUSED( event );

	if( !isHidden() )
		return;

	int32_t iRet = NX_RequestCommand( NX_REQUEST_PROCESS_SHOW );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_SHOW, iRet );
	}

	QMainWindow::show();
}

void NxLauncher::hideEvent(QHideEvent* event)
{
	Q_UNUSED( event );

	if( isHidden() )
		return;

	int32_t iRet = NX_RequestCommand( NX_REQUEST_PROCESS_HIDE );
	if( NX_REPLY_DONE != iRet )
	{
		printf("Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_HIDE, iRet );
	}

	QMainWindow::hide();
}
#ifdef CONFIG_USE_NO_QML
void NxLauncher::onButtonClicked(NxAppInfo* pInfo)
{
	if (pInfo) {
		m_pProcessManager->execute( pInfo->getPath()+ "/" + pInfo->getExec() );
	}
}

void NxLauncher::onPrevPageButtonClicked()
{
	m_pPageStackFrame->setPage(m_pPageStackFrame->currentPage()-1);
}

void NxLauncher::onNextPageButtonClicked()
{
	m_pPageStackFrame->setPage(m_pPageStackFrame->currentPage()+1);
}
#endif
