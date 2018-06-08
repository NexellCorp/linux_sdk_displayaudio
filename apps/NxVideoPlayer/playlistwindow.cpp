#include "playlistwindow.h"
#include "ui_playlistwindow.h"

#include <QScrollBar>

static void cbStatusHome( void *pObj )
{
	(void) pObj;
	qDebug(">>>>>>>>>>>>>>>> cbStatusHome\n");
}

static void cbStatusBack( void *pObj )
{
	PlayListWindow *pW = (PlayListWindow *)pObj;
	qDebug(">>>>>>>>>>>>>>>> cbStatusBack\n");
	pW->close();
}


PlayListWindow::PlayListWindow(QWidget *parent)
	: QDialog(parent)
	, m_pretIdx(-1)
	, m_selectCount(0)
	, ui(new Ui::PlayListWindow)
{
	ui->setupUi(this);

	memset(&m_selectIdx,0,sizeof(QModelIndex) );

	ui->listWidget->setStyleSheet(
		"QListWidget{"
			"padding-left : 20px;"
		"}"
		"QListWidget::item {"
			"padding : 20px;"
		"}"
	);

	ui->listWidget->verticalScrollBar()->setStyleSheet(
		"QScrollBar:vertical {"
			"width: 50px;"
			"min-height: 100px;"
		"}"
	);

	m_pStatusBar = new CNX_StatusBar( this );
	m_pStatusBar->move( 0, 0 );
	m_pStatusBar->resize( this->size().width(), this->size().height() * 1 / 10 );
	m_pStatusBar->RegOnClickedHome( cbStatusHome );
	m_pStatusBar->RegOnClickedBack( cbStatusBack );
	m_pStatusBar->SetTitleName( "Nexell Video Player" );

	setWindowFlags(Qt::Window|Qt::FramelessWindowHint);
	setAttribute(Qt::WA_DeleteOnClose);
}

PlayListWindow::~PlayListWindow()
{
	delete m_pStatusBar;
	delete ui;
}

//
//	List Index Control
//
void PlayListWindow::setList(CNX_FileList *pFileList)
{
	ui->listWidget->clear();
	for( int i=0 ; i < pFileList->GetSize() ; i++ )
	{
		ui->listWidget->addItem(  strrchr( (pFileList->GetList(i)).toStdString().c_str(), '/' ) +1 );
	}
}

void PlayListWindow::setCurrentIndex(int idx)
{
	ui->listWidget->setCurrentRow(idx);
}

int PlayListWindow::getCurrentIndex()
{
	return m_selectIdx.row();
}

//
//	Button Event Control
//


void PlayListWindow::on_btnCancel_released()
{
	this->reject();
}

void PlayListWindow::on_btnOk_released()
{
	m_selectIdx = ui->listWidget->currentIndex();
	this->accept();
}

void PlayListWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
	m_selectCount++;
	m_selectIdx = item->listWidget()->currentIndex();
	if(2 == m_selectCount)
	{
		if(m_pretIdx == m_selectIdx.row())
		{
			this->accept();
		}
	}
	else
	{
		m_selectCount = 1;
	}
	m_pretIdx = m_selectIdx.row();
}
