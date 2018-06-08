#include <stdio.h>

#include "avinmainwindow.h"
#include "avindisplayview.h"

AVInDisplayView::AVInDisplayView( QWidget *parent )
	: QGraphicsView(parent)
	, m_pParent ( NULL )
{
}


void AVInDisplayView::mouseReleaseEvent(QMouseEvent *event)
{
	Q_UNUSED( event );
	if( m_pParent )
		((AVInMainWindow*)m_pParent)->ToggleStatusBar();
}

void AVInDisplayView::SetParentWindow( QWidget *parent )
{
	m_pParent = parent;
}
