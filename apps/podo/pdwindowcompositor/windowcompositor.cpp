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

#include "windowcompositor.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QTouchEvent>

#include <QtWaylandCompositor/QWaylandXdgShell>
#include <QtWaylandCompositor/QWaylandWlShellSurface>
#include <QtWaylandCompositor/qwaylandinput.h>
#include <QtWaylandCompositor/qwaylanddrag.h>

#include <QDebug>
#include <QOpenGLContext>
#include <QWaylandClient>
#ifndef GL_TEXTURE_EXTERNAL_OES
#define GL_TEXTURE_EXTERNAL_OES 0x8D65
#endif

WindowCompositorView::WindowCompositorView()
    : m_textureTarget(GL_TEXTURE_2D)
    , m_texture(0)
    , m_wlShellSurface(nullptr)
    , m_xdgSurface(nullptr)
    , m_xdgPopup(nullptr)
    , m_parentView(nullptr)
{

}

GLuint WindowCompositorView::getTexture(GLenum *target)
{
    QWaylandBufferRef buf = currentBuffer();
    GLuint streamingTexture = buf.textureForPlane(0);
    if (streamingTexture)
        m_texture = streamingTexture;

    if (!buf.isShm() && buf.bufferFormatEgl() == QWaylandBufferRef::BufferFormatEgl_EXTERNAL_OES)
        m_textureTarget = GL_TEXTURE_EXTERNAL_OES;

    if (advance()) {
        buf = currentBuffer();
        if (!m_texture)
            glGenTextures(1, &m_texture);

        glBindTexture(m_textureTarget, m_texture);
        if (buf.isShm())
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        buf.bindToTexture();
    }

    buf.updateTexture();

    if (target)
        *target = m_textureTarget;

    return m_texture;
}

bool WindowCompositorView::isCursor() const
{
    return surface()->isCursorSurface();
}

void WindowCompositorView::onXdgSetMinimized()
{
   // m_xdgSurface->
}

void WindowCompositorView::onXdgSetMaximized()
{
    m_xdgSurface->requestMaximized(output()->geometry().size());

    // An improvement here, would have been to wait for the commit after the ack_configure for the
    // request above before moving the window. This would have prevented the window from being
    // moved until the contents of the window had actually updated. This improvement is left as an
    // exercise for the reader.
    setPosition(QPoint(0, 0));
}

void WindowCompositorView::onXdgUnsetMaximized()
{
    m_xdgSurface->requestUnMaximized();
}

void WindowCompositorView::onXdgSetFullscreen(QWaylandOutput* clientPreferredOutput)
{
    QWaylandOutput *outputToFullscreen = clientPreferredOutput
            ? clientPreferredOutput
            : output();

    m_xdgSurface->requestFullscreen(outputToFullscreen->geometry().size());

    // An improvement here, would have been to wait for the commit after the ack_configure for the
    // request above before moving the window. This would have prevented the window from being
    // moved until the contents of the window had actually updated. This improvement is left as an
    // exercise for the reader.
    setPosition(outputToFullscreen->position());
}

void WindowCompositorView::onOffsetForNextFrame(const QPoint &offset)
{
    m_offset = offset;
    setPosition(position() + offset);
}

void WindowCompositorView::onXdgWindowGeometryChanged()
{
 //   qDebug() << m_xdgSurface->windowGeometry();
}

void WindowCompositorView::onWlSetMaximized(QWaylandOutput *clientPreferredOutput)
{
    QWaylandOutput *outputToFullscreen = clientPreferredOutput
            ? clientPreferredOutput
            : output();

    this->setPosition(outputToFullscreen->position());
    m_wlShellSurface->sendConfigure(outputToFullscreen->window()->size(), QWaylandWlShellSurface::NoneEdge);
 //   m_wlShellSurface->requestFullscreen(outputToFullscreen->geometry().size());

    // An improvement here, would have been to wait for the commit after the ack_configure for the
    // request above before moving the window. This would have prevented the window from being
    // moved until the contents of the window had actually updated. This improvement is left as an
    // exercise for the reader.
    // setPosition(outputToFullscreen->position());
}

void WindowCompositorView::onWlgeometryChanged(const QPoint &position)
{
    qDebug()<< "WindowCompositorView::onWlgeometryChanged " << position;
}

void WindowCompositorView::onXdgUnsetFullscreen()
{
    onXdgUnsetMaximized();
}

//
//  SIGNALS
//      QWaylandWlShell
//      void    createShellSurface(QWaylandSurface *surface, const QWaylandResource &resource)
//      void    shellSurfaceCreated(QWaylandWlShellSurface *shellSurface)
//
//      QWaylandXdgShell
//      void    createXdgPopup(QWaylandSurface *surface, QWaylandSurface *parent, QWaylandInputDevice *seat, const QPoint &position, const QWaylandResource &resource)
//      void    createXdgSurface(QWaylandSurface *surface, const QWaylandResource &resource)
//      void    pong(uint serial)
//      void    xdgPopupCreated(QWaylandXdgPopup *xdgPopup)
//      void    xdgSurfaceCreated(QWaylandXdgSurface *xdgSurface)
//
//      QWaylandCompositor
//      void    createSurface(QWaylandClient *client, uint id, int version)
//      void    defaultInputDeviceChanged(QWaylandInputDevice *newDevice, QWaylandInputDevice *oldDevice)
//      void    defaultOutputChanged()
//      void    subsurfaceChanged(QWaylandSurface *child, QWaylandSurface *parent)
//      void    surfaceAboutToBeDestroyed(QWaylandSurface *surface)
//      void    surfaceCreated(QWaylandSurface *surface)
//      void    useHardwareIntegrationExtensionChanged()
//

WindowCompositor::WindowCompositor(QWindow *window)
    : QWaylandCompositor()
    , m_window(window)
    , m_wlShell(new QWaylandWlShell(this))
    , m_xdgShell(new QWaylandXdgShell(this))
{
    connect(m_wlShell, &QWaylandWlShell::shellSurfaceCreated, this, &WindowCompositor::onWlShellSurfaceCreated);
    connect(m_xdgShell, &QWaylandXdgShell::xdgSurfaceCreated, this, &WindowCompositor::onXdgSurfaceCreated);
    connect(m_xdgShell, &QWaylandXdgShell::createXdgPopup, this, &WindowCompositor::onCreateXdgPopup);

    m_pacpClient = new PACPClient();
    m_pacpClient->setName("pdwaylandcompositor");
    m_pacpClient->setClientType(PACPProtocol::COMPOSITOR);
    m_pacpClient->connectToServer();

    connect(m_pacpClient, &PACPClient::requestMoveAtCompositor, this, &WindowCompositor::moveWindowCompositorView);
    connect(m_pacpClient, &PACPClient::requestRaiseAtCompositor, this, &WindowCompositor::raiseWindowCompositorView);
    connect(m_pacpClient, &PACPClient::requestLowerAtCompositor, this, &WindowCompositor::lowerWindowCompositorView);
}

WindowCompositor::~WindowCompositor()
{
    m_viewPidMap.clear();
    m_pacpClient->deleteLater();
    m_pacpClient = 0;
}

void WindowCompositor::create()
{
    new QWaylandOutput(this, m_window);
    QWaylandCompositor::create();

    connect(this, &QWaylandCompositor::surfaceCreated, this, &WindowCompositor::onSurfaceCreated);
    connect(defaultInputDevice(), &QWaylandInputDevice::cursorSurfaceRequest, this, &WindowCompositor::adjustCursorSurface);
    connect(defaultInputDevice()->drag(), &QWaylandDrag::dragStarted, this, &WindowCompositor::startDrag);

    connect(this, &QWaylandCompositor::subsurfaceChanged, this, &WindowCompositor::onSubsurfaceChanged);
}

void WindowCompositor::onSurfaceCreated(QWaylandSurface *surface)
{
    connect(surface, &QWaylandSurface::surfaceDestroyed, this, &WindowCompositor::surfaceDestroyed);
    connect(surface, &QWaylandSurface::mappedChanged, this, &WindowCompositor::surfaceMappedChanged);
    connect(surface, &QWaylandSurface::redraw, this, &WindowCompositor::triggerRender);
    connect(surface, &QWaylandSurface::subsurfacePositionChanged, this, &WindowCompositor::onSubsurfacePositionChanged);

    WindowCompositorView *view = new WindowCompositorView;
    view->setSurface(surface);
    view->setOutput(outputFor(m_window));

    m_views << view;
    // viewReorder();

    connect(view, &QWaylandView::surfaceDestroyed, this, &WindowCompositor::viewSurfaceDestroyed);
    connect(surface, &QWaylandSurface::offsetForNextFrame, view, &WindowCompositorView::onOffsetForNextFrame);
}

void WindowCompositor::surfaceMappedChanged()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
    if (surface->isMapped()) {
        if (surface->role() == QWaylandWlShellSurface::role()
                || surface->role() == QWaylandXdgSurface::role()
                || surface->role() == QWaylandXdgPopup::role()) {
            defaultInputDevice()->setKeyboardFocus(surface);
        }
    } else if (popupActive()) {
        for (int i = 0; i < m_popupViews.count(); i++) {
            if (m_popupViews.at(i)->surface() == surface) {
                m_popupViews.removeAt(i);
                break;
            }
        }
    }
    triggerRender();
}

void WindowCompositor::surfaceDestroyed()
{
    // triggerRender();
}

void WindowCompositor::viewSurfaceDestroyed()
{
    WindowCompositorView *view = qobject_cast<WindowCompositorView*>(sender());
    // bool isTopView = (m_views.at(m_views.count() - 1) == view);

    unsigned int texture;
    unsigned int textureId = view->getTexture(&texture);
    if (textureId)
        glDeleteTextures(1, &textureId);

    m_views.removeAll(view);
    if(m_viewPidMap.values().contains(view))
    {
        QString key = m_viewPidMap.key(view);
        if(!key.isEmpty())
            m_viewPidMap.remove(key);
    }
    delete view;

    triggerRender();

    // if ( (0 != m_views.count()) && isTopView )
    // {
    //     WindowCompositorView *pTopView = m_views.at(m_views.count() - 1);
    //     if( !m_viewPidMap.values().contains(pTopView) )
    //         return;

    //     QWaylandInputDevice* inputDevice = defaultInputDevice();
    //     if( pTopView->surface())
    //         inputDevice->setKeyboardFocus(pTopView->surface());
    // }
}

WindowCompositorView * WindowCompositor::findView(const QWaylandSurface *s) const
{
    Q_FOREACH (WindowCompositorView* view, m_views) {
        if (view->surface() == s)
            return view;
    }
    return Q_NULLPTR;
}

void WindowCompositor::onWlShellSurfaceCreated(QWaylandWlShellSurface *wlShellSurface)
{
    connect(wlShellSurface, &QWaylandWlShellSurface::startMove, this, &WindowCompositor::onStartMove);
    connect(wlShellSurface, &QWaylandWlShellSurface::startResize, this, &WindowCompositor::onWlStartResize);
    connect(wlShellSurface, &QWaylandWlShellSurface::setTransient, this, &WindowCompositor::onSetTransient);
    connect(wlShellSurface, &QWaylandWlShellSurface::setPopup, this, &WindowCompositor::onSetPopup);

    WindowCompositorView *view = findView(wlShellSurface->surface());
    Q_ASSERT(view);
    view->m_wlShellSurface = wlShellSurface;
    if(!m_viewPidMap.contains(QString::number(wlShellSurface->surface()->client()->processId())))
        m_viewPidMap.insert(QString::number(wlShellSurface->surface()->client()->processId()),view);
    connect(wlShellSurface, &QWaylandWlShellSurface::setMaximized, view, &WindowCompositorView::onWlSetMaximized);

  //  connect(view, &QWaylandSurface::subsurfacePositionChanged, view, &WindowCompositorView::onWlgeometryChanged);
}

void WindowCompositor::onXdgSurfaceCreated(QWaylandXdgSurface *xdgSurface)
{
    connect(xdgSurface, &QWaylandXdgSurface::startMove, this, &WindowCompositor::onStartMove);
    connect(xdgSurface, &QWaylandXdgSurface::startResize, this, &WindowCompositor::onXdgStartResize);

    WindowCompositorView *view = findView(xdgSurface->surface());
    Q_ASSERT(view);
    view->m_xdgSurface = xdgSurface;

    connect(xdgSurface, &QWaylandXdgSurface::setMaximized, view, &WindowCompositorView::onXdgSetMaximized);
    connect(xdgSurface, &QWaylandXdgSurface::setFullscreen, view, &WindowCompositorView::onXdgSetFullscreen);
    connect(xdgSurface, &QWaylandXdgSurface::unsetMaximized, view, &WindowCompositorView::onXdgUnsetMaximized);
    connect(xdgSurface, &QWaylandXdgSurface::unsetFullscreen, view, &WindowCompositorView::onXdgUnsetFullscreen);
    connect(xdgSurface, &QWaylandXdgSurface::windowGeometryChanged, view, &WindowCompositorView::onXdgWindowGeometryChanged);
    connect(xdgSurface, &QWaylandXdgSurface::setMinimized, view, &WindowCompositorView::onXdgSetMinimized);

    if(!m_viewPidMap.contains(QString::number(xdgSurface->surface()->client()->processId())))
        m_viewPidMap.insert(QString::number(xdgSurface->surface()->client()->processId()),view);
}

void WindowCompositor::onCreateXdgPopup(QWaylandSurface *surface, QWaylandSurface *parent,
                                        QWaylandInputDevice *inputDevice, const QPoint &position,
                                        const QWaylandResource &resource)
{
    Q_UNUSED(inputDevice);

    QWaylandXdgPopup *xdgPopup = new QWaylandXdgPopup(m_xdgShell, surface, parent, resource);

    WindowCompositorView *view = findView(surface);
    Q_ASSERT(view);

    WindowCompositorView *parentView = findView(parent);
    Q_ASSERT(parentView);

    view->setPosition(parentView->position() + position);
    view->m_xdgPopup = xdgPopup;
}

void WindowCompositor::onStartMove()
{
    closePopups();
    emit startMove();
}

void WindowCompositor::onWlStartResize(QWaylandInputDevice *, QWaylandWlShellSurface::ResizeEdge edges)
{
    closePopups();
    emit startResize(int(edges), false);
}

void WindowCompositor::onXdgStartResize(QWaylandInputDevice *inputDevice,
                                        QWaylandXdgSurface::ResizeEdge edges)
{
    Q_UNUSED(inputDevice);
    emit startResize(int(edges), true);
}

void WindowCompositor::onSetTransient(QWaylandSurface *parent, const QPoint &relativeToParent, QWaylandWlShellSurface::FocusPolicy focusPolicy)
{
    Q_UNUSED(focusPolicy);
    QWaylandWlShellSurface *wlShellSurface = qobject_cast<QWaylandWlShellSurface*>(sender());
    WindowCompositorView *view = findView(wlShellSurface->surface());

    if (view) {
        raise(view);
        WindowCompositorView *parentView = findView(parent);
        if (parentView)
            view->setPosition(parentView->position() + relativeToParent);
    }
}

void WindowCompositor::onSetPopup(QWaylandInputDevice *inputDevice, QWaylandSurface *parent, const QPoint &relativeToParent)
{
    Q_UNUSED(inputDevice);
    QWaylandWlShellSurface *surface = qobject_cast<QWaylandWlShellSurface*>(sender());
    WindowCompositorView *view = findView(surface->surface());
    m_popupViews << view;
    if (view) {
        raise(view);
        WindowCompositorView *parentView = findView(parent);
        if (parentView)
            view->setPosition(parentView->position() + relativeToParent);
    }
}

void WindowCompositor::onSubsurfaceChanged(QWaylandSurface *child, QWaylandSurface *parent)
{
    WindowCompositorView *view = findView(child);
    WindowCompositorView *parentView = findView(parent);
    view->setParentView(parentView);
}

void WindowCompositor::onSubsurfacePositionChanged(const QPoint &position)
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface*>(sender());
    if (!surface)
        return;
    WindowCompositorView *view = findView(surface);
    view->setPosition(position);
    triggerRender();
}

void WindowCompositor::triggerRender()
{
    m_window->requestUpdate();
}

void WindowCompositor::moveWindowCompositorView(const QString &pid, const QPoint &position)
{
   //qDebug() << "WindowCompositor::moveWindowCompositorView move pid:" << pid << " point:" << position;
    if(m_viewPidMap.contains(pid))
    {
        qDebug() << "WindowCompositor::moveWindowCompositorView move pid:" << pid << " point:" << position;
        WindowCompositorView* view = m_viewPidMap.value(pid);
        view->setPosition(position);
        triggerRender();
    }

}

void WindowCompositor::raiseWindowCompositorView(const QString &pid)
{
    qDebug("%s(), %d\n", __FUNCTION__, __LINE__);
    if(m_viewPidMap.contains(pid))
    {
        qDebug() << "WindowCompositor::raiseWindowCompositorView pid:" << pid;
        WindowCompositorView* view = m_viewPidMap.value(pid);
        raise(view);
        triggerRender();
    }
}

void WindowCompositor::lowerWindowCompositorView(const QString &pid)
{
    qDebug("%s(), %d\n", __FUNCTION__, __LINE__);
    if(m_viewPidMap.contains(pid))
    {
        qDebug() << "WindowCompositor::lowerWindowCompositorView pid:" << pid;
        WindowCompositorView* view = m_viewPidMap.value(pid);
        lower(view);
        triggerRender();
    }
}

void WindowCompositor::startRender()
{
    QWaylandOutput *out = defaultOutput();
    if (out)
        out->frameStarted();
}

void WindowCompositor::endRender()
{
    QWaylandOutput *out = defaultOutput();
    if (out)
        out->sendFrameCallbacks();
}

void WindowCompositor::updateCursor()
{
    m_cursorView.advance();
    QImage image = m_cursorView.currentBuffer().image();
    if (!image.isNull())
        m_window->setCursor(QCursor(QPixmap::fromImage(image), m_cursorHotspotX, m_cursorHotspotY));
}

void WindowCompositor::adjustCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY)
{
    if ((m_cursorView.surface() != surface)) {
        if (m_cursorView.surface())
            disconnect(m_cursorView.surface(), &QWaylandSurface::redraw, this, &WindowCompositor::updateCursor);
        if (surface)
            connect(surface, &QWaylandSurface::redraw, this, &WindowCompositor::updateCursor);
    }

    m_cursorView.setSurface(surface);
    m_cursorHotspotX = hotspotX;
    m_cursorHotspotY = hotspotY;

    if (surface && surface->isMapped())
        updateCursor();
}

void WindowCompositor::closePopups()
{
    Q_FOREACH (WindowCompositorView *view, m_popupViews) {
        if (view->m_wlShellSurface)
            view->m_wlShellSurface->sendPopupDone();
    }
    m_popupViews.clear();

    m_xdgShell->closeAllPopups();
}

void WindowCompositor::handleMouseEvent(QWaylandView *target, QMouseEvent *me)
{
    if (target && popupActive() && me->type() == QEvent::MouseButtonPress
        && target->surface()->client() != m_popupViews.first()->surface()->client()) {
        closePopups();
    }
    QWaylandInputDevice *input = defaultInputDevice();
    QWaylandSurface *surface = target ? target->surface() : nullptr;
    switch (me->type()) {
        case QEvent::MouseButtonPress:
            input->sendMousePressEvent(me->button());
            if (surface != input->keyboardFocus()) {
                if (surface == nullptr
                        || surface->role() == QWaylandWlShellSurface::role()
                        || surface->role() == QWaylandXdgSurface::role()
                        || surface->role() == QWaylandXdgPopup::role()) {
                    input->setKeyboardFocus(surface);
                }
            }
            break;
    case QEvent::MouseButtonRelease:
         input->sendMouseReleaseEvent(me->button());
         break;
    case QEvent::MouseMove:
        input->sendMouseMoveEvent(target, me->localPos(), me->globalPos());
    default:
        break;
    }
}

void WindowCompositor::handleResize(WindowCompositorView *target, const QSize &initialSize, const QPoint &delta, int edge)
{
    QWaylandWlShellSurface *wlShellSurface = target->m_wlShellSurface;
    if (wlShellSurface) {
        QWaylandWlShellSurface::ResizeEdge edges = QWaylandWlShellSurface::ResizeEdge(edge);
        QSize newSize = wlShellSurface->sizeForResize(initialSize, delta, edges);
        wlShellSurface->sendConfigure(newSize, edges);
    }

    QWaylandXdgSurface *xdgSurface = target->m_xdgSurface;
    if (xdgSurface) {
        QWaylandXdgSurface::ResizeEdge edges = static_cast<QWaylandXdgSurface::ResizeEdge>(edge);
        QSize newSize = xdgSurface->sizeForResize(initialSize, delta, edges);
        xdgSurface->requestResizing(newSize);
    }
}

void WindowCompositor::startDrag()
{
    QWaylandDrag *currentDrag = defaultInputDevice()->drag();
    Q_ASSERT(currentDrag);
    WindowCompositorView *iconView = findView(currentDrag->icon());
    iconView->setPosition(m_window->mapFromGlobal(QCursor::pos()));

    emit dragStarted(iconView);
}

void WindowCompositor::handleDrag(WindowCompositorView *target, QMouseEvent *me)
{
    QPointF pos = me->localPos();
    QWaylandSurface *surface = 0;
    if (target) {
        pos -= target->position();
        surface = target->surface();
    }
    QWaylandDrag *currentDrag = defaultInputDevice()->drag();
    currentDrag->dragMove(surface, pos);
    if (me->buttons() == Qt::NoButton)
        currentDrag->drop();
}

// We only have a flat list of views, plus pointers from child to parent,
// so maintaining a stacking order gets a bit complex. A better data
// structure is left as an exercise for the reader.

void WindowCompositor::raise(WindowCompositorView *view)
{
    viewReorder();

    QString title;
    if(view->hasShell())
    {
        title = view->m_wlShellSurface->title();
    }else{
        title = view->m_xdgSurface->title();
    }
    if(title=="PDLauncher")
        return;

    int startPos = m_views.indexOf(view);
    int endPos   = m_views.count() - 1;

    for (int i = startPos; i < endPos; i++ )
    {
        m_views.swap(i, i+1);
    }

    WindowCompositorView *pTopView = m_views.at(m_views.count() - 1);
    QWaylandInputDevice* inputDevice = defaultInputDevice();
    if( pTopView->surface())
        inputDevice->setKeyboardFocus(pTopView->surface());

   // printViewList();
}

void WindowCompositor::lower(WindowCompositorView *view)
{
    viewReorder();

    QString title;
    if(view->hasShell())
    {
        title = view->m_wlShellSurface->title();
    }else{
        title = view->m_xdgSurface->title();
    }
    if(title=="PDLauncher")
        return;

    int startPos = findStartView();
    int endPos   = m_views.indexOf(view);

    if( startPos < 0 )
        return ;

    for( int i = endPos; i > startPos; i-- )
    {
        m_views.swap(i-1, i);
    }

    WindowCompositorView *pTopView = m_views.at(m_views.count() - 1);
    QWaylandInputDevice* inputDevice = defaultInputDevice();
    if( pTopView->surface())
        inputDevice->setKeyboardFocus(pTopView->surface());

    //printViewList();
}

void WindowCompositor::viewReorder()
{
    int viewCount = m_views.count()-1;
    while( viewCount )
    {
        unsigned int texture;
        WindowCompositorView* view = m_views.at(viewCount);
        int id = view->getTexture( &texture );

        if( view->isCursor() || !id || !texture )
        {
            if( findStartView() > viewCount )
                break;

            for( int j = viewCount; j > 0; j-- )
                m_views.swap(j, j-1);

            viewCount = m_views.count()-1;
            continue;
        }
        viewCount--;
    }
}

int WindowCompositor::findStartView()
{
    int index = -1;
    for( int i = 0; i < m_views.count(); i++ )
    {
        WindowCompositorView* view = m_views.at(i);
        unsigned int texture;
        int id = view->getTexture( &texture );

        if( !view->isCursor() && id && texture )
        {
            index = i;
            break;
        }
    }
    // printf(">>>>> start index of view : %d\n", index);
    return index;
}

void WindowCompositor::printViewList()
{
    printf("\n");
    for( int i = 0; i < m_views.count(); i++ )
    {
        WindowCompositorView* view = m_views.at(i);
        unsigned int texture;
        int id = view->getTexture( &texture );

        printf("index(%d) : view( %p ), hasShell( %d ), isCursor( %d ), textureId( %d ), texture( %d )\n",
            i,
            view, view->hasShell(), view->isCursor(),
            id, texture
        );
    }
}
