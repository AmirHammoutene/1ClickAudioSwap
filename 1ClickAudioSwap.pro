QT       += core gui
QT       -= network
greaterThan(QT_MAJOR_VERSION,4): QT += widgets
greaterThan(QT_MAJOR_VERSION,5): QT += core5compat multimedia

CONFIG += c++1z

TEMPLATE = app

CONFIG(release, debug|release):TARGET = 1ClickAudioSwap
CONFIG(debug, debug|release):TARGET = 1ClickAudioSwapd

SOURCES += \
    main.cpp \
    mainwidget.cpp

HEADERS += \
    mainwidget.h

RESOURCES += \
    resources.qrc

RC_FILE = resource.rc
