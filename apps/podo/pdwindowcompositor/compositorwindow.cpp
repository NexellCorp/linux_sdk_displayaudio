/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Wayland module
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>

#include "compositorwindow.h"
#include "global.h"
#include <QMouseEvent>
#include <QOpenGLWindow>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QProcess>
#include <QDir>
#include "windowcompositor.h"
#include <QtWaylandCompositor/qwaylandinput.h>

#define LAUNCHER_NAME	"NxLauncher"
#define LAUNCHER_PATH	"/usr/bin"

#define DBG_CONSOLE		0

CompositorWindow::CompositorWindow()
    : m_backgroundTexture(0)
    , m_compositor(0)
    , m_grabState(NoGrab)
    , m_dragIconView(0)
{
}

void CompositorWindow::setCompositor(WindowCompositor *comp) {
    m_compositor = comp;
    connect(m_compositor, &WindowCompositor::startMove, this, &CompositorWindow::startMove);
    connect(m_compositor, &WindowCompositor::startResize, this, &CompositorWindow::startResize);
    connect(m_compositor, &WindowCompositor::dragStarted, this, &CompositorWindow::startDrag);
}

static bool NX_RunProcess(const char *pExec, int32_t bWayland = true)
{
    if (NULL == pExec)
        return false;

	struct sigaction sigchld_action;
	sigchld_action.sa_handler = SIG_DFL;
	sigchld_action.sa_flags = SA_NOCLDWAIT;
	sigaction(SIGCHLD, &sigchld_action, NULL);

    int32_t pid = fork();
    if (0 > pid)
        return false;

    if(pid == 0) {
        char dev[256];
		sprintf(dev, "/dev/%s", "console");
        int32_t fd = ::open(dev, O_RDWR);
        if (0 > fd)
            return false;

		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);

		if (fd > 0)
			::close(fd);

        if( bWayland == true )
        {
            if (0 > execl(pExec, pExec, "-platform", "wayland", NULL))
            {
    			printf("Fail, execl(). (%s)\n", pExec);
                return false;
            }
        }
        else
        {
            if (0 > execl(pExec, pExec, NULL))
            {
                printf("Fail, execl(). (%s)\n", pExec);
                return false;
            }
        }
    }

    return true;
}

bool CompositorWindow::execApplication()
{
    QString appsPath = qgetenv("PODO_APPS_PATH");

    if (appsPath.isEmpty())
        appsPath = PD_DEFAULT_APPS_PATH;

    QString startApp = qgetenv("PODO_START_APP");

    if (startApp.isEmpty())
    {
		startApp = LAUNCHER_NAME;
//		if (startApp.compare("NxLauncher", Qt::CaseSensitive) == 0) {
#if 0
			QString mgrApp = "NxService.sh";
			QString mgrPath = "/usr/bin";

			QDir dir(mgrPath);
			if (!dir.exists(mgrPath)) {
				qWarning() << "CompositorWindow::execApplication not found start service path:" << mgrPath;
				return false;
			}

			QString mgrExecPath = mgrPath + "/" + mgrApp;
			qDebug() << "CompositorWindow::execApplication run services:" << mgrExecPath;
			QProcess::startDetached(mgrExecPath);
#else
            QString mgrApp = "NxService.sh";
            QString mgrPath = "/usr/bin";

            QDir dir(mgrPath);
            if (!dir.exists(mgrPath)) {
                qWarning() << "CompositorWindow::execApplication not found start service path:" << mgrPath;
                return false;
            }

            QString mgrExecPath = mgrPath + "/" + mgrApp;
            qDebug() << "CompositorWindow::execApplication run services:" << mgrExecPath;
#if DBG_CONSOLE
            NX_RunProcess(QString(mgrExecPath).toLocal8Bit().toStdString().c_str(), false);
#else
            QProcess::startDetached(mgrExecPath);
#endif
#endif
//		}
    }
	QString startAppPath = LAUNCHER_PATH;

    QDir dir(startAppPath);
    if (!dir.exists(startAppPath))
    {
        qWarning() << "CompositorWindow::execApplication not found start launcher app path:" << startAppPath;
        return false;
    }

    QString launcherExecPath = startAppPath + "/" + startApp;
    qDebug() << "CompositorWindow::execApplication run launcher processor:" << launcherExecPath;

#if DBG_CONSOLE
	return NX_RunProcess(QString(launcherExecPath).toLocal8Bit().toStdString().c_str());
#else
	return QProcess::startDetached(launcherExecPath);
#endif
}

void CompositorWindow::initializeGL()
{
    QImage backgroundImage = QImage(QLatin1String(":/background.png"));
    m_backgroundTexture = new QOpenGLTexture(backgroundImage, QOpenGLTexture::GenerateMipMaps);
    m_backgroundTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_backgroundImageSize = backgroundImage.size();
    m_textureBlitter.create();
}

void CompositorWindow::drawBackground()
{
    for (int y = 0; y < height(); y += m_backgroundImageSize.height()) {
        for (int x = 0; x < width(); x += m_backgroundImageSize.width()) {
            QMatrix4x4 targetTransform = QOpenGLTextureBlitter::targetTransform(QRect(QPoint(x,y), m_backgroundImageSize), QRect(QPoint(0,0), size()));
            m_textureBlitter.blit(m_backgroundTexture->textureId(),
                                  targetTransform,
                                  QOpenGLTextureBlitter::OriginTopLeft);
        }
    }
}

QPointF CompositorWindow::getAnchorPosition(const QPointF &position, int resizeEdge, const QSize &windowSize)
{
    float y = position.y();
    if (resizeEdge & QWaylandXdgSurface::ResizeEdge::TopEdge)
        y += windowSize.height();

    float x = position.x();
    if (resizeEdge & QWaylandXdgSurface::ResizeEdge::LeftEdge)
        x += windowSize.width();

    return QPointF(x, y);
}

QPointF CompositorWindow::getAnchoredPosition(const QPointF &anchorPosition, int resizeEdge, const QSize &windowSize)
{
    return anchorPosition - getAnchorPosition(QPointF(), resizeEdge, windowSize);
}

void CompositorWindow::paintGL()
{
    m_compositor->startRender();
    QOpenGLFunctions *functions = context()->functions();
    functions->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_textureBlitter.bind();
    //drawBackground();

    functions->glEnable(GL_BLEND);
    functions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLenum currentTarget = GL_TEXTURE_2D;
    Q_FOREACH (WindowCompositorView *view, m_compositor->views()) {
        if (view->isCursor())
            continue;
        GLenum target;
        GLuint textureId = view->getTexture(&target);
        if (!textureId || !target)
            continue;
        if (target != currentTarget) {
            currentTarget = target;
            m_textureBlitter.bind(currentTarget);
        }
        QWaylandSurface *surface = view->surface();
        if (surface && surface->isMapped()) {
            QSize s = surface->size();
            if (!s.isEmpty()) {
                if (m_mouseView == view && m_grabState == ResizeGrab && m_resizeAnchored)
                    view->setPosition(getAnchoredPosition(m_resizeAnchorPosition, m_resizeEdge, s));
                QPointF pos = view->position() + view->parentPosition();
                QRectF surfaceGeometry(pos, s);
                QOpenGLTextureBlitter::Origin surfaceOrigin =
                        view->currentBuffer().origin() == QWaylandSurface::OriginTopLeft
                        ? QOpenGLTextureBlitter::OriginTopLeft
                        : QOpenGLTextureBlitter::OriginBottomLeft;
                QMatrix4x4 targetTransform = QOpenGLTextureBlitter::targetTransform(surfaceGeometry, QRect(QPoint(), size()));
                m_textureBlitter.blit(textureId, targetTransform, surfaceOrigin);
            }
        }
    }
    functions->glDisable(GL_BLEND);

    m_textureBlitter.release();
    m_compositor->endRender();
}

WindowCompositorView *CompositorWindow::viewAt(const QPointF &point)
{
    WindowCompositorView *ret = 0;
    Q_FOREACH (WindowCompositorView *view, m_compositor->views()) {
        if (view == m_dragIconView)
            continue;
        QPointF topLeft = view->position();
        QWaylandSurface *surface = view->surface();
        QRectF geo(topLeft, surface->size());
        if (geo.contains(point))
            ret = view;
    }
    return ret;
}

void CompositorWindow::startMove()
{
    m_grabState = MoveGrab;
}

void CompositorWindow::startResize(int edge, bool anchored)
{
    if(m_mouseView.isNull())
        return;
    m_initialSize = m_mouseView->windowSize();
    m_grabState = ResizeGrab;
    m_resizeEdge = edge;
    m_resizeAnchored = anchored;
    m_resizeAnchorPosition = getAnchorPosition(m_mouseView->position(), edge, m_mouseView->surface()->size());
}

void CompositorWindow::startDrag(WindowCompositorView *dragIcon)
{
    m_grabState = DragGrab;
    m_dragIconView = dragIcon;
    m_compositor->raise(dragIcon);
}

void CompositorWindow::mousePressEvent(QMouseEvent *e)
{
    if (mouseGrab())
        return;
    if (m_mouseView.isNull()) {
        m_mouseView = viewAt(e->localPos());
        if (!m_mouseView) {
            m_compositor->closePopups();
            return;
        }
        if (e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::MetaModifier)
            m_grabState = MoveGrab; //start move
        else
            m_compositor->raise(m_mouseView);
        m_initialMousePos = e->localPos();
        m_mouseOffset = e->localPos() - m_mouseView->position();

        QMouseEvent moveEvent(QEvent::MouseMove, e->localPos(), e->globalPos(), Qt::NoButton, Qt::NoButton, e->modifiers());
        sendMouseEvent(&moveEvent, m_mouseView);
    }
    sendMouseEvent(e, m_mouseView);
}

void CompositorWindow::mouseReleaseEvent(QMouseEvent *e)
{
    if (!mouseGrab())
        sendMouseEvent(e, m_mouseView);
    if (e->buttons() == Qt::NoButton) {
        if (m_grabState == DragGrab) {
            WindowCompositorView *view = viewAt(e->localPos());
            m_compositor->handleDrag(view, e);
        }
        m_mouseView = 0;
        m_grabState = NoGrab;
    }
}

void CompositorWindow::mouseMoveEvent(QMouseEvent *e)
{
    switch (m_grabState) {
    case NoGrab: {
        WindowCompositorView *view = m_mouseView ? m_mouseView.data() : viewAt(e->localPos());
        sendMouseEvent(e, view);
        if (!view)
            setCursor(Qt::ArrowCursor);
    }
        break;
    case MoveGrab: {
        if(m_mouseView.isNull())
        {
            qDebug() << "CompositorWindow::mouseMoveEvent m_mouseView is null";
            break;
        }
        m_mouseView->setPosition(e->localPos() - m_mouseOffset);
        update();
    }
        break;
    case ResizeGrab: {
        QPoint delta = (e->localPos() - m_initialMousePos).toPoint();
        m_compositor->handleResize(m_mouseView, m_initialSize, delta, m_resizeEdge);
    }
        break;
    case DragGrab: {
        WindowCompositorView *view = viewAt(e->localPos());
        m_compositor->handleDrag(view, e);
        if (m_dragIconView) {
            m_dragIconView->setPosition(e->localPos() + m_dragIconView->offset());
            update();
        }
    }
        break;
    }
}

void CompositorWindow::sendMouseEvent(QMouseEvent *e, WindowCompositorView *target)
{
    if (!target)
        return;

    QPointF mappedPos = e->localPos() - target->position();
    QMouseEvent viewEvent(e->type(), mappedPos, e->localPos(), e->button(), e->buttons(), e->modifiers());
    m_compositor->handleMouseEvent(target, &viewEvent);
}

void CompositorWindow::keyPressEvent(QKeyEvent *e)
{
    m_compositor->defaultInputDevice()->sendKeyPressEvent(e->nativeScanCode());
}

void CompositorWindow::keyReleaseEvent(QKeyEvent *e)
{
    m_compositor->defaultInputDevice()->sendKeyReleaseEvent(e->nativeScanCode());
}
