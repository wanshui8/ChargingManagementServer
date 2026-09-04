QT += core gui widgets network sql charts

CONFIG += c++11
TARGET = charging_server
TEMPLATE = app

# 共享协议库
include(../common/common.pri)

SOURCES += \
    main.cpp \
    database.cpp \
    serverservice.cpp \
    adminloginwidget.cpp \
    mainwindow.cpp \
    salespage.cpp \
    pilestatuspage.cpp \
    pilemanagepage.cpp \
    stationmanagepage.cpp \
    usermanagepage.cpp \
    orderspage.cpp

HEADERS += \
    database.h \
    serverservice.h \
    adminloginwidget.h \
    mainwindow.h \
    salespage.h \
    pilestatuspage.h \
    pilemanagepage.h \
    stationmanagepage.h \
    usermanagepage.h \
    orderspage.h