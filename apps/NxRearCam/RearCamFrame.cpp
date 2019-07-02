#include "RearCamFrame.h"
#include "ui_RearCamFrame.h"
#include <QDesktopWidget>
#include <QTextCodec>
#include <QPainter>
#include <QGraphicsEllipseItem>


#include <NX_DAudioUtils.h>

#define LOG_TAG "[RearCam|Frame]"
#include <NX_Log.h>

#define DEFAULT_WIDTH	1024
#define DEFAULT_HEIGHT	600
//------------------------------------------
#define NX_CUSTOM_BASE QEvent::User

#define PGL_RECT_MARGINE	10
#define PGL_H_LINE_NUM	3
#define PGL_V_LINE_NUM  6

static	RearCamFrame *pFrame = NULL;

RearCamFrame::RearCamFrame(QWidget *parent)
	: QFrame(parent)
	, m_bIsInitialized(false)
	, m_bShowCamera(false)
	, m_bDrawPGL(false)
	, m_pRequestTerminate(NULL)
	, m_pRequestLauncherShow(NULL)
	, ui(new Ui::RearCamFrame)
{
	//UI Setting
	ui->setupUi(this);

	scene = new QGraphicsScene(parent);

	const QRect screen = QApplication::desktop()->screenGeometry();
	move(0, screen.height());

	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height());
	}

	pFrame = this;
}

RearCamFrame::~RearCamFrame()
{
	delete ui;
}

void RearCamFrame::paintEvent(QPaintEvent* event)
{
	if(m_bDrawPGL && pgl_enable)
	{
		int h_start_x, h_start_y, h_end_x, h_end_y;
		int v_start_x, v_start_y, v_end_x, v_end_y;

		for(int32_t i=0; i<PGL_H_LINE_NUM; i++)
		{
			h_start_x = (pgl_dsp_info.m_iDspWidth/2) - (dsp_info.iDspWidth /PGL_H_LINE_NUM)/2 - 90*i;
			h_start_y = dsp_info.iDspHeight / 2 + 90*i ;
			h_end_x = h_start_x + (dsp_info.iDspWidth /PGL_H_LINE_NUM) + (90*2)*i ;
			h_end_y = h_start_y;

			if(i == 0)
			{
				scene->addLine(h_start_x, h_start_y, h_end_x, h_end_y, QPen(Qt::green, 10, Qt::SolidLine));

				v_start_x = h_start_x - 5;
				v_start_y = h_start_y + 5;
				v_end_x = v_start_x - 90;
				v_end_y = v_start_y + 90;
				scene->addLine(v_start_x, v_start_y, v_end_x, v_end_y, QPen(Qt::green, 10, Qt::SolidLine));

				v_start_x = h_end_x + 5;
				v_start_y = h_start_y + 5;
				v_end_x = h_end_x + 90;
				v_end_y = h_end_y + 90;
				scene->addLine(v_start_x, v_start_y, v_end_x, v_end_y, QPen(Qt::green, 10, Qt::SolidLine));

			}
			else if(i == 1)
			{
				scene->addLine(h_start_x, h_start_y, h_end_x, h_end_y, QPen(Qt::yellow, 10, Qt::SolidLine));

				v_start_x = h_start_x - 5;
				v_start_y = h_start_y + 5;
				v_end_x = v_start_x - 90;
				v_end_y = v_start_y + 90;
				scene->addLine(v_start_x, v_start_y, v_end_x, v_end_y, QPen(Qt::yellow, 10, Qt::SolidLine));

				v_start_x = h_end_x + 5;
				v_start_y = h_start_y + 5;
				v_end_x = h_end_x + 90;
				v_end_y = h_end_y + 90;
				scene->addLine(v_start_x, v_start_y, v_end_x, v_end_y, QPen(Qt::yellow, 10, Qt::SolidLine));
			}
			else
			{
				scene->addLine(h_start_x, h_start_y, h_end_x, h_end_y, QPen(Qt::red, 10, Qt::SolidLine));

				v_start_x = h_start_x - 5;
				v_start_y = h_start_y + 5;
				v_end_x = v_start_x - 90;
				v_end_y = v_start_y + 90;
				scene->addLine(v_start_x, v_start_y, v_end_x, v_end_y, QPen(Qt::red, 10, Qt::SolidLine));

				v_start_x = h_end_x + 5;
				v_start_y = h_start_y + 5;
				v_end_x = h_end_x + 90;
				v_end_y = h_end_y + 90;
				scene->addLine(v_start_x, v_start_y, v_end_x, v_end_y, QPen(Qt::red, 10, Qt::SolidLine));
			}
		}

	}
}

void RearCamFrame::SetupUI()
{
	float widthRatio = (float)width() / DEFAULT_WIDTH;
	float heightRatio = (float)height() / DEFAULT_HEIGHT;
	int rx, ry, rw, rh;

	rx = widthRatio * ui->graphicsView->x();
	ry = heightRatio * ui->graphicsView->y();
	rw = widthRatio * ui->graphicsView->width();
	rh = heightRatio * ui->graphicsView->height();
	ui->graphicsView->setGeometry(rx, ry, rw, rh);

	scene->setSceneRect(rx-PGL_RECT_MARGINE, ry-PGL_RECT_MARGINE, rw-PGL_RECT_MARGINE*2, rh-PGL_RECT_MARGINE*2);
	ui->graphicsView->setScene(scene);
}

void RearCamFrame::TerminateEvent(NxTerminateEvent *)
{
	if (m_pRequestTerminate)
	{
		if (m_pRequestOpacity)
		{
			m_pRequestOpacity(true);
		}
		m_pRequestTerminate();
	}
}

void RearCamFrame::RegisterRequestTerminate(void (*cbFunc)(void))
{
	if (cbFunc)
	{
		m_pRequestTerminate = cbFunc;
	}
}

void RearCamFrame::RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk))
{
	if (cbFunc)
	{
		m_pRequestLauncherShow = cbFunc;
	}
}

void RearCamFrame::RegisterRequestOpacity(void (*cbFunc)(bool))
{
	if (cbFunc)
	{
		m_pRequestOpacity = cbFunc;
	}
}

bool RearCamFrame::ShowCamera()
{
	if( m_bShowCamera )
		return false;

	NXLOGI("[%s] ShowCamera", __FUNCTION__);

	int32_t m_bExitLoop = false;

	//-------------RearCam init---------------------------
	if( 0 > NX_RearCamInit( &vip_info, &dsp_info, &deinter_info ) )
	{
		NXLOGE("Fail, NX_RearCamInit().\n");
		return false;
	}

	//-------------RaerCam Start-------------------------
	NX_RearCamStart();

	update();

	m_bDrawPGL = true;

	m_bShowCamera = true;
	return true;
}

void RearCamFrame::HideCamera()
{
	if( !m_bShowCamera )
		return ;

	NX_RearCamDeInit();

	m_bDrawPGL = false;
	m_bShowCamera = false;

}

bool RearCamFrame::IsShowCamera()
{
	return m_bShowCamera;
}