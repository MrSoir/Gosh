#-------------------------------------------------
#
# Project created by QtCreator 2017-12-05T12:21:58
#
#-------------------------------------------------

QT       += core gui

CONFIG += c++14

QMAKE_CXXFLAGS += $(shell Magick++-config --cppflags --cxxflags)
LIBS += $(shell Magick++-config --ldflags --libs)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Gosh
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    filescoordinator.cpp \
    fileinfobd.cpp \
    staticfunctions.cpp \
    folderlistener.cpp \
    graphicsfile.cpp \
    graphicsview.cpp \
    dynamicfunctioncaller.cpp \
    windowcoordinator.cpp \
    custommainwindowbd.cpp \
    fileinforef.cpp \
    directoryselectordialog.cpp \
    directoryselectordialogviewer.cpp \
    elapseworker.cpp \
    elapseworkerfunctional.cpp \
    directoryselectionpane.cpp \
    searchfiledialog.cpp \
    deepsearch.cpp \
    deppsearchworker.cpp \
    waitingbargraphicsitem.cpp \
    customgraphicitems.cpp \
    elapsevalidator.cpp \
    menubar.cpp \
    tabselectorpane.cpp \
    tabcoordinator.cpp \
    windowcoordinatorpane.cpp \
    tabcoordinatorpane.cpp \
    tab.cpp \
    tabcaller.cpp \
    tabrect.cpp \
    removewindowdialog.cpp \
    tabcoordinatorii.cpp \
    infodialog.cpp \
    helpdialog.cpp \
    filehandler.cpp \
    programmimeassociation.cpp \
    openwithdialog.cpp \
    openwithgraphicsview.cpp \
    fileselector.cpp \
    zipfiles.cpp \
    canceller.cpp

HEADERS += \
        mainwindow.h \
    filescoordinator.h \
    fileinfobd.h \
    orderby.h \
    staticfunctions.h \
    folderlistener.h \
    graphicsfile.h \
    graphicsview.h \
    graphicsviewupdater.h \
    dynamicfunctioncaller.h \
    windowcoordinator.h \
    custommainwindowbd.h \
    fileinforef.h \
    directoryselectordialog.h \
    directoryselectordialogviewer.h \
    elapseworker.h \
    elapseworkerfunctional.h \
    directoryselectionpane.h \
    searchfiledialog.h \
    deepsearch.h \
    deppsearchworker.h \
    waitingbargraphicsitem.h \
    customgraphicitems.h \
    elapsevalidator.h \
    menubar.h \
    tabselectorpane.h \
    tabcoordinator.h \
    windowcoordinatorpane.h \
    tabcoordinatorpane.h \
    tab.h \
    tabcaller.h \
    tabrect.h \
    removewindowdialog.h \
    orientation.h \
    tabcoordinatorii.h \
    infodialog.h \
    helpdialog.h \
    filehandler.h \
    programmimeassociation.h \
    openwithdialog.h \
    openwithgraphicsview.h \
    fileselector.h \
    zipfiles.h \
    canceller.h

FORMS += \
        mainwindow.ui

RESOURCES += \
    resources.qrc
