QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ccommandpane.cpp \
    cdictpair.cpp \
    cfunctionpane.cpp \
    cfunctionparam.cpp \
    cjsonobj.cpp \
    customlistelement.cpp \
    keyboardwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    message.cpp \
    orientablepushbutton.cpp

HEADERS += \
    ccommandpane.h \
    cdictpair.h \
    cfunctionpane.h \
    cfunctionparam.h \
    cjsonobj.h \
    customlistelement.h \
    keyboardwindow.h \
    mainwindow.h \
    message.h \
    orientablepushbutton.h \
    roles.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = appicon.ico
