QT += gui gui-private core-private waylandcompositor waylandcompositor-private

HEADERS += \
    compositorwindow.h \
    windowcompositor.h

SOURCES += main.cpp \
    compositorwindow.cpp \
    windowcompositor.cpp
INCLUDEPATH += ../pacp ../core

RESOURCES += pdwindowcompositor.qrc

include(../pacp/pacp.pri))
