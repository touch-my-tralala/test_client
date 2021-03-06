#-------------------------------------------------
#
# Project created by QtCreator 2021-04-26T14:18:53
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client_test
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

CONFIG += c++11

SOURCES += \
        autoupdater/batfilecreator.cpp \
        autoupdater/restautoupdater.cpp \
        context_config/contextconfiguration.cpp \
        main.cpp \
        mainwindow.cpp \
        table_model/mytablewidget.cpp \
        table_model/tablemodel.cpp \
        widgets/hostinputdialog.cpp \
        widgets/sendgoosewidget.cpp \
        widgets/updateinputdialog.cpp

HEADERS += \
        autoupdater/batfilecreator.h \
        autoupdater/restautoupdater.h \
        context_config/contextconfiguration.h \
        json_keys/keys.h \
        mainwindow.h \
        table_model/mytablewidget.h \
        table_model/tablemodel.h \
        widgets/hostinputdialog.h \
        widgets/sendgoosewidget.h \
        widgets/updateinputdialog.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
