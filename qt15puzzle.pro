TEMPLATE = app
QT += widgets
CONFIG += qt thread
SOURCES += main.cpp \
    mainwindow.cpp \
    puzzlewidget.cpp \
    fifteenpuzzle.cpp \
    state.cpp
HEADERS += mainwindow.h \
    puzzlewidget.h \
    fifteenpuzzle.h \
    state.h

RESOURCES += \
    icons.qrc
