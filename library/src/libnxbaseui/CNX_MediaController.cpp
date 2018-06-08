#include "CNX_MediaController.h"

#include <QQuickView>
#include <QEvent>

CNX_MediaController::CNX_MediaController(QWidget *parent)
    : m_pParent( parent )
{
    m_pParent->installEventFilter( this );

    QQuickView *pView = new QQuickView();
    pView->setSource( QUrl("qrc:/qml/NxMediaController.qml") );
    pView->setResizeMode( QQuickView::SizeRootObjectToView );

    m_pChild = QWidget::createWindowContainer( pView, m_pParent );
    m_pChild->move(0, m_pParent->size().height() * 9 / 10);
    m_pChild->resize( m_pParent->size().width(), m_pParent->size().height() * 1 / 10 );

    m_pObject = (QObject*)pView->rootObject();
}

CNX_MediaController::~CNX_MediaController()
{
    m_pParent->removeEventFilter(this);
}

bool CNX_MediaController::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize)
    {
        m_pChild->move(0, m_pParent->size().height() * 9 / 10);
        m_pChild->resize( m_pParent->size().width(), m_pParent->size().height() * 1 / 10 );
    }

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* pKey = static_cast<QKeyEvent*>(event);
        qDebug("pressed key. ( %d )", pKey->key() );
    }

    return QObject::eventFilter(watched, event);
}
