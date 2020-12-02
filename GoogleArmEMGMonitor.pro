QT       += core gui printsupport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += app_bundle
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QCUSTOMPLOT_USE_OPENGL

#QMAKE_LFLAGS += -Bstatic

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    emgdatasource.cpp \
    main.cpp \
    mainwindow.cpp \
    preferences.cpp \
    qcustomplot.cpp \
    tcpsocket.cpp

HEADERS += \
    ErrorDef.h \
    MyTime.h \
    RingBuffer.h \
    SshSession.h \
    TempFile.h \
    emgdatasource.h \
    mainwindow.h \
    preferences.h \
    qcustomplot.h \
    tcpsocket.h

FORMS += \
    mainwindow.ui \
    preferences.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

macx: LIBS += -L/usr/local/lib/ -lssh

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include
